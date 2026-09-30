// =============================================================================
// std/encoding/cbor.ph - CBOR, the Concise Binary Object Representation (RFC 8949).
//
//     let mut s = strs.String.init();                   // writing
//     let mut w = cbor.Writer.init(s.writer());
//     w.map_header(2)!;
//     w.text("n")!; w.int(-7)!;
//     w.text("at")!; w.tag(1)!; w.uint(1363896240)!;    // a tagged epoch time
//
//     let mut doc = cbor.decode(s.bytes())!;            // reading
//     defer doc.deinit();
//     let n = doc.root.get("n")!.as_i64()!;
//     let d = cbor.diagnostic_alloc(doc.root)!;         // {"n": -7, "at": 1(1363896240)}
//
// Values cover the whole data model: unsigned and negative integers (all
// 65 bits of them), byte and text strings, arrays, maps with keys of any
// type, tags, simple values, booleans, null, undefined and floats (half,
// single and double). Writers use preferred serialization: the shortest
// argument for every head, and the shortest float that holds the value
// exactly. encode_canonical also sorts map keys (RFC 8949 section 4.2.1),
// for deterministic output. Indefinite-length items are read (and can be
// written with begin_array, begin_map and end).
//
// Decoding checks well-formedness strictly (reserved values, misplaced
// breaks, bad simple values, chunks of the wrong type) and that text
// strings are UTF-8; lengths beyond the input and nesting deeper than
// MAX_DEPTH are errors, and nothing recurses, so untrusted input is safe.
// =============================================================================

import "../collections/arena.ph" as arena;
import "../collections/vec.ph" as vec;
import "../fmt.ph" as fmt;
import "../io.ph" as io;
import "../mem.ph" as mem;
import "../os/fs.ph" as fs;
import "../str.ph" as strs;
import "base64.ph" as base64;
import "hex.ph" as hex;
import "json.ph" as json;

error { Truncated, Invalid, TooDeep, TrailingData, WrongType, NotFound, OutOfMemory }

/// Nesting deeper than this is refused (error.TooDeep).
pub const MAX_DEPTH: u32 = 512;

pub struct Pair {
    key: Value,
    value: Value,
}

pub struct Tagged {
    tag: u64,
    content: []Value,  // one value
}

pub union Value {
    Uint: u64,
    Neg: u64,  // the integer -1 - n
    Bytes: []u8,
    Text: []u8,
    Array: []Value,
    Map: []Pair,
    Tag: Tagged,
    Simple: u8,  // (other than false, true, null and undefined)
    Bool: bool,
    Null,
    Undefined,
    Float: f64,

    /// The entry with the text key `key` in a map.
    pub fn get(self: Value, key: []u8) !Value {
        match self {
            .Map(ps) => {
                for p in ps {
                    match p.key {
                        .Text(k) => {
                            if strs.eq(k, key) {
                                return p.value;
                            }
                        }
                        else => {}
                    }
                }
                return error.NotFound;
            }
            else => { return error.WrongType; }
        }
    }

    pub fn at(self: Value, i: usize) !Value {
        match self {
            .Array(xs) => {
                if i >= xs.len {
                    return error.NotFound;
                }
                return xs[i];
            }
            else => { return error.WrongType; }
        }
    }

    pub fn len(self: Value) usize {
        match self {
            .Array(xs) => { return xs.len; }
            .Map(ps)   => { return ps.len; }
            .Bytes(b)  => { return b.len; }
            .Text(t)   => { return t.len; }
            else       => { return 0; }
        }
    }

    pub fn as_i64(self: Value) !i64 {
        match self {
            .Uint(u) => {
                if (u >> 63) != 0 {
                    return error.WrongType;
                }
                return @bit_cast(i64, u);
            }
            .Neg(n) => {
                if (n >> 63) != 0 {
                    return error.WrongType;
                }
                return -1 - @bit_cast(i64, n);
            }
            else => { return error.WrongType; }
        }
    }

    pub fn as_u64(self: Value) !u64 {
        match self {
            .Uint(u) => { return u; }
            else     => { return error.WrongType; }
        }
    }

    /// Any number, as an f64.
    pub fn as_f64(self: Value) !f64 {
        match self {
            .Float(f) => { return f; }
            .Uint(u)  => { return @float_from_int(f64, u); }
            .Neg(n)   => { return -1.0 - @float_from_int(f64, n); }
            else      => { return error.WrongType; }
        }
    }

    pub fn as_bool(self: Value) !bool {
        match self {
            .Bool(b) => { return b; }
            else     => { return error.WrongType; }
        }
    }

    pub fn as_text(self: Value) ![]u8 {
        match self {
            .Text(t) => { return t; }
            else     => { return error.WrongType; }
        }
    }

    pub fn as_bytes(self: Value) ![]u8 {
        match self {
            .Bytes(b) => { return b; }
            else      => { return error.WrongType; }
        }
    }

