// =============================================================================
// std/font.ph - TrueType and OpenType fonts: outlines, metrics, kerning,
// text layout and rendering (through std/draw).
//
//     let mut f = font.Font.load("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf")!;
//     defer f.deinit();
//     font.draw_text(&mut canvas, &f, "Hello, world", 20.0, 60.0, 32.0, paint)!;  // baseline at y 60
//     let w = font.measure(&f, "Hello", 32.0);
//     let h = font.draw_text_box(&mut canvas, &f, long_text, 20.0, 100.0, 300.0, 16.0, paint)!;  // wrapped
//
// Fonts are TrueType (glyf outlines, simple and composite glyphs) or
// OpenType with CFF outlines (Type 2 charstrings, CID-keyed too), single
// files or collections (.ttc). Characters map to glyphs through cmap
// (formats 0, 4, 6, 12); kerning comes from GPOS pair adjustments or the
// kern table. Glyphs are drawn from their outlines, unhinted, with
// std/draw's exact anti-aliasing. There is no shaping beyond kerning (no
// ligatures, no complex scripts) and text runs left to right.
// =============================================================================

import "color.ph" as color;
import "draw.ph" as draw;
import "image.ph" as image;
import "io.ph" as io;
import "mem.ph" as mem;
import "str.ph" as strs;
import "vec.ph" as vec;

error { InvalidFont, Unsupported, OutOfMemory }

fn u8at(d: []u8, i: usize) u32 {
    if i >= d.len {
        return 0;
    }
    return @as(u32, d[i]);
}

fn u16at(d: []u8, i: usize) u32 {
    return (u8at(d, i) << 8) | u8at(d, i + 1);
}

fn i16at(d: []u8, i: usize) i32 {
    return @as(i32, @bit_cast(i16, @truncate(u16, u16at(d, i))));
}

fn u32at(d: []u8, i: usize) u32 {
    return (u16at(d, i) << 16) | u16at(d, i + 2);
}

fn tag_is(d: []u8, i: usize, t: []u8) bool {
    return ((i + 4) <= d.len) and @mem_eq(d[i..i + 4], t);
}

/// A table: where it is in the file, and how long (0, 0 if absent).
struct Table {
    off: usize,
    len: usize,
}

struct Cff {
    charstrings: usize,  // offset of the CharStrings INDEX (in the file)
    gsubrs: usize,       // the global Subrs INDEX
    subrs: usize,        // the local Subrs INDEX (0 if none), for non-CID fonts
    fdarray: usize,      // CID fonts: the Font DICT INDEX
    fdselect: usize,     // and the FDSelect
    base: usize,         // the CFF table's start
}

pub struct Font {
    data: []u8,
    owned: []mut u8,  // the file, if load read it
    units_per_em: u32,
    ascent: i32,   // font units, y up
    descent: i32,  // (negative)
    line_gap: i32,
    num_glyphs: u32,
    loc_long: bool,
    num_hmetrics: u32,
    cmap: usize,  // the chosen cmap subtable (0: none)
    glyf: Table,
    loca: Table,
    hmtx: Table,
    kern: Table,
    gpos: Table,
    cff: Cff,
    is_cff: bool,
    start: usize,  // the table directory (a collection holds several)

    /// A font from the bytes of a font file (the first font of a
    /// collection). The bytes must outlive the Font.
    pub fn init(data: []u8) !Font {
        return Font.init_index(data, 0);
    }

    /// The font at `index` in a collection (.ttc), or in a single font file
    /// (index 0).
    pub fn init_index(data: []u8, index: u32) !Font {
        let mut start: usize = 0;
        if tag_is(data, 0, "ttcf") {
            if index >= u32at(data, 8) {
                return error.InvalidFont;
            }
            start = @as(usize, u32at(data, 12 + (4 * @as(usize, index))));
        } else if index != 0 {
            return error.InvalidFont;
        }
        let ver = u32at(data, start);
        if (ver != 0x00010000) and not tag_is(data, start, "OTTO") and not tag_is(data, start, "true") {
            return error.InvalidFont;
        }
        let mut f = Font.{
            data: data,
            owned: @slice_from_ptr(@ptr_from_int(*mut u8, 1), 0),
            units_per_em: 0,
            ascent: 0,
            descent: 0,
            line_gap: 0,
            num_glyphs: 0,
            loc_long: false,
            num_hmetrics: 0,
            cmap: 0,
            glyf: Table.{ off: 0, len: 0 },
            loca: Table.{ off: 0, len: 0 },
            hmtx: Table.{ off: 0, len: 0 },
            kern: Table.{ off: 0, len: 0 },
            gpos: Table.{ off: 0, len: 0 },
            cff: Cff.{ charstrings: 0, gsubrs: 0, subrs: 0, fdarray: 0, fdselect: 0, base: 0 },
            is_cff: false,
            start: start,
        };
        let n = @as(usize, u16at(data, start + 4));
        let mut head = Table.{ off: 0, len: 0 };
        let mut hhea = Table.{ off: 0, len: 0 };
        let mut maxp = Table.{ off: 0, len: 0 };
        let mut cmap = Table.{ off: 0, len: 0 };
        let mut cff = Table.{ off: 0, len: 0 };
        for i in @as(usize, 0)..n {
            let r = start + 12 + (16 * i);
            let t = Table.{ off: @as(usize, u32at(data, r + 8)), len: @as(usize, u32at(data, r + 12)) };
            if (t.off > data.len) or (t.len > (data.len - t.off)) {
                return error.InvalidFont;
            }
            if tag_is(data, r, "head") {
                head = t;
            } else if tag_is(data, r, "hhea") {
                hhea = t;
            } else if tag_is(data, r, "maxp") {
                maxp = t;
            } else if tag_is(data, r, "cmap") {
                cmap = t;
            } else if tag_is(data, r, "glyf") {
                f.glyf = t;
            } else if tag_is(data, r, "loca") {
                f.loca = t;
            } else if tag_is(data, r, "hmtx") {
                f.hmtx = t;
            } else if tag_is(data, r, "kern") {
                f.kern = t;
            } else if tag_is(data, r, "GPOS") {
                f.gpos = t;
            } else if tag_is(data, r, "CFF ") {
                cff = t;
            }
        }
        if (head.len < 54) or (hhea.len < 36) or (maxp.len < 6) or (f.hmtx.len == 0) {
            return error.InvalidFont;
        }
        f.units_per_em = u16at(data, head.off + 18);
        f.loc_long = i16at(data, head.off + 50) != 0;
        f.ascent = i16at(data, hhea.off + 4);
        f.descent = i16at(data, hhea.off + 6);
        f.line_gap = i16at(data, hhea.off + 8);
        f.num_hmetrics = u16at(data, hhea.off + 34);
        f.num_glyphs = u16at(data, maxp.off + 4);
        if (f.units_per_em == 0) or (f.num_hmetrics == 0) {
            return error.InvalidFont;
        }
        if cff.len > 0 {
            f.is_cff = true;
            f.cff = parse_cff(data, cff.off)!;
        } else if (f.glyf.len == 0) or (f.loca.len == 0) {
            return error.Unsupported;  // bitmap-only or CFF2 fonts
        }
        f.cmap = choose_cmap(data, cmap);
        return f;
    }

