// =============================================================================
// std/image.ph - images in memory.
//
//     let mut img = image.Image.init(640, 480, 4, image.U8)!;  // RGBA, 8 bits
//     defer img.deinit();
//     img.set(10, 20, color.Color.rgb(1.0, 0.5, 0.0));
//     let c = img.get(10, 20);                                   // a color.Color
//     let hdr = img.convert(3, image.F32)!;                      // RGB, linear float
//
// An Image is a grid of pixels of 1 (gray), 2 (gray and alpha), 3 (RGB) or
// 4 (RGBA) channels, each channel U8 or U16 (sRGB-encoded integers, as most
// files hold) or F32 (linear light, unbounded: HDR). Rows are packed, top row
// first; U16 and F32 samples are in the machine's order (little-endian) and
// are reached with samples16() and samples32(). Alpha is straight (not
// premultiplied).
//
// Reading and writing files: std/image/formats (any format, by content or
// file extension), or the codecs std/image/png, jpeg, bmp, qoi, pnm (PPM,
// PGM, PFM), hdr (Radiance), exr (OpenEXR), gif, tiff and webp.
// =============================================================================

import "color.ph" as color;
import "mem.ph" as mem;

error { InvalidImage, Unsupported, TooLarge, OutOfMemory }

/// Sample types.
pub const U8: u8 = 1;
pub const U16: u8 = 2;
pub const F32: u8 = 3;

/// Images bigger than this many pixels are refused by the decoders (a guard
/// against hostile headers); 2^28 is 16384 x 16384.
pub const MAX_PIXELS: usize = 268435456;

pub fn bytes_per_sample(depth: u8) usize {
    if depth == U8 {
        return 1;
    }
    if depth == U16 {
        return 2;
    }
    return 4;
}

pub struct Image {
    width: usize,
    height: usize,
    channels: usize,  // 1 gray, 2 gray + alpha, 3 RGB, 4 RGBA
    depth: u8,        // U8, U16 or F32
    data: []mut u8,

    /// A new image, all zero (transparent black). Free it with deinit.
    pub fn init(width: usize, height: usize, channels: usize, depth: u8) !Image {
        if (channels < 1) or (channels > 4) or ((depth != U8) and (depth != U16) and (depth != F32)) {
            return error.Unsupported;
        }
        let px = @mul_checked(width, height) catch return error.TooLarge;
        if px > MAX_PIXELS {
            return error.TooLarge;
        }
        let n = (px * channels) * bytes_per_sample(depth);
        return Image.{
            width: width,
            height: height,
            channels: channels,
            depth: depth,
            data: mem.alloc_zeroed.[u8](@max(n, 1))!,
        };
    }

    pub fn deinit(self: *mut Image) void {
        mem.free(self.data);
    }

    pub fn clone(self: *Image) !Image {
        let mut c = Image.init(self.width, self.height, self.channels, self.depth)!;
        @mem_copy(c.data, self.data);
        return c;
    }

    pub fn has_alpha(self: *Image) bool {
        return (self.channels == 2) or (self.channels == 4);
    }

    pub fn is_gray(self: *Image) bool {
        return self.channels <= 2;
    }

    /// Bytes per row.
    pub fn stride(self: *Image) usize {
        return (self.width * self.channels) * bytes_per_sample(self.depth);
    }

    /// The samples of a U8 image.
    pub fn samples8(self: *Image) []mut u8 {
        return self.data[..self.width * self.height * self.channels];
    }

    /// The samples of a U16 image.
    pub fn samples16(self: *Image) []mut u16 {
        return @cast_slice(u16, self.data[..(self.width * self.height * self.channels) * 2]);
    }

    /// The samples of an F32 image.
    pub fn samples32(self: *Image) []mut f32 {
        return @cast_slice(f32, self.data[..(self.width * self.height * self.channels) * 4]);
    }

    /// Sample c of pixel (x, y) as linear light for colour channels, or as
    /// a fraction for alpha.
    fn sample(self: *Image, i: usize, is_alpha: bool) f32 {
        if self.depth == U8 {
            let v = self.data[i];
            if is_alpha {
                return @float_from_int(f32, v) / 255.0;
            }
            return color.linear8(v);
        }
        if self.depth == U16 {
            let v = @float_from_int(f32, self.samples16()[i]) / 65535.0;
            if is_alpha {
                return v;
            }
            return color.srgb_decode(v);
        }
        return self.samples32()[i];
    }

