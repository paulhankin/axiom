// =============================================================================
// std/crypto/chacha20poly1305.ph - the ChaCha20-Poly1305 AEAD (RFC 8439).
//
//     chacha20poly1305.seal(key, nonce, aad, plaintext, out)!;   // out: len + 16
//     chacha20poly1305.open(key, nonce, aad, sealed, out)!;      // AuthFailed if forged
//
// Keys are 32 bytes and nonces 12; a nonce must never be reused with the
// same key. Everything runs in time independent of the secret data.
// =============================================================================

import "subtle.ph" as subtle;

error { AuthFailed, InvalidLength }

/// The block counter (from 1) is 32 bits: at most 2^32 - 1 blocks of 64 bytes.
fn too_long(n: usize) bool {
    return @as(u64, n) > (((@as(u64, 1) << 32) - 1) * 64);
}

pub const KEY_LEN: usize = 32;
pub const NONCE_LEN: usize = 12;
pub const TAG_LEN: usize = 16;

fn le32(b: []u8) u32 {
    return ((@as(u32, b[0]) | (@as(u32, b[1]) << 8)) | (@as(u32, b[2]) << 16)) | (@as(u32, b[3]) << 24);
}

fn put_le32(b: []mut u8, v: u32) void {
    b[0] = @truncate(u8, v);
    b[1] = @truncate(u8, v >> 8);
    b[2] = @truncate(u8, v >> 16);
    b[3] = @truncate(u8, v >> 24);
}

// -----------------------------------------------------------------------------
// ChaCha20
// -----------------------------------------------------------------------------

fn quarter(s: *mut [16]u32, a: usize, b: usize, c: usize, d: usize) void {
    s[a] +%= s[b];
    s[d] = @rotl(s[d] ^ s[a], 16);
    s[c] +%= s[d];
    s[b] = @rotl(s[b] ^ s[c], 12);
    s[a] +%= s[b];
    s[d] = @rotl(s[d] ^ s[a], 8);
    s[c] +%= s[d];
    s[b] = @rotl(s[b] ^ s[c], 7);
}

/// One 64-byte keystream block.
pub fn block(key: []u8, counter: u32, nonce: []u8, out: []mut u8) void {
    let mut init: [16]u32 = [0x61707865, 0x3320646e, 0x79622d32, 0x6b206574, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0];
    for i in @as(usize, 0)..8 {
        init[4 + i] = le32(key[4 * i..(4 * i) + 4]);
    }
    init[12] = counter;
    init[13] = le32(nonce[0..4]);
    init[14] = le32(nonce[4..8]);
    init[15] = le32(nonce[8..12]);
    let mut s = init;
    for r in @as(u32, 0)..10 {
        quarter(&mut s, 0, 4, 8, 12);
        quarter(&mut s, 1, 5, 9, 13);
        quarter(&mut s, 2, 6, 10, 14);
        quarter(&mut s, 3, 7, 11, 15);
        quarter(&mut s, 0, 5, 10, 15);
        quarter(&mut s, 1, 6, 11, 12);
        quarter(&mut s, 2, 7, 8, 13);
        quarter(&mut s, 3, 4, 9, 14);
    }
    for i in @as(usize, 0)..16 {
        put_le32(out[4 * i..(4 * i) + 4], s[i] +% init[i]);
    }
}

/// XORs data with the keystream starting at block `counter` into out.
pub fn xor_stream(key: []u8, counter: u32, nonce: []u8, data: []u8, out: []mut u8) void {
    let mut ks: [64]u8 = [0; 64];
    let mut c = counter;
    let mut i: usize = 0;
    while i < data.len {
        block(key, c, nonce, ks[..]);
        c +%= 1;
        let n = @min(64, data.len - i);
        for j in 0..n {
            out[i + j] = data[i + j] ^ ks[j];
        }
        i += n;
    }
}

// -----------------------------------------------------------------------------
// Poly1305 (26-bit limbs, after poly1305-donna)
// -----------------------------------------------------------------------------

pub struct Poly1305 {
    r: [5]u32,
    h: [5]u32,
    pad: [4]u32,
    buf: [16]u8,
    nbuf: usize,

