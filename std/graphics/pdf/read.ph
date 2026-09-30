// =============================================================================
// std/graphics/pdf/read.ph - Reading PDF files: objects, streams, pages and text.
//
//     let data = fs.read_file("paper.pdf")!;
//     defer mem.free(data);
//     let mut r = read.Reader.open(data)!;
//     defer r.deinit();
//     for i in 0..r.page_count() {
//         let t = r.page_text(i)!;     // UTF-8, lines in reading order
//         defer mem.free(t);
//     }
//     let title = r.info("Title")!;   // metadata, as UTF-8
//
// The reader parses the cross-reference tables and streams (following
// updates with /Prev, and object streams), and rebuilds the table by
// scanning the file when it is missing or broken. Objects are parsed on
// demand (object(), resolve()); stream data is decoded through FlateDecode
// (with PNG and TIFF predictors), LZWDecode, ASCIIHexDecode, ASCII85Decode
// and RunLengthDecode. Text comes from interpreting the pages' content
// streams (and the forms they use), decoding strings through each font's
// ToUnicode map or its encoding (WinAnsi, MacRoman, Standard, differences
// by glyph name), and ordering it into lines and words by position.
//
// Untrusted files: nothing here trusts the input. Offsets and lengths are
// checked, nesting and reference chains are limited, the page tree and
// form XObjects are walked without recursion into cycles, decoded streams
// are capped (Reader.max_stream), and the work any one file can cause is
// bounded. Encrypted files are refused (error.Encrypted).
// =============================================================================

import "../../collections/map.ph" as maps;
import "../../collections/vec.ph" as vec;
import "../../compress/deflate.ph" as deflate;
import "../../hash.ph" as hash;
import "../../mem.ph" as mem;
import "../../str.ph" as strs;
import "tables.ph" as tb;

error { Malformed, Encrypted, NotFound, WrongType, TooLarge, Unsupported, OutOfMemory }

/// Arrays and dictionaries nested deeper than this are malformed.
pub const MAX_DEPTH: u32 = 64;

pub struct Ref {
    num: u32,
    gen: u32,
}

pub struct Dict {
    keys: [][]u8,
    vals: []Object,

    /// The value for a key (Null if absent; not resolved).
    pub fn get(self: Dict, key: []u8) Object {
        for i in 0..self.keys.len {
            if strs.eq(self.keys[i], key) {
                return self.vals[i];
            }
        }
        return Object.Null;
    }
}

pub struct Stream {
    dict: Dict,
    start: usize,  // the raw data in the file
    len: usize,
}

pub union Object {
    Null,
    Bool: bool,
    Int: i64,
    Real: f64,
    Str: []u8,
    Name: []u8,
    Array: []Object,
    Dict: Dict,
    Stream: Stream,
    Ref: Ref,

    pub fn is_null(self: Object) bool {
        match self {
            .Null => { return true; }
            else  => { return false; }
        }
    }

    pub fn as_int(self: Object) !i64 {
        match self {
            .Int(v) => { return v; }
            .Real(v) => {
                if (v > -9.0e18) and (v < 9.0e18) {
                    return @int_from_float(i64, v);
                }
                return error.WrongType;
            }
            else => { return error.WrongType; }
        }
    }

    pub fn as_num(self: Object) !f64 {
        match self {
            .Int(v)  => { return @float_from_int(f64, v); }
            .Real(v) => { return v; }
            else     => { return error.WrongType; }
        }
    }

    pub fn as_name(self: Object) ![]u8 {
        match self {
            .Name(v) => { return v; }
            else     => { return error.WrongType; }
        }
    }

    pub fn as_str(self: Object) ![]u8 {
        match self {
            .Str(v) => { return v; }
            else    => { return error.WrongType; }
        }
    }

    pub fn as_array(self: Object) ![]Object {
        match self {
            .Array(v) => { return v; }
            else      => { return error.WrongType; }
        }
    }

    /// The dictionary of a Dict or a Stream.
    pub fn as_dict(self: Object) !Dict {
        match self {
            .Dict(v)   => { return v; }
            .Stream(s) => { return s.dict; }
            else       => { return error.WrongType; }
        }
    }

    pub fn as_stream(self: Object) !Stream {
        match self {
            .Stream(s) => { return s; }
            else       => { return error.WrongType; }
        }
    }
}

fn no_objects() []Object {
    return @slice_from_ptr(@ptr_from_int(*Object, 8), 0);
}

fn empty_dict() Dict {
    let k: [][]u8 = @slice_from_ptr(@ptr_from_int(*[]u8, 8), 0);
    let v: []Object = @slice_from_ptr(@ptr_from_int(*Object, 8), 0);
    return Dict.{ keys: k, vals: v };
}

// -----------------------------------------------------------------------------
// Characters and the lexer
// -----------------------------------------------------------------------------

fn is_white(c: u8) bool {
    return (c == 0) or (c == 9) or (c == 10) or (c == 12) or (c == 13) or (c == 32);
}

fn is_delim(c: u8) bool {
    return (c == '(') or (c == ')') or (c == '<') or (c == '>') or (c == '[') or (c == ']') or (c == '{') or
           (c == '}') or (c == '/') or (c == '%');
}

fn is_regular(c: u8) bool {
    return not is_white(c) and not is_delim(c);
}

fn hexval(c: u8) i32 {
    if (c >= '0') and (c <= '9') {
        return @as(i32, c - '0');
    }
    if (c >= 'a') and (c <= 'f') {
        return @as(i32, c - 'a') + 10;
    }
    if (c >= 'A') and (c <= 'F') {
        return @as(i32, c - 'A') + 10;
    }
    return -1;
}

fn at(s: []u8, i: usize) u8 {
    if i < s.len {
        return s[i];
    }
    return 0;
}

fn skip_white(s: []u8, p: usize) usize {
    let mut i = p;
    while i < s.len {
        let c = s[i];
        if c == '%' {
            while (i < s.len) and (s[i] != '\n') and (s[i] != '\r') {
                i += 1;
            }
        } else if is_white(c) {
            i += 1;
        } else {
            break;
        }
    }
    return i;
}

/// A number at s[p..]: its value and whether it is an integer; the end, or p if none.
fn scan_number(s: []u8, p: usize, iv: *mut i64, fv: *mut f64, is_int: *mut bool) usize {
    let mut i = p;
    let mut neg = false;
    if (at(s, i) == '+') or (at(s, i) == '-') {
        neg = at(s, i) == '-';
        i += 1;
    }
    let mut v: i64 = 0;
    let mut f: f64 = 0.0;
    let mut digits: usize = 0;
    while (i < s.len) and (s[i] >= '0') and (s[i] <= '9') {
        let d = @as(i64, s[i] - '0');
        if v < 100000000000000000 {
            v = (v * 10) + d;
        }
        f = (f * 10.0) + @float_from_int(f64, d);
        i += 1;
        digits += 1;
    }
    let mut integer = true;
    if at(s, i) == '.' {
        integer = false;
        i += 1;
        let mut scale: f64 = 0.1;
        while (i < s.len) and (s[i] >= '0') and (s[i] <= '9') {
            f += @float_from_int(f64, s[i] - '0') * scale;
            scale *= 0.1;
            i += 1;
            digits += 1;
        }
    }
    if digits == 0 {
        return p;
    }
    if neg {
        v = -v;
        f = -f;
    }
    iv.* = v;
    fv.* = f;
    is_int.* = integer;
    return i;
}

// -----------------------------------------------------------------------------
// The reader
// -----------------------------------------------------------------------------

struct Entry {
    kind: u8,    // 0 free, 1 at an offset, 2 in an object stream
    off: u64,    // the offset, or the object stream's number
    index: u32,  // the index in the object stream
}

pub struct Reader {
    data: []u8,
    xref: vec.Vec.[Entry],
    trailer: Dict,
    cache: vec.Vec.[Object],
    state: vec.Vec.[u8],        // per object: 0 not parsed, 1 being parsed, 2 cached
    bytes: vec.Vec.[[]mut u8],  // memory the objects point into
    objects: vec.Vec.[[]mut Object],
    names: vec.Vec.[[]mut []u8],
    objstm: maps.Map.[u64, usize],  // object stream number -> index in `streams`
    streams: vec.Vec.[ObjStm],
    pages: vec.Vec.[Dict],
    /// The most a stream may decode to (bytes).
    max_stream: usize,
    /// Work left (bytes decoded and parsed); a file cannot use more.
    budget: usize,

    /// Opens a PDF held in memory (which must outlive the Reader).
    pub fn open(data: []u8) !Reader {
        let mut r = Reader.{
            data: data,
            xref: vec.Vec.[Entry].init(),
            trailer: empty_dict(),
            cache: vec.Vec.[Object].init(),
            state: vec.Vec.[u8].init(),
            bytes: vec.Vec.[[]mut u8].init(),
            objects: vec.Vec.[[]mut Object].init(),
            names: vec.Vec.[[]mut []u8].init(),
            objstm: maps.Map.[u64, usize].init(hash.hash_u64, hash.eq_u64),
            streams: vec.Vec.[ObjStm].init(),
            pages: vec.Vec.[Dict].init(),
            max_stream: 256 * 1024 * 1024,
            budget: (@min(data.len, 1 << 24) * 64) + (1 << 28),
        };
        r.load() catch |e| {
            r.deinit();
            return e;
        };
        return r;
    }

    pub fn deinit(self: *mut Reader) void {
        self.xref.deinit();
        self.cache.deinit();
        self.state.deinit();
        for b in self.bytes.items() {
            mem.free(b);
        }
        self.bytes.deinit();
        for o in self.objects.items() {
            mem.free(o);
        }
        self.objects.deinit();
        for n in self.names.items() {
            mem.free(n);
        }
        self.names.deinit();
        self.objstm.deinit();
        for o in self.streams.items() {
            let mut offs = o.offsets;
            offs.deinit();
        }
        self.streams.deinit();
        self.pages.deinit();
    }

    fn keep(self: *mut Reader, b: []mut u8) ![]u8 {
        self.bytes.push(b) catch |e| {
            mem.free(b);
            return e;
        };
        return b;
    }

    fn load(self: *mut Reader) !void {
        if not strs.contains(self.data[..@min(self.data.len, 1024)], "%PDF-") {
            return error.Malformed;
        }
        let ok = self.read_xref_chain() catch false;
        if not ok or (self.trailer.get("Root").is_null()) {
            self.reconstruct()!;
        }
        if not self.trailer.get("Encrypt").is_null() {
            return error.Encrypted;
        }
        self.load_pages() catch |e| {
            if e == error.OutOfMemory {
                return e;
            }
            // a damaged page tree: try again from a rebuilt table
            self.pages.clear();
            self.reconstruct()!;
            self.load_pages()!;
        };
    }

    fn set_entry(self: *mut Reader, num: usize, e: Entry) !void {
        if num >= (1 << 23) {
            return;
        }
        while self.xref.len <= num {
            self.xref.push(Entry.{ kind: 0, off: 0, index: 0 })!;
            self.cache.push(Object.Null)!;
            self.state.push(0)!;
        }
        self.xref.set(num, e);
    }

    /// Sets an entry unless a newer section already did.
    fn add_entry(self: *mut Reader, num: usize, e: Entry, seen: *mut vec.Vec.[u8]) !void {
        if num >= (1 << 23) {
            return;
        }
        while seen.len <= num {
            seen.push(0)!;
        }
        if seen.get(num) != 0 {
            return;
        }
        seen.set(num, 1);
        self.set_entry(num, e)!;
    }

    /// Follows startxref and the /Prev chain; false if the file has no usable table.
    fn read_xref_chain(self: *mut Reader) !bool {
        let d = self.data;
        let tail = d.len - @min(d.len, 2048);
        let sx = strs.last_index_of(d[tail..], "startxref") catch return false;
        let mut iv: i64 = 0;
        let mut fv: f64 = 0.0;
        let mut isint = false;
        let p = skip_white(d, tail + sx + 9);
        if scan_number(d, p, &mut iv, &mut fv, &mut isint) == p {
            return false;
        }
        let mut seen = vec.Vec.[u8].init();
        defer seen.deinit();
        let mut visited = vec.Vec.[i64].init();
        defer visited.deinit();
        let mut off = iv;
        let mut first = true;
        while (off >= 0) and (@as(u64, @bit_cast(u64, off)) < @as(u64, d.len)) and (visited.len < 256) {
            for v in visited.items() {
                if v == off {
                    return not first;
                }
            }
            visited.push(off)!;
            let o = @truncate(usize, @bit_cast(u64, off));
            let mut trailer = empty_dict();
            let q = skip_white(d, o);
            if strs.starts_with(d[q..], "xref") {
                trailer = self.xref_table(q + 4, &mut seen)!;
                // a hybrid file's cross-reference stream
                let xs = trailer.get("XRefStm").as_int() catch -1;
                if (xs > 0) and (xs < @as(i64, @bit_cast(i64, @as(u64, d.len)))) {
                    _ = self.xref_stream(@truncate(usize, @bit_cast(u64, xs)), &mut seen) catch empty_dict();
                }
            } else {
                trailer = self.xref_stream(q, &mut seen)!;
            }
            if first {
                self.trailer = trailer;
                first = false;
            }
            off = trailer.get("Prev").as_int() catch -1;
        }
        return not first;
    }

