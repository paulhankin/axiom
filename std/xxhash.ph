// =============================================================================
// std/xxhash.ph - xxHash: fast non-cryptographic checksums (XXH32 and XXH64).
//
//     let h = xxhash.xxh64(data, 0);
//
//     let mut s = xxhash.Xxh64.init(0);        // streaming
//     s.update(part1);
//     s.update(part2);
//     let h2 = s.digest();
//
// These are the checksums of the LZ4 and Zstandard formats, and good hashes
// for checking data or building tables when the inputs are not chosen by an
// adversary (std/hash keys its hashes for that).
// =============================================================================

const P32_1: u32 = 2654435761;
const P32_2: u32 = 2246822519;
const P32_3: u32 = 3266489917;
const P32_4: u32 = 668265263;
const P32_5: u32 = 374761393;

const P64_1: u64 = 11400714785074694791;
const P64_2: u64 = 14029467366897019727;
const P64_3: u64 = 1609587929392839161;
const P64_4: u64 = 9650029242287828579;
const P64_5: u64 = 2870177450012600261;

fn le32(b: []u8, i: usize) u32 {
    return @as(u32, b[i]) | (@as(u32, b[i + 1]) << 8) | (@as(u32, b[i + 2]) << 16) | (@as(u32, b[i + 3]) << 24);
}

fn le64(b: []u8, i: usize) u64 {
    return @as(u64, le32(b, i)) | (@as(u64, le32(b, i + 4)) << 32);
}

fn round32(acc: u32, x: u32) u32 {
    return @rotl(acc +% (x *% P32_2), 13) *% P32_1;
}

fn round64(acc: u64, x: u64) u64 {
    return @rotl(acc +% (x *% P64_2), 31) *% P64_1;
}

fn merge64(acc: u64, v: u64) u64 {
    return ((acc ^ round64(0, v)) *% P64_1) +% P64_4;
}

/// XXH32 of data.
pub fn xxh32(data: []u8, seed: u32) u32 {
    let mut s = Xxh32.init(seed);
    s.update(data);
    return s.digest();
}

/// XXH64 of data.
pub fn xxh64(data: []u8, seed: u64) u64 {
    let mut s = Xxh64.init(seed);
    s.update(data);
    return s.digest();
}

pub struct Xxh32 {
    v: [4]u32,
    buf: [16]u8,
    nbuf: usize,
    total: u64,
    seed: u32,

    pub fn init(seed: u32) Xxh32 {
        return Xxh32.{ v: [seed +% P32_1 +% P32_2, seed +% P32_2, seed, seed -% P32_1], buf: [0; 16], nbuf: 0, total: 0,
                       seed: seed };
    }

    fn stripe(self: *mut Xxh32, b: []u8, i: usize) void {
        self.v[0] = round32(self.v[0], le32(b, i));
        self.v[1] = round32(self.v[1], le32(b, i + 4));
        self.v[2] = round32(self.v[2], le32(b, i + 8));
        self.v[3] = round32(self.v[3], le32(b, i + 12));
    }

    pub fn update(self: *mut Xxh32, data: []u8) void {
        self.total += @as(u64, data.len);
        let mut i: usize = 0;
        if self.nbuf > 0 {
            while (self.nbuf < 16) and (i < data.len) {
                self.buf[self.nbuf] = data[i];
                self.nbuf += 1;
                i += 1;
            }
            if self.nbuf < 16 {
                return;
            }
            let b = self.buf;
            self.stripe(b[..], 0);
            self.nbuf = 0;
        }
        while (i + 16) <= data.len {
            self.stripe(data, i);
            i += 16;
        }
        while i < data.len {
            self.buf[self.nbuf] = data[i];
            self.nbuf += 1;
            i += 1;
        }
    }