    pub fn as_array(self: Value) ![]Value {
        match self {
            .Array(xs) => { return xs; }
            else       => { return error.WrongType; }
        }
    }

    pub fn as_map(self: Value) ![]Pair {
        match self {
            .Map(ps) => { return ps; }
            else     => { return error.WrongType; }
        }
    }

    /// A tagged value's content, if it has this tag.
    pub fn untag(self: Value, tag: u64) !Value {
        match self {
            .Tag(t) => {
                if t.tag == tag {
                    return t.content[0];
                }
                return error.WrongType;
            }
            else => { return error.WrongType; }
        }
    }
}

/// An integer value.
pub fn int(i: i64) Value {
    if i >= 0 {
        return Value.Uint(@bit_cast(u64, i));
    }
    return Value.Neg(@bit_cast(u64, -1 - i));
}

// -----------------------------------------------------------------------------
// Floats
// -----------------------------------------------------------------------------

/// Decodes an IEEE 754 half-precision float.
pub fn half_to_f64(h: u16) f64 {
    let sign = (h >> 15) != 0;
    let e = @as(i32, (h >> 10) & 31);
    let m = @as(f64, h & 1023);
    let mut v: f64 = 0.0;
    if e == 0 {
        v = m * 5.9604644775390625e-8;  // m * 2^-24
    } else if e == 31 {
        if m == 0.0 {
            v = 1.0 / 0.0;
        } else {
            v = 0.0 / 0.0;
        }
    } else {
        v = (1.0 + (m / 1024.0)) * pow2(e - 15);
    }
    if sign {
        return -v;
    }
    return v;
}

fn pow2(e: i32) f64 {
    let mut v: f64 = 1.0;
    if e >= 0 {
        for _k in @as(i32, 0)..e {
            v *= 2.0;
        }
    } else {
        for _k in e..0 {
            v /= 2.0;
        }
    }
    return v;
}

// The half-precision bits for x, if it holds x exactly (not NaN).
fn f64_to_half(x: f64, out: *mut u16) bool {
    let f = @float_cast(f32, x);
    if @as(f64, f) != x {
        return false;
    }
    let b = @bit_cast(u32, f);
    let sign = @truncate(u16, (b >> 16) & 0x8000);
    let e = @as(i32, @truncate(u8, b >> 23));
    let m = b & 0x7fffff;
    if e == 255 {
        if m != 0 {
            return false;
        }
        out.* = sign | 0x7c00;
        return true;
    }
    if (e == 0) and (m == 0) {
        out.* = sign;
        return true;
    }
    let ue = e - 127;
    if (ue >= -14) and (ue <= 15) {
        if (m & 0x1fff) != 0 {
            return false;
        }
        out.* = sign | @truncate(u16, @bit_cast(u32, ue + 15) << 10) | @truncate(u16, m >> 13);
        return true;
    }
    if (ue >= -24) and (ue < -14) {
        // subnormal: the value is (1.m) * 2^ue = frac * 2^-24
        let full = m | 0x800000;              // 24 bits: 1.m * 2^23
        let shift = @bit_cast(u32, -ue - 1);  // to count units of 2^-24: full >> (23 - (ue + 24))
        if (full & ((@as(u32, 1) << shift) - 1)) != 0 {
            return false;
        }
        out.* = sign | @truncate(u16, full >> shift);
        return true;
    }
    return false;
}

// -----------------------------------------------------------------------------
// Writing
// -----------------------------------------------------------------------------

/// Writes CBOR. After an array or map header come that many items (for a
/// map, key then value); after tag(), one item.
pub struct Writer {
    out: io.Writer,
    canonical: bool,  // sort map keys in value() (RFC 8949 section 4.2.1)

    pub fn init(out: io.Writer) Writer {
        return Writer.{ out: out, canonical: false };
    }

    fn head(self: *mut Writer, major: u8, v: u64) !void {
        let mut buf: [9]u8 = [0; 9];
        let mb = major << 5;
        let mut n: usize = 0;
        if v < 24 {
            buf[0] = mb | @truncate(u8, v);
        } else if v < 256 {
            buf[0] = mb | 24;
            n = 1;
        } else if v < 65536 {
            buf[0] = mb | 25;
            n = 2;
        } else if v < 4294967296 {
            buf[0] = mb | 26;
            n = 4;
        } else {
            buf[0] = mb | 27;
            n = 8;
        }
        for k in @as(usize, 0)..n {
            buf[1 + k] = @truncate(u8, v >> @as(u64, 8 * (n - 1 - k)));
        }
        self.out.write_all(buf[..n + 1])!;
    }

    pub fn uint(self: *mut Writer, v: u64) !void {
        return self.head(0, v);
    }

    pub fn int(self: *mut Writer, v: i64) !void {
        if v >= 0 {
            return self.head(0, @bit_cast(u64, v));
        }
        return self.head(1, @bit_cast(u64, -1 - v));
    }

    /// The integer -1 - n.
    pub fn neg(self: *mut Writer, n: u64) !void {
        return self.head(1, n);
    }

