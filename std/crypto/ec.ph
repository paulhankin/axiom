// =============================================================================
// std/crypto/ec.ph - the NIST curves P-256 and P-384: ECDH and ECDSA
// (FIPS 186-4, SEC 1).
//
//     let k = ec.P256.generate()!;
//     k.public_key(pub[..]);                    // 65 bytes: 04 || X || Y
//     k.ecdh(their_public, shared[..])!;        // 32 bytes
//     k.sign(digest, sig[..])!;                 // r || s, 64 bytes
//     ec.verify_p256(pub, digest, sig)          // bool
//
// and likewise ec.P384 and ec.verify_p384 (97, 48 and 96 bytes).
//
// Arithmetic modulo the field prime and the group order is Montgomery
// multiplication on 32-bit limbs with masked final subtractions, and points
// are added with the complete formulas of Renes, Costello and Batina
// (2016), so the scalar multiplication (a Montgomery ladder with masked
// swaps) runs in time independent of the scalar. Both curves share the
// code, generic over the number of limbs.
// =============================================================================

import "../sync.ph" as sync;
import "random.ph" as random;
import "subtle.ph" as subtle;

error { InvalidPoint, InvalidLength, InvalidKey, RandomFailed }

// -----------------------------------------------------------------------------
// Montgomery arithmetic modulo an odd number of N 32-bit limbs
// -----------------------------------------------------------------------------

/// N little-endian 32-bit limbs.
struct U[const N: usize] {
    w: [N]u32,
}

fn u_zero[const N: usize]() U.[N] {
    return U.[N].{ w: [0; N] };
}

fn u_from_be[const N: usize](b: []u8) U.[N] {
    let mut r = u_zero.[N]();
    for i in @as(usize, 0)..N {
        let o = ((4 * N) - 4) - (4 * i);
        r.w[i] = (((@as(u32, b[o]) << 24) | (@as(u32, b[o + 1]) << 16)) | (@as(u32, b[o + 2]) << 8)) | @as(
            u32, b[o + 3]);
    }
    return r;
}

fn u_to_be[const N: usize](a: *U.[N], out: []mut u8) void {
    for i in @as(usize, 0)..N {
        let o = ((4 * N) - 4) - (4 * i);
        let v = a.w[i];
        out[o] = @truncate(u8, v >> 24);
        out[o + 1] = @truncate(u8, v >> 16);
        out[o + 2] = @truncate(u8, v >> 8);
        out[o + 3] = @truncate(u8, v);
    }
}

/// a - b and the borrow (1 if a < b).
fn u_sub[const N: usize](a: *U.[N], b: *U.[N], out: *mut U.[N]) u32 {
    let mut borrow: u64 = 0;
    for i in @as(usize, 0)..N {
        let d = (@as(u64, a.w[i]) -% @as(u64, b.w[i])) -% borrow;
        out.w[i] = @truncate(u32, d);
        borrow = (d >> 32) & 1;
    }
    return @truncate(u32, borrow);
}

fn u_add[const N: usize](a: *U.[N], b: *U.[N], out: *mut U.[N]) u32 {
    let mut c: u64 = 0;
    for i in @as(usize, 0)..N {
        let s = (@as(u64, a.w[i]) + @as(u64, b.w[i])) + c;
        out.w[i] = @truncate(u32, s);
        c = s >> 32;
    }
    return @truncate(u32, c);
}

/// a if mask is all ones, else b.
fn u_select[const N: usize](mask: u32, a: *U.[N], b: *U.[N]) U.[N] {
    let mut r = u_zero.[N]();
    for i in @as(usize, 0)..N {
        r.w[i] = (a.w[i] & mask) | (b.w[i] & ~mask);
    }
    return r;
}

fn u_is_zero[const N: usize](a: *U.[N]) bool {
    let mut x: u32 = 0;
    for v in a.w {
        x = x | v;
    }
    return x == 0;
}

/// Arithmetic modulo m in Montgomery form (R = 2^(32N)).
struct Mod[const N: usize] {
    m: U.[N],
    minv: u32,   // -m^-1 mod 2^32
    r2: U.[N],   // R^2 mod m
    one: U.[N],  // R mod m: 1 in Montgomery form

    /// (a + b) mod m for a, b < m.
    fn add(self: *Self, a: *U.[N], b: *U.[N]) U.[N] {
        let mut s = u_zero.[N]();
        let carry = u_add(a, b, &mut s);
        let mut d = u_zero.[N]();
        let borrow = u_sub(&s, &self.m, &mut d);
        // keep s only if it was already below m (a borrow and no carry)
        let keep = borrow & (carry ^ 1);
        return u_select(0 -% keep, &s, &d);
    }