    pub fn digest(self: *Xxh32) u32 {
        let mut h: u32 = 0;
        if self.total >= 16 {
            h = @rotl(self.v[0], 1) +% @rotl(self.v[1], 7) +% @rotl(self.v[2], 12) +% @rotl(self.v[3], 18);
        } else {
            h = self.seed +% P32_5;
        }
        h +%= @truncate(u32, self.total);
        let b = self.buf[..self.nbuf];
        let mut i: usize = 0;
        while (i + 4) <= b.len {
            h = @rotl(h +% (le32(b, i) *% P32_3), 17) *% P32_4;
            i += 4;
        }
        while i < b.len {
            h = @rotl(h +% (@as(u32, b[i]) *% P32_5), 11) *% P32_1;
            i += 1;
        }
        h ^= h >> 15;
        h *%= P32_2;
        h ^= h >> 13;
        h *%= P32_3;
        h ^= h >> 16;
        return h;
    }
}

pub struct Xxh64 {
    v: [4]u64,
    buf: [32]u8,
    nbuf: usize,
    total: u64,
    seed: u64,

    pub fn init(seed: u64) Xxh64 {
        return Xxh64.{ v: [seed +% P64_1 +% P64_2, seed +% P64_2, seed, seed -% P64_1], buf: [0; 32], nbuf: 0, total: 0,
                       seed: seed };
    }

    fn stripe(self: *mut Xxh64, b: []u8, i: usize) void {
        self.v[0] = round64(self.v[0], le64(b, i));
        self.v[1] = round64(self.v[1], le64(b, i + 8));
        self.v[2] = round64(self.v[2], le64(b, i + 16));
        self.v[3] = round64(self.v[3], le64(b, i + 24));
    }

    pub fn update(self: *mut Xxh64, data: []u8) void {
        self.total += @as(u64, data.len);
        let mut i: usize = 0;
        if self.nbuf > 0 {
            while (self.nbuf < 32) and (i < data.len) {
                self.buf[self.nbuf] = data[i];
                self.nbuf += 1;
                i += 1;
            }
            if self.nbuf < 32 {
                return;
            }
            let b = self.buf;
            self.stripe(b[..], 0);
            self.nbuf = 0;
        }
        while (i + 32) <= data.len {
            self.stripe(data, i);
            i += 32;
        }
        while i < data.len {
            self.buf[self.nbuf] = data[i];
            self.nbuf += 1;
            i += 1;
        }
    }

    pub fn digest(self: *Xxh64) u64 {
        let mut h: u64 = 0;
        if self.total >= 32 {
            let v = self.v;
            h = @rotl(v[0], 1) +% @rotl(v[1], 7) +% @rotl(v[2], 12) +% @rotl(v[3], 18);
            h = merge64(h, v[0]);
            h = merge64(h, v[1]);
            h = merge64(h, v[2]);
            h = merge64(h, v[3]);
        } else {
            h = self.seed +% P64_5;
        }
        h +%= self.total;
        let b = self.buf[..self.nbuf];
        let mut i: usize = 0;
        while (i + 8) <= b.len {
            h ^= round64(0, le64(b, i));
            h = (@rotl(h, 27) *% P64_1) +% P64_4;
            i += 8;
        }
        if (i + 4) <= b.len {
            h ^= @as(u64, le32(b, i)) *% P64_1;
            h = (@rotl(h, 23) *% P64_2) +% P64_3;
            i += 4;
        }
        while i < b.len {
            h ^= @as(u64, b[i]) *% P64_5;
            h = @rotl(h, 11) *% P64_1;
            i += 1;
        }
        h ^= h >> 33;
        h *%= P64_2;
        h ^= h >> 29;
        h *%= P64_3;
        h ^= h >> 32;
        return h;
    }
}

// -----------------------------------------------------------------------------
// Tests
// -----------------------------------------------------------------------------

struct Vector {
    n: usize,
    seed: u32,
    h32: u32,
    h64: u64,
}

