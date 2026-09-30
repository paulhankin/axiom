// =============================================================================
// std/math/stats.ph - descriptive statistics and the normal distribution.
//
//     let m = stats.mean(xs)!;
//     let s = stats.stdev(xs)!;              // sample (n - 1) standard deviation
//     let q = stats.quantile(xs, 0.95)!;
//     let r = stats.correlation(xs, ys)!;
//     let fit = stats.linear_regression(xs, ys)!;   // fit.slope, fit.intercept
//
//     let mut acc = stats.Running.init();    // one pass, constant memory
//     for x in data { acc.push(x); }
//     acc.mean(); acc.stdev();
//
//     stats.normal_cdf(1.96);                // 0.975
//     stats.normal_quantile(0.975)!;         // 1.959963984540054
//
// Functions take []f64 and fail with Empty (or TooFew, for a variance of
// one value) rather than return NaN. Sums are compensated (Neumaier), and
// variances use two passes, so results are accurate to a few ulps even for
// data with a large mean. NaNs in the input give NaN results, except in the
// order statistics (median, quantile), which assume there are none.
// =============================================================================

import "../collections/vec.ph" as vec;
import "../math.ph" as math;
import "../mem.ph" as mem;

error { Empty, TooFew, DimensionMismatch, OutOfRange, OutOfMemory }

/// The sum, with Neumaier's compensation (exact to within an ulp or two
/// of the result, however the terms cancel).
pub fn sum(xs: []f64) f64 {
    let mut s: f64 = 0.0;
    let mut c: f64 = 0.0;
    for x in xs {
        let t = s + x;
        if @abs(s) >= @abs(x) {
            c += (s - t) + x;
        } else {
            c += (x - t) + s;
        }
        s = t;
    }
    return s + c;
}

pub fn mean(xs: []f64) !f64 {
    if xs.len == 0 {
        return error.Empty;
    }
    let n = @float_from_int(f64, xs.len);
    let m = sum(xs) / n;
    if not math.is_finite(m) {
        // the sum overflowed (or there are infinities or NaNs): scale first
        let mut s: f64 = 0.0;
        for x in xs {
            s += x / n;
        }
        return s;
    }
    return m;
}

// the sum of squared deviations from the mean, with a correction term
fn sum_sq_dev(xs: []f64, m: f64) f64 {
    let mut ss: f64 = 0.0;
    let mut c: f64 = 0.0;
    for x in xs {
        let d = x - m;
        ss += d * d;
        c += d;
    }
    // (the corrected two-pass formula: c is zero but for rounding)
    return ss - ((c * c) / @float_from_int(f64, xs.len));
}

/// The sample variance, with n - 1 in the denominator.
pub fn variance(xs: []f64) !f64 {
    if xs.len < 2 {
        return error.TooFew;
    }
    return sum_sq_dev(xs, mean(xs)!) / @float_from_int(f64, xs.len - 1);
}

/// The population variance, with n in the denominator.
pub fn pvariance(xs: []f64) !f64 {
    let m = mean(xs)!;
    return sum_sq_dev(xs, m) / @float_from_int(f64, xs.len);
}

/// The sample standard deviation.
pub fn stdev(xs: []f64) !f64 {
    return @sqrt(variance(xs)!);
}

/// The population standard deviation.
pub fn pstdev(xs: []f64) !f64 {
    return @sqrt(pvariance(xs)!);
}

pub fn min(xs: []f64) !f64 {
    if xs.len == 0 {
        return error.Empty;
    }
    let mut m = xs[0];
    for x in xs {
        if (x < m) or (x != x) {
            m = x;
        }
    }
    return m;
}

pub fn max(xs: []f64) !f64 {
    if xs.len == 0 {
        return error.Empty;
    }
    let mut m = xs[0];
    for x in xs {
        if (x > m) or (x != x) {
            m = x;
        }
    }
    return m;
}

