// =============================================================================
// std/protobuf.ph - Protocol Buffers: the wire format.
//
//     let mut e = protobuf.Encoder.init();              // writing
//     defer e.deinit();
//     e.uint64(1, 150)!;
//     e.string(2, "testing")!;
//     e.begin(3)!;                                      // a nested message
//     e.sint32(1, -5)!;
//     e.end()!;
//     let bytes = e.bytes();
//
//     let mut r = protobuf.Reader.init(bytes);          // reading
//     loop {
//         let f = r.next() catch |err| { if err == error.Done { break; } return err; };
//         if f.number == 2 { name = f.as_string()!; }
//         if f.number == 3 { let mut inner = f.message()!; ... }
//     }
//
// This is the encoding without schemas (and so without code generation):
// Encoder writes fields of every scalar type (int32/64, uint32/64,
// sint32/64 with zigzag, bool, enums, fixed and sfixed 32/64, float,
// double, bytes, string), nested messages and packed repeated fields;
// Reader yields fields with the conversions for each type, finds fields
// (`last`, for the protobuf rule that the last one wins), and walks packed
// values. decode_raw prints any message as `protoc --decode_raw` does.
//
// Reading is safe on untrusted input: varints over ten bytes, lengths past
// the end, unmatched groups and invalid wire types are errors.
// =============================================================================

import "fmt.ph" as fmt;
import "fs.ph" as fs;
import "hex.ph" as hex;
import "io.ph" as io;
import "mem.ph" as mem;
import "str.ph" as strs;
import "vec.ph" as vec;

error { Done, Truncated, Invalid, Overflow, WrongType, TooDeep, NotFound, OutOfMemory }

// Wire types
pub const VARINT: u8 = 0;
pub const I64: u8 = 1;
pub const LEN: u8 = 2;
pub const SGROUP: u8 = 3;  // (groups are deprecated; Reader skips over them)
pub const EGROUP: u8 = 4;
pub const I32: u8 = 5;

/// The largest field number.
pub const MAX_FIELD: u32 = 536870911;

/// Groups nested deeper than this are refused.
pub const MAX_DEPTH: u32 = 100;

pub fn zigzag32(v: i32) u32 {
    return @bit_cast(u32, (v << 1) ^ (v >> 31));
}

pub fn zigzag64(v: i64) u64 {
    return @bit_cast(u64, (v << 1) ^ (v >> 63));
}

pub fn unzigzag32(u: u32) i32 {
    return @bit_cast(i32, (u >> 1) ^ (0 -% (u & 1)));
}

pub fn unzigzag64(u: u64) i64 {
    return @bit_cast(i64, (u >> 1) ^ (0 -% (u & 1)));
}

/// Appends v as a varint.
pub fn put_varint(out: *mut strs.String, v: u64) !void {
    let mut buf: [10]u8 = [0; 10];
    let mut n: usize = 0;
    let mut x = v;
    while x >= 0x80 {
        buf[n] = @truncate(u8, x) | 0x80;
        x >>= 7;
        n += 1;
    }
    buf[n] = @truncate(u8, x);
    out.push_str(buf[..n + 1])!;
}

/// Reads a varint at s[at.*..], advancing at.
pub fn read_varint(s: []u8, at: *mut usize) !u64 {
    let mut v: u64 = 0;
    let mut shift: u64 = 0;
    let mut i = at.*;
    loop {
        if i >= s.len {
            return error.Truncated;
        }
        let b = s[i];
        i += 1;
        if (shift == 63) and (b > 1) {
            return error.Overflow;
        }
        v |= @as(u64, b & 0x7f) << shift;
        if (b & 0x80) == 0 {
            break;
        }
        shift += 7;
        if shift > 63 {
            return error.Overflow;
        }
    }
    at.* = i;
    return v;
}

// -----------------------------------------------------------------------------
// Writing
// -----------------------------------------------------------------------------

/// Builds a message in memory (nested messages need their lengths, which
/// end() fills in).
pub struct Encoder {
    buf: strs.String,
    open: vec.Vec.[usize],  // where each unfinished nested message's content starts

    pub fn init() Encoder {
        return Encoder.{ buf: strs.String.init(), open: vec.Vec.[usize].init() };
    }

    pub fn deinit(self: *mut Encoder) void {
        self.buf.deinit();
        self.open.deinit();
    }