    fn put(self: *mut Image, i: usize, v: f32, is_alpha: bool) void {
        if self.depth == U8 {
            if is_alpha {
                self.data[i] = @truncate(u8, @int_from_float(u32, @nearest(clamp01(v) * 255.0)));
            } else {
                self.data[i] = color.srgb8(v);
            }
        } else if self.depth == U16 {
            let mut e = v;
            if not is_alpha {
                e = color.srgb_encode(v);
            }
            self.samples16()[i] = @truncate(u16, @int_from_float(u32, @nearest(clamp01(e) * 65535.0)));
        } else {
            self.samples32()[i] = v;
        }
    }

    /// The pixel at (x, y), which must be inside the image.
    pub fn get(self: *Image, x: usize, y: usize) color.Color {
        let i = ((y * self.width) + x) * self.channels;
        if self.channels <= 2 {
            let g = self.sample(i, false);
            let mut a: f32 = 1.0;
            if self.channels == 2 {
                a = self.sample(i + 1, true);
            }
            return color.Color.{ r: g, g: g, b: g, a: a };
        }
        let mut a: f32 = 1.0;
        if self.channels == 4 {
            a = self.sample(i + 3, true);
        }
        return color.Color.{ r: self.sample(i, false), g: self.sample(i + 1, false), b: self.sample(i + 2, false),
                             a: a };
    }

    /// Sets the pixel at (x, y). Gray images take the luminance; images
    /// without alpha ignore it.
    pub fn set(self: *mut Image, x: usize, y: usize, c: color.Color) void {
        let i = ((y * self.width) + x) * self.channels;
        if self.channels <= 2 {
            let mut g = c.r;
            if (c.r != c.g) or (c.g != c.b) {
                g = c.luminance();
            }
            self.put(i, g, false);
            if self.channels == 2 {
                self.put(i + 1, c.a, true);
            }
            return;
        }
        self.put(i, c.r, false);
        self.put(i + 1, c.g, false);
        self.put(i + 2, c.b, false);
        if self.channels == 4 {
            self.put(i + 3, c.a, true);
        }
    }

    /// Fills the image with one colour.
    pub fn fill(self: *mut Image, c: color.Color) void {
        for y in @as(usize, 0)..self.height {
            for x in @as(usize, 0)..self.width {
                self.set(x, y, c);
            }
        }
    }

    /// A copy with other channels and sample type. Integer samples convert
    /// exactly between U8 and U16 (v * 257); to and from F32 through sRGB.
    pub fn convert(self: *Image, channels: usize, depth: u8) !Image {
        if (channels == self.channels) and (depth == self.depth) {
            return self.clone();
        }
        let mut out = Image.init(self.width, self.height, channels, depth)!;
        let n = self.width * self.height;
        let fast = (self.depth != F32) and (depth != F32);
        for p in @as(usize, 0)..n {
            if fast {
                // integers: move encoded values without going through linear
                let mut v: [4]u32 = [0, 0, 0, 0xffff];
                let si = p * self.channels;
                for c in @as(usize, 0)..self.channels {
                    let mut s: u32 = 0;
                    if self.depth == U8 {
                        s = @as(u32, self.data[si + c]) * 257;
                    } else {
                        s = @as(u32, self.samples16()[si + c]);
                    }
                    v[c] = s;
                }
                let mut rgba: [4]u32 = [v[0], v[0], v[0], 0xffff];
                if self.channels == 2 {
                    rgba[3] = v[1];
                } else if self.channels >= 3 {
                    rgba = [v[0], v[1], v[2], v[3]];
                }
                if channels <= 2 {
                    if self.channels >= 3 {
                        let lum = (color.Color.from_srgb(unit(rgba[0]), unit(rgba[1]), unit(rgba[2]), 1.0)).luminance();
                        rgba[0] = @int_from_float(u32, @nearest(clamp01(color.srgb_encode(lum)) * 65535.0));
                    }
                    rgba[1] = rgba[3];
                }
                let di = p * channels;
                for c in @as(usize, 0)..channels {
                    let mut s = rgba[c];
                    if (channels == 2) and (c == 1) {
                        s = rgba[3];
                    }
                    if depth == U8 {
                        out.data[di + c] = @truncate(u8, (s + 128) / 257);
                    } else {
                        out.samples16()[di + c] = @truncate(u16, s);
                    }
                }
            } else {
                let x = p % self.width;
                let y = p / self.width;
                out.set(x, y, self.get(x, y));
            }
        }
        return out;
    }