    fn xref_table(self: *mut Reader, p: usize, seen: *mut vec.Vec.[u8]) !Dict {
        let d = self.data;
        let mut i = p;
        while true {
            i = skip_white(d, i);
            if strs.starts_with(d[i..], "trailer") {
                let mut lx = Lexer.{ s: d, pos: i + 7 };
                let t = self.parse(&mut lx, 0)!;
                return t.as_dict();
            }
            let mut start: i64 = 0;
            let mut count: i64 = 0;
            let mut fv: f64 = 0.0;
            let mut isint = false;
            let a = scan_number(d, i, &mut start, &mut fv, &mut isint);
            if a == i {
                return error.Malformed;
            }
            let b0 = skip_white(d, a);
            let b = scan_number(d, b0, &mut count, &mut fv, &mut isint);
            if (b == b0) or (start < 0) or (count < 0) or (count > (1 << 23)) {
                return error.Malformed;
            }
            i = b;
            for k in 0..@truncate(usize, @bit_cast(u64, count)) {
                i = skip_white(d, i);
                let mut off: i64 = 0;
                let mut gen: i64 = 0;
                let e1 = scan_number(d, i, &mut off, &mut fv, &mut isint);
                let g0 = skip_white(d, e1);
                let e2 = scan_number(d, g0, &mut gen, &mut fv, &mut isint);
                let t0 = skip_white(d, e2);
                if (e1 == i) or (e2 == g0) or (t0 >= d.len) {
                    return error.Malformed;
                }
                let kind = d[t0];
                i = t0 + 1;
                let num = @truncate(usize, @bit_cast(u64, start)) + k;
                if (kind == 'n') and (off > 0) {
                    self.add_entry(num, Entry.{ kind: 1, off: @bit_cast(u64, off), index: 0 }, seen)!;
                } else {
                    self.add_entry(num, Entry.{ kind: 0, off: 0, index: 0 }, seen)!;
                }
            }
        }
        return error.Malformed;
    }

    fn xref_stream(self: *mut Reader, p: usize, seen: *mut vec.Vec.[u8]) !Dict {
        let mut lx = Lexer.{ s: self.data, pos: p };
        let obj = self.parse_indirect(&mut lx)!;
        let st = obj.as_stream()!;
        let data = self.decode(st)!;
        let w = st.dict.get("W").as_array()!;
        if w.len < 3 {
            return error.Malformed;
        }
        let mut ws: [3]usize = [0, 0, 0];
        for k in @as(usize, 0)..3 {
            let v = w[k].as_int()!;
            if (v < 0) or (v > 8) {
                return error.Malformed;
            }
            ws[k] = @truncate(usize, @bit_cast(u64, v));
        }
        let row = ws[0] + ws[1] + ws[2];
        if row == 0 {
            return error.Malformed;
        }
        let size = st.dict.get("Size").as_int() catch 0;
        let idx = st.dict.get("Index");
        let mut ranges = vec.Vec.[i64].init();
        defer ranges.deinit();
        match idx {
            .Array(a) => {
                for v in a {
                    ranges.push(v.as_int()!)!;
                }
            }
            else => {
                ranges.push(0)!;
                ranges.push(size)!;
            }
        }
        let mut pos: usize = 0;
        let mut k: usize = 0;
        while k + 1 < ranges.len {
            let start = ranges.get(k);
            let count = ranges.get(k + 1);
            if (start < 0) or (count < 0) or (count > (1 << 23)) {
                return error.Malformed;
            }
            for j in 0..@truncate(usize, @bit_cast(u64, count)) {
                if pos + row > data.len {
                    return st.dict;
                }
                let mut f: [3]u64 = [1, 0, 0];
                let mut q = pos;
                for c in @as(usize, 0)..3 {
                    if ws[c] > 0 {
                        let mut v: u64 = 0;
                        for _b in 0..ws[c] {
                            v = (v << 8) | @as(u64, data[q]);
                            q += 1;
                        }
                        f[c] = v;
                    }
                }
                pos += row;
                let num = @truncate(usize, @bit_cast(u64, start)) + j;
                if f[0] == 1 {
                    self.add_entry(num, Entry.{ kind: 1, off: f[1], index: 0 }, seen)!;
                } else if f[0] == 2 {
                    self.add_entry(num, Entry.{ kind: 2, off: f[1], index: @truncate(u32, f[2]) }, seen)!;
                } else {
                    self.add_entry(num, Entry.{ kind: 0, off: 0, index: 0 }, seen)!;
                }
            }
            k += 2;
        }
        return st.dict;
    }

    /// Rebuilds the table from "n g obj" markers, and the trailer from the
    /// last trailer dictionary or the catalog.
    fn reconstruct(self: *mut Reader) !void {
        let d = self.data;
        self.xref.clear();
        self.cache.clear();
        self.state.clear();
        let mut i: usize = 0;
        let mut last_trailer: usize = 0;
        while i + 3 < d.len {
            let o = strs.index_of_from(d, "obj", i) catch break;
            i = o + 3;
            if strs.starts_with(d[o..], "obj") and (o >= 4) {
                // walk back over "n g "
                let mut j = o;
                while (j > 0) and is_white(d[j - 1]) {
                    j -= 1;
                }
                let ge = j;
                while (j > 0) and (d[j - 1] >= '0') and (d[j - 1] <= '9') {
                    j -= 1;
                }
                if (j == ge) or (j == 0) or not is_white(d[j - 1]) {
                    continue;
                }
                while (j > 0) and is_white(d[j - 1]) {
                    j -= 1;
                }
                let ne = j;
                while (j > 0) and (d[j - 1] >= '0') and (d[j - 1] <= '9') {
                    j -= 1;
                }
                if (j == ne) or ((j > 0) and is_regular(d[j - 1])) {
                    continue;
                }
                let mut num: i64 = 0;
                let mut fv: f64 = 0.0;
                let mut isint = false;
                _ = scan_number(d, j, &mut num, &mut fv, &mut isint);
                if (num > 0) and (num < (1 << 23)) {
                    self.set_entry(
                        @truncate(usize, @bit_cast(u64, num)), Entry.{ kind: 1, off: @as(u64, j), index: 0 })!;
                }
            }
        }
        let mut k: usize = 0;
        while true {
            let t = strs.index_of_from(d, "trailer", k) catch break;
            last_trailer = t + 7;
            k = t + 7;
        }
        if last_trailer > 0 {
            let mut lx = Lexer.{ s: d, pos: last_trailer };
            let t = self.parse(&mut lx, 0) catch Object.Null;
            self.trailer = t.as_dict() catch empty_dict();
        }
        if self.trailer.get("Root").is_null() {
            // find the catalog
            for num in 1..self.xref.len {
                let o = self.object(@truncate(u32, num)) catch continue;
                let dd = o.as_dict() catch continue;
                let ty = dd.get("Type").as_name() catch continue;
                if strs.eq(ty, "Catalog") {
                    let keys = mem.alloc.[[]u8](1)!;
                    let vals = mem.alloc.[Object](1) catch |e| {
                        mem.free(keys);
                        return e;
                    };
                    keys[0] = "Root";
                    vals[0] = Object.Ref(Ref.{ num: @truncate(u32, num), gen: 0 });
                    self.names.push(keys)!;
                    self.objects.push(vals)!;
                    self.trailer = Dict.{ keys: keys, vals: vals };
                    break;
                }
            }
        }
        if self.trailer.get("Root").is_null() {
            return error.Malformed;
        }
    }

    // --- objects ---

    /// Indirect object `num` (Null if it does not exist).
    pub fn object(self: *mut Reader, num: u32) !Object {
        let n = @as(usize, num);
        if n >= self.xref.len {
            return Object.Null;
        }
        let st = self.state.get(n);
        if st == 2 {
            return self.cache.get(n);
        }
        if st == 1 {
            return Object.Null;  // a cycle
        }
        self.state.set(n, 1);
        let e = self.xref.get(n);
        let mut o = Object.Null;
        if (e.kind == 1) and (e.off < @as(u64, self.data.len)) {
            let mut lx = Lexer.{ s: self.data, pos: @truncate(usize, e.off) };
            o = self.parse_indirect(&mut lx) catch |err| {
                self.state.set(n, 0);
                if err == error.OutOfMemory {
                    return err;
                }
                return Object.Null;
            };
        } else if e.kind == 2 {
            o = self.from_objstm(e.off, e.index) catch |err| {
                self.state.set(n, 0);
                if err == error.OutOfMemory {
                    return err;
                }
                return Object.Null;
            };
        }
        self.cache.set(n, o);
        self.state.set(n, 2);
        return o;
    }

    /// Follows references (a few levels at most) to a direct object.
    pub fn resolve(self: *mut Reader, o: Object) !Object {
        let mut cur = o;
        for _k in @as(usize, 0)..16 {
            match cur {
                .Ref(r) => { cur = self.object(r.num)!; }
                else    => { return cur; }
            }
        }
        return Object.Null;
    }

    /// A dictionary entry, resolved.
    pub fn get(self: *mut Reader, d: Dict, key: []u8) !Object {
        return self.resolve(d.get(key));
    }

    fn from_objstm(self: *mut Reader, stm: u64, index: u32) !Object {
        let mut oi: usize = 0;
        if self.objstm.contains(stm) {
            oi = self.objstm.get(stm)!;
        } else {
            if stm >= (1 << 23) {
                return error.Malformed;
            }
            let so = self.object(@truncate(u32, stm))!;
            let st = so.as_stream()!;
            let data = self.decode(st)!;
            let n = st.dict.get("N").as_int()!;
            let first = st.dict.get("First").as_int()!;
            if (n < 0) or (first < 0) or (n > @as(i64, @bit_cast(i64, @as(u64, data.len)))) {
                return error.Malformed;
            }
            // the header: pairs of object number and offset
            let mut offs = vec.Vec.[u64].init();
            let mut lx = Lexer.{ s: data, pos: 0 };
            for _k in 0..@truncate(usize, @bit_cast(u64, n)) {
                let a = self.parse(&mut lx, 0) catch break;
                let b = self.parse(&mut lx, 0) catch break;
                _ = a;
                let off = b.as_int() catch -1;
                let abs = first + off;
                if (off < 0) or (abs < 0) or (abs >= @as(i64, @bit_cast(i64, @as(u64, data.len)))) {
                    offs.push(0xffffffffffffffff) catch |e| {
                        offs.deinit();
                        return e;
                    };
                } else {
                    offs.push(@bit_cast(u64, abs)) catch |e| {
                        offs.deinit();
                        return e;
                    };
                }
            }
            self.streams.push(ObjStm.{ data: data, offsets: offs }) catch |e| {
                offs.deinit();
                return e;
            };
            oi = self.streams.len - 1;
            self.objstm.put(stm, oi)!;
        }
        let os = self.streams.get(oi);
        if @as(usize, index) >= os.offsets.len {
            return error.Malformed;
        }
        let at_ = os.offsets.get(@as(usize, index));
        if at_ >= @as(u64, os.data.len) {
            return error.Malformed;
        }
        let mut lx2 = Lexer.{ s: os.data, pos: @truncate(usize, at_) };
        return self.parse(&mut lx2, 0);
    }

