// =============================================================================
// std/math.ph - elementary functions on f64 (and f32).
//
//     let y = math.sin(x) * math.exp(-t);
//     let h = math.atan2(dy, dx);
//     let g = math.powf(c, 1.0 / 2.4);      // f32 versions end in f
//
// Square roots, floor, ceil, trunc, rounding, fma, remainder, copysign and
// abs are builtins (@sqrt, @floor, ...). The functions here follow the
// algorithms of fdlibm (as in musl and FreeBSD). Measured against exact
// results, they are within 1 ulp over the whole domain (trigonometry of huge
// arguments included), except tan, expm1, sinh and tanh (2 ulps). The f32
// versions compute in f64 and round.
// =============================================================================

pub const PI: f64 = 3.14159265358979323846;
pub const TAU: f64 = 6.28318530717958647693;
pub const E: f64 = 2.71828182845904523536;
pub const LN2: f64 = 0.693147180559945309417;
pub const LN10: f64 = 2.30258509299404568402;
pub const SQRT2: f64 = 1.41421356237309504880;

/// Infinity and a quiet NaN.
pub fn inf() f64 {
    return @bit_cast(f64, @as(u64, 0x7ff0000000000000));
}

pub fn nan() f64 {
    return @bit_cast(f64, @as(u64, 0x7ff8000000000000));
}

pub fn is_nan(x: f64) bool {
    return x != x;
}

pub fn is_inf(x: f64) bool {
    return (x == inf()) or (x == -inf());
}

pub fn is_finite(x: f64) bool {
    return (x - x) == 0.0;
}

fn bits(x: f64) u64 {
    return @bit_cast(u64, x);
}

fn from_bits(b: u64) f64 {
    return @bit_cast(f64, b);
}

fn high(x: f64) u32 {
    return @truncate(u32, bits(x) >> 32);
}

fn low(x: f64) u32 {
    return @truncate(u32, bits(x));
}

fn with_high(x: f64, h: u32) f64 {
    return from_bits((@as(u64, h) << 32) | (bits(x) & 0xffffffff));
}

fn with_low(x: f64, l: u32) f64 {
    return from_bits((bits(x) & 0xffffffff00000000) | @as(u64, l));
}

fn from_words(h: u32, l: u32) f64 {
    return from_bits((@as(u64, h) << 32) | @as(u64, l));
}

/// x * 2^n.
pub fn ldexp(x: f64, n: i32) f64 {
    let mut y = x;
    let mut k = n;
    let p1023 = from_bits(0x7fe0000000000000);
    let m969 = from_bits(0x0360000000000000);  // 2^-1022 * 2^53
    if k > 1023 {
        y = y * p1023;
        k -= 1023;
        if k > 1023 {
            y = y * p1023;
            k = @min(k - 1023, 1023);
        }
    } else if k < -1022 {
        y = y * m969;
        k += 1022 - 53;
        if k < -1022 {
            y = y * m969;
            k = @max(k + (1022 - 53), -1022);
        }
    }
    return y * from_bits(@as(u64, @truncate(u32, 0x3ff + k)) << 52);
}

/// x as m * 2^e with 0.5 <= |m| < 1 (or x itself for 0, inf and NaN).
pub fn frexp(x: f64) Frexp {
    let e = @as(i32, @truncate(u16, (bits(x) >> 52) & 0x7ff));
    if e == 0 {
        if x == 0.0 {
            return Frexp.{ mantissa: x, exponent: 0 };
        }
        let r = frexp(x * from_bits(0x43f0000000000000));  // 2^64
        return Frexp.{ mantissa: r.mantissa, exponent: r.exponent - 64 };
    }
    if e == 0x7ff {
        return Frexp.{ mantissa: x, exponent: 0 };
    }
    let m = from_bits((bits(x) & 0x800fffffffffffff) | 0x3fe0000000000000);
    return Frexp.{ mantissa: m, exponent: e - 0x3fe };
}

pub struct Frexp {
    mantissa: f64,
    exponent: i32,
}

// -----------------------------------------------------------------------------
// Exponentials and logarithms
// -----------------------------------------------------------------------------

const LN2_HI: f64 = 6.93147180369123816490e-01;
const LN2_LO: f64 = 1.90821492927058770002e-10;
const INV_LN2: f64 = 1.44269504088896338700e+00;
const EP1: f64 = 1.66666666666666019037e-01;
const EP2: f64 = -2.77777777770155933842e-03;
const EP3: f64 = 6.61375632143793436117e-05;
const EP4: f64 = -1.65339022054652515390e-06;
const EP5: f64 = 4.13813679705723846039e-08;

/// e^x.
pub fn exp(x: f64) f64 {
    if x != x {
        return x;
    }
    if x > 709.782712893383973096 {
        return x * from_bits(0x7fe0000000000000);  // overflows to inf
    }
    if x < -745.13321910194110842 {
        return 0.0;
    }
    let hx = high(x) & 0x7fffffff;
    let mut hi: f64 = 0.0;
    let mut lo: f64 = 0.0;
    let mut k: i32 = 0;
    let mut r = x;
    if hx > 0x3fd62e42 {
        // |x| > ln2 / 2
        if hx >= 0x3ff0a2b2 {
            // |x| >= 1.5 ln2
            let mut half: f64 = 0.5;
            if x < 0.0 {
                half = -0.5;
            }
            k = @int_from_float(i32, (INV_LN2 * x) + half);
        } else if x < 0.0 {
            k = -1;
        } else {
            k = 1;
        }
        let kf = @float_from_int(f64, k);
        hi = x - (kf * LN2_HI);
        lo = kf * LN2_LO;
        r = hi - lo;
    } else if hx > 0x3e300000 {
        // |x| > 2^-28
        hi = x;
    } else {
        return 1.0 + x;
    }
    let xx = r * r;
    let c = r - (xx * (EP1 + (xx * (EP2 + (xx * (EP3 + (xx * (EP4 + (xx * EP5)))))))));
    let y = 1.0 + ((((r * c) / (2.0 - c)) - lo) + hi);
    if k == 0 {
        return y;
    }
    return ldexp(y, k);
}

