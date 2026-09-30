// =============================================================================
// std/image/av1/itx.ph - AV1 inverse transforms of every size and type, added
// to the prediction (as dav1d does it: rows then columns, with its rounding
// and intermediate clipping).
// =============================================================================

import "itx1d.ph" as d1;

// transform types (dav1d's numbering)
pub const DCT_DCT: u32 = 0;
pub const ADST_DCT: u32 = 1;
pub const DCT_ADST: u32 = 2;
pub const ADST_ADST: u32 = 3;
pub const FLIPADST_DCT: u32 = 4;
pub const DCT_FLIPADST: u32 = 5;
pub const FLIPADST_FLIPADST: u32 = 6;
pub const ADST_FLIPADST: u32 = 7;
pub const FLIPADST_ADST: u32 = 8;
pub const IDTX: u32 = 9;
pub const V_DCT: u32 = 10;
pub const H_DCT: u32 = 11;
pub const V_ADST: u32 = 12;
pub const H_ADST: u32 = 13;
pub const V_FLIPADST: u32 = 14;
pub const H_FLIPADST: u32 = 15;
pub const WHT_WHT: u32 = 16;

const IDENTITY: u32 = 0;
const DCT: u32 = 1;
const ADST: u32 = 2;
const FLIPADST: u32 = 3;

// [type] -> (vertical, horizontal) 1-D kinds
let KINDS: [32]u32 = [DCT, DCT, ADST, DCT, DCT, ADST, ADST, ADST, FLIPADST, DCT, DCT, FLIPADST, FLIPADST, FLIPADST,
                      ADST, FLIPADST, FLIPADST, ADST, IDENTITY, IDENTITY, DCT, IDENTITY, IDENTITY, DCT, ADST, IDENTITY,
                      IDENTITY, ADST, FLIPADST, IDENTITY, IDENTITY, FLIPADST];

fn run_1d(kind: u32, n: usize, c: []mut i32, stride: usize, min: i32, max: i32) void {
    if kind == IDENTITY {
        if n == 4 {
            d1.rav1d_inv_identity4_1d_c(c, stride, min, max);
        } else if n == 8 {
            d1.rav1d_inv_identity8_1d_c(c, stride, min, max);
        } else if n == 16 {
            d1.rav1d_inv_identity16_1d_c(c, stride, min, max);
        } else {
            d1.rav1d_inv_identity32_1d_c(c, stride, min, max);
        }
    } else if kind == DCT {
        if n == 4 {
            d1.rav1d_inv_dct4_1d_c(c, stride, min, max);
        } else if n == 8 {
            d1.rav1d_inv_dct8_1d_c(c, stride, min, max);
        } else if n == 16 {
            d1.rav1d_inv_dct16_1d_c(c, stride, min, max);
        } else if n == 32 {
            d1.rav1d_inv_dct32_1d_c(c, stride, min, max);
        } else {
            d1.rav1d_inv_dct64_1d_c(c, stride, min, max);
        }
    } else if kind == ADST {
        if n == 4 {
            d1.rav1d_inv_adst4_1d_c(c, stride, min, max);
        } else if n == 8 {
            d1.rav1d_inv_adst8_1d_c(c, stride, min, max);
        } else {
            d1.rav1d_inv_adst16_1d_c(c, stride, min, max);
        }
    } else {
        if n == 4 {
            d1.rav1d_inv_flipadst4_1d_c(c, stride, min, max);
        } else if n == 8 {
            d1.rav1d_inv_flipadst8_1d_c(c, stride, min, max);
        } else {
            d1.rav1d_inv_flipadst16_1d_c(c, stride, min, max);
        }
    }
}

fn shift_for(w: usize, h: usize) u32 {
    let a = w * h;
    if (a <= 32) {
        return 0;  // 4x4, 4x8, 8x4
    }
    if (w == 4) or (h == 4) {
        return 1;  // 4x16, 16x4
    }
    if (w == 8) and (h == 8) {
        return 1;
    }
    if ((w == 8) and (h == 16)) or ((w == 16) and (h == 8)) {
        return 1;
    }
    if ((w == 8) and (h == 32)) or ((w == 32) and (h == 8)) {
        return 2;
    }
    if (w == 16) and (h == 16) {
        return 2;
    }
    if ((w == 16) and (h == 32)) or ((w == 32) and (h == 16)) {
        return 1;
    }
    if ((w == 16) and (h == 64)) or ((w == 64) and (h == 16)) {
        return 2;
    }
    if (w == 32) and (h == 32) {
        return 2;
    }
    if ((w == 32) and (h == 64)) or ((w == 64) and (h == 32)) {
        return 1;
    }
    return 2;  // 64x64
}

