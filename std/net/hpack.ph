// =============================================================================
// std/net/hpack.ph - HPACK, HTTP/2's header compression (RFC 7541).
//
//     let mut enc = hpack.Encoder.init();
//     defer enc.deinit();
//     enc.encode(headers, &mut block)!;           // headers: []hpack.Header
//
//     let mut dec = hpack.Decoder.init(4096);
//     defer dec.deinit();
//     let hs = dec.decode(block.bytes())!;        // valid until the next decode
//
// Both sides keep the dynamic table that later blocks refer to, so blocks
// must be decoded in the order they were encoded. The encoder indexes every
// header (so repeats cost a byte or two) and Huffman-codes strings where
// that is shorter, as Python's hpack does, whose output it matches. The
// decoder checks everything: indices, integer sizes, Huffman padding, table
// size updates, and the total size of the header list (max_list_size).
// =============================================================================

import "../collections/vec.ph" as vec;
import "../encoding/hex.ph" as hex;
import "../mem.ph" as mem;
import "../os/fs.ph" as fs;
import "../str.ph" as strs;
import "hpack/tables.ph" as t;

error { DecodeError, ListTooLarge, OutOfMemory }

pub struct Header {
    name: []u8,
    value: []u8,
}

struct Entry {
    name: []mut u8,  // owned
    value: []mut u8,
}

// The dynamic table: newest first; sizes count 32 bytes per entry too.
struct Table {
    entries: vec.Vec.[Entry],
    size: usize,
    max: usize,

    fn init(max: usize) Table {
        return Table.{ entries: vec.Vec.[Entry].init(), size: 0, max: max };
    }

    fn deinit(self: *mut Table) void {
        for e in self.entries.items() {
            mem.free(e.name);
            mem.free(e.value);
        }
        self.entries.deinit();
    }

    fn evict_to(self: *mut Table, limit: usize) void {
        while (self.size > limit) and (self.entries.len > 0) {
            let e = self.entries.pop() catch break;
            self.size -= e.name.len + e.value.len + 32;
            mem.free(e.name);
            mem.free(e.value);
        }
    }

    fn resize(self: *mut Table, max: usize) void {
        self.max = max;
        self.evict_to(max);
    }

    fn add(self: *mut Table, name: []u8, value: []u8) !void {
        let need = name.len + value.len + 32;
        if need > self.max {
            // (an entry bigger than the table empties it)
            self.evict_to(0);
            return;
        }
        self.evict_to(self.max - need);
        let n = strs.dup(name)!;
        let v = strs.dup(value) catch |e| {
            mem.free(n);
            return e;
        };
        self.entries.insert(0, Entry.{ name: n, value: v }) catch |e| {
            mem.free(n);
            mem.free(v);
            return e;
        };
        self.size += need;
    }

    // entry i (1-based, the static table first)
    fn get(self: *Table, i: usize, name: *mut []u8, value: *mut []u8) bool {
        if i == 0 {
            return false;
        }
        if i <= 61 {
            name.* = t.STATIC_NAMES[i - 1];
            value.* = t.STATIC_VALUES[i - 1];
            return true;
        }
        let d = i - 62;
        if d >= self.entries.len {
            return false;
        }
        let e = self.entries.get(d);
        name.* = e.name;
        value.* = e.value;
        return true;
    }
}

// -----------------------------------------------------------------------------
// Integers, strings and Huffman
// -----------------------------------------------------------------------------

fn put_int(out: *mut strs.String, first: u8, prefix: u32, v: usize) !void {
    let max = (@as(usize, 1) << @truncate(usize, prefix)) - 1;
    if v < max {
        out.push(first | @truncate(u8, v))!;
        return;
    }
    out.push(first | @truncate(u8, max))!;
    let mut r = v - max;
    while r >= 128 {
        out.push(@truncate(u8, r & 127) | 0x80)!;
        r >>= 7;
    }
    out.push(@truncate(u8, r))!;
}

fn get_int(b: []u8, i: *mut usize, prefix: u32) !usize {
    if i.* >= b.len {
        return error.DecodeError;
    }
    let max = (@as(usize, 1) << @truncate(usize, prefix)) - 1;
    let mut v = @as(usize, b[i.*]) & max;
    i.* += 1;
    if v < max {
        return v;
    }
    let mut shift: usize = 0;
    loop {
        if (i.* >= b.len) or (shift > 21) {
            return error.DecodeError;  // (no header needs more than 2^28)
        }
        let c = b[i.*];
        i.* += 1;
        v += @as(usize, c & 127) << shift;
        shift += 7;
        if (c & 0x80) == 0 {
            return v;
        }
    }
}

