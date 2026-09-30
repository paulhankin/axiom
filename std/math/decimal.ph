// =============================================================================
// std/math/decimal.ph - arbitrary-precision decimal numbers.
//
//     let mut a = decimal.Decimal.parse("0.1")!;
//     defer a.deinit();
//     let mut b = decimal.Decimal.parse("0.2")!;
//     defer b.deinit();
//     let mut c = decimal.Decimal.init();
//     defer c.deinit();
//     c.add(&a, &b)!;                          // exactly 0.3
//     c.div(&a, &b, 28, decimal.HALF_EVEN)!;   // 28 significant digits
//     c.round(&c, 2, decimal.HALF_UP)!;        // two decimal places
//     let s = c.to_string()!;                  // mem.free(s)
//
// A Decimal is a coefficient (a BigInt) times a power of ten, so decimal
// fractions such as 0.1 are exact, and trailing zeros are kept (1.50 has
// two places; normalize removes them). Addition, subtraction and
// multiplication are exact; division and square roots round to a given
// number of significant digits; round, quantize and round_sig round to a
// place, with one of the rounding modes below. Results agree with Python's
// decimal module (its to-sci-string format included), except that there
// is no negative zero, NaN or infinity.
//
// As in bigint, operations are methods on the result, which may be one of
// the operands: z.add(&x, &y). A Decimal owns its memory (deinit frees it).
//
// Exponents are limited to +-10^15, and results that would need more than
// MAX_DIGITS digits (such as 1e1000000 + 1) fail with Overflow, so hostile
// input cannot make an operation take unbounded time or memory.
// =============================================================================

import "../fmt.ph" as fmt;
import "../io.ph" as io;
import "../mem.ph" as mem;
import "../os/fs.ph" as fs;
import "../str.ph" as strs;
import "bigint.ph" as bigint;

error { InvalidNumber, Overflow, DivisionByZero, OutOfRange, OutOfMemory }

/// Rounding modes: to the nearest, ties to even (banker's rounding) ...
pub const HALF_EVEN: u8 = 0;
/// ... to the nearest, ties away from zero (school rounding) ...
pub const HALF_UP: u8 = 1;
/// ... to the nearest, ties toward zero ...
pub const HALF_DOWN: u8 = 2;
/// ... toward zero (truncation) ...
pub const DOWN: u8 = 3;
/// ... away from zero ...
pub const UP: u8 = 4;
/// ... toward minus infinity ...
pub const FLOOR: u8 = 5;
/// ... and toward plus infinity.
pub const CEILING: u8 = 6;

/// The most digits a result may have.
pub const MAX_DIGITS: u64 = 1000000;
const MAX_EXP: i64 = 1000000000000000;

pub struct Decimal {
    coef: bigint.BigInt,  // signed
    exp: i64,

    /// Zero, owning no memory.
    pub fn init() Decimal {
        return Decimal.{ coef: bigint.BigInt.init(), exp: 0 };
    }

    pub fn deinit(self: *mut Decimal) void {
        self.coef.deinit();
    }

    pub fn from_i64(v: i64) !Decimal {
        return Decimal.{ coef: bigint.BigInt.from_i64(v)!, exp: 0 };
    }

    /// coef x 10^exp.
    pub fn from_parts(coef: *bigint.BigInt, exp: i64) !Decimal {
        if (exp > MAX_EXP) or (exp < -MAX_EXP) {
            return error.Overflow;
        }
        return Decimal.{ coef: bigint.BigInt.clone(coef)!, exp: exp };
    }

    /// The shortest decimal that reads back as x (so 0.1 gives 0.1, not
    /// the binary value's 55 digits). Infinities and NaN are InvalidNumber.
    pub fn from_f64(x: f64) !Decimal {
        let mut buf: [40]u8 = [0; 40];
        let s = fmt.format_float(buf[..], x)!;
        return Decimal.parse(s)!;
    }