// From the Python xxhash package, over bytes (i * 31 + 7) & 255.
let VECTORS: [30]Vector = [
    Vector.{ n: 0, seed: 0, h32: 0x02cc5d05, h64: 0xef46db3751d8e999 },
    Vector.{ n: 0, seed: 2654435761, h32: 0x36b78ae7, h64: 0xac75fda2929b17ef },
    Vector.{ n: 1, seed: 0, h32: 0x002e0d32, h64: 0xa96c7f0ce858bbb7 },
    Vector.{ n: 1, seed: 2654435761, h32: 0xd40fe509, h64: 0x84e535b36672440d },
    Vector.{ n: 3, seed: 0, h32: 0xaca17380, h64: 0x56e6957632a487f9 },
    Vector.{ n: 3, seed: 2654435761, h32: 0xed8936d0, h64: 0xd095a9239060f0a3 },
    Vector.{ n: 4, seed: 0, h32: 0x073faa82, h64: 0xc60d15b1e3ff8f04 },
    Vector.{ n: 4, seed: 2654435761, h32: 0x893f71b0, h64: 0xd4e4b946fb0bfbb4 },
    Vector.{ n: 15, seed: 0, h32: 0x9f29f87b, h64: 0xae2a37eb9357caa7 },
    Vector.{ n: 15, seed: 2654435761, h32: 0x22dc4620, h64: 0x283deb489df92be1 },
    Vector.{ n: 16, seed: 0, h32: 0x3f6c9665, h64: 0xa19ad429b02bc413 },
    Vector.{ n: 16, seed: 2654435761, h32: 0x17393d4a, h64: 0x06f2d4c8d982f096 },
    Vector.{ n: 17, seed: 0, h32: 0xe048ecdb, h64: 0xfe9f0feb7eeedc09 },
    Vector.{ n: 17, seed: 2654435761, h32: 0x5981b004, h64: 0xade2a507106baee8 },
    Vector.{ n: 31, seed: 0, h32: 0x00f1525c, h64: 0x4a74f3a1a39ad4a1 },
    Vector.{ n: 31, seed: 2654435761, h32: 0x2b78ecd4, h64: 0xb806f858c84789ea },
    Vector.{ n: 32, seed: 0, h32: 0xee42e668, h64: 0x8d57d6a4671cc43d },
    Vector.{ n: 32, seed: 2654435761, h32: 0xc01e797e, h64: 0x8ef2e38dde10f162 },
    Vector.{ n: 33, seed: 0, h32: 0x770b5126, h64: 0x62c9fd21ed857664 },
    Vector.{ n: 33, seed: 2654435761, h32: 0x1e57dd16, h64: 0x5a653650552f822c },
    Vector.{ n: 63, seed: 0, h32: 0x471925bb, h64: 0x5c320a0d2707057f },
    Vector.{ n: 63, seed: 2654435761, h32: 0x33baa8d9, h64: 0xcdec73bbccfd2d50 },
    Vector.{ n: 64, seed: 0, h32: 0xf2a797fa, h64: 0x7bbabbc45729d17e },
    Vector.{ n: 64, seed: 2654435761, h32: 0x589b1f20, h64: 0x861fd034d84dab20 },
    Vector.{ n: 100, seed: 0, h32: 0x75936eb8, h64: 0xefa0ad2d3e70c151 },
    Vector.{ n: 100, seed: 2654435761, h32: 0x81d373f5, h64: 0x4ac13414471c7284 },
    Vector.{ n: 299, seed: 0, h32: 0x49cced94, h64: 0x6c576b5c4a8cfa96 },
    Vector.{ n: 299, seed: 2654435761, h32: 0xaf6cbb4d, h64: 0x0e3be107c462b685 },
    Vector.{ n: 300, seed: 0, h32: 0xec3c8ce2, h64: 0x8d2ba0cd7fece76c },
    Vector.{ n: 300, seed: 2654435761, h32: 0xe829d7c1, h64: 0x88a87907c214d1bd },
];

test "xxhash" {
    let mut data: [300]u8 = [0; 300];
    for i in @as(usize, 0)..300 {
        data[i] = @truncate(u8, (i * 31) + 7);
    }
    for v in VECTORS {
        let d = data[..v.n];
        @expect_eq(xxh32(d, v.seed), v.h32);
        @expect_eq(xxh64(d, @as(u64, v.seed)), v.h64);
        // streaming, in uneven pieces
        let mut s32 = Xxh32.init(v.seed);
        let mut s64 = Xxh64.init(@as(u64, v.seed));
        let mut i: usize = 0;
        let mut step: usize = 1;
        while i < d.len {
            let e = @min(i + step, d.len);
            s32.update(d[i..e]);
            s64.update(d[i..e]);
            i = e;
            step = (step * 3) + 1;
        }
        @expect_eq(s32.digest(), v.h32);
        @expect_eq(s64.digest(), v.h64);
    }
}