/// The geometric mean, of positive values (OutOfRange otherwise).
pub fn geometric_mean(xs: []f64) !f64 {
    if xs.len == 0 {
        return error.Empty;
    }
    let mut s: f64 = 0.0;
    let mut c: f64 = 0.0;
    for x in xs {
        if not (x > 0.0) {
            return error.OutOfRange;
        }
        // compensated, like sum
        let l = math.log(x);
        let t = s + l;
        if @abs(s) >= @abs(l) {
            c += (s - t) + l;
        } else {
            c += (l - t) + s;
        }
        s = t;
    }
    return math.exp((s + c) / @float_from_int(f64, xs.len));
}

/// The harmonic mean, of positive values (OutOfRange otherwise).
pub fn harmonic_mean(xs: []f64) !f64 {
    if xs.len == 0 {
        return error.Empty;
    }
    let mut s: f64 = 0.0;
    for x in xs {
        if not (x > 0.0) {
            return error.OutOfRange;
        }
        s += 1.0 / x;
    }
    return @float_from_int(f64, xs.len) / s;
}

fn less(a: f64, b: f64) bool {
    return a < b;
}

/// Sorts xs into increasing order, in place.
pub fn sort(xs: []mut f64) void {
    vec.sort.[f64](xs, less);
}

/// The median: the middle value, or the mean of the middle two.
pub fn median(xs: []f64) !f64 {
    return quantile(xs, 0.5)!;
}

/// The p quantile (0 <= p <= 1), interpolating linearly between the order
/// statistics: with the values sorted, position p (n - 1). (This is
/// definition 7 of Hyndman and Fan, the default of R, NumPy and
/// spreadsheets.) It sorts a copy; for several quantiles of the same data,
/// sort once and use sorted_quantile.
pub fn quantile(xs: []f64, p: f64) !f64 {
    if xs.len == 0 {
        return error.Empty;
    }
    let s = mem.alloc.[f64](xs.len)!;
    defer mem.free(s);
    @mem_copy(s, xs);
    sort(s);
    return sorted_quantile(s, p)!;
}

/// quantile for data already in increasing order.
pub fn sorted_quantile(sorted: []f64, p: f64) !f64 {
    if sorted.len == 0 {
        return error.Empty;
    }
    if not ((p >= 0.0) and (p <= 1.0)) {
        return error.OutOfRange;
    }
    let h = p * @float_from_int(f64, sorted.len - 1);
    let lo = @floor(h);
    let i = @int_from_float(usize, lo);
    if (i + 1) >= sorted.len {
        return sorted[sorted.len - 1];
    }
    let f = h - lo;
    let a = sorted[i];
    let b = sorted[i + 1];
    if f == 0.0 {
        return a;
    }
    // (a + f (b - a) is exact at the ends and monotone in f)
    return a + (f * (b - a));
}

/// The sample covariance of paired data, with n - 1 in the denominator.
pub fn covariance(xs: []f64, ys: []f64) !f64 {
    if xs.len != ys.len {
        return error.DimensionMismatch;
    }
    if xs.len < 2 {
        return error.TooFew;
    }
    let mx = mean(xs)!;
    let my = mean(ys)!;
    let mut s: f64 = 0.0;
    for k in @as(usize, 0)..xs.len {
        s += (xs[k] - mx) * (ys[k] - my);
    }
    return s / @float_from_int(f64, xs.len - 1);
}

/// Pearson's correlation coefficient, in [-1, 1]. Constant data (no
/// variance) gives OutOfRange.
pub fn correlation(xs: []f64, ys: []f64) !f64 {
    if xs.len != ys.len {
        return error.DimensionMismatch;
    }
    if xs.len < 2 {
        return error.TooFew;
    }
    let mx = mean(xs)!;
    let my = mean(ys)!;
    let mut sxy: f64 = 0.0;
    let mut sxx: f64 = 0.0;
    let mut syy: f64 = 0.0;
    for k in @as(usize, 0)..xs.len {
        let dx = xs[k] - mx;
        let dy = ys[k] - my;
        sxy += dx * dy;
        sxx += dx * dx;
        syy += dy * dy;
    }
    if (sxx == 0.0) or (syy == 0.0) {
        return error.OutOfRange;
    }
    let r = sxy / (@sqrt(sxx) * @sqrt(syy));
    return @max(-1.0, @min(1.0, r));
}