    /// Parses [+-]digits[.digits][e[+-]digits], with digits on at least one
    /// side of the point ("1.", ".5" and "1e3" are numbers). Surrounding
    /// spaces and underscores are not accepted.
    pub fn parse(s: []u8) !Decimal {
        let mut i: usize = 0;
        let mut neg = false;
        if (s.len > 0) and ((s[0] == '+') or (s[0] == '-')) {
            neg = s[0] == '-';
            i = 1;
        }
        let digits_start = i;
        while (i < s.len) and strs.is_digit(s[i]) {
            i += 1;
        }
        let int_end = i;
        let mut frac_start = i;
        let mut frac_end = i;
        if (i < s.len) and (s[i] == '.') {
            i += 1;
            frac_start = i;
            while (i < s.len) and strs.is_digit(s[i]) {
                i += 1;
            }
            frac_end = i;
        }
        let nd = (int_end - digits_start) + (frac_end - frac_start);
        if nd == 0 {
            return error.InvalidNumber;
        }
        if @as(u64, nd) > MAX_DIGITS {
            return error.Overflow;
        }
        let mut e: i64 = 0;
        if (i < s.len) and ((s[i] == 'e') or (s[i] == 'E')) {
            i += 1;
            let mut eneg = false;
            if (i < s.len) and ((s[i] == '+') or (s[i] == '-')) {
                eneg = s[i] == '-';
                i += 1;
            }
            if (i == s.len) or not strs.is_digit(s[i]) {
                return error.InvalidNumber;
            }
            while (i < s.len) and strs.is_digit(s[i]) {
                if e > MAX_EXP {
                    return error.Overflow;
                }
                e = (e * 10) + @as(i64, s[i] - '0');
                i += 1;
            }
            if eneg {
                e = -e;
            }
        }
        if i != s.len {
            return error.InvalidNumber;
        }
        // the digits without the point
        let buf = mem.alloc.[u8](nd + 1)!;
        defer mem.free(buf);
        buf[0] = '+';
        if neg {
            buf[0] = '-';
        }
        @mem_copy(buf[1..1 + (int_end - digits_start)], s[digits_start..int_end]);
        @mem_copy(buf[1 + (int_end - digits_start)..], s[frac_start..frac_end]);
        e -= @as(i64, @int_cast(u32, @min(frac_end - frac_start, 0xffffffff)));
        if (e > MAX_EXP) or (e < -MAX_EXP) {
            return error.Overflow;
        }
        return Decimal.{ coef: bigint.BigInt.parse(buf, 10)!, exp: e };
    }

    pub fn clone(self: *Decimal) !Decimal {
        return Decimal.{ coef: bigint.BigInt.clone(&self.coef)!, exp: self.exp };
    }

    pub fn set(self: *mut Decimal, x: *Decimal) !void {
        self.coef.set(&x.coef)!;
        self.exp = x.exp;
    }

    pub fn is_zero(self: *Decimal) bool {
        return self.coef.is_zero();
    }

    /// -1, 0 or 1.
    pub fn sign(self: *Decimal) i32 {
        return self.coef.sign();
    }

    /// The exponent: the value is coefficient x 10^exponent.
    pub fn exponent(self: *Decimal) i64 {
        return self.exp;
    }

    /// The coefficient, owned by the Decimal.
    pub fn coefficient(self: *Decimal) *bigint.BigInt {
        return &self.coef;
    }

    /// Whether the value is a whole number.
    pub fn is_integer(self: *Decimal) !bool {
        if (self.exp >= 0) or self.coef.is_zero() {
            return true;
        }
        let mut t = self.clone()!;
        defer t.deinit();
        t.normalize(&t)!;
        return t.exp >= 0;
    }

    /// Compares values: -1, 0 or 1 (1.0 and 1.00 are equal).
    pub fn cmp(self: *Decimal, x: *Decimal) !i32 {
        let sa = self.coef.sign();
        let sb = x.coef.sign();
        if sa != sb {
            if sa < sb {
                return -1;
            }
            return 1;
        }
        if sa == 0 {
            return 0;
        }
        // the magnitudes: by adjusted exponent first, then aligned
        let aa = self.exp + @as(i64, @int_cast(u32, digit_count(&self.coef)!));
        let ab = x.exp + @as(i64, @int_cast(u32, digit_count(&x.coef)!));
        let mut c: i32 = 0;
        if aa != ab {
            c = 1;
            if aa < ab {
                c = -1;
            }
        } else {
            let mut u = bigint.BigInt.init();
            defer u.deinit();
            let mut v = bigint.BigInt.init();
            defer v.deinit();
            align(&self.coef, self.exp, &x.coef, x.exp, &mut u, &mut v)!;
            c = u.cmp_abs(&v);
        }
        return c * sa;
    }

    pub fn eq(self: *Decimal, x: *Decimal) !bool {
        return (self.cmp(x)!) == 0;
    }

    pub fn neg_of(self: *mut Decimal, x: *Decimal) !void {
        self.coef.neg_of(&x.coef)!;
        self.exp = x.exp;
    }

    pub fn abs(self: *mut Decimal, x: *Decimal) !void {
        self.coef.abs(&x.coef)!;
        self.exp = x.exp;
    }

    /// self = x + y, exactly; the exponent is the smaller of theirs.
    pub fn add(self: *mut Decimal, x: *Decimal, y: *Decimal) !void {
        let mut u = bigint.BigInt.init();
        defer u.deinit();
        let mut v = bigint.BigInt.init();
        defer v.deinit();
        align(&x.coef, x.exp, &y.coef, y.exp, &mut u, &mut v)!;
        let e = @min(x.exp, y.exp);
        self.coef.add(&u, &v)!;
        self.exp = e;
    }

