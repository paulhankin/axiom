// =============================================================================
// std/crypto/sha3.ph - SHA-3 and SHAKE (FIPS 202): the Keccak-f[1600]
// sponge.
//
//     let mut d: [32]u8 = [0; 32];
//     sha3.sha3_256(data, d[..]);
//
//     let mut x = sha3.Sha3.shake256();       // any output length
//     x.update(seed);
//     x.squeeze(out1[..]);
//     x.squeeze(out2[..]);                    // continues the stream
// =============================================================================

let RC: [24]u64 = [
    0x0000000000000001,
    0x0000000000008082,
    0x800000000000808a,
    0x8000000080008000,
    0x000000000000808b,
    0x0000000080000001,
    0x8000000080008081,
    0x8000000000008009,
    0x000000000000008a,
    0x0000000000000088,
    0x0000000080008009,
    0x000000008000000a,
    0x000000008000808b,
    0x800000000000008b,
    0x8000000000008089,
    0x8000000000008003,
    0x8000000000008002,
    0x8000000000000080,
    0x000000000000800a,
    0x800000008000000a,
    0x8000000080008081,
    0x8000000000008080,
    0x0000000080000001,
    0x8000000080008008,
];

// rho rotations and pi positions, in the order the lanes are visited
let ROT: [24]u64 = [1, 3, 6, 10, 15, 21, 28, 36, 45, 55, 2, 14, 27, 41, 56, 8, 25, 43, 62, 18, 39, 61, 20, 44];
let PI: [24]usize = [10, 7, 11, 17, 18, 3, 5, 16, 8, 21, 24, 4, 15, 23, 19, 13, 12, 2, 20, 14, 22, 9, 6, 1];

/// Keccak-f[1600] on the state.
pub fn keccak_f(st: *mut [25]u64) void {
    let mut bc: [5]u64 = [0; 5];
    for round in @as(usize, 0)..24 {
        // theta
        for i in @as(usize, 0)..5 {
            bc[i] = (((st[i] ^ st[i + 5]) ^ st[i + 10]) ^ st[i + 15]) ^ st[i + 20];
        }
        for i in @as(usize, 0)..5 {
            let t = bc[(i + 4) % 5] ^ @rotl(bc[(i + 1) % 5], 1);
            for j in [@as(usize, 0), 5, 10, 15, 20]{
                st[j + i] ^= t;
            }
        }
        // rho and pi
        let mut t = st[1];
        for i in @as(usize, 0)..24 {
            let j = PI[i];
            let b = st[j];
            st[j] = @rotl(t, ROT[i]);
            t = b;
        }
        // chi
        for j in [@as(usize, 0), 5, 10, 15, 20]{
            for i in @as(usize, 0)..5 {
                bc[i] = st[j + i];
            }
            for i in @as(usize, 0)..5 {
                st[j + i] ^= ~bc[(i + 1) % 5] & bc[(i + 2) % 5];
            }
        }
        // iota
        st[0] ^= RC[round];
    }
}

pub struct Sha3 {
    st: [25]u64,
    rate: usize,  // bytes absorbed per permutation
    pos: usize,   // in the current block
    dsbyte: u8,   // 0x06 for SHA-3, 0x1f for SHAKE
    out_len: usize,
    squeezing: bool,

    fn with(rate: usize, dsbyte: u8, out_len: usize) Sha3 {
        return Sha3.{ st: [0; 25], rate: rate, pos: 0, dsbyte: dsbyte, out_len: out_len, squeezing: false };
    }

    pub fn sha3_224() Sha3 {
        return Sha3.with(144, 0x06, 28);
    }

    pub fn sha3_256() Sha3 {
        return Sha3.with(136, 0x06, 32);
    }

    pub fn sha3_384() Sha3 {
        return Sha3.with(104, 0x06, 48);
    }

    pub fn sha3_512() Sha3 {
        return Sha3.with(72, 0x06, 64);
    }

    /// SHAKE128: any output length, 128-bit security.
    pub fn shake128() Sha3 {
        return Sha3.with(168, 0x1f, 0);
    }

    /// SHAKE256: any output length, 256-bit security.
    pub fn shake256() Sha3 {
        return Sha3.with(136, 0x1f, 0);
    }

    /// The digest size of the SHA-3 functions (0 for SHAKE).
    pub fn size(self: *Sha3) usize {
        return self.out_len;
    }

    fn xor_byte(self: *mut Sha3, i: usize, b: u8) void {
        self.st[i / 8] ^= @as(u64, b) << @truncate(u64, 8 * (i % 8));
    }

    pub fn update(self: *mut Sha3, data: []u8) void {
        if self.squeezing {
            @trap();  // absorbing after squeezing is a programming error
        }
        let mut i: usize = 0;
        // whole lanes when aligned
        while i < data.len {
            if ((self.pos % 8) == 0) and ((i + 8) <= data.len) and ((self.pos + 8) <= self.rate) {
                let d = data[i..i + 8];
                let w = (
                    (((((@as(u64, d[0]) | (@as(u64, d[1]) << 8)) | (@as(u64, d[2]) << 16)) | (@as(u64,
                                                                                                  d[3]) << 24)) |
                      (@as(u64, d[4]) << 32)) | (@as(u64, d[5]) << 40)) | (@as(u64, d[6]) << 48)) | (
                          @as(u64, d[7]) << 56);
                self.st[self.pos / 8] ^= w;
                self.pos += 8;
                i += 8;
            } else {
                self.xor_byte(self.pos, data[i]);
                self.pos += 1;
                i += 1;
            }
            if self.pos == self.rate {
                keccak_f(&mut self.st);
                self.pos = 0;
            }
        }
    }