    pub fn bytes(self: *mut Writer, b: []u8) !void {
        self.head(2, @as(u64, b.len))!;
        self.out.write_all(b)!;
    }

    pub fn text(self: *mut Writer, t: []u8) !void {
        self.head(3, @as(u64, t.len))!;
        self.out.write_all(t)!;
    }

    pub fn array_header(self: *mut Writer, n: usize) !void {
        return self.head(4, @as(u64, n));
    }

    pub fn map_header(self: *mut Writer, n: usize) !void {
        return self.head(5, @as(u64, n));
    }

    /// An array of unknown length: items, then end().
    pub fn begin_array(self: *mut Writer) !void {
        self.out.write_all("\x9f")!;
    }

    pub fn begin_map(self: *mut Writer) !void {
        self.out.write_all("\xbf")!;
    }

    pub fn end(self: *mut Writer) !void {
        self.out.write_all("\xff")!;
    }

    pub fn tag(self: *mut Writer, t: u64) !void {
        return self.head(6, t);
    }

    pub fn simple(self: *mut Writer, v: u8) !void {
        if (v >= 24) and (v < 32) {
            return error.Invalid;
        }
        return self.head(7, @as(u64, v));
    }

    pub fn boolean(self: *mut Writer, b: bool) !void {
        if b {
            return self.out.write_all("\xf5");
        }
        return self.out.write_all("\xf4");
    }

    pub fn nil(self: *mut Writer) !void {
        return self.out.write_all("\xf6");
    }

    pub fn undefined(self: *mut Writer) !void {
        return self.out.write_all("\xf7");
    }

    /// A float, as the shortest of half, single and double precision that
    /// holds it exactly (NaN as the half-precision quiet NaN).
    pub fn float(self: *mut Writer, x: f64) !void {
        if x != x {
            return self.out.write_all("\xf9\x7e\x00");
        }
        let mut h: u16 = 0;
        if f64_to_half(x, &mut h) {
            let b: [3]u8 = [0xf9, @truncate(u8, h >> 8), @truncate(u8, h)];
            return self.out.write_all(b[..]);
        }
        let f = @float_cast(f32, x);
        if @as(f64, f) == x {
            let u = @bit_cast(u32, f);
            let b: [5]u8 = [
                0xfa, @truncate(u8, u >> 24), @truncate(u8, u >> 16), @truncate(u8, u >> 8), @truncate(u8, u)];
            return self.out.write_all(b[..]);
        }
        return self.float64(x);
    }

    pub fn float64(self: *mut Writer, x: f64) !void {
        let u = @bit_cast(u64, x);
        let mut b: [9]u8 = [0xfb; 9];
        for k in @as(usize, 0)..8 {
            b[1 + k] = @truncate(u8, u >> @as(u64, 56 - (8 * k)));
        }
        return self.out.write_all(b[..]);
    }

    /// Writes a whole value.
    pub fn value(self: *mut Writer, v: Value) !void {
        let mut work = vec.Vec.[Value].init();
        defer work.deinit();
        work.push(v)!;
        while work.len > 0 {
            let x = work.pop()!;
            match x {
                .Uint(u)  => { self.head(0, u)!; }
                .Neg(n)   => { self.head(1, n)!; }
                .Bytes(b) => { self.bytes(b)!; }
                .Text(t)  => { self.text(t)!; }
                .Array(xs) => {
                    self.array_header(xs.len)!;
                    let mut k = xs.len;
                    while k > 0 {
                        k -= 1;
                        work.push(xs[k])!;
                    }
                }
                .Map(ps) => {
                    if self.canonical {
                        self.sorted_map(ps)!;
                    } else {
                        self.map_header(ps.len)!;
                        let mut k = ps.len;
                        while k > 0 {
                            k -= 1;
                            work.push(ps[k].value)!;
                            work.push(ps[k].key)!;
                        }
                    }
                }
                .Tag(t) => {
                    self.tag(t.tag)!;
                    work.push(t.content[0])!;
                }
                .Simple(s) => { self.simple(s)!; }
                .Bool(b)   => { self.boolean(b)!; }
                .Null      => { self.nil()!; }
                .Undefined => { self.undefined()!; }
                .Float(f)  => { self.float(f)!; }
            }
        }
    }

    // A map with its entries in the order of their encoded keys.
    fn sorted_map(self: *mut Writer, ps: []Pair) !void {
        let mut buf = strs.String.init();
        defer buf.deinit();
        let mut ents = vec.Vec.[Entry].init();
        defer ents.deinit();
        let mut w = Writer.{ out: buf.writer(), canonical: true };
        for p in ps {
            let st = buf.len;
            w.value(p.key)!;
            let ke = buf.len;
            w.value(p.value)!;
            ents.push(Entry.{ start: st, key_end: ke, end: buf.len })!;
        }
        let b = buf.bytes();
        // insertion sort by key bytes (maps are usually small)
        let es = ents.items();
        for i in @as(usize, 1)..es.len {
            let e = es[i];
            let mut j = i;
            while (j > 0) and key_less(b, e, es[j - 1]) {
                es[j] = es[j - 1];
                j -= 1;
            }
            es[j] = e;
        }
        self.map_header(ps.len)!;
        for e in es {
            self.out.write_all(b[e.start..e.end])!;
        }
    }

