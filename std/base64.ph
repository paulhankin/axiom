// =============================================================================
// std/base64.ph - Base64 (RFC 4648), standard and URL-safe alphabets.
//
//     let n = base64.encoded_len(data.len);
//     base64.encode(data, out[..n], base64.STANDARD);
//     let bytes = base64.decode(text, out, base64.STANDARD)!;
//     let owned = base64.decode_alloc(text, base64.STANDARD)!;  // mem.free
// =============================================================================

import "mem.ph" as mem;

error { InvalidBase64, NoSpaceLeft, OutOfMemory }

pub struct Encoding {
    alphabet: []u8,
    pad: bool,
}

pub let STANDARD: Encoding = Encoding.{ alphabet: "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/",
                                        pad: true };
pub let URL: Encoding = Encoding.{ alphabet: "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_",
                                   pad: false };

/// The length of the encoding of n bytes.
pub fn encoded_len_with(n: usize, enc: Encoding) usize {
    if enc.pad {
        return ((n + 2) / 3) * 4;
    }
    return ((n * 4) + 2) / 3;
}

/// The length of the standard (padded) encoding of n bytes.
pub fn encoded_len(n: usize) usize {
    return ((n + 2) / 3) * 4;
}

/// Encodes data into out, which must be encoded_len_with(data.len) long.
pub fn encode(data: []u8, out: []mut u8, enc: Encoding) []u8 {
    let a = enc.alphabet;
    let mut o: usize = 0;
    let mut i: usize = 0;
    while (i + 3) <= data.len {
        let v = ((@as(u32, data[i]) << 16) | (@as(u32, data[i + 1]) << 8)) | @as(u32, data[i + 2]);
        out[o] = a[@as(usize, (v >> 18) & 63)];
        out[o + 1] = a[@as(usize, (v >> 12) & 63)];
        out[o + 2] = a[@as(usize, (v >> 6) & 63)];
        out[o + 3] = a[@as(usize, v & 63)];
        o += 4;
        i += 3;
    }
    let rest = data.len - i;
    if rest > 0 {
        let mut v = @as(u32, data[i]) << 16;
        if rest == 2 {
            v = v | (@as(u32, data[i + 1]) << 8);
        }
        out[o] = a[@as(usize, (v >> 18) & 63)];
        out[o + 1] = a[@as(usize, (v >> 12) & 63)];
        o += 2;
        if rest == 2 {
            out[o] = a[@as(usize, (v >> 6) & 63)];
            o += 1;
        }
        if enc.pad {
            while (o % 4) != 0 {
                out[o] = '=';
                o += 1;
            }
        }
    }
    return out[..o];
}

/// Encodes into new memory (free it with mem.free).
pub fn encode_alloc(data: []u8, enc: Encoding) ![]mut u8 {
    let out = mem.alloc.[u8](encoded_len_with(data.len, enc))!;
    _ = encode(data, out, enc);
    return out;
}

fn value_of(c: u8, enc: *Encoding) u32 {
    if (c >= 'A') and (c <= 'Z') {
        return @as(u32, c - 'A');
    }
    if (c >= 'a') and (c <= 'z') {
        return @as(u32, c - 'a') + 26;
    }
    if (c >= '0') and (c <= '9') {
        return @as(u32, c - '0') + 52;
    }
    if c == enc.alphabet[62] {
        return 62;
    }
    if c == enc.alphabet[63] {
        return 63;
    }
    return 64;
}

/// The most bytes that len characters can decode to.
pub fn decoded_len(len: usize) usize {
    return ((len + 3) / 4) * 3;
}

/// Decodes text into out (decoded_len(text.len) bytes suffice). Whitespace
/// is skipped; padding is optional; anything else fails with
/// InvalidBase64.
pub fn decode(text: []u8, out: []mut u8, enc: Encoding) ![]u8 {
    let mut acc: u32 = 0;
    let mut nbits: u32 = 0;
    let mut o: usize = 0;
    let mut padding: u32 = 0;
    let mut chars: usize = 0;
    for c in text {
        if (c == ' ') or (c == '\n') or (c == '\r') or (c == '\t') {
            continue;
        }
        if c == '=' {
            padding += 1;
            continue;
        }
        if padding > 0 {
            return error.InvalidBase64;  // data after padding
        }
        let v = value_of(c, &enc);
        if v > 63 {
            return error.InvalidBase64;
        }
        chars += 1;
        acc = (acc << 6) | v;
        nbits += 6;
        if nbits >= 8 {
            nbits -= 8;
            if o >= out.len {
                return error.NoSpaceLeft;
            }
            out[o] = @truncate(u8, acc >> nbits);
            o += 1;
        }
    }
    // leftover bits must be zero and fewer than a byte
    if (nbits >= 6) or ((acc & ((@as(u32, 1) << nbits) - 1)) != 0) or (padding > 2) {
        return error.InvalidBase64;
    }
    // padding, if any, is exactly what completes the last group
    if (padding > 0) and (@as(usize, padding) != ((4 - (chars % 4)) % 4)) {
        return error.InvalidBase64;
    }
    return out[..o];
}

/// Decodes into new memory (free it with mem.free).
pub fn decode_alloc(text: []u8, enc: Encoding) ![]mut u8 {
    let buf = mem.alloc.[u8](decoded_len(text.len))!;
    let got = decode(text, buf, enc) catch |e| {
        mem.free(buf);
        return e;
    };
    return mem.realloc(buf, got.len) catch |e| {
        mem.free(buf);
        return e;
    };
}

test "rfc 4648 vectors" {
    let ins: [7][]u8 = ["", "f", "fo", "foo", "foob", "fooba", "foobar"];
    let outs: [7][]u8 = ["", "Zg==", "Zm8=", "Zm9v", "Zm9vYg==", "Zm9vYmE=", "Zm9vYmFy"];
    let mut b: [16]u8 = [0; 16];
    let mut d: [16]u8 = [0; 16];
    for i in 0..ins.len {
        let e = encode(ins[i], b[..encoded_len(ins[i].len)], STANDARD);
        @expect_eq(e, outs[i]);
        @expect_eq(decode(e, d[..], STANDARD)!, ins[i]);
    }
    @expect_eq(encode("\xfb\xff", b[..encoded_len_with(2, URL)], URL), "-_8");
    @expect_eq(decode("-_8", d[..], URL)!, "\xfb\xff");
    @expect_eq(decode("Zm9v\nYmFy\n", d[..], STANDARD)!, "foobar");
    let bad: [4][]u8 = ["Zm9v!", "Zg=x", "Zh==", "Z"];
    for s in bad {
        _ = decode(s, d[..], STANDARD) catch continue;
        @trap();
    }
    let owned = decode_alloc("aGVsbG8=", STANDARD)!;
    defer mem.free(owned);
    @expect_eq(owned, "hello");
}

test "padding must be exact" {
    let mut out: [16]u8 = [0; 16];
    @expect(@mem_eq(decode("Zm8=", out[..], STANDARD)!, "fo"));
    @expect(@mem_eq(decode("Zm8", out[..], STANDARD)!, "fo"));
    @expect(@mem_eq(decode("Zm9v", out[..], STANDARD)!, "foo"));
    _ = decode("Zm8==", out[..], STANDARD) catch |e| {
        @expect(e == error.InvalidBase64);
        _ = decode("Zm9v==", out[..], STANDARD) catch return;
        @expect(false);
        return;
    };
    @expect(false);
}