    /// A copy of the rectangle at (x, y), clipped to the image.
    pub fn crop(self: *Image, x: usize, y: usize, w: usize, h: usize) !Image {
        let x0 = @min(x, self.width);
        let y0 = @min(y, self.height);
        let cw = @min(w, self.width - x0);
        let ch = @min(h, self.height - y0);
        let mut out = Image.init(cw, ch, self.channels, self.depth)!;
        let bpp = self.channels * bytes_per_sample(self.depth);
        for row in @as(usize, 0)..ch {
            let src = (((y0 + row) * self.width) + x0) * bpp;
            @mem_copy(out.data[(row * cw) * bpp..((row + 1) * cw) * bpp], self.data[src..src + (cw * bpp)]);
        }
        return out;
    }

    /// Mirrors the image top to bottom, in place.
    pub fn flip_vertical(self: *mut Image) void {
        let s = self.stride();
        let mut top: usize = 0;
        let mut bottom = self.height;
        while (top + 1) < bottom {
            bottom -= 1;
            for i in @as(usize, 0)..s {
                let t = self.data[(top * s) + i];
                self.data[(top * s) + i] = self.data[(bottom * s) + i];
                self.data[(bottom * s) + i] = t;
            }
            top += 1;
        }
    }

    /// A resized copy, filtered with a triangle (bilinear when enlarging,
    /// averaging all covered pixels when shrinking), in linear light.
    pub fn resize(self: *Image, width: usize, height: usize) !Image {
        let mut out = Image.init(width, height, self.channels, self.depth)!;
        if (self.width == 0) or (self.height == 0) or (width == 0) or (height == 0) {
            return out;
        }
        self.resize_into(&mut out) catch |e| {
            out.deinit();
            return e;
        };
        return out;
    }

    // The filter is separable: each source row the vertical filter needs
    // is filtered horizontally once (premultiplied linear RGBA and the
    // weight sum, five values a pixel), in a ring of the last few rows.
    fn resize_into(self: *Image, out: *mut Image) !void {
        let width = out.width;
        let mut xs = Taps.init(self.width, width)!;
        defer xs.deinit();
        let mut ys = Taps.init(self.height, out.height)!;
        defer ys.deinit();
        let k = ys.k;
        let rw = width * 5;
        let ring = mem.alloc.[f32](k * rw)!;
        defer mem.free(ring);
        let ring_row = mem.alloc.[usize](k)!;
        defer mem.free(ring_row);
        @mem_set(ring_row, NO_ROW);
        let line = mem.alloc.[f32](self.width * 4)!;
        defer mem.free(line);
        let acc = mem.alloc.[f32](rw)!;
        defer mem.free(acc);
        for y in @as(usize, 0)..out.height {
            @mem_set(acc, 0.0);
            let sy0 = ys.start[y];
            for t in @as(usize, 0)..k {
                let wy = ys.w[(y * k) + t];
                if not (wy > 0.0) {
                    continue;
                }
                let sy = sy0 + t;
                let slot = sy % k;
                let row = ring[slot * rw..(slot + 1) * rw];
                if ring_row[slot] != sy {
                    self.filter_row(sy, &xs, line, row);
                    ring_row[slot] = sy;
                }
                for i in @as(usize, 0)..rw {
                    acc[i] += wy * row[i];
                }
            }
            for x in @as(usize, 0)..width {
                let p = acc[x * 5..(x * 5) + 5];
                let wsum = p[4];
                if wsum > 0.0 {
                    let a = p[3] / wsum;
                    let mut c = color.Color.{ r: 0.0, g: 0.0, b: 0.0, a: a };
                    if p[3] > 0.0 {
                        c = color.Color.{ r: p[0] / p[3], g: p[1] / p[3], b: p[2] / p[3], a: a };
                    }
                    out.set(x, y, c);
                }
            }
        }
    }