fn huffman_len(s: []u8) usize {
    let mut bits: usize = 0;
    for c in s {
        bits += @as(usize, t.HUFF_LENS[@as(usize, c)]);
    }
    return (bits + 7) / 8;
}

fn put_huffman(out: *mut strs.String, s: []u8) !void {
    let mut acc: u64 = 0;
    let mut n: u32 = 0;
    for c in s {
        let code = @as(u64, t.HUFF_CODES[@as(usize, c)]);
        let len = @as(u32, t.HUFF_LENS[@as(usize, c)]);
        acc = (acc << @as(u64, len)) | code;
        n += len;
        while n >= 8 {
            n -= 8;
            out.push(@truncate(u8, acc >> @as(u64, n)))!;
        }
    }
    if n > 0 {
        // padding: the most significant bits of EOS (all ones)
        out.push(@truncate(u8, (acc << @as(u64, 8 - n)) | ((@as(u64, 1) << @as(u64, 8 - n)) - 1)))!;
    }
}

fn put_string(out: *mut strs.String, s: []u8) !void {
    let h = huffman_len(s);
    if h <= s.len {
        put_int(out, 0x80, 7, h)!;
        put_huffman(out, s)!;
    } else {
        put_int(out, 0, 7, s.len)!;
        out.push_str(s)!;
    }
}

// The decoding tree: node k has children tree[2k], tree[2k + 1]; a value
// >= 0x8000 is a leaf holding symbol value - 0x8000.
struct Tree {
    nodes: []mut u16,
    count: usize,

    fn build() !Tree {
        let mut tr = Tree.{ nodes: mem.alloc.[u16](2 * 512)!, count: 1 };
        @mem_set(@cast_slice(u8, tr.nodes), 0);
        for sym in @as(usize, 0)..257 {
            let code = t.HUFF_CODES[sym];
            let len = @as(u32, t.HUFF_LENS[sym]);
            let mut node: usize = 0;
            let mut k = len;
            while k > 0 {
                k -= 1;
                let bit = @as(usize, (code >> k) & 1);
                let slot = (2 * node) + bit;
                if k == 0 {
                    tr.nodes[slot] = 0x8000 | @truncate(u16, sym);
                } else {
                    if tr.nodes[slot] == 0 {
                        tr.nodes[slot] = @truncate(u16, tr.count);
                        tr.count += 1;
                    }
                    node = @as(usize, tr.nodes[slot]);
                }
            }
        }
        return tr;
    }
}

fn decode_huffman(tree: *Tree, s: []u8, out: *mut strs.String) !void {
    let mut node: usize = 0;
    let mut depth: u32 = 0;  // bits since the last symbol
    let mut ones = true;     // and all of them ones
    for c in s {
        let mut k: u32 = 8;
        while k > 0 {
            k -= 1;
            let bit = @as(usize, (c >> @truncate(u8, k)) & 1);
            let next = tree.nodes[(2 * node) + bit];
            depth += 1;
            if bit == 0 {
                ones = false;
            }
            if next >= 0x8000 {
                let sym = next - 0x8000;
                if sym == 256 {
                    return error.DecodeError;  // EOS in a string
                }
                out.push(@truncate(u8, sym))!;
                node = 0;
                depth = 0;
                ones = true;
            } else if next == 0 {
                return error.DecodeError;
            } else {
                node = @as(usize, next);
            }
        }
    }
    // padding: fewer than 8 bits, all ones
    if (depth > 7) or not ones {
        return error.DecodeError;
    }
}

// -----------------------------------------------------------------------------
// The encoder and the decoder
// -----------------------------------------------------------------------------

pub struct Encoder {
    table: Table,
    pending: vec.Vec.[usize],  // table size changes to announce

    pub fn init() Encoder {
        return Encoder.{ table: Table.init(4096), pending: vec.Vec.[usize].init() };
    }

    pub fn deinit(self: *mut Encoder) void {
        self.table.deinit();
        self.pending.deinit();
    }

    /// Changes the table's size (within what the peer allows, its
    /// SETTINGS_HEADER_TABLE_SIZE); announced at the start of the next block.
    pub fn set_max_size(self: *mut Encoder, n: usize) !void {
        if n == self.table.max {
            return;
        }
        self.table.resize(n);
        self.pending.push(n)!;
    }

