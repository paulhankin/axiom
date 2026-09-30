// =============================================================================
// std/math/rand.ph - fast, seedable pseudo-random numbers (not for secrets: use
// std/crypto/random for keys, tokens and nonces).
//
//     let mut r = rand.Rng.seed(42);           // the same sequence every run
//     let mut e = rand.Rng.from_entropy()!;    // a different one each run
//     let die = r.range(1, 6);                 // 1..=6, uniform
//     let x = r.float();                       // [0, 1)
//     let g = r.normal(0.0, 1.0);
//     rand.shuffle(&mut r, items[..]);
//
// The generator is xoshiro256** (Blackman and Vigna), seeded through
// splitmix64. `jump` advances by 2^128 steps, for independent streams on
// several threads.
// =============================================================================

import "../crypto/random.ph" as random;
import "../math.ph" as math;

error { OutOfMemory, Unavailable, InvalidArgument }

pub struct Rng {
    s: [4]u64,

    /// A generator whose sequence is fixed by seed.
    pub fn seed(seed: u64) Rng {
        let mut x = seed;
        let mut r = Rng.{ s: [0; 4] };
        for i in @as(usize, 0)..4 {
            x +%= 0x9E3779B97F4A7C15;
            let mut z = x;
            z = (z ^ (z >> 30)) *% 0xBF58476D1CE4E5B9;
            z = (z ^ (z >> 27)) *% 0x94D049BB133111EB;
            r.s[i] = z ^ (z >> 31);
        }
        return r;
    }

    /// A generator seeded from the system's secure random source.
    pub fn from_entropy() !Rng {
        let mut r = Rng.{ s: [0; 4] };
        while ((r.s[0] | r.s[1]) | (r.s[2] | r.s[3])) == 0 {
            for i in @as(usize, 0)..4 {
                r.s[i] = random.uint64()!;
            }
        }
        return r;
    }

    /// The next 64 random bits.
    pub fn next_u64(self: *mut Rng) u64 {
        let s = &mut self.s;
        let result = @rotl(s[1] *% 5, 7) *% 9;
        let t = s[1] << 17;
        s[2] ^= s[0];
        s[3] ^= s[1];
        s[1] ^= s[2];
        s[0] ^= s[3];
        s[2] ^= t;
        s[3] = @rotl(s[3], 45);
        return result;
    }

    pub fn next_u32(self: *mut Rng) u32 {
        return @truncate(u32, self.next_u64() >> 32);
    }

    /// Uniform in [0, n) without bias (Lemire's method); n > 0.
    pub fn below(self: *mut Rng, n: u64) u64 {
        if n == 0 {
            @trap();
        }
        let mut m = mul_wide(self.next_u64(), n);
        if m[1] < n {
            let t = (0 -% n) % n;
            while m[1] < t {
                m = mul_wide(self.next_u64(), n);
            }
        }
        return m[0];
    }

    /// Uniform in [lo, hi], both included (lo <= hi).
    pub fn range(self: *mut Rng, lo: i64, hi: i64) i64 {
        if lo > hi {
            @trap();
        }
        let span = @bit_cast(u64, hi -% lo);
        if span == @max_value(u64) {
            return @bit_cast(i64, self.next_u64());
        }
        return lo +% @bit_cast(i64, self.below(span + 1));
    }

    /// A uniform index into a collection of n elements.
    pub fn index(self: *mut Rng, n: usize) usize {
        return @truncate(usize, self.below(@as(u64, n)));
    }

    /// Uniform in [0, 1), with 53 random bits.
    pub fn float(self: *mut Rng) f64 {
        return @float_from_int(f64, self.next_u64() >> 11) * 1.1102230246251565e-16;
    }

    /// Uniform in [lo, hi).
    pub fn float_range(self: *mut Rng, lo: f64, hi: f64) f64 {
        return lo + ((hi - lo) * self.float());
    }

    pub fn boolean(self: *mut Rng) bool {
        return (self.next_u64() >> 63) == 1;
    }

    /// true with probability p.
    pub fn chance(self: *mut Rng, p: f64) bool {
        return self.float() < p;
    }