    /// Writes a JSON value.
    pub fn json(self: *mut Writer, v: json.Value) !void {
        let mut work = vec.Vec.[json.Value].init();
        defer work.deinit();
        work.push(v)!;
        while work.len > 0 {
            let x = work.pop()!;
            match x {
                .Null    => { self.nil()!; }
                .Bool(b) => { self.boolean(b)!; }
                .Int(i)  => { self.int(i)!; }
                .Float(f) => {
                    if (f == @float_from_int(f64, @int_from_float(i64, f))) and (f > -9.0e18) and (f < 9.0e18) {
                        self.int(@int_from_float(i64, f))!;
                    } else {
                        self.float(f)!;
                    }
                }
                .String(s) => { self.text(s)!; }
                .Array(xs) => {
                    self.array_header(xs.len)!;
                    let mut k = xs.len;
                    while k > 0 {
                        k -= 1;
                        work.push(xs[k])!;
                    }
                }
                .Object(ms) => {
                    self.map_header(ms.len)!;
                    let mut k = ms.len;
                    while k > 0 {
                        k -= 1;
                        work.push(ms[k].value)!;
                        work.push(json.Value.String(ms[k].key))!;
                    }
                }
            }
        }
    }
}

struct Entry {
    start: usize,
    key_end: usize,
    end: usize,
}

fn key_less(b: []u8, x: Entry, y: Entry) bool {
    return strs.compare(b[x.start..x.key_end], b[y.start..y.key_end]) < 0;
}

/// The CBOR encoding of a value (preferred serialization); the caller frees
/// it.
pub fn encode(v: Value) ![]mut u8 {
    return encode_with(v, false);
}

/// The deterministic encoding (map keys sorted).
pub fn encode_canonical(v: Value) ![]mut u8 {
    return encode_with(v, true);
}

fn encode_with(v: Value, canonical: bool) ![]mut u8 {
    let mut s = strs.String.init();
    let mut w = Writer.init(s.writer());
    w.canonical = canonical;
    w.value(v) catch |e| {
        s.deinit();
        return e;
    };
    return s.take();
}

/// Converts JSON text to CBOR; the caller frees the result.
pub fn from_json_text(text: []u8) ![]mut u8 {
    let mut doc = json.parse(text)!;
    defer doc.deinit();
    let mut s = strs.String.init();
    let mut w = Writer.init(s.writer());
    w.json(doc.root) catch |e| {
        s.deinit();
        return e;
    };
    return s.take();
}

// -----------------------------------------------------------------------------
// Reading
// -----------------------------------------------------------------------------

/// A decoded value and the memory behind it.
pub struct Document {
    root: Value,
    arena: arena.Arena,

    pub fn deinit(self: *mut Document) void {
        self.arena.deinit();
        self.root = Value.Null;
    }
}

/// Decodes one data item that fills all of bytes.
pub fn decode(bytes: []u8) !Document {
    let mut used: usize = 0;
    let mut doc = decode_prefix(bytes, &mut used)!;
    if used != bytes.len {
        doc.deinit();
        return error.TrailingData;
    }
    return doc;
}

/// Decodes the data item at the start of bytes (for a sequence of items,
/// RFC 8742), and sets used to its length.
pub fn decode_prefix(bytes: []u8, used: *mut usize) !Document {
    let mut d = Decoder.{ s: bytes, i: 0, arena: arena.Arena.init() };
    let v = d.item_tree() catch |e| {
        d.arena.deinit();
        return e;
    };
    used.* = d.i;
    return Document.{ root: v, arena: d.arena };
}

const F_ARRAY: u8 = 0;
const F_MAP: u8 = 1;
const F_TAG: u8 = 2;

struct Frame {
    kind: u8,
    indefinite: bool,
    left: u64,     // items still to come (definite lengths; a map's keys and values both count)
    start: usize,  // where its items begin in the list of open items
    tag: u64,
}

// What one head was: a complete value, the start of a container, or a break.
const H_VALUE: u8 = 0;
const H_ARRAY: u8 = 1;
const H_MAP: u8 = 2;
const H_TAG: u8 = 3;
const H_BREAK: u8 = 4;

struct Decoder {
    s: []u8,
    i: usize,
    arena: arena.Arena,

    fn byte(self: *mut Decoder) !u8 {
        if self.i >= self.s.len {
            return error.Truncated;
        }
        let b = self.s[self.i];
        self.i += 1;
        return b;
    }

    fn take(self: *mut Decoder, n: u64) ![]u8 {
        if n > @as(u64, self.s.len - self.i) {
            return error.Truncated;
        }
        let k = @truncate(usize, n);
        let b = self.s[self.i..self.i + k];
        self.i += k;
        return b;
    }

