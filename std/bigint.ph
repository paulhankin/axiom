// =============================================================================
// std/bigint.ph - arbitrary-precision integers.
//
//     let mut a = bigint.BigInt.parse("123456789012345678901234567890", 10)!;
//     defer a.deinit();
//     let mut b = bigint.BigInt.from_i64(-42)!;
//     defer b.deinit();
//     let mut c = bigint.BigInt.init();
//     defer c.deinit();
//     c.mul(&a, &b)!;                 // c = a * b
//     c.add(&c, &a)!;                 // the result may be an operand
//     let s = c.to_string(10)!;       // mem.free(s)
//
// Operations are methods on the result, which may be one of the operands,
// as in Go's math/big: z.add(&x, &y) sets z = x + y. An BigInt owns its
// memory (deinit frees it); copying the struct would share that memory, so
// use set() or clone() instead.
//
// Division comes in two kinds: quo_rem truncates toward zero (the
// remainder has the sign of the dividend, like Phalange's / and %), and
// div_mod is Euclidean (the modulus is never negative). Shifts right round
// toward minus infinity; the bitwise operations take non-negative values.
//
// Multiplication switches to Karatsuba's algorithm for large operands;
// mod_pow uses Montgomery multiplication for odd moduli. mod_pow_secret
// runs in time that depends only on the sizes of its operands, for secret
// exponents such as RSA private keys; everything else takes time that
// depends on the values.
// =============================================================================

import "io.ph" as io;
import "mem.ph" as mem;

error { DivisionByZero, InvalidNumber, Overflow, Negative, NotInvertible, NoSpaceLeft, OutOfMemory }

const KARATSUBA: usize = 40;  // limbs; below this, schoolbook multiplication

fn empty_limbs() []mut u32 {
    return @slice_from_ptr(@ptr_from_int(*mut u32, 4), 0);
}

// -----------------------------------------------------------------------------
// Natural numbers as little-endian slices of 32-bit limbs
// -----------------------------------------------------------------------------

/// The length of x without its leading (most significant) zero limbs.
fn norm(x: []u32) usize {
    let mut n = x.len;
    while (n > 0) and (x[n - 1] == 0) {
        n -= 1;
    }
    return n;
}

fn nat_cmp(x: []u32, y: []u32) i32 {
    if x.len != y.len {
        if x.len < y.len {
            return -1;
        }
        return 1;
    }
    let mut i = x.len;
    while i > 0 {
        i -= 1;
        if x[i] != y[i] {
            if x[i] < y[i] {
                return -1;
            }
            return 1;
        }
    }
    return 0;
}

/// z = x + y for x.len >= y.len, z.len >= x.len; returns the carry.
fn nat_add(z: []mut u32, x: []u32, y: []u32) u32 {
    let mut c: u64 = 0;
    for i in 0..y.len {
        let s = (@as(u64, x[i]) +% @as(u64, y[i])) +% c;
        z[i] = @truncate(u32, s);
        c = s >> 32;
    }
    for i in y.len..x.len {
        let s = @as(u64, x[i]) +% c;
        z[i] = @truncate(u32, s);
        c = s >> 32;
    }
    return @truncate(u32, c);
}

/// z = x - y for x >= y (as numbers), x.len >= y.len; returns the borrow.
fn nat_sub(z: []mut u32, x: []u32, y: []u32) u32 {
    let mut b: u64 = 0;
    for i in 0..y.len {
        let d = (@as(u64, x[i]) -% @as(u64, y[i])) -% b;
        z[i] = @truncate(u32, d);
        b = (d >> 32) & 1;
    }
    for i in y.len..x.len {
        let d = @as(u64, x[i]) -% b;
        z[i] = @truncate(u32, d);
        b = (d >> 32) & 1;
    }
    return @truncate(u32, b);
}

/// z[0..x.len] += x * m; returns the carry out of the top.
fn nat_mul_add_small(z: []mut u32, x: []u32, m: u32) u32 {
    let mut c: u64 = 0;
    let mm = @as(u64, m);
    let zz = z[..x.len];  // one bounds check, so the loop needs none
    let n = x.len;
    let mut i: usize = 0;
    while (i + 4) <= n {
        let t0 = ((@as(u64, x[i]) *% mm) +% @as(u64, zz[i])) +% c;
        zz[i] = @truncate(u32, t0);
        let t1 = ((@as(u64, x[i + 1]) *% mm) +% @as(u64, zz[i + 1])) +% (t0 >> 32);
        zz[i + 1] = @truncate(u32, t1);
        let t2 = ((@as(u64, x[i + 2]) *% mm) +% @as(u64, zz[i + 2])) +% (t1 >> 32);
        zz[i + 2] = @truncate(u32, t2);
        let t3 = ((@as(u64, x[i + 3]) *% mm) +% @as(u64, zz[i + 3])) +% (t2 >> 32);
        zz[i + 3] = @truncate(u32, t3);
        c = t3 >> 32;
        i += 4;
    }
    while i < n {
        let t = ((@as(u64, x[i]) *% mm) +% @as(u64, zz[i])) +% c;
        zz[i] = @truncate(u32, t);
        c = t >> 32;
        i += 1;
    }
    return @truncate(u32, c);
}

/// z = x * y (schoolbook); z.len == x.len + y.len, z is overwritten.
fn nat_mul_basic(z: []mut u32, x: []u32, y: []u32) void {
    @mem_set(z, 0);
    for j in 0..y.len {
        if y[j] != 0 {
            z[j + x.len] = nat_mul_add_small(z[j..j + x.len], x, y[j]);
        }
    }
}

/// z += x (z.len >= x.len); returns the carry.
fn nat_add_into(z: []mut u32, x: []u32) u32 {
    let mut c: u64 = 0;
    for i in 0..x.len {
        let s = (@as(u64, z[i]) +% @as(u64, x[i])) +% c;
        z[i] = @truncate(u32, s);
        c = s >> 32;
    }
    let mut i = x.len;
    while (c != 0) and (i < z.len) {
        let s = @as(u64, z[i]) +% c;
        z[i] = @truncate(u32, s);
        c = s >> 32;
        i += 1;
    }
    return @truncate(u32, c);
}

/// z -= x (z >= x as numbers).
fn nat_sub_from(z: []mut u32, x: []u32) void {
    let mut b: u64 = 0;
    for i in 0..x.len {
        let d = (@as(u64, z[i]) -% @as(u64, x[i])) -% b;
        z[i] = @truncate(u32, d);
        b = (d >> 32) & 1;
    }
    let mut i = x.len;
    while (b != 0) and (i < z.len) {
        let d = @as(u64, z[i]) -% b;
        z[i] = @truncate(u32, d);
        b = (d >> 32) & 1;
        i += 1;
    }
}