    /// The message so far (valid until the next change).
    pub fn bytes(self: *Encoder) []u8 {
        return self.buf.bytes();
    }

    /// The message, which the caller then owns (the Encoder is left empty).
    pub fn take(self: *mut Encoder) ![]mut u8 {
        self.open.clear();
        return self.buf.take();
    }

    pub fn key(self: *mut Encoder, field: u32, wire: u8) !void {
        if (field == 0) or (field > MAX_FIELD) {
            return error.Invalid;
        }
        return put_varint(&mut self.buf, (@as(u64, field) << 3) | @as(u64, wire));
    }

    pub fn uint64(self: *mut Encoder, field: u32, v: u64) !void {
        self.key(field, VARINT)!;
        put_varint(&mut self.buf, v)!;
    }

    pub fn uint32(self: *mut Encoder, field: u32, v: u32) !void {
        return self.uint64(field, @as(u64, v));
    }

    /// int64 (negative values take ten bytes: use sint64 for those).
    pub fn int64(self: *mut Encoder, field: u32, v: i64) !void {
        return self.uint64(field, @bit_cast(u64, v));
    }

    pub fn int32(self: *mut Encoder, field: u32, v: i32) !void {
        return self.uint64(field, @bit_cast(u64, @as(i64, v)));
    }

    pub fn sint32(self: *mut Encoder, field: u32, v: i32) !void {
        return self.uint64(field, @as(u64, zigzag32(v)));
    }

    pub fn sint64(self: *mut Encoder, field: u32, v: i64) !void {
        return self.uint64(field, zigzag64(v));
    }

    pub fn boolean(self: *mut Encoder, field: u32, v: bool) !void {
        let mut x: u64 = 0;
        if v {
            x = 1;
        }
        return self.uint64(field, x);
    }

    pub fn enumeration(self: *mut Encoder, field: u32, v: i32) !void {
        return self.int32(field, v);
    }

    fn little(self: *mut Encoder, v: u64, n: usize) !void {
        let mut b: [8]u8 = [0; 8];
        for k in @as(usize, 0)..n {
            b[k] = @truncate(u8, v >> @as(u64, 8 * k));
        }
        self.buf.push_str(b[..n])!;
    }

    pub fn fixed32(self: *mut Encoder, field: u32, v: u32) !void {
        self.key(field, I32)!;
        self.little(@as(u64, v), 4)!;
    }

    pub fn fixed64(self: *mut Encoder, field: u32, v: u64) !void {
        self.key(field, I64)!;
        self.little(v, 8)!;
    }

    pub fn sfixed32(self: *mut Encoder, field: u32, v: i32) !void {
        return self.fixed32(field, @bit_cast(u32, v));
    }

    pub fn sfixed64(self: *mut Encoder, field: u32, v: i64) !void {
        return self.fixed64(field, @bit_cast(u64, v));
    }

    pub fn float(self: *mut Encoder, field: u32, v: f32) !void {
        return self.fixed32(field, @bit_cast(u32, v));
    }

    pub fn double(self: *mut Encoder, field: u32, v: f64) !void {
        return self.fixed64(field, @bit_cast(u64, v));
    }

    pub fn bytes_field(self: *mut Encoder, field: u32, b: []u8) !void {
        self.key(field, LEN)!;
        put_varint(&mut self.buf, @as(u64, b.len))!;
        self.buf.push_str(b)!;
    }

    pub fn string(self: *mut Encoder, field: u32, s: []u8) !void {
        return self.bytes_field(field, s);
    }

    /// Starts a nested message (fields follow, then end()).
    pub fn begin(self: *mut Encoder, field: u32) !void {
        self.key(field, LEN)!;
        self.open.push(self.buf.len)!;
    }

    /// Ends the innermost nested message, inserting its length.
    pub fn end(self: *mut Encoder) !void {
        let st = self.open.pop() catch return error.Invalid;
        let n = self.buf.len - st;
        let mut tmp = strs.String.init();
        defer tmp.deinit();
        put_varint(&mut tmp, @as(u64, n))!;
        let k = tmp.len;
        self.buf.reserve(k)!;
        let b = self.buf.buf;
        let mut i = self.buf.len;
        while i > st {
            i -= 1;
            b[i + k] = b[i];
        }
        @mem_copy(b[st..st + k], tmp.bytes());
        self.buf.len += k;
    }