/// 2^x.
pub fn exp2(x: f64) f64 {
    return pow(2.0, x);
}

const Q1: f64 = -3.33333333333331316428e-02;
const Q2: f64 = 1.58730158725481460165e-03;
const Q3: f64 = -7.93650757867487942473e-05;
const Q4: f64 = 4.00821782732936239552e-06;
const Q5: f64 = -2.01099218183624371326e-07;

/// e^x - 1 for |x| <= ln2 / 2 (fdlibm's s_expm1.c without reduction).
fn expm1_small(x: f64) f64 {
    let hfx = 0.5 * x;
    let hxs = x * hfx;
    let r1 = 1.0 + (hxs * (Q1 + (hxs * (Q2 + (hxs * (Q3 + (hxs * (Q4 + (hxs * Q5)))))))));
    let t = 3.0 - (r1 * hfx);
    let e = hxs * ((r1 - t) / (6.0 - (x * t)));
    return x - ((x * e) - hxs);
}

/// e^x - 1, accurate for small x.
pub fn expm1(x: f64) f64 {
    if x != x {
        return x;
    }
    if x > 709.782712893383973096 {
        return x * from_bits(0x7fe0000000000000);
    }
    if x < -38.0 {
        return -1.0;
    }
    let hx = high(x) & 0x7fffffff;
    if hx < 0x3c900000 {
        return x;  // |x| < 2^-54
    }
    if hx <= 0x3fd62e42 {
        return expm1_small(x);  // |x| <= ln2 / 2
    }
    // x = k ln2 + r with r = hi - lo; c is what the rounding of r lost
    let k = @int_from_float(i32, @nearest(x * INV_LN2));
    let kf = @float_from_int(f64, k);
    let hi = x - (kf * LN2_HI);
    let lo = kf * LN2_LO;
    let r = hi - lo;
    let c = (hi - r) - lo;
    let mut em = expm1_small(r);
    em = em + (c * (1.0 + em));  // e^(r + c) - 1
    // e^x - 1 = 2^k (1 + em) - 1
    if k > 56 {
        return ldexp(1.0 + em, k);
    }
    let two_k = ldexp(1.0, k);
    if k < -1 {
        return ((two_k * em) - 0.5) + (two_k - 0.5);
    }
    return (two_k * em) + (two_k - 1.0);
}

const LG1: f64 = 6.666666666666735130e-01;
const LG2: f64 = 3.999999999940941908e-01;
const LG3: f64 = 2.857142874366239149e-01;
const LG4: f64 = 2.222219843214978396e-01;
const LG5: f64 = 1.818357216161805012e-01;
const LG6: f64 = 1.531383769920937332e-01;
const LG7: f64 = 1.479819860511658591e-01;

/// The natural logarithm.
pub fn log(x0: f64) f64 {
    let mut x = x0;
    let mut u = bits(x);
    let mut hx = @truncate(u32, u >> 32);
    let mut k: i32 = 0;
    if (hx < 0x00100000) or ((hx >> 31) != 0) {
        if (u << 1) == 0 {
            return -1.0 / (x * x);  // log(+-0) = -inf
        }
        if (hx >> 31) != 0 {
            return (x - x) / 0.0;  // log(-x) = NaN
        }
        // subnormal: scale up
        k -= 54;
        x = x * from_bits(0x4350000000000000);  // 2^54
        u = bits(x);
        hx = @truncate(u32, u >> 32);
    } else if hx >= 0x7ff00000 {
        return x;
    } else if (hx == 0x3ff00000) and ((u << 32) == 0) {
        return 0.0;
    }
    // reduce x into [sqrt(2)/2, sqrt(2)]
    hx += 0x3ff00000 - 0x3fe6a09e;
    k += @as(i32, @truncate(u16, hx >> 20)) - 0x3ff;
    hx = (hx & 0x000fffff) + 0x3fe6a09e;
    x = from_bits((@as(u64, hx) << 32) | (u & 0xffffffff));
    let f = x - 1.0;
    let hfsq = (0.5 * f) * f;
    let s = f / (2.0 + f);
    let z = s * s;
    let w = z * z;
    let t1 = w * (LG2 + (w * (LG4 + (w * LG6))));
    let t2 = z * (LG1 + (w * (LG3 + (w * (LG5 + (w * LG7))))));
    let r = t2 + t1;
    let dk = @float_from_int(f64, k);
    return ((((s * (hfsq + r)) + (dk * LN2_LO)) - hfsq) + f) + (dk * LN2_HI);
}

/// log(1 + x), accurate for small x.
pub fn log1p(x: f64) f64 {
    let u = 1.0 + x;
    if u == 1.0 {
        return x;
    }
    if not is_finite(u) or (u <= 0.0) {
        return log(u);
    }
    // log(u) plus the part of x that 1 + x rounded away
    return log(u) + ((x - (u - 1.0)) / u);
}

pub fn log2(x: f64) f64 {
    let r = frexp(x);
    if (r.mantissa == 0.5) and is_finite(x) {
        return @float_from_int(f64, r.exponent - 1);  // exact for powers of two
    }
    return log(x) * INV_LN2;
}

pub fn log10(x: f64) f64 {
    return log(x) * 0.434294481903251827651;
}

// pow, after fdlibm's e_pow.c
let BP: [2]f64 = [1.0, 1.5];
let DP_H: [2]f64 = [0.0, 5.84962487220764160156e-01];
let DP_L: [2]f64 = [0.0, 1.35003920212974897128e-08];
const TWO53: f64 = 9007199254740992.0;
const HUGE: f64 = 1.0e300;
const TINY: f64 = 1.0e-300;
const L1: f64 = 5.99999999999994648725e-01;
const L2: f64 = 4.28571428578550184252e-01;
const L3: f64 = 3.33333329818377432918e-01;
const L4: f64 = 2.72728123808534006489e-01;
const L5: f64 = 2.30660745775561754067e-01;
const L6: f64 = 2.06975017800338417784e-01;
const LG2_F: f64 = 6.93147180559945286227e-01;
const LG2_H: f64 = 6.93147182464599609375e-01;
const LG2_L: f64 = -1.90465429995776804525e-09;
const OVT: f64 = 8.0085662595372944372e-17;
const CP: f64 = 9.61796693925975554329e-01;
const CP_H: f64 = 9.61796700954437255859e-01;
const CP_L: f64 = -7.02846165095275826516e-09;
const IVLN2_H: f64 = 1.44269502162933349609e+00;
const IVLN2_L: f64 = 1.92596299112661746887e-08;

