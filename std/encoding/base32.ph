// =============================================================================
// std/encoding/base32.ph - Base32 (RFC 4648): the standard and "extended hex"
// alphabets, with or without padding.
//
//     let s = base32.encode_alloc(data, base32.STANDARD)!;   // mem.free
//     let b = base32.decode_alloc(s, base32.STANDARD)!;
//
// Decoding accepts either case and skips line breaks.
// =============================================================================

import "../mem.ph" as mem;

error { InvalidBase32, NoSpaceLeft, OutOfMemory }

pub struct Encoding {
    alphabet: []u8,
    pad: bool,
}

pub let STANDARD: Encoding = Encoding.{ alphabet: "ABCDEFGHIJKLMNOPQRSTUVWXYZ234567", pad: true };
pub let STANDARD_NO_PAD: Encoding = Encoding.{ alphabet: "ABCDEFGHIJKLMNOPQRSTUVWXYZ234567", pad: false };
pub let HEX: Encoding = Encoding.{ alphabet: "0123456789ABCDEFGHIJKLMNOPQRSTUV", pad: true };

/// The length of the encoding of n bytes.
pub fn encoded_len(n: usize, enc: Encoding) usize {
    if enc.pad {
        return ((n + 4) / 5) * 8;
    }
    return ((n * 8) + 4) / 5;
}

/// Encodes data into out (encoded_len(data.len, enc) bytes).
pub fn encode(data: []u8, out: []mut u8, enc: Encoding) []u8 {
    let a = enc.alphabet;
    let mut o: usize = 0;
    let mut acc: u64 = 0;
    let mut bits: u32 = 0;
    for b in data {
        acc = (acc << 8) | @as(u64, b);
        bits += 8;
        while bits >= 5 {
            bits -= 5;
            out[o] = a[@truncate(usize, (acc >> @as(u64, bits)) & 31)];
            o += 1;
        }
        acc = acc & ((@as(u64, 1) << @as(u64, bits)) - 1);
    }
    if bits > 0 {
        out[o] = a[@truncate(usize, (acc << @as(u64, 5 - bits)) & 31)];
        o += 1;
    }
    if enc.pad {
        while (o % 8) != 0 {
            out[o] = '=';
            o += 1;
        }
    }
    return out[..o];
}

/// Encodes into new memory (free it with mem.free).
pub fn encode_alloc(data: []u8, enc: Encoding) ![]mut u8 {
    let out = mem.alloc.[u8](encoded_len(data.len, enc))!;
    _ = encode(data, out, enc);
    return out;
}

fn value_of(c0: u8, enc: *Encoding) u32 {
    let mut c = c0;
    if (c >= 'a') and (c <= 'z') {
        c -= 32;
    }
    for i in @as(usize, 0)..32 {
        if enc.alphabet[i] == c {
            return @truncate(u32, i);
        }
    }
    return 99;
}

/// Decodes text into out.
pub fn decode(text: []u8, out: []mut u8, enc: Encoding) ![]u8 {
    let mut acc: u64 = 0;
    let mut bits: u32 = 0;
    let mut o: usize = 0;
    let mut chars: usize = 0;
    let mut padding: usize = 0;
    for c in text {
        if (c == '\n') or (c == '\r') {
            continue;
        }
        if c == '=' {
            padding += 1;
            continue;
        }
        if padding > 0 {
            return error.InvalidBase32;
        }
        let v = value_of(c, &enc);
        if v > 31 {
            return error.InvalidBase32;
        }
        chars += 1;
        acc = (acc << 5) | @as(u64, v);
        bits += 5;
        if bits >= 8 {
            bits -= 8;
            if o >= out.len {
                return error.NoSpaceLeft;
            }
            out[o] = @truncate(u8, acc >> @as(u64, bits));
            o += 1;
            acc = acc & ((@as(u64, 1) << @as(u64, bits)) - 1);
        }
    }
    // a group of 8 can end after 2, 4, 5 or 7 characters, with the unused
    // bits zero, and padding (if any) completes it
    let r = chars % 8;
    if (r == 1) or (r == 3) or (r == 6) or (acc != 0) {
        return error.InvalidBase32;
    }
    if (padding > 0) and (((chars + padding) % 8) != 0) {
        return error.InvalidBase32;
    }
    return out[..o];
}

/// Decodes into new memory (free it with mem.free).
pub fn decode_alloc(text: []u8, enc: Encoding) ![]mut u8 {
    let buf = mem.alloc.[u8](((text.len * 5) / 8) + 1)!;
    let got = decode(text, buf, enc) catch |e| {
        mem.free(buf);
        return e;
    };
    return mem.realloc(buf, got.len) catch |e| {
        mem.free(buf);
        return e;
    };
}

// -----------------------------------------------------------------------------
// Tests
// -----------------------------------------------------------------------------

test "rfc 4648 vectors" {
    let ins: [7][]u8 = ["", "f", "fo", "foo", "foob", "fooba", "foobar"];
    let std_outs: [7][]u8 = ["", "MY======", "MZXQ====", "MZXW6===", "MZXW6YQ=", "MZXW6YTB", "MZXW6YTBOI======"];
    let hex_outs: [7][]u8 = ["", "CO======", "CPNG====", "CPNMU===", "CPNMUOG=", "CPNMUOJ1", "CPNMUOJ1E8======"];
    let mut b: [32]u8 = [0; 32];
    let mut d: [32]u8 = [0; 32];
    for i in @as(usize, 0)..7 {
        @expect_eq(encode(ins[i], b[..encoded_len(ins[i].len, STANDARD)], STANDARD), std_outs[i]);
        @expect_eq(decode(std_outs[i], d[..], STANDARD)!, ins[i]);
        @expect_eq(encode(ins[i], b[..encoded_len(ins[i].len, HEX)], HEX), hex_outs[i]);
        @expect_eq(decode(hex_outs[i], d[..], HEX)!, ins[i]);
    }
    @expect_eq(encode("foobar", b[..encoded_len(6, STANDARD_NO_PAD)], STANDARD_NO_PAD), "MZXW6YTBOI");
    @expect_eq(decode("mzxw6ytboi", d[..], STANDARD_NO_PAD)!, "foobar");
    @expect(bad(decode("M", d[..], STANDARD)));
    @expect(bad(decode("MZ=X", d[..], STANDARD)));
    @expect(bad(decode("MZ======x", d[..], STANDARD)));
    @expect(bad(decode("MZ", d[..], STANDARD)));  // the leftover bits must be zero
    @expect(bad(decode("MY=====", d[..], STANDARD)));
    let e = encode_alloc("hello", STANDARD)!;
    defer mem.free(e);
    let back = decode_alloc(e, STANDARD)!;
    defer mem.free(back);
    @expect_eq(back, "hello");
}

fn bad(r: ![]u8) bool {
    _ = r catch return true;
    return false;
}