    /// Reads a font file (the first font, if it is a collection).
    pub fn load(path: []u8) !Font {
        let file = io.open(path)!;
        defer file.close();
        let bytes = file.reader().read_all()!;
        let mut f = Font.init(bytes) catch |e| {
            mem.free(bytes);
            return e;
        };
        f.owned = bytes;
        return f;
    }

    /// The bytes of a table ("head", "glyf", "CFF ", ...), empty if the
    /// font has none: for code that reads what Font does not, or embeds
    /// the font (std/pdf).
    pub fn table(self: *Font, tag: []u8) []u8 {
        let n = @as(usize, u16at(self.data, self.start + 4));
        for i in @as(usize, 0)..n {
            let r = self.start + 12 + (16 * i);
            if (tag.len == 4) and tag_is(self.data, r, tag) {
                let off = @as(usize, u32at(self.data, r + 8));
                let len = @as(usize, u32at(self.data, r + 12));
                if (off <= self.data.len) and (len <= self.data.len - off) {
                    return self.data[off..off + len];
                }
            }
        }
        return self.data[..0];
    }

    /// The tags of the font's tables, in order ("head", "glyf", ...).
    pub fn table_count(self: *Font) usize {
        return @as(usize, u16at(self.data, self.start + 4));
    }

    pub fn table_tag(self: *Font, i: usize) []u8 {
        let r = self.start + 12 + (16 * i);
        if r + 4 > self.data.len {
            return self.data[..0];
        }
        return self.data[r..r + 4];
    }

    pub fn deinit(self: *mut Font) void {
        if self.owned.len > 0 {
            mem.free(self.owned);
        }
    }

    /// The glyph for a Unicode code point (0, the missing glyph, if none).
    pub fn glyph_index(self: *Font, cp: u32) u32 {
        let d = self.data;
        let t = self.cmap;
        if t == 0 {
            return 0;
        }
        let format = u16at(d, t);
        if format == 4 {
            let segx2 = @as(usize, u16at(d, t + 6));
            let ends = t + 14;
            let starts = ends + segx2 + 2;
            let deltas = starts + segx2;
            let ranges = deltas + segx2;
            // binary search for the first segment ending at or after cp
            let mut lo: usize = 0;
            let mut hi = segx2 / 2;
            while lo < hi {
                let mid = (lo + hi) / 2;
                if u16at(d, ends + (2 * mid)) < cp {
                    lo = mid + 1;
                } else {
                    hi = mid;
                }
            }
            if (lo >= (segx2 / 2)) or (cp > 0xffff) {
                return 0;
            }
            let s = u16at(d, starts + (2 * lo));
            if cp < s {
                return 0;
            }
            let delta = u16at(d, deltas + (2 * lo));
            let ro = u16at(d, ranges + (2 * lo));
            if ro == 0 {
                return (cp + delta) & 0xffff;
            }
            let g = u16at(d, ranges + (2 * lo) + @as(usize, ro) + (2 * @as(usize, cp - s)));
            if g == 0 {
                return 0;
            }
            return (g + delta) & 0xffff;
        }
        if format == 12 {
            let n = @as(usize, u32at(d, t + 12));
            let mut lo: usize = 0;
            let mut hi = n;
            while lo < hi {
                let mid = (lo + hi) / 2;
                let g = t + 16 + (12 * mid);
                if u32at(d, g + 4) < cp {
                    lo = mid + 1;
                } else {
                    hi = mid;
                }
            }
            if lo < n {
                let g = t + 16 + (12 * lo);
                let s = u32at(d, g);
                if cp >= s {
                    return u32at(d, g + 8) + (cp - s);
                }
            }
            return 0;
        }
        if format == 6 {
            let first = u16at(d, t + 6);
            let count = u16at(d, t + 8);
            if (cp >= first) and (cp < (first + count)) {
                return u16at(d, t + 10 + (2 * @as(usize, cp - first)));
            }
            return 0;
        }
        if (format == 0) and (cp < 256) {
            return u8at(d, t + 6 + @as(usize, cp));
        }
        return 0;
    }

    /// The glyph's advance width, in font units.
    pub fn advance(self: *Font, gid: u32) i32 {
        let i = @min(gid, self.num_hmetrics - 1);
        return @as(i32, @bit_cast(i32, u16at(self.data, self.hmtx.off + (4 * @as(usize, i)))));
    }

    /// The kerning between two glyphs, in font units (GPOS pair
    /// adjustments if the font has them, else the kern table).
    pub fn kerning(self: *Font, left: u32, right: u32) i32 {
        if self.gpos.len > 0 {
            return gpos_kern(self.data, self.gpos.off, left, right);
        }
        if self.kern.len > 0 {
            return kern_table(self.data, self.kern.off, left, right);
        }
        return 0;
    }

    /// Appends the glyph's outline to a path: its origin at (x, y) (the
    /// baseline, in the path's y-down space), one font unit = scale.
    pub fn glyph_path(self: *Font, gid: u32, path: *mut draw.Path, x: f64, y: f64, scale: f64) !void {
        if gid >= self.num_glyphs {
            return;
        }
        let mut pen = Pen.{ path: path, x: x, y: y, s: scale, open: false, work: 1 << 20 };
        if self.is_cff {
            return cff_glyph(self, gid, &mut pen);
        }
        return self.glyf_glyph(gid, &mut pen, 1.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0);
    }

    fn glyf_range(self: *Font, gid: u32) [2]usize {
        let d = self.data;
        let l = self.loca.off;
        let i = @as(usize, gid);
        let mut a: usize = 0;
        let mut b: usize = 0;
        if self.loc_long {
            a = @as(usize, u32at(d, l + (4 * i)));
            b = @as(usize, u32at(d, l + (4 * (i + 1))));
        } else {
            a = 2 * @as(usize, u16at(d, l + (2 * i)));
            b = 2 * @as(usize, u16at(d, l + (2 * (i + 1))));
        }
        if (b <= a) or (b > self.glyf.len) {
            return [0, 0];
        }
        return [self.glyf.off + a, self.glyf.off + b];
    }