/// Spearman's rank correlation: Pearson's correlation of the ranks (tied
/// values sharing the mean of their ranks).
pub fn spearman(xs: []f64, ys: []f64) !f64 {
    if xs.len != ys.len {
        return error.DimensionMismatch;
    }
    let rx = mem.alloc.[f64](xs.len)!;
    defer mem.free(rx);
    let ry = mem.alloc.[f64](ys.len)!;
    defer mem.free(ry);
    ranks(xs, rx)!;
    ranks(ys, ry)!;
    return correlation(rx, ry)!;
}

/// The ranks of xs (1 for the smallest), ties getting the mean of the
/// ranks they span.
pub fn ranks(xs: []f64, out: []mut f64) !void {
    if out.len != xs.len {
        return error.DimensionMismatch;
    }
    let n = xs.len;
    let idx = mem.alloc.[Ranked](n)!;
    defer mem.free(idx);
    for k in @as(usize, 0)..n {
        idx[k] = Ranked.{ v: xs[k], i: k };
    }
    vec.sort.[Ranked](idx, ranked_less);
    let mut a: usize = 0;
    while a < n {
        let mut b = a + 1;
        while (b < n) and (idx[b].v == idx[a].v) {
            b += 1;
        }
        // ranks a + 1 .. b, averaged
        let r = @float_from_int(f64, a + b + 1) * 0.5;
        for k in a..b {
            out[idx[k].i] = r;
        }
        a = b;
    }
}

struct Ranked {
    v: f64,
    i: usize,
}

fn ranked_less(a: Ranked, b: Ranked) bool {
    return a.v < b.v;
}

/// A least-squares line y = slope x + intercept.
pub struct Fit {
    slope: f64,
    intercept: f64,
    r2: f64,  // the coefficient of determination
}

/// The ordinary least-squares line through paired data. Constant xs give
/// OutOfRange.
pub fn linear_regression(xs: []f64, ys: []f64) !Fit {
    if xs.len != ys.len {
        return error.DimensionMismatch;
    }
    if xs.len < 2 {
        return error.TooFew;
    }
    let mx = mean(xs)!;
    let my = mean(ys)!;
    let mut sxy: f64 = 0.0;
    let mut sxx: f64 = 0.0;
    let mut syy: f64 = 0.0;
    for k in @as(usize, 0)..xs.len {
        let dx = xs[k] - mx;
        let dy = ys[k] - my;
        sxy += dx * dy;
        sxx += dx * dx;
        syy += dy * dy;
    }
    if sxx == 0.0 {
        return error.OutOfRange;
    }
    let slope = sxy / sxx;
    let mut r2: f64 = 1.0;
    if syy != 0.0 {
        r2 = @min(1.0, (sxy * sxy) / (sxx * syy));
    }
    return Fit.{ slope: slope, intercept: my - (slope * mx), r2: r2 };
}

/// Counts xs into counts.len equal bins over [lo, hi): bin k holds
/// lo + k w <= x < lo + (k + 1) w, with w = (hi - lo) / counts.len. Values
/// outside the range (and NaNs) are not counted; the number counted is
/// returned. The counts are added to, so clear them first.
pub fn histogram(xs: []f64, lo: f64, hi: f64, counts: []mut usize) !usize {
    if (counts.len == 0) or not (hi > lo) {
        return error.OutOfRange;
    }
    let nb = @float_from_int(f64, counts.len);
    let scale = nb / (hi - lo);
    let mut n: usize = 0;
    for x in xs {
        if not ((x >= lo) and (x < hi)) {
            continue;
        }
        let f = @floor((x - lo) * scale);
        // (rounding can put a value just under hi at nb)
        let k = @int_from_float(usize, @min(f, nb - 1.0));
        counts[k] += 1;
        n += 1;
    }
    return n;
}

/// Statistics of a stream of values, in one pass and constant memory
/// (Welford's algorithm).
pub struct Running {
    n: u64,
    m: f64,   // the mean so far
    m2: f64,  // the sum of squared deviations from it
    lo: f64,
    hi: f64,

    pub fn init() Running {
        return Running.{ n: 0, m: 0.0, m2: 0.0, lo: math.inf(), hi: -math.inf() };
    }