    // The argument of a head with additional information ai (< 28).
    fn arg(self: *mut Decoder, ai: u8) !u64 {
        if ai < 24 {
            return @as(u64, ai);
        }
        let mut n: u64 = 1;
        if ai == 25 {
            n = 2;
        } else if ai == 26 {
            n = 4;
        } else if ai == 27 {
            n = 8;
        }
        let b = self.take(n)!;
        let mut v: u64 = 0;
        for c in b {
            v = (v << 8) | @as(u64, c);
        }
        return v;
    }

    // A string of major type 2 or 3; indefinite ones are joined.
    fn string(self: *mut Decoder, major: u8, ai: u8) ![]u8 {
        if ai != 31 {
            let n = self.arg(ai)!;
            let b = self.take(n)!;
            if (major == 3) and not strs.valid_utf8(b) {
                return error.Invalid;
            }
            return b;
        }
        let mut all = strs.String.init();
        defer all.deinit();
        loop {
            let h = self.byte()!;
            if h == 0xff {
                break;
            }
            if ((h >> 5) != major) or ((h & 31) >= 28) {
                return error.Invalid;  // chunks are definite strings of the same type
            }
            let b = self.take(self.arg(h & 31)!)!;
            if (major == 3) and not strs.valid_utf8(b) {
                return error.Invalid;
            }
            all.push_str(b)!;
        }
        return self.arena.dup(all.bytes());
    }

    // Reads one head: a value into v, or a container's header (its length
    // into n, or NONE for indefinite).
    fn head(self: *mut Decoder, v: *mut Value, n: *mut u64) !u8 {
        let b = self.byte()!;
        let major = b >> 5;
        let ai = b & 31;
        if (ai >= 28) and (ai <= 30) {
            return error.Invalid;
        }
        if major == 7 {
            if ai == 31 {
                return H_BREAK;
            }
            if ai < 20 {
                v.* = Value.Simple(ai);
            } else if ai == 20 {
                v.* = Value.Bool(false);
            } else if ai == 21 {
                v.* = Value.Bool(true);
            } else if ai == 22 {
                v.* = Value.Null;
            } else if ai == 23 {
                v.* = Value.Undefined;
            } else if ai == 24 {
                let s = self.byte()!;
                if s < 32 {
                    return error.Invalid;
                }
                v.* = Value.Simple(s);
            } else if ai == 25 {
                v.* = Value.Float(half_to_f64(@truncate(u16, self.arg(ai)!)));
            } else if ai == 26 {
                v.* = Value.Float(@as(f64, @bit_cast(f32, @truncate(u32, self.arg(ai)!))));
            } else {
                v.* = Value.Float(@bit_cast(f64, self.arg(ai)!));
            }
            return H_VALUE;
        }
        if (ai == 31) and ((major == 0) or (major == 1) or (major == 6)) {
            return error.Invalid;
        }
        if major == 0 {
            v.* = Value.Uint(self.arg(ai)!);
            return H_VALUE;
        }
        if major == 1 {
            v.* = Value.Neg(self.arg(ai)!);
            return H_VALUE;
        }
        if major == 2 {
            v.* = Value.Bytes(self.string(2, ai)!);
            return H_VALUE;
        }
        if major == 3 {
            v.* = Value.Text(self.string(3, ai)!);
            return H_VALUE;
        }
        if major == 6 {
            n.* = self.arg(ai)!;
            return H_TAG;
        }
        if ai == 31 {
            n.* = NONE64;
        } else {
            n.* = self.arg(ai)!;
        }
        if major == 4 {
            return H_ARRAY;
        }
        return H_MAP;
    }