    pub fn init(key: []u8) Poly1305 {
        let mut p = Poly1305.{ r: [0; 5], h: [0; 5], pad: [0; 4], buf: [0; 16], nbuf: 0 };
        // r with the clamping of RFC 8439 2.5
        p.r[0] = le32(key[0..4]) & 0x3ffffff;
        p.r[1] = (le32(key[3..7]) >> 2) & 0x3ffff03;
        p.r[2] = (le32(key[6..10]) >> 4) & 0x3ffc0ff;
        p.r[3] = (le32(key[9..13]) >> 6) & 0x3f03fff;
        p.r[4] = (le32(key[12..16]) >> 8) & 0x00fffff;
        for i in @as(usize, 0)..4 {
            p.pad[i] = le32(key[16 + (4 * i)..20 + (4 * i)]);
        }
        return p;
    }

    fn blocks(self: *mut Poly1305, m: []u8, hibit: u32) void {
        let r0 = @as(u64, self.r[0]);
        let r1 = @as(u64, self.r[1]);
        let r2 = @as(u64, self.r[2]);
        let r3 = @as(u64, self.r[3]);
        let r4 = @as(u64, self.r[4]);
        let s1 = r1 * 5;
        let s2 = r2 * 5;
        let s3 = r3 * 5;
        let s4 = r4 * 5;
        let mut h0 = self.h[0];
        let mut h1 = self.h[1];
        let mut h2 = self.h[2];
        let mut h3 = self.h[3];
        let mut h4 = self.h[4];
        let mut at: usize = 0;
        while (at + 16) <= m.len {
            let b = m[at..at + 16];
            h0 +%= le32(b[0..4]) & 0x3ffffff;
            h1 +%= (le32(b[3..7]) >> 2) & 0x3ffffff;
            h2 +%= (le32(b[6..10]) >> 4) & 0x3ffffff;
            h3 +%= (le32(b[9..13]) >> 6) & 0x3ffffff;
            h4 +%= (le32(b[12..16]) >> 8) | hibit;
            let d0 = (
                (((@as(u64, h0) * r0) + (@as(u64, h1) * s4)) + (@as(u64, h2) * s3)) + (@as(u64, h3) * s2)) + (
                    @as(u64, h4) * s1);
            let mut d1 = (
                (((@as(u64, h0) * r1) + (@as(u64, h1) * r0)) + (@as(u64, h2) * s4)) + (@as(u64, h3) * s3)) + (
                    @as(u64, h4) * s2);
            let mut d2 = (
                (((@as(u64, h0) * r2) + (@as(u64, h1) * r1)) + (@as(u64, h2) * r0)) + (@as(u64, h3) * s4)) + (
                    @as(u64, h4) * s3);
            let mut d3 = (
                (((@as(u64, h0) * r3) + (@as(u64, h1) * r2)) + (@as(u64, h2) * r1)) + (@as(u64, h3) * r0)) + (
                    @as(u64, h4) * s4);
            let mut d4 = (
                (((@as(u64, h0) * r4) + (@as(u64, h1) * r3)) + (@as(u64, h2) * r2)) + (@as(u64, h3) * r1)) + (
                    @as(u64, h4) * r0);
            let mut c = d0 >> 26;
            h0 = @truncate(u32, d0) & 0x3ffffff;
            d1 += c;
            c = d1 >> 26;
            h1 = @truncate(u32, d1) & 0x3ffffff;
            d2 += c;
            c = d2 >> 26;
            h2 = @truncate(u32, d2) & 0x3ffffff;
            d3 += c;
            c = d3 >> 26;
            h3 = @truncate(u32, d3) & 0x3ffffff;
            d4 += c;
            c = d4 >> 26;
            h4 = @truncate(u32, d4) & 0x3ffffff;
            h0 +%= @truncate(u32, c * 5);
            let c2 = h0 >> 26;
            h0 = h0 & 0x3ffffff;
            h1 +%= c2;
            at += 16;
        }
        self.h[0] = h0;
        self.h[1] = h1;
        self.h[2] = h2;
        self.h[3] = h3;
        self.h[4] = h4;
    }

    pub fn update(self: *mut Poly1305, data: []u8) void {
        let mut d = data;
        if self.nbuf > 0 {
            let k = @min(16 - self.nbuf, d.len);
            @mem_copy(self.buf[self.nbuf..self.nbuf + k], d[..k]);
            self.nbuf += k;
            d = d[k..];
            if self.nbuf < 16 {
                return;
            }
            let b = self.buf;
            self.blocks(b[..], 1 << 24);
            self.nbuf = 0;
        }
        let full = d.len - (d.len % 16);
        self.blocks(d[..full], 1 << 24);
        @mem_copy(self.buf[..d.len - full], d[full..]);
        self.nbuf = d.len - full;
    }