    /// Appends the block encoding headers to out.
    pub fn encode(self: *mut Encoder, headers: []Header, out: *mut strs.String) !void {
        for n in self.pending.items() {
            put_int(out, 0x20, 5, n)!;
        }
        self.pending.clear();
        for h in headers {
            self.encode_one(h, out)!;
        }
    }

    fn encode_one(self: *mut Encoder, h: Header, out: *mut strs.String) !void {
        // a whole match, or a name: the static table first, then the
        // dynamic one, newest first
        let mut name_at: usize = 0;
        for k in @as(usize, 0)..61 {
            if strs.eq(t.STATIC_NAMES[k], h.name) {
                if strs.eq(t.STATIC_VALUES[k], h.value) {
                    put_int(out, 0x80, 7, k + 1)!;
                    return;
                }
                if name_at == 0 {
                    name_at = k + 1;
                }
            }
        }
        let es = self.table.entries.items();
        for k in @as(usize, 0)..es.len {
            if strs.eq(es[k].name, h.name) {
                if strs.eq(es[k].value, h.value) {
                    put_int(out, 0x80, 7, k + 62)!;
                    return;
                }
                if name_at == 0 {
                    name_at = k + 62;
                }
            }
        }
        // a literal, indexed
        if name_at != 0 {
            put_int(out, 0x40, 6, name_at)!;
        } else {
            out.push(0x40)!;
            put_string(out, h.name)!;
        }
        put_string(out, h.value)!;
        self.table.add(h.name, h.value)!;
    }
}

pub struct Decoder {
    table: Table,
    allowed: usize,        // the largest table the encoder may ask for
    max_list_size: usize,  // names, values and 32 bytes each (0: no limit)
    tree: Tree,
    store: strs.String,
    spans: vec.Vec.[[4]usize],
    out: vec.Vec.[Header],

    /// A decoder whose peer may use a table of up to table_size bytes (what
    /// this side advertised in SETTINGS_HEADER_TABLE_SIZE).
    pub fn init(table_size: usize) Decoder {
        let tree = Tree.build() catch Tree.{ nodes: mem.alloc.[u16](0) catch @trap(), count: 0 };
        return Decoder.{ table: Table.init(table_size), allowed: table_size, max_list_size: 65536, tree: tree,
                         store: strs.String.init(), spans: vec.Vec.[[4]usize].init(), out: vec.Vec.[Header].init() };
    }

    pub fn deinit(self: *mut Decoder) void {
        self.table.deinit();
        mem.free(self.tree.nodes);
        self.store.deinit();
        self.spans.deinit();
        self.out.deinit();
    }

    /// Decodes a block; the headers stay valid until the next decode.
    pub fn decode(self: *mut Decoder, b: []u8) ![]Header {
        if self.tree.nodes.len == 0 {
            return error.OutOfMemory;
        }
        self.store.clear();
        self.spans.clear();
        self.out.clear();
        let mut i: usize = 0;
        let mut list_size: usize = 0;
        let mut first = true;
        while i < b.len {
            let c = b[i];
            if (c & 0xe0) == 0x20 {
                // a table size update: only at the start of a block
                if not first {
                    return error.DecodeError;
                }
                let n = get_int(b, &mut i, 5)!;
                if n > self.allowed {
                    return error.DecodeError;
                }
                self.table.resize(n);
                continue;
            }
            first = false;
            let mut name: []u8 = "";
            let mut value: []u8 = "";
            let mut index = false;
            let na = self.store.len;
            let mut nl: usize = 0;
            let mut va: usize = 0;
            let mut vl: usize = 0;
            if (c & 0x80) != 0 {
                let k = get_int(b, &mut i, 7)!;
                if not self.table.get(k, &mut name, &mut value) {
                    return error.DecodeError;
                }
                self.store.push_str(name)!;
                nl = name.len;
                va = self.store.len;
                self.store.push_str(value)!;
                vl = value.len;
            } else {
                let mut prefix: u32 = 4;  // without indexing (0x00) or never (0x10)
                if (c & 0x40) != 0 {
                    prefix = 6;
                    index = true;
                }
                let k = get_int(b, &mut i, prefix)!;
                if k == 0 {
                    self.read_string(b, &mut i)!;
                } else {
                    if not self.table.get(k, &mut name, &mut value) {
                        return error.DecodeError;
                    }
                    self.store.push_str(name)!;
                }
                nl = self.store.len - na;
                va = self.store.len;
                self.read_string(b, &mut i)!;
                vl = self.store.len - va;
            }
            list_size += nl + vl + 32;
            if (self.max_list_size != 0) and (list_size > self.max_list_size) {
                return error.ListTooLarge;
            }
            if index {
                let st = self.store.bytes();
                self.table.add(st[na..na + nl], st[va..va + vl])!;
            }
            self.spans.push([na, nl, va, vl])!;
        }
        let st = self.store.bytes();
        for sp in self.spans.items() {
            self.out.push(Header.{ name: st[sp[0]..sp[0] + sp[1]], value: st[sp[2]..sp[2] + sp[3]] })!;
        }
        return self.out.items();
    }