/// x^y.
pub fn pow(x: f64, y: f64) f64 {
    let hx = @bit_cast(i32, high(x));
    let lx = low(x);
    let hy = @bit_cast(i32, high(y));
    let ly = low(y);
    let mut ix = @bit_cast(i32, high(x) & 0x7fffffff);
    let iy = @bit_cast(i32, high(y) & 0x7fffffff);
    if (iy == 0) and (ly == 0) {
        return 1.0;  // x^0 = 1, even for NaN x
    }
    if (hx == 0x3ff00000) and (lx == 0) {
        return 1.0;  // 1^y = 1, even for NaN y
    }
    if (ix > 0x7ff00000) or ((ix == 0x7ff00000) and (lx != 0)) or (iy > 0x7ff00000) or
       ((iy == 0x7ff00000) and (ly != 0)) {
        return x + y;  // NaN
    }
    // when x < 0: is y an odd integer (1), an even one (2), or neither (0)?
    let mut yisint: i32 = 0;
    if hx < 0 {
        if iy >= 0x43400000 {
            yisint = 2;
        } else if iy >= 0x3ff00000 {
            let k = (iy >> 20) - 0x3ff;
            if k > 20 {
                let sh = @as(u32, @truncate(u8, 52 - k));
                let j = ly >> sh;
                if (j << sh) == ly {
                    yisint = 2 - @bit_cast(i32, j & 1);
                }
            } else if ly == 0 {
                let sh = @as(u32, @truncate(u8, 20 - k));
                let j = @bit_cast(u32, iy) >> sh;
                if (j << sh) == @bit_cast(u32, iy) {
                    yisint = 2 - @bit_cast(i32, j & 1);
                }
            }
        }
    }
    if ly == 0 {
        if iy == 0x7ff00000 {
            // y is +-inf
            if ((ix - 0x3ff00000) | @bit_cast(i32, lx)) == 0 {
                return 1.0;  // (-1)^+-inf
            }
            if ix >= 0x3ff00000 {
                if hy >= 0 {
                    return y;
                }
                return 0.0;
            }
            if hy < 0 {
                return -y;
            }
            return 0.0;
        }
        if iy == 0x3ff00000 {
            if hy < 0 {
                return 1.0 / x;
            }
            return x;
        }
        if hy == 0x40000000 {
            return x * x;
        }
        if (hy == 0x3fe00000) and (hx >= 0) {
            return @sqrt(x);
        }
    }
    let mut ax = @abs(x);
    if (lx == 0) and ((ix == 0x7ff00000) or (ix == 0) or (ix == 0x3ff00000)) {
        // x is +-0, +-inf or +-1
        let mut z = ax;
        if hy < 0 {
            z = 1.0 / z;
        }
        if hx < 0 {
            if ((ix - 0x3ff00000) | yisint) == 0 {
                z = (z - z) / (z - z);  // (-1)^non-integer
            } else if yisint == 1 {
                z = -z;
            }
        }
        return z;
    }
    let mut s: f64 = 1.0;
    if hx < 0 {
        if yisint == 0 {
            return (x - x) / (x - x);  // negative^non-integer
        }
        if yisint == 1 {
            s = -1.0;
        }
    }
    let mut t1: f64 = 0.0;
    let mut t2: f64 = 0.0;
    if iy > 0x41e00000 {
        // |y| > 2^31
        if iy > 0x43f00000 {
            // |y| > 2^64: overflow or underflow
            if ix <= 0x3fefffff {
                if hy < 0 {
                    return HUGE * HUGE;
                }
                return TINY * TINY;
            }
            if hy > 0 {
                return HUGE * HUGE;
            }
            return TINY * TINY;
        }
        if ix < 0x3fefffff {
            if hy < 0 {
                return (s * HUGE) * HUGE;
            }
            return (s * TINY) * TINY;
        }
        if ix > 0x3ff00000 {
            if hy > 0 {
                return (s * HUGE) * HUGE;
            }
            return (s * TINY) * TINY;
        }
        // |1 - x| <= 2^-20: log(x) by x - x^2/2 + x^3/3 - x^4/4
        let t = ax - 1.0;
        let w = (t * t) * (0.5 - (t * (0.3333333333333333333333 - (t * 0.25))));
        let u = IVLN2_H * t;
        let v = (t * IVLN2_L) - (w * INV_LN2);
        t1 = with_low(u + v, 0);
        t2 = v - (t1 - u);
    } else {
        let mut n: i32 = 0;
        if ix < 0x00100000 {
            // subnormal x
            ax = ax * TWO53;
            n -= 53;
            ix = @bit_cast(i32, high(ax));
        }
        n += (ix >> 20) - 0x3ff;
        let j = ix & 0x000fffff;
        ix = j | 0x3ff00000;
        let mut k: usize = 0;
        if j <= 0x3988e {
            k = 0;  // |x| < sqrt(3/2)
        } else if j < 0xbb67a {
            k = 1;  // |x| < sqrt(3)
        } else {
            n += 1;
            ix -= 0x00100000;
        }
        ax = with_high(ax, @bit_cast(u32, ix));
        // ss = s_h + s_l = (x - 1) / (x + 1) or (x - 1.5) / (x + 1.5)
        let u = ax - BP[k];
        let v = 1.0 / (ax + BP[k]);
        let ss = u * v;
        let s_h = with_low(ss, 0);
        let t_h0 = from_words(
            @bit_cast(u32, (((ix >> 1) | 0x20000000) + 0x00080000) + (@as(i32, @truncate(u8, k)) << 18)), 0);
        let t_l0 = ax - (t_h0 - BP[k]);
        let s_l = v * ((u - (s_h * t_h0)) - (s_h * t_l0));
        // log(ax)
        let mut s2 = ss * ss;
        let mut r = (s2 * s2) * (L1 + (s2 * (L2 + (s2 * (L3 + (s2 * (L4 + (s2 * (L5 + (s2 * L6))))))))));
        r += s_l * (s_h + ss);
        s2 = s_h * s_h;
        let t_h = with_low((3.0 + s2) + r, 0);
        let t_l = r - ((t_h - 3.0) - s2);
        let uu = s_h * t_h;
        let vv = (s_l * t_h) + (t_l * ss);
        let p_h = with_low(uu + vv, 0);
        let p_l = vv - (p_h - uu);
        let z_h = CP_H * p_h;
        let z_l = ((CP_L * p_h) + (p_l * CP)) + DP_L[k];
        // log2(ax) = n + dp_h + z_h + z_l
        let t = @float_from_int(f64, n);
        t1 = with_low(((z_h + z_l) + DP_H[k]) + t, 0);
        t2 = z_l - (((t1 - t) - DP_H[k]) - z_h);
    }
    // (y1 + y2) * (t1 + t2)
    let y1 = with_low(y, 0);
    let p_l = ((y - y1) * t1) + (y * t2);
    let mut p_h = y1 * t1;
    let z = p_l + p_h;
    let j = @bit_cast(i32, high(z));
    let i = @bit_cast(i32, low(z));
    if j >= 0x40900000 {
        // z >= 1024
        if ((j - 0x40900000) | i) != 0 {
            return (s * HUGE) * HUGE;
        }
        if (p_l + OVT) > (z - p_h) {
            return (s * HUGE) * HUGE;
        }
    } else if (j & 0x7fffffff) >= 0x4090cc00 {
        // z <= -1075
        if ((j - @bit_cast(i32, @as(u32, 0xc090cc00))) | i) != 0 {
            return (s * TINY) * TINY;
        }
        if p_l <= (z - p_h) {
            return (s * TINY) * TINY;
        }
    }
    // 2^(p_h + p_l)
    let ii = j & 0x7fffffff;
    let mut k = (ii >> 20) - 0x3ff;
    let mut n: i32 = 0;
    if ii > 0x3fe00000 {
        // |z| > 0.5: n = [z + 0.5]
        n = j + (0x00100000 >> @as(u32, @truncate(u8, k + 1)));
        k = ((n & 0x7fffffff) >> 20) - 0x3ff;
        let t = from_words(@bit_cast(u32, n & ~(0x000fffff >> @as(u32, @truncate(u8, k)))), 0);
        n = ((n & 0x000fffff) | 0x00100000) >> @as(u32, @truncate(u8, 20 - k));
        if j < 0 {
            n = -n;
        }
        p_h -= t;
    }
    let t = with_low(p_l + p_h, 0);
    let u = t * LG2_H;
    let v = ((p_l - (t - p_h)) * LG2_F) + (t * LG2_L);
    let mut zz = u + v;
    let w = v - (zz - u);
    let tt = zz * zz;
    let tt1 = zz - (tt * (EP1 + (tt * (EP2 + (tt * (EP3 + (tt * (EP4 + (tt * EP5)))))))));
    let r = ((zz * tt1) / (tt1 - 2.0)) - (w + (zz * w));
    zz = 1.0 - (r - zz);
    let jj = @bit_cast(i32, high(zz)) + (n << 20);
    if (jj >> 20) <= 0 {
        zz = ldexp(zz, n);  // subnormal result
    } else {
        zz = with_high(zz, @bit_cast(u32, jj));
    }
    return s * zz;
}