/// z = x * y; z.len == x.len + y.len.
fn nat_mul(z: []mut u32, x: []u32, y: []u32) !void {
    if (x.len < KARATSUBA) or (y.len < KARATSUBA) {
        nat_mul_basic(z, x, y);
        return;
    }
    // Karatsuba: x = x1*B + x0, y = y1*B + y0 with B = 2^(32h);
    // x*y = z2*B^2 + (z1 - z2 - z0)*B + z0 with z1 = (x0+x1)(y0+y1).
    let h = @min(x.len, y.len) / 2;
    let x0 = x[..h];
    let x1 = x[h..];
    let y0 = y[..h];
    let y1 = y[h..];
    @mem_set(z, 0);
    let z0 = mem.alloc.[u32](2 * h)!;
    defer mem.free(z0);
    nat_mul(z0, x0, y0)!;
    let z2 = mem.alloc.[u32](x1.len + y1.len)!;
    defer mem.free(z2);
    nat_mul(z2, x1, y1)!;
    let sx = mem.alloc.[u32](x1.len + 1)!;
    defer mem.free(sx);
    sx[x1.len] = nat_add(sx[..x1.len], x1, x0);
    let sy = mem.alloc.[u32](y1.len + 1)!;
    defer mem.free(sy);
    sy[y1.len] = nat_add(sy[..y1.len], y1, y0);
    let z1 = mem.alloc.[u32](sx.len + sy.len)!;
    defer mem.free(z1);
    nat_mul(z1, sx, sy)!;
    nat_sub_from(z1, z0);
    nat_sub_from(z1, z2);
    @mem_copy(z[..z0.len], z0);
    _ = nat_add_into(z[2 * h..], z2[..norm(z2)]);
    _ = nat_add_into(z[h..], z1[..norm(z1)]);
}

/// q = x / d, returning x % d (d != 0); q.len == x.len.
fn nat_div_small(q: []mut u32, x: []u32, d: u32) u32 {
    let dd = @as(u64, d);
    let mut r: u64 = 0;
    let mut i = x.len;
    while i > 0 {
        i -= 1;
        let cur = (r << 32) | @as(u64, x[i]);
        q[i] = @truncate(u32, cur / dd);
        r = cur % dd;
    }
    return @truncate(u32, r);
}

/// q = x / 10^9, returning x % 10^9: the constant divisor compiles to a
/// multiplication.
fn nat_div_1e9(q: []mut u32, x: []u32) u32 {
    let mut r: u64 = 0;
    let mut i = x.len;
    while i > 0 {
        i -= 1;
        let cur = (r << 32) | @as(u64, x[i]);
        let qq = cur / 1_000_000_000;
        q[i] = @truncate(u32, qq);
        r = cur - (qq * 1_000_000_000);
    }
    return @truncate(u32, r);
}

/// Knuth's algorithm D: q = u / v and r = u % v for normalized u, v with
/// v.len >= 2 and u >= v. q.len == u.len - v.len + 1, r.len == v.len.
fn nat_div(q: []mut u32, r: []mut u32, u: []u32, v: []u32) !void {
    let n = v.len;
    let m = u.len - n;
    let s = @clz(v[n - 1]);
    // shift so that the top limb of v has its top bit set
    let vn = mem.alloc.[u32](n)!;
    defer mem.free(vn);
    let un = mem.alloc.[u32](u.len + 1)!;
    defer mem.free(un);
    _ = shl_limbs(vn, v, s);
    un[u.len] = shl_limbs(un[..u.len], u, s);
    let vtop = @as(u64, vn[n - 1]);
    let vnext = @as(u64, vn[n - 2]);
    let mut j = m + 1;
    while j > 0 {
        j -= 1;
        // estimate q from the top two limbs of the remainder
        let top = (@as(u64, un[j + n]) << 32) | @as(u64, un[j + n - 1]);
        let mut qhat = top / vtop;
        let mut rhat = top % vtop;
        while (qhat > 0xFFFFFFFF) or ((qhat * vnext) > ((rhat << 32) | @as(u64, un[j + n - 2]))) {
            qhat -= 1;
            rhat += vtop;
            if rhat > 0xFFFFFFFF {
                break;
            }
        }
        // multiply and subtract
        let mut borrow: i64 = 0;
        let mut carry: u64 = 0;
        for i in 0..n {
            let p = (qhat * @as(u64, vn[i])) + carry;
            carry = p >> 32;
            let t = (@as(i64, un[i + j]) - @as(i64, @truncate(u32, p))) + borrow;
            un[i + j] = @truncate(u32, @bit_cast(u64, t));
            borrow = t >> 32;
        }
        let t = (@as(i64, un[j + n]) - @bit_cast(i64, carry)) + borrow;
        un[j + n] = @truncate(u32, @bit_cast(u64, t));
        q[j] = @truncate(u32, qhat);
        if t < 0 {
            // qhat was one too big: add v back
            q[j] -= 1;
            let mut c: u64 = 0;
            for i in 0..n {
                let s2 = (@as(u64, un[i + j]) + @as(u64, vn[i])) + c;
                un[i + j] = @truncate(u32, s2);
                c = s2 >> 32;
            }
            un[j + n] +%= @truncate(u32, c);
        }
    }
    shr_limbs(r, un[..n], s, un[n]);
}

/// out = x << s (s < 32); returns the bits shifted out of the top.
fn shl_limbs(out: []mut u32, x: []u32, s: u32) u32 {
    if s == 0 {
        @mem_copy(out[..x.len], x);
        return 0;
    }
    let mut carry: u32 = 0;
    for i in 0..x.len {
        let v = x[i];
        out[i] = (v << s) | carry;
        carry = v >> (32 - s);
    }
    return carry;
}

/// out = (x, with `above` as the next limb up) >> s (s < 32).
fn shr_limbs(out: []mut u32, x: []u32, s: u32, above: u32) void {
    if s == 0 {
        @mem_copy(out[..x.len], x);
        return;
    }
    for i in 0..x.len {
        let mut hi = above;
        if (i + 1) < x.len {
            hi = x[i + 1];
        }
        out[i] = (x[i] >> s) | (hi << (32 - s));
    }
}

// -----------------------------------------------------------------------------
// BigInt
// -----------------------------------------------------------------------------

pub struct BigInt {
    d: []mut u32,  // capacity: the value's limbs are d[..n]
    n: usize,
    neg: bool,  // never set for zero

    /// Zero, owning no memory.
    pub fn init() BigInt {
        return BigInt.{ d: empty_limbs(), n: 0, neg: false };
    }

    pub fn deinit(self: *mut BigInt) void {
        mem.free(self.d);
        self.* = BigInt.init();
    }