    /// Writes the 16-byte tag.
    pub fn final(self: *mut Poly1305, out: []mut u8) void {
        if self.nbuf > 0 {
            // the last partial block is padded with 1 then zeros
            let mut b: [16]u8 = [0; 16];
            @mem_copy(b[..self.nbuf], self.buf[..self.nbuf]);
            b[self.nbuf] = 1;
            self.blocks(b[..], 0);
        }
        let mut h0 = self.h[0];
        let mut h1 = self.h[1];
        let mut h2 = self.h[2];
        let mut h3 = self.h[3];
        let mut h4 = self.h[4];
        // full carry
        let mut c = h1 >> 26;
        h1 = h1 & 0x3ffffff;
        h2 +%= c;
        c = h2 >> 26;
        h2 = h2 & 0x3ffffff;
        h3 +%= c;
        c = h3 >> 26;
        h3 = h3 & 0x3ffffff;
        h4 +%= c;
        c = h4 >> 26;
        h4 = h4 & 0x3ffffff;
        h0 +%= c * 5;
        c = h0 >> 26;
        h0 = h0 & 0x3ffffff;
        h1 +%= c;
        // g = h + 5 - 2^130; use it if it did not go negative
        let mut g0 = h0 +% 5;
        c = g0 >> 26;
        g0 = g0 & 0x3ffffff;
        let mut g1 = h1 +% c;
        c = g1 >> 26;
        g1 = g1 & 0x3ffffff;
        let mut g2 = h2 +% c;
        c = g2 >> 26;
        g2 = g2 & 0x3ffffff;
        let mut g3 = h3 +% c;
        c = g3 >> 26;
        g3 = g3 & 0x3ffffff;
        let g4 = (h4 +% c) -% (1 << 26);
        let mask = (g4 >> 31) -% 1;  // all ones if g4 did not borrow
        h0 = (h0 & ~mask) | (g0 & mask);
        h1 = (h1 & ~mask) | (g1 & mask);
        h2 = (h2 & ~mask) | (g2 & mask);
        h3 = (h3 & ~mask) | (g3 & mask);
        h4 = (h4 & ~mask) | (g4 & mask);
        // h mod 2^128, plus the pad
        let w0 = h0 | (h1 << 26);
        let w1 = (h1 >> 6) | (h2 << 20);
        let w2 = (h2 >> 12) | (h3 << 14);
        let w3 = (h3 >> 18) | (h4 << 8);
        let mut f = @as(u64, w0) + @as(u64, self.pad[0]);
        put_le32(out[0..4], @truncate(u32, f));
        f = (@as(u64, w1) + @as(u64, self.pad[1])) + (f >> 32);
        put_le32(out[4..8], @truncate(u32, f));
        f = (@as(u64, w2) + @as(u64, self.pad[2])) + (f >> 32);
        put_le32(out[8..12], @truncate(u32, f));
        f = (@as(u64, w3) + @as(u64, self.pad[3])) + (f >> 32);
        put_le32(out[12..16], @truncate(u32, f));
    }
}

// -----------------------------------------------------------------------------
// The AEAD
// -----------------------------------------------------------------------------

fn pad16(p: *mut Poly1305, n: usize) void {
    let zeros: [16]u8 = [0; 16];
    if (n % 16) != 0 {
        p.update(zeros[..16 - (n % 16)]);
    }
}

fn compute_tag(key: []u8, nonce: []u8, aad: []u8, ct: []u8, tag: []mut u8) void {
    let mut otk: [64]u8 = [0; 64];
    block(key, 0, nonce, otk[..]);
    let mut p = Poly1305.init(otk[..32]);
    p.update(aad);
    pad16(&mut p, aad.len);
    p.update(ct);
    pad16(&mut p, ct.len);
    let mut lens: [16]u8 = [0; 16];
    let al = @as(u64, aad.len);
    let cl = @as(u64, ct.len);
    for i in @as(usize, 0)..8 {
        lens[i] = @truncate(u8, al >> @truncate(u32, 8 * i));
        lens[8 + i] = @truncate(u8, cl >> @truncate(u32, 8 * i));
    }
    p.update(lens[..]);
    p.final(tag);
}