    /// A packed repeated field of varints (int32/64, uint32/64, bool, enum).
    pub fn packed_varints(self: *mut Encoder, field: u32, vs: []u64) !void {
        self.begin(field)!;
        for v in vs {
            put_varint(&mut self.buf, v)!;
        }
        self.end()!;
    }

    /// A packed repeated sint64 field.
    pub fn packed_sint64(self: *mut Encoder, field: u32, vs: []i64) !void {
        self.begin(field)!;
        for v in vs {
            put_varint(&mut self.buf, zigzag64(v))!;
        }
        self.end()!;
    }

    pub fn packed_fixed32(self: *mut Encoder, field: u32, vs: []u32) !void {
        self.begin(field)!;
        for v in vs {
            self.little(@as(u64, v), 4)!;
        }
        self.end()!;
    }

    pub fn packed_fixed64(self: *mut Encoder, field: u32, vs: []u64) !void {
        self.begin(field)!;
        for v in vs {
            self.little(v, 8)!;
        }
        self.end()!;
    }

    pub fn packed_double(self: *mut Encoder, field: u32, vs: []f64) !void {
        self.begin(field)!;
        for v in vs {
            self.little(@bit_cast(u64, v), 8)!;
        }
        self.end()!;
    }

    /// Bytes that are already encoded fields.
    pub fn raw(self: *mut Encoder, b: []u8) !void {
        self.buf.push_str(b)!;
    }
}

// -----------------------------------------------------------------------------
// Reading
// -----------------------------------------------------------------------------

pub struct Field {
    number: u32,
    wire: u8,
    bits: u64,   // VARINT, I64 and I32 values
    data: []u8,  // LEN contents (and a group's contents)

    fn want(self: Field, w: u8) !void {
        if self.wire != w {
            return error.WrongType;
        }
    }

    pub fn as_u64(self: Field) !u64 {
        self.want(VARINT)!;
        return self.bits;
    }

    pub fn as_i64(self: Field) !i64 {
        self.want(VARINT)!;
        return @bit_cast(i64, self.bits);
    }

    /// int32 (as protobuf does, keeping the low 32 bits).
    pub fn as_i32(self: Field) !i32 {
        self.want(VARINT)!;
        return @bit_cast(i32, @truncate(u32, self.bits));
    }

    pub fn as_u32(self: Field) !u32 {
        self.want(VARINT)!;
        return @truncate(u32, self.bits);
    }

    pub fn as_sint32(self: Field) !i32 {
        self.want(VARINT)!;
        return unzigzag32(@truncate(u32, self.bits));
    }

    pub fn as_sint64(self: Field) !i64 {
        self.want(VARINT)!;
        return unzigzag64(self.bits);
    }

    pub fn as_bool(self: Field) !bool {
        self.want(VARINT)!;
        return self.bits != 0;
    }

    pub fn as_fixed32(self: Field) !u32 {
        self.want(I32)!;
        return @truncate(u32, self.bits);
    }

    pub fn as_fixed64(self: Field) !u64 {
        self.want(I64)!;
        return self.bits;
    }

    pub fn as_sfixed32(self: Field) !i32 {
        return @bit_cast(i32, self.as_fixed32()!);
    }

    pub fn as_sfixed64(self: Field) !i64 {
        return @bit_cast(i64, self.as_fixed64()!);
    }

    pub fn as_float(self: Field) !f32 {
        return @bit_cast(f32, self.as_fixed32()!);
    }

    pub fn as_double(self: Field) !f64 {
        return @bit_cast(f64, self.as_fixed64()!);
    }

    pub fn as_bytes(self: Field) ![]u8 {
        self.want(LEN)!;
        return self.data;
    }

    /// A string field (which must be UTF-8).
    pub fn as_string(self: Field) ![]u8 {
        self.want(LEN)!;
        if not strs.valid_utf8(self.data) {
            return error.Invalid;
        }
        return self.data;
    }

    /// A nested message's fields.
    pub fn message(self: Field) !Reader {
        if (self.wire != LEN) and (self.wire != SGROUP) {
            return error.WrongType;
        }
        return Reader.init(self.data);
    }

    /// The values of a packed varint field (or of a single unpacked one).
    pub fn varints(self: Field) !Varints {
        if self.wire == VARINT {
            return Varints.{ s: "", i: 0, single: true, one: self.bits };
        }
        self.want(LEN)!;
        return Varints.{ s: self.data, i: 0, single: false, one: 0 };
    }