    /// "n g obj ... endobj" (with stream data).
    fn parse_indirect(self: *mut Reader, lx: *mut Lexer) !Object {
        let mut iv: i64 = 0;
        let mut fv: f64 = 0.0;
        let mut isint = false;
        lx.pos = skip_white(lx.s, lx.pos);
        let a = scan_number(lx.s, lx.pos, &mut iv, &mut fv, &mut isint);
        if a == lx.pos {
            return error.Malformed;
        }
        let b0 = skip_white(lx.s, a);
        let b = scan_number(lx.s, b0, &mut iv, &mut fv, &mut isint);
        if b == b0 {
            return error.Malformed;
        }
        lx.pos = skip_white(lx.s, b);
        if not strs.starts_with(lx.s[lx.pos..], "obj") {
            return error.Malformed;
        }
        lx.pos += 3;
        let o = self.parse(lx, 0)!;
        // a stream?
        let p = skip_white(lx.s, lx.pos);
        match o {
            .Dict(dict) => {
                if strs.starts_with(lx.s[p..], "stream") {
                    let mut s = p + 6;
                    if at(lx.s, s) == '\r' {
                        s += 1;
                    }
                    if at(lx.s, s) == '\n' {
                        s += 1;
                    }
                    let mut len: i64 = -1;
                    let lo = dict.get("Length");
                    match lo {
                        .Int(v) => { len = v; }
                        .Ref(r) => {
                            let lv = self.object(r.num) catch Object.Null;
                            len = lv.as_int() catch -1;
                        }
                        else => {}
                    }
                    let d = lx.s;
                    let mut ok = (len >= 0) and (@as(u64, @bit_cast(u64, len)) <= @as(u64, d.len - @min(s, d.len)));
                    if ok {
                        let e = s + @truncate(usize, @bit_cast(u64, len));
                        let q = skip_white(d, e);
                        ok = strs.starts_with(d[q..], "endstream");
                    }
                    if not ok {
                        // find endstream instead
                        let e = strs.index_of_from(d, "endstream", s) catch d.len;
                        let mut t = e;
                        if (t > s) and (d[t - 1] == '\n') {
                            t -= 1;
                        }
                        if (t > s) and (d[t - 1] == '\r') {
                            t -= 1;
                        }
                        len = @as(i64, @bit_cast(i64, @as(u64, t - s)));
                    }
                    return Object.Stream(Stream.{ dict: dict, start: s, len: @truncate(usize, @bit_cast(u64, len)) });
                }
            }
            else => {}
        }
        return o;
    }

    /// One object from the lexer.
    fn parse(self: *mut Reader, lx: *mut Lexer, depth: u32) !Object {
        if depth > MAX_DEPTH {
            return error.Malformed;
        }
        let s = lx.s;
        lx.pos = skip_white(s, lx.pos);
        if lx.pos >= s.len {
            return error.Malformed;
        }
        let c = s[lx.pos];
        if c == '/' {
            return Object.Name(self.name(lx)!);
        }
        if c == '(' {
            return Object.Str(self.literal(lx)!);
        }
        if c == '<' {
            if at(s, lx.pos + 1) == '<' {
                lx.pos += 2;
                let mut keys = vec.Vec.[[]u8].init();
                defer keys.deinit();
                let mut vals = vec.Vec.[Object].init();
                defer vals.deinit();
                while true {
                    lx.pos = skip_white(s, lx.pos);
                    if lx.pos >= s.len {
                        return error.Malformed;
                    }
                    if (s[lx.pos] == '>') and (at(s, lx.pos + 1) == '>') {
                        lx.pos += 2;
                        break;
                    }
                    if s[lx.pos] != '/' {
                        // junk: skip a token
                        _ = self.parse(lx, depth + 1)!;
                        continue;
                    }
                    let k = self.name(lx)!;
                    let p0 = skip_white(s, lx.pos);
                    if (at(s, p0) == '>') and (at(s, p0 + 1) == '>') {
                        keys.push(k)!;
                        vals.push(Object.Null)!;
                        continue;
                    }
                    let v = self.parse(lx, depth + 1)!;
                    keys.push(k)!;
                    vals.push(v)!;
                }
                return Object.Dict(self.make_dict(keys.items(), vals.items())!);
            }
            return Object.Str(self.hex_string(lx)!);
        }
        if c == '[' {
            lx.pos += 1;
            let mut items = vec.Vec.[Object].init();
            defer items.deinit();
            while true {
                lx.pos = skip_white(s, lx.pos);
                if lx.pos >= s.len {
                    return error.Malformed;
                }
                if s[lx.pos] == ']' {
                    lx.pos += 1;
                    break;
                }
                items.push(self.parse(lx, depth + 1)!)!;
            }
            let arr = mem.alloc.[Object](items.len)!;
            @mem_copy(arr, items.items());
            self.objects.push(arr) catch |e| {
                mem.free(arr);
                return e;
            };
            return Object.Array(arr);
        }
        if (c == '+') or (c == '-') or (c == '.') or ((c >= '0') and (c <= '9')) {
            let mut iv: i64 = 0;
            let mut fv: f64 = 0.0;
            let mut isint = false;
            let e = scan_number(s, lx.pos, &mut iv, &mut fv, &mut isint);
            if e == lx.pos {
                lx.pos += 1;
                return Object.Null;
            }
            lx.pos = e;
            if not isint {
                return Object.Real(fv);
            }
            // "n g R"?
            let p1 = skip_white(s, e);
            let mut g: i64 = 0;
            let e2 = scan_number(s, p1, &mut g, &mut fv, &mut isint);
            if (e2 != p1) and isint {
                let p2 = skip_white(s, e2);
                if (at(s, p2) == 'R') and not is_regular(at(s, p2 + 1)) and (iv >= 0) and (iv < (1 << 31)) and
                   (g >= 0) and (g < 65536) {
                    lx.pos = p2 + 1;
                    return Object.Ref(
                        Ref.{ num: @truncate(u32, @bit_cast(u64, iv)), gen: @truncate(u32, @bit_cast(u64, g)) });
                }
            }
            return Object.Int(iv);
        }
        // keywords
        let start = lx.pos;
        while (lx.pos < s.len) and is_regular(s[lx.pos]) {
            lx.pos += 1;
        }
        if lx.pos == start {
            lx.pos += 1;  // a stray delimiter
            return Object.Null;
        }
        let w = s[start..lx.pos];
        if strs.eq(w, "true") {
            return Object.Bool(true);
        }
        if strs.eq(w, "false") {
            return Object.Bool(false);
        }
        return Object.Null;
    }

    fn make_dict(self: *mut Reader, keys: [][]u8, vals: []Object) !Dict {
        let k = mem.alloc.[[]u8](keys.len)!;
        @mem_copy(k, keys);
        self.names.push(k) catch |e| {
            mem.free(k);
            return e;
        };
        let v = mem.alloc.[Object](vals.len)!;
        @mem_copy(v, vals);
        self.objects.push(v) catch |e| {
            mem.free(v);
            return e;
        };
        return Dict.{ keys: k, vals: v };
    }

    fn name(self: *mut Reader, lx: *mut Lexer) ![]u8 {
        let s = lx.s;
        lx.pos += 1;
        let start = lx.pos;
        let mut escapes = false;
        while (lx.pos < s.len) and is_regular(s[lx.pos]) {
            if s[lx.pos] == '#' {
                escapes = true;
            }
            lx.pos += 1;
        }
        let raw = s[start..lx.pos];
        if not escapes {
            return raw;
        }
        let mut out = strs.String.init();
        let mut i: usize = 0;
        while i < raw.len {
            if (raw[i] == '#') and (i + 2 < raw.len + 1) and (hexval(at(raw, i + 1)) >= 0) and
               (hexval(at(raw, i + 2)) >= 0) {
                out.push(@truncate(u8, @bit_cast(u32, (hexval(raw[i + 1]) << 4) | hexval(raw[i + 2])))) catch |e| {
                    out.deinit();
                    return e;
                };
                i += 3;
            } else {
                out.push(raw[i]) catch |e| {
                    out.deinit();
                    return e;
                };
                i += 1;
            }
        }
        return self.keep(out.take()!);
    }

    fn literal(self: *mut Reader, lx: *mut Lexer) ![]u8 {
        let s = lx.s;
        lx.pos += 1;
        let mut out = strs.String.init();
        let r = literal_into(s, &mut lx.pos, &mut out);
        r catch |e| {
            out.deinit();
            return e;
        };
        return self.keep(out.take()!);
    }

    fn hex_string(self: *mut Reader, lx: *mut Lexer) ![]u8 {
        let s = lx.s;
        lx.pos += 1;
        let mut out = strs.String.init();
        let r = hex_into(s, &mut lx.pos, &mut out);
        r catch |e| {
            out.deinit();
            return e;
        };
        return self.keep(out.take()!);
    }

    // --- streams ---

    /// A stream's data, decoded (kept until deinit).
    pub fn stream_data(self: *mut Reader, st: Stream) ![]u8 {
        return self.decode(st);
    }

    fn decode(self: *mut Reader, st: Stream) ![]u8 {
        if (st.start > self.data.len) or (st.len > self.data.len - st.start) {
            return error.Malformed;
        }
        let raw = self.data[st.start..st.start + st.len];
        let filters = self.resolve(st.dict.get("Filter"))!;
        let parms = self.resolve(st.dict.get("DecodeParms"))!;
        let mut cur = mem.dup.[u8](raw)!;
        let mut k: usize = 0;
        while true {
            let mut f: []u8 = "";
            let mut pr = Object.Null;
            match filters {
                .Name(n) => {
                    if k == 0 {
                        f = n;
                        pr = parms;
                    }
                }
                .Array(a) => {
                    if k < a.len {
                        let fo = self.resolve(a[k]) catch Object.Null;
                        f = fo.as_name() catch "";
                        match parms {
                            .Array(pa) => {
                                if k < pa.len {
                                    pr = self.resolve(pa[k]) catch Object.Null;
                                }
                            }
                            else => {}
                        }
                        if f.len == 0 {
                            f = "?";
                        }
                    }
                }
                else => {}
            }
            if f.len == 0 {
                break;
            }
            let next = self.apply_filter(f, cur, pr) catch |e| {
                mem.free(cur);
                return e;
            };
            mem.free(cur);
            cur = next;
            k += 1;
            if k > 8 {
                break;
            }
        }
        if cur.len > self.budget {
            mem.free(cur);
            return error.TooLarge;
        }
        self.budget -= cur.len;
        return self.keep(cur);
    }

    fn apply_filter(self: *mut Reader, f: []u8, data: []u8, parms: Object) ![]mut u8 {
        let limit = @min(self.max_stream, self.budget);
        if strs.eq(f, "FlateDecode") or strs.eq(f, "Fl") {
            let out = deflate.decompress(data, deflate.ZLIB, limit) catch |e| {
                if (e == error.OutOfMemory) or (e == error.TooLarge) {
                    return e;
                }
                // raw deflate?
                let raw = deflate.decompress(data, deflate.RAW, limit) catch |e2| {
                    if (e2 == error.OutOfMemory) or (e2 == error.TooLarge) {
                        return e2;
                    }
                    return mem.alloc.[u8](0);
                };
                return predict(raw, parms);
            };
            return predict(out, parms);
        }
        if strs.eq(f, "LZWDecode") or strs.eq(f, "LZW") {
            let mut early: i64 = 1;
            match parms {
                .Dict(d) => { early = d.get("EarlyChange").as_int() catch 1; }
                else => {}
            }
            let out = lzw_decode(data, early != 0, limit)!;
            return predict(out, parms);
        }
        if strs.eq(f, "ASCIIHexDecode") or strs.eq(f, "AHx") {
            let mut out = strs.String.init();
            let mut p: usize = 0;
            let mut tmp: []u8 = data;
            let mut src = strs.String.init();
            defer src.deinit();
            src.push('<') catch |e| {
                out.deinit();
                return e;
            };
            src.push_str(tmp) catch |e| {
                out.deinit();
                return e;
            };
            tmp = src.bytes();
            hex_into(tmp, &mut p, &mut out) catch |e| {
                out.deinit();
                return e;
            };
            return out.take();
        }
        if strs.eq(f, "ASCII85Decode") or strs.eq(f, "A85") {
            return ascii85(data, limit);
        }
        if strs.eq(f, "RunLengthDecode") or strs.eq(f, "RL") {
            return run_length(data, limit);
        }
        // images (DCT, JPX, JBIG2, CCITT) and anything else stay as they are
        return mem.dup.[u8](data);
    }

    // --- pages ---