    // One data item, with its nested items, read with an explicit stack.
    fn item_tree(self: *mut Decoder) !Value {
        let mut frames = vec.Vec.[Frame].init();
        defer frames.deinit();
        let mut vals = vec.Vec.[Value].init();
        defer vals.deinit();
        let mut pairs = vec.Vec.[Pair].init();
        defer pairs.deinit();
        loop {
            let mut v = Value.Null;
            let mut n: u64 = 0;
            let h = self.head(&mut v, &mut n)!;
            let mut done = h == H_VALUE;
            if h == H_BREAK {
                if (frames.len == 0) or not frames.get(frames.len - 1).indefinite {
                    return error.Invalid;
                }
                let top = frames.get(frames.len - 1);
                if (top.kind == F_MAP) and (((vals.len - top.start) % 2) != 0) {
                    return error.Invalid;  // a key without a value
                }
                v = self.close(top, &mut vals, &mut pairs)!;
                frames.truncate(frames.len - 1);
                done = true;
            } else if not done {
                let mut left = n;
                let indefinite = n == NONE64;
                if h == H_MAP and not indefinite {
                    if n > (@as(u64, self.s.len) / 2) {
                        return error.Truncated;
                    }
                    left = n * 2;
                }
                if h == H_TAG {
                    left = 1;
                }
                if not indefinite and (left > @as(u64, self.s.len - self.i)) {
                    return error.Truncated;  // (each item takes a byte at least)
                }
                if frames.len >= @as(usize, MAX_DEPTH) {
                    return error.TooDeep;
                }
                let mut kind = F_ARRAY;
                if h == H_MAP {
                    kind = F_MAP;
                } else if h == H_TAG {
                    kind = F_TAG;
                }
                let f = Frame.{ kind: kind, indefinite: indefinite, left: left, start: vals.len, tag: n };
                if not indefinite and (left == 0) {
                    v = self.close(f, &mut vals, &mut pairs)!;
                    done = true;
                } else {
                    frames.push(f)!;
                }
            }
            if not done {
                continue;
            }
            // v is complete: add it to the open containers, closing the full ones
            loop {
                if frames.len == 0 {
                    return v;
                }
                vals.push(v)!;
                let top = frames.get_ptr(frames.len - 1);
                if top.indefinite {
                    break;
                }
                top.left -= 1;
                if top.left > 0 {
                    break;
                }
                v = self.close(top.*, &mut vals, &mut pairs)!;
                frames.truncate(frames.len - 1);
            }
        }
    }

    // Builds a finished container from its items (removing them).
    fn close(self: *mut Decoder, f: Frame, vals: *mut vec.Vec.[Value], pairs: *mut vec.Vec.[Pair]) !Value {
        let items = vals.items()[f.start..];
        let mut v = Value.Null;
        if f.kind == F_MAP {
            pairs.clear();
            for k in @as(usize, 0)..items.len / 2 {
                pairs.push(Pair.{ key: items[2 * k], value: items[(2 * k) + 1] })!;
            }
            v = Value.Map(self.arena.copy.[Pair](pairs.items())!);
        } else if f.kind == F_TAG {
            v = Value.Tag(Tagged.{ tag: f.tag, content: self.arena.copy.[Value](items)! });
        } else {
            v = Value.Array(self.arena.copy.[Value](items)!);
        }
        vals.truncate(f.start);
        return v;
    }
}

const NONE64: u64 = 0xffffffffffffffff;

// -----------------------------------------------------------------------------
// Diagnostic notation and JSON
// -----------------------------------------------------------------------------

/// Writes a value in diagnostic notation (RFC 8949 section 8), for
/// example [1, "two", h'03', 1(1363896240), {"a": 2.5}].
pub fn diagnostic(out: io.Writer, v: Value) !void {
    let mut work = vec.Vec.[Item].init();
    defer work.deinit();
    work.push(Item.{ v: v, before: "", after: "" })!;
    while work.len > 0 {
        let it = work.pop()!;
        out.write_all(it.before)!;
        if it.after.len > 0 {
            // (a closing bracket: no value)
            out.write_all(it.after)!;
            continue;
        }
        match it.v {
            .Uint(u) => { fmt.write(out, "{}", [fmt.int(u)])!; }
            .Neg(n) => {
                if n == NONE64 {
                    out.write_all("-18446744073709551616")!;
                } else {
                    fmt.write(out, "-{}", [fmt.int(n + 1)])!;
                }
            }
            .Bytes(b) => {
                let h = hex.encode_alloc(b)!;
                defer mem.free(h);
                out.write_all("h'")!;
                out.write_all(h)!;
                out.write_all("'")!;
            }
            .Text(t) => { json.write_string(out, t)!; }
            .Array(xs) => {
                out.write_all("[")!;
                work.push(Item.{ v: Value.Null, before: "", after: "]" })!;
                let mut k = xs.len;
                while k > 0 {
                    k -= 1;
                    let mut b: []u8 = ", ";
                    if k == 0 {
                        b = "";
                    }
                    work.push(Item.{ v: xs[k], before: b, after: "" })!;
                }
            }
            .Map(ps) => {
                out.write_all("{")!;
                work.push(Item.{ v: Value.Null, before: "", after: "}" })!;
                let mut k = ps.len;
                while k > 0 {
                    k -= 1;
                    work.push(Item.{ v: ps[k].value, before: ": ", after: "" })!;
                    let mut b: []u8 = ", ";
                    if k == 0 {
                        b = "";
                    }
                    work.push(Item.{ v: ps[k].key, before: b, after: "" })!;
                }
            }
            .Tag(t) => {
                fmt.write(out, "{}(", [fmt.int(t.tag)])!;
                work.push(Item.{ v: Value.Null, before: "", after: ")" })!;
                work.push(Item.{ v: t.content[0], before: "", after: "" })!;
            }
            .Simple(s) => { fmt.write(out, "simple({})", [fmt.int(s)])!; }
            .Bool(b) => {
                if b {
                    out.write_all("true")!;
                } else {
                    out.write_all("false")!;
                }
            }
            .Null      => { out.write_all("null")!; }
            .Undefined => { out.write_all("undefined")!; }
            .Float(f)  => { diag_float(out, f)!; }
        }
    }
}