    // Source row sy, filtered horizontally into row.
    fn filter_row(self: *Image, sy: usize, xs: *Taps, line: []mut f32, row: []mut f32) void {
        for x in @as(usize, 0)..self.width {
            let c = self.get(x, sy);
            // weight colour by alpha so transparent pixels don't bleed
            line[x * 4] = c.r * c.a;
            line[(x * 4) + 1] = c.g * c.a;
            line[(x * 4) + 2] = c.b * c.a;
            line[(x * 4) + 3] = c.a;
        }
        let k = xs.k;
        for x in @as(usize, 0)..xs.n {
            let mut v: [5]f32 = [0.0; 5];
            let s0 = xs.start[x];
            for t in @as(usize, 0)..k {
                let w = xs.w[(x * k) + t];
                if w > 0.0 {
                    let q = (s0 + t) * 4;
                    v[0] += w * line[q];
                    v[1] += w * line[q + 1];
                    v[2] += w * line[q + 2];
                    v[3] += w * line[q + 3];
                    v[4] += w;
                }
            }
            @mem_copy(row[x * 5..(x * 5) + 5], v[..]);
        }
    }
}

const NO_ROW: usize = 0xffffffff;

/// The weights of a triangle filter from n_in samples to n: output i takes
/// input start[i] + t with weight w[i * k + t] (0 past the filter's end).
struct Taps {
    n: usize,
    k: usize,
    start: []mut usize,
    w: []mut f32,

    fn init(n_in: usize, n: usize) !Taps {
        let s = @float_from_int(f32, n_in) / @float_from_int(f32, n);
        let r = @max(s, 1.0);
        let k = @int_from_float(usize, 2.0 * r) + 2;
        let start = mem.alloc.[usize](n)!;
        let w = mem.alloc.[f32](@mul_checked(n, k) catch {
            mem.free(start);
            return error.OutOfMemory;
        }) catch |e| {
            mem.free(start);
            return e;
        };
        @mem_set(w, 0.0);
        let last = @float_from_int(f32, n_in - 1);
        for i in @as(usize, 0)..n {
            let c = ((@float_from_int(f32, i) + 0.5) * s) - 0.5;
            let lo = @max(@floor(c - r) + 1.0, 0.0);
            let hi = @min(@floor(c + r), last);
            let first = @int_from_float(usize, lo);
            start[i] = first;
            let mut j = lo;
            while j <= hi {
                let t = @int_from_float(usize, j) - first;
                if t < k {
                    w[(i * k) + t] = @max(1.0 - (@abs(j - c) / r), 0.0);
                }
                j += 1.0;
            }
        }
        return Taps.{ n: n, k: k, start: start, w: w };
    }

    fn deinit(self: *mut Taps) void {
        mem.free(self.start);
        mem.free(self.w);
    }
}

fn unit(v: u32) f32 {
    return @float_from_int(f32, v) / 65535.0;
}

fn clamp01(v: f32) f32 {
    if not (v > 0.0) {
        return 0.0;
    }
    return @min(v, 1.0);
}

// -----------------------------------------------------------------------------
// Half-precision floats (as OpenEXR stores)
// -----------------------------------------------------------------------------

/// The value of an IEEE binary16.
pub fn half_to_f32(h: u16) f32 {
    let sign = @as(u32, h >> 15) << 31;
    let e = @as(u32, (h >> 10) & 0x1f);
    let m = @as(u32, h & 0x3ff);
    if e == 0 {
        // zero or subnormal: m * 2^-24
        let v = @float_from_int(f32, m) * 5.9604644775390625e-8;
        return @bit_cast(f32, @bit_cast(u32, v) | sign);
    }
    if e == 31 {
        return @bit_cast(f32, (sign | 0x7f800000) | (m << 13));  // inf, NaN
    }
    return @bit_cast(f32, (sign | ((e + 112) << 23)) | (m << 13));
}