    /// self = x - y, exactly.
    pub fn sub(self: *mut Decimal, x: *Decimal, y: *Decimal) !void {
        let mut u = bigint.BigInt.init();
        defer u.deinit();
        let mut v = bigint.BigInt.init();
        defer v.deinit();
        align(&x.coef, x.exp, &y.coef, y.exp, &mut u, &mut v)!;
        let e = @min(x.exp, y.exp);
        self.coef.sub(&u, &v)!;
        self.exp = e;
    }

    /// self = x y, exactly; the exponents add.
    pub fn mul(self: *mut Decimal, x: *Decimal, y: *Decimal) !void {
        let e = x.exp + y.exp;
        if (e > MAX_EXP) or (e < -MAX_EXP) {
            return error.Overflow;
        }
        if @as(u64, digit_count(&x.coef)! + digit_count(&y.coef)!) > (MAX_DIGITS + 1) {
            return error.Overflow;
        }
        self.coef.mul(&x.coef, &y.coef)!;
        self.exp = e;
    }

    /// self = x / y, rounded to prec significant digits (1 or more). An
    /// exact quotient keeps no more trailing zeros than it needs to reach
    /// the exponent x.exp - y.exp (so 1.00 / 4 is 0.25 and 10 / 5 is 2).
    pub fn div(self: *mut Decimal, x: *Decimal, y: *Decimal, prec: u32, mode: u8) !void {
        if y.coef.is_zero() {
            return error.DivisionByZero;
        }
        if (prec == 0) or (@as(u64, prec) > MAX_DIGITS) {
            return error.OutOfRange;
        }
        let ideal = x.exp - y.exp;
        if x.coef.is_zero() {
            self.coef.set_i64(0)!;
            self.exp = clamp_exp(ideal)!;
            return;
        }
        let neg = x.coef.sign() != y.coef.sign();
        let mut a = bigint.BigInt.init();
        defer a.deinit();
        a.abs(&x.coef)!;
        let mut b = bigint.BigInt.init();
        defer b.deinit();
        b.abs(&y.coef)!;
        // scale so the quotient has at least prec + 1 digits
        let s = (
            (@as(i64, prec) + @as(i64, @int_cast(u32, digit_count(&b)!))) - @as(
                i64, @int_cast(u32, digit_count(&a)!))) + 1;
        if s > 0 {
            mul_pow10(&mut a, @as(u64, @bit_cast(u64, s)))!;
        } else if s < 0 {
            mul_pow10(&mut b, @bit_cast(u64, -s))!;
        }
        let mut q = bigint.BigInt.init();
        defer q.deinit();
        let mut r = bigint.BigInt.init();
        defer r.deinit();
        bigint.BigInt.quo_rem(&mut q, &mut r, &a, &b)!;
        let mut e = ideal - s;
        let exact = r.is_zero();
        if not exact {
            // a sticky digit: the quotient is more than q, less than q + 1
            q.mul_i64(&q, 10)!;
            q.add_i64(&q, 1)!;
            e -= 1;
        }
        if neg {
            q.neg_of(&q)!;
        }
        let n = digit_count(&q)!;
        if n > @as(usize, prec) {
            let k = n - @as(usize, prec);
            round_drop(&mut q, @as(u64, k), mode)!;
            e += @as(i64, @int_cast(u32, k));
            // rounding up can make a digit more (999 -> 1000)
            if digit_count(&q)! > @as(usize, prec) {
                round_drop(&mut q, 1, DOWN)!;
                e += 1;
            }
        }
        if exact {
            // remove trailing zeros down to the ideal exponent
            strip_zeros(&mut q, &mut e, ideal)!;
        }
        self.coef.set(&q)!;
        self.exp = clamp_exp(e)!;
    }