    /// The values of a packed fixed-size field: 4 or 8 bytes each.
    pub fn fixed(self: Field, size: usize) !Fixed {
        self.want(LEN)!;
        if ((size != 4) and (size != 8)) or ((self.data.len % size) != 0) {
            return error.Invalid;
        }
        return Fixed.{ s: self.data, i: 0, size: size };
    }
}

pub struct Varints {
    s: []u8,
    i: usize,
    single: bool,
    one: u64,

    pub fn next(self: *mut Varints) !u64 {
        if self.single {
            self.single = false;
            self.s = "";
            return self.one;
        }
        if self.i >= self.s.len {
            return error.Done;
        }
        return read_varint(self.s, &mut self.i);
    }
}

pub struct Fixed {
    s: []u8,
    i: usize,
    size: usize,

    /// The next value's bits (bit_cast them for floats and signed types).
    pub fn next(self: *mut Fixed) !u64 {
        if self.i >= self.s.len {
            return error.Done;
        }
        let mut v: u64 = 0;
        for k in @as(usize, 0)..self.size {
            v |= @as(u64, self.s[self.i + k]) << @as(u64, 8 * k);
        }
        self.i += self.size;
        return v;
    }
}

/// Reads the fields of a message in order.
pub struct Reader {
    s: []u8,
    i: usize,

    pub fn init(bytes: []u8) Reader {
        return Reader.{ s: bytes, i: 0 };
    }

    /// The next field, or error.Done at the end.
    pub fn next(self: *mut Reader) !Field {
        if self.i >= self.s.len {
            return error.Done;
        }
        let k = read_varint(self.s, &mut self.i)!;
        let num = k >> 3;
        let wire = @truncate(u8, k & 7);
        if (num == 0) or (num > @as(u64, MAX_FIELD)) {
            return error.Invalid;
        }
        let mut f = Field.{ number: @truncate(u32, num), wire: wire, bits: 0, data: "" };
        if wire == VARINT {
            f.bits = read_varint(self.s, &mut self.i)!;
        } else if (wire == I64) or (wire == I32) {
            let mut n: usize = 4;
            if wire == I64 {
                n = 8;
            }
            if n > (self.s.len - self.i) {
                return error.Truncated;
            }
            for j in @as(usize, 0)..n {
                f.bits |= @as(u64, self.s[self.i + j]) << @as(u64, 8 * j);
            }
            self.i += n;
        } else if wire == LEN {
            let n = read_varint(self.s, &mut self.i)!;
            if n > @as(u64, self.s.len - self.i) {
                return error.Truncated;
            }
            let m = @truncate(usize, n);
            f.data = self.s[self.i..self.i + m];
            self.i += m;
        } else if wire == SGROUP {
            let st = self.i;
            let end = self.skip_group(f.number)!;
            f.data = self.s[st..end];
        } else {
            return error.Invalid;  // (a stray end-group, or wire types 6 and 7)
        }
        return f;
    }

    // Skips to the end of a group: returns where its contents end.
    fn skip_group(self: *mut Reader, number: u32) !usize {
        let mut open = vec.Vec.[u32].init();
        defer open.deinit();
        open.push(number)!;
        loop {
            let at = self.i;
            if self.i >= self.s.len {
                return error.Truncated;
            }
            let k = read_varint(self.s, &mut self.i)!;
            let num = @truncate(u32, k >> 3);
            let wire = @truncate(u8, k & 7);
            if wire == EGROUP {
                if num != open.items()[open.len - 1] {
                    return error.Invalid;
                }
                open.truncate(open.len - 1);
                if open.len == 0 {
                    return at;
                }
                continue;
            }
            if wire == SGROUP {
                if open.len >= @as(usize, MAX_DEPTH) {
                    return error.TooDeep;
                }
                open.push(num)!;
                continue;
            }
            self.i = at;
            _ = self.next()!;
        }
    }

    /// The last field with this number (for singular fields, the last one
    /// wins), or error.NotFound.
    pub fn last(self: *Reader, number: u32) !Field {
        let mut r = Reader.init(self.s);
        let mut found = false;
        let mut out = Field.{ number: 0, wire: 0, bits: 0, data: "" };
        loop {
            let f = r.next() catch |e| {
                if e == error.Done {
                    break;
                }
                return e;
            };
            if f.number == number {
                out = f;
                found = true;
            }
        }
        if not found {
            return error.NotFound;
        }
        return out;
    }

