// =============================================================================
// std/crypto/aes_gcm.ph - AES-128 and AES-256 in GCM mode (FIPS 197,
// NIST SP 800-38D).
//
//     let g = aes_gcm.AesGcm.init(key)!;          // 16 or 32 bytes
//     g.seal(nonce, aad, plaintext, out)!;         // out: len + 16
//     g.open(nonce, aad, sealed, out)!;            // AuthFailed if forged
//
// Nonces are 12 bytes and must never repeat under one key. The cipher runs
// in time independent of the key and data: the S-box is 16 table lookups
// with vector shuffles over all 256 entries (no memory access depends on a
// secret), and GHASH multiplies with integer multiplications whose unused
// bits are masked off (after BearSSL's ghash_ctmul64).
// =============================================================================

import "subtle.ph" as subtle;

error { AuthFailed, InvalidLength }

pub const NONCE_LEN: usize = 12;
pub const TAG_LEN: usize = 16;

let SBOX: [256]u8 = [
    0x63,
    0x7c,
    0x77,
    0x7b,
    0xf2,
    0x6b,
    0x6f,
    0xc5,
    0x30,
    0x01,
    0x67,
    0x2b,
    0xfe,
    0xd7,
    0xab,
    0x76,
    0xca,
    0x82,
    0xc9,
    0x7d,
    0xfa,
    0x59,
    0x47,
    0xf0,
    0xad,
    0xd4,
    0xa2,
    0xaf,
    0x9c,
    0xa4,
    0x72,
    0xc0,
    0xb7,
    0xfd,
    0x93,
    0x26,
    0x36,
    0x3f,
    0xf7,
    0xcc,
    0x34,
    0xa5,
    0xe5,
    0xf1,
    0x71,
    0xd8,
    0x31,
    0x15,
    0x04,
    0xc7,
    0x23,
    0xc3,
    0x18,
    0x96,
    0x05,
    0x9a,
    0x07,
    0x12,
    0x80,
    0xe2,
    0xeb,
    0x27,
    0xb2,
    0x75,
    0x09,
    0x83,
    0x2c,
    0x1a,
    0x1b,
    0x6e,
    0x5a,
    0xa0,
    0x52,
    0x3b,
    0xd6,
    0xb3,
    0x29,
    0xe3,
    0x2f,
    0x84,
    0x53,
    0xd1,
    0x00,
    0xed,
    0x20,
    0xfc,
    0xb1,
    0x5b,
    0x6a,
    0xcb,
    0xbe,
    0x39,
    0x4a,
    0x4c,
    0x58,
    0xcf,
    0xd0,
    0xef,
    0xaa,
    0xfb,
    0x43,
    0x4d,
    0x33,
    0x85,
    0x45,
    0xf9,
    0x02,
    0x7f,
    0x50,
    0x3c,
    0x9f,
    0xa8,
    0x51,
    0xa3,
    0x40,
    0x8f,
    0x92,
    0x9d,
    0x38,
    0xf5,
    0xbc,
    0xb6,
    0xda,
    0x21,
    0x10,
    0xff,
    0xf3,
    0xd2,
    0xcd,
    0x0c,
    0x13,
    0xec,
    0x5f,
    0x97,
    0x44,
    0x17,
    0xc4,
    0xa7,
    0x7e,
    0x3d,
    0x64,
    0x5d,
    0x19,
    0x73,
    0x60,
    0x81,
    0x4f,
    0xdc,
    0x22,
    0x2a,
    0x90,
    0x88,
    0x46,
    0xee,
    0xb8,
    0x14,
    0xde,
    0x5e,
    0x0b,
    0xdb,
    0xe0,
    0x32,
    0x3a,
    0x0a,
    0x49,
    0x06,
    0x24,
    0x5c,
    0xc2,
    0xd3,
    0xac,
    0x62,
    0x91,
    0x95,
    0xe4,
    0x79,
    0xe7,
    0xc8,
    0x37,
    0x6d,
    0x8d,
    0xd5,
    0x4e,
    0xa9,
    0x6c,
    0x56,
    0xf4,
    0xea,
    0x65,
    0x7a,
    0xae,
    0x08,
    0xba,
    0x78,
    0x25,
    0x2e,
    0x1c,
    0xa6,
    0xb4,
    0xc6,
    0xe8,
    0xdd,
    0x74,
    0x1f,
    0x4b,
    0xbd,
    0x8b,
    0x8a,
    0x70,
    0x3e,
    0xb5,
    0x66,
    0x48,
    0x03,
    0xf6,
    0x0e,
    0x61,
    0x35,
    0x57,
    0xb9,
    0x86,
    0xc1,
    0x1d,
    0x9e,
    0xe1,
    0xf8,
    0x98,
    0x11,
    0x69,
    0xd9,
    0x8e,
    0x94,
    0x9b,
    0x1e,
    0x87,
    0xe9,
    0xce,
    0x55,
    0x28,
    0xdf,
    0x8c,
    0xa1,
    0x89,
    0x0d,
    0xbf,
    0xe6,
    0x42,
    0x68,
    0x41,
    0x99,
    0x2d,
    0x0f,
    0xb0,
    0x54,
    0xbb,
    0x16,
];