/// Encrypts plaintext into out (plaintext.len + 16 bytes: the ciphertext,
/// then the tag), authenticating aad as well.
pub fn seal(key: []u8, nonce: []u8, aad: []u8, plaintext: []u8, out: []mut u8) !void {
    if (key.len != KEY_LEN) or (nonce.len != NONCE_LEN) or (out.len != (plaintext.len + TAG_LEN)) or
       too_long(plaintext.len) {
        return error.InvalidLength;
    }
    let ct = out[..plaintext.len];
    xor_stream(key, 1, nonce, plaintext, ct);
    compute_tag(key, nonce, aad, ct, out[plaintext.len..]);
}

/// Checks and decrypts sealed (ciphertext then tag) into out
/// (sealed.len - 16 bytes). Fails with AuthFailed, leaving out zeroed, if
/// anything was altered.
pub fn open(key: []u8, nonce: []u8, aad: []u8, sealed: []u8, out: []mut u8) !void {
    if (key.len != KEY_LEN) or (nonce.len != NONCE_LEN) or (sealed.len < TAG_LEN) or
       (out.len != (sealed.len - TAG_LEN)) or too_long(sealed.len - TAG_LEN) {
        return error.InvalidLength;
    }
    let ct = sealed[..sealed.len - TAG_LEN];
    let mut tag: [16]u8 = [0; 16];
    compute_tag(key, nonce, aad, ct, tag[..]);
    if not subtle.equal(tag[..], sealed[ct.len..]) {
        @mem_set(out, 0);
        return error.AuthFailed;
    }
    xor_stream(key, 1, nonce, ct, out);
}

// -----------------------------------------------------------------------------
// Tests (RFC 8439)
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

test "chacha20 block (2.3.2)" {
    let mut key: [32]u8 = [0; 32];
    for i in @as(usize, 0)..32 {
        key[i] = @truncate(u8, i);
    }
    let mut n: [12]u8 = [0; 12];
    let nonce = unhex("000000090000004a00000000", n[..]);
    let mut out: [64]u8 = [0; 64];
    block(key[..], 1, nonce, out[..]);
    let mut w: [64]u8 = [0; 64];
    @expect_eq(out[..],
               unhex("10f1e7e4d13b5915500fdd1fa32071c4c7d1f4c733c068030422aa9ac3d46c4ed2826446079faa0914c2d705d98b02a2b5129cd1de164eb9cbd083e8a2503c4e",
                     w[..]));
}

test "poly1305 (2.5.2)" {
    let mut k: [32]u8 = [0; 32];
    let key = unhex("85d6be7857556d337f4452fe42d506a80103808afb0db2fd4abff6af4149f51b", k[..]);
    let mut p = Poly1305.init(key);
    p.update("Cryptographic Forum Research Group");
    let mut tag: [16]u8 = [0; 16];
    p.final(tag[..]);
    let mut w: [16]u8 = [0; 16];
    @expect_eq(tag[..], unhex("a8061dc1305136c6c22b8baf0c0127a9", w[..]));
}

test "aead (2.8.2)" {
    let pt =
        "Ladies and Gentlemen of the class of '99: If I could offer you only one tip for the future, sunscreen would be it.";
    let mut k: [32]u8 = [0; 32];
    let key = unhex("808182838485868788898a8b8c8d8e8f909192939495969798999a9b9c9d9e9f", k[..]);
    let mut n: [12]u8 = [0; 12];
    let nonce = unhex("070000004041424344454647", n[..]);
    let mut a: [12]u8 = [0; 12];
    let aad = unhex("50515253c0c1c2c3c4c5c6c7", a[..]);
    let mut out: [130]u8 = [0; 130];
    seal(key, nonce, aad, pt, out[..])!;
    let mut w: [130]u8 = [0; 130];
    let want =
        unhex("d31a8d34648e60db7b86afbc53ef7ec2a4aded51296e08fea9e2b5a736ee62d63dbea45e8ca9671282fafb69da92728b1a71de0a9e060b2905d6a5b67ecd3b3692ddbd7f2d778b8c9803aee328091b58fab324e4fad675945585808b4831d7bc3ff4def08e4b7a9de576d26586cec64b61161ae10b594f09e26a7e902ecbd0600691",
              w[..]);
    @expect_eq(out[..], want);
    let mut back: [114]u8 = [0; 114];
    open(key, nonce, aad, out[..], back[..])!;
    @expect_eq(back[..], pt);
    out[5] ^= 1;
    open(key, nonce, aad, out[..], back[..]) catch |e| {
        @expect_eq(e, error.AuthFailed);
        return;
    };
    @trap();
}