    /// How many fields have this number.
    pub fn count(self: *Reader, number: u32) !usize {
        let mut r = Reader.init(self.s);
        let mut n: usize = 0;
        loop {
            let f = r.next() catch |e| {
                if e == error.Done {
                    break;
                }
                return e;
            };
            if f.number == number {
                n += 1;
            }
        }
        return n;
    }
}

/// Whether bytes parse completely as a message (as decode_raw guesses).
pub fn is_message(bytes: []u8) bool {
    let mut r = Reader.init(bytes);
    loop {
        _ = r.next() catch |e| {
            return e == error.Done;
        };
    }
}

// -----------------------------------------------------------------------------
// decode_raw
// -----------------------------------------------------------------------------

/// Writes a message's fields as `protoc --decode_raw` does: numbers, with
/// length-delimited fields shown as nested messages when they parse as
/// one, else as strings.
pub fn decode_raw(out: io.Writer, bytes: []u8) !void {
    return raw_fields(out, bytes, 0);
}

pub fn decode_raw_alloc(bytes: []u8) ![]mut u8 {
    let mut s = strs.String.init();
    decode_raw(s.writer(), bytes) catch |e| {
        s.deinit();
        return e;
    };
    return s.take();
}

fn indent(out: io.Writer, depth: u32) !void {
    for _k in @as(u32, 0)..depth {
        out.write_all("  ")!;
    }
}

fn raw_fields(out: io.Writer, bytes: []u8, depth: u32) !void {
    if depth > MAX_DEPTH {
        return error.TooDeep;
    }
    let mut r = Reader.init(bytes);
    loop {
        let f = r.next() catch |e| {
            if e == error.Done {
                return;
            }
            return e;
        };
        indent(out, depth)!;
        if (f.wire == LEN) or (f.wire == SGROUP) {
            if (f.data.len > 0) and ((f.wire == SGROUP) or is_message(f.data)) and (depth < MAX_DEPTH) {
                fmt.write(out, "{} {{\n", [fmt.int(f.number)])!;
                raw_fields(out, f.data, depth + 1)!;
                indent(out, depth)!;
                out.write_all("}\n")!;
                continue;
            }
            fmt.write(out, "{}: \"", [fmt.int(f.number)])!;
            escape_c(out, f.data)!;
            out.write_all("\"\n")!;
        } else if f.wire == VARINT {
            fmt.write(out, "{}: {}\n", [fmt.int(f.number), fmt.int(f.bits)])!;
        } else if f.wire == I32 {
            fmt.write(out, "{}: 0x{:08x}\n", [fmt.int(f.number), fmt.int(f.bits)])!;
        } else {
            fmt.write(out, "{}: 0x{:016x}\n", [fmt.int(f.number), fmt.int(f.bits)])!;
        }
    }
}

// Bytes with C escapes (as protoc prints them).
fn escape_c(out: io.Writer, s: []u8) !void {
    for c in s {
        if c == '"' {
            out.write_all("\\\"")!;
        } else if c == '\\' {
            out.write_all("\\\\")!;
        } else if c == '\n' {
            out.write_all("\\n")!;
        } else if c == '\r' {
            out.write_all("\\r")!;
        } else if c == '\t' {
            out.write_all("\\t")!;
        } else if (c < 0x20) or (c >= 0x7f) {
            let d: [4]u8 = ['\\', '0' + (c >> 6), '0' + ((c >> 3) & 7), '0' + (c & 7)];
            out.write_all(d[..])!;
        } else {
            let b: [1]u8 = [c];
            out.write_all(b[..])!;
        }
    }
}

// -----------------------------------------------------------------------------
// Tests
// -----------------------------------------------------------------------------