    fn load_pages(self: *mut Reader) !void {
        let root = self.get(self.trailer, "Root")!;
        let cat = root.as_dict() catch return error.Malformed;
        let top = self.get(cat, "Pages")!;
        let top_d = top.as_dict() catch return error.Malformed;
        // depth first, with inherited attributes copied into each page
        let mut stack = vec.Vec.[Dict].init();
        defer stack.deinit();
        let mut seen = vec.Vec.[u8].init();
        defer seen.deinit();
        for _k in 0..self.xref.len {
            seen.push(0)!;
        }
        stack.push(top_d)!;
        let mut steps: usize = 0;
        while stack.len > 0 {
            steps += 1;
            if steps > (1 << 22) {
                return error.Malformed;
            }
            let d = stack.pop()!;
            let kids = self.get(d, "Kids")!;
            let ty = self.get(d, "Type")!;
            let tn = ty.as_name() catch "";
            match kids {
                .Array(a) => {
                    if not strs.eq(tn, "Page") {
                        // children in reverse, so they come off the stack in order
                        let mut k = a.len;
                        while k > 0 {
                            k -= 1;
                            let mut num: usize = 0;
                            match a[k] {
                                .Ref(r) => { num = @as(usize, r.num); }
                                else => {}
                            }
                            if (num > 0) and (num < seen.len) {
                                if seen.get(num) != 0 {
                                    continue;
                                }
                                seen.set(num, 1);
                            }
                            let kid = self.resolve(a[k])!;
                            let kd = kid.as_dict() catch continue;
                            stack.push(self.inherit(d, kd)!)!;
                        }
                        continue;
                    }
                }
                else => {}
            }
            self.pages.push(d)!;
        }
    }

    /// A child with its parent's inheritable attributes where it has none.
    fn inherit(self: *mut Reader, parent: Dict, kid: Dict) !Dict {
        let names: [4][]u8 = ["Resources", "MediaBox", "CropBox", "Rotate"];
        let mut keys = vec.Vec.[[]u8].init();
        defer keys.deinit();
        let mut vals = vec.Vec.[Object].init();
        defer vals.deinit();
        keys.extend(kid.keys)!;
        vals.extend(kid.vals)!;
        let mut added = false;
        for n in names {
            if kid.get(n).is_null() and not parent.get(n).is_null() {
                keys.push(n)!;
                vals.push(parent.get(n))!;
                added = true;
            }
        }
        if not added {
            return kid;
        }
        return self.make_dict(keys.items(), vals.items());
    }

    pub fn page_count(self: *Reader) usize {
        return self.pages.len;
    }

    /// A page's dictionary (with inherited attributes).
    pub fn page(self: *Reader, i: usize) !Dict {
        if i >= self.pages.len {
            return error.NotFound;
        }
        return self.pages.get(i);
    }

    /// A page's size in points: [width, height] of its MediaBox.
    pub fn page_size(self: *mut Reader, i: usize) ![2]f64 {
        let d = self.page(i)!;
        let mb = self.get(d, "MediaBox")!;
        let a = mb.as_array() catch return [612.0, 792.0];
        if a.len < 4 {
            return [612.0, 792.0];
        }
        let x0 = (self.resolve(a[0])!).as_num() catch 0.0;
        let y0 = (self.resolve(a[1])!).as_num() catch 0.0;
        let x1 = (self.resolve(a[2])!).as_num() catch 612.0;
        let y1 = (self.resolve(a[3])!).as_num() catch 792.0;
        return [@abs(x1 - x0), @abs(y1 - y0)];
    }

    /// A metadata entry (Title, Author, ...) as UTF-8; empty if absent.
    pub fn info(self: *mut Reader, key: []u8) ![]mut u8 {
        let io_ = self.get(self.trailer, "Info")!;
        let d = io_.as_dict() catch return mem.alloc.[u8](0);
        let v = self.get(d, key)!;
        let s = v.as_str() catch return mem.alloc.[u8](0);
        return text_string(s);
    }

    /// The text of page i, as UTF-8 lines.
    pub fn page_text(self: *mut Reader, i: usize) ![]mut u8 {
        let d = self.page(i)!;
        let mut ex = Extractor.init(self);
        defer ex.deinit();
        let res = self.get(d, "Resources")!;
        let rd = res.as_dict() catch empty_dict();
        let contents = self.get(d, "Contents")!;
        let mut data = strs.String.init();
        defer data.deinit();
        match contents {
            .Stream(st) => { data.push_str(self.decode(st) catch "")!; }
            .Array(a) => {
                for c in a {
                    let o = self.resolve(c)!;
                    let st = o.as_stream() catch continue;
                    data.push_str(self.decode(st) catch "")!;
                    data.push('\n')!;
                }
            }
            else => {}
        }
        ex.run(data.bytes(), rd, 0)!;
        return ex.out.take();
    }
}

struct Lexer {
    s: []u8,
    pos: usize,
}

struct ObjStm {
    data: []u8,
    offsets: vec.Vec.[u64],  // where each object starts in data
}

/// A literal string's contents from s[pos] (just after the '(').
fn literal_into(s: []u8, pos: *mut usize, out: *mut strs.String) !void {
    let mut depth: usize = 1;
    let mut i = pos.*;
    while i < s.len {
        let c = s[i];
        i += 1;
        if c == '\\' {
            if i >= s.len {
                break;
            }
            let e = s[i];
            i += 1;
            if e == 'n' {
                out.push('\n')!;
            } else if e == 'r' {
                out.push('\r')!;
            } else if e == 't' {
                out.push('\t')!;
            } else if e == 'b' {
                out.push(8)!;
            } else if e == 'f' {
                out.push(12)!;
            } else if (e >= '0') and (e <= '7') {
                let mut v: u32 = @as(u32, e - '0');
                let mut n: usize = 1;
                while (n < 3) and (i < s.len) and (s[i] >= '0') and (s[i] <= '7') {
                    v = (v * 8) + @as(u32, s[i] - '0');
                    i += 1;
                    n += 1;
                }
                out.push(@truncate(u8, v))!;
            } else if e == '\r' {
                if at(s, i) == '\n' {
                    i += 1;
                }
            } else if e == '\n' {
                // a line continuation
            } else {
                out.push(e)!;
            }
        } else if c == '(' {
            depth += 1;
            out.push(c)!;
        } else if c == ')' {
            depth -= 1;
            if depth == 0 {
                break;
            }
            out.push(c)!;
        } else {
            out.push(c)!;
        }
    }
    pos.* = i;
}

/// A hex string's contents from s[pos] (just after the '<').
fn hex_into(s: []u8, pos: *mut usize, out: *mut strs.String) !void {
    let mut i = pos.*;
    let mut hi: i32 = -1;
    while i < s.len {
        let c = s[i];
        i += 1;
        if c == '>' {
            break;
        }
        let v = hexval(c);
        if v < 0 {
            continue;
        }
        if hi < 0 {
            hi = v;
        } else {
            out.push(@truncate(u8, @bit_cast(u32, (hi << 4) | v)))!;
            hi = -1;
        }
    }
    if hi >= 0 {
        out.push(@truncate(u8, @bit_cast(u32, hi << 4)))!;
    }
    pos.* = i;
}

/// A text string (UTF-16 with a byte order mark, UTF-8 with one, or
/// PDFDocEncoding) as UTF-8.
pub fn text_string(s: []u8) ![]mut u8 {
    let mut out = strs.String.init();
    let r = text_string_into(s, &mut out);
    r catch |e| {
        out.deinit();
        return e;
    };
    return out.take();
}

fn text_string_into(s: []u8, out: *mut strs.String) !void {
    if (s.len >= 2) and (((s[0] == 0xfe) and (s[1] == 0xff)) or ((s[0] == 0xff) and (s[1] == 0xfe))) {
        let le = s[0] == 0xff;
        let mut i: usize = 2;
        while i + 1 < s.len {
            let mut u = (@as(u32, s[i]) << 8) | @as(u32, s[i + 1]);
            if le {
                u = (@as(u32, s[i + 1]) << 8) | @as(u32, s[i]);
            }
            i += 2;
            if (u >= 0xd800) and (u < 0xdc00) and (i + 1 < s.len) {
                let mut lo = (@as(u32, s[i]) << 8) | @as(u32, s[i + 1]);
                if le {
                    lo = (@as(u32, s[i + 1]) << 8) | @as(u32, s[i]);
                }
                if (lo >= 0xdc00) and (lo < 0xe000) {
                    u = 0x10000 + ((u - 0xd800) << 10) + (lo - 0xdc00);
                    i += 2;
                }
            }
            if (u >= 0xd800) and (u < 0xe000) {
                u = 0xfffd;
            }
            out.push_rune(u)!;
        }
        return;
    }
    if (s.len >= 3) and (s[0] == 0xef) and (s[1] == 0xbb) and (s[2] == 0xbf) {
        out.push_str(s[3..])!;
        return;
    }
    for c in s {
        let u = @as(u32, tb.PDF_DOC[@as(usize, c)]);
        if u != 0 {
            out.push_rune(u)!;
        }
    }
}

// -----------------------------------------------------------------------------
// Filters
// -----------------------------------------------------------------------------

/// PNG (10 and up) and TIFF (2) predictors.
fn predict(data: []mut u8, parms: Object) ![]mut u8 {
    let d = parms.as_dict() catch return data;
    let pred = d.get("Predictor").as_int() catch 1;
    if pred < 2 {
        return data;
    }
    let colors = @min(@max(d.get("Colors").as_int() catch 1, 1), 32);
    let bpc = @min(@max(d.get("BitsPerComponent").as_int() catch 8, 1), 16);
    let cols = @min(@max(d.get("Columns").as_int() catch 1, 1), 1 << 24);
    let bpp = @truncate(usize, @bit_cast(u64, @max((colors * bpc) / 8, 1)));
    let row = @truncate(usize, @bit_cast(u64, ((colors * bpc * cols) + 7) / 8));
    if pred == 2 {
        if bpc == 8 {
            let mut i: usize = 0;
            while i < data.len {
                let e = @min(i + row, data.len);
                for k in i + bpp..e {
                    data[k] = data[k] +% data[k - bpp];
                }
                i = e;
            }
        }
        return data;
    }
    // PNG: each row starts with a filter type byte
    let mut out = mem.alloc.[u8]((data.len / (row + 1)) * row) catch |e| {
        mem.free(data);
        return e;
    };
    let rows = data.len / (row + 1);
    for r in 0..rows {
        let src = data[r * (row + 1)..(r + 1) * (row + 1)];
        let ft = src[0];
        let o = r * row;
        for k in 0..row {
            let x = src[k + 1];
            let mut a: u8 = 0;
            if k >= bpp {
                a = out[o + k - bpp];
            }
            let mut b: u8 = 0;
            if r > 0 {
                b = out[o + k - row];
            }
            let mut c: u8 = 0;
            if (r > 0) and (k >= bpp) {
                c = out[o + k - row - bpp];
            }
            let mut v = x;
            if ft == 1 {
                v = x +% a;
            } else if ft == 2 {
                v = x +% b;
            } else if ft == 3 {
                v = x +% @truncate(u8, (@as(u32, a) + @as(u32, b)) / 2);
            } else if ft == 4 {
                let p = @as(i32, a) + @as(i32, b) - @as(i32, c);
                let pa = @abs(p - @as(i32, a));
                let pb = @abs(p - @as(i32, b));
                let pc = @abs(p - @as(i32, c));
                let mut pr = c;
                if (pa <= pb) and (pa <= pc) {
                    pr = a;
                } else if pb <= pc {
                    pr = b;
                }
                v = x +% pr;
            }
            out[o + k] = v;
        }
    }
    mem.free(data);
    return out;
}

fn lzw_decode(data: []u8, early: bool, limit: usize) ![]mut u8 {
    let mut out = strs.String.init();
    // the table: each entry is a previous entry plus a byte
    let mut prefix = vec.Vec.[u32].init();
    defer prefix.deinit();
    let mut last = vec.Vec.[u8].init();
    defer last.deinit();
    let mut first = vec.Vec.[u8].init();
    defer first.deinit();
    let mut lens = vec.Vec.[u32].init();
    defer lens.deinit();
    let mut tmp = vec.Vec.[u8].init();
    defer tmp.deinit();
    let r = lzw_run(data, early, limit, &mut out, &mut prefix, &mut last, &mut first, &mut lens, &mut tmp);
    r catch |e| {
        out.deinit();
        return e;
    };
    return out.take();
}

fn lzw_reset(prefix: *mut vec.Vec.[u32], last: *mut vec.Vec.[u8], first: *mut vec.Vec.[u8],
             lens: *mut vec.Vec.[u32]) !void {
    prefix.clear();
    last.clear();
    first.clear();
    lens.clear();
    for k in @as(u32, 0)..258 {
        prefix.push(0xffffffff)!;
        last.push(@truncate(u8, k))!;
        first.push(@truncate(u8, k))!;
        lens.push(1)!;
    }
}