    pub fn from_i64(v: i64) !BigInt {
        let mut z = BigInt.init();
        z.set_i64(v)!;
        return z;
    }

    pub fn from_u64(v: u64) !BigInt {
        let mut z = BigInt.init();
        z.set_u64(v)!;
        return z;
    }

    /// A new BigInt equal to x.
    pub fn clone(x: *BigInt) !BigInt {
        let mut z = BigInt.init();
        z.set(x)!;
        return z;
    }

    fn limbs(self: *BigInt) []u32 {
        return self.d[..self.n];
    }

    fn reserve(self: *mut BigInt, n: usize) !void {
        if self.d.len < n {
            let fresh = mem.alloc.[u32](@max(n, 4))!;
            @mem_copy(fresh[..self.n], self.d[..self.n]);
            mem.free(self.d);
            self.d = fresh;
        }
    }

    /// Takes ownership of limbs (which the BigInt frees) as its magnitude.
    fn adopt(self: *mut BigInt, limbs: []mut u32, neg: bool) void {
        mem.free(self.d);
        self.d = limbs;
        self.n = norm(limbs);
        self.neg = neg and (self.n > 0);
    }

    pub fn set(self: *mut BigInt, x: *BigInt) !void {
        if @int_from_ptr(self) == @int_from_ptr(x) {
            return;
        }
        self.reserve(x.n)!;
        @mem_copy(self.d[..x.n], x.d[..x.n]);
        self.n = x.n;
        self.neg = x.neg;
    }

    pub fn set_u64(self: *mut BigInt, v: u64) !void {
        self.reserve(2)!;
        self.d[0] = @truncate(u32, v);
        self.d[1] = @truncate(u32, v >> 32);
        self.n = norm(self.d[..2]);
        self.neg = false;
    }

    pub fn set_i64(self: *mut BigInt, v: i64) !void {
        let mut m = @bit_cast(u64, v);
        if v < 0 {
            m = 0 -% m;
        }
        self.set_u64(m)!;
        self.neg = v < 0;
    }

    // --- queries -------------------------------------------------------------

    pub fn is_zero(self: *BigInt) bool {
        return self.n == 0;
    }

    /// -1, 0 or 1.
    pub fn sign(self: *BigInt) i32 {
        if self.n == 0 {
            return 0;
        }
        if self.neg {
            return -1;
        }
        return 1;
    }

    pub fn is_odd(self: *BigInt) bool {
        return (self.n > 0) and ((self.d[0] & 1) == 1);
    }

    /// Negative, zero or positive as self < x, self == x, self > x.
    pub fn cmp(self: *BigInt, x: *BigInt) i32 {
        if self.neg != x.neg {
            if self.neg {
                return -1;
            }
            return 1;
        }
        let c = nat_cmp(self.limbs(), x.limbs());
        if self.neg {
            return -c;
        }
        return c;
    }

    /// Compares absolute values.
    pub fn cmp_abs(self: *BigInt, x: *BigInt) i32 {
        return nat_cmp(self.limbs(), x.limbs());
    }

    pub fn cmp_i64(self: *BigInt, v: i64) i32 {
        let mut t: [2]u32 = [0; 2];
        let mut m = @bit_cast(u64, v);
        if v < 0 {
            m = 0 -% m;
        }
        t[0] = @truncate(u32, m);
        t[1] = @truncate(u32, m >> 32);
        let tv = BigInt.{ d: t[..], n: norm(t[..]), neg: v < 0 };
        return self.cmp(&tv);
    }

    /// The number of bits in the absolute value (0 for zero).
    pub fn bit_len(self: *BigInt) usize {
        if self.n == 0 {
            return 0;
        }
        return (32 * self.n) - @as(usize, @clz(self.d[self.n - 1]));
    }

    /// Bit i of the absolute value.
    pub fn bit(self: *BigInt, i: usize) u32 {
        let k = i / 32;
        if k >= self.n {
            return 0;
        }
        return (self.d[k] >> @truncate(u32, i % 32)) & 1;
    }

    /// The number of trailing zero bits of the absolute value (0 for zero).
    pub fn trailing_zeros(self: *BigInt) usize {
        for i in 0..self.n {
            if self.d[i] != 0 {
                return (32 * i) + @as(usize, @ctz(self.d[i]));
            }
        }
        return 0;
    }

    pub fn to_u64(self: *BigInt) !u64 {
        if self.neg or (self.n > 2) {
            return error.Overflow;
        }
        let mut v: u64 = 0;
        if self.n > 0 {
            v = @as(u64, self.d[0]);
        }
        if self.n > 1 {
            v = v | (@as(u64, self.d[1]) << 32);
        }
        return v;
    }

    pub fn to_i64(self: *BigInt) !i64 {
        if self.n > 2 {
            return error.Overflow;
        }
        let mut m: u64 = 0;
        if self.n > 0 {
            m = @as(u64, self.d[0]);
        }
        if self.n > 1 {
            m = m | (@as(u64, self.d[1]) << 32);
        }
        if self.neg {
            if m > 9223372036854775808 {
                return error.Overflow;
            }
            return @bit_cast(i64, 0 -% m);
        }
        if m > 9223372036854775807 {
            return error.Overflow;
        }
        return @bit_cast(i64, m);
    }

    /// The nearest f64 (truncating the bits below the top 64; inf if huge).
    pub fn to_f64(self: *BigInt) f64 {
        let bl = self.bit_len();
        let mut v: f64 = 0.0;
        if bl <= 64 {
            let mut m: u64 = 0;
            for i in 0..self.n {
                m = m | (@as(u64, self.d[i]) << @truncate(u32, 32 * i));
            }
            v = @float_from_int(f64, m);
        } else {
            // the top 64 bits, scaled
            let shift = bl - 64;
            let mut m: u64 = 0;
            for k in @as(usize, 0)..64 {
                m = (m << 1) | @as(u64, self.bit((bl - 1) - k));
            }
            v = @float_from_int(f64, m);
            let mut s = shift;
            while s > 0 {
                let step = @min(s, 1000);
                let mut f: f64 = 1.0;
                for i in @as(usize, 0)..step {
                    f = f * 2.0;
                }
                v = v * f;
                s -= step;
            }
        }
        if self.neg {
            return -v;
        }
        return v;
    }

    // --- sign ----------------------------------------------------------------

    /// self = -x.
    pub fn neg_of(self: *mut BigInt, x: *BigInt) !void {
        self.set(x)!;
        self.neg = (not self.neg) and (self.n > 0);
    }

    /// self = |x|.
    pub fn abs(self: *mut BigInt, x: *BigInt) !void {
        self.set(x)!;
        self.neg = false;
    }

    // --- addition ------------------------------------------------------------