    /// self = the square root of x, to prec significant digits, rounded
    /// to nearest-even; exact roots keep trailing zeros only down to half
    /// x's exponent. Negative x is OutOfRange.
    pub fn sqrt(self: *mut Decimal, x: *Decimal, prec: u32) !void {
        if x.coef.sign() < 0 {
            return error.OutOfRange;
        }
        if (prec == 0) or (@as(u64, prec) > MAX_DIGITS) {
            return error.OutOfRange;
        }
        let ideal = floor_half(x.exp);
        if x.coef.is_zero() {
            self.coef.set_i64(0)!;
            self.exp = ideal;
            return;
        }
        let mut a = bigint.BigInt.clone(&x.coef)!;
        defer a.deinit();
        // scale by an even power so the root has at least prec + 1 digits
        let n = @as(i64, @int_cast(u32, digit_count(&a)!));
        let mut s = @max((2 * (@as(i64, prec) + 1)) - n, 0);
        if ((x.exp - s) % 2) != 0 {
            s += 1;
        }
        mul_pow10(&mut a, @bit_cast(u64, s))!;
        let mut r = bigint.BigInt.init();
        defer r.deinit();
        r.sqrt(&a)!;
        let mut e = floor_half(x.exp - s);
        let mut sq = bigint.BigInt.init();
        defer sq.deinit();
        sq.mul(&r, &r)!;
        let exact = sq.cmp(&a) == 0;
        if not exact {
            r.mul_i64(&r, 10)!;
            r.add_i64(&r, 1)!;
            e -= 1;
        }
        let nd = digit_count(&r)!;
        if nd > @as(usize, prec) {
            let k = nd - @as(usize, prec);
            round_drop(&mut r, @as(u64, k), HALF_EVEN)!;
            e += @as(i64, @int_cast(u32, k));
            if digit_count(&r)! > @as(usize, prec) {
                round_drop(&mut r, 1, DOWN)!;
                e += 1;
            }
        }
        if exact {
            strip_zeros(&mut r, &mut e, ideal)!;
        }
        self.coef.set(&r)!;
        self.exp = e;
    }

    /// self = x rounded to the exponent exp (so to -exp decimal places);
    /// a larger x.exp adds trailing zeros.
    pub fn quantize(self: *mut Decimal, x: *Decimal, exp: i64, mode: u8) !void {
        if (exp > MAX_EXP) or (exp < -MAX_EXP) {
            return error.Overflow;
        }
        if x.exp < exp {
            let k = @bit_cast(u64, exp - x.exp);
            self.coef.set(&x.coef)!;
            let n = @as(u64, digit_count(&x.coef)!);
            if k > n {
                // every digit goes: the result is 0 or one unit
                let s = x.coef.sign();
                self.coef.set_i64(0)!;
                // (x is below half a unit, so only UP, CEILING and FLOOR round it up)
                if (s != 0) and (((mode == UP) or ((mode == CEILING) and (s > 0))) or ((mode == FLOOR) and (s < 0))) {
                    self.coef.set_i64(@as(i64, s))!;
                }
            } else {
                round_drop(&mut self.coef, k, mode)!;
            }
        } else {
            let k = @bit_cast(u64, x.exp - exp);
            if (@as(u64, digit_count(&x.coef)!) + k) > MAX_DIGITS {
                return error.Overflow;
            }
            self.coef.set(&x.coef)!;
            if not self.coef.is_zero() {
                mul_pow10(&mut self.coef, k)!;
            }
        }
        self.exp = exp;
    }

    /// self = x rounded to `places` decimal places (negative: to tens,
    /// hundreds, ...).
    pub fn round(self: *mut Decimal, x: *Decimal, places: i64, mode: u8) !void {
        self.quantize(x, -places, mode)!;
    }

    /// self = x rounded to prec significant digits (unchanged if it has
    /// no more).
    pub fn round_sig(self: *mut Decimal, x: *Decimal, prec: u32, mode: u8) !void {
        if prec == 0 {
            return error.OutOfRange;
        }
        let n = digit_count(&x.coef)!;
        self.set(x)!;
        if n <= @as(usize, prec) {
            return;
        }
        let k = n - @as(usize, prec);
        round_drop(&mut self.coef, @as(u64, k), mode)!;
        self.exp = clamp_exp(x.exp + @as(i64, @int_cast(u32, k)))!;
        if digit_count(&self.coef)! > @as(usize, prec) {
            round_drop(&mut self.coef, 1, DOWN)!;
            self.exp = clamp_exp(self.exp + 1)!;
        }
    }

    /// self = x without trailing zeros (1.500 -> 1.5, 100 -> 1E+2).
    pub fn normalize(self: *mut Decimal, x: *Decimal) !void {
        self.set(x)!;
        if self.coef.is_zero() {
            self.exp = 0;
            return;
        }
        let mut e = self.exp;
        strip_zeros(&mut self.coef, &mut e, MAX_EXP)!;
        self.exp = e;
    }

    /// The value truncated toward zero, if it fits.
    pub fn to_i64(self: *Decimal) !i64 {
        let mut t = Decimal.init();
        defer t.deinit();
        if self.exp > 20 {
            if self.coef.is_zero() {
                return 0;
            }
            return error.Overflow;
        }
        t.quantize(self, 0, DOWN)!;
        return t.coef.to_i64() catch return error.Overflow;
    }

