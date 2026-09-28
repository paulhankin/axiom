// =============================================================================
// std/crypto/p256.ph - the NIST P-256 curve: ECDH and ECDSA (FIPS 186-4,
// SEC 1).
//
//     let k = p256.PrivateKey.generate()!;
//     k.public_key(pub[..]);                       // 65 bytes: 04 || X || Y
//     k.ecdh(their_public, shared[..])!;           // 32 bytes
//     k.sign(digest, sig[..])!;                    // r || s, 64 bytes
//     p256.verify(pub, digest, sig)                // bool
//
// Arithmetic modulo the field prime and the group order is Montgomery
// multiplication on eight 32-bit limbs with masked final subtractions, and
// points are added with the complete formulas of Renes, Costello and Batina
// (2016), so the scalar multiplication (a Montgomery ladder with masked
// swaps) runs in time independent of the scalar.
// =============================================================================

import "random.ph" as random;
import "subtle.ph" as subtle;

error { InvalidPoint, InvalidLength, InvalidKey, RandomFailed }

// -----------------------------------------------------------------------------
// Montgomery arithmetic modulo a 256-bit odd number
// -----------------------------------------------------------------------------

/// Eight little-endian 32-bit limbs.
struct U256 {
    w: [8]u32,
}

fn u_zero() U256 {
    return U256.{ w: [0; 8] };
}

fn u_from_be(b: []u8) U256 {
    let mut r = u_zero();
    for i in @as(usize, 0)..8 {
        let o = 28 - (4 * i);
        r.w[i] = (((@as(u32, b[o]) << 24) | (@as(u32, b[o + 1]) << 16)) | (@as(u32, b[o + 2]) << 8)) | @as(
            u32, b[o + 3]);
    }
    return r;
}

fn u_to_be(a: *U256, out: []mut u8) void {
    for i in @as(usize, 0)..8 {
        let o = 28 - (4 * i);
        let v = a.w[i];
        out[o] = @truncate(u8, v >> 24);
        out[o + 1] = @truncate(u8, v >> 16);
        out[o + 2] = @truncate(u8, v >> 8);
        out[o + 3] = @truncate(u8, v);
    }
}

/// a - b and the borrow (1 if a < b).
fn u_sub(a: *U256, b: *U256, out: *mut U256) u32 {
    let mut borrow: u64 = 0;
    for i in @as(usize, 0)..8 {
        let d = (@as(u64, a.w[i]) -% @as(u64, b.w[i])) -% borrow;
        out.w[i] = @truncate(u32, d);
        borrow = (d >> 32) & 1;
    }
    return @truncate(u32, borrow);
}

fn u_add(a: *U256, b: *U256, out: *mut U256) u32 {
    let mut c: u64 = 0;
    for i in @as(usize, 0)..8 {
        let s = (@as(u64, a.w[i]) + @as(u64, b.w[i])) + c;
        out.w[i] = @truncate(u32, s);
        c = s >> 32;
    }
    return @truncate(u32, c);
}

/// a if mask is all ones, else b.
fn u_select(mask: u32, a: *U256, b: *U256) U256 {
    let mut r = u_zero();
    for i in @as(usize, 0)..8 {
        r.w[i] = (a.w[i] & mask) | (b.w[i] & ~mask);
    }
    return r;
}

fn u_is_zero(a: *U256) bool {
    let mut x: u32 = 0;
    for v in a.w {
        x = x | v;
    }
    return x == 0;
}

/// Arithmetic modulo m in Montgomery form (R = 2^256).
struct Mod {
    m: U256,
    minv: u32,  // -m^-1 mod 2^32
    r2: U256,   // R^2 mod m
    one: U256,  // R mod m: 1 in Montgomery form

    /// (a + b) mod m for a, b < m.
    fn add(self: *Mod, a: *U256, b: *U256) U256 {
        let mut s = u_zero();
        let carry = u_add(a, b, &mut s);
        let mut d = u_zero();
        let borrow = u_sub(&s, &self.m, &mut d);
        // keep s only if it was already below m (a borrow and no carry)
        let keep = borrow & (carry ^ 1);
        return u_select(0 -% keep, &s, &d);
    }