    /// self = |x| + |y| or |x| - |y| with the given sign handling.
    fn add_signed(self: *mut BigInt, x: *BigInt, xneg: bool, y: *BigInt, yneg: bool) !void {
        let xs = x.limbs();
        let ys = y.limbs();
        if xneg == yneg {
            let mut a = xs;
            let mut b = ys;
            if a.len < b.len {
                a = ys;
                b = xs;
            }
            let out = mem.alloc.[u32](a.len + 1)!;
            out[a.len] = nat_add(out[..a.len], a, b);
            self.adopt(out, xneg);
            return;
        }
        let c = nat_cmp(xs, ys);
        if c == 0 {
            self.n = 0;
            self.neg = false;
            return;
        }
        if c > 0 {
            let out = mem.alloc.[u32](xs.len)!;
            _ = nat_sub(out, xs, ys);
            self.adopt(out, xneg);
        } else {
            let out = mem.alloc.[u32](ys.len)!;
            _ = nat_sub(out, ys, xs);
            self.adopt(out, yneg);
        }
    }

    /// self = x + y.
    pub fn add(self: *mut BigInt, x: *BigInt, y: *BigInt) !void {
        self.add_signed(x, x.neg, y, y.neg)!;
    }

    /// self = x - y.
    pub fn sub(self: *mut BigInt, x: *BigInt, y: *BigInt) !void {
        self.add_signed(x, x.neg, y, (not y.neg) and (y.n > 0))!;
    }

    pub fn add_i64(self: *mut BigInt, x: *BigInt, v: i64) !void {
        let mut t = BigInt.from_i64(v)!;
        defer t.deinit();
        self.add(x, &t)!;
    }

    // --- multiplication ------------------------------------------------------

    /// self = x * y.
    pub fn mul(self: *mut BigInt, x: *BigInt, y: *BigInt) !void {
        if (x.n == 0) or (y.n == 0) {
            self.n = 0;
            self.neg = false;
            return;
        }
        let out = mem.alloc.[u32](x.n + y.n)!;
        nat_mul(out, x.limbs(), y.limbs()) catch |e| {
            mem.free(out);
            return e;
        };
        self.adopt(out, x.neg != y.neg);
    }

    pub fn mul_i64(self: *mut BigInt, x: *BigInt, v: i64) !void {
        let mut t = BigInt.from_i64(v)!;
        defer t.deinit();
        self.mul(x, &t)!;
    }

    /// self = x^e.
    pub fn pow(self: *mut BigInt, x: *BigInt, e: u64) !void {
        let mut result = BigInt.from_u64(1)!;
        defer result.deinit();
        let mut base = BigInt.clone(x)!;
        defer base.deinit();
        let mut k = e;
        while k > 0 {
            if (k & 1) == 1 {
                result.mul(&result, &base)!;
            }
            k = k >> 1;
            if k > 0 {
                base.mul(&base, &base)!;
            }
        }
        self.set(&result)!;
    }

    // --- division ------------------------------------------------------------

    /// q = x / y truncated toward zero and r = x - q*y (the sign of x).
    /// q and r must be different Ints; either may be x or y.
    pub fn quo_rem(q: *mut BigInt, r: *mut BigInt, x: *BigInt, y: *BigInt) !void {
        if y.n == 0 {
            return error.DivisionByZero;
        }
        let xneg = x.neg;
        let yneg = y.neg;
        if nat_cmp(x.limbs(), y.limbs()) < 0 {
            r.set(x)!;
            q.n = 0;
            q.neg = false;
            return;
        }
        let qd = mem.alloc.[u32](x.n)!;
        let rd = mem.alloc.[u32](y.n) catch |e| {
            mem.free(qd);
            return e;
        };
        if y.n == 1 {
            @mem_set(rd, 0);
            rd[0] = nat_div_small(qd, x.limbs(), y.d[0]);
        } else {
            @mem_set(qd, 0);
            nat_div(qd[..(x.n - y.n) + 1], rd, x.limbs(), y.limbs()) catch |e| {
                mem.free(qd);
                mem.free(rd);
                return e;
            };
        }
        q.adopt(qd, xneg != yneg);
        r.adopt(rd, xneg);
    }

    /// self = x / y truncated toward zero.
    pub fn quo(self: *mut BigInt, x: *BigInt, y: *BigInt) !void {
        let mut r = BigInt.init();
        defer r.deinit();
        BigInt.quo_rem(self, &mut r, x, y)!;
    }

    /// self = x % y with the sign of x.
    pub fn rem(self: *mut BigInt, x: *BigInt, y: *BigInt) !void {
        let mut q = BigInt.init();
        defer q.deinit();
        BigInt.quo_rem(&mut q, self, x, y)!;
    }

    /// Euclidean division: q = x div y and m = x mod y, with 0 <= m < |y|.
    pub fn div_mod(q: *mut BigInt, m: *mut BigInt, x: *BigInt, y: *BigInt) !void {
        let yneg = y.neg;
        let mut yc = BigInt.clone(y)!;
        defer yc.deinit();
        BigInt.quo_rem(q, m, x, y)!;
        if m.neg {
            // m += |y|; q -= sign(y)
            yc.neg = false;
            m.add(m, &yc)!;
            if yneg {
                q.add_i64(q, 1)!;
            } else {
                q.add_i64(q, -1)!;
            }
        }
    }

    /// self = x mod m (Euclidean, so 0 <= self < |m|).
    pub fn mod(self: *mut BigInt, x: *BigInt, m: *BigInt) !void {
        let mut q = BigInt.init();
        defer q.deinit();
        BigInt.div_mod(&mut q, self, x, m)!;
    }

    // --- shifts and bits -----------------------------------------------------

    /// self = x * 2^s.
    pub fn shl(self: *mut BigInt, x: *BigInt, s: usize) !void {
        if x.n == 0 {
            self.n = 0;
            self.neg = false;
            return;
        }
        let limbs = s / 32;
        let bits = @truncate(u32, s % 32);
        let out = mem.alloc.[u32]((x.n + limbs) + 1)!;
        @mem_set(out[..limbs], 0);
        out[x.n + limbs] = shl_limbs(out[limbs..limbs + x.n], x.limbs(), bits);
        self.adopt(out, x.neg);
    }