    /// (a - b) mod m for a, b < m.
    fn sub(self: *Self, a: *U.[N], b: *U.[N]) U.[N] {
        let mut d = u_zero.[N]();
        let borrow = u_sub(a, b, &mut d);
        let mut e = u_zero.[N]();
        _ = u_add(&d, &self.m, &mut e);
        return u_select(0 -% borrow, &e, &d);
    }

    /// a * b * R^-1 mod m (CIOS), in constant time.
    fn mul(self: *Self, a: *U.[N], b: *U.[N]) U.[N] {
        let mut t: [14]u32 = [0; 14];  // N + 2 limbs used
        for i in @as(usize, 0)..N {
            let mut c: u64 = 0;
            let bi = @as(u64, b.w[i]);
            for j in @as(usize, 0)..N {
                let s = ((@as(u64, a.w[j]) * bi) + @as(u64, t[j])) + c;
                t[j] = @truncate(u32, s);
                c = s >> 32;
            }
            let s1 = @as(u64, t[N]) + c;
            t[N] = @truncate(u32, s1);
            t[N + 1] = @truncate(u32, s1 >> 32);
            let u = @as(u64, t[0] *% self.minv);
            let s2 = (u * @as(u64, self.m.w[0])) + @as(u64, t[0]);
            c = s2 >> 32;
            for j in @as(usize, 1)..N {
                let s = ((u * @as(u64, self.m.w[j])) + @as(u64, t[j])) + c;
                t[j - 1] = @truncate(u32, s);
                c = s >> 32;
            }
            let s3 = @as(u64, t[N]) + c;
            t[N - 1] = @truncate(u32, s3);
            t[N] = t[N + 1] + @truncate(u32, s3 >> 32);
        }
        let mut r = u_zero.[N]();
        for j in @as(usize, 0)..N {
            r.w[j] = t[j];
        }
        let mut d = u_zero.[N]();
        let borrow = u_sub(&r, &self.m, &mut d);
        // r < m exactly when the subtraction borrowed and t[N] is zero
        let keep = borrow & @truncate(u32, ((@as(u64, t[N]) -% 1) >> 63) & 1);
        return u_select(0 -% keep, &r, &d);
    }

    fn to_mont(self: *Self, a: *U.[N]) U.[N] {
        return self.mul(a, &self.r2);
    }

    fn from_mont(self: *Self, a: *U.[N]) U.[N] {
        let mut one = u_zero.[N]();
        one.w[0] = 1;
        return self.mul(a, &one);
    }

    /// a^(m-2): the inverse for a prime m (0 for 0). The exponent is public,
    /// so the sequence of operations is fixed.
    fn inv(self: *Self, a: *U.[N]) U.[N] {
        let mut e = u_zero.[N]();
        let mut two = u_zero.[N]();
        two.w[0] = 2;
        _ = u_sub(&self.m, &two, &mut e);
        let mut r = self.one;
        let mut i: usize = 32 * N;
        while i > 0 {
            i -= 1;
            r = self.mul(&r, &r);
            let bit = (e.w[i / 32] >> @truncate(u32, i % 32)) & 1;
            let t = self.mul(&r, a);
            r = u_select(0 -% bit, &t, &r);
        }
        return r;
    }

    /// Reduces a value below 2^(32N) into [0, m) (one conditional
    /// subtraction suffices for the NIST moduli, all above 2^(32N - 1)).
    fn reduce(self: *Self, a: *U.[N]) U.[N] {
        let mut d = u_zero.[N]();
        let borrow = u_sub(a, &self.m, &mut d);
        return u_select(0 -% borrow, a, &d);
    }
}

fn mod_init[const N: usize](m: U.[N]) Mod.[N] {
    let mut x: u32 = 1;
    for i in @as(u32, 0)..5 {
        x = x *% (2 -% (m.w[0] *% x));
    }
    let mut md = Mod.[N].{ m: m, minv: 0 -% x, r2: u_zero.[N](), one: u_zero.[N]() };
    // R mod m and R^2 mod m by doubling 1 (the modulus is public)
    let mut v = u_zero.[N]();
    v.w[0] = 1;
    for i in @as(usize, 0)..(64 * N) {
        v = md.add(&v, &v);
        if i == ((32 * N) - 1) {
            md.one = v;
        }
    }
    md.r2 = v;
    return md;
}