    /// The nearest f64 (correctly rounded).
    pub fn to_f64(self: *Decimal) !f64 {
        let s = self.to_string()!;
        defer mem.free(s);
        return fmt.parse_float(s) catch return error.OutOfRange;
    }

    /// Writes the number as Python's decimal does: plainly (123.45,
    /// 0.000012) unless the exponent is positive or the number is below
    /// 10^-6, then in scientific notation (1.2345E+7, 1.2E-7).
    pub fn format(self: *Decimal, w: io.Writer) !void {
        let ds = self.digits()!;
        defer mem.free(ds);
        if self.coef.sign() < 0 {
            w.write_all("-")!;
        }
        let n = @as(i64, @int_cast(u32, ds.len));
        let adjusted = self.exp + (n - 1);
        if (self.exp <= 0) and (adjusted >= -6) {
            if self.exp == 0 {
                w.write_all(ds)!;
            } else if n > -self.exp {
                let k = @as(usize, @int_cast(u32, n + self.exp));
                w.write_all(ds[..k])!;
                w.write_all(".")!;
                w.write_all(ds[k..])!;
            } else {
                w.write_all("0.")!;
                let mut z = -self.exp - n;
                while z > 0 {
                    w.write_all("0")!;
                    z -= 1;
                }
                w.write_all(ds)!;
            }
            return;
        }
        w.write_all(ds[..1])!;
        if ds.len > 1 {
            w.write_all(".")!;
            w.write_all(ds[1..])!;
        }
        w.write_all("E")!;
        if adjusted >= 0 {
            w.write_all("+")!;
        }
        let mut buf: [24]u8 = [0; 24];
        w.write_all(fmt.format_int(buf[..], adjusted))!;
    }

    /// Writes the number without an exponent (1E+3 as 1000, 1E-8 as
    /// 0.00000001); Overflow if that needs more than MAX_DIGITS digits.
    pub fn format_plain(self: *Decimal, w: io.Writer) !void {
        let ds = self.digits()!;
        defer mem.free(ds);
        let n = @as(i64, @int_cast(u32, ds.len));
        let width = @max(n, 0) + @abs(self.exp);
        if @bit_cast(u64, width) > MAX_DIGITS {
            return error.Overflow;
        }
        if self.coef.sign() < 0 {
            w.write_all("-")!;
        }
        if self.exp >= 0 {
            w.write_all(ds)!;
            if not self.coef.is_zero() {
                let mut z = self.exp;
                while z > 0 {
                    w.write_all("0")!;
                    z -= 1;
                }
            }
            return;
        }
        if n > -self.exp {
            let k = @as(usize, @int_cast(u32, n + self.exp));
            w.write_all(ds[..k])!;
            w.write_all(".")!;
            w.write_all(ds[k..])!;
            return;
        }
        w.write_all("0.")!;
        let mut z = -self.exp - n;
        while z > 0 {
            w.write_all("0")!;
            z -= 1;
        }
        w.write_all(ds)!;
    }

    /// format, into a new string the caller frees.
    pub fn to_string(self: *Decimal) ![]mut u8 {
        let mut s = strs.String.init();
        let w = s.writer();
        self.format(w) catch |e| {
            s.deinit();
            return e;
        };
        return s.take();
    }

    // the decimal digits of |coef| (the caller frees them)
    fn digits(self: *Decimal) ![]mut u8 {
        let mut a = bigint.BigInt.init();
        defer a.deinit();
        a.abs(&self.coef)!;
        return a.to_string(10)!;
    }
}

fn clamp_exp(e: i64) !i64 {
    if (e > MAX_EXP) or (e < -MAX_EXP) {
        return error.Overflow;
    }
    return e;
}

fn floor_half(e: i64) i64 {
    if e >= 0 {
        return e / 2;
    }
    return -((1 - e) / 2);
}

// 10^k
fn pow10(k: u64) !bigint.BigInt {
    let mut ten = bigint.BigInt.from_u64(10)!;
    defer ten.deinit();
    let mut r = bigint.BigInt.init();
    r.pow(&ten, k)!;
    return r;
}

// b *= 10^k
fn mul_pow10(b: *mut bigint.BigInt, k: u64) !void {
    if k == 0 {
        return;
    }
    if k > (MAX_DIGITS * 2) {
        return error.Overflow;
    }
    let mut p = pow10(k)!;
    defer p.deinit();
    b.mul(b, &p)!;
}

// The number of decimal digits of |b| (1 for zero).
fn digit_count(b: *bigint.BigInt) !usize {
    let bits = b.bit_len();
    if bits == 0 {
        return 1;
    }
    // 10^(d - 1) <= |b| < 10^d, and d is est or est + 1
    let est = ((@as(u64, bits - 1) * 30103) / 100000) + 1;
    let mut p = pow10(est)!;
    defer p.deinit();
    if b.cmp_abs(&p) >= 0 {
        return @truncate(usize, est + 1);
    }
    return @truncate(usize, est);
}