    pub fn push(self: *mut Running, x: f64) void {
        self.n += 1;
        let d = x - self.m;
        self.m += d / @float_from_int(f64, self.n);
        self.m2 += d * (x - self.m);
        if x < self.lo {
            self.lo = x;
        }
        if x > self.hi {
            self.hi = x;
        }
    }

    /// Combines another stream's statistics into this one (Chan et al.),
    /// as if its values had been pushed here.
    pub fn merge(self: *mut Running, o: *Running) void {
        if o.n == 0 {
            return;
        }
        if self.n == 0 {
            self.* = o.*;
            return;
        }
        let na = @float_from_int(f64, self.n);
        let nb = @float_from_int(f64, o.n);
        let n = na + nb;
        let d = o.m - self.m;
        self.m += d * (nb / n);
        self.m2 += o.m2 + (((d * d) * na) * (nb / n));
        self.n += o.n;
        self.lo = @min(self.lo, o.lo);
        self.hi = @max(self.hi, o.hi);
    }

    pub fn count(self: *Running) u64 {
        return self.n;
    }

    /// The mean (NaN when empty).
    pub fn mean(self: *Running) f64 {
        if self.n == 0 {
            return math.nan();
        }
        return self.m;
    }

    /// The sample variance (NaN with fewer than two values).
    pub fn variance(self: *Running) f64 {
        if self.n < 2 {
            return math.nan();
        }
        return self.m2 / @float_from_int(f64, self.n - 1);
    }

    pub fn pvariance(self: *Running) f64 {
        if self.n == 0 {
            return math.nan();
        }
        return self.m2 / @float_from_int(f64, self.n);
    }

    pub fn stdev(self: *Running) f64 {
        return @sqrt(self.variance());
    }

    pub fn pstdev(self: *Running) f64 {
        return @sqrt(self.pvariance());
    }

    /// The smallest and largest values (infinities when empty).
    pub fn min(self: *Running) f64 {
        return self.lo;
    }

    pub fn max(self: *Running) f64 {
        return self.hi;
    }
}

// -----------------------------------------------------------------------------
// The normal distribution
// -----------------------------------------------------------------------------

const SQRT1_2: f64 = 0.707106781186547524401;
const INV_SQRT_2PI: f64 = 0.398942280401432677940;

/// The standard normal density.
pub fn normal_pdf(x: f64) f64 {
    return INV_SQRT_2PI * math.exp(-0.5 * (x * x));
}

/// The standard normal distribution function, P(Z <= x), to within
/// 1e-15 relative near the middle; far into the lower tail the error grows
/// like x^2 ulps (the rounding of x / sqrt 2, magnified).
pub fn normal_cdf(x: f64) f64 {
    return 0.5 * math.erfc(-x * SQRT1_2);
}

/// P(Z > x), accurate where it is tiny.
pub fn normal_sf(x: f64) f64 {
    return 0.5 * math.erfc(x * SQRT1_2);
}

/// The inverse of normal_cdf: the x with P(Z <= x) = p, for 0 < p < 1 (0
/// and 1 give infinities). Acklam's approximation, refined by a step of
/// Halley's method, which brings it to within a few ulps.
pub fn normal_quantile(p: f64) !f64 {
    if not ((p >= 0.0) and (p <= 1.0)) {
        return error.OutOfRange;
    }
    if p == 0.0 {
        return -math.inf();
    }
    if p == 1.0 {
        return math.inf();
    }
    let a: [6]f64 = [-3.969683028665376e+01, 2.209460984245205e+02, -2.759285104469687e+02, 1.383577518672690e+02,
                     -3.066479806614716e+01, 2.506628277459239e+00];
    let b: [5]f64 = [-5.447609879822406e+01, 1.615858368580409e+02, -1.556989798598866e+02, 6.680131188771972e+01,
                     -1.328068155288572e+01];
    let c: [6]f64 = [-7.784894002430293e-03, -3.223964580411365e-01, -2.400758277161838e+00, -2.549732539343734e+00,
                     4.374664141464968e+00, 2.938163982698783e+00];
    let d: [4]f64 = [7.784695709041462e-03, 3.224671290700398e-01, 2.445134137142996e+00, 3.754408661907416e+00];
    let plow: f64 = 0.02425;
    let mut x: f64 = 0.0;
    if p < plow {
        let q = @sqrt(-2.0 * math.log(p));
        x = horner(c[..], q) / ((horner(d[..], q) * q) + 1.0);
    } else if p <= (1.0 - plow) {
        let q = p - 0.5;
        let r = q * q;
        x = (horner(a[..], r) * q) / ((horner(b[..], r) * r) + 1.0);
    } else {
        let q = @sqrt(-2.0 * math.log1p(-p));
        x = -(horner(c[..], q) / ((horner(d[..], q) * q) + 1.0));
    }
    // Halley: e = cdf(x) - p, computed on the smaller tail for accuracy
    for _k in @as(usize, 0)..2 {
        let mut e: f64 = 0.0;
        if x < 0.0 {
            e = normal_cdf(x) - p;
        } else {
            e = (1.0 - p) - normal_sf(x);
        }
        let pdf = normal_pdf(x);
        if pdf == 0.0 {
            break;
        }
        let u = e / pdf;
        x -= u / (1.0 + ((x * u) * 0.5));
    }
    return x;
}