test "the encoding guide's examples" {
    let mut e = Encoder.init();
    defer e.deinit();
    e.uint64(1, 150)!;
    @expect_eq(e.bytes(), "\x08\x96\x01");
    e.string(2, "testing")!;
    e.begin(3)!;
    e.uint64(1, 150)!;
    e.end()!;
    let packed: [3]u64 = [3, 270, 86942];
    e.packed_varints(4, packed[..])!;
    @expect_eq(e.bytes(), "\x08\x96\x01\x12\x07testing\x1a\x03\x08\x96\x01\x22\x06\x03\x8e\x02\x9e\xa7\x05");
    let raw = decode_raw_alloc(e.bytes())!;
    defer mem.free(raw);
    // (the packed field happens to parse as a message too, as protoc also shows)
    @expect_eq(raw, "1: 150\n2: \"testing\"\n3 {\n  1: 150\n}\n4: \"\\003\\216\\002\\236\\247\\005\"\n");
    let mut r = Reader.init(e.bytes());
    @expect_eq(r.last(2)!.as_string()!, "testing");
    let mut inner = r.last(3)!.message()!;
    @expect_eq(inner.next()!.as_u64()!, 150);
    let mut vs = r.last(4)!.varints()!;
    let mut sum: u64 = 0;
    while true {
        sum += vs.next() catch break;
    }
    @expect_eq(sum, 3 + 270 + 86942);
    @expect_eq(r.count(1)!, 1);
    @expect(fails_field(r.last(9)));
    // deeper nesting fills in every length
    let mut d = Encoder.init();
    defer d.deinit();
    for _k in @as(usize, 0)..3 {
        d.begin(1)!;
    }
    d.string(2, "x")!;
    for _k in @as(usize, 0)..3 {
        d.end()!;
    }
    @expect_eq(d.bytes(), "\x0a\x07\x0a\x05\x0a\x03\x12\x01x");
}

fn fails_field(r: !Field) bool {
    _ = r catch return true;
    return false;
}

test "every scalar type, against the protobuf library" {
    let data = fs.read_file("std/protobuf/testdata/scalars.txt") catch return;
    defer mem.free(data);
    let mut lines = strs.lines(data);
    _ = lines.next()!;
    let mut n: usize = 0;
    while true {
        let line = lines.next() catch break;
        let mut f = strs.fields(line);
        let typ = f.next()!;
        let num = fmt.parse_int.[u32](f.next()!)!;
        let val = f.next()!;
        let want = hex.decode_alloc(f.next()!)!;
        defer mem.free(want);
        let mut e = Encoder.init();
        defer e.deinit();
        let mut r = Reader.init(want);
        let fld = r.next()!;
        @expect_eq(fld.number, num);
        if strs.eq(typ, "int32") {
            let v = fmt.parse_int.[i32](val)!;
            e.int32(num, v)!;
            @expect_eq(fld.as_i32()!, v);
        } else if strs.eq(typ, "int64") {
            let v = fmt.parse_int.[i64](val)!;
            e.int64(num, v)!;
            @expect_eq(fld.as_i64()!, v);
        } else if strs.eq(typ, "uint32") {
            let v = fmt.parse_int.[u32](val)!;
            e.uint32(num, v)!;
            @expect_eq(fld.as_u32()!, v);
        } else if strs.eq(typ, "uint64") {
            let v = fmt.parse_int.[u64](val)!;
            e.uint64(num, v)!;
            @expect_eq(fld.as_u64()!, v);
        } else if strs.eq(typ, "sint32") {
            let v = fmt.parse_int.[i32](val)!;
            e.sint32(num, v)!;
            @expect_eq(fld.as_sint32()!, v);
        } else if strs.eq(typ, "sint64") {
            let v = fmt.parse_int.[i64](val)!;
            e.sint64(num, v)!;
            @expect_eq(fld.as_sint64()!, v);
        } else if strs.eq(typ, "bool") {
            let v = strs.eq(val, "1");
            e.boolean(num, v)!;
            @expect_eq(fld.as_bool()!, v);
        } else if strs.eq(typ, "fixed32") {
            let v = fmt.parse_int.[u32](val)!;
            e.fixed32(num, v)!;
            @expect_eq(fld.as_fixed32()!, v);
        } else if strs.eq(typ, "fixed64") {
            let v = fmt.parse_int.[u64](val)!;
            e.fixed64(num, v)!;
            @expect_eq(fld.as_fixed64()!, v);
        } else if strs.eq(typ, "sfixed32") {
            let v = fmt.parse_int.[i32](val)!;
            e.sfixed32(num, v)!;
            @expect_eq(fld.as_sfixed32()!, v);
        } else if strs.eq(typ, "sfixed64") {
            let v = fmt.parse_int.[i64](val)!;
            e.sfixed64(num, v)!;
            @expect_eq(fld.as_sfixed64()!, v);
        } else if strs.eq(typ, "float") {
            let v = @float_cast(f32, fmt.parse_float(val)!);
            e.float(num, v)!;
            @expect_eq(fld.as_float()!, v);
        } else if strs.eq(typ, "double") {
            let v = fmt.parse_float(val)!;
            e.double(num, v)!;
            @expect_eq(fld.as_double()!, v);
        } else {
            let mut hv = val;
            if strs.eq(val, "-") {
                hv = "";
            }
            let sv = hex.decode_alloc(hv)!;
            defer mem.free(sv);
            e.string(num, sv)!;
            @expect_eq(fld.as_string()!, sv);
        }
        @expect_eq(e.bytes(), want);
        n += 1;
    }
    @expect(n > 40);
}