// -----------------------------------------------------------------------------
// The curve y^2 = x^3 - 3x + b over GF(p)
// -----------------------------------------------------------------------------

/// Projective coordinates (X : Y : Z), in Montgomery form; the point at
/// infinity is (0 : 1 : 0).
struct Point[const N: usize] {
    x: U.[N],
    y: U.[N],
    z: U.[N],
}

struct Curve[const N: usize] {
    fp: Mod.[N],
    fn_: Mod.[N],
    b: U.[N],  // Montgomery form
    g: Point.[N],
}

let P256_P: [32]u8 = [
    0xff,
    0xff,
    0xff,
    0xff,
    0x00,
    0x00,
    0x00,
    0x01,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0xff,
    0xff,
    0xff,
    0xff,
    0xff,
    0xff,
    0xff,
    0xff,
    0xff,
    0xff,
    0xff,
    0xff,
];
let P256_N: [32]u8 = [
    0xff,
    0xff,
    0xff,
    0xff,
    0x00,
    0x00,
    0x00,
    0x00,
    0xff,
    0xff,
    0xff,
    0xff,
    0xff,
    0xff,
    0xff,
    0xff,
    0xbc,
    0xe6,
    0xfa,
    0xad,
    0xa7,
    0x17,
    0x9e,
    0x84,
    0xf3,
    0xb9,
    0xca,
    0xc2,
    0xfc,
    0x63,
    0x25,
    0x51,
];
let P256_B: [32]u8 = [
    0x5a,
    0xc6,
    0x35,
    0xd8,
    0xaa,
    0x3a,
    0x93,
    0xe7,
    0xb3,
    0xeb,
    0xbd,
    0x55,
    0x76,
    0x98,
    0x86,
    0xbc,
    0x65,
    0x1d,
    0x06,
    0xb0,
    0xcc,
    0x53,
    0xb0,
    0xf6,
    0x3b,
    0xce,
    0x3c,
    0x3e,
    0x27,
    0xd2,
    0x60,
    0x4b,
];
let P256_GX: [32]u8 = [
    0x6b,
    0x17,
    0xd1,
    0xf2,
    0xe1,
    0x2c,
    0x42,
    0x47,
    0xf8,
    0xbc,
    0xe6,
    0xe5,
    0x63,
    0xa4,
    0x40,
    0xf2,
    0x77,
    0x03,
    0x7d,
    0x81,
    0x2d,
    0xeb,
    0x33,
    0xa0,
    0xf4,
    0xa1,
    0x39,
    0x45,
    0xd8,
    0x98,
    0xc2,
    0x96,
];
let P256_GY: [32]u8 = [
    0x4f,
    0xe3,
    0x42,
    0xe2,
    0xfe,
    0x1a,
    0x7f,
    0x9b,
    0x8e,
    0xe7,
    0xeb,
    0x4a,
    0x7c,
    0x0f,
    0x9e,
    0x16,
    0x2b,
    0xce,
    0x33,
    0x57,
    0x6b,
    0x31,
    0x5e,
    0xce,
    0xcb,
    0xb6,
    0x40,
    0x68,
    0x37,
    0xbf,
    0x51,
    0xf5,
];