    fn read_string(self: *mut Decoder, b: []u8, i: *mut usize) !void {
        if i.* >= b.len {
            return error.DecodeError;
        }
        let huff = (b[i.*] & 0x80) != 0;
        let n = get_int(b, i, 7)!;
        if (i.* + n) > b.len {
            return error.DecodeError;
        }
        let s = b[i.*..i.* + n];
        i.* += n;
        if huff {
            decode_huffman(&self.tree, s, &mut self.store)!;
        } else {
            self.store.push_str(s)!;
        }
    }
}

// -----------------------------------------------------------------------------
// Tests
// -----------------------------------------------------------------------------

test "Python's hpack and the RFC's examples" {
    let data = fs.read_file("std/net/hpack/testdata/cases.txt") catch return;
    defer mem.free(data);
    let mut enc = Encoder.init();
    defer enc.deinit();
    let mut dec = Decoder.init(4096);
    defer dec.deinit();
    let mut lines = strs.lines(data);
    let mut blocks: usize = 0;
    let mut hs = vec.Vec.[Header].init();
    defer hs.deinit();
    let mut out = strs.String.init();
    defer out.deinit();
    while true {
        let line = lines.next() catch break;
        if strs.eq(line, "reset") {
            enc.deinit();
            enc = Encoder.init();
            dec.deinit();
            dec = Decoder.init(4096);
            continue;
        }
        if strs.starts_with(line, "size ") {
            let n = io_parse(line[5..]);
            enc.set_max_size(n)!;
            continue;
        }
        let mut f = strs.split(line, "\t");
        let hx = f.next()!;
        let want = hex.decode_alloc(hx)!;
        defer mem.free(want);
        hs.clear();
        while true {
            let h = f.next() catch break;
            let c = strs.index_of(h, ": ")!;
            hs.push(Header.{ name: h[..c], value: h[c + 2..] })!;
        }
        // decoding Python's block gives the list; encoding the list, the block
        let got = dec.decode(want)!;
        @expect(got.len == hs.len);
        for k in @as(usize, 0)..got.len {
            if not strs.eq(got[k].name, hs.items()[k].name) or not strs.eq(got[k].value, hs.items()[k].value) {
                @expect_eq(line, "");
            }
        }
        out.clear();
        enc.encode(hs.items(), &mut out)!;
        if not strs.eq(out.bytes(), want) {
            let mine = hex.encode_alloc(out.bytes())!;
            defer mem.free(mine);
            @expect_eq(mine, hx);
        }
        blocks += 1;
    }
    @expect(blocks == 102);
}

fn io_parse(s: []u8) usize {
    let mut v: usize = 0;
    for c in s {
        v = (v * 10) + @as(usize, c - '0');
    }
    return v;
}

fn bad(d: *mut Decoder, b: []u8) bool {
    _ = d.decode(b) catch |e| return (e == error.DecodeError) or (e == error.ListTooLarge);
    return false;
}

test "hostile blocks" {
    let mut d = Decoder.init(4096);
    defer d.deinit();
    // index 0, beyond the tables, an integer too long, a string past the
    // end, a size update above the limit or after a header, bad padding, EOS
    let cases: [8][]u8 = ["\x80", "\xff\x00", "\xff\xff\xff\xff\xff\xff\x7f", "\x40\x05abc", "\x3f\xe2\x1f", "\x82\x20",
                          "\x40\x81\x00\x01a", "\x40\x84\xff\xff\xff\xff\x01a"];
    for k in @as(usize, 0)..cases.len {
        if not bad(&mut d, cases[k]) {
            @expect_eq(k, 99);
        }
    }
    // a list larger than the limit
    d.max_list_size = 40;
    @expect(bad(&mut d, "\x40\x01a\x0aaaaaaaaaaa"));
}