test "a real message: descriptor.proto" {
    let bin = fs.read_file("std/protobuf/testdata/descriptor.bin") catch return;
    defer mem.free(bin);
    let facts = fs.read_file("std/protobuf/testdata/facts.txt")!;
    defer mem.free(facts);
    let mut r = Reader.init(bin);
    // FileDescriptorProto: 1 name, 2 package, 4 message_type, 5 enum_type
    let mut names = strs.String.init();
    defer names.deinit();
    let mut fields: usize = 0;
    let mut it = Reader.init(bin);
    loop {
        let f = it.next() catch |e| {
            if e == error.Done {
                break;
            }
            return e;
        };
        if f.number == 4 {
            let mut m = f.message()!;
            let name = m.last(1)!.as_string()!;
            let nf = m.count(2)!;  // DescriptorProto.field
            fields += nf;
            fmt.write(names.writer(), "message {} {}\n", [fmt.str(name), fmt.int(nf)])!;
        }
    }
    let mut want = strs.String.init();
    defer want.deinit();
    fmt.write(want.writer(), "name {}\npackage {}\nmessages {}\nfields {}\nenums {}\n{}",
              [fmt.str(r.last(1)!.as_string()!), fmt.str(r.last(2)!.as_string()!), fmt.int(r.count(4)!),
               fmt.int(fields), fmt.int(r.count(5)!), fmt.str(names.bytes())])!;
    let body = facts[(strs.index_of(facts, "\n")!) + 1..];
    @expect_eq(want.bytes(), body);
    // copying every field reproduces the message
    let mut e = Encoder.init();
    defer e.deinit();
    let mut all = Reader.init(bin);
    loop {
        let f = all.next() catch |err| {
            if err == error.Done {
                break;
            }
            return err;
        };
        if f.wire == LEN {
            e.bytes_field(f.number, f.data)!;
        } else if f.wire == VARINT {
            e.uint64(f.number, f.bits)!;
        } else if f.wire == I32 {
            e.fixed32(f.number, @truncate(u32, f.bits))!;
        } else {
            e.fixed64(f.number, f.bits)!;
        }
    }
    @expect_eq(e.bytes(), bin);
    let dump = decode_raw_alloc(bin)!;
    defer mem.free(dump);
    @expect(strs.starts_with(dump,
                             "1: \"google/protobuf/descriptor.proto\"\n2: \"google.protobuf\"\n4 {\n  1: \"FileDescriptorSet\"\n"));
}

fn fails_all(bytes: []u8) bool {
    let mut r = Reader.init(bytes);
    loop {
        _ = r.next() catch |e| {
            return e != error.Done;
        };
    }
}

test "malformed input" {
    let bad: [8][]u8 = [
        "\x08",  // a key without its value
        "\x08\xff\xff\xff\xff\xff\xff\xff\xff\xff\x02",  // a varint over 64 bits
        "\x12\x05ab",    // a length past the end
        "\x00\x01",      // field 0
        "\x0e",          // wire type 6
        "\x0c",          // an end-group with no start
        "\x0b\x08\x01",  // a group that never ends
        "\x0b\x14",      // a group ended with the wrong number
    ];
    for b in bad {
        @expect(fails_all(b));
    }
    @expect(not fails_all("\x0b\x08\x01\x0c\x10\x02"));  // a group, skipped over
    let mut s = strs.String.init();
    defer s.deinit();
    for _k in @as(usize, 0)..1000 {
        s.push(0x0b)!;
    }
    @expect(fails_all(s.bytes()));
    // nested length-delimited fields that all parse as messages
    let mut e = Encoder.init();
    defer e.deinit();
    for _k in @as(usize, 0)..500 {
        e.begin(1)!;
    }
    for _k in @as(usize, 0)..500 {
        e.end()!;
    }
    let raw = decode_raw_alloc(e.bytes()) catch return;
    mem.free(raw);
}