// cbrt, after fdlibm's s_cbrt.c
const B1: u32 = 715094163;

/// The cube root.
pub fn cbrt(x: f64) f64 {
    let mut hx = high(x) & 0x7fffffff;
    if hx >= 0x7ff00000 {
        return x + x;  // NaN, inf
    }
    if hx >= 0x7e000000 {
        // |x| >= 2^993: scale down so that t^3 + 2x cannot overflow
        return cbrt(x * from_bits(0x0180000000000000)) * from_bits(0x54c0000000000000);  // 2^-999, 2^333
    }
    if hx < 0x00100000 {
        // zero or subnormal: scale up (t^3 must not be subnormal)
        if x == 0.0 {
            return x;
        }
        return cbrt(x * from_bits(0x4350000000000000)) * from_bits(0x3ed0000000000000);  // 2^54, 2^-18
    }
    let mut u = bits(x);
    hx = (hx / 3) + B1;
    u = (u & 0x8000000000000000) | (@as(u64, hx) << 32);
    let mut t = from_bits(u);
    // two Halley steps take the 5 good bits to more than 23
    for i in @as(usize, 0)..2 {
        let t3 = (t * t) * t;
        t = t * ((t3 + (x + x)) / ((t3 + t3) + x));
    }
    // round to 23 bits, then one Newton step to 53
    t = from_bits((bits(t) + 0x80000000) & 0xffffffffc0000000);
    let s = t * t;
    let mut r = x / s;
    let w = t + t;
    r = (r - t) / (w + r);
    return t + (t * r);
}

/// sqrt(x^2 + y^2) without undue overflow or underflow.
pub fn hypot(x: f64, y: f64) f64 {
    let mut a = @abs(x);
    let mut b = @abs(y);
    if is_inf(a) or is_inf(b) {
        return inf();
    }
    if (a != a) or (b != b) {
        return a + b;
    }
    if a < b {
        let t = a;
        a = b;
        b = t;
    }
    if b == 0.0 {
        return a;
    }
    // scale into a range where the squares neither overflow nor underflow
    let mut scale: f64 = 1.0;
    if a > from_bits(0x5f30000000000000) {
        // 2^500
        a = a * from_bits(0x1a70000000000000);  // 2^-600
        b = b * from_bits(0x1a70000000000000);
        scale = from_bits(0x6570000000000000);  // 2^600
    } else if a < from_bits(0x20b0000000000000) {
        // 2^-500
        a = a * from_bits(0x6570000000000000);
        b = b * from_bits(0x6570000000000000);
        scale = from_bits(0x1a70000000000000);
    }
    // a^2 + b^2 exactly enough, as a double-double
    let aa = a * a;
    let aa_lo = @fma(a, a, -aa);
    let bb = b * b;
    let bb_lo = @fma(b, b, -bb);
    let sh = aa + bb;
    let sl = ((aa - sh) + bb) + (aa_lo + bb_lo);
    let h = @sqrt(sh);
    // one Newton step on the residual
    let resid = @fma(-h, h, sh) + sl;
    return (h + (resid / (2.0 * h))) * scale;
}