    /// self = floor(x / 2^s).
    pub fn shr(self: *mut BigInt, x: *BigInt, s: usize) !void {
        let limbs = s / 32;
        let bits = @truncate(u32, s % 32);
        if limbs >= x.n {
            if x.neg {
                self.set_i64(-1)!;
            } else {
                self.n = 0;
                self.neg = false;
            }
            return;
        }
        // a negative number rounds down: -(|x| >> s) - 1 if bits were lost
        let mut lost = false;
        if x.neg {
            for i in 0..limbs {
                if x.d[i] != 0 {
                    lost = true;
                }
            }
            if (bits > 0) and ((x.d[limbs] & ((@as(u32, 1) << bits) - 1)) != 0) {
                lost = true;
            }
        }
        let n = x.n - limbs;
        let out = mem.alloc.[u32](n)!;
        shr_limbs(out, x.d[limbs..x.n], bits, 0);
        let neg = x.neg;
        self.adopt(out, neg);
        if lost {
            self.add_i64(self, -1)!;
        }
    }

    fn bitwise(self: *mut BigInt, x: *BigInt, y: *BigInt, op: u8) !void {
        if x.neg or y.neg {
            return error.Negative;
        }
        let n = @max(x.n, y.n);
        let out = mem.alloc.[u32](@max(n, 1))!;
        for i in 0..n {
            let mut a: u32 = 0;
            let mut b: u32 = 0;
            if i < x.n {
                a = x.d[i];
            }
            if i < y.n {
                b = y.d[i];
            }
            if op == '&' {
                out[i] = a & b;
            } else if op == '|' {
                out[i] = a | b;
            } else {
                out[i] = a ^ b;
            }
        }
        if n == 0 {
            out[0] = 0;
        }
        self.adopt(out, false);
    }

    /// self = x & y (x, y >= 0).
    pub fn bit_and(self: *mut BigInt, x: *BigInt, y: *BigInt) !void {
        self.bitwise(x, y, '&')!;
    }

    /// self = x | y (x, y >= 0).
    pub fn bit_or(self: *mut BigInt, x: *BigInt, y: *BigInt) !void {
        self.bitwise(x, y, '|')!;
    }

    /// self = x ^ y (x, y >= 0).
    pub fn bit_xor(self: *mut BigInt, x: *BigInt, y: *BigInt) !void {
        self.bitwise(x, y, '^')!;
    }

    /// Sets bit i of the absolute value to b.
    pub fn set_bit(self: *mut BigInt, i: usize, b: u32) !void {
        let k = i / 32;
        if k >= self.n {
            if b == 0 {
                return;
            }
            self.reserve(k + 1)!;
            @mem_set(self.d[self.n..k + 1], 0);
            self.n = k + 1;
        }
        let mask = @as(u32, 1) << @truncate(u32, i % 32);
        if b == 0 {
            self.d[k] = self.d[k] & ~mask;
        } else {
            self.d[k] = self.d[k] | mask;
        }
        self.n = norm(self.d[..self.n]);
        if self.n == 0 {
            self.neg = false;
        }
    }

    // --- number theory -------------------------------------------------------

    /// self = gcd(|x|, |y|).
    pub fn gcd(self: *mut BigInt, x: *BigInt, y: *BigInt) !void {
        let mut a = BigInt.init();
        defer a.deinit();
        let mut b = BigInt.init();
        defer b.deinit();
        a.abs(x)!;
        b.abs(y)!;
        let mut t = BigInt.init();
        defer t.deinit();
        while not b.is_zero() {
            t.rem(&a, &b)!;
            a.set(&b)!;
            b.set(&t)!;
        }
        self.set(&a)!;
    }

    /// self = x^-1 mod m (m > 1); NotInvertible if gcd(x, m) != 1.
    pub fn mod_inverse(self: *mut BigInt, x: *BigInt, m: *BigInt) !void {
        if m.sign() <= 0 {
            return error.InvalidNumber;
        }
        // extended Euclid on (x mod m, m)
        let mut r0 = BigInt.init();
        defer r0.deinit();
        r0.mod(x, m)!;
        let mut r1 = BigInt.clone(m)!;
        defer r1.deinit();
        let mut s0 = BigInt.from_i64(1)!;
        defer s0.deinit();
        let mut s1 = BigInt.init();
        defer s1.deinit();
        let mut q = BigInt.init();
        defer q.deinit();
        let mut t = BigInt.init();
        defer t.deinit();
        let mut u = BigInt.init();
        defer u.deinit();
        // invariant: s0 * x = r0 (mod m), s1 * x = r1 (mod m)
        while not r1.is_zero() {
            BigInt.quo_rem(&mut q, &mut t, &r0, &r1)!;
            r0.set(&r1)!;
            r1.set(&t)!;
            u.mul(&q, &s1)!;
            t.sub(&s0, &u)!;
            s0.set(&s1)!;
            s1.set(&t)!;
        }
        if r0.cmp_i64(1) != 0 {
            return error.NotInvertible;
        }
        self.mod(&s0, m)!;
    }

    /// self = floor(sqrt(x)) (x >= 0).
    pub fn sqrt(self: *mut BigInt, x: *BigInt) !void {
        if x.neg {
            return error.Negative;
        }
        if x.n == 0 {
            self.n = 0;
            self.neg = false;
            return;
        }
        // Newton's method from a power of two above the root
        let mut r = BigInt.from_u64(1)!;
        defer r.deinit();
        r.shl(&r, (x.bit_len() / 2) + 1)!;
        let mut t = BigInt.init();
        defer t.deinit();
        loop {
            // t = (r + x / r) / 2
            t.quo(x, &r)!;
            t.add(&t, &r)!;
            t.shr(&t, 1)!;
            if t.cmp(&r) >= 0 {
                break;
            }
            r.set(&t)!;
        }
        self.set(&r)!;
    }

    /// self = x^e mod m (e >= 0, m > 0), with Montgomery multiplication when
    /// m is odd. Time depends on the values; see mod_pow_secret.
    pub fn mod_pow(self: *mut BigInt, x: *BigInt, e: *BigInt, m: *BigInt) !void {
        mod_pow_impl(self, x, e, m, false)!;
    }

    /// Like mod_pow for an odd m, in time that depends only on the sizes of
    /// the operands (for secret exponents).
    pub fn mod_pow_secret(self: *mut BigInt, x: *BigInt, e: *BigInt, m: *BigInt) !void {
        if not m.is_odd() {
            return error.InvalidNumber;
        }
        mod_pow_impl(self, x, e, m, true)!;
    }

    // --- conversion ----------------------------------------------------------