// -----------------------------------------------------------------------------
// AES (encryption only: GCM needs no more)
// -----------------------------------------------------------------------------

/// SubBytes on 16 bytes: for each high nibble h, a shuffle looks up row h
/// of the S-box by the low nibble, kept only in lanes whose high nibble is
/// h. Every row is read, whatever the data.
fn sub_bytes(x: u8x16) u8x16 {
    let lo = x & @splat(@as(u8, 0x0f));
    let hi = x >> 4;
    let mut r: u8x16 = @splat(@as(u8, 0));
    for h in @as(usize, 0)..16 {
        let row = @load(u8x16, SBOX[16 * h..(16 * h) + 16]);
        let t = @swizzle(row, lo);
        let m = @bit_cast(u8x16, @lanes_eq(hi, @splat(@truncate(u8, h))));
        r = r | (t & m);
    }
    return r;
}

fn shift_rows(s: u8x16) u8x16 {
    return @shuffle(s, s, [0, 5, 10, 15, 4, 9, 14, 3, 8, 13, 2, 7, 12, 1, 6, 11]);
}

/// Multiplication by x in GF(2^8), lane-wise.
fn xtime(v: u8x16) u8x16 {
    let top = @bit_cast(u8x16, @lanes_lt(@bit_cast(i8x16, v), @splat(@as(i8, 0))));
    return (v +% v) ^ (top & @splat(@as(u8, 0x1b)));
}

fn mix_columns(a: u8x16) u8x16 {
    // rotations of each 4-byte column by 1, 2 and 3 rows
    let r1 = @shuffle(a, a, [1, 2, 3, 0, 5, 6, 7, 4, 9, 10, 11, 8, 13, 14, 15, 12]);
    let r2 = @shuffle(a, a, [2, 3, 0, 1, 6, 7, 4, 5, 10, 11, 8, 9, 14, 15, 12, 13]);
    let r3 = @shuffle(a, a, [3, 0, 1, 2, 7, 4, 5, 6, 11, 8, 9, 10, 15, 12, 13, 14]);
    return ((xtime(a ^ r1) ^ r1) ^ r2) ^ r3;
}

pub struct Aes {
    rk: [15]u8x16,
    rounds: usize,