    /// A TrueType glyph under the transform (xx, yx, xy, yy, dx, dy), in
    /// font units.
    fn glyf_glyph(self: *Font, gid: u32, pen: *mut Pen, xx: f64, yx: f64, xy: f64, yy: f64, dx: f64, dy: f64,
                  depth: u32) !void {
        let r = self.glyf_range(gid);
        if (r[1] - r[0]) < 10 {
            return;  // empty (a space)
        }
        let d = self.data[..r[1]];
        let g = r[0];
        let ncont = i16at(d, g);
        pen.spend(1)!;
        if ncont < 0 {
            if depth > 8 {
                return error.InvalidFont;
            }
            // components
            let mut p = g + 10;
            loop {
                let flags = u16at(d, p);
                let cg = u16at(d, p + 2);
                p += 4;
                let mut a: f64 = 0.0;
                let mut b: f64 = 0.0;
                if (flags & 1) != 0 {
                    a = @float_from_int(f64, i16at(d, p));
                    b = @float_from_int(f64, i16at(d, p + 2));
                    p += 4;
                } else {
                    a = @float_from_int(f64, @as(i32, @bit_cast(i8, @truncate(u8, u8at(d, p)))));
                    b = @float_from_int(f64, @as(i32, @bit_cast(i8, @truncate(u8, u8at(d, p + 1)))));
                    p += 2;
                }
                if (flags & 2) == 0 {
                    a = 0.0;  // matching points: not supported
                    b = 0.0;
                }
                let mut m: [4]f64 = [1.0, 0.0, 0.0, 1.0];
                if (flags & 8) != 0 {
                    m[0] = f2dot14(d, p);
                    m[3] = m[0];
                    p += 2;
                } else if (flags & 0x40) != 0 {
                    m[0] = f2dot14(d, p);
                    m[3] = f2dot14(d, p + 2);
                    p += 4;
                } else if (flags & 0x80) != 0 {
                    m[0] = f2dot14(d, p);
                    m[1] = f2dot14(d, p + 2);
                    m[2] = f2dot14(d, p + 4);
                    m[3] = f2dot14(d, p + 6);
                    p += 8;
                }
                if (flags & 0x800) != 0 {
                    // scaled offset
                    let na = (m[0] * a) + (m[2] * b);
                    let nb = (m[1] * a) + (m[3] * b);
                    a = na;
                    b = nb;
                }
                // compose: component transform, then ours
                let cxx = (xx * m[0]) + (xy * m[1]);
                let cyx = (yx * m[0]) + (yy * m[1]);
                let cxy = (xx * m[2]) + (xy * m[3]);
                let cyy = (yx * m[2]) + (yy * m[3]);
                let cdx = ((xx * a) + (xy * b)) + dx;
                let cdy = ((yx * a) + (yy * b)) + dy;
                self.glyf_glyph(cg, pen, cxx, cyx, cxy, cyy, cdx, cdy, depth + 1)!;
                if (flags & 0x20) == 0 {
                    break;
                }
            }
            return;
        }
        let n_cont = @as(usize, @bit_cast(u32, ncont));
        let ends = g + 10;
        if n_cont == 0 {
            return;
        }
        let npts = @as(usize, u16at(d, ends + (2 * (n_cont - 1)))) + 1;
        pen.spend(npts)!;
        let ilen = @as(usize, u16at(d, ends + (2 * n_cont)));
        let mut p = ends + (2 * n_cont) + 2 + ilen;
        // flags
        let flags = mem.alloc.[u8](npts)!;
        defer mem.free(flags);
        let mut k: usize = 0;
        while k < npts {
            let f = @truncate(u8, u8at(d, p));
            p += 1;
            flags[k] = f;
            k += 1;
            if (f & 8) != 0 {
                let rep = @as(usize, u8at(d, p));
                p += 1;
                for i in @as(usize, 0)..rep {
                    if k < npts {
                        flags[k] = f;
                        k += 1;
                    }
                }
            }
        }
        // coordinates
        let xs = mem.alloc.[f64](2 * npts)!;
        defer mem.free(xs);
        let mut v: i32 = 0;
        for i in @as(usize, 0)..npts {
            let f = flags[i];
            if (f & 2) != 0 {
                let dv = @bit_cast(i32, u8at(d, p));
                p += 1;
                if (f & 16) != 0 {
                    v += dv;
                } else {
                    v -= dv;
                }
            } else if (f & 16) == 0 {
                v += i16at(d, p);
                p += 2;
            }
            xs[2 * i] = @float_from_int(f64, v);
        }
        v = 0;
        for i in @as(usize, 0)..npts {
            let f = flags[i];
            if (f & 4) != 0 {
                let dv = @bit_cast(i32, u8at(d, p));
                p += 1;
                if (f & 32) != 0 {
                    v += dv;
                } else {
                    v -= dv;
                }
            } else if (f & 32) == 0 {
                v += i16at(d, p);
                p += 2;
            }
            xs[(2 * i) + 1] = @float_from_int(f64, v);
        }
        // transform
        for i in @as(usize, 0)..npts {
            let px = xs[2 * i];
            let py = xs[(2 * i) + 1];
            xs[2 * i] = ((xx * px) + (xy * py)) + dx;
            xs[(2 * i) + 1] = ((yx * px) + (yy * py)) + dy;
        }
        // contours: quadratic splines with implied on-curve midpoints
        let mut first: usize = 0;
        for c in @as(usize, 0)..n_cont {
            let last = @as(usize, u16at(d, ends + (2 * c)));
            if (last < first) or (last >= npts) {
                return error.InvalidFont;
            }
            quad_contour(pen, xs, flags, first, last)!;
            first = last + 1;
        }
    }
}

fn f2dot14(d: []u8, i: usize) f64 {
    return @float_from_int(f64, i16at(d, i)) / 16384.0;
}

/// Draws into a path, from font units (y up) to the path's space.
struct Pen {
    path: *mut draw.Path,
    x: f64,
    y: f64,
    s: f64,
    open: bool,
    work: usize,  // left of the glyph's budget (a malformed font can nest subroutines or components exponentially)

    fn spend(self: *mut Pen, n: usize) !void {
        if n > self.work {
            return error.InvalidFont;
        }
        self.work -= n;
    }

    fn move_to(self: *mut Pen, x: f64, y: f64) !void {
        if self.open {
            self.path.close()!;
        }
        self.path.move_to(self.x + (x * self.s), self.y - (y * self.s))!;
        self.open = true;
    }

    fn line_to(self: *mut Pen, x: f64, y: f64) !void {
        self.path.line_to(self.x + (x * self.s), self.y - (y * self.s))!;
    }

    fn quad_to(self: *mut Pen, cx: f64, cy: f64, x: f64, y: f64) !void {
        self.path.quad_to(self.x + (cx * self.s), self.y - (cy * self.s), self.x + (x * self.s),
                          self.y - (y * self.s))!;
    }

    fn cubic_to(self: *mut Pen, ax: f64, ay: f64, bx: f64, by: f64, x: f64, y: f64) !void {
        self.path.cubic_to(self.x + (ax * self.s), self.y - (ay * self.s), self.x + (bx * self.s),
                           self.y - (by * self.s), self.x + (x * self.s), self.y - (y * self.s))!;
    }

    fn close(self: *mut Pen) !void {
        if self.open {
            self.path.close()!;
            self.open = false;
        }
    }
}

fn quad_contour(pen: *mut Pen, xs: []f64, flags: []u8, first: usize, last: usize) !void {
    let n = (last - first) + 1;
    if n < 2 {
        return;
    }
    // start on an on-curve point (or between two off-curve ones)
    let mut s = first;
    while (s <= last) and ((flags[s] & 1) == 0) {
        s += 1;
    }
    let mut sx: f64 = 0.0;
    let mut sy: f64 = 0.0;
    let mut start_i: usize = 0;
    if s > last {
        sx = (xs[2 * first] + xs[2 * last]) / 2.0;
        sy = (xs[(2 * first) + 1] + xs[(2 * last) + 1]) / 2.0;
        start_i = 0;
    } else {
        sx = xs[2 * s];
        sy = xs[(2 * s) + 1];
        start_i = (s - first) + 1;
    }
    pen.move_to(sx, sy)!;
    let mut have_ctrl = false;
    let mut cx: f64 = 0.0;
    let mut cy: f64 = 0.0;
    for k in @as(usize, 0)..n {
        let i = first + ((start_i + k) % n);
        let px = xs[2 * i];
        let py = xs[(2 * i) + 1];
        if (flags[i] & 1) != 0 {
            if have_ctrl {
                pen.quad_to(cx, cy, px, py)!;
                have_ctrl = false;
            } else {
                pen.line_to(px, py)!;
            }
        } else {
            if have_ctrl {
                let mx = (cx + px) / 2.0;
                let my = (cy + py) / 2.0;
                pen.quad_to(cx, cy, mx, my)!;
            }
            cx = px;
            cy = py;
            have_ctrl = true;
        }
    }
    if have_ctrl {
        pen.quad_to(cx, cy, sx, sy)!;
    }
    pen.close()!;
}