fn lzw_run(data: []u8, early: bool, limit: usize, out: *mut strs.String, prefix: *mut vec.Vec.[u32],
           last: *mut vec.Vec.[u8], first: *mut vec.Vec.[u8], lens: *mut vec.Vec.[u32], tmp: *mut vec.Vec.[u8]) !void {
    lzw_reset(prefix, last, first, lens)!;
    let mut bits: u32 = 9;
    let mut acc: u64 = 0;
    let mut have: u32 = 0;
    let mut prev: u32 = 0xffffffff;
    let mut i: usize = 0;
    while true {
        while (have < bits) and (i < data.len) {
            acc = (acc << 8) | @as(u64, data[i]);
            have += 8;
            i += 1;
        }
        if have < bits {
            break;
        }
        let code = @truncate(u32, (acc >> @as(u64, have - bits)) & ((@as(u64, 1) << @as(u64, bits)) - 1));
        have -= bits;
        if code == 256 {
            lzw_reset(prefix, last, first, lens)!;
            bits = 9;
            prev = 0xffffffff;
            continue;
        }
        if code == 257 {
            break;
        }
        let n = @truncate(u32, prefix.len);
        if code > n {
            break;  // corrupt
        }
        if prev != 0xffffffff {
            // add prev + first byte of this code's string
            let fb = if_code(code == n, first.get(@as(usize, prev)), first.get(@as(usize, @min(code, n - 1))));
            if n < 4096 {
                prefix.push(prev)!;
                last.push(fb)!;
                first.push(first.get(@as(usize, prev)))!;
                lens.push(lens.get(@as(usize, prev)) + 1)!;
            }
        } else if code == n {
            break;
        }
        // emit code's string
        let c = @min(code, @truncate(u32, prefix.len) - 1);
        tmp.clear();
        let mut k = c;
        let mut guard: u32 = 0;
        while (k != 0xffffffff) and (guard < 4097) {
            tmp.push(last.get(@as(usize, k)))!;
            k = prefix.get(@as(usize, k));
            guard += 1;
        }
        let t = tmp.items();
        let mut j = t.len;
        while j > 0 {
            j -= 1;
            out.push(t[j])!;
        }
        if out.len > limit {
            return error.TooLarge;
        }
        prev = c;
        let size = @truncate(u32, prefix.len) + if_u32(early, 1, 0);
        if (size >= 512) and (bits == 9) {
            bits = 10;
        } else if (size >= 1024) and (bits == 10) {
            bits = 11;
        } else if (size >= 2048) and (bits == 11) {
            bits = 12;
        }
    }
}

fn if_code(c: bool, a: u8, b: u8) u8 {
    if c {
        return a;
    }
    return b;
}

fn if_u32(c: bool, a: u32, b: u32) u32 {
    if c {
        return a;
    }
    return b;
}

fn ascii85(data: []u8, limit: usize) ![]mut u8 {
    let mut out = strs.String.init();
    let mut group: [5]u32 = [0, 0, 0, 0, 0];
    let mut n: usize = 0;
    let mut i: usize = 0;
    if strs.starts_with(data, "<~") {
        i = 2;
    }
    while i < data.len {
        let c = data[i];
        i += 1;
        if c == '~' {
            break;
        }
        if is_white(c) {
            continue;
        }
        if (c == 'z') and (n == 0) {
            out.push_str("\x00\x00\x00\x00") catch |e| {
                out.deinit();
                return e;
            };
            continue;
        }
        if (c < '!') or (c > 'u') {
            continue;
        }
        group[n] = @as(u32, c - '!');
        n += 1;
        if n == 5 {
            let mut v: u64 = 0;
            for k in @as(usize, 0)..5 {
                v = (v * 85) + @as(u64, group[k]);
            }
            let w = @truncate(u32, v);
            for k in @as(u32, 0)..4 {
                out.push(@truncate(u8, w >> (24 - (8 * k)))) catch |e| {
                    out.deinit();
                    return e;
                };
            }
            n = 0;
        }
        if out.len > limit {
            out.deinit();
            return error.TooLarge;
        }
    }
    if n > 1 {
        for k in n..5 {
            group[k] = 84;
        }
        let mut v: u64 = 0;
        for k in @as(usize, 0)..5 {
            v = (v * 85) + @as(u64, group[k]);
        }
        let w = @truncate(u32, v);
        for k in @as(u32, 0)..@truncate(u32, n - 1) {
            out.push(@truncate(u8, w >> (24 - (8 * k)))) catch |e| {
                out.deinit();
                return e;
            };
        }
    }
    return out.take();
}

fn run_length(data: []u8, limit: usize) ![]mut u8 {
    let mut out = strs.String.init();
    let mut i: usize = 0;
    while i < data.len {
        let n = @as(usize, data[i]);
        i += 1;
        if n == 128 {
            break;
        }
        if n < 128 {
            let e = @min(i + n + 1, data.len);
            out.push_str(data[i..e]) catch |err| {
                out.deinit();
                return err;
            };
            i = e;
        } else if i < data.len {
            for _k in 0..257 - n {
                out.push(data[i]) catch |err| {
                    out.deinit();
                    return err;
                };
            }
            i += 1;
        }
        if out.len > limit {
            out.deinit();
            return error.TooLarge;
        }
    }
    return out.take();
}

// -----------------------------------------------------------------------------
// Text extraction
// -----------------------------------------------------------------------------

/// How a font's strings become text.
struct FontMap {
    key: []u8,                    // the resource name
    two_byte: bool,               // Identity-H style: every code is 2 bytes
    ranges: [4]u32,               // codespace: for each code length (1-4 bytes), whether it is used
    uni: maps.Map.[u32, u64],     // code -> (offset << 16 | length) in `pool`
    simple: [256]u32,             // simple fonts: the encoding's character per code
    widths: maps.Map.[u32, f64],  // code (or CID) -> width in 1/1000
    default_width: f64,
    has_simple: bool,
}

struct Extractor {
    r: *mut Reader,
    out: strs.String,
    pool: strs.String,
    fonts: vec.Vec.[FontMap],
    // the text state
    font: usize,  // index in fonts, or NONE
    size: f64,
    char_space: f64,
    word_space: f64,
    hscale: f64,
    leading: f64,
    rise: f64,
    tm: [6]f64,
    tlm: [6]f64,
    ctm: [6]f64,
    stack: vec.Vec.[[6]f64],
    // where the last character ended, in device space
    last_x: f64,
    last_y: f64,
    last_size: f64,
    started: bool,
    steps: usize,

    fn init(r: *mut Reader) Extractor {
        let id: [6]f64 = [1.0, 0.0, 0.0, 1.0, 0.0, 0.0];
        return Extractor.{
            r: r,
            out: strs.String.init(),
            pool: strs.String.init(),
            fonts: vec.Vec.[FontMap].init(),
            font: NO_FONT,
            size: 0.0,
            char_space: 0.0,
            word_space: 0.0,
            hscale: 1.0,
            leading: 0.0,
            rise: 0.0,
            tm: id,
            tlm: id,
            ctm: id,
            stack: vec.Vec.[[6]f64].init(),
            last_x: 0.0,
            last_y: 0.0,
            last_size: 0.0,
            started: false,
            steps: 0,
        };
    }

    fn deinit(self: *mut Extractor) void {
        self.out.deinit();
        self.pool.deinit();
        for f in self.fonts.items() {
            let mut u = f.uni;
            u.deinit();
            let mut w = f.widths;
            w.deinit();
        }
        self.fonts.deinit();
        self.stack.deinit();
    }

    /// Interprets a content stream with its resources.
    fn run(self: *mut Extractor, data: []u8, res: Dict, depth: u32) !void {
        if depth > 8 {
            return;
        }
        let mut ops = vec.Vec.[Operand].init();
        defer ops.deinit();
        let mut scratch = strs.String.init();
        defer scratch.deinit();
        let mut i: usize = 0;
        while i < data.len {
            self.steps += 1;
            if self.steps > (1 << 26) {
                return;
            }
            i = skip_white(data, i);
            if i >= data.len {
                break;
            }
            let c = data[i];
            if (c == '+') or (c == '-') or (c == '.') or ((c >= '0') and (c <= '9')) {
                let mut iv: i64 = 0;
                let mut fv: f64 = 0.0;
                let mut isint = false;
                let e = scan_number(data, i, &mut iv, &mut fv, &mut isint);
                if e == i {
                    i += 1;
                    continue;
                }
                ops.push(Operand.{ kind: OP_NUM, num: fv, a: 0, b: 0 })!;
                i = e;
            } else if c == '(' {
                let a = scratch.len;
                let mut p = i + 1;
                literal_into(data, &mut p, &mut scratch)!;
                ops.push(Operand.{ kind: OP_STR, num: 0.0, a: a, b: scratch.len })!;
                i = p;
            } else if (c == '<') and (at(data, i + 1) != '<') {
                let a = scratch.len;
                let mut p = i + 1;
                hex_into(data, &mut p, &mut scratch)!;
                ops.push(Operand.{ kind: OP_STR, num: 0.0, a: a, b: scratch.len })!;
                i = p;
            } else if c == '/' {
                let a = scratch.len;
                i += 1;
                while (i < data.len) and is_regular(data[i]) {
                    scratch.push(data[i])!;
                    i += 1;
                }
                ops.push(Operand.{ kind: OP_NAME, num: 0.0, a: a, b: scratch.len })!;
            } else if c == '[' {
                ops.push(Operand.{ kind: OP_OPEN, num: 0.0, a: 0, b: 0 })!;
                i += 1;
            } else if c == ']' {
                ops.push(Operand.{ kind: OP_CLOSE, num: 0.0, a: 0, b: 0 })!;
                i += 1;
            } else if (c == '<') and (at(data, i + 1) == '<') {
                // a dictionary operand (marked content): skip it
                let mut depth2: usize = 0;
                while i < data.len {
                    if (data[i] == '<') and (at(data, i + 1) == '<') {
                        depth2 += 1;
                        i += 2;
                    } else if (data[i] == '>') and (at(data, i + 1) == '>') {
                        depth2 -= 1;
                        i += 2;
                        if depth2 == 0 {
                            break;
                        }
                    } else if data[i] == '(' {
                        let mut p = i + 1;
                        let mut junk = strs.String.init();
                        defer junk.deinit();
                        literal_into(data, &mut p, &mut junk)!;
                        i = p;
                    } else {
                        i += 1;
                    }
                }
                ops.push(Operand.{ kind: OP_OTHER, num: 0.0, a: 0, b: 0 })!;
            } else if is_regular(c) {
                let s = i;
                while (i < data.len) and is_regular(data[i]) {
                    i += 1;
                }
                let op = data[s..i];
                if strs.eq(op, "BI") {
                    // an inline image: skip to EI
                    let id = strs.index_of_from(data, "ID", i) catch data.len;
                    let mut k = id + 2;
                    while k + 2 <= data.len {
                        if is_white(at(data, k)) and (at(data, k + 1) == 'E') and (at(data, k + 2) == 'I') and
                           ((k + 3 >= data.len) or not is_regular(data[k + 3])) {
                            break;
                        }
                        k += 1;
                    }
                    i = @min(k + 3, data.len);
                } else {
                    self.operator(op, ops.items(), scratch.bytes(), res, depth)!;
                }
                ops.clear();
                scratch.clear();
            } else {
                i += 1;
            }
            if ops.len > 4096 {
                ops.clear();
                scratch.clear();
            }
        }
    }

    fn num(ops: []Operand, k: usize) f64 {
        if (k < ops.len) and (ops[k].kind == OP_NUM) {
            return ops[k].num;
        }
        return 0.0;
    }