// -----------------------------------------------------------------------------
// Trigonometry
// -----------------------------------------------------------------------------

const S1: f64 = -1.66666666666666324348e-01;
const S2: f64 = 8.33333333332248946124e-03;
const S3: f64 = -1.98412698298579493134e-04;
const S4: f64 = 2.75573137070700676789e-06;
const S5: f64 = -2.50507602534068634195e-08;
const S6: f64 = 1.58969099521155010221e-10;

/// sin(x + y) for |x| <= pi/4, y a tail below x's last bit.
fn k_sin(x: f64, y: f64) f64 {
    let z = x * x;
    let w = z * z;
    let r = (S2 + (z * (S3 + (z * S4)))) + ((z * w) * (S5 + (z * S6)));
    let v = z * x;
    return x - (((z * ((0.5 * y) - (v * r))) - y) - (v * S1));
}

const C1: f64 = 4.16666666666666019037e-02;
const C2: f64 = -1.38888888888741095749e-03;
const C3: f64 = 2.48015872894767294178e-05;
const C4: f64 = -2.75573143513906633035e-07;
const C5: f64 = 2.08757232129817482790e-09;
const C6: f64 = -1.13596475577881948265e-11;

/// cos(x + y) for |x| <= pi/4.
fn k_cos(x: f64, y: f64) f64 {
    let z = x * x;
    let w = z * z;
    let r = (z * (C1 + (z * (C2 + (z * C3))))) + ((w * w) * (C4 + (z * (C5 + (z * C6)))));
    let hz = 0.5 * z;
    let ww = 1.0 - hz;
    return ww + (((1.0 - ww) - hz) + ((z * r) - (x * y)));
}

const INVPIO2: f64 = 6.36619772367581382433e-01;
const PIO2_1: f64 = 1.57079632673412561417e+00;
const PIO2_1T: f64 = 6.07710050650619224932e-11;
const PIO2_2: f64 = 6.07710050630396597660e-11;
const PIO2_2T: f64 = 2.02226624879595063154e-21;
const PIO2_3: f64 = 2.02226624871116645580e-21;
const PIO2_3T: f64 = 8.47842766036889956997e-32;
const PIO4: f64 = 7.85398163397448278999e-01;
const PIO4_LO: f64 = 3.06161699786838301793e-17;

// 4/pi in binary: the integer bit, then 1216 bits of fraction
let M4PI: [20]u64 = [
    0x0000000000000001,
    0x45f306dc9c882a53,
    0xf84eafa3ea69bb81,
    0xb6c52b3278872083,
    0xfca2c757bd778ac3,
    0x6e48dc74849ba5c0,
    0x0c925dd413a32439,
    0xfc3bd63962534e7d,
    0xd1046bea5d768909,
    0xd338e04d68befc82,
    0x7323ac7306a673e9,
    0x3908bf177bf25076,
    0x3ff12fffbc0b301f,
    0xde5e2316b414da3e,
    0xda6cfd9e4f96136e,
    0x9e8c7ecd3cbfd45a,
    0xea4f758fd7cbe2f6,
    0x7a0e73ef14a525d4,
    0xd7f6bf623f1aba10,
    0xac06608df8f6d757,
];

/// x reduced by multiples of pi/2: x = n * pi/2 + (y0 + y1), |y0 + y1| <= pi/4.
struct Reduced {
    n: u32,  // modulo 4
    y0: f64,
    y1: f64,
}

/// The high and low 64 bits of a * b.
fn mul64(a: u64, b: u64) [2]u64 {
    let a0 = a & 0xffffffff;
    let a1 = a >> 32;
    let b0 = b & 0xffffffff;
    let b1 = b >> 32;
    let p00 = a0 * b0;
    let p01 = a0 * b1;
    let p10 = a1 * b0;
    let p11 = a1 * b1;
    let mid = ((p00 >> 32) + (p01 & 0xffffffff)) + (p10 & 0xffffffff);
    let hi = ((p11 + (p01 >> 32)) + (p10 >> 32)) + (mid >> 32);
    let lo = (mid << 32) | (p00 & 0xffffffff);
    return [hi, lo];
}