    fn pad(self: *mut Sha3) void {
        self.xor_byte(self.pos, self.dsbyte);
        self.xor_byte(self.rate - 1, 0x80);
        keccak_f(&mut self.st);
        self.pos = 0;
        self.squeezing = true;
    }

    /// Writes the next out.len bytes of output (SHAKE: call as often as
    /// needed; the SHA-3 functions: once, via final).
    pub fn squeeze(self: *mut Sha3, out: []mut u8) void {
        if not self.squeezing {
            self.pad();
        }
        for k in @as(usize, 0)..out.len {
            if self.pos == self.rate {
                keccak_f(&mut self.st);
                self.pos = 0;
            }
            out[k] = @truncate(u8, self.st[self.pos / 8] >> @truncate(u64, 8 * (self.pos % 8)));
            self.pos += 1;
        }
    }

    /// Writes the digest (size() bytes) to out.
    pub fn final(self: *mut Sha3, out: []mut u8) void {
        self.squeeze(out[..self.out_len]);
    }
}

pub fn sha3_224(data: []u8, out: []mut u8) void {
    let mut h = Sha3.sha3_224();
    h.update(data);
    h.final(out);
}

pub fn sha3_256(data: []u8, out: []mut u8) void {
    let mut h = Sha3.sha3_256();
    h.update(data);
    h.final(out);
}

pub fn sha3_384(data: []u8, out: []mut u8) void {
    let mut h = Sha3.sha3_384();
    h.update(data);
    h.final(out);
}

pub fn sha3_512(data: []u8, out: []mut u8) void {
    let mut h = Sha3.sha3_512();
    h.update(data);
    h.final(out);
}

/// SHAKE128 of data, out.len bytes of it.
pub fn shake128(data: []u8, out: []mut u8) void {
    let mut h = Sha3.shake128();
    h.update(data);
    h.squeeze(out);
}

/// SHAKE256 of data, out.len bytes of it.
pub fn shake256(data: []u8, out: []mut u8) void {
    let mut h = Sha3.shake256();
    h.update(data);
    h.squeeze(out);
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

test "sha-3 and shake vectors" {
    let mut d: [64]u8 = [0; 64];
    let mut h: [128]u8 = [0; 128];
    sha3_224("abc", d[..28]);
    @expect_eq(hex(d[..28], h[..]), "e642824c3f8cf24ad09234ee7d3c766fc9a3a5168d0c94ad73b46fdf");
    sha3_256("abc", d[..32]);
    @expect_eq(hex(d[..32], h[..]), "3a985da74fe225b2045c172d6bd390bd855f086e3e9d525b46bfe24511431532");
    sha3_384("abc", d[..48]);
    @expect_eq(hex(d[..48], h[..]),
               "ec01498288516fc926459f58e2c6ad8df9b473cb0fc08c2596da7cf0e49be4b298d88cea927ac7f539f1edf228376d25");
    sha3_512("abc", d[..]);
    @expect_eq(hex(d[..], h[..]),
               "b751850b1a57168a5693cd924b6b096e08f621827444f70d884f5d0240d2712e10e116e9192af3c91a7ec57647e3934057340b4cf408d5a56592f8274eec53f0");
    sha3_256("", d[..32]);
    @expect_eq(hex(d[..32], h[..]), "a7ffc6f8bf1ed76651c14756a061d662f580ff4de43b49fa82d80a4b80f8434a");
    let a200: [200]u8 = ['a'; 200];
    sha3_256(a200[..], d[..32]);
    @expect_eq(hex(d[..32], h[..]), "cce34485baf2bf2aca99b94833892a4f52896d3d153f7b840cc4f9fe695f1387");
    shake128("abc", d[..40]);
    @expect_eq(hex(d[..40], h[..]), "5881092dd818bf5cf8a3ddb793fbcba74097d5c526a6d35f97b83351940f2cc844c50af32acd3f2c");
    // squeezing in pieces gives the same stream
    let mut x = Sha3.shake128();
    x.update("ab");
    x.update("c");
    let mut p: [40]u8 = [0; 40];
    x.squeeze(p[..3]);
    x.squeeze(p[3..40]);
    @expect_eq(p[..], d[..40]);
    shake256("", d[..32]);
    @expect_eq(hex(d[..32], h[..]), "46b9dd2b0ba88d13233b3feb743eeb243fcd52ea62b81b82b50c27646ed5762f");
    let mut m = Sha3.sha3_512();
    let chunk: [101]u8 = ['a'; 101];
    let mut left: usize = 1000000;
    while left > 0 {
        let n = @min(left, 101);
        m.update(chunk[..n]);
        left -= n;
    }
    m.final(d[..]);
    @expect_eq(hex(d[..], h[..]),
               "3c3a876da14034ab60627c077bb98f7e120a2a5370212dffb3385a18d4f38859ed311d0a9d5141ce9cc5c66ee689b266a8aa18ace8282a0e0db596c90b0a7b87");
}
