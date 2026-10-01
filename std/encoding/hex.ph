// =============================================================================
// std/encoding/hex.ph - hexadecimal encoding, and hex dumps.
//
//     let s = hex.encode_alloc(bytes)!;          // "deadbeef" (mem.free)
//     let b = hex.decode_alloc("DEAD beef")!;    // white space is skipped
//     let d = hex.dump_alloc(bytes)!;            // like `xxd`
//     hex.encode_to(w, bytes)!;                  // to an io.Writer (also dump_to)
// =============================================================================

import "../io.ph" as io;
import "../mem.ph" as mem;
import "../str.ph" as strs;

error { InvalidHex, NoSpaceLeft, OutOfMemory }

let DIGITS: []u8 = "0123456789abcdef";
let UPPER_DIGITS: []u8 = "0123456789ABCDEF";

/// Encodes data into out (2 * data.len bytes), in lowercase.
pub fn encode(data: []u8, out: []mut u8) []u8 {
    return encode_with(data, out, DIGITS);
}

/// Encodes data into out (2 * data.len bytes), in uppercase.
pub fn encode_upper(data: []u8, out: []mut u8) []u8 {
    return encode_with(data, out, UPPER_DIGITS);
}

fn encode_with(data: []u8, out: []mut u8, digits: []u8) []u8 {
    let o = out[..2 * data.len];
    for i in @as(usize, 0)..data.len {
        o[2 * i] = digits[@as(usize, data[i] >> 4)];
        o[(2 * i) + 1] = digits[@as(usize, data[i] & 15)];
    }
    return o;
}

/// Encodes into new memory (free it with mem.free).
pub fn encode_alloc(data: []u8) ![]mut u8 {
    let out = mem.alloc.[u8](@mul_checked(data.len, 2) catch return error.OutOfMemory)!;
    _ = encode(data, out);
    return out;
}

/// Encodes data to w in lowercase, a kilobyte of text at a time.
pub fn encode_to(w: io.Writer, data: []u8) !void {
    let mut buf: [1024]u8 = [0; 1024];
    let mut i: usize = 0;
    while i < data.len {
        let n = @min(512, data.len - i);
        w.write_all(encode(data[i..i + n], buf[..]))!;
        i += n;
    }
}

fn value(c: u8) u8 {
    if (c >= '0') and (c <= '9') {
        return c - '0';
    }
    if (c >= 'a') and (c <= 'f') {
        return (c - 'a') + 10;
    }
    if (c >= 'A') and (c <= 'F') {
        return (c - 'A') + 10;
    }
    return 255;
}

/// Decodes text (either case; spaces, tabs and line breaks between bytes
/// are skipped) into out.
pub fn decode(text: []u8, out: []mut u8) ![]u8 {
    let mut o: usize = 0;
    let mut i: usize = 0;
    while i < text.len {
        let c = text[i];
        if (c == ' ') or (c == '\t') or (c == '\n') or (c == '\r') {
            i += 1;
            continue;
        }
        if (i + 1) >= text.len {
            return error.InvalidHex;  // an odd digit out
        }
        let hi = value(c);
        let lo = value(text[i + 1]);
        if (hi | lo) > 15 {
            return error.InvalidHex;
        }
        if o >= out.len {
            return error.NoSpaceLeft;
        }
        out[o] = (hi << 4) | lo;
        o += 1;
        i += 2;
    }
    return out[..o];
}

/// Decodes into new memory (free it with mem.free).
pub fn decode_alloc(text: []u8) ![]mut u8 {
    let buf = mem.alloc.[u8](text.len / 2)!;
    let got = decode(text, buf) catch |e| {
        mem.free(buf);
        return e;
    };
    return mem.realloc(buf, got.len) catch |e| {
        mem.free(buf);
        return e;
    };
}