let P384_P: [48]u8 = [
    0xff,
    0xff,
    0xff,
    0xff,
    0xff,
    0xff,
    0xff,
    0xff,
    0xff,
    0xff,
    0xff,
    0xff,
    0xff,
    0xff,
    0xff,
    0xff,
    0xff,
    0xff,
    0xff,
    0xff,
    0xff,
    0xff,
    0xff,
    0xff,
    0xff,
    0xff,
    0xff,
    0xff,
    0xff,
    0xff,
    0xff,
    0xfe,
    0xff,
    0xff,
    0xff,
    0xff,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0xff,
    0xff,
    0xff,
    0xff,
];
let P384_N: [48]u8 = [
    0xff,
    0xff,
    0xff,
    0xff,
    0xff,
    0xff,
    0xff,
    0xff,
    0xff,
    0xff,
    0xff,
    0xff,
    0xff,
    0xff,
    0xff,
    0xff,
    0xff,
    0xff,
    0xff,
    0xff,
    0xff,
    0xff,
    0xff,
    0xff,
    0xc7,
    0x63,
    0x4d,
    0x81,
    0xf4,
    0x37,
    0x2d,
    0xdf,
    0x58,
    0x1a,
    0x0d,
    0xb2,
    0x48,
    0xb0,
    0xa7,
    0x7a,
    0xec,
    0xec,
    0x19,
    0x6a,
    0xcc,
    0xc5,
    0x29,
    0x73,
];
let P384_B: [48]u8 = [
    0xb3,
    0x31,
    0x2f,
    0xa7,
    0xe2,
    0x3e,
    0xe7,
    0xe4,
    0x98,
    0x8e,
    0x05,
    0x6b,
    0xe3,
    0xf8,
    0x2d,
    0x19,
    0x18,
    0x1d,
    0x9c,
    0x6e,
    0xfe,
    0x81,
    0x41,
    0x12,
    0x03,
    0x14,
    0x08,
    0x8f,
    0x50,
    0x13,
    0x87,
    0x5a,
    0xc6,
    0x56,
    0x39,
    0x8d,
    0x8a,
    0x2e,
    0xd1,
    0x9d,
    0x2a,
    0x85,
    0xc8,
    0xed,
    0xd3,
    0xec,
    0x2a,
    0xef,
];
let P384_GX: [48]u8 = [
    0xaa,
    0x87,
    0xca,
    0x22,
    0xbe,
    0x8b,
    0x05,
    0x37,
    0x8e,
    0xb1,
    0xc7,
    0x1e,
    0xf3,
    0x20,
    0xad,
    0x74,
    0x6e,
    0x1d,
    0x3b,
    0x62,
    0x8b,
    0xa7,
    0x9b,
    0x98,
    0x59,
    0xf7,
    0x41,
    0xe0,
    0x82,
    0x54,
    0x2a,
    0x38,
    0x55,
    0x02,
    0xf2,
    0x5d,
    0xbf,
    0x55,
    0x29,
    0x6c,
    0x3a,
    0x54,
    0x5e,
    0x38,
    0x72,
    0x76,
    0x0a,
    0xb7,
];
let P384_GY: [48]u8 = [
    0x36,
    0x17,
    0xde,
    0x4a,
    0x96,
    0x26,
    0x2c,
    0x6f,
    0x5d,
    0x9e,
    0x98,
    0xbf,
    0x92,
    0x92,
    0xdc,
    0x29,
    0xf8,
    0xf4,
    0x1d,
    0xbd,
    0x28,
    0x9a,
    0x14,
    0x7c,
    0xe9,
    0xda,
    0x31,
    0x13,
    0xb5,
    0xf0,
    0xb8,
    0xc0,
    0x0a,
    0x60,
    0xb1,
    0xce,
    0x1d,
    0x7e,
    0x81,
    0x9d,
    0x7a,
    0x43,
    0x1d,
    0x7c,
    0x90,
    0xea,
    0x0e,
    0x5f,
];

fn make_curve[const N: usize](p: []u8, n: []u8, b: []u8, gx: []u8, gy: []u8) Curve.[N] {
    let fp = mod_init(u_from_be.[N](p));
    let fnn = mod_init(u_from_be.[N](n));
    let bb = u_from_be.[N](b);
    let x = u_from_be.[N](gx);
    let y = u_from_be.[N](gy);
    return Curve.[N].{
        fp: fp, fn_: fnn, b: fp.to_mont(&bb), g: Point.[N].{ x: fp.to_mont(&x), y: fp.to_mont(&y), z: fp.one } };
}

// The curves, computed on first use (racing threads store the same values).
let mut CURVE256: Curve.[8] = Curve.[8].{
    fp: Mod.[8].{ m: U.[8].{ w: [0; 8] }, minv: 0, r2: U.[8].{ w: [0; 8] }, one: U.[8].{ w: [0; 8] } },
    fn_: Mod.[8].{ m: U.[8].{ w: [0; 8] }, minv: 0, r2: U.[8].{ w: [0; 8] }, one: U.[8].{ w: [0; 8] } },
    b: U.[8].{ w: [0; 8] },
    g: Point.[8].{ x: U.[8].{ w: [0; 8] }, y: U.[8].{ w: [0; 8] }, z: U.[8].{ w: [0; 8] } },
};
let mut CURVE384: Curve.[12] = Curve.[12].{
    fp: Mod.[12].{ m: U.[12].{ w: [0; 12] }, minv: 0, r2: U.[12].{ w: [0; 12] }, one: U.[12].{ w: [0; 12] } },
    fn_: Mod.[12].{ m: U.[12].{ w: [0; 12] }, minv: 0, r2: U.[12].{ w: [0; 12] }, one: U.[12].{ w: [0; 12] } },
    b: U.[12].{ w: [0; 12] },
    g: Point.[12].{ x: U.[12].{ w: [0; 12] }, y: U.[12].{ w: [0; 12] }, z: U.[12].{ w: [0; 12] } },
};
let mut ONCE256: sync.Once = sync.Once.{ state: 0 };
let mut ONCE384: sync.Once = sync.Once.{ state: 0 };