fn choose_cmap(d: []u8, t: Table) usize {
    if t.len < 4 {
        return 0;
    }
    let n = @as(usize, u16at(d, t.off + 2));
    let mut best: usize = 0;
    let mut best_score: u32 = 0;
    for i in @as(usize, 0)..n {
        let r = t.off + 4 + (8 * i);
        let plat = u16at(d, r);
        let enc = u16at(d, r + 2);
        let sub = t.off + @as(usize, u32at(d, r + 4));
        if sub >= d.len {
            continue;
        }
        let format = u16at(d, sub);
        let mut score: u32 = 0;
        if (format == 12) and (((plat == 3) and (enc == 10)) or (plat == 0)) {
            score = 5;
        } else if (format == 4) and (((plat == 3) and (enc == 1)) or (plat == 0)) {
            score = 4;
        } else if format == 4 {
            score = 3;
        } else if (format == 6) or (format == 0) {
            score = 1;
        }
        if score > best_score {
            best_score = score;
            best = sub;
        }
    }
    return best;
}

// -----------------------------------------------------------------------------
// Kerning
// -----------------------------------------------------------------------------

fn kern_table(d: []u8, t: usize, left: u32, right: u32) i32 {
    let ntables = @as(usize, u16at(d, t + 2));
    let mut p = t + 4;
    let key = (left << 16) | right;
    let mut total: i32 = 0;
    for i in @as(usize, 0)..ntables {
        let len = @as(usize, u16at(d, p + 2));
        let coverage = u16at(d, p + 4);
        if ((coverage >> 8) == 0) and ((coverage & 1) != 0) {
            let npairs = @as(usize, u16at(d, p + 6));
            let pairs = p + 14;
            let mut lo: usize = 0;
            let mut hi = npairs;
            while lo < hi {
                let mid = (lo + hi) / 2;
                let k = u32at(d, pairs + (6 * mid));
                if k < key {
                    lo = mid + 1;
                } else {
                    hi = mid;
                }
            }
            if (lo < npairs) and (u32at(d, pairs + (6 * lo)) == key) {
                total += i16at(d, pairs + (6 * lo) + 4);
            }
        }
        if len == 0 {
            break;
        }
        p += len;
    }
    return total;
}

/// The index of glyph g in a Coverage table, or -1.
fn coverage_index(d: []u8, c: usize, g: u32) i64 {
    let format = u16at(d, c);
    let n = @as(usize, u16at(d, c + 2));
    if format == 1 {
        let mut lo: usize = 0;
        let mut hi = n;
        while lo < hi {
            let mid = (lo + hi) / 2;
            let v = u16at(d, c + 4 + (2 * mid));
            if v == g {
                return @as(i64, @truncate(u32, mid));
            }
            if v < g {
                lo = mid + 1;
            } else {
                hi = mid;
            }
        }
        return -1;
    }
    if format == 2 {
        let mut lo: usize = 0;
        let mut hi = n;
        while lo < hi {
            let mid = (lo + hi) / 2;
            let r = c + 4 + (6 * mid);
            if u16at(d, r + 2) < g {
                lo = mid + 1;
            } else {
                hi = mid;
            }
        }
        if lo < n {
            let r = c + 4 + (6 * lo);
            let s = u16at(d, r);
            if g >= s {
                return @as(i64, u16at(d, r + 4) + (g - s));
            }
        }
    }
    return -1;
}

fn class_of(d: []u8, c: usize, g: u32) u32 {
    let format = u16at(d, c);
    if format == 1 {
        let start = u16at(d, c + 2);
        let n = u16at(d, c + 4);
        if (g >= start) and (g < (start + n)) {
            return u16at(d, c + 6 + (2 * @as(usize, g - start)));
        }
        return 0;
    }
    if format == 2 {
        let n = @as(usize, u16at(d, c + 2));
        let mut lo: usize = 0;
        let mut hi = n;
        while lo < hi {
            let mid = (lo + hi) / 2;
            let r = c + 4 + (6 * mid);
            if u16at(d, r + 2) < g {
                lo = mid + 1;
            } else {
                hi = mid;
            }
        }
        if lo < n {
            let r = c + 4 + (6 * lo);
            if g >= u16at(d, r) {
                return u16at(d, r + 4);
            }
        }
    }
    return 0;
}

fn value_size(f: u32) usize {
    return 2 * @as(usize, @popcount(f & 0xff));
}

/// The XAdvance of a ValueRecord at v with format f.
fn x_advance(d: []u8, v: usize, f: u32) i32 {
    if (f & 4) == 0 {
        return 0;
    }
    let mut off: usize = 0;
    if (f & 1) != 0 {
        off += 2;
    }
    if (f & 2) != 0 {
        off += 2;
    }
    return i16at(d, v + off);
}

fn gpos_kern(d: []u8, t: usize, left: u32, right: u32) i32 {
    let features = t + @as(usize, u16at(d, t + 6));
    let lookups = t + @as(usize, u16at(d, t + 8));
    let nf = @as(usize, u16at(d, features));
    let nl = @as(usize, u16at(d, lookups));
    for fi in @as(usize, 0)..nf {
        let rec = features + 2 + (6 * fi);
        if not tag_is(d, rec, "kern") {
            continue;
        }
        let ft = features + @as(usize, u16at(d, rec + 4));
        let count = @as(usize, u16at(d, ft + 2));
        for k in @as(usize, 0)..count {
            let li = @as(usize, u16at(d, ft + 4 + (2 * k)));
            if li >= nl {
                continue;
            }
            let lk = lookups + @as(usize, u16at(d, lookups + 2 + (2 * li)));
            let ty = u16at(d, lk);
            let nsub = @as(usize, u16at(d, lk + 4));
            for s in @as(usize, 0)..nsub {
                let mut st = lk + @as(usize, u16at(d, lk + 6 + (2 * s)));
                let mut sty = ty;
                if ty == 9 {
                    sty = u16at(d, st + 2);
                    st = st + @as(usize, u32at(d, st + 4));
                }
                if sty != 2 {
                    continue;
                }
                let r = pair_pos(d, st, left, right);
                if r[0] != 0 {
                    return @truncate(i32, r[1]);
                }
            }
        }
        // on to the next kern feature: scripts may list different lookups
    }
    return 0;
}