    /// (a - b) mod m for a, b < m.
    fn sub(self: *Mod, a: *U256, b: *U256) U256 {
        let mut d = u_zero();
        let borrow = u_sub(a, b, &mut d);
        let mut e = u_zero();
        _ = u_add(&d, &self.m, &mut e);
        return u_select(0 -% borrow, &e, &d);
    }

    /// a * b * R^-1 mod m (CIOS), in constant time.
    fn mul(self: *Mod, a: *U256, b: *U256) U256 {
        let mut t: [10]u32 = [0; 10];
        for i in @as(usize, 0)..8 {
            let mut c: u64 = 0;
            let bi = @as(u64, b.w[i]);
            for j in @as(usize, 0)..8 {
                let s = ((@as(u64, a.w[j]) * bi) + @as(u64, t[j])) + c;
                t[j] = @truncate(u32, s);
                c = s >> 32;
            }
            let s1 = @as(u64, t[8]) + c;
            t[8] = @truncate(u32, s1);
            t[9] = @truncate(u32, s1 >> 32);
            let u = @as(u64, t[0] *% self.minv);
            let s2 = (u * @as(u64, self.m.w[0])) + @as(u64, t[0]);
            c = s2 >> 32;
            for j in @as(usize, 1)..8 {
                let s = ((u * @as(u64, self.m.w[j])) + @as(u64, t[j])) + c;
                t[j - 1] = @truncate(u32, s);
                c = s >> 32;
            }
            let s3 = @as(u64, t[8]) + c;
            t[7] = @truncate(u32, s3);
            t[8] = t[9] + @truncate(u32, s3 >> 32);
        }
        let mut r = u_zero();
        for j in @as(usize, 0)..8 {
            r.w[j] = t[j];
        }
        let mut d = u_zero();
        let borrow = u_sub(&r, &self.m, &mut d);
        // r < m exactly when the subtraction borrowed and t[8] is zero
        let keep = borrow & @truncate(u32, ((@as(u64, t[8]) -% 1) >> 63) & 1);
        return u_select(0 -% keep, &r, &d);
    }

    fn to_mont(self: *Mod, a: *U256) U256 {
        return self.mul(a, &self.r2);
    }

    fn from_mont(self: *Mod, a: *U256) U256 {
        let mut one = u_zero();
        one.w[0] = 1;
        return self.mul(a, &one);
    }

    /// a^(m-2): the inverse for a prime m (0 for 0). The exponent is public,
    /// so the sequence of operations is fixed.
    fn inv(self: *Mod, a: *U256) U256 {
        let mut e = u_zero();
        let mut two = u_zero();
        two.w[0] = 2;
        _ = u_sub(&self.m, &two, &mut e);
        let mut r = self.one;
        let mut i: usize = 256;
        while i > 0 {
            i -= 1;
            r = self.mul(&r, &r);
            let bit = (e.w[i / 32] >> @truncate(u32, i % 32)) & 1;
            let t = self.mul(&r, a);
            r = u_select(0 -% bit, &t, &r);
        }
        return r;
    }

    /// Reduces a value below 2^256 into [0, m) (one conditional subtraction
    /// suffices for the P-256 moduli, both above 2^255).
    fn reduce(self: *Mod, a: *U256) U256 {
        let mut d = u_zero();
        let borrow = u_sub(a, &self.m, &mut d);
        return u_select(0 -% borrow, a, &d);
    }
}

fn mod_init(m: U256) Mod {
    let mut x: u32 = 1;
    for i in @as(u32, 0)..5 {
        x = x *% (2 -% (m.w[0] *% x));
    }
    let mut md = Mod.{ m: m, minv: 0 -% x, r2: u_zero(), one: u_zero() };
    // R mod m and R^2 mod m by doubling 1 (the modulus is public)
    let mut v = u_zero();
    v.w[0] = 1;
    for i in @as(usize, 0)..512 {
        v = md.add(&v, &v);
        if i == 255 {
            md.one = v;
        }
    }
    md.r2 = v;
    return md;
}