fn build256(_u: u8) void {
    CURVE256 = make_curve.[8](P256_P[..], P256_N[..], P256_B[..], P256_GX[..], P256_GY[..]);
}

fn build384(_u: u8) void {
    CURVE384 = make_curve.[12](P384_P[..], P384_N[..], P384_B[..], P384_GX[..], P384_GY[..]);
}

fn p256() *Curve.[8] {
    ONCE256.call(build256, @as(u8, 0));
    return &CURVE256;
}

fn p384() *Curve.[12] {
    ONCE384.call(build384, @as(u8, 0));
    return &CURVE384;
}

fn infinity[const N: usize](c: *Curve.[N]) Point.[N] {
    return Point.[N].{ x: u_zero.[N](), y: c.fp.one, z: u_zero.[N]() };
}

/// P + Q by Algorithm 4 of Renes, Costello and Batina: complete (right for
/// doubling and the point at infinity too) on curves with a = -3.
fn point_add[const N: usize](c: *Curve.[N], p: *Point.[N], q: *Point.[N]) Point.[N] {
    let f = &c.fp;
    let mut t0 = f.mul(&p.x, &q.x);
    let mut t1 = f.mul(&p.y, &q.y);
    let mut t2 = f.mul(&p.z, &q.z);
    let mut t3 = f.add(&p.x, &p.y);
    let mut t4 = f.add(&q.x, &q.y);
    t3 = f.mul(&t3, &t4);
    t4 = f.add(&t0, &t1);
    t3 = f.sub(&t3, &t4);
    t4 = f.add(&p.y, &p.z);
    let mut x3 = f.add(&q.y, &q.z);
    t4 = f.mul(&t4, &x3);
    x3 = f.add(&t1, &t2);
    t4 = f.sub(&t4, &x3);
    x3 = f.add(&p.x, &p.z);
    let mut y3 = f.add(&q.x, &q.z);
    x3 = f.mul(&x3, &y3);
    y3 = f.add(&t0, &t2);
    y3 = f.sub(&x3, &y3);
    let mut z3 = f.mul(&c.b, &t2);
    x3 = f.sub(&y3, &z3);
    z3 = f.add(&x3, &x3);
    x3 = f.add(&x3, &z3);
    z3 = f.sub(&t1, &x3);
    x3 = f.add(&t1, &x3);
    y3 = f.mul(&c.b, &y3);
    t1 = f.add(&t2, &t2);
    t2 = f.add(&t1, &t2);
    y3 = f.sub(&y3, &t2);
    y3 = f.sub(&y3, &t0);
    t1 = f.add(&y3, &y3);
    y3 = f.add(&t1, &y3);
    t1 = f.add(&t0, &t0);
    t0 = f.add(&t1, &t0);
    t0 = f.sub(&t0, &t2);
    t1 = f.mul(&t4, &y3);
    t2 = f.mul(&t0, &y3);
    y3 = f.mul(&x3, &z3);
    y3 = f.add(&y3, &t2);
    x3 = f.mul(&t3, &x3);
    x3 = f.sub(&x3, &t1);
    z3 = f.mul(&t4, &z3);
    t1 = f.mul(&t3, &t0);
    z3 = f.add(&z3, &t1);
    return Point.[N].{ x: x3, y: y3, z: z3 };
}

fn point_cswap[const N: usize](a: *mut Point.[N], b: *mut Point.[N], bit: u32) void {
    let mask = 0 -% bit;
    let na = Point.[N].{ x: u_select(mask, &b.x, &a.x), y: u_select(mask, &b.y, &a.y), z: u_select(mask, &b.z, &a.z) };
    let nb = Point.[N].{ x: u_select(mask, &a.x, &b.x), y: u_select(mask, &a.y, &b.y), z: u_select(mask, &a.z, &b.z) };
    a.* = na;
    b.* = nb;
}

/// [k]P for a scalar (not in Montgomery form), as a constant-time ladder.
fn scalar_mul[const N: usize](c: *Curve.[N], k: *U.[N], p: *Point.[N]) Point.[N] {
    let mut r0 = infinity(c);
    let mut r1 = p.*;
    let mut i: usize = 32 * N;
    while i > 0 {
        i -= 1;
        let bit = (k.w[i / 32] >> @truncate(u32, i % 32)) & 1;
        point_cswap(&mut r0, &mut r1, bit);
        r1 = point_add(c, &r0, &r1);
        r0 = point_add(c, &r0, &r0);
        point_cswap(&mut r0, &mut r1, bit);
    }
    return r0;
}