// u, v = the coefficients a and b scaled to the smaller exponent.
fn align(a: *bigint.BigInt, ea: i64, b: *bigint.BigInt, eb: i64, u: *mut bigint.BigInt, v: *mut bigint.BigInt) !void {
    u.set(a)!;
    v.set(b)!;
    if ea > eb {
        if not u.is_zero() {
            let k = @bit_cast(u64, ea - eb);
            if (@as(u64, digit_count(a)!) + k) > MAX_DIGITS {
                return error.Overflow;
            }
            mul_pow10(u, k)!;
        }
    } else if eb > ea {
        if not v.is_zero() {
            let k = @bit_cast(u64, eb - ea);
            if (@as(u64, digit_count(b)!) + k) > MAX_DIGITS {
                return error.Overflow;
            }
            mul_pow10(v, k)!;
        }
    }
}

// q = q / 10^k, rounded by mode (k >= 1).
fn round_drop(q: *mut bigint.BigInt, k: u64, mode: u8) !void {
    let neg = q.sign() < 0;
    let mut d = pow10(k)!;
    defer d.deinit();
    let mut quo = bigint.BigInt.init();
    defer quo.deinit();
    let mut rem = bigint.BigInt.init();
    defer rem.deinit();
    bigint.BigInt.quo_rem(&mut quo, &mut rem, q, &d)!;
    if not rem.is_zero() {
        // compare twice the remainder with the divisor
        let mut r2 = bigint.BigInt.init();
        defer r2.deinit();
        r2.abs(&rem)!;
        r2.shl(&r2, 1)!;
        let half = r2.cmp_abs(&d);  // <0: below half, 0: exactly half
        let mut up = false;
        if mode == UP {
            up = true;
        } else if mode == CEILING {
            up = not neg;
        } else if mode == FLOOR {
            up = neg;
        } else if mode == HALF_UP {
            up = half >= 0;
        } else if mode == HALF_DOWN {
            up = half > 0;
        } else if mode == HALF_EVEN {
            up = (half > 0) or ((half == 0) and quo.is_odd());
        }
        if up {
            if neg {
                quo.add_i64(&quo, -1)!;
            } else {
                quo.add_i64(&quo, 1)!;
            }
        }
    }
    q.set(&quo)!;
}

// Removes trailing zeros of q while e < limit.
fn strip_zeros(q: *mut bigint.BigInt, e: *mut i64, limit: i64) !void {
    if q.is_zero() {
        return;
    }
    let mut ten = bigint.BigInt.from_u64(10)!;
    defer ten.deinit();
    let mut quo = bigint.BigInt.init();
    defer quo.deinit();
    let mut rem = bigint.BigInt.init();
    defer rem.deinit();
    while e.* < limit {
        bigint.BigInt.quo_rem(&mut quo, &mut rem, q, &ten)!;
        if not rem.is_zero() {
            break;
        }
        q.set(&quo)!;
        e.* += 1;
    }
}

// -----------------------------------------------------------------------------
// Tests
// -----------------------------------------------------------------------------

fn dec(s: []u8) Decimal {
    return Decimal.parse(s) catch Decimal.init();
}

fn is(d: *Decimal, want: []u8) bool {
    let s = d.to_string() catch return false;
    defer mem.free(s);
    return strs.eq(s, want);
}

test "parse and format" {
    // str(Decimal(s)) in Python, for each s (but Python keeps the sign of -0)
    let cases: [16][2][]u8 = [
        ["0", "0"],
        ["-0.00", "0.00"],
        ["123.450", "123.450"],
        ["1e3", "1E+3"],
        ["1000", "1000"],
        ["0.000001", "0.000001"],
        ["0.0000001", "1E-7"],
        ["-12.5e-10", "-1.25E-9"],
        [".5", "0.5"],
        ["5.", "5"],
        ["+7", "7"],
        ["1.23E+5", "1.23E+5"],
        ["123e-2", "1.23"],
        ["0e5", "0E+5"],
        ["123456789012345678901234567890.5", "123456789012345678901234567890.5"],
        ["0.00012e2", "0.012"],
    ];
    for c in cases {
        let mut d = Decimal.parse(c[0])!;
        defer d.deinit();
        @expect(is(&d, c[1]));
    }
    let bad: [8][]u8 = ["", ".", "-", "1e", "1e+", "1..2", " 1", "0x10"];
    for b in bad {
        let mut d = Decimal.parse(b) catch |e| {
            @expect(e == error.InvalidNumber);
            continue;
        };
        d.deinit();
        @expect(false);
    }
    let mut p = dec("1.5E+3");
    defer p.deinit();
    let mut bw_buf: [32]u8 = [0; 32];
    let mut bw = io.BufferWriter.init(bw_buf[..]);
    p.format_plain(bw.writer())!;
    let mut q = dec("-12E-8");
    defer q.deinit();
    q.format_plain(bw.writer())!;
    @expect_eq(bw.written(), "1500-0.00000012");
    let mut f = Decimal.from_f64(0.1)!;
    defer f.deinit();
    @expect(is(&f, "0.1"));
    @expect(f.to_f64()! == 0.1);
    let mut big = dec("1e400");
    defer big.deinit();
    @expect(big.to_f64()! == @bit_cast(f64, @as(u64, 0x7ff0000000000000)));
}