    /// Parses digits in `base` (2 to 36) with an optional sign; base 0 reads
    /// a 0x, 0o or 0b prefix (else decimal). Underscores between digits are
    /// allowed.
    pub fn parse(s: []u8, base: u32) !BigInt {
        let mut i: usize = 0;
        let mut neg = false;
        if (s.len > 0) and ((s[0] == '-') or (s[0] == '+')) {
            neg = s[0] == '-';
            i = 1;
        }
        let mut b = base;
        if b == 0 {
            b = 10;
            if ((i + 1) < s.len) and (s[i] == '0') {
                let p = s[i + 1] | 0x20;
                if p == 'x' {
                    b = 16;
                } else if p == 'o' {
                    b = 8;
                } else if p == 'b' {
                    b = 2;
                }
                if b != 10 {
                    i += 2;
                }
            }
        }
        if (b < 2) or (b > 36) or (i >= s.len) {
            return error.InvalidNumber;
        }
        // take digits in chunks that fit a limb multiplier
        let mut chunk: u32 = 1;
        let mut per: u32 = 0;
        while (@as(u64, chunk) * @as(u64, b)) <= 0xFFFFFFFF {
            chunk *= b;
            per += 1;
        }
        let mut z = BigInt.init();
        z.reserve(((s.len * 6) / 32) + 2) catch |e| {
            return e;
        };
        let mut acc: u32 = 0;
        let mut mult: u32 = 1;
        let mut count: u32 = 0;
        let mut digits: usize = 0;
        let mut last_us = true;
        while i < s.len {
            let c = s[i];
            i += 1;
            if c == '_' {
                if last_us {
                    z.deinit();
                    return error.InvalidNumber;
                }
                last_us = true;
                continue;
            }
            let d = digit_value(c);
            if d >= b {
                z.deinit();
                return error.InvalidNumber;
            }
            last_us = false;
            acc = (acc * b) + d;
            mult *= b;
            count += 1;
            digits += 1;
            if count == per {
                z.mul_add_small(mult, acc) catch |e| {
                    z.deinit();
                    return e;
                };
                acc = 0;
                mult = 1;
                count = 0;
            }
        }
        if (digits == 0) or last_us {
            z.deinit();
            return error.InvalidNumber;
        }
        if count > 0 {
            z.mul_add_small(mult, acc) catch |e| {
                z.deinit();
                return e;
            };
        }
        z.neg = neg and (z.n > 0);
        return z;
    }

    /// self = self * m + a (self >= 0).
    fn mul_add_small(self: *mut BigInt, m: u32, a: u32) !void {
        self.reserve(self.n + 1)!;
        let mut c = @as(u64, a);
        for i in 0..self.n {
            let t = (@as(u64, self.d[i]) * @as(u64, m)) + c;
            self.d[i] = @truncate(u32, t);
            c = t >> 32;
        }
        if c != 0 {
            self.d[self.n] = @truncate(u32, c);
            self.n += 1;
        }
    }

    /// Writes the digits in `base` (2 to 36, lower case) with a '-' if negative.
    pub fn format(self: *BigInt, w: io.Writer, base: u32) !void {
        if (base < 2) or (base > 36) {
            return error.InvalidNumber;
        }
        if self.n == 0 {
            w.write_all("0")!;
            return;
        }
        let digits = "0123456789abcdefghijklmnopqrstuvwxyz";
        if (base & (base - 1)) == 0 {
            // a power of two: read the digits straight from the bits
            let bits = @as(usize, @ctz(base));
            let nd = (self.bit_len() + bits - 1) / bits;
            let out = mem.alloc.[u8](nd)!;
            defer mem.free(out);
            for k in 0..nd {
                let mut v: u32 = 0;
                for j in 0..bits {
                    v = v | (self.bit((k * bits) + j) << @truncate(u32, j));
                }
                out[(nd - 1) - k] = digits[@as(usize, v)];
            }
            if self.neg {
                w.write_all("-")!;
            }
            w.write_all(out)!;
            return;
        }
        // divide repeatedly by the largest power of base that fits a limb
        let mut chunk: u32 = 1;
        let mut per: usize = 0;
        while (@as(u64, chunk) * @as(u64, base)) <= 0xFFFFFFFF {
            chunk *= base;
            per += 1;
        }
        if base == 10 {
            chunk = 1_000_000_000;
            per = 9;
        }
        let max_digits = ((self.n * 32) + per) + 2;
        let out = mem.alloc.[u8](max_digits)!;
        defer mem.free(out);
        let work = mem.alloc.[u32](self.n)!;
        defer mem.free(work);
        @mem_copy(work, self.limbs());
        let mut wn = self.n;
        let mut at = out.len;
        while wn > 0 {
            let mut r: u32 = 0;
            if base == 10 {
                r = nat_div_1e9(work[..wn], work[..wn]);
            } else {
                r = nat_div_small(work[..wn], work[..wn], chunk);
            }
            wn = norm(work[..wn]);
            let mut rr = r;
            for k in 0..per {
                if (wn == 0) and (rr == 0) {
                    break;
                }
                at -= 1;
                out[at] = digits[@as(usize, rr % base)];
                rr = rr / base;
            }
        }
        if self.neg {
            w.write_all("-")!;
        }
        w.write_all(out[at..])!;
    }

    /// The digits in `base` in new memory (free it with mem.free).
    pub fn to_string(self: *BigInt, base: u32) ![]mut u8 {
        let mut b = io.BufferWriter.init(mem.alloc.[u8](((self.n * 32) + 8))!);
        let w = b.writer();
        self.format(w, base) catch |e| {
            mem.free(b.buf);
            return e;
        };
        return mem.realloc(b.buf, b.len) catch |e| {
            mem.free(b.buf);
            return e;
        };
    }

    /// The absolute value from big-endian bytes.
    pub fn from_bytes(be: []u8) !BigInt {
        let mut z = BigInt.init();
        let n = (be.len + 3) / 4;
        z.reserve(@max(n, 1))!;
        @mem_set(z.d[..n], 0);
        for i in 0..be.len {
            let b = be[(be.len - 1) - i];
            z.d[i / 4] = z.d[i / 4] | (@as(u32, b) << @truncate(u32, 8 * (i % 4)));
        }
        z.n = norm(z.d[..n]);
        return z;
    }

    /// The number of bytes of the absolute value.
    pub fn byte_len(self: *BigInt) usize {
        return (self.bit_len() + 7) / 8;
    }

    /// Writes the absolute value as big-endian bytes filling out (with
    /// leading zeros); NoSpaceLeft if it does not fit.
    pub fn to_bytes(self: *BigInt, out: []mut u8) !void {
        if self.byte_len() > out.len {
            return error.NoSpaceLeft;
        }
        for i in 0..out.len {
            let k = i / 4;
            let mut b: u8 = 0;
            if k < self.n {
                b = @truncate(u8, self.d[k] >> @truncate(u32, 8 * (i % 4)));
            }
            out[(out.len - 1) - i] = b;
        }
    }
}

fn digit_value(c: u8) u32 {
    if (c >= '0') and (c <= '9') {
        return @as(u32, c - '0');
    }
    let l = c | 0x20;
    if (l >= 'a') and (l <= 'z') {
        return @as(u32, l - 'a') + 10;
    }
    return 99;
}