/// [found, value] for a PairPos subtable.
fn pair_pos(d: []u8, st: usize, left: u32, right: u32) [2]i64 {
    let format = u16at(d, st);
    let cov = coverage_index(d, st + @as(usize, u16at(d, st + 2)), left);
    if cov < 0 {
        return [0, 0];
    }
    let vf1 = u16at(d, st + 4);
    let vf2 = u16at(d, st + 6);
    let s1 = value_size(vf1);
    let s2 = value_size(vf2);
    if format == 1 {
        let nsets = @as(i64, u16at(d, st + 8));
        if cov >= nsets {
            return [0, 0];
        }
        let set = st + @as(usize, u16at(d, st + 10 + (2 * @truncate(usize, @bit_cast(u64, cov)))));
        let n = @as(usize, u16at(d, set));
        let rec = 2 + s1 + s2;
        let mut lo: usize = 0;
        let mut hi = n;
        while lo < hi {
            let mid = (lo + hi) / 2;
            let g = u16at(d, set + 2 + (rec * mid));
            if g == right {
                return [1, @as(i64, x_advance(d, set + 2 + (rec * mid) + 2, vf1))];
            }
            if g < right {
                lo = mid + 1;
            } else {
                hi = mid;
            }
        }
        return [0, 0];
    }
    if format == 2 {
        let cd1 = st + @as(usize, u16at(d, st + 8));
        let cd2 = st + @as(usize, u16at(d, st + 10));
        let n1 = @as(usize, u16at(d, st + 12));
        let n2 = @as(usize, u16at(d, st + 14));
        let c1 = @as(usize, class_of(d, cd1, left));
        let c2 = @as(usize, class_of(d, cd2, right));
        if (c1 >= n1) or (c2 >= n2) {
            return [0, 0];
        }
        let rec = s1 + s2;
        let at = st + 16 + (rec * ((c1 * n2) + c2));
        return [1, @as(i64, x_advance(d, at, vf1))];
    }
    return [0, 0];
}

// -----------------------------------------------------------------------------
// CFF
// -----------------------------------------------------------------------------

/// An INDEX: its count, and where item i is.
fn index_count(d: []u8, at: usize) usize {
    return @as(usize, u16at(d, at));
}

fn off_at(d: []u8, p: usize, size: usize) usize {
    let mut v: usize = 0;
    for k in @as(usize, 0)..size {
        v = (v << 8) | @as(usize, u8at(d, p + k));
    }
    return v;
}

/// [start, end) of item i of the INDEX at `at`.
fn index_item(d: []u8, at: usize, i: usize) [2]usize {
    let n = index_count(d, at);
    if i >= n {
        return [0, 0];
    }
    let size = @as(usize, u8at(d, at + 2));
    let offs = at + 3;
    let data = offs + ((n + 1) * size) - 1;
    return [data + off_at(d, offs + (i * size), size), data + off_at(d, offs + ((i + 1) * size), size)];
}

/// Where the INDEX at `at` ends.
fn index_end(d: []u8, at: usize) usize {
    let n = index_count(d, at);
    if n == 0 {
        return at + 2;
    }
    let size = @as(usize, u8at(d, at + 2));
    let offs = at + 3;
    return (offs + ((n + 1) * size) - 1) + off_at(d, offs + (n * size), size);
}

/// DICT values: operators to their (last) operands.
struct Dict {
    ops: [8]u32,
    vals: [8][2]f64,
    n: usize,

    fn get(self: *Dict, op: u32, k: usize) f64 {
        for i in @as(usize, 0)..self.n {
            if self.ops[i] == op {
                return self.vals[i][k];
            }
        }
        return 0.0;
    }

    fn has(self: *Dict, op: u32) bool {
        for i in @as(usize, 0)..self.n {
            if self.ops[i] == op {
                return true;
            }
        }
        return false;
    }
}

const OP_CHARSTRINGS: u32 = 17;
const OP_PRIVATE: u32 = 18;
const OP_SUBRS: u32 = 19;
const OP_FDARRAY: u32 = 1236;   // 12 36
const OP_FDSELECT: u32 = 1237;  // 12 37

/// Parses the operators we need from a DICT.
fn parse_dict(d: []u8, a: usize, b: usize) Dict {
    let mut dict = Dict.{ ops: [0; 8], vals: [[0.0, 0.0]; 8], n: 0 };
    let mut stack: [48]f64 = [0.0; 48];
    let mut sp: usize = 0;
    let mut p = a;
    let end = @min(b, d.len);
    while p < end {
        let c = u8at(d, p);
        if c <= 21 {
            let mut op = c;
            p += 1;
            if c == 12 {
                op = 1200 + u8at(d, p);
                p += 1;
            }
            if (op == OP_CHARSTRINGS) or (op == OP_PRIVATE) or (op == OP_SUBRS) or (op == OP_FDARRAY) or
               (op == OP_FDSELECT) {
                if dict.n < 8 {
                    dict.ops[dict.n] = op;
                    if sp >= 2 {
                        dict.vals[dict.n] = [stack[sp - 2], stack[sp - 1]];
                    } else if sp == 1 {
                        dict.vals[dict.n] = [stack[0], stack[0]];
                    }
                    dict.n += 1;
                }
            }
            sp = 0;
            continue;
        }
        let mut v: f64 = 0.0;
        if c == 28 {
            v = @float_from_int(f64, i16at(d, p + 1));
            p += 3;
        } else if c == 29 {
            v = @float_from_int(f64, @bit_cast(i32, u32at(d, p + 1)));
            p += 5;
        } else if c == 30 {
            // a real: skip its nibbles (not needed for the operators we use)
            p += 1;
            loop {
                let bb = u8at(d, p);
                p += 1;
                if ((bb & 15) == 15) or ((bb >> 4) == 15) or (p >= end) {
                    break;
                }
            }
        } else if (c >= 32) and (c <= 246) {
            v = @float_from_int(f64, @as(i32, @bit_cast(i32, c)) - 139);
            p += 1;
        } else if (c >= 247) and (c <= 250) {
            v = @float_from_int(
                f64, (((@as(i32, @bit_cast(i32, c)) - 247) * 256) + @as(i32, @bit_cast(i32, u8at(d, p + 1)))) + 108);
            p += 2;
        } else if (c >= 251) and (c <= 254) {
            v = @float_from_int(
                f64, ((-(@as(i32, @bit_cast(i32, c)) - 251) * 256) - @as(i32, @bit_cast(i32, u8at(d, p + 1)))) - 108);
            p += 2;
        } else {
            p += 1;
        }
        if sp < 48 {
            stack[sp] = v;
            sp += 1;
        }
    }
    return dict;
}

fn parse_cff(d: []u8, base: usize) !Cff {
    let hdr = @as(usize, u8at(d, base + 2));
    let names = base + hdr;
    let tops = index_end(d, names);
    let strings = index_end(d, tops);
    let gsubrs = index_end(d, strings);
    let top = index_item(d, tops, 0);
    if top[1] <= top[0] {
        return error.InvalidFont;
    }
    let dict = parse_dict(d, top[0], top[1]);
    if not dict.has(OP_CHARSTRINGS) {
        return error.InvalidFont;
    }
    let mut cff = Cff.{
        charstrings: base + @int_from_float(usize, dict.get(OP_CHARSTRINGS, 1)),
        gsubrs: gsubrs,
        subrs: 0,
        fdarray: 0,
        fdselect: 0,
        base: base,
    };
    if dict.has(OP_FDARRAY) {
        cff.fdarray = base + @int_from_float(usize, dict.get(OP_FDARRAY, 1));
        cff.fdselect = base + @int_from_float(usize, dict.get(OP_FDSELECT, 1));
    } else if dict.has(OP_PRIVATE) {
        cff.subrs = private_subrs(d, base, dict.get(OP_PRIVATE, 0), dict.get(OP_PRIVATE, 1));
    }
    return cff;
}

/// The local Subrs INDEX of a Private DICT (size, offset), or 0.
fn private_subrs(d: []u8, base: usize, size: f64, off: f64) usize {
    let a = base + @int_from_float(usize, off);
    let pd = parse_dict(d, a, a + @int_from_float(usize, size));
    if pd.has(OP_SUBRS) {
        return a + @int_from_float(usize, pd.get(OP_SUBRS, 1));
    }
    return 0;
}