struct Item {
    v: Value,
    before: []u8,
    after: []u8,
}

fn diag_float(out: io.Writer, f: f64) !void {
    if f != f {
        return out.write_all("NaN");
    }
    if (f - f) != 0.0 {
        if f > 0.0 {
            return out.write_all("Infinity");
        }
        return out.write_all("-Infinity");
    }
    let mut buf = strs.String.init();
    defer buf.deinit();
    let m = @max(f, -f);
    if (m != 0.0) and ((m < 1.0e-4) or (m >= 1.0e16)) {
        fmt.write(buf.writer(), "{:e}", [fmt.float(f)])!;  // (as Python and JavaScript would)
    } else {
        fmt.write(buf.writer(), "{}", [fmt.float(f)])!;
    }
    out.write_all(buf.bytes())!;
    if not strs.contains(buf.bytes(), ".") and not strs.contains(buf.bytes(), "e") {
        out.write_all(".0")!;
    }
}

/// The diagnostic notation of a value; the caller frees it.
pub fn diagnostic_alloc(v: Value) ![]mut u8 {
    let mut s = strs.String.init();
    diagnostic(s.writer(), v) catch |e| {
        s.deinit();
        return e;
    };
    return s.take();
}

/// Writes a value as JSON (RFC 8949 section 6.1): byte strings as base64url,
/// tags as their content, undefined, simple values, NaN and the infinities
/// as null, and map keys that are not text as their diagnostic notation.
pub fn to_json(out: io.Writer, v: Value) !void {
    let mut work = vec.Vec.[Item].init();
    defer work.deinit();
    work.push(Item.{ v: v, before: "", after: "" })!;
    while work.len > 0 {
        let it = work.pop()!;
        out.write_all(it.before)!;
        if it.after.len > 0 {
            if strs.eq(it.after, "key") {
                match it.v {
                    .Text(t) => { json.write_string(out, t)!; }
                    else => {
                        let d = diagnostic_alloc(it.v)!;
                        defer mem.free(d);
                        json.write_string(out, d)!;
                    }
                }
            } else {
                out.write_all(it.after)!;
            }
            continue;
        }
        match it.v {
            .Uint(u) => { fmt.write(out, "{}", [fmt.int(u)])!; }
            .Neg(n) => {
                if n == NONE64 {
                    out.write_all("-18446744073709551616")!;
                } else {
                    fmt.write(out, "-{}", [fmt.int(n + 1)])!;
                }
            }
            .Bytes(b) => {
                let e = base64.encode_alloc(b, base64.URL)!;
                defer mem.free(e);
                json.write_string(out, e)!;
            }
            .Text(t) => { json.write_string(out, t)!; }
            .Array(xs) => {
                out.write_all("[")!;
                work.push(Item.{ v: Value.Null, before: "", after: "]" })!;
                let mut k = xs.len;
                while k > 0 {
                    k -= 1;
                    let mut b: []u8 = ",";
                    if k == 0 {
                        b = "";
                    }
                    work.push(Item.{ v: xs[k], before: b, after: "" })!;
                }
            }
            .Map(ps) => {
                out.write_all("{")!;
                work.push(Item.{ v: Value.Null, before: "", after: "}" })!;
                let mut k = ps.len;
                while k > 0 {
                    k -= 1;
                    work.push(Item.{ v: ps[k].value, before: ":", after: "" })!;
                    let mut b: []u8 = ",";
                    if k == 0 {
                        b = "";
                    }
                    work.push(Item.{ v: ps[k].key, before: b, after: "key" })!;
                }
            }
            .Tag(t) => { work.push(Item.{ v: t.content[0], before: "", after: "" })!; }
            .Bool(b) => {
                if b {
                    out.write_all("true")!;
                } else {
                    out.write_all("false")!;
                }
            }
            .Float(f) => {
                if (f != f) or ((f - f) != 0.0) {
                    out.write_all("null")!;
                } else {
                    fmt.write(out, "{}", [fmt.float(f)])!;
                }
            }
            else => { out.write_all("null")!; }
        }
    }
}

// -----------------------------------------------------------------------------
// Tests
// -----------------------------------------------------------------------------

test "RFC 8949 appendix A" {
    let data = fs.read_file("std/encoding/cbor/testdata/appendix_a.txt") catch return;
    defer mem.free(data);
    let mut lines = strs.lines(data);
    _ = lines.next()!;
    let mut n: usize = 0;
    while true {
        let line = lines.next() catch break;
        let mut parts = strs.split(line, "\t");
        let hx = parts.next()!;
        let rt = strs.eq(parts.next()!, "1");
        let want = parts.remainder();
        let bytes = hex.decode_alloc(hx)!;
        defer mem.free(bytes);
        if strs.eq(hx, "f818") {
            // simple(24) in two bytes: RFC 7049 allowed it, RFC 8949 makes it
            // not well-formed
            @expect(fails(decode(bytes)));
            n += 1;
            continue;
        }
        let mut doc = decode(bytes)!;
        defer doc.deinit();
        let got = diagnostic_alloc(doc.root)!;
        defer mem.free(got);
        if not strs.eq(got, want) {
            fmt.print("{}: got {} want {}\n", [fmt.str(hx), fmt.str(got), fmt.str(want)]) catch {};
        }
        @expect_eq(got, want);
        if rt {
            let again = encode(doc.root)!;
            defer mem.free(again);
            @expect_eq(again, bytes);
        }
        n += 1;
    }
    @expect_eq(n, 82);
}