/// A hex dump: offset, 16 bytes in hex, and the printable ones as text,
/// a line each (free it with mem.free).
pub fn dump_alloc(data: []u8) ![]mut u8 {
    let mut s = strs.String.init();
    dump_to(s.writer(), data) catch |e| {
        s.deinit();
        return e;
    };
    return s.take();
}

/// Writes the hex dump of data (see dump_alloc) to w, about 4 KB at a time.
pub fn dump_to(w: io.Writer, data: []u8) !void {
    let mut out: [4096]u8 = [0; 4096];
    let mut k: usize = 0;
    let mut off: usize = 0;
    while off < data.len {
        if (out.len - k) < 80 {
            w.write_all(out[..k])!;
            k = 0;
        }
        let n = @min(16, data.len - off);
        let mut sh: usize = 32;
        while sh > 0 {
            sh -= 4;
            out[k] = DIGITS[(off >> sh) & 15];
            k += 1;
        }
        out[k] = ':';
        k += 1;
        for j in @as(usize, 0)..16 {
            if (j % 2) == 0 {
                out[k] = ' ';
                k += 1;
            }
            if j < n {
                out[k] = DIGITS[@as(usize, data[off + j] >> 4)];
                out[k + 1] = DIGITS[@as(usize, data[off + j] & 15)];
            } else {
                out[k] = ' ';
                out[k + 1] = ' ';
            }
            k += 2;
        }
        out[k] = ' ';
        out[k + 1] = ' ';
        k += 2;
        for j in @as(usize, 0)..n {
            let c = data[off + j];
            if (c >= 0x20) and (c < 0x7f) {
                out[k] = c;
            } else {
                out[k] = '.';
            }
            k += 1;
        }
        out[k] = '\n';
        k += 1;
        off += n;
    }
    w.write_all(out[..k])!;
}

// -----------------------------------------------------------------------------
// Tests
// -----------------------------------------------------------------------------

test "hex" {
    let mut out: [8]u8 = [0; 8];
    @expect_eq(encode("\xde\xad\xbe\xef", out[..]), "deadbeef");
    @expect_eq(encode_upper("\x00\x7f", out[..]), "007F");
    let mut b: [4]u8 = [0; 4];
    @expect_eq(decode("DE ad\nbE eF", b[..])!, "\xde\xad\xbe\xef");
    @expect(bad(decode("abc", b[..])));
    @expect(bad(decode("zz", b[..])));
    @expect(bad(decode("0011223344", b[..])));
    let e = encode_alloc("hi")!;
    defer mem.free(e);
    @expect_eq(e, "6869");
    let d = decode_alloc("6869")!;
    defer mem.free(d);
    @expect_eq(d, "hi");
    let x = dump_alloc("Hello, hex dump!\x00\x01")!;
    defer mem.free(x);
    @expect_eq(x,
               "00000000: 4865 6c6c 6f2c 2068 6578 2064 756d 7021  Hello, hex dump!\n00000010: 0001                                     ..\n");
}

test "encode_to and dump_to match the allocating forms" {
    let mut data: [1300]u8 = [0; 1300];
    for i in @as(usize, 0)..data.len {
        data[i] = @truncate(u8, (i * 89) + 7);
    }
    let e = encode_alloc(data[..])!;
    defer mem.free(e);
    let mut s = strs.String.init();
    defer s.deinit();
    encode_to(s.writer(), data[..])!;
    @expect_eq(s.bytes(), e);
    let d = dump_alloc(data[..])!;
    defer mem.free(d);
    s.clear();
    dump_to(s.writer(), data[..])!;
    @expect_eq(s.bytes(), d);
    @expect_eq(d.len, (81 * 68) + 56);
    let mut small: [100]u8 = [0; 100];
    let mut bw = io.BufferWriter.init(small[..]);
    dump_to(bw.writer(), data[..]) catch |err| {
        @expect_eq(err, error.NoSpaceLeft);
        return;
    };
    @trap();
}

fn bad(r: ![]u8) bool {
    _ = r catch return true;
    return false;
}