test "arithmetic" {
    let mut a = dec("0.1");
    defer a.deinit();
    let mut b = dec("0.2");
    defer b.deinit();
    let mut c = Decimal.init();
    defer c.deinit();
    c.add(&a, &b)!;
    @expect(is(&c, "0.3"));
    let mut d = dec("1.30");
    defer d.deinit();
    let mut e = dec("1.2");
    defer e.deinit();
    c.add(&d, &e)!;
    @expect(is(&c, "2.50"));
    c.sub(&e, &d)!;
    @expect(is(&c, "-0.10"));
    c.mul(&d, &e)!;
    @expect(is(&c, "1.560"));
    // the result may be an operand
    c.mul(&c, &c)!;
    @expect(is(&c, "2.433600"));
    let mut big = dec("1e999999999");
    defer big.deinit();
    let mut one = dec("1");
    defer one.deinit();
    @expect(fails(c.add(&big, &one)));
    // comparison ignores trailing zeros and does not scale huge exponents
    let mut x = dec("1.50");
    defer x.deinit();
    let mut y = dec("1.5");
    defer y.deinit();
    @expect(x.cmp(&y)! == 0);
    @expect(big.cmp(&one)! == 1);
    let mut tiny = dec("-1e-999999999");
    defer tiny.deinit();
    @expect(tiny.cmp(&one)! == -1);
    @expect(one.cmp(&tiny)! == 1);
    let mut m = dec("-7.25");
    defer m.deinit();
    @expect(m.to_i64()! == -7);
    @expect(overflows(big.to_i64()));
    @expect(one.is_integer()!);
    @expect(not m.is_integer()!);
    let mut n = dec("12.000");
    defer n.deinit();
    @expect(n.is_integer()!);
    n.normalize(&n)!;
    @expect(is(&n, "12"));
    let mut h = dec("1200");
    defer h.deinit();
    h.normalize(&h)!;
    @expect(is(&h, "1.2E+3"));
}

fn overflows(r: !i64) bool {
    _ = r catch |e| return e == error.Overflow;
    return false;
}

fn fails(r: !void) bool {
    r catch return true;
    return false;
}

test "division and roots" {
    // Python: getcontext().prec = p; str(Decimal(a) / Decimal(b))
    let cases: [10][4][]u8 = [
        ["1", "3", "28", "0.3333333333333333333333333333"],
        ["2", "3", "28", "0.6666666666666666666666666667"],
        ["1", "4", "28", "0.25"],
        ["1.00", "4", "28", "0.25"],
        ["10", "5", "28", "2"],
        ["100", "5.0", "28", "2E+1"],
        ["-1", "7", "10", "-0.1428571429"],
        ["1e10", "3", "5", "3.3333E+9"],
        ["999", "1", "2", "1.0E+3"],
        ["12345", "0.001", "28", "1.2345E+7"],
    ];
    for k in @as(usize, 0)..10 {
        let cs = cases[k];
        let mut a = dec(cs[0]);
        defer a.deinit();
        let mut b = dec(cs[1]);
        defer b.deinit();
        let mut q = Decimal.init();
        defer q.deinit();
        q.div(&a, &b, fmt.parse_int.[u32](cs[2])!, HALF_EVEN)!;
        if not is(&q, cs[3]) {
            @expect_eq(k, 99);
        }
    }
    let mut z = dec("0");
    defer z.deinit();
    let mut o = dec("1");
    defer o.deinit();
    let mut q = Decimal.init();
    defer q.deinit();
    @expect(fails(q.div(&o, &z, 10, HALF_EVEN)));
    // Python: getcontext().prec = p; str(Decimal(a).sqrt())
    let roots: [7][3][]u8 = [
        ["2", "28", "1.414213562373095048801688724"],
        ["4", "28", "2"],
        ["0.25", "28", "0.5"],
        ["100", "28", "10"],
        ["1e-7", "10", "0.0003162277660"],
        ["16.00", "28", "4.0"],
        ["1e100", "5", "1E+50"],
    ];
    for k in @as(usize, 0)..7 {
        let rs = roots[k];
        let mut a = dec(rs[0]);
        defer a.deinit();
        q.sqrt(&a, fmt.parse_int.[u32](rs[1])!)!;
        if not is(&q, rs[2]) {
            @expect_eq(k, 99);
        }
    }
    let mut neg = dec("-4");
    defer neg.deinit();
    @expect(fails(q.sqrt(&neg, 10)));
}