fn clip_px(v: i32, max: i32) u16 {
    if v < 0 {
        return 0;
    }
    if v > max {
        return @truncate(u16, @bit_cast(u32, max));
    }
    return @truncate(u16, @bit_cast(u32, v));
}

fn iclip(v: i32, lo: i32, hi: i32) i32 {
    if v < lo {
        return lo;
    }
    if v > hi {
        return hi;
    }
    return v;
}

/// Inverse-transforms the coefficients (column-major, up to 32x32; zeroed
/// afterwards) of a w x h transform of a type and adds them to
/// pix[dst..] (stride).
pub fn add(pix: []mut u16, dst: usize, stride: usize, coef: []mut i32, eob: i32, w: usize, h: usize, typ: u32,
           bitdepth: u32, tmp: []mut i32) void {
    let maxv = @bit_cast(i32, (@as(u32, 1) << bitdepth) - 1);
    if typ == WHT_WHT {
        wht(pix, dst, stride, coef, maxv, tmp);
        return;
    }
    let shift = shift_for(w, h);
    let is_rect2 = ((w * 2) == h) or ((h * 2) == w);
    let rnd = (@as(i32, 1) << shift) >> 1;
    if (typ == DCT_DCT) and (eob < 1) {
        let mut dc = coef[0];
        coef[0] = 0;
        if is_rect2 {
            dc = ((dc * 181) + 128) >> 8;
        }
        dc = ((dc * 181) + 128) >> 8;
        dc = (dc + rnd) >> shift;
        dc = (((dc * 181) + 128) + 2048) >> 12;
        for y in @as(usize, 0)..h {
            for x in @as(usize, 0)..w {
                let i = dst + (y * stride) + x;
                pix[i] = clip_px(@as(i32, pix[i]) + dc, maxv);
            }
        }
        return;
    }
    let sh = @min(h, 32);
    let sw = @min(w, 32);
    let mut row_min: i32 = -32768;
    let mut col_min: i32 = -32768;
    if bitdepth != 8 {
        row_min = (~maxv) << 7;
        col_min = (~maxv) << 5;
    }
    let row_max = ~row_min;
    let col_max = ~col_min;
    let vk = KINDS[@as(usize, typ) * 2];
    let hk = KINDS[(@as(usize, typ) * 2) + 1];
    @mem_set(@cast_slice(u8, tmp[..w * h]), 0);
    for y in @as(usize, 0)..sh {
        let row = tmp[y * w..(y + 1) * w];
        for x in @as(usize, 0)..sw {
            let c = coef[y + (x * sh)];
            if is_rect2 {
                row[x] = ((c * 181) + 128) >> 8;
            } else {
                row[x] = c;
            }
        }
        run_1d(hk, w, row, 1, row_min, row_max);
    }
    @mem_set(@cast_slice(u8, coef[..sh * sw]), 0);
    for i in @as(usize, 0)..(w * sh) {
        tmp[i] = iclip((tmp[i] + rnd) >> shift, col_min, col_max);
    }
    for x in @as(usize, 0)..w {
        run_1d(vk, h, tmp[x..], w, col_min, col_max);
    }
    for y in @as(usize, 0)..h {
        for x in @as(usize, 0)..w {
            let i = dst + (y * stride) + x;
            pix[i] = clip_px(@as(i32, pix[i]) + ((tmp[(y * w) + x] + 8) >> 4), maxv);
        }
    }
}

fn wht(pix: []mut u16, dst: usize, stride: usize, coef: []mut i32, maxv: i32, tmp: []mut i32) void {
    for y in @as(usize, 0)..4 {
        for x in @as(usize, 0)..4 {
            tmp[(y * 4) + x] = coef[y + (x * 4)] >> 2;
        }
        d1.rav1d_inv_wht4_1d_c(tmp[y * 4..], 1);
    }
    @mem_set(@cast_slice(u8, coef[..16]), 0);
    for x in @as(usize, 0)..4 {
        d1.rav1d_inv_wht4_1d_c(tmp[x..], 4);
    }
    for y in @as(usize, 0)..4 {
        for x in @as(usize, 0)..4 {
            let i = dst + (y * stride) + x;
            pix[i] = clip_px(@as(i32, pix[i]) + tmp[(y * 4) + x], maxv);
        }
    }
}