// -----------------------------------------------------------------------------
// The curve y^2 = x^3 - 3x + b over GF(p)
// -----------------------------------------------------------------------------

let P_BE: [32]u8 = [0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                    0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff];
let N_BE: [32]u8 = [0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
                    0xbc, 0xe6, 0xfa, 0xad, 0xa7, 0x17, 0x9e, 0x84, 0xf3, 0xb9, 0xca, 0xc2, 0xfc, 0x63, 0x25, 0x51];
let B_BE: [32]u8 = [0x5a, 0xc6, 0x35, 0xd8, 0xaa, 0x3a, 0x93, 0xe7, 0xb3, 0xeb, 0xbd, 0x55, 0x76, 0x98, 0x86, 0xbc,
                    0x65, 0x1d, 0x06, 0xb0, 0xcc, 0x53, 0xb0, 0xf6, 0x3b, 0xce, 0x3c, 0x3e, 0x27, 0xd2, 0x60, 0x4b];
let GX_BE: [32]u8 = [0x6b, 0x17, 0xd1, 0xf2, 0xe1, 0x2c, 0x42, 0x47, 0xf8, 0xbc, 0xe6, 0xe5, 0x63, 0xa4, 0x40, 0xf2,
                     0x77, 0x03, 0x7d, 0x81, 0x2d, 0xeb, 0x33, 0xa0, 0xf4, 0xa1, 0x39, 0x45, 0xd8, 0x98, 0xc2, 0x96];
let GY_BE: [32]u8 = [0x4f, 0xe3, 0x42, 0xe2, 0xfe, 0x1a, 0x7f, 0x9b, 0x8e, 0xe7, 0xeb, 0x4a, 0x7c, 0x0f, 0x9e, 0x16,
                     0x2b, 0xce, 0x33, 0x57, 0x6b, 0x31, 0x5e, 0xce, 0xcb, 0xb6, 0x40, 0x68, 0x37, 0xbf, 0x51, 0xf5];

/// Projective coordinates (X : Y : Z), in Montgomery form; the point at
/// infinity is (0 : 1 : 0).
struct Point {
    x: U256,
    y: U256,
    z: U256,
}

struct Curve {
    fp: Mod,
    fn_: Mod,
    b: U256,  // Montgomery form
    g: Point,
}

let mut CURVE: Curve = Curve.{
    fp: Mod.{ m: U256.{ w: [0; 8] }, minv: 0, r2: U256.{ w: [0; 8] }, one: U256.{ w: [0; 8] } },
    fn_: Mod.{ m: U256.{ w: [0; 8] }, minv: 0, r2: U256.{ w: [0; 8] }, one: U256.{ w: [0; 8] } },
    b: U256.{ w: [0; 8] },
    g: Point.{ x: U256.{ w: [0; 8] }, y: U256.{ w: [0; 8] }, z: U256.{ w: [0; 8] } },
};
let mut CURVE_READY: u32 = 0;

fn curve() *Curve {
    if @atomic_load(&CURVE_READY) == 0 {
        // racing threads compute and store the same values
        let fp = mod_init(u_from_be(P_BE[..]));
        let fnn = mod_init(u_from_be(N_BE[..]));
        let b = u_from_be(B_BE[..]);
        let gx = u_from_be(GX_BE[..]);
        let gy = u_from_be(GY_BE[..]);
        CURVE = Curve.{ fp: fp, fn_: fnn, b: fp.to_mont(&b), g: Point.{ x: fp.to_mont(&gx), y: fp.to_mont(&gy),
                                                                        z: fp.one } };
        @atomic_store(&mut CURVE_READY, 1);
    }
    return &CURVE;
}

fn infinity(c: *Curve) Point {
    return Point.{ x: u_zero(), y: c.fp.one, z: u_zero() };
}

