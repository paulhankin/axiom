// =============================================================================
// std/image/av1/msac.ph - AV1's multi-symbol arithmetic decoder, as dav1d
// implements it (CDFs held as 32768 - p, with an adaptation counter after
// the last value).
// =============================================================================

pub struct Msac {
    data: []u8,
    pos: usize,
    dif: u64,
    rng: u32,
    cnt: i32,
    update: bool,  // adapt the CDFs

    pub fn init(data: []u8, disable_update: bool) Msac {
        let mut m = Msac.{ data: data, pos: 0, dif: 0, rng: 0x8000, cnt: -15, update: not disable_update };
        m.refill();
        return m;
    }

    fn refill(self: *mut Msac) void {
        let mut c = (64 - 24) - self.cnt;
        let mut dif = self.dif;
        loop {
            if self.pos >= self.data.len {
                // past the end: ones (the data is inverted)
                dif |= (@as(u64, 1) << @as(u64, @bit_cast(u32, c + 8))) - 1;
                break;
            }
            dif |= @as(u64, self.data[self.pos] ^ 0xff) << @as(u64, @bit_cast(u32, c));
            self.pos += 1;
            c -= 8;
            if c < 0 {
                break;
            }
        }
        self.dif = dif;
        self.cnt = (64 - 24) - c;
    }

    fn norm(self: *mut Msac, dif: u64, rng: u32) void {
        let d = @as(i32, @truncate(u8, @clz(rng))) - 16;
        let cnt = self.cnt;
        self.dif = dif << @as(u64, @bit_cast(u32, d));
        self.rng = rng << @bit_cast(u32, d);
        self.cnt = cnt - d;
        if @bit_cast(u32, cnt) < @bit_cast(u32, d) {
            self.refill();
        }
    }

    /// A bit of probability one half.
    pub fn bool_equi(self: *mut Msac) bool {
        let r = self.rng;
        let mut v = ((r >> 8) << 7) + 4;
        let vw = @as(u64, v) << 48;
        let ret = self.dif >= vw;
        let mut dif = self.dif;
        if ret {
            dif -= vw;
            v = r - v;
        }
        self.norm(dif, v);
        return not ret;
    }

    /// A bit whose zero has probability f / 32768 (f as dav1d holds it).
    pub fn bool_prob(self: *mut Msac, f: u32) bool {
        let r = self.rng;
        let mut v = (((r >> 8) * (f >> 6)) >> 1) + 4;
        let vw = @as(u64, v) << 48;
        let ret = self.dif >= vw;
        let mut dif = self.dif;
        if ret {
            dif -= vw;
            v = r - v;
        }
        self.norm(dif, v);
        return not ret;
    }

    pub fn bools(self: *mut Msac, n: u32) u32 {
        let mut v: u32 = 0;
        for _q in @as(u32, 0)..n {
            v = v << 1;
            if self.bool_equi() {
                v |= 1;
            }
        }
        return v;
    }

    /// A symbol of n + 1 values from an adaptive CDF.
    pub fn symbol(self: *mut Msac, cdf: []mut u16, n: u32) u32 {
        let c = @truncate(u32, self.dif >> 48);
        let r = self.rng >> 8;
        let mut u: u32 = 0;
        let mut v = self.rng;
        let mut val: u32 = 0;
        loop {
            u = v;
            v = (r * (@as(u32, cdf[@as(usize, val)]) >> 6)) >> 1;
            v += 4 * (n - val);
            if c >= v {
                break;
            }
            val += 1;
        }
        self.norm(self.dif -% (@as(u64, v) << 48), u - v);
        if self.update {
            let count = cdf[@as(usize, n)];
            let mut rate = 4 + (count >> 4);
            if n > 2 {
                rate += 1;
            }
            for i in @as(usize, 0)..@as(usize, val) {
                cdf[i] += (32768 - cdf[i]) >> rate;
            }
            for i in @as(usize, val)..@as(usize, n) {
                cdf[i] -= cdf[i] >> rate;
            }
            if count < 32 {
                cdf[@as(usize, n)] = count + 1;
            }
        }
        return val;
    }

    /// A bit from an adaptive CDF of two symbols.
    pub fn bool_adapt(self: *mut Msac, cdf: []mut u16) bool {
        let bit = self.bool_prob(@as(u32, cdf[0]));
        if self.update {
            let count = cdf[1];
            let rate = 4 + (count >> 4);
            if bit {
                cdf[0] += (32768 - cdf[0]) >> rate;
            } else {
                cdf[0] -= cdf[0] >> rate;
            }
            if count < 32 {
                cdf[1] = count + 1;
            }
        }
        return bit;
    }

    /// A coefficient's level above 2 (3 to 14, and 15 meaning more)
    pub fn hi_tok(self: *mut Msac, cdf: []mut u16) u32 {
        let mut br = self.symbol(cdf, 3);
        let mut tok = 3 + br;
        if br == 3 {
            br = self.symbol(cdf, 3);
            tok = 6 + br;
            if br == 3 {
                br = self.symbol(cdf, 3);
                tok = 9 + br;
                if br == 3 {
                    tok = 12 + self.symbol(cdf, 3);
                }
            }
        }
        return tok;
    }

    pub fn uniform(self: *mut Msac, n: u32) u32 {
        let mut l: u32 = 1;
        while (n >> l) != 0 {
            l += 1;
        }
        let m = (@as(u32, 1) << l) - n;
        let v = self.bools(l - 1);
        if v < m {
            return v;
        }
        let mut e: u32 = 0;
        if self.bool_equi() {
            e = 1;
        }
        return ((v << 1) - m) + e;
    }

    pub fn subexp(self: *mut Msac, rf: u32, n: u32, k0: u32) i32 {
        let mut k = k0;
        let mut a: u32 = 0;
        if self.bool_equi() {
            if self.bool_equi() {
                k += 1;
                if self.bool_equi() {
                    k += 1;
                }
            }
            a = @as(u32, 1) << k;
        }
        let v = self.bools(k) + a;
        if (rf * 2) <= n {
            return @bit_cast(i32, inv_recenter(rf, v));
        }
        return @bit_cast(i32, (n - 1) - inv_recenter((n - 1) - rf, v));
    }

    /// An Exp-Golomb value
    pub fn golomb(self: *mut Msac) u32 {
        let mut len: u32 = 0;
        let mut val: u32 = 1;
        while (not self.bool_equi()) and (len < 32) {
            len += 1;
        }
        for _q in @as(u32, 0)..len {
            val = val << 1;
            if self.bool_equi() {
                val |= 1;
            }
        }
        return val - 1;
    }
}

fn inv_recenter(r: u32, v: u32) u32 {
    if v > (r << 1) {
        return v;
    }
    if (v & 1) == 0 {
        return (v >> 1) +% r;
    }
    return r -% ((v + 1) >> 1);
}