fn subr_bias(n: usize) i64 {
    if n < 1240 {
        return 107;
    }
    if n < 33900 {
        return 1131;
    }
    return 32768;
}

struct Cs {
    d: []u8,
    pen: *mut Pen,
    stack: [48]f64,
    sp: usize,
    x: f64,
    y: f64,
    nhints: usize,
    have_width: bool,
    open: bool,
    gsubrs: usize,
    subrs: usize,
    done: bool,

    fn push(self: *mut Cs, v: f64) void {
        if self.sp < 48 {
            self.stack[self.sp] = v;
            self.sp += 1;
        }
    }

    /// Stack-clearing operators may start with the glyph's width: drops it
    /// when the operator takes a count of arguments of the other parity.
    fn width_if(self: *mut Cs, odd: bool) usize {
        if not self.have_width {
            self.have_width = true;
            if ((self.sp % 2) == 1) == odd and (self.sp > 0) {
                return 1;
            }
        }
        return 0;
    }

    fn move_to(self: *mut Cs, dx: f64, dy: f64) !void {
        self.x += dx;
        self.y += dy;
        self.pen.move_to(self.x, self.y)!;
        self.open = true;
    }

    fn line_to(self: *mut Cs, dx: f64, dy: f64) !void {
        self.x += dx;
        self.y += dy;
        self.pen.line_to(self.x, self.y)!;
    }

    fn curve_to(self: *mut Cs, dx1: f64, dy1: f64, dx2: f64, dy2: f64, dx3: f64, dy3: f64) !void {
        let ax = self.x + dx1;
        let ay = self.y + dy1;
        let bx = ax + dx2;
        let by = ay + dy2;
        self.x = bx + dx3;
        self.y = by + dy3;
        self.pen.cubic_to(ax, ay, bx, by, self.x, self.y)!;
    }

    fn run(self: *mut Cs, a: usize, b: usize, depth: u32) !void {
        if depth > 10 {
            return error.InvalidFont;
        }
        let d = self.d;
        let mut p = a;
        while (p < b) and not self.done {
            self.pen.spend(1)!;
            let c = u8at(d, p);
            p += 1;
            if c >= 32 {
                if c <= 246 {
                    self.push(@float_from_int(f64, @as(i32, @bit_cast(i32, c)) - 139));
                } else if c <= 250 {
                    self.push(
                        @float_from_int(
                            f64, (
                                ((@as(i32, @bit_cast(i32, c)) - 247) * 256) + @as(
                                    i32, @bit_cast(i32, u8at(d, p)))) + 108));
                    p += 1;
                } else if c <= 254 {
                    self.push(
                        @float_from_int(
                            f64, (
                                (-(@as(i32, @bit_cast(i32, c)) - 251) * 256) - @as(
                                    i32, @bit_cast(i32, u8at(d, p)))) - 108));
                    p += 1;
                } else {
                    self.push(@float_from_int(f64, @bit_cast(i32, u32at(d, p))) / 65536.0);
                    p += 4;
                }
                continue;
            }
            if c == 28 {
                self.push(@float_from_int(f64, i16at(d, p)));
                p += 2;
                continue;
            }
            let s = self.stack;
            if (c == 1) or (c == 3) or (c == 18) or (c == 23) {
                // stems
                let w = self.width_if(true);
                self.nhints += (self.sp - w) / 2;
                self.sp = 0;
            } else if (c == 19) or (c == 20) {
                // hintmask / cntrmask (a vstem may be implied by the stack)
                let w = self.width_if(true);
                self.nhints += (self.sp - w) / 2;
                self.sp = 0;
                p += (self.nhints + 7) / 8;
            } else if c == 21 {
                let w = self.width_if(true);
                self.close_open()!;
                self.move_to(s[w], s[w + 1])!;
                self.sp = 0;
            } else if c == 22 {
                let w = self.width_if(false);
                self.close_open()!;
                self.move_to(s[w], 0.0)!;
                self.sp = 0;
            } else if c == 4 {
                let w = self.width_if(false);
                self.close_open()!;
                self.move_to(0.0, s[w])!;
                self.sp = 0;
            } else if c == 5 {
                let mut i: usize = 0;
                while (i + 1) < self.sp {
                    self.line_to(s[i], s[i + 1])!;
                    i += 2;
                }
                self.sp = 0;
            } else if (c == 6) or (c == 7) {
                // alternating horizontal and vertical lines
                let mut horiz = c == 6;
                for i in @as(usize, 0)..self.sp {
                    if horiz {
                        self.line_to(s[i], 0.0)!;
                    } else {
                        self.line_to(0.0, s[i])!;
                    }
                    horiz = not horiz;
                }
                self.sp = 0;
            } else if c == 8 {
                let mut i: usize = 0;
                while (i + 5) < self.sp {
                    self.curve_to(s[i], s[i + 1], s[i + 2], s[i + 3], s[i + 4], s[i + 5])!;
                    i += 6;
                }
                self.sp = 0;
            } else if (c == 10) or (c == 29) {
                // callsubr / callgsubr
                if self.sp == 0 {
                    return error.InvalidFont;
                }
                self.sp -= 1;
                let mut idx = self.subrs;
                if c == 29 {
                    idx = self.gsubrs;
                }
                if idx == 0 {
                    return error.InvalidFont;
                }
                let n = index_count(d, idx);
                let k = @int_from_float(i64, self.stack[self.sp]) + subr_bias(n);
                if (k < 0) or (k >= @as(i64, @truncate(u32, n))) {
                    return error.InvalidFont;
                }
                let r = index_item(d, idx, @truncate(usize, @bit_cast(u64, k)));
                self.run(r[0], r[1], depth + 1)!;
            } else if c == 11 {
                return;  // return from a subroutine
            } else if c == 14 {
                // endchar
                _ = self.width_if(false);
                self.close_open()!;
                self.done = true;
                return;
            } else if c == 24 {
                // rcurveline
                let mut i: usize = 0;
                while (i + 7) < self.sp {
                    self.curve_to(s[i], s[i + 1], s[i + 2], s[i + 3], s[i + 4], s[i + 5])!;
                    i += 6;
                }
                if (i + 1) < self.sp {
                    self.line_to(s[i], s[i + 1])!;
                }
                self.sp = 0;
            } else if c == 25 {
                // rlinecurve
                let mut i: usize = 0;
                while (i + 7) < self.sp {
                    self.line_to(s[i], s[i + 1])!;
                    i += 2;
                }
                if (i + 5) < self.sp {
                    self.curve_to(s[i], s[i + 1], s[i + 2], s[i + 3], s[i + 4], s[i + 5])!;
                }
                self.sp = 0;
            } else if c == 26 {
                // vvcurveto
                let mut i: usize = 0;
                let mut dx1: f64 = 0.0;
                if (self.sp % 2) == 1 {
                    dx1 = s[0];
                    i = 1;
                }
                while (i + 3) < self.sp {
                    self.curve_to(dx1, s[i], s[i + 1], s[i + 2], 0.0, s[i + 3])!;
                    dx1 = 0.0;
                    i += 4;
                }
                self.sp = 0;
            } else if c == 27 {
                // hhcurveto
                let mut i: usize = 0;
                let mut dy1: f64 = 0.0;
                if (self.sp % 2) == 1 {
                    dy1 = s[0];
                    i = 1;
                }
                while (i + 3) < self.sp {
                    self.curve_to(s[i], dy1, s[i + 1], s[i + 2], s[i + 3], 0.0)!;
                    dy1 = 0.0;
                    i += 4;
                }
                self.sp = 0;
            } else if (c == 30) or (c == 31) {
                // vhcurveto / hvcurveto: alternating starts
                let mut horiz = c == 31;
                let mut i: usize = 0;
                while (i + 3) < self.sp {
                    let last = (i + 4) >= (self.sp - 1);
                    let mut extra: f64 = 0.0;
                    if last and ((self.sp - i) == 5) {
                        extra = s[i + 4];
                    }
                    if horiz {
                        self.curve_to(s[i], 0.0, s[i + 1], s[i + 2], extra, s[i + 3])!;
                    } else {
                        self.curve_to(0.0, s[i], s[i + 1], s[i + 2], s[i + 3], extra)!;
                    }
                    horiz = not horiz;
                    i += 4;
                }
                self.sp = 0;
            } else if c == 12 {
                let op = u8at(d, p);
                p += 1;
                self.flex_op(op)!;
                self.sp = 0;
            } else {
                self.sp = 0;  // an unknown (reserved) operator
            }
        }
    }