test "writing" {
    let mut s = strs.String.init();
    defer s.deinit();
    let mut w = Writer.init(s.writer());
    w.map_header(3)!;
    w.text("n")!;
    w.int(-7)!;
    w.int(10)!;
    w.begin_array()!;
    w.float(0.5)!;
    w.float(1.0e-7)!;
    w.bytes("\x01")!;
    w.end()!;
    w.text("at")!;
    w.tag(1)!;
    w.uint(1363896240)!;
    let mut doc = decode(s.bytes())!;
    defer doc.deinit();
    let d = diagnostic_alloc(doc.root)!;
    defer mem.free(d);
    @expect_eq(d, "{\"n\": -7, 10: [0.5, 1e-7, h'01'], \"at\": 1(1363896240)}");
    @expect_eq(doc.root.get("n")!.as_i64()!, -7);
    @expect_eq(doc.root.get("at")!.untag(1)!.as_u64()!, 1363896240);
    // the canonical form sorts keys by their encodings: 10 < "n" < "at"
    let c = encode_canonical(doc.root)!;
    defer mem.free(c);
    let mut d2 = decode(c)!;
    defer d2.deinit();
    let cd = diagnostic_alloc(d2.root)!;
    defer mem.free(cd);
    @expect_eq(cd, "{10: [0.5, 1e-7, h'01'], \"n\": -7, \"at\": 1(1363896240)}");
    let mut j = strs.String.init();
    defer j.deinit();
    to_json(j.writer(), d2.root)!;
    @expect_eq(j.bytes(), "{\"10\":[0.5,1e-7,\"AQ\"],\"n\":-7,\"at\":1363896240}");
    // half floats: every exact one is chosen
    let halves: [6]f64 = [65504.0, -2.0, 0.00006103515625, 5.960464477539063e-8, 1.0 / 0.0, 3.0e-8];
    let sizes: [6]usize = [3, 3, 3, 3, 3, 9];
    for k in @as(usize, 0)..6 {
        let e = encode(Value.Float(halves[k]))!;
        defer mem.free(e);
        @expect_eq(e.len, sizes[k]);
        let mut back = decode(e)!;
        defer back.deinit();
        @expect_eq(back.root.as_f64()!, halves[k]);
    }
    let j2 = from_json_text("[1, -2, 2.5, \"x\", {\"k\": null}]")!;
    defer mem.free(j2);
    let mut d3 = decode(j2)!;
    defer d3.deinit();
    let dd = diagnostic_alloc(d3.root)!;
    defer mem.free(dd);
    @expect_eq(dd, "[1, -2, 2.5, \"x\", {\"k\": null}]");
}

fn fails(r: !Document) bool {
    let mut d = r catch return true;
    d.deinit();
    return false;
}

test "malformed input" {
    let bad: [14][]u8 = [
        "",
        "\x1c",                  // reserved additional information
        "\xff",                  // a break outside an indefinite item
        "\x1f",                  // an indefinite integer
        "\xf8\x18",              // a two-byte simple value below 32
        "\x5f\x61\x61\xff",      // a text chunk in a byte string
        "\x62\xc3\x28",          // not UTF-8
        "\x9a\xff\xff\xff\xff",  // an array longer than the input
        "\xbf\x01\xff",          // a key without a value
        "\x82\x01",              // truncated
        "\xc6",                  // a tag without content
        "\x01\x02",              // trailing data
        "\x5f\x41\x00",          // an unfinished indefinite string
        "\x9f\x01",              // an unfinished indefinite array
    ];
    for b in bad {
        if not fails(decode(b)) {
            fmt.print("accepted {}\n", [fmt.int(b.len)]) catch {};
        }
        @expect(fails(decode(b)));
    }
    let mut s = strs.String.init();
    defer s.deinit();
    for _k in @as(usize, 0)..100000 {
        s.push(0x81)!;
    }
    s.push(0x00)!;
    @expect(fails(decode(s.bytes())));
    s.clear();
    for _k in @as(usize, 0)..100000 {
        s.push(0xc6)!;  // tags nest too
    }
    s.push(0x00)!;
    @expect(fails(decode(s.bytes())));
    let mut used: usize = 0;
    let mut d = decode_prefix("\x01\x02", &mut used)!;
    d.deinit();
    @expect_eq(used, 1);
}