fn rem_pio2(x: f64) Reduced {
    let ix = high(x) & 0x7fffffff;
    if ix < 0x413921fb {
        // |x| < 2^20 pi/2: Cody-Waite with a three-part pi/2
        let fnv = @nearest(x * INVPIO2);
        let n = @truncate(u32, @bit_cast(u64, @int_from_float(i64, fnv)));
        let mut r = x - (fnv * PIO2_1);
        let mut w = fnv * PIO2_1T;
        let mut y0 = r - w;
        let ex = @as(i32, @truncate(u16, ix >> 20));
        let mut ey = @as(i32, @truncate(u16, (bits(y0) >> 52) & 0x7ff));
        if (ex - ey) > 16 {
            // cancellation: use more of pi/2
            let t = r;
            w = fnv * PIO2_2;
            r = t - w;
            w = (fnv * PIO2_2T) - ((t - r) - w);
            y0 = r - w;
            ey = @as(i32, @truncate(u16, (bits(y0) >> 52) & 0x7ff));
            if (ex - ey) > 49 {
                let t2 = r;
                w = fnv * PIO2_3;
                r = t2 - w;
                w = (fnv * PIO2_3T) - ((t2 - r) - w);
                y0 = r - w;
            }
        }
        let y1 = (r - y0) - w;
        return Reduced.{ n: n & 3, y0: y0, y1: y1 };
    }
    if ix >= 0x7ff00000 {
        return Reduced.{ n: 0, y0: x - x, y1: 0.0 };  // inf or NaN: NaN
    }
    // Payne-Hanek: multiply the mantissa by the bits of 4/pi that matter
    let neg = x < 0.0;
    let ax = @abs(x);
    let b = bits(ax);
    let e2 = @as(i32, @truncate(u16, b >> 52)) - (1023 + 52);
    let m = (b & 0x000fffffffffffff) | 0x0010000000000000;
    let at = @as(u32, @truncate(u16, e2 + 61));
    let digit = @as(usize, at / 64);
    let sh = at % 64;
    let z0 = (M4PI[digit] << sh) | ((M4PI[digit + 1] >> (64 - sh)));
    let z1 = (M4PI[digit + 1] << sh) | ((M4PI[digit + 2] >> (64 - sh)));
    let z2 = (M4PI[digit + 2] << sh) | ((M4PI[digit + 3] >> (64 - sh)));
    let p2 = mul64(z2, m);
    let p1 = mul64(z1, m);
    let z0lo = z0 *% m;
    let lo = p1[1] +% p2[0];
    let mut carry: u64 = 0;
    if lo < p1[1] {
        carry = 1;
    }
    let mut hi = (z0lo +% p1[0]) +% carry;
    // the top 3 bits are the octant; the next 125 the fraction F
    let mut j = @truncate(u32, hi >> 61);
    let mut fh = (hi << 3) | (lo >> 61);
    let mut fl = lo << 3;
    let mut fneg = false;
    if (j & 1) == 1 {
        // round up to the even octant: the fraction becomes F - 1, which is
        // negated here exactly (in floating point it would lose bits)
        j = (j + 1) & 7;
        fl = (~fl) +% 1;
        fh = ~fh;
        if fl == 0 {
            fh = fh +% 1;
        }
        fneg = true;
    }
    // F as a double-double zh + zl
    let lz = @as(u64, @clz(fh));
    let nh = (fh << lz) | ((fl >> (64 - lz)) & low_mask(lz));
    let nl = fl << lz;
    let zh = ldexp(@float_from_int(f64, nh >> 11), -53 - @as(i32, @truncate(u8, lz)));
    let zl = ldexp(@float_from_int(f64, ((nh & 0x7ff) << 42) | (nl >> 22)), -106 - @as(i32, @truncate(u8, lz)));
    // times pi/4, also as a double-double
    let p = zh * PIO4;
    let err = @fma(zh, PIO4, -p) + ((zh * PIO4_LO) + (zl * PIO4));
    let mut y0 = p + err;
    let mut y1 = err - (y0 - p);
    if fneg != neg {
        y0 = -y0;
        y1 = -y1;
    }
    let mut n = j / 2;
    if neg {
        n = (4 - n) & 3;
    }
    return Reduced.{ n: n, y0: y0, y1: y1 };
}

/// The low k bits set (k < 64), for taking the bits shifted in from below.
fn low_mask(k: u64) u64 {
    return (@as(u64, 1) << k) - 1;
}

pub fn sin(x: f64) f64 {
    let ix = high(x) & 0x7fffffff;
    if ix <= 0x3fe921fb {
        // |x| <= pi/4
        if ix < 0x3e500000 {
            return x;  // |x| < 2^-26
        }
        return k_sin(x, 0.0);
    }
    let r = rem_pio2(x);
    if r.n == 0 {
        return k_sin(r.y0, r.y1);
    }
    if r.n == 1 {
        return k_cos(r.y0, r.y1);
    }
    if r.n == 2 {
        return -k_sin(r.y0, r.y1);
    }
    return -k_cos(r.y0, r.y1);
}

pub fn cos(x: f64) f64 {
    let ix = high(x) & 0x7fffffff;
    if ix <= 0x3fe921fb {
        if ix < 0x3e46a09e {
            return 1.0;  // |x| < 2^-27 sqrt(2)
        }
        return k_cos(x, 0.0);
    }
    let r = rem_pio2(x);
    if r.n == 0 {
        return k_cos(r.y0, r.y1);
    }
    if r.n == 1 {
        return -k_sin(r.y0, r.y1);
    }
    if r.n == 2 {
        return -k_cos(r.y0, r.y1);
    }
    return k_sin(r.y0, r.y1);
}

/// Both sin(x) and cos(x).
pub fn sincos(x: f64) SinCos {
    let ix = high(x) & 0x7fffffff;
    if ix <= 0x3fe921fb {
        return SinCos.{ sin: sin(x), cos: cos(x) };
    }
    let r = rem_pio2(x);
    let s = k_sin(r.y0, r.y1);
    let c = k_cos(r.y0, r.y1);
    if r.n == 0 {
        return SinCos.{ sin: s, cos: c };
    }
    if r.n == 1 {
        return SinCos.{ sin: c, cos: -s };
    }
    if r.n == 2 {
        return SinCos.{ sin: -s, cos: -c };
    }
    return SinCos.{ sin: -c, cos: s };
}

pub struct SinCos {
    sin: f64,
    cos: f64,
}

pub fn tan(x: f64) f64 {
    let ix = high(x) & 0x7fffffff;
    if ix < 0x3e400000 {
        return x;
    }
    let r = rem_pio2(x);
    let s = k_sin(r.y0, r.y1);
    let c = k_cos(r.y0, r.y1);
    if (r.n & 1) == 0 {
        return s / c;
    }
    return -c / s;
}

// atan, after fdlibm's s_atan.c
let ATANHI: [4]f64 = [4.63647609000806093515e-01, 7.85398163397448278999e-01, 9.82793723247329054082e-01,
                      1.57079632679489655800e+00];
let ATANLO: [4]f64 = [2.26987774529616870924e-17, 3.06161699786838301793e-17, 1.39033110312309984516e-17,
                      6.12323399573676603587e-17];
let AT: [11]f64 = [
    3.33333333333329318027e-01,
    -1.99999999998764832476e-01,
    1.42857142725034663711e-01,
    -1.11111104054623557880e-01,
    9.09088713343650656196e-02,
    -7.69187620504482999495e-02,
    6.66107313738753120669e-02,
    -5.83357013379057348645e-02,
    4.97687799461593236017e-02,
    -3.65315727442169155270e-02,
    1.62858201153657823623e-02,
];