// -----------------------------------------------------------------------------
// Modular exponentiation
// -----------------------------------------------------------------------------

/// -m^-1 mod 2^32 for odd m0 (Newton's iteration).
fn mont_inv(m0: u32) u32 {
    let mut x: u32 = 1;
    for i in @as(u32, 0)..5 {
        x = x *% (2 -% (m0 *% x));
    }
    return 0 -% x;
}

/// z = a * b * R^-1 mod m (CIOS). a, b < m; z, t scratch of m.len and
/// m.len + 2 limbs. The final subtraction is done with masks, so the time
/// does not depend on the values.
fn mont_mul(z: []mut u32, a0: []u32, b: []u32, m0: []u32, minv: u32, t0: []mut u32) void {
    let n = m0.len;
    // (the same lengths throughout, so the loops need no bounds checks;
    // no sum below can pass 2^64 - 1, so they wrap without checks)
    let a = a0[..n];
    let m = m0[..n];
    let t = t0[..n + 2];
    @mem_set(t, 0);
    for i in 0..n {
        // t += a * b[i]
        let mut c: u64 = 0;
        let bi = @as(u64, b[i]);
        for j in 0..n {
            let s = ((@as(u64, a[j]) *% bi) +% @as(u64, t[j])) +% c;
            t[j] = @truncate(u32, s);
            c = s >> 32;
        }
        let s1 = @as(u64, t[n]) +% c;
        t[n] = @truncate(u32, s1);
        t[n + 1] = @truncate(u32, s1 >> 32);
        // t = (t + u * m) / 2^32 with u making the low limb zero
        let u = @as(u64, t[0] *% minv);
        let s2 = (u *% @as(u64, m[0])) +% @as(u64, t[0]);
        c = s2 >> 32;
        for j in 1..n {
            let s = ((u *% @as(u64, m[j])) +% @as(u64, t[j])) +% c;
            t[j - 1] = @truncate(u32, s);
            c = s >> 32;
        }
        let s3 = @as(u64, t[n]) +% c;
        t[n - 1] = @truncate(u32, s3);
        t[n] = t[n + 1] +% @truncate(u32, s3 >> 32);
    }
    // z = t - m if t >= m, else t, without branching on the values
    let mut borrow: u64 = 0;
    for j in 0..n {
        let d = (@as(u64, t[j]) -% @as(u64, m[j])) -% borrow;
        z[j] = @truncate(u32, d);
        borrow = (d >> 32) & 1;
    }
    // keep t when it was smaller: t[n] == 0 and a borrow happened
    let keep_t = (@as(u64, borrow) & (@as(u64, (@as(u64, t[n]) -% 1) >> 63) & 1)) & 1;
    let mask = 0 -% @truncate(u32, keep_t);
    for j in 0..n {
        z[j] = (t[j] & mask) | (z[j] & ~mask);
    }
}

fn mod_pow_impl(z: *mut BigInt, x: *BigInt, e: *BigInt, m: *BigInt, secret: bool) !void {
    if (m.sign() <= 0) or e.neg {
        return error.InvalidNumber;
    }
    if m.cmp_i64(1) == 0 {
        z.n = 0;
        z.neg = false;
        return;
    }
    // the base reduced into [0, m)
    let mut b = BigInt.init();
    defer b.deinit();
    b.mod(x, m)!;
    if not m.is_odd() {
        // plain square-and-multiply with a division each step
        let mut r = BigInt.from_u64(1)!;
        defer r.deinit();
        let mut i = e.bit_len();
        while i > 0 {
            i -= 1;
            r.mul(&r, &r)!;
            r.mod(&r, m)!;
            if e.bit(i) == 1 {
                r.mul(&r, &b)!;
                r.mod(&r, m)!;
            }
        }
        z.set(&r)!;
        return;
    }
    let n = m.n;
    let mm = m.limbs();
    let minv = mont_inv(mm[0]);
    // R^2 mod m, to move numbers into Montgomery form
    let mut r2 = BigInt.from_u64(1)!;
    defer r2.deinit();
    r2.shl(&r2, 64 * n)!;
    r2.mod(&r2, m)!;
    let scratch = mem.alloc.[u32]((20 * n) + 2)!;
    defer mem.free(scratch);
    @mem_set(scratch, 0);
    let t = scratch[0..n + 2];
    let acc = scratch[n + 2..(2 * n) + 2];
    let tmp = scratch[(2 * n) + 2..(3 * n) + 2];
    let bm = scratch[(3 * n) + 2..(4 * n) + 2];
    // table[k] = b^k in Montgomery form for k < 16
    let table = scratch[(4 * n) + 2..(20 * n) + 2];
    let mut padded_b = mem.alloc.[u32](n)!;
    defer mem.free(padded_b);
    @mem_set(padded_b, 0);
    @mem_copy(padded_b[..b.n], b.limbs());
    let mut padded_r2 = mem.alloc.[u32](n)!;
    defer mem.free(padded_r2);
    @mem_set(padded_r2, 0);
    @mem_copy(padded_r2[..r2.n], r2.limbs());
    mont_mul(bm, padded_b, padded_r2, mm, minv, t);
    // one in Montgomery form: R mod m = mont_mul(1, R^2)
    let mut one = mem.alloc.[u32](n)!;
    defer mem.free(one);
    @mem_set(one, 0);
    one[0] = 1;
    mont_mul(table[0..n], one, padded_r2, mm, minv, t);
    @mem_copy(table[n..2 * n], bm);
    for k in @as(usize, 2)..16 {
        mont_mul(tmp, table[(k - 1) * n..k * n], bm, mm, minv, t);
        @mem_copy(table[k * n..(k + 1) * n], tmp);
    }
    @mem_copy(acc, table[0..n]);
    // 4-bit fixed windows from the top; with `secret`, every window does
    // four squarings and one multiplication by an entry read with masks
    let mut bits = e.bit_len();
    if secret {
        bits = 32 * e.n;
    }
    let windows = (bits + 3) / 4;
    let mut wi = windows;
    while wi > 0 {
        wi -= 1;
        for s in @as(usize, 0)..4 {
            mont_mul(tmp, acc, acc, mm, minv, t);
            @mem_copy(acc, tmp);
        }
        let mut k: u32 = 0;
        for s in @as(usize, 0)..4 {
            k = k | (e.bit((4 * wi) + s) << @truncate(u32, s));
        }
        if secret {
            // read table[k] touching every entry
            @mem_set(bm, 0);
            for j in @as(u32, 0)..16 {
                let mask = 0 -% @truncate(u32, (@as(u64, j ^ k) -% 1) >> 63);
                let row = table[@as(usize, j) * n..(@as(usize, j) + 1) * n];
                for q in 0..n {
                    bm[q] = bm[q] | (row[q] & mask);
                }
            }
            mont_mul(tmp, acc, bm, mm, minv, t);
            @mem_copy(acc, tmp);
        } else if k != 0 {
            mont_mul(tmp, acc, table[@as(usize, k) * n..(@as(usize, k) + 1) * n], mm, minv, t);
            @mem_copy(acc, tmp);
        }
    }
    // out of Montgomery form: mont_mul(acc, 1)
    mont_mul(tmp, acc, one, mm, minv, t);
    let out = mem.alloc.[u32](n)!;
    @mem_copy(out, tmp);
    z.adopt(out, false);
}