    /// Expands a 16-byte (AES-128) or 32-byte (AES-256) key.
    pub fn init(key: []u8) !Aes {
        let nk = key.len / 4;
        if (key.len != 16) and (key.len != 32) {
            return error.InvalidLength;
        }
        let rounds = nk + 6;
        let mut w: [60]u32 = [0; 60];
        for i in 0..nk {
            w[i] = (
                ((@as(u32, key[4 * i]) << 24) | (@as(u32, key[(4 * i) + 1]) << 16)) | (
                    @as(u32, key[(4 * i) + 2]) << 8)) | @as(u32, key[(4 * i) + 3]);
        }
        let mut rcon: u32 = 1;
        for i in nk..4 * (rounds + 1) {
            let mut t = w[i - 1];
            if (i % nk) == 0 {
                t = sub_word(@rotl(t, 8)) ^ (rcon << 24);
                rcon = rcon << 1;
                if rcon == 0x100 {
                    rcon = 0x1b;
                }
            } else if (nk > 6) and ((i % nk) == 4) {
                t = sub_word(t);
            }
            w[i] = w[i - nk] ^ t;
        }
        let mut a = Aes.{ rk: [@splat(@as(u8, 0)); 15], rounds: rounds };
        for r in 0..rounds + 1 {
            let mut b: [16]u8 = [0; 16];
            for c in @as(usize, 0)..4 {
                let v = w[(4 * r) + c];
                b[4 * c] = @truncate(u8, v >> 24);
                b[(4 * c) + 1] = @truncate(u8, v >> 16);
                b[(4 * c) + 2] = @truncate(u8, v >> 8);
                b[(4 * c) + 3] = @truncate(u8, v);
            }
            a.rk[r] = @load(u8x16, b[..]);
        }
        return a;
    }

    pub fn encrypt_block(self: *Aes, input: u8x16) u8x16 {
        let mut s = input ^ self.rk[0];
        for r in 1..self.rounds {
            s = mix_columns(shift_rows(sub_bytes(s))) ^ self.rk[r];
        }
        return shift_rows(sub_bytes(s)) ^ self.rk[self.rounds];
    }
}