pub fn atan(x0: f64) f64 {
    let mut x = x0;
    let ix = high(x) & 0x7fffffff;
    let neg = (high(x) >> 31) != 0;
    if ix >= 0x44100000 {
        // |x| >= 2^66
        if x != x {
            return x;
        }
        let z = ATANHI[3] + ATANLO[3];
        if neg {
            return -z;
        }
        return z;
    }
    let mut id: i32 = -1;
    if ix < 0x3fdc0000 {
        // |x| < 0.4375
        if ix < 0x3e400000 {
            return x;  // |x| < 2^-27
        }
    } else {
        x = @abs(x);
        if ix < 0x3ff30000 {
            // |x| < 1.1875
            if ix < 0x3fe60000 {
                // 7/16 <= |x| < 11/16
                id = 0;
                x = ((2.0 * x) - 1.0) / (2.0 + x);
            } else {
                id = 1;
                x = (x - 1.0) / (x + 1.0);
            }
        } else if ix < 0x40038000 {
            // |x| < 2.4375
            id = 2;
            x = (x - 1.5) / (1.0 + (1.5 * x));
        } else {
            id = 3;
            x = -1.0 / x;
        }
    }
    let z = x * x;
    let w = z * z;
    let s1 = z * (AT[0] + (w * (AT[2] + (w * (AT[4] + (w * (AT[6] + (w * (AT[8] + (w * AT[10]))))))))));
    let s2 = w * (AT[1] + (w * (AT[3] + (w * (AT[5] + (w * (AT[7] + (w * AT[9]))))))));
    if id < 0 {
        return x - (x * (s1 + s2));
    }
    let k = @as(usize, @truncate(u8, @bit_cast(u32, id)));
    let r = ATANHI[k] - (((x * (s1 + s2)) - ATANLO[k]) - x);
    if neg {
        return -r;
    }
    return r;
}

const PI_LO: f64 = 1.2246467991473531772e-16;

/// The angle of the point (x, y) from the positive x axis, in (-pi, pi].
pub fn atan2(y: f64, x: f64) f64 {
    if (x != x) or (y != y) {
        return x + y;
    }
    if x == 1.0 {
        return atan(y);
    }
    let ix = high(x) & 0x7fffffff;
    let iy = high(y) & 0x7fffffff;
    let m = ((high(y) >> 31) & 1) | ((high(x) >> 30) & 2);  // 2 * sign(x) + sign(y)
    if y == 0.0 {
        if m <= 1 {
            return y;
        }
        if m == 2 {
            return PI;
        }
        return -PI;
    }
    if x == 0.0 {
        if (m & 1) != 0 {
            return -PI / 2.0;
        }
        return PI / 2.0;
    }
    if ix == 0x7ff00000 {
        if iy == 0x7ff00000 {
            if m == 0 {
                return PI / 4.0;
            }
            if m == 1 {
                return -PI / 4.0;
            }
            if m == 2 {
                return (3.0 * PI) / 4.0;
            }
            return (-3.0 * PI) / 4.0;
        }
        if m == 0 {
            return 0.0;
        }
        if m == 1 {
            return -0.0;
        }
        if m == 2 {
            return PI;
        }
        return -PI;
    }
    if ((ix + (64 << 20)) < iy) or (iy == 0x7ff00000) {
        // |y/x| > 2^64
        if (m & 1) != 0 {
            return -PI / 2.0;
        }
        return PI / 2.0;
    }
    let mut z: f64 = 0.0;
    if ((m & 2) == 0) or ((iy + (64 << 20)) >= ix) {
        z = atan(@abs(y / x));
    }
    if m == 0 {
        return z;
    }
    if m == 1 {
        return -z;
    }
    if m == 2 {
        return PI - (z - PI_LO);
    }
    return (z - PI_LO) - PI;
}

const PIO2_HI: f64 = 1.57079632679489655800e+00;
const PIO2_LO: f64 = 6.12323399573676603587e-17;
const PS0: f64 = 1.66666666666666657415e-01;
const PS1: f64 = -3.25565818622400915405e-01;
const PS2: f64 = 2.01212532134862925881e-01;
const PS3: f64 = -4.00555345006794114027e-02;
const PS4: f64 = 7.91534994289814532176e-04;
const PS5: f64 = 3.47933107596021167570e-05;
const QS1: f64 = -2.40339491173441421878e+00;
const QS2: f64 = 2.02094576023350569471e+00;
const QS3: f64 = -6.88283971605453293030e-01;
const QS4: f64 = 7.70381505559019352791e-02;

/// (asin(sqrt(z)) - sqrt(z)) / sqrt(z)^3, a rational approximation.
fn asin_r(z: f64) f64 {
    let p = z * (PS0 + (z * (PS1 + (z * (PS2 + (z * (PS3 + (z * (PS4 + (z * PS5))))))))));
    let q = 1.0 + (z * (QS1 + (z * (QS2 + (z * (QS3 + (z * QS4)))))));
    return p / q;
}

pub fn asin(x: f64) f64 {
    let ax = @abs(x);
    if ax >= 1.0 {
        if ax == 1.0 {
            return @copysign(PIO2_HI + PIO2_LO, x);
        }
        return (x - x) / (x - x);
    }
    if ax < 0.5 {
        if ax < 1.0e-8 {
            return x;
        }
        return x + (x * asin_r(x * x));
    }
    let z = (1.0 - ax) * 0.5;
    let sq = @sqrt(z);
    let r = asin_r(z);
    let mut t: f64 = 0.0;
    if ax >= 0.975 {
        t = PIO2_HI - ((2.0 * (sq + (sq * r))) - PIO2_LO);
    } else {
        let w = with_low(sq, 0);
        let c = (z - (w * w)) / (sq + w);
        let p = ((2.0 * sq) * r) - (PIO2_LO - (2.0 * c));
        let q = PIO4 - (2.0 * w);
        t = PIO4 - (p - q);
    }
    return @copysign(t, x);
}