    fn flex_op(self: *mut Cs, op: u32) !void {
        let s = self.stack;
        if (op == 35) and (self.sp >= 12) {
            // flex
            self.curve_to(s[0], s[1], s[2], s[3], s[4], s[5])!;
            self.curve_to(s[6], s[7], s[8], s[9], s[10], s[11])!;
        } else if (op == 34) and (self.sp >= 7) {
            // hflex
            let y0 = self.y;
            self.curve_to(s[0], 0.0, s[1], s[2], s[3], 0.0)!;
            self.curve_to(s[4], 0.0, s[5], y0 - (self.y + 0.0), s[6], 0.0)!;
        } else if (op == 36) and (self.sp >= 9) {
            // hflex1
            let y0 = self.y;
            self.curve_to(s[0], s[1], s[2], s[3], s[4], 0.0)!;
            self.curve_to(s[5], 0.0, s[6], s[7], s[8], y0 - (self.y + s[7]))!;
        } else if (op == 37) and (self.sp >= 11) {
            // flex1: the last point moves along the longer axis
            let x0 = self.x;
            let y0 = self.y;
            let dx = (((s[0] + s[2]) + s[4]) + s[6]) + s[8];
            let dy = (((s[1] + s[3]) + s[5]) + s[7]) + s[9];
            self.curve_to(s[0], s[1], s[2], s[3], s[4], s[5])!;
            if @abs(dx) > @abs(dy) {
                self.curve_to(s[6], s[7], s[8], s[9], s[10], y0 - (self.y + s[7] + s[9]))!;
            } else {
                self.curve_to(s[6], s[7], s[8], s[9], x0 - (self.x + s[6] + s[8]), s[10])!;
            }
        }
    }

    fn close_open(self: *mut Cs) !void {
        if self.open {
            self.pen.close()!;
            self.open = false;
        }
    }
}

fn cff_glyph(f: *Font, gid: u32, pen: *mut Pen) !void {
    let d = f.data;
    let cs = f.cff;
    let r = index_item(d, cs.charstrings, @as(usize, gid));
    if r[1] <= r[0] {
        return;
    }
    let mut subrs = cs.subrs;
    if cs.fdarray != 0 {
        // CID-keyed: this glyph's Font DICT gives its Private DICT
        let fd = fd_of(d, cs.fdselect, gid);
        let fr = index_item(d, cs.fdarray, fd);
        if fr[1] > fr[0] {
            let fdict = parse_dict(d, fr[0], fr[1]);
            if fdict.has(OP_PRIVATE) {
                subrs = private_subrs(d, cs.base, fdict.get(OP_PRIVATE, 0), fdict.get(OP_PRIVATE, 1));
            }
        }
    }
    let mut run = Cs.{
        d: d,
        pen: pen,
        stack: [0.0; 48],
        sp: 0,
        x: 0.0,
        y: 0.0,
        nhints: 0,
        have_width: false,
        open: false,
        gsubrs: cs.gsubrs,
        subrs: subrs,
        done: false,
    };
    run.run(r[0], r[1], 0)!;
    run.close_open()!;
}

fn fd_of(d: []u8, sel: usize, gid: u32) usize {
    let format = u8at(d, sel);
    if format == 0 {
        return @as(usize, u8at(d, sel + 1 + @as(usize, gid)));
    }
    if format == 3 {
        let n = @as(usize, u16at(d, sel + 1));
        for i in @as(usize, 0)..n {
            let r = sel + 3 + (3 * i);
            let first = u16at(d, r);
            let next = u16at(d, r + 3);
            if (gid >= first) and (gid < next) {
                return @as(usize, u8at(d, r + 2));
            }
        }
    }
    return 0;
}

// -----------------------------------------------------------------------------
// Text
// -----------------------------------------------------------------------------

/// A glyph placed by layout: its id and pen position (baseline, y down).
pub struct Glyph {
    id: u32,
    x: f64,
    y: f64,
}

/// Laid-out text: glyphs, and the lines' extent.
pub struct Layout {
    glyphs: vec.Vec.[Glyph],
    width: f64,   // of the widest line
    height: f64,  // from the first line's top to the last one's bottom
    lines: usize,

    pub fn deinit(self: *mut Layout) void {
        self.glyphs.deinit();
    }
}

/// The scale from font units to pixels at a size (the em, in pixels).
pub fn scale_for(f: *Font, size: f64) f64 {
    return size / @float_from_int(f64, f.units_per_em);
}

/// The distance between baselines at a size.
pub fn line_height(f: *Font, size: f64) f64 {
    return @float_from_int(f64, (f.ascent - f.descent) + f.line_gap) * scale_for(f, size);
}

/// The width of one line of text at a size (with kerning).
pub fn measure(f: *Font, text: []u8, size: f64) f64 {
    let s = scale_for(f, size);
    let mut it = strs.runes(text);
    let mut x: f64 = 0.0;
    let mut prev: u32 = 0xffffffff;
    loop {
        let r = it.next() catch break;
        let g = f.glyph_index(r);
        if prev != 0xffffffff {
            x += @float_from_int(f64, f.kerning(prev, g)) * s;
        }
        x += @float_from_int(f64, f.advance(g)) * s;
        prev = g;
    }
    return x;
}

/// Lays text out from (x, y) (the first baseline), wrapping lines longer
/// than max_width (0: no wrapping) at spaces (or anywhere, for a word too
/// long to fit); "\n" starts a new line.
pub fn layout(f: *Font, text: []u8, x: f64, y: f64, size: f64, max_width: f64) !Layout {
    let mut out = Layout.{ glyphs: vec.Vec.[Glyph].init(), width: 0.0, height: 0.0, lines: 0 };
    layout_into(f, text, x, y, size, max_width, &mut out) catch |e| {
        out.deinit();
        return e;
    };
    return out;
}