fn sub_word(w: u32) u32 {
    let v: u8x16 = @vec(u8x16, @truncate(u8, w >> 24), @truncate(u8, w >> 16), @truncate(u8, w >> 8), @truncate(u8, w),
                        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
    let s = sub_bytes(v);
    return (
        ((@as(u32, @extract_lane(s, 0)) << 24) | (@as(u32, @extract_lane(s, 1)) << 16)) | (
            @as(u32, @extract_lane(s, 2)) << 8)) | @as(u32, @extract_lane(s, 3));
}

// -----------------------------------------------------------------------------
// GHASH
// -----------------------------------------------------------------------------

fn be64(b: []u8) u64 {
    let mut v: u64 = 0;
    for i in @as(usize, 0)..8 {
        v = (v << 8) | @as(u64, b[i]);
    }
    return v;
}

fn put_be64(b: []mut u8, v: u64) void {
    for i in @as(usize, 0)..8 {
        b[i] = @truncate(u8, v >> @truncate(u32, 56 - (8 * i)));
    }
}

/// The low 64 bits of the carry-less product of x and y, with ordinary
/// multiplications on operands with holes so that carries cannot spread.
fn bmul64(x: u64, y: u64) u64 {
    let m0: u64 = 0x1111111111111111;
    let m1: u64 = 0x2222222222222222;
    let m2: u64 = 0x4444444444444444;
    let m3: u64 = 0x8888888888888888;
    let x0 = x & m0;
    let x1 = x & m1;
    let x2 = x & m2;
    let x3 = x & m3;
    let y0 = y & m0;
    let y1 = y & m1;
    let y2 = y & m2;
    let y3 = y & m3;
    let z0 = (((x0 *% y0) ^ (x1 *% y3)) ^ (x2 *% y2)) ^ (x3 *% y1);
    let z1 = (((x0 *% y1) ^ (x1 *% y0)) ^ (x2 *% y3)) ^ (x3 *% y2);
    let z2 = (((x0 *% y2) ^ (x1 *% y1)) ^ (x2 *% y0)) ^ (x3 *% y3);
    let z3 = (((x0 *% y3) ^ (x1 *% y2)) ^ (x2 *% y1)) ^ (x3 *% y0);
    return (((z0 & m0) | (z1 & m1)) | (z2 & m2)) | (z3 & m3);
}

struct Wide {
    hi: u64,
    lo: u64,
}

/// The full 128-bit carry-less product: the high half comes from the
/// product of the bit-reversed operands.
fn clmul(a: u64, b: u64) Wide {
    return Wide.{ hi: rev64(bmul64(rev64(a), rev64(b))) >> 1, lo: bmul64(a, b) };
}

fn rev64(x: u64) u64 {
    let mut v = x;
    v = ((v & 0x5555555555555555) << 1) | ((v >> 1) & 0x5555555555555555);
    v = ((v & 0x3333333333333333) << 2) | ((v >> 2) & 0x3333333333333333);
    v = ((v & 0x0F0F0F0F0F0F0F0F) << 4) | ((v >> 4) & 0x0F0F0F0F0F0F0F0F);
    v = ((v & 0x00FF00FF00FF00FF) << 8) | ((v >> 8) & 0x00FF00FF00FF00FF);
    v = ((v & 0x0000FFFF0000FFFF) << 16) | ((v >> 16) & 0x0000FFFF0000FFFF);
    return (v << 32) | (v >> 32);
}

struct Ghash {
    y0: u64,
    y1: u64,
    h0: u64,
    h1: u64,
    buf: [16]u8,
    nbuf: usize,

    fn init(h: []u8) Ghash {
        return Ghash.{ y0: 0, y1: 0, h0: be64(h[0..8]), h1: be64(h[8..16]), buf: [0; 16], nbuf: 0 };
    }

    /// y = (y ^ block) * H in GF(2^128). GCM numbers bits from the most
    /// significant end, so both operands are bit-reversed into ordinary
    /// polynomial order, multiplied, reduced by x^128 + x^7 + x^2 + x + 1,
    /// and reversed back.
    fn block(self: *mut Ghash, src: []u8) void {
        let v1 = self.y0 ^ be64(src[0..8]);
        let v0 = self.y1 ^ be64(src[8..16]);
        // a = a1 x^64 + a0 in ordinary order
        let a0 = rev64(v1);
        let a1 = rev64(v0);
        let b0 = rev64(self.h0);
        let b1 = rev64(self.h1);
        // Karatsuba: three 64x64 carry-less products
        let lo = clmul(a0, b0);
        let hi = clmul(a1, b1);
        let mid = clmul(a0 ^ a1, b0 ^ b1);
        let m0 = (mid.lo ^ lo.lo) ^ hi.lo;
        let m1 = (mid.hi ^ lo.hi) ^ hi.hi;
        let r0 = lo.lo;
        let r1 = lo.hi ^ m0;
        let r2 = hi.lo ^ m1;
        let r3 = hi.hi;
        // reduce: (r3 r2) x^128 = (r3 r2)(x^7 + x^2 + x + 1)
        let t0 = ((r2 ^ (r2 << 1)) ^ (r2 << 2)) ^ (r2 << 7);
        let t1 = ((((r3 ^ (r3 << 1)) ^ (r3 << 2)) ^ (r3 << 7)) ^ (((r2 >> 63) ^ (r2 >> 62)) ^ (r2 >> 57)));
        // the bits pushed past x^128 come round once more
        let o = ((r3 >> 63) ^ (r3 >> 62)) ^ (r3 >> 57);
        let t0b = t0 ^ (((o ^ (o << 1)) ^ (o << 2)) ^ (o << 7));
        let n0 = r0 ^ t0b;
        let n1 = r1 ^ t1;
        self.y0 = rev64(n0);
        self.y1 = rev64(n1);
    }

    /// Absorbs data, zero-padded to a whole block.
    fn padded(self: *mut Ghash, data: []u8) void {
        let mut i: usize = 0;
        while (i + 16) <= data.len {
            self.block(data[i..i + 16]);
            i += 16;
        }
        if i < data.len {
            let mut b: [16]u8 = [0; 16];
            @mem_copy(b[..data.len - i], data[i..]);
            self.block(b[..]);
        }
    }
}

// -----------------------------------------------------------------------------
// GCM
// -----------------------------------------------------------------------------

pub struct AesGcm {
    aes: Aes,
    h: [16]u8,

    pub fn init(key: []u8) !AesGcm {
        let aes = Aes.init(key)!;
        let mut g = AesGcm.{ aes: aes, h: [0; 16] };
        @store(g.h[..], aes.encrypt_block(@splat(@as(u8, 0))));
        return g;
    }

    /// XORs data with the keystream from counter block j (incremented per
    /// block in its last 32 bits) into out.
    fn ctr(self: *AesGcm, j: *mut [16]u8, data: []u8, out: []mut u8) void {
        let mut i: usize = 0;
        let mut ks: [16]u8 = [0; 16];
        while i < data.len {
            inc32(j);
            @store(ks[..], self.aes.encrypt_block(@load(u8x16, j[..])));
            let n = @min(16, data.len - i);
            if n == 16 {
                @store(out[i..i + 16], @load(u8x16, data[i..i + 16]) ^ @load(u8x16, ks[..]));
            } else {
                for k in 0..n {
                    out[i + k] = data[i + k] ^ ks[k];
                }
            }
            i += n;
        }
    }

    fn tag(self: *AesGcm, j0: *[16]u8, aad: []u8, ct: []u8, out: []mut u8) void {
        let mut g = Ghash.init(self.h[..]);
        g.padded(aad);
        g.padded(ct);
        let mut lens: [16]u8 = [0; 16];
        put_be64(lens[0..8], @as(u64, aad.len) * 8);
        put_be64(lens[8..16], @as(u64, ct.len) * 8);
        g.block(lens[..]);
        let mut s: [16]u8 = [0; 16];
        put_be64(s[0..8], g.y0);
        put_be64(s[8..16], g.y1);
        let e = self.aes.encrypt_block(@load(u8x16, j0[..]));
        @store(out[..16], @load(u8x16, s[..]) ^ e);
    }

    /// Encrypts plaintext into out (plaintext.len + 16 bytes: ciphertext,
    /// then tag), authenticating aad too.
    pub fn seal(self: *AesGcm, nonce: []u8, aad: []u8, plaintext: []u8, out: []mut u8) !void {
        if (nonce.len != NONCE_LEN) or (out.len != (plaintext.len + TAG_LEN)) {
            return error.InvalidLength;
        }
        let j0 = counter0(nonce);
        let mut j = j0;
        self.ctr(&mut j, plaintext, out[..plaintext.len]);
        self.tag(&j0, aad, out[..plaintext.len], out[plaintext.len..]);
    }

    /// Checks and decrypts sealed (ciphertext then tag) into out
    /// (sealed.len - 16 bytes); AuthFailed, with out zeroed, if altered.
    pub fn open(self: *AesGcm, nonce: []u8, aad: []u8, sealed: []u8, out: []mut u8) !void {
        if (nonce.len != NONCE_LEN) or (sealed.len < TAG_LEN) or (out.len != (sealed.len - TAG_LEN)) {
            return error.InvalidLength;
        }
        let ct = sealed[..sealed.len - TAG_LEN];
        let j0 = counter0(nonce);
        let mut t: [16]u8 = [0; 16];
        self.tag(&j0, aad, ct, t[..]);
        if not subtle.equal(t[..], sealed[ct.len..]) {
            @mem_set(out, 0);
            return error.AuthFailed;
        }
        let mut j = j0;
        self.ctr(&mut j, ct, out);
    }
}

fn counter0(nonce: []u8) [16]u8 {
    let mut j: [16]u8 = [0; 16];
    @mem_copy(j[..12], nonce);
    j[15] = 1;
    return j;
}

fn inc32(j: *mut [16]u8) void {
    let mut i: usize = 16;
    while i > 12 {
        i -= 1;
        j[i] +%= 1;
        if j[i] != 0 {
            break;
        }
    }
}

// -----------------------------------------------------------------------------
// Tests (FIPS 197 appendix C, GCM spec test cases)
// -----------------------------------------------------------------------------

fn unhex(s: []u8, out: []mut u8) []mut u8 {
    for i in 0..s.len / 2 {
        out[i] = (hexv(s[2 * i]) << 4) | hexv(s[(2 * i) + 1]);
    }
    return out[..s.len / 2];
}

fn hexv(c: u8) u8 {
    if c <= '9' {
        return c - '0';
    }
    return ((c | 0x20) - 'a') + 10;
}

test "aes block" {
    let mut kb: [32]u8 = [0; 32];
    let mut pb: [16]u8 = [0; 16];
    let mut wb: [16]u8 = [0; 16];
    let pt = unhex("00112233445566778899aabbccddeeff", pb[..]);
    let a128 = Aes.init(unhex("000102030405060708090a0b0c0d0e0f", kb[..]))!;
    let mut out: [16]u8 = [0; 16];
    @store(out[..], a128.encrypt_block(@load(u8x16, pt)));
    @expect_eq(out[..], unhex("69c4e0d86a7b0430d8cdb78070b4c55a", wb[..]));
    let a256 = Aes.init(unhex("000102030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f", kb[..]))!;
    @store(out[..], a256.encrypt_block(@load(u8x16, pt)));
    @expect_eq(out[..], unhex("8ea2b7ca516745bfeafc49904b496089", wb[..]));
}

test "gcm" {
    let mut kb: [32]u8 = [0; 32];
    let mut nb: [12]u8 = [0; 12];
    let mut pb: [64]u8 = [0; 64];
    let mut ab: [20]u8 = [0; 20];
    let mut wb: [80]u8 = [0; 80];
    // test case 2: zero key, one zero block
    let g0 = AesGcm.init(unhex("00000000000000000000000000000000", kb[..]))!;
    let zero: [16]u8 = [0; 16];
    let mut o0: [32]u8 = [0; 32];
    g0.seal(unhex("000000000000000000000000", nb[..]), "", zero[..], o0[..])!;
    @expect_eq(o0[..], unhex("0388dace60b6a392f328c2b971b2fe78ab6e47d42cec13bdf53a67b21257bddf", wb[..]));
    // test case 4: aad and a partial block
    let g = AesGcm.init(unhex("feffe9928665731c6d6a8f9467308308", kb[..]))!;
    let nonce = unhex("cafebabefacedbaddecaf888", nb[..]);
    let pt =
        unhex("d9313225f88406e5a55909c5aff5269a86a7a9531534f7da2e4c303d8a318a721c3c0c95956809532fcf0e2449a6b525b16aedf5aa0de657ba637b39",
              pb[..]);
    let aad = unhex("feedfacedeadbeeffeedfacedeadbeefabaddad2", ab[..]);
    let mut o: [76]u8 = [0; 76];
    g.seal(nonce, aad, pt, o[..])!;
    @expect_eq(o[..],
               unhex("42831ec2217774244b7221b784d0d49ce3aa212f2c02a4e035c17e2329aca12e21d514b25466931c7d8f6a5aac84aa051ba30b396a0aac973d58e0915bc94fbc3221a5db94fae95ae7121a47",
                     wb[..]));
    let mut back: [60]u8 = [0; 60];
    g.open(nonce, aad, o[..], back[..])!;
    @expect_eq(back[..], pt);
    // test case 16: AES-256
    let g2 = AesGcm.init(unhex("feffe9928665731c6d6a8f9467308308feffe9928665731c6d6a8f9467308308", kb[..]))!;
    g2.seal(nonce, aad, pt, o[..])!;
    @expect_eq(o[..],
               unhex("522dc1f099567d07f47f37a32a84427d643a8cdcbfe5c0c97598a2bd2555d1aa8cb08e48590dbb3da7b08b1056828838c5f61e6393ba7a0abcc9f66276fc6ece0f4e1768cddf8853bb2d551b",
                     wb[..]));
    o[0] ^= 1;
    g2.open(nonce, aad, o[..], back[..]) catch |e| {
        @expect_eq(e, error.AuthFailed);
        return;
    };
    @trap();
}