    fn operator(self: *mut Extractor, op: []u8, ops: []Operand, sc: []u8, res: Dict, depth: u32) !void {
        let n = ops.len;
        if strs.eq(op, "q") {
            if self.stack.len < 256 {
                self.stack.push(self.ctm)!;
            }
        } else if strs.eq(op, "Q") {
            self.ctm = self.stack.pop() catch self.ctm;
        } else if strs.eq(op, "cm") and (n >= 6) {
            let m: [6]f64 = [
                Extractor.num(ops, n - 6),
                Extractor.num(ops, n - 5),
                Extractor.num(ops, n - 4),
                Extractor.num(ops, n - 3),
                Extractor.num(ops, n - 2),
                Extractor.num(ops, n - 1),
            ];
            self.ctm = mul(m, self.ctm);
        } else if strs.eq(op, "BT") {
            self.tm = [1.0, 0.0, 0.0, 1.0, 0.0, 0.0];
            self.tlm = self.tm;
        } else if strs.eq(op, "Tf") and (n >= 2) {
            self.size = Extractor.num(ops, n - 1);
            if ops[n - 2].kind == OP_NAME {
                self.font = self.load_font(sc[ops[n - 2].a..ops[n - 2].b], res)!;
            }
        } else if strs.eq(op, "Tc") and (n >= 1) {
            self.char_space = Extractor.num(ops, n - 1);
        } else if strs.eq(op, "Tw") and (n >= 1) {
            self.word_space = Extractor.num(ops, n - 1);
        } else if strs.eq(op, "Tz") and (n >= 1) {
            self.hscale = Extractor.num(ops, n - 1) / 100.0;
        } else if strs.eq(op, "TL") and (n >= 1) {
            self.leading = Extractor.num(ops, n - 1);
        } else if strs.eq(op, "Ts") and (n >= 1) {
            self.rise = Extractor.num(ops, n - 1);
        } else if (strs.eq(op, "Td") or strs.eq(op, "TD")) and (n >= 2) {
            let tx = Extractor.num(ops, n - 2);
            let ty = Extractor.num(ops, n - 1);
            if strs.eq(op, "TD") {
                self.leading = -ty;
            }
            self.tlm = mul([1.0, 0.0, 0.0, 1.0, tx, ty], self.tlm);
            self.tm = self.tlm;
        } else if strs.eq(op, "Tm") and (n >= 6) {
            self.tlm = [
                Extractor.num(ops, n - 6),
                Extractor.num(ops, n - 5),
                Extractor.num(ops, n - 4),
                Extractor.num(ops, n - 3),
                Extractor.num(ops, n - 2),
                Extractor.num(ops, n - 1),
            ];
            self.tm = self.tlm;
        } else if strs.eq(op, "T*") {
            self.next_line();
        } else if strs.eq(op, "Tj") and (n >= 1) and (ops[n - 1].kind == OP_STR) {
            self.show(sc[ops[n - 1].a..ops[n - 1].b])!;
        } else if strs.eq(op, "'") and (n >= 1) and (ops[n - 1].kind == OP_STR) {
            self.next_line();
            self.show(sc[ops[n - 1].a..ops[n - 1].b])!;
        } else if strs.eq(op, "\"") and (n >= 3) and (ops[n - 1].kind == OP_STR) {
            self.word_space = Extractor.num(ops, n - 3);
            self.char_space = Extractor.num(ops, n - 2);
            self.next_line();
            self.show(sc[ops[n - 1].a..ops[n - 1].b])!;
        } else if strs.eq(op, "TJ") {
            for o in ops {
                if o.kind == OP_STR {
                    self.show(sc[o.a..o.b])!;
                } else if o.kind == OP_NUM {
                    let tx = (-o.num / 1000.0) * self.size * self.hscale;
                    self.tm = mul([1.0, 0.0, 0.0, 1.0, tx, 0.0], self.tm);
                }
            }
        } else if strs.eq(op, "Do") and (n >= 1) and (ops[n - 1].kind == OP_NAME) {
            self.form(sc[ops[n - 1].a..ops[n - 1].b], res, depth)!;
        }
    }

    fn next_line(self: *mut Extractor) void {
        self.tlm = mul([1.0, 0.0, 0.0, 1.0, 0.0, -self.leading], self.tlm);
        self.tm = self.tlm;
    }

    /// A form XObject: its content, with its own resources.
    fn form(self: *mut Extractor, name: []u8, res: Dict, depth: u32) !void {
        let xo = self.r.get(res, "XObject")!;
        let xd = xo.as_dict() catch return;
        let f = self.r.get(xd, name)!;
        let st = f.as_stream() catch return;
        let sub = self.r.get(st.dict, "Subtype")!;
        let sn = sub.as_name() catch return;
        if not strs.eq(sn, "Form") {
            return;
        }
        let data = self.r.decode(st) catch return;
        let fr = self.r.get(st.dict, "Resources")!;
        let frd = fr.as_dict() catch res;
        let saved = self.ctm;
        let mo = self.r.get(st.dict, "Matrix")!;
        let ma = mo.as_array() catch no_objects();
        if ma.len == 6 {
            let mut m: [6]f64 = [1.0, 0.0, 0.0, 1.0, 0.0, 0.0];
            for k in @as(usize, 0)..6 {
                m[k] = ma[k].as_num() catch m[k];
            }
            self.ctm = mul(m, self.ctm);
        }
        // fonts are looked up by name in the form's own resources
        let saved_fonts = self.fonts.len;
        let saved_font = self.font;
        self.run(data, frd, depth + 1)!;
        // forget the form's fonts (their names may clash with the page's)
        while self.fonts.len > saved_fonts {
            let mut f2 = self.fonts.pop()!;
            f2.uni.deinit();
            f2.widths.deinit();
        }
        self.font = saved_font;
        self.ctm = saved;
    }

    /// Shows a string: decodes it to characters, placing each.
    fn show(self: *mut Extractor, s: []u8) !void {
        if self.font == NO_FONT {
            return;
        }
        let fi = self.font;
        let mut i: usize = 0;
        while i < s.len {
            let f = self.fonts.get_ptr(fi);
            // the code: its length from the codespace
            let mut len: usize = 1;
            if f.two_byte {
                len = 2;
            } else {
                let mut code: u32 = @as(u32, s[i]);
                len = 1;
                while (len < 4) and (f.ranges[len - 1] == 0) and (i + len < s.len) {
                    code = (code << 8) | @as(u32, s[i + len]);
                    len += 1;
                }
                _ = code;
            }
            let mut code: u32 = 0;
            for k in 0..len {
                code = (code << 8) | @as(u32, at(s, i + k));
            }
            i += len;
            let w = self.width(fi, code);
            // where the glyph goes, in device space
            let trm = mul([self.size * self.hscale, 0.0, 0.0, self.size, 0.0, self.rise], mul(self.tm, self.ctm));
            let x = trm[4];
            let y = trm[5];
            let dev_size = @sqrt((trm[2] * trm[2]) + (trm[3] * trm[3]));
            self.place(fi, code, x, y, dev_size)!;
            // advance
            let mut adv = ((w / 1000.0) * self.size) + self.char_space;
            if (len == 1) and (code == 32) {
                adv += self.word_space;
            }
            adv *= self.hscale;
            self.tm = mul([1.0, 0.0, 0.0, 1.0, adv, 0.0], self.tm);
            let end = mul(self.tm, self.ctm);
            self.last_x = end[4];
            self.last_y = end[5];
        }
    }

    fn width(self: *mut Extractor, fi: usize, code: u32) f64 {
        let f = self.fonts.get_ptr(fi);
        return f.widths.get(code) catch f.default_width;
    }

    /// Appends a character, with a line break or a space before it when it
    /// is far from the last one.
    fn place(self: *mut Extractor, fi: usize, code: u32, x: f64, y: f64, size: f64) !void {
        let f = self.fonts.get_ptr(fi);
        let mut text: []u8 = "";
        let mut buf: [4]u8 = [0; 4];
        if f.uni.contains(code) {
            let v = f.uni.get(code)!;
            let off = @truncate(usize, v >> 16);
            let len = @truncate(usize, v & 0xffff);
            text = self.pool.bytes()[off..off + len];
        } else if f.has_simple and (code < 256) {
            let u = f.simple[@as(usize, code)];
            if u != 0 {
                text = strs.encode_rune(buf[..], u);
            }
        }
        if text.len == 0 {
            return;
        }
        let sz = @max(size, 1.0);
        if self.started {
            let dy = @abs(y - self.last_y);
            let dx = x - self.last_x;
            if dy > (0.5 * @max(sz, self.last_size)) {
                self.out.push('\n')!;
            } else if (dx > (0.15 * sz)) or (dx < -(0.5 * sz)) {
                let lastc = self.out.buf[@max(self.out.len, 1) - 1];
                if (self.out.len > 0) and (lastc != ' ') and (lastc != '\n') and (text[0] != ' ') {
                    self.out.push(' ')!;
                }
            }
        }
        self.out.push_str(text)!;
        self.started = true;
        self.last_size = sz;
    }

    fn load_font(self: *mut Extractor, key: []u8, res: Dict) !usize {
        for i in 0..self.fonts.len {
            if strs.eq(self.fonts.get(i).key, key) {
                return i;
            }
        }
        let fo = self.r.get(res, "Font")!;
        let fd = fo.as_dict() catch return NO_FONT;
        let f = self.r.get(fd, key)!;
        let d = f.as_dict() catch return NO_FONT;
        let mut m = FontMap.{
            key: key,
            two_byte: false,
            ranges: [0, 0, 0, 0],
            uni: maps.Map.[u32, u64].init(hash.hash_u32, hash.eq_u32),
            simple: [0; 256],
            widths: maps.Map.[u32, f64].init(hash.hash_u32, hash.eq_u32),
            default_width: 500.0,
            has_simple: false,
        };
        // (the key's bytes live in the content scratch; keep a copy)
        let kc = self.r.keep(strs.dup(key)!)!;
        m.key = kc;
        self.setup_font(&mut m, d) catch |e| {
            m.uni.deinit();
            m.widths.deinit();
            if e == error.OutOfMemory {
                return e;
            }
            return NO_FONT;
        };
        self.fonts.push(m) catch |e| {
            m.uni.deinit();
            m.widths.deinit();
            return e;
        };
        return self.fonts.len - 1;
    }

    fn setup_font(self: *mut Extractor, m: *mut FontMap, d: Dict) !void {
        let sub = self.r.get(d, "Subtype")!;
        let sn = sub.as_name() catch "";
        let r = self.r;
        if strs.eq(sn, "Type0") {
            m.two_byte = true;
            m.default_width = 1000.0;
            let desc = r.get(d, "DescendantFonts")!;
            let da = desc.as_array() catch no_objects();
            if da.len > 0 {
                let cf = r.resolve(da[0])!;
                let cd = cf.as_dict() catch empty_dict();
                let dw = r.get(cd, "DW")!;
                m.default_width = dw.as_num() catch 1000.0;
                let wo = r.get(cd, "W")!;
                let wa = wo.as_array() catch no_objects();
                let mut k: usize = 0;
                while k < wa.len {
                    let first = (r.resolve(wa[k])!).as_int() catch break;
                    if k + 1 >= wa.len {
                        break;
                    }
                    let second = r.resolve(wa[k + 1])!;
                    match second {
                        .Array(ws) => {
                            for j in 0..@min(ws.len, 65536) {
                                let w = (r.resolve(ws[j])!).as_num() catch 0.0;
                                m.widths.put(@truncate(u32, @bit_cast(u64, first)) +% @truncate(u32, j), w)!;
                            }
                            k += 2;
                        }
                        else => {
                            let last = second.as_int() catch break;
                            if k + 2 >= wa.len {
                                break;
                            }
                            let w = (r.resolve(wa[k + 2])!).as_num() catch 0.0;
                            let mut c = first;
                            while (c <= last) and (c - first < 65536) {
                                m.widths.put(@truncate(u32, @bit_cast(u64, c)), w)!;
                                c += 1;
                            }
                            k += 3;
                        }
                    }
                }
            }
        } else {
            // a simple font: one-byte codes, its encoding, then its widths
            m.has_simple = true;
            m.ranges[0] = 1;
            let mut base = tb.STANDARD[..];
            if strs.eq(sn, "TrueType") {
                base = tb.WIN_ANSI[..];
            }
            let enc = r.get(d, "Encoding")!;
            let mut diffs: []Object = no_objects();
            match enc {
                .Name(n) => { base = encoding_table(n, base); }
                .Dict(ed) => {
                    let bo = r.get(ed, "BaseEncoding")!;
                    let bn = bo.as_name() catch "";
                    base = encoding_table(bn, base);
                    let dfo = r.get(ed, "Differences")!;
                    diffs = dfo.as_array() catch no_objects();
                }
                else => {}
            }
            for c in @as(usize, 0)..256 {
                let mut u = @as(u32, base[c]);
                if (u == 0) and (c >= 32) and (c < 127) {
                    u = @truncate(u32, c);
                }
                m.simple[c] = u;
            }
            let mut code: i64 = 0;
            for o in diffs {
                match o {
                    .Int(v) => { code = v; }
                    .Name(gname) => {
                        if (code >= 0) and (code < 256) {
                            let u = glyph_unicode(gname);
                            if u != 0 {
                                m.simple[@truncate(usize, @bit_cast(u64, code))] = u;
                            }
                        }
                        code += 1;
                    }
                    else => {}
                }
            }
            let fc = (r.get(d, "FirstChar")!).as_int() catch 0;
            let wo = r.get(d, "Widths")!;
            let wa = wo.as_array() catch no_objects();
            for j in 0..@min(wa.len, 256) {
                let w = (r.resolve(wa[j])!).as_num() catch 0.0;
                m.widths.put(@truncate(u32, @bit_cast(u64, fc)) +% @truncate(u32, j), w)!;
            }
        }
        // ToUnicode, when there is one, wins
        let tu = r.get(d, "ToUnicode")!;
        let ts = tu.as_stream() catch return;
        let cm = r.decode(ts) catch return;
        self.parse_cmap(m, cm)!;
    }