// cs[0] x^(n-1) + ... + cs[n-1]
fn horner(cs: []f64, x: f64) f64 {
    let mut r: f64 = 0.0;
    for v in cs {
        r = (r * x) + v;
    }
    return r;
}

// -----------------------------------------------------------------------------
// Tests
// -----------------------------------------------------------------------------

fn near(a: f64, b: f64, rel: f64) bool {
    return @abs(a - b) <= (rel * @max(@abs(b), 1.0e-300));
}

fn fails(r: !f64) bool {
    _ = r catch return true;
    return false;
}

test "summaries" {
    // values from Python's statistics module
    let xs: [10]f64 = [2.5, 3.25, 1.0, 9.75, 4.5, 3.25, 7.0, 0.5, 6.0, 2.25];
    @expect(sum(xs[..]) == 40.0);
    @expect(mean(xs[..])! == 4.0);
    @expect(near(variance(xs[..])!, 8.222222222222221, 1.0e-15));
    @expect(near(pvariance(xs[..])!, 7.4, 1.0e-15));
    @expect(near(stdev(xs[..])!, 2.8674417556808756, 1.0e-15));
    @expect(near(pstdev(xs[..])!, 2.7202941017470885, 1.0e-15));
    @expect(min(xs[..])! == 0.5);
    @expect(max(xs[..])! == 9.75);
    @expect(median(xs[..])! == 3.25);
    @expect(median(xs[..9])! == 3.25);
    @expect(median(xs[..8])! == 3.25);
    @expect(median(xs[..4])! == 2.875);
    @expect(quantile(xs[..], 0.0)! == 0.5);
    @expect(quantile(xs[..], 1.0)! == 9.75);
    // numpy.quantile(xs, 0.9) and 0.25
    @expect(near(quantile(xs[..], 0.9)!, 7.274999999999999, 1.0e-15));
    @expect(near(quantile(xs[..], 0.25)!, 2.3125, 1.0e-15));
    @expect(near(geometric_mean(xs[..])!, 2.9773683491185157, 1.0e-15));
    @expect(near(harmonic_mean(xs[..])!, 1.9630401955849572, 1.0e-15));
    @expect(fails(mean(xs[..0])));
    @expect(fails(variance(xs[..1])));
    @expect(fails(quantile(xs[..], 1.5)));
    let zero: [2]f64 = [1.0, 0.0];
    @expect(fails(geometric_mean(zero[..])));
    // a large mean does not spoil the variance, and cancellation not the sum
    let big: [4]f64 = [1.0e9 + 4.0, 1.0e9 + 7.0, 1.0e9 + 13.0, 1.0e9 + 16.0];
    @expect(variance(big[..])! == 30.0);
    let canc: [4]f64 = [1.0e100, 1.0, -1.0e100, 1.0];
    @expect(sum(canc[..]) == 2.0);
    let huge: [2]f64 = [1.5e308, 1.5e308];
    @expect(mean(huge[..])! == 1.5e308);
}