/// The affine x and y (normal form), or InvalidPoint for infinity.
fn to_affine[const N: usize](c: *Curve.[N], p: *Point.[N], x: *mut U.[N], y: *mut U.[N]) !void {
    if u_is_zero(&p.z) {
        return error.InvalidPoint;
    }
    let zi = c.fp.inv(&p.z);
    let ax = c.fp.mul(&p.x, &zi);
    let ay = c.fp.mul(&p.y, &zi);
    x.* = c.fp.from_mont(&ax);
    y.* = c.fp.from_mont(&ay);
}

/// Parses an uncompressed point (04 || X || Y) and checks it is on the curve.
fn decode_point[const N: usize](c: *Curve.[N], b: []u8) !Point.[N] {
    if (b.len != (1 + (8 * N))) or (b[0] != 4) {
        return error.InvalidPoint;
    }
    let x = u_from_be.[N](b[1..1 + (4 * N)]);
    let y = u_from_be.[N](b[1 + (4 * N)..1 + (8 * N)]);
    let mut t = u_zero.[N]();
    if (u_sub(&x, &c.fp.m, &mut t) == 0) or (u_sub(&y, &c.fp.m, &mut t) == 0) {
        return error.InvalidPoint;  // not below p
    }
    let f = &c.fp;
    let mx = f.to_mont(&x);
    let my = f.to_mont(&y);
    // y^2 == x^3 - 3x + b
    let y2 = f.mul(&my, &my);
    let x2 = f.mul(&mx, &mx);
    let x3 = f.mul(&x2, &mx);
    let tx = f.add(&mx, &mx);
    let three_x = f.add(&tx, &mx);
    let r0 = f.sub(&x3, &three_x);
    let rhs = f.add(&r0, &c.b);
    let mut d = u_zero.[N]();
    _ = u_sub(&y2, &rhs, &mut d);
    if not u_is_zero(&d) {
        return error.InvalidPoint;
    }
    return Point.[N].{ x: mx, y: my, z: f.one };
}

fn encode_point[const N: usize](c: *Curve.[N], p: *Point.[N], out: []mut u8) !void {
    let mut x = u_zero.[N]();
    let mut y = u_zero.[N]();
    to_affine(c, p, &mut x, &mut y)!;
    out[0] = 4;
    u_to_be(&x, out[1..1 + (4 * N)]);
    u_to_be(&y, out[1 + (4 * N)..1 + (8 * N)]);
}

// -----------------------------------------------------------------------------
// Keys, ECDH and ECDSA (generic over the curve)
// -----------------------------------------------------------------------------

/// Whether 0 < k < n.
fn scalar_ok[const N: usize](c: *Curve.[N], k: *U.[N]) bool {
    let mut t = u_zero.[N]();
    return (not u_is_zero(k)) and (u_sub(k, &c.fn_.m, &mut t) == 1);
}

/// A random scalar in [1, n).
fn random_scalar[const N: usize](c: *Curve.[N]) !U.[N] {
    loop {
        let mut b: [48]u8 = [0; 48];
        random.fill(b[..4 * N]) catch return error.RandomFailed;
        let k = u_from_be.[N](b[..4 * N]);
        if scalar_ok(c, &k) {
            return k;
        }
    }
}

fn scalar_from_bytes[const N: usize](c: *Curve.[N], b: []u8) !U.[N] {
    if b.len != (4 * N) {
        return error.InvalidLength;
    }
    let d = u_from_be.[N](b);
    if not scalar_ok(c, &d) {
        return error.InvalidKey;
    }
    return d;
}

fn public_of[const N: usize](c: *Curve.[N], d: *U.[N], out: []mut u8) void {
    let q = scalar_mul(c, d, &c.g);
    encode_point(c, &q, out[..1 + (8 * N)]) catch @trap();
}

fn ecdh_of[const N: usize](c: *Curve.[N], d: *U.[N], public: []u8, out: []mut u8) !void {
    let q = decode_point(c, public)!;
    let s = scalar_mul(c, d, &q);
    let mut x = u_zero.[N]();
    let mut y = u_zero.[N]();
    to_affine(c, &s, &mut x, &mut y)!;
    u_to_be(&x, out[..4 * N]);
}