/// The nearest binary16 (ties to even; overflow gives infinity).
pub fn f32_to_half(v: f32) u16 {
    let b = @bit_cast(u32, v);
    let sign = @truncate(u16, (b >> 16) & 0x8000);
    let e = @as(i32, @truncate(u8, b >> 23));
    let m = b & 0x7fffff;
    if e == 255 {
        if m != 0 {
            return (sign | 0x7e00);  // NaN
        }
        return sign | 0x7c00;
    }
    let he = e - 112;  // rebias from 127 to 15
    if he >= 31 {
        return sign | 0x7c00;
    }
    if he <= 0 {
        // subnormal or zero
        if he < -10 {
            return sign;
        }
        let mm = m | 0x800000;
        let shift = @as(u32, @truncate(u8, 14 - he));
        let mut r = mm >> shift;
        let rem = mm & ((@as(u32, 1) << shift) - 1);
        let half = @as(u32, 1) << (shift - 1);
        if (rem > half) or ((rem == half) and ((r & 1) != 0)) {
            r += 1;
        }
        return sign | @truncate(u16, r);
    }
    let mut r = (@as(u32, @truncate(u8, @bit_cast(u32, he))) << 10) | (m >> 13);
    let rem = m & 0x1fff;
    if (rem > 0x1000) or ((rem == 0x1000) and ((r & 1) != 0)) {
        r += 1;  // may carry into the exponent, up to infinity: correct
    }
    return sign | @truncate(u16, r);
}

// -----------------------------------------------------------------------------
// Tests
// -----------------------------------------------------------------------------

test "pixels and conversion" {
    let mut img = Image.init(3, 2, 4, U8)!;
    defer img.deinit();
    img.set(1, 1, color.Color.from_rgba8(color.Rgba8.from_u32(0x40806020)));
    @expect_eq(img.get(1, 1).to_rgba8().to_u32(), 0x40806020);
    @expect_eq(img.data[(1 * 3 + 1) * 4], 0x40);
    let mut g = img.convert(1, U16)!;
    defer g.deinit();
    @expect_eq(g.samples16().len, 6);
    let mut back = img.convert(4, U16)!;
    defer back.deinit();
    @expect_eq(back.samples16()[(1 * 3 + 1) * 4], 0x4040);
    let mut f = img.convert(3, F32)!;
    defer f.deinit();
    let c = f.get(1, 1);
    @expect(@abs(c.r - color.linear8(0x40)) < 1.0e-6);
    let mut again = f.convert(4, U8)!;
    defer again.deinit();
    @expect_eq(again.get(1, 1).to_rgba8().to_u32(), 0x408060ff);
    let mut cr = img.crop(1, 1, 5, 5)!;
    defer cr.deinit();
    @expect_eq(cr.width, 2);
    @expect_eq(cr.height, 1);
    @expect_eq(cr.get(0, 0).to_rgba8().to_u32(), 0x40806020);
    img.flip_vertical();
    @expect_eq(img.get(1, 0).to_rgba8().to_u32(), 0x40806020);
}

test "resize" {
    let mut img = Image.init(4, 4, 3, U8)!;
    defer img.deinit();
    img.fill(color.Color.rgb(1.0, 0.0, 0.0));
    let mut small = img.resize(2, 2)!;
    defer small.deinit();
    @expect_eq(small.get(1, 1).to_rgba8().to_u32(), 0xff0000ff);
    let mut big = img.resize(9, 7)!;
    defer big.deinit();
    @expect_eq(big.get(8, 6).to_rgba8().to_u32(), 0xff0000ff);
    // black and white columns average to linear 0.5, not encoded 0.5
    let mut bw = Image.init(2, 1, 1, U8)!;
    defer bw.deinit();
    bw.data[1] = 255;
    let mut one = bw.resize(1, 1)!;
    defer one.deinit();
    @expect_eq(one.data[0], 188);
}

test "half floats" {
    @expect_eq(f32_to_half(1.0), 0x3c00);
    @expect_eq(f32_to_half(-2.0), 0xc000);
    @expect_eq(f32_to_half(65504.0), 0x7bff);
    @expect_eq(f32_to_half(65520.0), 0x7c00);  // rounds up to infinity
    @expect_eq(f32_to_half(5.9604644775390625e-08), 0x0001);
    @expect_eq(f32_to_half(2.98e-08), 0x0000);
    @expect_eq(f32_to_half(0.1), 0x2e66);
    @expect_eq(half_to_f32(0x3555), 0.333251953125);
    @expect_eq(half_to_f32(0x0001), 5.9604644775390625e-08);
    // every half survives the round trip
    for i in @as(u32, 0)..65536 {
        let h = @truncate(u16, i);
        if ((h & 0x7c00) == 0x7c00) and ((h & 0x3ff) != 0) {
            continue;  // NaN
        }
        @expect_eq(f32_to_half(half_to_f32(h)), h);
    }
}