    /// A ToUnicode CMap: codespace ranges, bfchar and bfrange.
    fn parse_cmap(self: *mut Extractor, m: *mut FontMap, s: []u8) !void {
        let mut i: usize = 0;
        let mut toks = vec.Vec.[Tok].init();
        defer toks.deinit();
        let mut buf = strs.String.init();
        defer buf.deinit();
        // tokens: hex strings, arrays, and keywords
        while i < s.len {
            i = skip_white(s, i);
            if i >= s.len {
                break;
            }
            let c = s[i];
            if (c == '<') and (at(s, i + 1) != '<') {
                let a = buf.len;
                let mut p = i + 1;
                hex_into(s, &mut p, &mut buf)!;
                toks.push(Tok.{ kind: T_HEX, a: a, b: buf.len })!;
                i = p;
            } else if c == '[' {
                toks.push(Tok.{ kind: T_OPEN, a: 0, b: 0 })!;
                i += 1;
            } else if c == ']' {
                toks.push(Tok.{ kind: T_CLOSE, a: 0, b: 0 })!;
                i += 1;
            } else if is_regular(c) {
                let st = i;
                while (i < s.len) and is_regular(s[i]) {
                    i += 1;
                }
                let w = s[st..i];
                let mut kind = T_OTHER;
                if strs.eq(w, "endcodespacerange") {
                    kind = T_END_SPACE;
                } else if strs.eq(w, "endbfchar") {
                    kind = T_END_CHAR;
                } else if strs.eq(w, "endbfrange") {
                    kind = T_END_RANGE;
                }
                toks.push(Tok.{ kind: kind, a: 0, b: 0 })!;
            } else {
                i += 1;
            }
            if toks.len > (1 << 22) {
                break;
            }
        }
        let b = buf.bytes();
        let t = toks.items();
        let mut spaces = false;
        let mut k: usize = 0;
        let mut start: usize = 0;
        while k < t.len {
            let kind = t[k].kind;
            if kind == T_END_SPACE {
                // pairs of hex strings before it
                let mut j = start;
                while j + 1 < k {
                    if (t[j].kind == T_HEX) and (t[j + 1].kind == T_HEX) {
                        let len = t[j].b - t[j].a;
                        if (len >= 1) and (len <= 4) {
                            m.ranges[len - 1] = 1;
                            spaces = true;
                        }
                        j += 2;
                    } else {
                        j += 1;
                    }
                }
                start = k + 1;
            } else if kind == T_END_CHAR {
                let mut j = start;
                while j + 1 < k {
                    if (t[j].kind == T_HEX) and (t[j + 1].kind == T_HEX) {
                        let code = be_num(b[t[j].a..t[j].b]);
                        self.map_code(m, code, b[t[j + 1].a..t[j + 1].b], 0)!;
                        j += 2;
                    } else {
                        j += 1;
                    }
                }
                start = k + 1;
            } else if kind == T_END_RANGE {
                let mut j = start;
                while j + 2 < k {
                    if (t[j].kind != T_HEX) or (t[j + 1].kind != T_HEX) {
                        j += 1;
                        continue;
                    }
                    let lo = be_num(b[t[j].a..t[j].b]);
                    let hi = be_num(b[t[j + 1].a..t[j + 1].b]);
                    if (hi < lo) or (hi - lo > 65535) {
                        j += 3;
                        continue;
                    }
                    if t[j + 2].kind == T_HEX {
                        let dst = b[t[j + 2].a..t[j + 2].b];
                        let mut c = lo;
                        while c <= hi {
                            self.map_code(m, c, dst, c - lo)!;
                            if c == 0xffffffff {
                                break;
                            }
                            c += 1;
                        }
                        j += 3;
                    } else if t[j + 2].kind == T_OPEN {
                        let mut q = j + 3;
                        let mut c = lo;
                        while (q < k) and (t[q].kind == T_HEX) and (c <= hi) {
                            self.map_code(m, c, b[t[q].a..t[q].b], 0)!;
                            c += 1;
                            q += 1;
                        }
                        while (q < k) and (t[q].kind != T_CLOSE) {
                            q += 1;
                        }
                        j = q + 1;
                    } else {
                        j += 1;
                    }
                }
                start = k + 1;
            } else if kind == T_OTHER {
                // keywords like beginbfchar start a section
                start = k + 1;
            }
            k += 1;
        }
        if spaces and m.two_byte and (m.ranges[1] == 0) {
            // a Type0 font whose codes are not 2 bytes
            m.two_byte = false;
        }
        if not m.two_byte and not spaces {
            m.ranges[0] = 1;
        }
    }

    /// Maps a code to UTF-16BE text (plus `add` to its last unit, for ranges).
    fn map_code(self: *mut Extractor, m: *mut FontMap, code: u32, dst: []u8, add: u32) !void {
        let off = self.pool.len;
        let mut i: usize = 0;
        while i + 1 < dst.len {
            let mut u = (@as(u32, dst[i]) << 8) | @as(u32, dst[i + 1]);
            i += 2;
            if i + 1 >= dst.len {
                u +%= add;
            }
            if (u >= 0xd800) and (u < 0xdc00) and (i + 1 < dst.len) {
                let mut lo = (@as(u32, dst[i]) << 8) | @as(u32, dst[i + 1]);
                i += 2;
                if i + 1 >= dst.len {
                    lo +%= add;
                }
                if (lo >= 0xdc00) and (lo < 0xe000) {
                    u = 0x10000 + ((u - 0xd800) << 10) + (lo - 0xdc00);
                }
            }
            if ((u >= 0xd800) and (u < 0xe000)) or (u > 0x10ffff) {
                u = 0xfffd;
            }
            if u != 0 {
                self.pool.push_rune(u)!;
            }
        }
        let len = self.pool.len - off;
        if (len > 0) and (len < 65536) and (@as(u64, off) < (@as(u64, 1) << 40)) {
            m.uni.put(code, (@as(u64, off) << 16) | @as(u64, len))!;
        }
    }
}

pub const NO_FONT: usize = 0xffffffff;

const OP_NUM: u8 = 0;
const OP_STR: u8 = 1;
const OP_NAME: u8 = 2;
const OP_OPEN: u8 = 3;
const OP_CLOSE: u8 = 4;
const OP_OTHER: u8 = 5;

struct Operand {
    kind: u8,
    num: f64,
    a: usize,
    b: usize,
}

const T_HEX: u8 = 0;
const T_OPEN: u8 = 1;
const T_CLOSE: u8 = 2;
const T_OTHER: u8 = 3;
const T_END_SPACE: u8 = 4;
const T_END_CHAR: u8 = 5;
const T_END_RANGE: u8 = 6;

struct Tok {
    kind: u8,
    a: usize,
    b: usize,
}

fn be_num(b: []u8) u32 {
    let mut v: u32 = 0;
    for k in 0..@min(b.len, 4) {
        v = (v << 8) | @as(u32, b[k]);
    }
    return v;
}

/// a then b, as PDF matrices [a b c d e f].
fn mul(a: [6]f64, b: [6]f64) [6]f64 {
    return [
        (a[0] * b[0]) + (a[1] * b[2]),
        (a[0] * b[1]) + (a[1] * b[3]),
        (a[2] * b[0]) + (a[3] * b[2]),
        (a[2] * b[1]) + (a[3] * b[3]),
        (a[4] * b[0]) + (a[5] * b[2]) + b[4],
        (a[4] * b[1]) + (a[5] * b[3]) + b[5],
    ];
}

fn encoding_table(name: []u8, dflt: []u16) []u16 {
    if strs.eq(name, "WinAnsiEncoding") {
        return tb.WIN_ANSI[..];
    }
    if strs.eq(name, "MacRomanEncoding") {
        return tb.MAC_ROMAN[..];
    }
    if strs.eq(name, "StandardEncoding") {
        return tb.STANDARD[..];
    }
    return dflt;
}

/// A glyph name's character: the Adobe Glyph List, uniXXXX, uXXXX[XX].
pub fn glyph_unicode(name0: []u8) u32 {
    // drop a suffix: "a.sc", "f_i" keeps its first part
    let mut name = name0;
    let dot = strs.index_of_byte(name, '.') catch name.len;
    name = name[..dot];
    let mut lo: usize = 0;
    let mut hi: usize = tb.GLYPH_COUNT;
    while lo < hi {
        let mid = (lo + hi) / 2;
        let k = tb.GLYPH_NAMES[@as(usize, tb.GLYPH_OFFSET[mid])..@as(usize, tb.GLYPH_OFFSET[mid + 1])];
        let c = strs.compare(k, name);
        if c == 0 {
            return @as(u32, tb.GLYPH_UNICODE[mid]);
        }
        if c < 0 {
            lo = mid + 1;
        } else {
            hi = mid;
        }
    }
    let mut hex: []u8 = "";
    if strs.starts_with(name, "uni") and (name.len >= 7) {
        hex = name[3..7];
    } else if strs.starts_with(name, "u") and (name.len >= 5) and (name.len <= 7) {
        hex = name[1..];
    }
    if hex.len == 0 {
        return 0;
    }
    let mut v: u32 = 0;
    for c in hex {
        let h = hexval(c);
        if h < 0 {
            return 0;
        }
        v = (v << 4) | @bit_cast(u32, h);
    }
    if (v > 0x10ffff) or ((v >= 0xd800) and (v < 0xe000)) {
        return 0;
    }
    return v;
}

// -----------------------------------------------------------------------------
// Tests
// -----------------------------------------------------------------------------

/// A PDF from object bodies (object i + 1 is objs[i]), with a
/// cross-reference table; the catalog is object 1.
fn build(objs: [][]u8) ![]mut u8 {
    let mut out = strs.String.init();
    out.push_str("%PDF-1.7\n")!;
    let mut offs = vec.Vec.[usize].init();
    defer offs.deinit();
    for i in 0..objs.len {
        offs.push(out.len)!;
        put_dec(&mut out, i + 1)!;
        out.push_str(" 0 obj\n")!;
        out.push_str(objs[i])!;
        out.push_str("\nendobj\n")!;
    }
    let xref = out.len;
    out.push_str("xref\n0 ")!;
    put_dec(&mut out, objs.len + 1)!;
    out.push_str("\n0000000000 65535 f \n")!;
    for o in offs.items() {
        let mut d: [10]u8 = [0; 10];
        let mut v = o;
        let mut k: usize = 10;
        while k > 0 {
            k -= 1;
            d[k] = @truncate(u8, v % 10) + '0';
            v /= 10;
        }
        out.push_str(d[..])!;
        out.push_str(" 00000 n \n")!;
    }
    out.push_str("trailer\n<< /Size ")!;
    put_dec(&mut out, objs.len + 1)!;
    out.push_str(" /Root 1 0 R /Info << /Title (T\\351st) >> >>\nstartxref\n")!;
    put_dec(&mut out, xref)!;
    out.push_str("\n%%EOF\n")!;
    return out.take();
}

fn put_dec(out: *mut strs.String, v: usize) !void {
    if v >= 10 {
        put_dec(out, v / 10)!;
    }
    out.push(@truncate(u8, v % 10) + '0')!;
}

fn stream_obj(dict: []u8, data: []u8) ![]mut u8 {
    let mut s = strs.String.init();
    s.push_str("<< ")!;
    s.push_str(dict)!;
    s.push_str(" /Length ")!;
    put_dec(&mut s, data.len)!;
    s.push_str(" >>\nstream\n")!;
    s.push_str(data)!;
    s.push_str("\nendstream")!;
    return s.take();
}