test "paired data" {
    let xs: [6]f64 = [1.0, 2.0, 3.0, 4.0, 5.0, 6.0];
    let ys: [6]f64 = [2.1, 3.9, 6.2, 7.8, 10.1, 12.0];
    // from Python: statistics.covariance, correlation, linear_regression
    @expect(near(covariance(xs[..], ys[..])!, 6.969999999999999, 1.0e-14));
    @expect(near(correlation(xs[..], ys[..])!, 0.9992296688695032, 1.0e-14));
    let fit = linear_regression(xs[..], ys[..])!;
    @expect(near(fit.slope, 1.991428571428571, 1.0e-14));
    @expect(near(fit.intercept, 0.046666666666667744, 1.0e-13));
    @expect(near(fit.r2, 0.998459931149057, 1.0e-14));
    // ranks with ties, and Spearman's coefficient
    let t: [6]f64 = [10.0, 20.0, 10.0, 30.0, 20.0, 5.0];
    let mut r: [6]f64 = [0.0; 6];
    ranks(t[..], r[..])!;
    let want: [6]f64 = [2.5, 4.5, 2.5, 6.0, 4.5, 1.0];
    for k in @as(usize, 0)..6 {
        @expect(r[k] == want[k]);
    }
    let sq: [6]f64 = [1.0, 4.0, 9.0, 16.0, 25.0, 36.0];
    @expect(spearman(xs[..], sq[..])! == 1.0);
    @expect(fails(correlation(xs[..], sq[..3])));
    let flat: [3]f64 = [2.0, 2.0, 2.0];
    @expect(fails(correlation(flat[..], xs[..3])));
}

test "histogram" {
    let xs: [9]f64 = [0.0, 0.1, 0.25, 0.5, 0.75, 0.999999999, 1.0, -0.1, math.nan()];
    let mut c: [4]usize = [0; 4];
    @expect(histogram(xs[..], 0.0, 1.0, c[..])! == 6);
    @expect((c[0] == 2) and (c[1] == 1) and (c[2] == 1) and (c[3] == 2));
}

test "running" {
    let xs: [10]f64 = [2.5, 3.25, 1.0, 9.75, 4.5, 3.25, 7.0, 0.5, 6.0, 2.25];
    let mut a = Running.init();
    let mut b = Running.init();
    for k in @as(usize, 0)..10 {
        if k < 4 {
            a.push(xs[k]);
        } else {
            b.push(xs[k]);
        }
    }
    let mut all = Running.init();
    for x in xs {
        all.push(x);
    }
    @expect(near(all.mean(), 4.0, 1.0e-15));
    @expect(near(all.variance(), 8.222222222222221, 1.0e-14));
    a.merge(&b);
    @expect(a.count() == 10);
    @expect(near(a.mean(), 4.0, 1.0e-15));
    @expect(near(a.variance(), 8.222222222222221, 1.0e-14));
    @expect((a.min() == 0.5) and (a.max() == 9.75));
    let e = Running.init();
    @expect(math.is_nan(e.mean()));
}

test "normal distribution" {
    // from Python's statistics.NormalDist
    @expect(near(normal_cdf(1.96), 0.9750021048517795, 1.0e-15));
    @expect(near(normal_cdf(-1.0), 0.15865525393145707, 1.0e-15));
    @expect(near(normal_cdf(-10.0), 7.619853024160593e-24, 5.0e-14));
    @expect(near(normal_sf(10.0), 7.619853024160593e-24, 5.0e-14));
    @expect(near(normal_pdf(0.5), 0.3520653267642995, 1.0e-15));
    let ps: [9]f64 = [1.0e-300, 1.0e-10, 0.001, 0.02425, 0.1, 0.5, 0.9, 0.975, 0.999999];
    let want: [9]f64 = [-37.0470962993612, -6.361340902404056, -3.090232306167813, -1.9729610513118845,
                        -1.2815515655446008, 0.0, 1.2815515655446008, 1.9599639845400536, 4.753424308817089];
    for k in @as(usize, 0)..9 {
        @expect(near(normal_quantile(ps[k])!, want[k], 4.0e-16) or (@abs(normal_quantile(ps[k])! - want[k]) < 1.0e-16));
    }
    @expect(normal_quantile(0.0)! == -math.inf());
    @expect(fails(normal_quantile(1.5)));
}