    /// Fills buf with random bytes.
    pub fn fill(self: *mut Rng, buf: []mut u8) void {
        let mut i: usize = 0;
        while i < buf.len {
            let x = self.next_u64();
            let n = @min(8, buf.len - i);
            for k in @as(usize, 0)..n {
                buf[i + k] = @truncate(u8, x >> @truncate(u64, 8 * k));
            }
            i += n;
        }
    }

    /// Normally distributed (Marsaglia's polar method).
    pub fn normal(self: *mut Rng, mean: f64, sd: f64) f64 {
        loop {
            let u = (2.0 * self.float()) - 1.0;
            let v = (2.0 * self.float()) - 1.0;
            let s = (u * u) + (v * v);
            if (s > 0.0) and (s < 1.0) {
                return mean + ((sd * u) * @sqrt((-2.0 * math.log(s)) / s));
            }
        }
    }

    /// Exponentially distributed with the given rate (mean 1/rate).
    pub fn exponential(self: *mut Rng, rate: f64) f64 {
        return -math.log(1.0 - self.float()) / rate;
    }

    /// Poisson distributed with mean lambda (Knuth's method for small
    /// means, a normal approximation above 500).
    pub fn poisson(self: *mut Rng, lambda: f64) u64 {
        if lambda > 500.0 {
            let x = @nearest(self.normal(lambda, @sqrt(lambda)));
            return @int_from_float(u64, @max(x, 0.0));
        }
        let l = math.exp(-lambda);
        let mut k: u64 = 0;
        let mut p: f64 = 1.0;
        loop {
            p = p * self.float();
            if p <= l {
                return k;
            }
            k += 1;
        }
    }

    /// Index i with probability weights[i] / sum(weights).
    pub fn weighted(self: *mut Rng, weights: []f64) !usize {
        let mut total: f64 = 0.0;
        for w in weights {
            if not (w >= 0.0) {
                return error.InvalidArgument;
            }
            total += w;
        }
        if not (total > 0.0) {
            return error.InvalidArgument;
        }
        let mut x = self.float() * total;
        for i in @as(usize, 0)..weights.len {
            if x < weights[i] {
                return i;
            }
            x -= weights[i];
        }
        // rounding: the last index with any weight
        let mut i = weights.len;
        while i > 0 {
            i -= 1;
            if weights[i] > 0.0 {
                return i;
            }
        }
        return 0;
    }

    /// Advances as far as 2^128 calls to next_u64: gives a stream that does
    /// not overlap this one for any practical length.
    pub fn jump(self: *mut Rng) void {
        let J: [4]u64 = [0x180ec6d33cfd0aba, 0xd5a61266f0c9392c, 0xa9582618e03fc9aa, 0x39abdc4529b1661c];
        let mut t: [4]u64 = [0; 4];
        for j in J {
            for b in @as(u64, 0)..64 {
                if ((j >> b) & 1) == 1 {
                    for i in @as(usize, 0)..4 {
                        t[i] ^= self.s[i];
                    }
                }
                _ = self.next_u64();
            }
        }
        self.s = t;
    }
}

// The 128-bit product of a and b as [high, low].
fn mul_wide(a: u64, b: u64) [2]u64 {
    let a0 = a & 0xffffffff;
    let a1 = a >> 32;
    let b0 = b & 0xffffffff;
    let b1 = b >> 32;
    let p00 = a0 *% b0;
    let p01 = a0 *% b1;
    let p10 = a1 *% b0;
    let p11 = a1 *% b1;
    let mid = ((p00 >> 32) +% (p01 & 0xffffffff)) +% (p10 & 0xffffffff);
    let lo = (p00 & 0xffffffff) | (mid << 32);
    let hi = ((p11 +% (p01 >> 32)) +% (p10 >> 32)) +% (mid >> 32);
    return [hi, lo];
}

/// Puts xs in a uniformly random order (Fisher-Yates).
pub fn shuffle[T](r: *mut Rng, xs: []mut T) void {
    let mut i = xs.len;
    while i > 1 {
        let j = r.index(i);
        i -= 1;
        let t = xs[i];
        xs[i] = xs[j];
        xs[j] = t;
    }
}