/// The digest as a scalar: its leftmost 32N bits, reduced mod n.
fn digest_scalar[const N: usize](c: *Curve.[N], digest: []u8) U.[N] {
    let mut b: [48]u8 = [0; 48];
    let len = 4 * N;
    if digest.len >= len {
        @mem_copy(b[..len], digest[..len]);
    } else {
        @mem_copy(b[len - digest.len..len], digest);
    }
    let e = u_from_be.[N](b[..len]);
    return c.fn_.reduce(&e);
}

fn sign_with[const N: usize](c: *Curve.[N], d: *U.[N], digest: []u8, sig: []mut u8) !void {
    let n = &c.fn_;
    let e = digest_scalar(c, digest);
    loop {
        let k = random_scalar(c)!;
        let kg = scalar_mul(c, &k, &c.g);
        let mut x = u_zero.[N]();
        let mut y = u_zero.[N]();
        to_affine(c, &kg, &mut x, &mut y)!;
        let r = n.reduce(&x);
        if u_is_zero(&r) {
            continue;
        }
        // s = k^-1 (e + r d) mod n, in Montgomery form throughout
        let km = n.to_mont(&k);
        let kinv = n.inv(&km);
        let rm = n.to_mont(&r);
        let dm = n.to_mont(d);
        let em = n.to_mont(&e);
        let rd = n.mul(&rm, &dm);
        let sum = n.add(&em, &rd);
        let sm = n.mul(&kinv, &sum);
        let s = n.from_mont(&sm);
        if u_is_zero(&s) {
            continue;
        }
        u_to_be(&r, sig[0..4 * N]);
        u_to_be(&s, sig[4 * N..8 * N]);
        return;
    }
}

fn verify_with[const N: usize](c: *Curve.[N], public: []u8, digest: []u8, sig: []u8) bool {
    if sig.len != (8 * N) {
        return false;
    }
    let q = decode_point(c, public) catch return false;
    let r = u_from_be.[N](sig[0..4 * N]);
    let s = u_from_be.[N](sig[4 * N..8 * N]);
    if (not scalar_ok(c, &r)) or not scalar_ok(c, &s) {
        return false;
    }
    let n = &c.fn_;
    let e = digest_scalar(c, digest);
    let sm = n.to_mont(&s);
    let w = n.inv(&sm);
    let em = n.to_mont(&e);
    let rm = n.to_mont(&r);
    let u1m = n.mul(&em, &w);
    let u2m = n.mul(&rm, &w);
    let u1 = n.from_mont(&u1m);
    let u2 = n.from_mont(&u2m);
    let a = scalar_mul(c, &u1, &c.g);
    let b = scalar_mul(c, &u2, &q);
    let sum = point_add(c, &a, &b);
    let mut x = u_zero.[N]();
    let mut y = u_zero.[N]();
    to_affine(c, &sum, &mut x, &mut y) catch return false;
    let xr = n.reduce(&x);
    let mut rb: [48]u8 = [0; 48];
    let mut xb: [48]u8 = [0; 48];
    u_to_be(&r, rb[..4 * N]);
    u_to_be(&xr, xb[..4 * N]);
    return subtle.equal(rb[..4 * N], xb[..4 * N]);
}

// -----------------------------------------------------------------------------
// P-256 and P-384
// -----------------------------------------------------------------------------

/// A P-256 private key. Public keys are uncompressed points (65 bytes:
/// 04 || X || Y), shared secrets 32 bytes, signatures r || s (64 bytes).
pub struct P256 {
    d: U.[8],

    /// The key with this 32-byte big-endian scalar (1 <= d < n).
    pub fn from_bytes(b: []u8) !P256 {
        return P256.{ d: scalar_from_bytes(p256(), b)! };
    }

    pub fn generate() !P256 {
        return P256.{ d: random_scalar(p256())! };
    }

    pub fn to_bytes(self: *P256, out: []mut u8) void {
        u_to_be(&self.d, out[..32]);
    }

    pub fn public_key(self: *P256, out: []mut u8) void {
        public_of(p256(), &self.d, out);
    }

    /// The x coordinate of d * Q for an uncompressed public key Q.
    pub fn ecdh(self: *P256, public: []u8, out: []mut u8) !void {
        ecdh_of(p256(), &self.d, public, out)!;
    }

    /// Signs a digest (its leftmost 256 bits) with a fresh random nonce.
    pub fn sign(self: *P256, digest: []u8, sig: []mut u8) !void {
        sign_with(p256(), &self.d, digest, sig)!;
    }
}

/// A P-384 private key: public keys are 97 bytes, shared secrets 48 bytes,
/// signatures 96 bytes.
pub struct P384 {
    d: U.[12],

