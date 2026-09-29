// =============================================================================
// std/crypto/sha1.ph - SHA-1 (FIPS 180-4).
//
// SHA-1 is broken for collision resistance: use it only where a protocol
// requires it (WebSocket handshakes, HMAC-SHA1 in TOTP, git object names),
// never for new signatures or integrity checks.
//
//     let mut d: [20]u8 = [0; 20];
//     sha1.sha1(data, d[..]);
// =============================================================================

pub const SIZE: usize = 20;
pub const BLOCK: usize = 64;

pub struct Sha1 {
    h: [5]u32,
    buf: [64]u8,
    nbuf: usize,
    total: u64,

    pub fn init() Sha1 {
        return Sha1.{ h: [0x67452301, 0xefcdab89, 0x98badcfe, 0x10325476, 0xc3d2e1f0], buf: [0; 64], nbuf: 0,
                      total: 0 };
    }

    fn block(self: *mut Sha1, p: []u8) void {
        let mut w: [80]u32 = [0; 80];
        for i in @as(usize, 0)..16 {
            let k = 4 * i;
            w[i] = (((@as(u32, p[k]) << 24) | (@as(u32, p[k + 1]) << 16)) | (@as(u32, p[k + 2]) << 8)) | @as(
                u32, p[k + 3]);
        }
        for i in @as(usize, 16)..80 {
            w[i] = @rotl(((w[i - 3] ^ w[i - 8]) ^ w[i - 14]) ^ w[i - 16], 1);
        }
        let mut a = self.h[0];
        let mut b = self.h[1];
        let mut c = self.h[2];
        let mut d = self.h[3];
        let mut e = self.h[4];
        for i in @as(usize, 0)..80 {
            let mut f: u32 = 0;
            let mut k: u32 = 0;
            if i < 20 {
                f = (b & c) | (~b & d);
                k = 0x5a827999;
            } else if i < 40 {
                f = (b ^ c) ^ d;
                k = 0x6ed9eba1;
            } else if i < 60 {
                f = ((b & c) | (b & d)) | (c & d);
                k = 0x8f1bbcdc;
            } else {
                f = (b ^ c) ^ d;
                k = 0xca62c1d6;
            }
            let t = (((@rotl(a, 5) +% f) +% e) +% k) +% w[i];
            e = d;
            d = c;
            c = @rotl(b, 30);
            b = a;
            a = t;
        }
        self.h[0] +%= a;
        self.h[1] +%= b;
        self.h[2] +%= c;
        self.h[3] +%= d;
        self.h[4] +%= e;
    }

    pub fn update(self: *mut Sha1, data: []u8) void {
        self.total +%= @as(u64, data.len);
        let mut d = data;
        if self.nbuf > 0 {
            let k = @min(64 - self.nbuf, d.len);
            @mem_copy(self.buf[self.nbuf..self.nbuf + k], d[..k]);
            self.nbuf += k;
            d = d[k..];
            if self.nbuf < 64 {
                return;
            }
            let b = self.buf;
            self.block(b[..]);
            self.nbuf = 0;
        }
        while d.len >= 64 {
            self.block(d[..64]);
            d = d[64..];
        }
        @mem_copy(self.buf[..d.len], d);
        self.nbuf = d.len;
    }

    /// Writes the 20-byte digest to out.
    pub fn final(self: *mut Sha1, out: []mut u8) void {
        let bits = self.total *% 8;
        let pad: [64]u8 = [0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                           0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                           0, 0, 0];
        let mut n = 56 - self.nbuf;
        if self.nbuf >= 56 {
            n = 120 - self.nbuf;
        }
        let total = self.total;
        self.update(pad[..@min(n, 64)]);
        if n > 64 {
            self.update(pad[1..1 + (n - 64)]);
        }
        let mut len: [8]u8 = [0; 8];
        for i in @as(usize, 0)..8 {
            len[i] = @truncate(u8, bits >> @truncate(u64, 56 - (8 * i)));
        }
        self.update(len[..]);
        self.total = total;
        for i in @as(usize, 0)..5 {
            out[4 * i] = @truncate(u8, self.h[i] >> 24);
            out[(4 * i) + 1] = @truncate(u8, self.h[i] >> 16);
            out[(4 * i) + 2] = @truncate(u8, self.h[i] >> 8);
            out[(4 * i) + 3] = @truncate(u8, self.h[i]);
        }
    }
}

/// One-shot SHA-1 of data into out (20 bytes).
pub fn sha1(data: []u8, out: []mut u8) void {
    let mut h = Sha1.init();
    h.update(data);
    h.final(out);
}

// -----------------------------------------------------------------------------
// Tests
// -----------------------------------------------------------------------------

fn hex(d: []u8, out: []mut u8) []u8 {
    let digits = "0123456789abcdef";
    for i in @as(usize, 0)..d.len {
        out[2 * i] = digits[@as(usize, d[i] >> 4)];
        out[(2 * i) + 1] = digits[@as(usize, d[i] & 15)];
    }
    return out[..2 * d.len];
}

test "sha-1 vectors" {
    let mut d: [20]u8 = [0; 20];
    let mut h: [40]u8 = [0; 40];
    sha1("abc", d[..]);
    @expect_eq(hex(d[..], h[..]), "a9993e364706816aba3e25717850c26c9cd0d89d");
    sha1("", d[..]);
    @expect_eq(hex(d[..], h[..]), "da39a3ee5e6b4b0d3255bfef95601890afd80709");
    sha1("abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq", d[..]);
    @expect_eq(hex(d[..], h[..]), "84983e441c3bd26ebaae4aa1f95129e5e54670f1");
    // a million a's, fed unevenly
    let mut s = Sha1.init();
    let chunk: [97]u8 = ['a'; 97];
    let mut left: usize = 1000000;
    while left > 0 {
        let n = @min(left, 97);
        s.update(chunk[..n]);
        left -= n;
    }
    s.final(d[..]);
    @expect_eq(hex(d[..], h[..]), "34aa973cd4c4daa4f61eeb2bdbad27316534016f");
}