/// A uniformly random element of xs (not empty).
pub fn choose[T](r: *mut Rng, xs: []T) T {
    return xs[r.index(xs.len)];
}

// -----------------------------------------------------------------------------
// Tests
// -----------------------------------------------------------------------------

test "xoshiro256** reference outputs" {
    let mut r = Rng.{ s: [1, 2, 3, 4] };
    @expect_eq(r.next_u64(), 11520);
    @expect_eq(r.next_u64(), 0);
    @expect_eq(r.next_u64(), 1509978240);
    @expect_eq(r.next_u64(), 1215971899390074240);
    let mut s = Rng.seed(42);
    @expect_eq(s.next_u64(), 0x15780b2e0c2ec716);
}

test "wide multiplication" {
    let m = mul_wide(0xffffffffffffffff, 0xffffffffffffffff);
    @expect_eq(m[0], 0xfffffffffffffffe);
    @expect_eq(m[1], 1);
    let n = mul_wide(0x123456789abcdef0, 0x0fedcba987654321);
    @expect_eq(n[0], 0x0121fa00ad77d742);
    @expect_eq(n[1], 0x2236d88fe5618cf0);
}

test "ranges and distributions" {
    let mut r = Rng.seed(7);
    let mut counts: [6]u32 = [0; 6];
    for _i in @as(usize, 0)..60000 {
        let d = r.range(1, 6);
        @expect((d >= 1) and (d <= 6));
        counts[@as(usize, @truncate(u8, @bit_cast(u64, d - 1)))] += 1;
    }
    for c in counts {
        @expect((c > 9500) and (c < 10500));
    }
    @expect_eq(r.range(5, 5), 5);
    let big = r.range(@min_value(i64), @max_value(i64));
    _ = big;
    let mut sum: f64 = 0.0;
    let mut sq: f64 = 0.0;
    for _i in @as(usize, 0)..20000 {
        let x = r.normal(3.0, 2.0);
        sum += x;
        sq += x * x;
    }
    let mean = sum / 20000.0;
    let sd = @sqrt((sq / 20000.0) - (mean * mean));
    @expect(@abs(mean - 3.0) < 0.1);
    @expect(@abs(sd - 2.0) < 0.1);
    let mut e: f64 = 0.0;
    for _i in @as(usize, 0)..20000 {
        let f = r.float();
        @expect((f >= 0.0) and (f < 1.0));
        e += r.exponential(4.0);
    }
    @expect(@abs((e / 20000.0) - 0.25) < 0.02);
    let mut p: u64 = 0;
    for _i in @as(usize, 0)..2000 {
        p += r.poisson(3.0);
    }
    @expect((p > 5600) and (p < 6400));
    let w: [3]f64 = [0.0, 1.0, 3.0];
    let mut hits: [3]u32 = [0; 3];
    for _i in @as(usize, 0)..8000 {
        hits[r.weighted(w[..])!] += 1;
    }
    @expect_eq(hits[0], 0);
    @expect((hits[2] > 5700) and (hits[2] < 6300));
    let none: [2]f64 = [0.0, 0.0];
    _ = r.weighted(none[..]) catch |err| {
        @expect_eq(err, error.InvalidArgument);
        return;
    };
    @trap();
}

test "shuffle, choose, jump, fill" {
    let mut r = Rng.seed(1);
    let mut xs: [50]u32 = [0; 50];
    for i in @as(usize, 0)..50 {
        xs[i] = @truncate(u32, i);
    }
    shuffle(&mut r, xs[..]);
    let mut seen: u64 = 0;
    let mut moved = false;
    for i in @as(usize, 0)..50 {
        seen |= @as(u64, 1) << @as(u64, xs[i]);
        if xs[i] != @truncate(u32, i) {
            moved = true;
        }
    }
    @expect_eq(seen, (@as(u64, 1) << 50) - 1);
    @expect(moved);
    @expect(choose(&mut r, xs[..]) < 50);
    let mut a = Rng.seed(9);
    let mut b = a;
    b.jump();
    @expect(a.next_u64() != b.next_u64());
    let mut buf: [13]u8 = [0; 13];
    a.fill(buf[..]);
    let mut e = Rng.from_entropy()!;
    _ = e.next_u64();
}