pub fn acos(x: f64) f64 {
    let ax = @abs(x);
    if ax >= 1.0 {
        if x == 1.0 {
            return 0.0;
        }
        if x == -1.0 {
            return 2.0 * PIO2_HI;
        }
        return (x - x) / (x - x);
    }
    if ax < 0.5 {
        if ax < 1.0e-17 {
            return PIO2_HI + PIO2_LO;
        }
        return PIO2_HI - (x - (PIO2_LO - (x * asin_r(x * x))));
    }
    if x < 0.0 {
        let z = (1.0 + x) * 0.5;
        let sq = @sqrt(z);
        let w = (asin_r(z) * sq) - PIO2_LO;
        return 2.0 * (PIO2_HI - (sq + w));
    }
    let z = (1.0 - x) * 0.5;
    let sq = @sqrt(z);
    let df = with_low(sq, 0);
    let c = (z - (df * df)) / (sq + df);
    let w = (asin_r(z) * sq) + c;
    return 2.0 * (df + w);
}

// -----------------------------------------------------------------------------
// Hyperbolic functions
// -----------------------------------------------------------------------------

pub fn sinh(x: f64) f64 {
    let a = @abs(x);
    let mut r: f64 = 0.0;
    if a < 1.0 {
        let t = expm1(a);
        r = 0.5 * (t + (t / (t + 1.0)));
    } else if a < 709.0 {
        let t = exp(a);
        r = 0.5 * (t - (1.0 / t));
    } else {
        let t = exp(0.5 * a);
        r = (0.5 * t) * t;
    }
    return @copysign(r, x);
}

pub fn cosh(x: f64) f64 {
    let a = @abs(x);
    if a < 709.0 {
        let t = exp(a);
        return 0.5 * (t + (1.0 / t));
    }
    let t = exp(0.5 * a);
    return (0.5 * t) * t;
}

pub fn tanh(x: f64) f64 {
    let a = @abs(x);
    let mut r: f64 = 1.0;
    if a < 22.0 {
        // t / (t + 2) with t = e^2a - 1, the division corrected with an fma
        let t = expm1(2.0 * a);
        let d = t + 2.0;
        let d_lo = (2.0 - d) + t;
        let q = t / d;
        r = q + ((@fma(-q, d, t) - (q * d_lo)) / d);
    }
    if x != x {
        return x;
    }
    return @copysign(r, x);
}

// -----------------------------------------------------------------------------
// f32 versions
// -----------------------------------------------------------------------------

pub fn expf(x: f32) f32 {
    return @float_cast(f32, exp(@as(f64, x)));
}

pub fn logf(x: f32) f32 {
    return @float_cast(f32, log(@as(f64, x)));
}

pub fn log2f(x: f32) f32 {
    return @float_cast(f32, log2(@as(f64, x)));
}

pub fn powf(x: f32, y: f32) f32 {
    return @float_cast(f32, pow(@as(f64, x), @as(f64, y)));
}

pub fn cbrtf(x: f32) f32 {
    return @float_cast(f32, cbrt(@as(f64, x)));
}

pub fn sinf(x: f32) f32 {
    return @float_cast(f32, sin(@as(f64, x)));
}

pub fn cosf(x: f32) f32 {
    return @float_cast(f32, cos(@as(f64, x)));
}

pub fn tanf(x: f32) f32 {
    return @float_cast(f32, tan(@as(f64, x)));
}

pub fn atanf(x: f32) f32 {
    return @float_cast(f32, atan(@as(f64, x)));
}

pub fn atan2f(y: f32, x: f32) f32 {
    return @float_cast(f32, atan2(@as(f64, y), @as(f64, x)));
}

pub fn hypotf(x: f32, y: f32) f32 {
    return @float_cast(f32, hypot(@as(f64, x), @as(f64, y)));
}

pub fn tanhf(x: f32) f32 {
    return @float_cast(f32, tanh(@as(f64, x)));
}

// -----------------------------------------------------------------------------
// Tests
// -----------------------------------------------------------------------------

/// Whether a and b are within n ulps.
fn close(a: f64, b: f64, n: u64) bool {
    if a == b {
        return true;
    }
    let ia = @bit_cast(i64, bits(a));
    let ib = @bit_cast(i64, bits(b));
    if (ia < 0) != (ib < 0) {
        return false;
    }
    let d = @abs(ia - ib);
    return @bit_cast(u64, d) <= n;
}

test "values" {
    @expect(close(exp(1.0), E, 1));
    @expect(close(log(E), 1.0, 1));
    @expect_eq(log(1.0), 0.0);
    @expect_eq(exp(0.0), 1.0);
    @expect_eq(pow(2.0, 10.0), 1024.0);
    @expect_eq(pow(-2.0, 3.0), -8.0);
    @expect(is_nan(pow(-2.0, 0.5)));
    @expect(close(pow(10.0, 0.5), 3.1622776601683795, 1));
    @expect_eq(cbrt(27.0), 3.0);
    @expect_eq(cbrt(-8.0), -2.0);
    @expect(close(sin(PI / 6.0), 0.5, 1));
    @expect(close(cos(PI / 3.0), 0.5, 1));
    @expect(close(atan2(1.0, 1.0), PI / 4.0, 1));
    @expect(close(atan2(-1.0, -1.0), (-3.0 * PI) / 4.0, 1));
    @expect_eq(atan2(0.0, -1.0), PI);
    @expect(close(sin(1.0e22), -0.8522008497671888, 1));
    @expect(close(cos(1.0e300), -0.5753861119575491, 1));
    @expect_eq(log2(1024.0), 10.0);
    @expect(close(log10(1000.0), 3.0, 1));
    @expect(close(expm1(1.0e-10), 1.00000000005e-10, 1));
    @expect(close(log1p(1.0e-10), 9.999999999500001e-11, 1));
    @expect(close(tanh(0.5), 0.46211715726000974, 2));
    @expect(close(hypot(3.0, 4.0), 5.0, 0));
    @expect(is_inf(exp(1000.0)));
    @expect_eq(exp(-1000.0), 0.0);
    @expect(is_inf(-log(0.0)));
    @expect(is_nan(log(-1.0)));
    let f = frexp(48.0);
    @expect_eq(f.mantissa, 0.75);
    @expect_eq(f.exponent, 6);
    @expect_eq(ldexp(0.75, 6), 48.0);
    @expect_eq(ldexp(1.0, -1074), from_bits(1));
    @expect_eq(sinf(0.0), 0.0);
    @expect_eq(powf(2.0, 3.0), 8.0);
}