/// P + Q by Algorithm 4 of Renes, Costello and Batina: complete (right for
/// doubling and the point at infinity too) on curves with a = -3.
fn point_add(c: *Curve, p: *Point, q: *Point) Point {
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
    return Point.{ x: x3, y: y3, z: z3 };
}

fn point_cswap(a: *mut Point, b: *mut Point, bit: u32) void {
    let mask = 0 -% bit;
    let na = Point.{ x: u_select(mask, &b.x, &a.x), y: u_select(mask, &b.y, &a.y), z: u_select(mask, &b.z, &a.z) };
    let nb = Point.{ x: u_select(mask, &a.x, &b.x), y: u_select(mask, &a.y, &b.y), z: u_select(mask, &a.z, &b.z) };
    a.* = na;
    b.* = nb;
}

/// [k]P for a scalar (not in Montgomery form), as a constant-time ladder.
fn scalar_mul(c: *Curve, k: *U256, p: *Point) Point {
    let mut r0 = infinity(c);
    let mut r1 = p.*;
    let mut i: usize = 256;
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
fn to_affine(c: *Curve, p: *Point, x: *mut U256, y: *mut U256) !void {
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
fn decode_point(c: *Curve, b: []u8) !Point {
    if (b.len != 65) or (b[0] != 4) {
        return error.InvalidPoint;
    }
    let x = u_from_be(b[1..33]);
    let y = u_from_be(b[33..65]);
    let mut t = u_zero();
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
    let mut d = u_zero();
    _ = u_sub(&y2, &rhs, &mut d);
    if not u_is_zero(&d) {
        return error.InvalidPoint;
    }
    return Point.{ x: mx, y: my, z: f.one };
}

fn encode_point(c: *Curve, p: *Point, out: []mut u8) !void {
    let mut x = u_zero();
    let mut y = u_zero();
    to_affine(c, p, &mut x, &mut y)!;
    out[0] = 4;
    u_to_be(&x, out[1..33]);
    u_to_be(&y, out[33..65]);
}

// -----------------------------------------------------------------------------
// Keys, ECDH and ECDSA
// -----------------------------------------------------------------------------

/// Whether 0 < k < n.
fn scalar_ok(c: *Curve, k: *U256) bool {
    let mut t = u_zero();
    return (not u_is_zero(k)) and (u_sub(k, &c.fn_.m, &mut t) == 1);
}

/// A random scalar in [1, n).
fn random_scalar(c: *Curve) !U256 {
    loop {
        let mut b: [32]u8 = [0; 32];
        random.fill(b[..]) catch return error.RandomFailed;
        let k = u_from_be(b[..]);
        if scalar_ok(c, &k) {
            return k;
        }
    }
}

pub struct PrivateKey {
    d: U256,

    /// A key from its 32-byte big-endian scalar (1 <= d < n).
    pub fn from_bytes(b: []u8) !PrivateKey {
        if b.len != 32 {
            return error.InvalidLength;
        }
        let c = curve();
        let d = u_from_be(b);
        if not scalar_ok(c, &d) {
            return error.InvalidKey;
        }
        return PrivateKey.{ d: d };
    }

    pub fn generate() !PrivateKey {
        return PrivateKey.{ d: random_scalar(curve())! };
    }

    pub fn to_bytes(self: *PrivateKey, out: []mut u8) void {
        u_to_be(&self.d, out[..32]);
    }

    /// The public key, uncompressed: 04 || X || Y (65 bytes).
    pub fn public_key(self: *PrivateKey, out: []mut u8) void {
        let c = curve();
        let q = scalar_mul(c, &self.d, &c.g);
        encode_point(c, &q, out[..65]) catch @trap();
    }

    /// The shared secret with an uncompressed public key: the x coordinate
    /// of d * Q (32 bytes).
    pub fn ecdh(self: *PrivateKey, public: []u8, out: []mut u8) !void {
        let c = curve();
        let q = decode_point(c, public)!;
        let s = scalar_mul(c, &self.d, &q);
        let mut x = u_zero();
        let mut y = u_zero();
        to_affine(c, &s, &mut x, &mut y)!;
        u_to_be(&x, out[..32]);
    }

    /// Signs a message digest (its leftmost 256 bits are used) with a fresh
    /// random nonce; the signature is r || s, 64 bytes.
    pub fn sign(self: *PrivateKey, digest: []u8, sig: []mut u8) !void {
        let c = curve();
        let n = &c.fn_;
        let e = digest_scalar(c, digest);
        loop {
            let k = random_scalar(c)!;
            let kg = scalar_mul(c, &k, &c.g);
            let mut x = u_zero();
            let mut y = u_zero();
            to_affine(c, &kg, &mut x, &mut y)!;
            let r = n.reduce(&x);
            if u_is_zero(&r) {
                continue;
            }
            // s = k^-1 (e + r d) mod n, in Montgomery form throughout
            let km = n.to_mont(&k);
            let kinv = n.inv(&km);
            let rm = n.to_mont(&r);
            let dm = n.to_mont(&self.d);
            let em = n.to_mont(&e);
            let rd = n.mul(&rm, &dm);
            let sum = n.add(&em, &rd);
            let sm = n.mul(&kinv, &sum);
            let s = n.from_mont(&sm);
            if u_is_zero(&s) {
                continue;
            }
            u_to_be(&r, sig[0..32]);
            u_to_be(&s, sig[32..64]);
            return;
        }
    }
}

/// The digest as a scalar: its leftmost 256 bits, reduced mod n.
fn digest_scalar(c: *Curve, digest: []u8) U256 {
    let mut b: [32]u8 = [0; 32];
    if digest.len >= 32 {
        @mem_copy(b[..], digest[..32]);
    } else {
        @mem_copy(b[32 - digest.len..], digest);
    }
    let e = u_from_be(b[..]);
    return c.fn_.reduce(&e);
}

/// Whether sig (r || s, 64 bytes) is a valid signature of digest by the
/// uncompressed public key.
pub fn verify(public: []u8, digest: []u8, sig: []u8) bool {
    if sig.len != 64 {
        return false;
    }
    let c = curve();
    let q = decode_point(c, public) catch return false;
    let r = u_from_be(sig[0..32]);
    let s = u_from_be(sig[32..64]);
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
    let mut x = u_zero();
    let mut y = u_zero();
    to_affine(c, &sum, &mut x, &mut y) catch return false;
    let xr = n.reduce(&x);
    let mut rb: [32]u8 = [0; 32];
    let mut xb: [32]u8 = [0; 32];
    u_to_be(&r, rb[..]);
    u_to_be(&xr, xb[..]);
    return subtle.equal(rb[..], xb[..]);
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
    let i = PrivateKey.from_bytes(unhex("c88f01f510d9ac3f70a292daa2316de544e9aab8afe84049c62a9c57862d1433", b[..]))!;
    let mut gi: [65]u8 = [0; 65];
    i.public_key(gi[..]);
    @expect_eq(gi[..],
               unhex("04dad0b65394221cf9b051e1feca5787d098dfe637fc90b9ef945d0c37725811805271a0461cdb8252d61f1c456fa3e59ab1f45b33accf5f58389e0577b8990bb3",
                     w[..]));
    let r = PrivateKey.from_bytes(unhex("c6ef9c5d78ae012a011164acb397ce2088685d8f06bf9be0b283ab46476bee53", b[..]))!;
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
    let k = PrivateKey.generate()!;
    let mut pubk: [65]u8 = [0; 65];
    k.public_key(pubk[..]);
    let digest: [32]u8 = [0x42; 32];
    let mut sig: [64]u8 = [0; 64];
    k.sign(digest[..], sig[..])!;
    @expect(verify(pubk[..], digest[..], sig[..]));
    let other: [32]u8 = [0x43; 32];
    @expect(not verify(pubk[..], other[..], sig[..]));
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
    @expect(verify(q, d, s));
    s[10] ^= 1;
    @expect(not verify(q, d, s));
}