test "rounding" {
    // Python: str(Decimal(v).quantize(Decimal('0.1'), rounding=mode)),
    // over HALF_EVEN, HALF_UP, HALF_DOWN, DOWN, UP, FLOOR, CEILING
    let vs: [6][]u8 = ["2.25", "2.35", "-2.25", "2.21", "-2.29", "0.04"];
    let want: [6][7][]u8 = [
        ["2.2", "2.3", "2.2", "2.2", "2.3", "2.2", "2.3"],
        ["2.4", "2.4", "2.3", "2.3", "2.4", "2.3", "2.4"],
        ["-2.2", "-2.3", "-2.2", "-2.2", "-2.3", "-2.3", "-2.2"],
        ["2.2", "2.2", "2.2", "2.2", "2.3", "2.2", "2.3"],
        ["-2.3", "-2.3", "-2.3", "-2.2", "-2.3", "-2.3", "-2.2"],
        ["0.0", "0.0", "0.0", "0.0", "0.1", "0.0", "0.1"],
    ];
    let mut r = Decimal.init();
    defer r.deinit();
    for k in @as(usize, 0)..6 {
        let mut v = dec(vs[k]);
        defer v.deinit();
        for m in @as(usize, 0)..7 {
            r.round(&v, 1, @truncate(u8, m))!;
            if not is(&r, want[k][m]) {
                @expect_eq((k * 10) + m, 99);
            }
        }
    }
    // rounding far past every digit, and adding places
    let mut v = dec("0.0004");
    defer v.deinit();
    r.round(&v, 1, UP)!;
    @expect(is(&r, "0.1"));
    r.round(&v, 1, HALF_UP)!;
    @expect(is(&r, "0.0"));
    r.round(&v, 6, HALF_UP)!;
    @expect(is(&r, "0.000400"));
    let mut w = dec("1234.5");
    defer w.deinit();
    r.round(&w, -2, HALF_EVEN)!;
    @expect(is(&r, "1.2E+3"));
    r.round_sig(&w, 3, HALF_EVEN)!;
    @expect(is(&r, "1.23E+3"));
    let mut nines = dec("9.99");
    defer nines.deinit();
    r.round_sig(&nines, 2, HALF_UP)!;
    @expect(is(&r, "10"));
    r.round(&nines, 1, HALF_UP)!;
    @expect(is(&r, "10.0"));
}

test "against Python's decimal" {
    let data = fs.read_file("std/math/decimal/testdata/cases.txt") catch return;
    defer mem.free(data);
    let mut lines = strs.lines(data);
    let mut count: usize = 0;
    while true {
        let line = lines.next() catch break;
        let mut f = strs.fields(line);
        let op = f.next()!;
        let mut a = Decimal.parse(f.next()!)!;
        defer a.deinit();
        let mut b = Decimal.parse(f.next()!)!;
        defer b.deinit();
        let n = fmt.parse_int.[i64](f.next()!)!;
        let mode = fmt.parse_int.[u8](f.next()!)!;
        let want = f.next()!;
        let mut r = Decimal.init();
        defer r.deinit();
        if strs.eq(op, "cmp") {
            let c = a.cmp(&b)!;
            @expect(c == fmt.parse_int.[i32](want)!);
            count += 1;
            continue;
        }
        if strs.eq(op, "add") {
            r.add(&a, &b)!;
        } else if strs.eq(op, "sub") {
            r.sub(&a, &b)!;
        } else if strs.eq(op, "mul") {
            r.mul(&a, &b)!;
        } else if strs.eq(op, "div") {
            r.div(&a, &b, @truncate(u32, @bit_cast(u64, n)), mode)!;
        } else if strs.eq(op, "sqrt") {
            r.sqrt(&a, @truncate(u32, @bit_cast(u64, n)))!;
        } else if strs.eq(op, "quantize") {
            r.quantize(&a, n, mode)!;
        } else if strs.eq(op, "round_sig") {
            r.round_sig(&a, @truncate(u32, @bit_cast(u64, n)), mode)!;
        }
        if not is(&r, want) {
            @expect_eq(line, "");
        }
        count += 1;
    }
    @expect(count == 600);
}