fn page_text_of(pdf: []u8, i: usize) ![]mut u8 {
    let mut r = Reader.open(pdf)!;
    defer r.deinit();
    return r.page_text(i);
}

let CMAP: []u8 =
    "/CIDInit /ProcSet findresource begin 12 dict begin begincmap\n1 begincodespacerange <0000> <FFFF> endcodespacerange\n2 beginbfchar <0001> <0048> <0002> <0069> endbfchar\n1 beginbfrange <0003> <0004> [<0041> <D83DDE00>] endbfrange\n1 beginbfrange <0010> <0012> <0061> endbfrange\nendcmap end end";

test "simple fonts, encodings and ToUnicode maps" {
    let content =
        "BT /F1 12 Tf 72 700 Td (Caf\\351 \\223ok\\224) Tj ET\nBT /F2 12 Tf 72 680 Td (ABCD) Tj ET\nBT /F3 12 Tf 72 660 Td <00010002> Tj [<0003> -2000 <0004>] TJ 0 -20 Td <001000110012> Tj ET";
    let c6 = stream_obj("", content)!;
    defer mem.free(c6);
    let c8 = stream_obj("", CMAP)!;
    defer mem.free(c8);
    let objs: [8][]u8 = [
        "<< /Type /Catalog /Pages 2 0 R >>",
        "<< /Type /Pages /Kids [3 0 R] /Count 1 >>",
        "<< /Type /Page /Parent 2 0 R /MediaBox [0 0 612 792] /Resources << /Font << /F1 4 0 R /F2 5 0 R /F3 7 0 R >> >> /Contents 6 0 R >>",
        "<< /Type /Font /Subtype /Type1 /BaseFont /Helvetica /Encoding /WinAnsiEncoding >>",
        "<< /Type /Font /Subtype /Type1 /BaseFont /X /Encoding << /Differences [65 /Alpha /uni263A /fi] >> >>",
        c6,
        "<< /Type /Font /Subtype /Type0 /BaseFont /Y /Encoding /Identity-H /DescendantFonts [<< /Type /Font /Subtype /CIDFontType2 /DW 1000 >>] /ToUnicode 8 0 R >>",
        c8,
    ];
    let pdf = build(objs[..])!;
    defer mem.free(pdf);
    let mut r = Reader.open(pdf)!;
    defer r.deinit();
    @expect_eq(r.page_count(), 1);
    let sz = r.page_size(0)!;
    @expect_eq(sz[0], 612.0);
    let title = r.info("Title")!;
    defer mem.free(title);
    @expect(strs.eq(title, "T\xc3\xa9st"));
    let t = r.page_text(0)!;
    defer mem.free(t);
    @expect(strs.eq(t,
                    "Caf\xc3\xa9 \xe2\x80\x9cok\xe2\x80\x9d\n\xce\x91\xe2\x98\xba\xef\xac\x81D\nHiA \xf0\x9f\x98\x80\nabc"));
}

test "object streams and compressed cross-reference streams" {
    // objects 3 (the page) and 4 (a font) live in object stream 5
    let o3 =
        "<< /Type /Page /Parent 2 0 R /MediaBox [0 0 200 100] /Resources << /Font << /F1 4 0 R >> >> /Contents 6 0 R >>";
    let o4 = "<< /Type /Font /Subtype /Type1 /BaseFont /Helvetica >>";
    let mut hdr = strs.String.init();
    defer hdr.deinit();
    hdr.push_str("3 0 4 ")!;
    put_dec(&mut hdr, o3.len + 1)!;
    hdr.push(' ')!;
    let mut body = strs.String.init();
    defer body.deinit();
    body.push_str(hdr.bytes())!;
    body.push_str(o3)!;
    body.push(' ')!;
    body.push_str(o4)!;
    let packed = deflate.compress(body.bytes(), deflate.ZLIB, 6)!;
    defer mem.free(packed);
    let mut sdict = strs.String.init();
    defer sdict.deinit();
    sdict.push_str("/Type /ObjStm /N 2 /Filter /FlateDecode /First ")!;
    put_dec(&mut sdict, hdr.len)!;
    let content = "BT /F1 10 Tf 10 50 Td (packed) Tj ET";
    let cpacked = deflate.compress(content, deflate.ZLIB, 6)!;
    defer mem.free(cpacked);
    let mut out = strs.String.init();
    defer out.deinit();
    out.push_str("%PDF-1.5\n")!;
    let mut offs: [7]usize = [0; 7];
    offs[1] = out.len;
    out.push_str("1 0 obj << /Type /Catalog /Pages 2 0 R >> endobj\n")!;
    offs[2] = out.len;
    out.push_str("2 0 obj << /Type /Pages /Kids [3 0 R] /Count 1 >> endobj\n")!;
    offs[5] = out.len;
    let o5 = stream_obj(sdict.bytes(), packed)!;
    defer mem.free(o5);
    out.push_str("5 0 obj ")!;
    out.push_str(o5)!;
    out.push_str(" endobj\n")!;
    offs[6] = out.len;
    let o6 = stream_obj("/Filter /FlateDecode", cpacked)!;
    defer mem.free(o6);
    out.push_str("6 0 obj ")!;
    out.push_str(o6)!;
    out.push_str(" endobj\n")!;
    // the cross-reference stream, rows of W [1 4 2], PNG "Up" predicted
    let mut rows = strs.String.init();
    defer rows.deinit();
    let mut prev: [7]u8 = [0; 7];
    for n in @as(usize, 0)..8 {
        let mut row: [7]u8 = [0; 7];
        if (n == 1) or (n == 2) or (n == 5) or (n == 6) {
            row[0] = 1;
            let v = offs[n];
            row[1] = @truncate(u8, v >> 24);
            row[2] = @truncate(u8, v >> 16);
            row[3] = @truncate(u8, v >> 8);
            row[4] = @truncate(u8, v);
        } else if (n == 3) or (n == 4) {
            row[0] = 2;
            row[4] = 5;
            row[6] = @truncate(u8, n - 3);
        } else if n == 7 {
            row[0] = 1;
            let v = out.len;
            row[1] = @truncate(u8, v >> 24);
            row[2] = @truncate(u8, v >> 16);
            row[3] = @truncate(u8, v >> 8);
            row[4] = @truncate(u8, v);
        }
        rows.push(2)!;
        for k in @as(usize, 0)..7 {
            rows.push(row[k] -% prev[k])!;
        }
        prev = row;
    }
    let xpacked = deflate.compress(rows.bytes(), deflate.ZLIB, 6)!;
    defer mem.free(xpacked);
    let xref_at = out.len;
    let o7 =
        stream_obj("/Type /XRef /Size 8 /W [1 4 2] /Root 1 0 R /Filter /FlateDecode /DecodeParms << /Predictor 12 /Columns 7 >>",
                   xpacked)!;
    defer mem.free(o7);
    out.push_str("7 0 obj ")!;
    out.push_str(o7)!;
    out.push_str(" endobj\nstartxref\n")!;
    put_dec(&mut out, xref_at)!;
    out.push_str("\n%%EOF\n")!;
    let mut r = Reader.open(out.bytes())!;
    defer r.deinit();
    @expect_eq(r.page_count(), 1);
    @expect_eq(r.xref.get(4).kind, 2);
    let t = r.page_text(0)!;
    defer mem.free(t);
    @expect(strs.eq(t, "packed"));
    let sz = r.page_size(0)!;
    @expect_eq(sz[1], 100.0);
}

test "filters" {
    let a = ascii85("87cURD]i,\"Ebo8;+AbHq+T~>", 1000)!;
    defer mem.free(a);
    @expect(strs.eq(a, "Hello World, PDF!"));
    let rl = run_length("\x02abc\xfdz\x80", 1000)!;
    defer mem.free(rl);
    @expect(strs.eq(rl, "abczzzz"));
    // the example from the PDF specification
    let lz = lzw_decode("\x80\x0b\x60\x50\x22\x0c\x0c\x85\x01", true, 1000)!;
    defer mem.free(lz);
    @expect(strs.eq(lz, "-----A---B"));
    // decoding stops at the size limit
    @expect(failed(run_length("\x81x\x81x\x81x", 200)));
    let mut out = strs.String.init();
    defer out.deinit();
    let mut p: usize = 1;
    hex_into("<48 65 6c6C6f 7>", &mut p, &mut out)!;
    @expect(strs.eq(out.bytes(), "Hellop"));
    @expect_eq(glyph_unicode("Euro"), 0x20ac);
    @expect_eq(glyph_unicode("uni00E9"), 0xe9);
    @expect_eq(glyph_unicode("u1F600"), 0x1f600);
    @expect_eq(glyph_unicode("a.sc"), 'a');
    @expect_eq(glyph_unicode("nonsense"), 0);
}

fn failed(r: ![]mut u8) bool {
    let v = r catch return true;
    mem.free(v);
    return false;
}

fn simple_pdf(content: []u8) ![]mut u8 {
    let c = stream_obj("", content)!;
    defer mem.free(c);
    let objs: [5][]u8 = [
        "<< /Type /Catalog /Pages 2 0 R >>",
        "<< /Type /Pages /Kids [3 0 R] /Count 1 /Resources << /Font << /F1 4 0 R >> >> >>",
        "<< /Type /Page /Parent 2 0 R /MediaBox [0 0 612 792] /Contents 5 0 R >>",
        "<< /Type /Font /Subtype /Type1 /BaseFont /Helvetica >>",
        c,
    ];
    return build(objs[..]);
}

test "damaged files are repaired" {
    let pdf = simple_pdf("BT /F1 12 Tf 72 700 Td (repaired) Tj ET")!;
    defer mem.free(pdf);
    // no cross-reference table at all
    let x = strs.last_index_of(pdf, "xref") catch 0;
    let t1 = page_text_of(pdf[..x], 0)!;
    defer mem.free(t1);
    @expect(strs.eq(t1, "repaired"));
    // a table pointing at the wrong places
    let bad = strs.dup(pdf)!;
    defer mem.free(bad);
    let mut k = strs.last_index_of(bad, "xref") catch 0;
    while k + 18 <= bad.len {
        if strs.starts_with(bad[k + 10..], " 00000 n") {
            @mem_copy(bad[k..k + 10], "0000000001");
        }
        k += 1;
    }
    let t2 = page_text_of(bad, 0)!;
    defer mem.free(t2);
    @expect(strs.eq(t2, "repaired"));
}

test "hostile structures" {
    // deep nesting, self references, a page tree that loops, huge counts
    let mut deep = strs.String.init();
    defer deep.deinit();
    for _k in @as(usize, 0)..100000 {
        deep.push('[')!;
    }
    let objs: [6][]u8 = [
        "<< /Type /Catalog /Pages 2 0 R >>",
        "<< /Type /Pages /Kids [3 0 R 2 0 R 6 0 R] /Count 99999999999 >>",
        "<< /Type /Page /Parent 2 0 R /Contents 5 0 R /Resources << /Font << /F1 4 0 R >> /XObject << /X 5 0 R >> >> >>",
        "4 0 R",
        "<< /Length 5 0 R /Subtype /Form /Resources << /XObject << /X 5 0 R >> >> >>\nstream\nBT /F1 9 Tf (loop) Tj ET /X Do\nendstream",
        deep.bytes(),
    ];
    let pdf = build(objs[..])!;
    defer mem.free(pdf);
    let mut r = Reader.open(pdf)!;
    defer r.deinit();
    @expect_eq(r.page_count(), 1);
    let t = r.page_text(0)!;
    defer mem.free(t);
    @expect(r.resolve(r.object(4)!)!.is_null());
    // mutations of a good file never trap
    let good = simple_pdf("BT /F1 12 Tf 72 700 Td (fuzz) Tj ET q 1 0 0 1 5 5 cm Q")!;
    defer mem.free(good);
    let mut buf = mem.alloc.[u8](good.len)!;
    defer mem.free(buf);
    let mut seed: u64 = 0x9e3779b97f4a7c15;
    for _round in @as(usize, 0)..300 {
        @mem_copy(buf, good);
        for _m in @as(usize, 0)..4 {
            seed ^= seed << 13;
            seed ^= seed >> 7;
            seed ^= seed << 17;
            let at_ = @truncate(usize, seed % @as(u64, buf.len));
            buf[at_] = @truncate(u8, seed >> 32);
        }
        let mut rr = Reader.open(buf) catch continue;
        for i in 0..rr.page_count() {
            let pt = rr.page_text(i) catch continue;
            mem.free(pt);
        }
        rr.deinit();
    }
}