    pub fn from_bytes(b: []u8) !P384 {
        return P384.{ d: scalar_from_bytes(p384(), b)! };
    }

    pub fn generate() !P384 {
        return P384.{ d: random_scalar(p384())! };
    }

    pub fn to_bytes(self: *P384, out: []mut u8) void {
        u_to_be(&self.d, out[..48]);
    }

    pub fn public_key(self: *P384, out: []mut u8) void {
        public_of(p384(), &self.d, out);
    }

    pub fn ecdh(self: *P384, public: []u8, out: []mut u8) !void {
        ecdh_of(p384(), &self.d, public, out)!;
    }

    pub fn sign(self: *P384, digest: []u8, sig: []mut u8) !void {
        sign_with(p384(), &self.d, digest, sig)!;
    }
}

/// Whether sig (r || s, 64 bytes) is a P-256 signature of digest by the
/// uncompressed public key.
pub fn verify_p256(public: []u8, digest: []u8, sig: []u8) bool {
    return verify_with(p256(), public, digest, sig);
}

/// The same for P-384 (97-byte keys, 96-byte signatures).
pub fn verify_p384(public: []u8, digest: []u8, sig: []u8) bool {
    return verify_with(p384(), public, digest, sig);
}

// -----------------------------------------------------------------------------
// Tests
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

test "public key and ecdh (RFC 5903 8.1)" {
    let mut b: [65]u8 = [0; 65];
    let mut w: [65]u8 = [0; 65];
    let i = P256.from_bytes(unhex("c88f01f510d9ac3f70a292daa2316de544e9aab8afe84049c62a9c57862d1433", b[..]))!;
    let mut gi: [65]u8 = [0; 65];
    i.public_key(gi[..]);
    @expect_eq(gi[..],
               unhex("04dad0b65394221cf9b051e1feca5787d098dfe637fc90b9ef945d0c37725811805271a0461cdb8252d61f1c456fa3e59ab1f45b33accf5f58389e0577b8990bb3",
                     w[..]));
    let r = P256.from_bytes(unhex("c6ef9c5d78ae012a011164acb397ce2088685d8f06bf9be0b283ab46476bee53", b[..]))!;
    let mut gr: [65]u8 = [0; 65];
    r.public_key(gr[..]);
    let mut s1: [32]u8 = [0; 32];
    let mut s2: [32]u8 = [0; 32];
    i.ecdh(gr[..], s1[..])!;
    r.ecdh(gi[..], s2[..])!;
    @expect_eq(s1[..], unhex("d6840f6b42f6edafd13116e0e12565202fef8e9ece7dce03812464d04b9442de", w[..]));
    @expect_eq(s2[..], s1[..]);
    gr[40] ^= 1;
    i.ecdh(gr[..], s1[..]) catch |e| {
        @expect_eq(e, error.InvalidPoint);
        return;
    };
    @trap();
}

test "ecdsa" {
    let k = P256.generate()!;
    let mut pubk: [65]u8 = [0; 65];
    k.public_key(pubk[..]);
    let digest: [32]u8 = [0x42; 32];
    let mut sig: [64]u8 = [0; 64];
    k.sign(digest[..], sig[..])!;
    @expect(verify_p256(pubk[..], digest[..], sig[..]));
    let other: [32]u8 = [0x43; 32];
    @expect(not verify_p256(pubk[..], other[..], sig[..]));
    // RFC 6979 A.2.5 (P-256, SHA-256, "sample"): a fixed signature verifies
    let mut pb: [65]u8 = [0; 65];
    let mut db: [32]u8 = [0; 32];
    let mut sb: [64]u8 = [0; 64];
    let q =
        unhex("0460fed4ba255a9d31c961eb74c6356d68c049b8923b61fa6ce669622e60f29fb67903fe1008b8bc99a41ae9e95628bc64f2f1b20c2d7e9f5177a3c294d4462299",
              pb[..]);
    let d = unhex("af2bdbe1aa9b6ec1e2ade1d694f41fc71a831d0268e9891562113d8a62add1bf", db[..]);
    let s =
        unhex("efd48b2aacb6a8fd1140dd9cd45e81d69d2c877b56aaf991c34d0ea84eaf3716f7cb1c942d657c41d436c7a1b6e29f65f3e900dbb9aff4064dc4ab2f843acda8",
              sb[..]);
    @expect(verify_p256(q, d, s));
    s[10] ^= 1;
    @expect(not verify_p256(q, d, s));
}