fn layout_into(f: *Font, text: []u8, x0: f64, y0: f64, size: f64, max_width: f64, out: *mut Layout) !void {
    let s = scale_for(f, size);
    let lh = line_height(f, size);
    let mut y = y0;
    let mut line_start: usize = 0;  // index in out.glyphs
    let mut x = x0;
    let mut prev: u32 = 0xffffffff;
    let mut last_space: usize = @max_value(usize);  // glyph index after the last space in this line
    let mut it = strs.runes(text);
    out.lines = 1;
    loop {
        let r = it.next() catch break;
        if r == '\n' {
            out.width = @max(out.width, x - x0);
            x = x0;
            y += lh;
            out.lines += 1;
            line_start = out.glyphs.len;
            prev = 0xffffffff;
            last_space = @max_value(usize);
            continue;
        }
        let g = f.glyph_index(r);
        let mut kern: f64 = 0.0;
        if prev != 0xffffffff {
            kern = @float_from_int(f64, f.kerning(prev, g)) * s;
        }
        let adv = @float_from_int(f64, f.advance(g)) * s;
        if (max_width > 0.0) and (r != ' ') and (((x + kern) + adv) > (x0 + max_width)) and
           (out.glyphs.len > line_start) {
            // wrap: after the last space if there is one, else here
            let mut from = out.glyphs.len;
            if (last_space != @max_value(usize)) and (last_space < out.glyphs.len) {
                from = last_space;
            }
            let mut line_end = x;
            if from < out.glyphs.len {
                line_end = out.glyphs.get(from - 1).x;  // where the space starts
            }
            out.width = @max(out.width, line_end - x0);
            y += lh;
            out.lines += 1;
            // move the glyphs after the break to the new line
            let shift = out.glyphs.len - from;
            let mut nx = x0;
            if shift > 0 {
                let dx = out.glyphs.get(from).x - x0;
                for k in from..out.glyphs.len {
                    let gl = out.glyphs.get_ptr(k);
                    gl.x -= dx;
                    gl.y = y;
                }
                nx = x - dx;
            }
            line_start = from;
            x = nx;
            kern = 0.0;
            last_space = @max_value(usize);
            if shift > 0 {
                prev = out.glyphs.get(out.glyphs.len - 1).id;
                kern = @float_from_int(f64, f.kerning(prev, g)) * s;
            }
        }
        x += kern;
        out.glyphs.push(Glyph.{ id: g, x: x, y: y })!;
        x += adv;
        prev = g;
        if r == ' ' {
            last_space = out.glyphs.len;
        }
    }
    out.width = @max(out.width, x - x0);
    out.height = (@float_from_int(f64, out.lines - 1) * lh) + (@float_from_int(f64, f.ascent - f.descent) * s);
}

/// Draws laid-out text.
pub fn draw_layout(cv: *mut draw.Canvas, f: *Font, l: *Layout, size: f64, paint: draw.Paint) !void {
    let s = scale_for(f, size);
    let mut p = draw.Path.init();
    defer p.deinit();
    for g in l.glyphs.items() {
        f.glyph_path(g.id, &mut p, g.x, g.y, s)!;
    }
    cv.fill(&p, paint, draw.NONZERO)!;
}

/// Draws one line of text with its baseline starting at (x, y).
pub fn draw_text(cv: *mut draw.Canvas, f: *Font, text: []u8, x: f64, y: f64, size: f64, paint: draw.Paint) !void {
    let mut l = layout(f, text, x, y, size, 0.0)!;
    defer l.deinit();
    draw_layout(cv, f, &l, size, paint)!;
}

/// Draws text wrapped to max_width, its first line's top at (x, y);
/// returns the height used.
pub fn draw_text_box(cv: *mut draw.Canvas, f: *Font, text: []u8, x: f64, y: f64, max_width: f64, size: f64,
                     paint: draw.Paint) !f64 {
    let baseline = y + (@float_from_int(f64, f.ascent) * scale_for(f, size));
    let mut l = layout(f, text, x, baseline, size, max_width)!;
    defer l.deinit();
    draw_layout(cv, f, &l, size, paint)!;
    return l.height;
}

// -----------------------------------------------------------------------------
// Tests (with the fonts of the system, when there are any)
// -----------------------------------------------------------------------------

fn try_load(path: []u8) !Font {
    return Font.load(path);
}

test "truetype" {
    let mut f = try_load("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf") catch return;
    defer f.deinit();
    @expect_eq(f.units_per_em, 2048);
    let a = f.glyph_index('A');
    let v = f.glyph_index('V');
    @expect(a != 0);
    @expect_eq(f.glyph_index(0x10ffff), 0);
    @expect(f.advance(a) > 1000);
    @expect(f.kerning(a, v) < 0);  // "AV" kerns
    @expect_eq(f.kerning(f.glyph_index('x'), f.glyph_index('x')), 0);
    let mut p = draw.Path.init();
    defer p.deinit();
    f.glyph_path(a, &mut p, 0.0, 0.0, 1.0)!;
    @expect(p.verbs.len > 5);
    // composite glyph (Aacute)
    p.clear();
    f.glyph_path(f.glyph_index(0xc1), &mut p, 0.0, 0.0, 1.0)!;
    @expect(p.verbs.len > 5);
    // a render: ink appears, and "Hi" is wider than "i"
    let mut img = image.Image.init(80, 40, 1, image.U8)!;
    defer img.deinit();
    let mut cv = draw.Canvas.init(&mut img);
    defer cv.deinit();
    draw_text(&mut cv, &f, "Hi", 5.0, 30.0, 24.0, draw.Paint.solid(color.Color.rgb(1.0, 1.0, 1.0)))!;
    let mut ink: u64 = 0;
    for b in img.samples8() {
        ink += @as(u64, b);
    }
    @expect(ink > 20000);
    @expect(measure(&f, "Hi", 24.0) > measure(&f, "i", 24.0));
    // wrapping
    let mut l = layout(&f, "one two three four", 0.0, 0.0, 20.0, 80.0)!;
    defer l.deinit();
    @expect(l.lines >= 2);
    @expect(l.width <= 80.0);
    @expect(l.width < measure(&f, "one two ", 20.0));  // no trailing space
}

test "collection" {
    let file = io.open("/usr/share/fonts/truetype/wqy/wqy-zenhei.ttc") catch return;
    let bytes = file.reader().read_all()!;
    file.close();
    defer mem.free(bytes);
    let mut a = Font.init_index(bytes, 0)!;
    defer a.deinit();
    let mut b = Font.init_index(bytes, 1)!;
    defer b.deinit();
    let g = a.glyph_index(0x4e2d);  // 中
    @expect(g != 0);
    @expect_eq(b.glyph_index(0x4e2d), g);
    @expect(a.advance(a.glyph_index('i')) != b.advance(b.glyph_index('i')));  // proportional and mono
    let mut p = draw.Path.init();
    defer p.deinit();
    a.glyph_path(g, &mut p, 0.0, 0.0, 1.0)!;
    @expect(p.verbs.len > 5);
    _ = Font.init_index(bytes, 9) catch |e| {
        @expect_eq(e, error.InvalidFont);
        return;
    };
    @trap();
}

test "cff" {
    let mut f = try_load("/usr/share/fonts/opentype/tlwg/Loma.otf") catch return;
    defer f.deinit();
    @expect(f.is_cff);
    let g = f.glyph_index('g');
    @expect(g != 0);
    let mut p = draw.Path.init();
    defer p.deinit();
    f.glyph_path(g, &mut p, 0.0, 0.0, 1.0)!;
    @expect(p.verbs.len > 5);
}