// -----------------------------------------------------------------------------
// Tests (phalange test std/bigint.ph)
// -----------------------------------------------------------------------------

fn str_of(x: *BigInt, base: u32) []mut u8 {
    return x.to_string(base) catch @trap();
}

fn eq_str(x: *BigInt, base: u32, want: []u8) void {
    let s = str_of(x, base);
    defer mem.free(s);
    @expect_eq(s, want);
}

test "parse and format" {
    let mut a = BigInt.parse("-123456789012345678901234567890", 10)!;
    defer a.deinit();
    eq_str(&a, 10, "-123456789012345678901234567890");
    eq_str(&a, 16, "-18ee90ff6c373e0ee4e3f0ad2");
    let mut b = BigInt.parse("0xDEAD_beef_0000_0000_1", 0)!;
    defer b.deinit();
    eq_str(&b, 16, "deadbeef000000001");
    let mut z = BigInt.parse("-0", 10)!;
    defer z.deinit();
    eq_str(&z, 10, "0");
    @expect_eq(z.sign(), 0);
    let bad: [6][]u8 = ["", "-", "12a", "_1", "1__2", "0x"];
    for s in bad {
        var_check_bad(s);
    }
    let mut c = BigInt.from_i64(-9223372036854775807 - 1)!;
    defer c.deinit();
    @expect_eq(c.to_i64()!, -9223372036854775807 - 1);
    eq_str(&c, 2, "-1000000000000000000000000000000000000000000000000000000000000000");
    let bytes: [5]u8 = [1, 2, 3, 4, 5];
    let mut d = BigInt.from_bytes(bytes[..])!;
    defer d.deinit();
    eq_str(&d, 16, "102030405");
    let mut out: [8]u8 = [0; 8];
    d.to_bytes(out[..])!;
    @expect_eq(out[3..], bytes[..]);
}

fn var_check_bad(s: []u8) void {
    let mut x = BigInt.parse(s, 0) catch return;
    x.deinit();
    @trap();
}

test "arithmetic" {
    let mut a = BigInt.parse("340282366920938463463374607431768211457", 10)!;  // 2^128 + 1
    defer a.deinit();
    let mut b = BigInt.parse("-18446744073709551617", 10)!;  // -(2^64 + 1)
    defer b.deinit();
    let mut c = BigInt.init();
    defer c.deinit();
    c.add(&a, &b)!;
    eq_str(&c, 10, "340282366920938463444927863358058659840");
    c.sub(&b, &a)!;
    eq_str(&c, 10, "-340282366920938463481821351505477763074");
    c.mul(&a, &b)!;
    eq_str(&c, 10, "-6277101735386680764176071790128604879584176795969512275969");
    // aliasing: c = c * c
    c.mul(&c, &c)!;
    eq_str(&c, 16, "1000000000000000200000000000000030000000000000004000000000000000300000000000000020000000000000001");
    let mut q = BigInt.init();
    defer q.deinit();
    let mut r = BigInt.init();
    defer r.deinit();
    let mut seven = BigInt.from_i64(7)!;
    defer seven.deinit();
    let mut m13 = BigInt.from_i64(-13)!;
    defer m13.deinit();
    BigInt.quo_rem(&mut q, &mut r, &m13, &seven)!;
    @expect_eq(q.to_i64()!, -1);
    @expect_eq(r.to_i64()!, -6);
    BigInt.div_mod(&mut q, &mut r, &m13, &seven)!;
    @expect_eq(q.to_i64()!, -2);
    @expect_eq(r.to_i64()!, 1);
    BigInt.quo_rem(&mut q, &mut r, &a, &b)!;
    eq_str(&q, 10, "-18446744073709551615");
    eq_str(&r, 10, "2");
    let mut zero = BigInt.init();
    BigInt.quo_rem(&mut q, &mut r, &a, &zero) catch |e| {
        @expect_eq(e, error.DivisionByZero);
        return;
    };
    @trap();
}

test "powers, gcd, inverse, sqrt" {
    let mut two = BigInt.from_i64(2)!;
    defer two.deinit();
    let mut p = BigInt.init();
    defer p.deinit();
    p.pow(&two, 127)!;
    p.add_i64(&p, -1)!;  // a Mersenne prime
    eq_str(&p, 10, "170141183460469231731687303715884105727");
    let mut r = BigInt.init();
    defer r.deinit();
    let mut e = BigInt.init();
    defer e.deinit();
    e.add_i64(&p, -1)!;
    let mut three = BigInt.from_i64(3)!;
    defer three.deinit();
    r.mod_pow(&three, &e, &p)!;  // Fermat
    @expect_eq(r.to_i64()!, 1);
    r.mod_pow_secret(&three, &e, &p)!;
    @expect_eq(r.to_i64()!, 1);
    let mut even = BigInt.from_i64(1000)!;
    defer even.deinit();
    let mut e2 = BigInt.from_i64(123)!;
    defer e2.deinit();
    r.mod_pow(&three, &e2, &even)!;
    @expect_eq(r.to_i64()!, 827);
    let mut a = BigInt.from_i64(240)!;
    defer a.deinit();
    let mut b = BigInt.from_i64(-46)!;
    defer b.deinit();
    r.gcd(&a, &b)!;
    @expect_eq(r.to_i64()!, 2);
    r.mod_inverse(&three, &p)!;
    let mut chk = BigInt.init();
    defer chk.deinit();
    chk.mul(&r, &three)!;
    chk.mod(&chk, &p)!;
    @expect_eq(chk.to_i64()!, 1);
    r.sqrt(&p)!;
    eq_str(&r, 10, "13043817825332782212");
    let mut s = BigInt.init();
    defer s.deinit();
    s.shl(&p, 100)!;
    s.shr(&s, 99)!;
    chk.mul(&p, &two)!;
    @expect_eq(s.cmp(&chk), 0);
    let mut neg = BigInt.from_i64(-5)!;
    defer neg.deinit();
    s.shr(&neg, 1)!;
    @expect_eq(s.to_i64()!, -3);
}
