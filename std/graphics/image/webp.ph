// =============================================================================
// std/graphics/image/webp.ph - WebP images and animations (read and write).
//
//     let mut img = webp.decode(bytes)!;           // the first frame
//     let mut anim = webp.decode_all(bytes)!;      // every frame, composited
//     defer anim.deinit();
//
//     let out = webp.encode(&img)!;                // lossless; mem.free(out)
//     let movie = webp.encode_animation(frames, delays, 0)!;
//     webp.encode_to(w, &img)!;                    // or to an io.Writer
//
// Decoding reads lossy images (VP8, std/graphics/image/webp/vp8.ph), lossless ones
// (VP8L: every transform, colour caches, meta prefix codes), alpha (raw or
// compressed, with each filter) and animations, composited as libwebp's
// WebPAnimDecoder does, so the pixels match libwebp's. Images come out as U8
// RGB, or RGBA when the file has alpha or is animated.
//
// Encoding is lossless (VP8L), with the subtract-green and predictor
// transforms or, for images of 256 colours or fewer, a palette, and LZ77
// with prefix codes.
// =============================================================================

import "../../collections/vec.ph" as vec;
import "../../io.ph" as io;
import "../../mem.ph" as mem;
import "../../os/fs.ph" as fs;
import "../../str.ph" as strs;
import "../image.ph" as image;
import "png.ph" as png;
import "webp/vp8.ph" as vp8;

error { InvalidImage, TooLarge, Unsupported, OutOfMemory }

pub fn is_webp(data: []u8) bool {
    return (data.len >= 16) and strs.eq(data[..4], "RIFF") and strs.eq(data[8..12], "WEBP");
}

pub struct Frame {
    image: image.Image,  // the whole canvas, RGBA
    delay_ms: u32,
}

pub struct Animation {
    width: usize,
    height: usize,
    loops: i32,  // 0: for ever; -1: a still image
    frames: vec.Vec.[Frame],

    pub fn deinit(self: *mut Animation) void {
        for f in self.frames.items() {
            let mut img = f.image;
            img.deinit();
        }
        self.frames.deinit();
    }
}

fn le16(b: []u8, i: usize) usize {
    return @as(usize, b[i]) | (@as(usize, b[i + 1]) << 8);
}

fn le24(b: []u8, i: usize) usize {
    return le16(b, i) | (@as(usize, b[i + 2]) << 16);
}

fn le32(b: []u8, i: usize) usize {
    return le24(b, i) | (@as(usize, b[i + 3]) << 24);
}

// -----------------------------------------------------------------------------
// Lossless (VP8L): bits, prefix codes
// -----------------------------------------------------------------------------

struct Bits {
    data: []u8,
    pos: usize,
    acc: u64,
    n: u64,

    fn fill(self: *mut Bits) void {
        while self.n <= 56 {
            let mut b: u64 = 0;
            if self.pos < self.data.len {
                b = @as(u64, self.data[self.pos]);
            }
            self.pos += 1;
            self.acc |= b << self.n;
            self.n += 8;
        }
    }

    fn read(self: *mut Bits, k: u64) u32 {
        if self.n < k {
            self.fill();
        }
        let v = self.acc & ((@as(u64, 1) << k) - 1);
        self.acc >>= k;
        self.n -= k;
        return @truncate(u32, v);
    }

    // past the end (by more than the 8 bytes of look-ahead)
    fn overrun(self: *Bits) bool {
        return self.pos > (self.data.len + 8);
    }
}

// a canonical prefix code: an 8-bit table for short codes, counts and sorted
// symbols for the rest
struct Tree {
    single: i32,  // the one symbol of a code with only one (read in no bits), or -1
    counts: [16]u16,
    syms: usize,  // offset of the symbols in the Codes arena
    fast: usize,  // offset of the 256-entry table: symbol << 4 | length, or 0
}

struct Codes {
    trees: vec.Vec.[Tree],
    syms: vec.Vec.[u16],
    fast: vec.Vec.[u32],

    fn init() Codes {
        return Codes.{ trees: vec.Vec.[Tree].init(), syms: vec.Vec.[u16].init(), fast: vec.Vec.[u32].init() };
    }

    fn deinit(self: *mut Codes) void {
        self.trees.deinit();
        self.syms.deinit();
        self.fast.deinit();
    }

    fn add(self: *mut Codes, lens: []u8) !void {
        let mut tr = Tree.{ single: -1, counts: [0; 16], syms: self.syms.len, fast: self.fast.len };
        let mut used: usize = 0;
        let mut last: usize = 0;
        for s in @as(usize, 0)..lens.len {
            if lens[s] != 0 {
                tr.counts[@as(usize, lens[s])] += 1;
                used += 1;
                last = s;
            }
        }
        if used == 0 {
            return error.InvalidImage;
        }
        if used == 1 {
            tr.single = @as(i32, @truncate(u16, last));
            self.trees.push(tr)!;
            return;
        }
        let mut left: i32 = 1;
        for l in @as(usize, 1)..16 {
            left = (left * 2) - @as(i32, tr.counts[l]);
            if left < 0 {
                return error.InvalidImage;
            }
        }
        let mut offs: [16]usize = [0; 16];
        for l in @as(usize, 1)..15 {
            offs[l + 1] = offs[l] + @as(usize, tr.counts[l]);
        }
        let base = self.syms.len;
        for _q in @as(usize, 0)..used {
            self.syms.push(0)!;
        }
        let sy = self.syms.items();
        for s in @as(usize, 0)..lens.len {
            if lens[s] != 0 {
                sy[base + offs[@as(usize, lens[s])]] = @truncate(u16, s);
                offs[@as(usize, lens[s])] += 1;
            }
        }
        let fb = self.fast.len;
        for _q in @as(usize, 0)..256 {
            self.fast.push(0)!;
        }
        let ft = self.fast.items();
        // canonical codes, MSB first; the table is indexed by the next 8 bits
        // as read (LSB first)
        let mut code: u32 = 0;
        let mut k: usize = 0;
        for l in @as(usize, 1)..9 {
            for _q in @as(usize, 0)..@as(usize, tr.counts[l]) {
                let s = @as(u32, sy[base + k]);
                let mut r: u32 = 0;
                for b in @as(usize, 0)..l {
                    r |= ((code >> @truncate(u32, b)) & 1) << @truncate(u32, (l - 1) - b);
                }
                let mut j = @as(usize, r);
                while j < 256 {
                    ft[fb + j] = (s << 4) | @truncate(u32, l);
                    j += @as(usize, 1) << l;
                }
                code += 1;
                k += 1;
            }
            code <<= 1;
        }
        self.trees.push(tr)!;
    }

    fn decode(self: *Codes, t: usize, b: *mut Bits) !u32 {
        let tr = self.trees.items()[t];
        if tr.single >= 0 {
            return @bit_cast(u32, tr.single);
        }
        if b.n < 16 {
            b.fill();
        }
        let f = self.fast.items()[tr.fast + @truncate(usize, b.acc & 255)];
        if f != 0 {
            let l = @as(u64, f & 15);
            b.acc >>= l;
            b.n -= l;
            return f >> 4;
        }
        let sy = self.syms.items();
        let mut code: i32 = 0;
        let mut first: i32 = 0;
        let mut index: i32 = 0;
        for l in @as(usize, 1)..16 {
            code |= @as(i32, @truncate(u8, b.acc & 1));
            b.acc >>= 1;
            b.n -= 1;
            let count = @as(i32, tr.counts[l]);
            if (code - first) < count {
                return @as(u32, sy[tr.syms + @as(usize, @bit_cast(u32, index + (code - first)))]);
            }
            index += count;
            first += count;
            first <<= 1;
            code <<= 1;
        }
        return error.InvalidImage;
    }
}

let CODE_LENGTH_ORDER: [19]u8 = [17, 18, 0, 1, 2, 3, 4, 5, 16, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15];

// one prefix code of an alphabet
fn read_code(b: *mut Bits, codes: *mut Codes, alphabet: usize, lens: []mut u8) !void {
    @mem_set(lens[..alphabet], 0);
    if b.read(1) == 1 {
        let n = b.read(1) + 1;
        let first8 = b.read(1);
        let s0 = @as(usize, b.read(1 + (7 * @as(u64, first8))));
        if s0 >= alphabet {
            return error.InvalidImage;
        }
        lens[s0] = 1;
        if n == 2 {
            let s1 = @as(usize, b.read(8));
            if s1 >= alphabet {
                return error.InvalidImage;
            }
            lens[s1] = 1;
        }
        codes.add(lens[..alphabet])!;
        return;
    }
    let mut cl: [19]u8 = [0; 19];
    let nc = @as(usize, b.read(4)) + 4;
    for i in @as(usize, 0)..nc {
        cl[@as(usize, CODE_LENGTH_ORDER[i])] = @truncate(u8, b.read(3));
    }
    let mut lc = Codes.init();
    defer lc.deinit();
    lc.add(cl[..])!;
    let mut max = alphabet;
    if b.read(1) == 1 {
        let nb = 2 + (2 * @as(u64, b.read(3)));
        max = @as(usize, b.read(nb)) + 2;
        if max > alphabet {
            return error.InvalidImage;
        }
    }
    let mut prev: u8 = 8;
    let mut s: usize = 0;
    while s < alphabet {
        if max == 0 {
            break;
        }
        max -= 1;
        let c = lc.decode(0, b)!;
        if c < 16 {
            lens[s] = @truncate(u8, c);
            s += 1;
            if c != 0 {
                prev = @truncate(u8, c);
            }
            continue;
        }
        let mut rep: usize = 0;
        let mut v: u8 = 0;
        if c == 16 {
            rep = 3 + @as(usize, b.read(2));
            v = prev;
        } else if c == 17 {
            rep = 3 + @as(usize, b.read(3));
        } else {
            rep = 11 + @as(usize, b.read(7));
        }
        if (s + rep) > alphabet {
            return error.InvalidImage;
        }
        @mem_set(lens[s..s + rep], v);
        s += rep;
        if b.overrun() {
            return error.InvalidImage;
        }
    }
    codes.add(lens[..alphabet])!;
}

let DIST_MAP: [120]u8 = [0x18, 0x07, 0x17, 0x19, 0x28, 0x06, 0x27, 0x29, 0x16, 0x1a, 0x26, 0x2a, 0x38, 0x05, 0x37, 0x39,
                         0x15, 0x1b, 0x36, 0x3a, 0x25, 0x2b, 0x48, 0x04, 0x47, 0x49, 0x14, 0x1c, 0x35, 0x3b, 0x46, 0x4a,
                         0x24, 0x2c, 0x58, 0x45, 0x4b, 0x34, 0x3c, 0x03, 0x57, 0x59, 0x13, 0x1d, 0x56, 0x5a, 0x23, 0x2d,
                         0x44, 0x4c, 0x55, 0x5b, 0x33, 0x3d, 0x68, 0x02, 0x67, 0x69, 0x12, 0x1e, 0x66, 0x6a, 0x22, 0x2e,
                         0x54, 0x5c, 0x43, 0x4d, 0x65, 0x6b, 0x32, 0x3e, 0x78, 0x01, 0x77, 0x79, 0x53, 0x5d, 0x11, 0x1f,
                         0x64, 0x6c, 0x42, 0x4e, 0x76, 0x7a, 0x21, 0x2f, 0x75, 0x7b, 0x31, 0x3f, 0x63, 0x6d, 0x52, 0x5e,
                         0x00, 0x74, 0x7c, 0x41, 0x4f, 0x10, 0x20, 0x62, 0x6e, 0x30, 0x73, 0x7d, 0x51, 0x5f, 0x40, 0x72,
                         0x7e, 0x61, 0x6f, 0x50, 0x71, 0x7f, 0x60, 0x70];

// a length or distance from its prefix symbol and extra bits
fn copy_value(b: *mut Bits, sym: u32) usize {
    if sym < 4 {
        return @as(usize, sym) + 1;
    }
    let extra = (sym - 2) >> 1;
    let offset = (2 + (sym & 1)) << extra;
    return @as(usize, offset + b.read(@as(u64, extra))) + 1;
}

fn plane_distance(w: usize, code: usize) usize {
    if code > 120 {
        return code - 120;
    }
    let dc = @as(usize, DIST_MAP[code - 1]);
    let yo = dc >> 4;
    let xo = dc & 15;
    // (8 - xo) may be negative: distance = yo * w + 8 - xo
    let d = ((yo * w) + 8);
    if d <= xo {
        return 1;
    }
    return d - xo;
}

fn tiles(n: usize, bits: u32) usize {
    return (n + ((@as(usize, 1) << bits) - 1)) >> bits;
}

// the prefix-coded pixels of one image (after its transforms, if any)
fn read_pixels(b: *mut Bits, w: usize, h: usize, level0: bool) ![]mut u32 {
    let mut cache_bits: u32 = 0;
    if b.read(1) == 1 {
        cache_bits = b.read(4);
        if (cache_bits < 1) or (cache_bits > 11) {
            return error.InvalidImage;
        }
    }
    let mut meta_bits: u32 = 0;
    let mut meta: []mut u32 = @slice_from_ptr(@ptr_from_int(*mut u32, 4), 0);
    let mut ngroups: usize = 1;
    if level0 and (b.read(1) == 1) {
        meta_bits = b.read(3) + 2;
        meta = read_image(b, tiles(w, meta_bits), tiles(h, meta_bits), false)!;
        for i in @as(usize, 0)..meta.len {
            ngroups = @max(ngroups, @as(usize, (meta[i] >> 8) & 0xffff) + 1);
        }
    }
    defer {
        if meta.len > 0 {
            mem.free(meta);
        }
    }
    let cache_size = (@as(usize, 1) << cache_bits) & ~@as(usize, 1);
    let mut codes = Codes.init();
    defer codes.deinit();
    let lens = mem.alloc.[u8](280 + 2048)!;
    defer mem.free(lens);
    for _g in @as(usize, 0)..ngroups {
        let mut cs: usize = 0;
        if cache_bits > 0 {
            cs = @as(usize, 1) << cache_bits;
        }
        read_code(b, &mut codes, 280 + cs, lens)!;
        read_code(b, &mut codes, 256, lens)!;
        read_code(b, &mut codes, 256, lens)!;
        read_code(b, &mut codes, 256, lens)!;
        read_code(b, &mut codes, 40, lens)!;
        if b.overrun() {
            return error.InvalidImage;
        }
    }
    _ = cache_size;
    let total = w * h;
    let px = mem.alloc.[u32](@max(total, 1))!;
    let mut cache = mem.alloc_zeroed.[u32](@as(usize, 1) << cache_bits) catch |e| {
        mem.free(px);
        return e;
    };
    defer mem.free(cache);
    let shift = 32 - cache_bits;
    let mw = tiles(w, meta_bits);
    let mut pos: usize = 0;
    let mut cached: usize = 0;  // pixels [..cached] are in the cache
    let mut x: usize = 0;
    let mut y: usize = 0;
    while pos < total {
        let mut g: usize = 0;
        if meta_bits > 0 {
            g = @as(usize, (meta[((y >> meta_bits) * mw) + (x >> meta_bits)] >> 8) & 0xffff);
        }
        let t = g * 5;
        let green = codes.decode(t, b) catch |e| {
            mem.free(px);
            return e;
        };
        if green < 256 {
            let r = codes.decode(t + 1, b) catch |e| {
                mem.free(px);
                return e;
            };
            let bl = codes.decode(t + 2, b) catch |e| {
                mem.free(px);
                return e;
            };
            let a = codes.decode(t + 3, b) catch |e| {
                mem.free(px);
                return e;
            };
            px[pos] = (((a << 24) | (r << 16)) | (green << 8)) | bl;
            pos += 1;
            x += 1;
            if x == w {
                x = 0;
                y += 1;
            }
        } else if green < 280 {
            let len = copy_value(b, green - 256);
            let ds = codes.decode(t + 4, b) catch |e| {
                mem.free(px);
                return e;
            };
            let dist = plane_distance(w, copy_value(b, ds));
            if (dist > pos) or (len > (total - pos)) {
                mem.free(px);
                return error.InvalidImage;
            }
            for k in @as(usize, 0)..len {
                px[pos + k] = px[(pos + k) - dist];
            }
            pos += len;
            x += len;
            while x >= w {
                x -= w;
                y += 1;
            }
        } else {
            let key = @as(usize, green - 280);
            if (cache_bits == 0) or (key >= cache.len) {
                mem.free(px);
                return error.InvalidImage;
            }
            while cached < pos {
                cache[@as(usize, (px[cached] *% 0x1e35a7bd) >> shift)] = px[cached];
                cached += 1;
            }
            px[pos] = cache[key];
            pos += 1;
            x += 1;
            if x == w {
                x = 0;
                y += 1;
            }
        }
        if cache_bits > 0 {
            while cached < pos {
                cache[@as(usize, (px[cached] *% 0x1e35a7bd) >> shift)] = px[cached];
                cached += 1;
            }
        }
        if b.overrun() {
            mem.free(px);
            return error.InvalidImage;
        }
    }
    return px;
}

struct Transform {
    kind: u32,
    bits: u32,
    width: usize,     // the image's width before it
    data: []mut u32,  // the sub-image, or the palette (256 entries)
}

// an image with its transforms (level0: the main image, which may have them)
fn read_image(b: *mut Bits, w: usize, h: usize, level0: bool) ![]mut u32 {
    let mut ts: [4]Transform = [
        Transform.{ kind: 0, bits: 0, width: 0, data: @slice_from_ptr(@ptr_from_int(*mut u32, 4), 0) }; 4];
    let mut nt: usize = 0;
    let mut seen: u32 = 0;
    let mut cw = w;
    defer {
        for i in @as(usize, 0)..nt {
            mem.free(ts[i].data);
        }
    }
    if level0 {
        while b.read(1) == 1 {
            let kind = b.read(2);
            if (seen & (@as(u32, 1) << kind)) != 0 {
                return error.InvalidImage;
            }
            seen |= @as(u32, 1) << kind;
            let mut tr = Transform.{
                kind: kind, bits: 0, width: cw, data: @slice_from_ptr(@ptr_from_int(*mut u32, 4), 0) };
            if (kind == 0) or (kind == 1) {
                tr.bits = b.read(3) + 2;
                tr.data = read_image(b, tiles(cw, tr.bits), tiles(h, tr.bits), false)!;
            } else if kind == 3 {
                let n = @as(usize, b.read(8)) + 1;
                if n <= 2 {
                    tr.bits = 3;
                } else if n <= 4 {
                    tr.bits = 2;
                } else if n <= 16 {
                    tr.bits = 1;
                }
                let pal = read_image(b, n, 1, false)!;
                tr.data = mem.alloc_zeroed.[u32](256) catch |e| {
                    mem.free(pal);
                    return e;
                };
                tr.data[0] = pal[0];
                for i in @as(usize, 1)..n {
                    tr.data[i] = add_pixels(pal[i], tr.data[i - 1]);
                }
                mem.free(pal);
                cw = tiles(cw, tr.bits);
            } else {
                tr.data = mem.alloc.[u32](1)!;
            }
            ts[nt] = tr;
            nt += 1;
        }
    }
    let mut px = read_pixels(b, cw, h, level0)!;
    // undo the transforms, last first
    let mut k = nt;
    while k > 0 {
        k -= 1;
        let tr = ts[k];
        if tr.kind == 0 {
            unpredict(px, tr.width, h, tr.bits, tr.data);
        } else if tr.kind == 1 {
            uncolor(px, tr.width, h, tr.bits, tr.data);
        } else if tr.kind == 2 {
            for i in @as(usize, 0)..px.len {
                let g = (px[i] >> 8) & 0xff;
                px[i] = (px[i] & 0xff00ff00) | (((px[i] & 0x00ff00ff) + ((g << 16) | g)) & 0x00ff00ff);
            }
        } else {
            let full = mem.alloc.[u32](@max(tr.width * h, 1)) catch |e| {
                mem.free(px);
                return e;
            };
            let pw = tiles(tr.width, tr.bits);
            let per = @as(usize, 1) << tr.bits;
            let bpp = @as(u32, 8) >> tr.bits;
            let mask = (@as(u32, 1) << bpp) - 1;
            for y in @as(usize, 0)..h {
                for x in @as(usize, 0)..tr.width {
                    let p = (px[(y * pw) + (x >> tr.bits)] >> 8) & 0xff;
                    let idx = (p >> (@truncate(u32, x & (per - 1)) * bpp)) & mask;
                    full[(y * tr.width) + x] = tr.data[@as(usize, idx)];
                }
            }
            mem.free(px);
            px = full;
        }
    }
    return px;
}

fn add_pixels(a: u32, b: u32) u32 {
    let ag = ((a & 0xff00ff00) +% (b & 0xff00ff00)) & 0xff00ff00;
    let rb = ((a & 0x00ff00ff) +% (b & 0x00ff00ff)) & 0x00ff00ff;
    return ag | rb;
}

fn average2(a: u32, b: u32) u32 {
    return (((a ^ b) & 0xfefefefe) >> 1) +% (a & b);
}

fn channel(p: u32, s: u32) i32 {
    return @as(i32, @truncate(u8, p >> s));
}

fn clamp255(v: i32) u32 {
    if v < 0 {
        return 0;
    }
    if v > 255 {
        return 255;
    }
    return @bit_cast(u32, v);
}

fn select(l: u32, t: u32, tl: u32) u32 {
    let mut pl: i32 = 0;
    let mut pt: i32 = 0;
    for s in [@as(u32, 0), 8, 16, 24]{
        let a = channel(t, s) - channel(tl, s);
        let c = channel(l, s) - channel(tl, s);
        pl += @max(a, -a);
        pt += @max(c, -c);
    }
    if pl < pt {
        return l;
    }
    return t;
}

fn clamp_add_sub_full(a: u32, b: u32, c: u32) u32 {
    let mut r: u32 = 0;
    for s in [@as(u32, 0), 8, 16, 24]{
        r |= clamp255((channel(a, s) + channel(b, s)) - channel(c, s)) << s;
    }
    return r;
}

fn clamp_add_sub_half(a: u32, b: u32) u32 {
    let mut r: u32 = 0;
    for s in [@as(u32, 0), 8, 16, 24]{
        let x = channel(a, s);
        r |= clamp255(x + ((x - channel(b, s)) / 2)) << s;
    }
    return r;
}

fn predict(mode: u32, l: u32, t: u32, tr: u32, tl: u32) u32 {
    if mode == 1 {
        return l;
    }
    if mode == 2 {
        return t;
    }
    if mode == 3 {
        return tr;
    }
    if mode == 4 {
        return tl;
    }
    if mode == 5 {
        return average2(average2(l, tr), t);
    }
    if mode == 6 {
        return average2(l, tl);
    }
    if mode == 7 {
        return average2(l, t);
    }
    if mode == 8 {
        return average2(tl, t);
    }
    if mode == 9 {
        return average2(t, tr);
    }
    if mode == 10 {
        return average2(average2(l, tl), average2(t, tr));
    }
    if mode == 11 {
        return select(l, t, tl);
    }
    if mode == 12 {
        return clamp_add_sub_full(l, t, tl);
    }
    if mode == 13 {
        return clamp_add_sub_half(average2(l, t), tl);
    }
    return 0xff000000;
}

fn unpredict(px: []mut u32, w: usize, h: usize, bits: u32, modes: []u32) void {
    let mw = tiles(w, bits);
    for y in @as(usize, 0)..h {
        for x in @as(usize, 0)..w {
            let i = (y * w) + x;
            let mut p: u32 = 0xff000000;
            if y == 0 {
                if x > 0 {
                    p = px[i - 1];
                }
            } else if x == 0 {
                p = px[i - w];
            } else {
                let m = (modes[((y >> bits) * mw) + (x >> bits)] >> 8) & 15;
                p = predict(m, px[i - 1], px[i - w], px[(i - w) + 1], px[(i - w) - 1]);
            }
            px[i] = add_pixels(px[i], p);
        }
    }
}

fn delta(t: u32, c: u32) i32 {
    return (@as(i32, @bit_cast(i8, @truncate(u8, t))) * @as(i32, @bit_cast(i8, @truncate(u8, c)))) >> 5;
}

fn uncolor(px: []mut u32, w: usize, h: usize, bits: u32, elems: []u32) void {
    let mw = tiles(w, bits);
    for y in @as(usize, 0)..h {
        for x in @as(usize, 0)..w {
            let e = elems[((y >> bits) * mw) + (x >> bits)];
            let i = (y * w) + x;
            let p = px[i];
            let g = (p >> 8) & 0xff;
            let r = @bit_cast(u32, @as(i32, @truncate(u8, p >> 16)) + delta(e, g)) & 0xff;
            let b = @bit_cast(u32, (@as(i32, @truncate(u8, p)) + delta(e >> 8, g)) + delta(e >> 16, r)) & 0xff;
            px[i] = ((p & 0xff00ff00) | (r << 16)) | b;
        }
    }
}

// a whole VP8L bitstream (after its chunk header)
fn decode_vp8l(data: []u8, w: *mut usize, h: *mut usize, alpha: *mut bool) ![]mut u32 {
    if (data.len < 5) or (data[0] != 0x2f) {
        return error.InvalidImage;
    }
    let mut b = Bits.{ data: data[1..], pos: 0, acc: 0, n: 0 };
    w.* = @as(usize, b.read(14)) + 1;
    h.* = @as(usize, b.read(14)) + 1;
    alpha.* = b.read(1) == 1;
    if b.read(3) != 0 {
        return error.Unsupported;
    }
    if (w.* * h.*) > image.MAX_PIXELS {
        return error.TooLarge;
    }
    return read_image(&mut b, w.*, h.*, true);
}

// -----------------------------------------------------------------------------
// Frames: VP8 or VP8L, with ALPH
// -----------------------------------------------------------------------------

struct Pic {
    width: usize,
    height: usize,
    rgba: []mut u8,
    alpha: bool,
}

fn decode_alpha(data: []u8, w: usize, h: usize, out: []mut u8) !void {
    if data.len < 1 {
        return error.InvalidImage;
    }
    let method = data[0] & 3;
    let filter = (data[0] >> 2) & 3;
    if method == 0 {
        if (data.len - 1) < (w * h) {
            return error.InvalidImage;
        }
        @mem_copy(out[..w * h], data[1..1 + (w * h)]);
    } else if method == 1 {
        let mut b = Bits.{ data: data[1..], pos: 0, acc: 0, n: 0 };
        let px = read_image(&mut b, w, h, true)!;
        for i in @as(usize, 0)..(w * h) {
            out[i] = @truncate(u8, px[i] >> 8);
        }
        mem.free(px);
    } else {
        return error.InvalidImage;
    }
    if filter == 0 {
        return;
    }
    for y in @as(usize, 0)..h {
        let row = y * w;
        for x in @as(usize, 0)..w {
            let mut pred: u8 = 0;
            if (y == 0) and (x > 0) {
                pred = out[row + x - 1];
            } else if (y > 0) and (x == 0) {
                pred = out[row - w];
            } else if (y > 0) and (x > 0) {
                let l = out[row + x - 1];
                let t = out[(row - w) + x];
                if filter == 1 {
                    pred = l;
                } else if filter == 2 {
                    pred = t;
                } else {
                    let g = (@as(i32, l) + @as(i32, t)) - @as(i32, out[(row - w) + x - 1]);
                    pred = @truncate(u8, clamp255(g));
                }
            }
            out[row + x] = out[row + x] +% pred;
        }
    }
}

// a frame's chunks: [ALPH] then VP8, or VP8L
fn decode_pic(data: []u8) !Pic {
    let mut p: usize = 0;
    let mut alph: []u8 = data[..0];
    let mut have_alph = false;
    while (p + 8) <= data.len {
        let tag = data[p..p + 4];
        let size = le32(data, p + 4);
        if size > (data.len - (p + 8)) {
            return error.InvalidImage;
        }
        let body = data[p + 8..p + 8 + size];
        if strs.eq(tag, "ALPH") {
            alph = body;
            have_alph = true;
        } else if strs.eq(tag, "VP8 ") {
            let hd = vp8.header(body) catch return error.InvalidImage;
            if (hd.width * hd.height) > image.MAX_PIXELS {
                return error.TooLarge;
            }
            let rgba = mem.alloc.[u8](hd.width * hd.height * 4)!;
            vp8.decode(body, rgba) catch |e| {
                mem.free(rgba);
                if e == error.OutOfMemory {
                    return e;
                }
                return error.InvalidImage;
            };
            if have_alph {
                let am = mem.alloc.[u8](hd.width * hd.height) catch |e| {
                    mem.free(rgba);
                    return e;
                };
                defer mem.free(am);
                decode_alpha(alph, hd.width, hd.height, am) catch |e| {
                    mem.free(rgba);
                    return e;
                };
                for i in @as(usize, 0)..am.len {
                    rgba[(i * 4) + 3] = am[i];
                }
            }
            return Pic.{ width: hd.width, height: hd.height, rgba: rgba, alpha: have_alph };
        } else if strs.eq(tag, "VP8L") {
            let mut w: usize = 0;
            let mut h: usize = 0;
            let mut alpha = false;
            let px = decode_vp8l(body, &mut w, &mut h, &mut alpha)!;
            defer mem.free(px);
            let rgba = mem.alloc.[u8](w * h * 4)!;
            for i in @as(usize, 0)..(w * h) {
                let v = px[i];
                rgba[i * 4] = @truncate(u8, v >> 16);
                rgba[(i * 4) + 1] = @truncate(u8, v >> 8);
                rgba[(i * 4) + 2] = @truncate(u8, v);
                rgba[(i * 4) + 3] = @truncate(u8, v >> 24);
            }
            return Pic.{ width: w, height: h, rgba: rgba, alpha: alpha };
        }
        p += 8 + size + (size & 1);
    }
    return error.InvalidImage;
}

// -----------------------------------------------------------------------------
// Files and animations
// -----------------------------------------------------------------------------

/// The first frame: RGB, or RGBA with alpha or for animations.
pub fn decode(data: []u8) !image.Image {
    let mut a = decode_frames(data, true)!;
    let f = a.frames.get(0);
    a.frames.clear();
    a.deinit();
    return f.image;
}

/// Every frame, composited onto the canvas.
pub fn decode_all(data: []u8) !Animation {
    return decode_frames(data, false);
}

fn to_image(p: *Pic, channels: usize) !image.Image {
    let mut img = image.Image.init(p.width, p.height, channels, image.Depth.U8)!;
    if channels == 4 {
        @mem_copy(img.data[..p.rgba.len], p.rgba);
    } else {
        for i in @as(usize, 0)..(p.width * p.height) {
            img.data[i * 3] = p.rgba[i * 4];
            img.data[(i * 3) + 1] = p.rgba[(i * 4) + 1];
            img.data[(i * 3) + 2] = p.rgba[(i * 4) + 2];
        }
    }
    return img;
}

fn blend_channel(src: u32, sa: u32, dst: u32, da: u32, scale: u32, shift: u32) u32 {
    let s = (src >> shift) & 0xff;
    let d = (dst >> shift) & 0xff;
    return (((s * sa) + (d * da)) * scale) >> 24;
}

// libwebp's BlendPixelNonPremult; pixels as R | G << 8 | B << 16 | A << 24
fn blend(src: u32, dst: u32) u32 {
    let sa = src >> 24;
    if sa == 0 {
        return dst;
    }
    let da = dst >> 24;
    let dfa = (da * (256 - sa)) >> 8;
    let ba = sa + dfa;
    let scale = (@as(u32, 1) << 24) / ba;
    let r = blend_channel(src, sa, dst, dfa, scale, 0);
    let g = blend_channel(src, sa, dst, dfa, scale, 8);
    let b = blend_channel(src, sa, dst, dfa, scale, 16);
    return ((r | (g << 8)) | (b << 16)) | (ba << 24);
}

fn get_px(b: []u8, i: usize) u32 {
    return ((@as(u32, b[i]) | (@as(u32, b[i + 1]) << 8)) | (@as(u32, b[i + 2]) << 16)) | (@as(u32, b[i + 3]) << 24);
}

fn set_px(b: []mut u8, i: usize, v: u32) void {
    b[i] = @truncate(u8, v);
    b[i + 1] = @truncate(u8, v >> 8);
    b[i + 2] = @truncate(u8, v >> 16);
    b[i + 3] = @truncate(u8, v >> 24);
}

struct Rect {
    x: usize,
    y: usize,
    w: usize,
    h: usize,
    dispose: bool,  // to transparent, after it is shown
    blend: bool,
    alpha: bool,
}

fn decode_frames(data: []u8, first_only: bool) !Animation {
    if not is_webp(data) {
        return error.InvalidImage;
    }
    let end = @min(data.len, le32(data, 4) + 8);
    let body = data[12..end];
    let mut anim = Animation.{ width: 0, height: 0, loops: -1, frames: vec.Vec.[Frame].init() };
    let first = body[..4];
    if strs.eq(first, "VP8 ") or strs.eq(first, "VP8L") {
        let mut p = decode_pic(body) catch |e| {
            anim.deinit();
            return e;
        };
        defer mem.free(p.rgba);
        let mut ch: usize = 3;
        if p.alpha {
            ch = 4;
        }
        let img = to_image(&p, ch)!;
        anim.width = p.width;
        anim.height = p.height;
        anim.frames.push(Frame.{ image: img, delay_ms: 0 })!;
        return anim;
    }
    if not strs.eq(first, "VP8X") or (body.len < 18) {
        return error.InvalidImage;
    }
    let flags = body[8];
    let cw = le24(body, 12) + 1;
    let chh = le24(body, 15) + 1;
    if (cw * chh) > image.MAX_PIXELS {
        return error.TooLarge;
    }
    anim.width = cw;
    anim.height = chh;
    if (flags & 2) == 0 {
        // a still image, maybe with alpha
        let mut p = decode_pic(body[18..]) catch |e| {
            anim.deinit();
            return e;
        };
        defer mem.free(p.rgba);
        let mut ch: usize = 3;
        if (flags & 0x10) != 0 {
            ch = 4;
        }
        let img = to_image(&p, ch)!;
        anim.frames.push(Frame.{ image: img, delay_ms: 0 })!;
        return anim;
    }
    // an animation, composited as WebPAnimDecoder does
    anim.loops = 0;
    let n = cw * chh * 4;
    let canvas = mem.alloc_zeroed.[u8](n)!;
    defer mem.free(canvas);
    let prev = mem.alloc_zeroed.[u8](n)!;  // the last canvas, disposed
    defer mem.free(prev);
    let mut last = Rect.{ x: 0, y: 0, w: 0, h: 0, dispose: false, blend: false, alpha: false };
    let mut last_key = false;
    let mut count: usize = 0;
    let mut p: usize = 18;
    while (p + 8) <= body.len {
        let tag = body[p..p + 4];
        let size = le32(body, p + 4);
        if size > (body.len - (p + 8)) {
            break;
        }
        let c = body[p + 8..p + 8 + size];
        p += 8 + size + (size & 1);
        if strs.eq(tag, "ANIM") and (c.len >= 6) {
            anim.loops = @as(i32, @truncate(u16, le16(c, 4)));
            continue;
        }
        if not strs.eq(tag, "ANMF") or (c.len < 16) {
            continue;
        }
        let mut pic = decode_pic(c[16..]) catch |e| {
            anim.deinit();
            return e;
        };
        defer mem.free(pic.rgba);
        let r = Rect.{
            x: le24(c, 0) * 2,
            y: le24(c, 3) * 2,
            w: le24(c, 6) + 1,
            h: le24(c, 9) + 1,
            dispose: (c[15] & 1) != 0,
            blend: (c[15] & 2) == 0,
            alpha: pic.alpha,
        };
        let delay = @truncate(u32, le24(c, 12));
        if ((r.x + r.w) > cw) or ((r.y + r.h) > chh) or (r.w != pic.width) or (r.h != pic.height) {
            anim.deinit();
            return error.InvalidImage;
        }
        let full = (r.w == cw) and (r.h == chh);
        let mut key = count == 0;
        if not key {
            if ((not r.alpha) or (not r.blend)) and full {
                key = true;
            } else if last.dispose and (((last.w == cw) and (last.h == chh)) or last_key) {
                key = true;
            }
        }
        if key {
            @mem_set(canvas, 0);
        } else {
            @mem_copy(canvas, prev);
        }
        for y in @as(usize, 0)..r.h {
            let o = (((r.y + y) * cw) + r.x) * 4;
            @mem_copy(canvas[o..o + (r.w * 4)], pic.rgba[(y * r.w) * 4..((y + 1) * r.w) * 4]);
        }
        if (count > 0) and r.blend and not key {
            for y in @as(usize, 0)..r.h {
                let cy = r.y + y;
                for x in @as(usize, 0)..r.w {
                    let cx = r.x + x;
                    if last.dispose and (cy >= last.y) and (cy < (last.y + last.h)) and (cx >= last.x) and
                       (cx < (last.x + last.w)) {
                        continue;  // (the disposed area is not blended)
                    }
                    let o = ((cy * cw) + cx) * 4;
                    let s = get_px(canvas, o);
                    if (s >> 24) != 255 {
                        set_px(canvas, o, blend(s, get_px(prev, o)));
                    }
                }
            }
        }
        let mut img = image.Image.init(cw, chh, 4, image.Depth.U8) catch |e| {
            anim.deinit();
            return e;
        };
        @mem_copy(img.data[..n], canvas);
        anim.frames.push(Frame.{ image: img, delay_ms: delay }) catch |e| {
            img.deinit();
            anim.deinit();
            return e;
        };
        count += 1;
        if first_only {
            return anim;
        }
        last = r;
        last_key = key;
        @mem_copy(prev, canvas);
        if r.dispose {
            for y in @as(usize, 0)..r.h {
                let o = (((r.y + y) * cw) + r.x) * 4;
                @mem_set(prev[o..o + (r.w * 4)], 0);
            }
        }
    }
    if count == 0 {
        anim.deinit();
        return error.InvalidImage;
    }
    return anim;
}

// -----------------------------------------------------------------------------
// Encoding (lossless)
// -----------------------------------------------------------------------------

// Bits, least significant first, gathered in buf and written to out
// when it fills.
struct Writer {
    out: io.Writer,
    buf: [4096]u8,
    len: usize,
    acc: u64,
    n: u64,

    fn put(self: *mut Writer, v: u32, k: u64) !void {
        self.acc |= @as(u64, v) << self.n;
        self.n += k;
        if self.len > (self.buf.len - 8) {
            self.out.write_all(self.buf[..self.len])!;
            self.len = 0;
        }
        while self.n >= 8 {
            self.buf[self.len] = @truncate(u8, self.acc);
            self.len += 1;
            self.acc >>= 8;
            self.n -= 8;
        }
    }

    // pads the last byte and writes everything out
    fn flush(self: *mut Writer) !void {
        if self.n > 0 {
            self.buf[self.len] = @truncate(u8, self.acc);
            self.len += 1;
        }
        self.acc = 0;
        self.n = 0;
        self.out.write_all(self.buf[..self.len])!;
        self.len = 0;
    }
}

// code lengths of at most max_len bits for the frequencies (0 for unused
// symbols): Huffman's, with the frequencies flattened until they fit
fn code_lengths(freq: []u32, lens: []mut u8, max_len: u8) !void {
    let n = freq.len;
    @mem_set(lens, 0);
    let f = mem.alloc.[u32](n)!;
    defer mem.free(f);
    @mem_copy(f, freq);
    // nodes: leaves then internal; weights and parents
    let w = mem.alloc.[u64](2 * n)!;
    defer mem.free(w);
    let parent = mem.alloc.[u32](2 * n)!;
    defer mem.free(parent);
    let order = mem.alloc.[u32](n)!;
    defer mem.free(order);
    loop {
        let mut m: usize = 0;
        for s in @as(usize, 0)..n {
            if f[s] != 0 {
                order[m] = @truncate(u32, s);
                m += 1;
            }
        }
        if m == 0 {
            return;
        }
        if m == 1 {
            lens[@as(usize, order[0])] = 1;
            return;
        }
        // sort the used symbols by weight (insertion sort; alphabets are small)
        for i in @as(usize, 1)..m {
            let v = order[i];
            let mut j = i;
            while (j > 0) and (f[@as(usize, order[j - 1])] > f[@as(usize, v)]) {
                order[j] = order[j - 1];
                j -= 1;
            }
            order[j] = v;
        }
        // the two-queue construction: leaves in order, then internal nodes
        for i in @as(usize, 0)..m {
            w[i] = @as(u64, f[@as(usize, order[i])]);
        }
        let mut li: usize = 0;
        let mut ni: usize = m;
        let mut nn: usize = m;
        for _q in @as(usize, 0)..(m - 1) {
            let mut pick: [2]usize = [0; 2];
            for k in @as(usize, 0)..2 {
                if (li < m) and ((ni >= nn) or (w[li] <= w[ni])) {
                    pick[k] = li;
                    li += 1;
                } else {
                    pick[k] = ni;
                    ni += 1;
                }
            }
            w[nn] = w[pick[0]] + w[pick[1]];
            parent[pick[0]] = @truncate(u32, nn);
            parent[pick[1]] = @truncate(u32, nn);
            nn += 1;
        }
        // depths, from the root down
        let d = mem.alloc.[u8](nn)!;
        defer mem.free(d);
        d[nn - 1] = 0;
        let mut i = nn - 1;
        let mut too_long = false;
        while i > 0 {
            i -= 1;
            d[i] = d[@as(usize, parent[i])] + 1;
            if (i < m) and (d[i] > max_len) {
                too_long = true;
            }
        }
        if not too_long {
            for k in @as(usize, 0)..m {
                lens[@as(usize, order[k])] = d[k];
            }
            return;
        }
        for s in @as(usize, 0)..n {
            if f[s] != 0 {
                f[s] = (f[s] >> 1) | 1;
            }
        }
    }
}

// canonical codes, bit-reversed for writing LSB first
fn codes_for(lens: []u8, codes: []mut u32) void {
    let mut count: [16]u32 = [0; 16];
    for s in @as(usize, 0)..lens.len {
        count[@as(usize, lens[s])] += 1;
    }
    count[0] = 0;
    let mut next: [16]u32 = [0; 16];
    let mut c: u32 = 0;
    for l in @as(usize, 1)..16 {
        c = (c + count[l - 1]) << 1;
        next[l] = c;
    }
    for s in @as(usize, 0)..lens.len {
        let l = @as(usize, lens[s]);
        codes[s] = 0;
        if l == 0 {
            continue;
        }
        let v = next[l];
        next[l] += 1;
        let mut r: u32 = 0;
        for b in @as(usize, 0)..l {
            r |= ((v >> @truncate(u32, b)) & 1) << @truncate(u32, (l - 1) - b);
        }
        codes[s] = r;
    }
}

fn used_symbols(lens: []u8) usize {
    let mut n: usize = 0;
    for s in @as(usize, 0)..lens.len {
        if lens[s] != 0 {
            n += 1;
        }
    }
    return n;
}

// a symbol (nothing when the code has only one)
fn put_sym(wr: *mut Writer, lens: []u8, codes: []u32, s: usize) !void {
    if used_symbols(lens) > 1 {
        wr.put(codes[s], @as(u64, lens[s]))!;
    }
}

// a prefix code's description
fn write_code(wr: *mut Writer, lens: []mut u8) !void {
    let mut used: [2]usize = [0; 2];
    let mut n: usize = 0;
    for s in @as(usize, 0)..lens.len {
        if lens[s] != 0 {
            if n < 2 {
                used[n] = s;
            }
            n += 1;
        }
    }
    if n == 0 {
        lens[0] = 1;  // (a code must have a symbol)
        n = 1;
    }
    if (n <= 2) and (used[0] < 256) and (used[1] < 256) {
        // the simple form: one or two symbols of 1 bit
        wr.put(1, 1)!;
        wr.put(@truncate(u32, n - 1), 1)!;
        if used[0] < 2 {
            wr.put(0, 1)!;
            wr.put(@truncate(u32, used[0]), 1)!;
        } else {
            wr.put(1, 1)!;
            wr.put(@truncate(u32, used[0]), 8)!;
        }
        if n == 2 {
            wr.put(@truncate(u32, used[1]), 8)!;
        }
        return;
    }
    // lengths, with runs of zeros as 17 (3-10) and 18 (11-138)
    let mut toks = vec.Vec.[u32].init();  // symbol | extra << 8
    defer toks.deinit();
    let mut i: usize = 0;
    while i < lens.len {
        if lens[i] == 0 {
            let mut r: usize = 1;
            while ((i + r) < lens.len) and (lens[i + r] == 0) and (r < 138) {
                r += 1;
            }
            if r >= 11 {
                toks.push(18 | (@truncate(u32, r - 11) << 8))!;
                i += r;
                continue;
            }
            if r >= 3 {
                toks.push(17 | (@truncate(u32, r - 3) << 8))!;
                i += r;
                continue;
            }
        }
        toks.push(@as(u32, lens[i]))!;
        i += 1;
    }
    let mut freq: [19]u32 = [0; 19];
    for t in toks.items() {
        freq[@as(usize, t & 0xff)] += 1;
    }
    let mut cl: [19]u8 = [0; 19];
    code_lengths(freq[..], cl[..], 7)!;
    let mut cc: [19]u32 = [0; 19];
    codes_for(cl[..], cc[..]);
    let mut nc: usize = 4;
    for k in @as(usize, 0)..19 {
        if cl[@as(usize, CODE_LENGTH_ORDER[k])] != 0 {
            nc = @max(nc, k + 1);
        }
    }
    wr.put(0, 1)!;
    wr.put(@truncate(u32, nc - 4), 4)!;
    for k in @as(usize, 0)..nc {
        wr.put(@as(u32, cl[@as(usize, CODE_LENGTH_ORDER[k])]), 3)!;
    }
    wr.put(0, 1)!;  // (every symbol's length follows)
    for t in toks.items() {
        let s = @as(usize, t & 0xff);
        put_sym(wr, cl[..], cc[..], s)!;
        if s == 17 {
            wr.put(t >> 8, 3)!;
        } else if s == 18 {
            wr.put(t >> 8, 7)!;
        }
    }
}

// the prefix symbol and extra bits of a length or distance code (v >= 1)
fn prefix_of(v: usize, extra: *mut u32, nextra: *mut u64) u32 {
    let x = v - 1;
    if x < 4 {
        extra.* = 0;
        nextra.* = 0;
        return @truncate(u32, x);
    }
    let mut h: u32 = 0;
    while (x >> @as(usize, h + 1)) != 0 {
        h += 1;
    }
    let second = @truncate(u32, (x >> @as(usize, h - 1)) & 1);
    nextra.* = @as(u64, h - 1);
    extra.* = @truncate(u32, x & ((@as(usize, 1) << @as(usize, h - 1)) - 1));
    return (2 * h) + second;
}

const HASH_BITS: u32 = 16;

// pixels, prefix-coded with LZ77 (one group of codes, no colour cache)
fn write_image(wr: *mut Writer, px: []u32, w: usize, level0: bool) !void {
    wr.put(0, 1)!;  // no colour cache
    if level0 {
        wr.put(0, 1)!;  // no meta prefix codes
    }
    let n = px.len;
    // the short distance codes for this width
    let near = mem.alloc_zeroed.[u8]((8 * w) + 9)!;
    defer mem.free(near);
    let mut c: usize = 120;
    while c >= 1 {
        let d = plane_distance(w, c);
        if d < near.len {
            near[d] = @truncate(u8, c);
        }
        c -= 1;
    }
    // tokens: a literal is (0, pixel); a copy is (length | 1 << 31, code)
    let mut toks = vec.Vec.[u32].init();
    defer toks.deinit();
    let head = mem.alloc.[i32](@as(usize, 1) << HASH_BITS)!;
    defer mem.free(head);
    @mem_set(@cast_slice(u8, head), 0xff);
    let prev = mem.alloc.[i32](@max(n, 1))!;
    defer mem.free(prev);
    let mut i: usize = 0;
    while i < n {
        let mut best_len: usize = 0;
        let mut best_dist: usize = 0;
        if (i + 2) < n {
            // the pixel to the left and the one above first, then the chain
            let mut cands: [2]usize = [1, w];
            for k in @as(usize, 0)..2 {
                let d = cands[k];
                if (d == 0) or (d > i) {
                    continue;
                }
                let mut l: usize = 0;
                while ((i + l) < n) and (l < 4096) and (px[(i + l) - d] == px[i + l]) {
                    l += 1;
                }
                if l > best_len {
                    best_len = l;
                    best_dist = d;
                }
            }
            let hsh = @as(usize, ((px[i] *% 0x9e3779b1) ^ (px[i + 1] *% 0x85ebca6b)) >> (32 - HASH_BITS));
            let mut cand = head[hsh];
            let mut steps: usize = 0;
            while (cand >= 0) and (steps < 32) {
                let p = @as(usize, @bit_cast(u32, cand));
                let d = i - p;
                if d > 1000000 {
                    break;
                }
                let mut l: usize = 0;
                while ((i + l) < n) and (l < 4096) and (px[p + l] == px[i + l]) {
                    l += 1;
                }
                if l > best_len {
                    best_len = l;
                    best_dist = d;
                }
                cand = prev[p];
                steps += 1;
            }
        }
        let mut take: usize = 1;
        if best_len >= 3 {
            let mut code = best_dist + 120;
            if (best_dist < near.len) and (near[best_dist] != 0) {
                code = @as(usize, near[best_dist]);
            }
            toks.push(@truncate(u32, best_len) | 0x80000000)!;
            toks.push(@truncate(u32, code))!;
            take = best_len;
        } else {
            toks.push(0)!;
            toks.push(px[i])!;
        }
        for k in @as(usize, 0)..take {
            let j = i + k;
            if (j + 1) < n {
                let hj = @as(usize, ((px[j] *% 0x9e3779b1) ^ (px[j + 1] *% 0x85ebca6b)) >> (32 - HASH_BITS));
                prev[j] = head[hj];
                head[hj] = @bit_cast(i32, @truncate(u32, j));
            }
        }
        i += take;
    }
    // the codes
    let fg = mem.alloc_zeroed.[u32](280)!;
    defer mem.free(fg);
    let fr = mem.alloc_zeroed.[u32](256)!;
    defer mem.free(fr);
    let fb = mem.alloc_zeroed.[u32](256)!;
    defer mem.free(fb);
    let fa = mem.alloc_zeroed.[u32](256)!;
    defer mem.free(fa);
    let fd = mem.alloc_zeroed.[u32](40)!;
    defer mem.free(fd);
    let t = toks.items();
    let mut e: u32 = 0;
    let mut ne: u64 = 0;
    let mut k: usize = 0;
    while k < t.len {
        if (t[k] & 0x80000000) != 0 {
            fg[256 + @as(usize, prefix_of(@as(usize, t[k] & 0xffff), &mut e, &mut ne))] += 1;
            fd[@as(usize, prefix_of(@as(usize, t[k + 1]), &mut e, &mut ne))] += 1;
        } else {
            let p = t[k + 1];
            fg[@as(usize, (p >> 8) & 0xff)] += 1;
            fr[@as(usize, (p >> 16) & 0xff)] += 1;
            fb[@as(usize, p & 0xff)] += 1;
            fa[@as(usize, p >> 24)] += 1;
        }
        k += 2;
    }
    let lg = mem.alloc.[u8](280)!;
    defer mem.free(lg);
    let lr = mem.alloc.[u8](256)!;
    defer mem.free(lr);
    let lb = mem.alloc.[u8](256)!;
    defer mem.free(lb);
    let la = mem.alloc.[u8](256)!;
    defer mem.free(la);
    let ld = mem.alloc.[u8](40)!;
    defer mem.free(ld);
    code_lengths(fg, lg, 15)!;
    code_lengths(fr, lr, 15)!;
    code_lengths(fb, lb, 15)!;
    code_lengths(fa, la, 15)!;
    code_lengths(fd, ld, 15)!;
    write_code(wr, lg)!;
    write_code(wr, lr)!;
    write_code(wr, lb)!;
    write_code(wr, la)!;
    write_code(wr, ld)!;
    let cg = mem.alloc.[u32](280)!;
    defer mem.free(cg);
    let cr = mem.alloc.[u32](256)!;
    defer mem.free(cr);
    let cb = mem.alloc.[u32](256)!;
    defer mem.free(cb);
    let ca = mem.alloc.[u32](256)!;
    defer mem.free(ca);
    let cd = mem.alloc.[u32](40)!;
    defer mem.free(cd);
    codes_for(lg, cg);
    codes_for(lr, cr);
    codes_for(lb, cb);
    codes_for(la, ca);
    codes_for(ld, cd);
    k = 0;
    while k < t.len {
        if (t[k] & 0x80000000) != 0 {
            let ls = prefix_of(@as(usize, t[k] & 0xffff), &mut e, &mut ne);
            put_sym(wr, lg, cg, 256 + @as(usize, ls))!;
            wr.put(e, ne)!;
            let ds = prefix_of(@as(usize, t[k + 1]), &mut e, &mut ne);
            put_sym(wr, ld, cd, @as(usize, ds))!;
            wr.put(e, ne)!;
        } else {
            let p = t[k + 1];
            put_sym(wr, lg, cg, @as(usize, (p >> 8) & 0xff))!;
            put_sym(wr, lr, cr, @as(usize, (p >> 16) & 0xff))!;
            put_sym(wr, lb, cb, @as(usize, p & 0xff))!;
            put_sym(wr, la, ca, @as(usize, p >> 24))!;
        }
        k += 2;
    }
}

fn sub_pixels(a: u32, b: u32) u32 {
    let ag = ((a | 0x00ff00ff) -% (b & 0xff00ff00)) & 0xff00ff00;
    let rb = ((a | 0xff00ff00) -% (b & 0x00ff00ff)) & 0x00ff00ff;
    return ag | rb;
}

fn residual_cost(r: u32) u32 {
    let mut c: u32 = 0;
    for s in [@as(u32, 0), 8, 16, 24]{
        let v = @as(i32, @bit_cast(i8, @truncate(u8, r >> s)));
        c += @bit_cast(u32, @max(v, -v));
    }
    return c;
}

// a VP8L bitstream for ARGB pixels (which it changes), written to out
fn encode_vp8l(px: []mut u32, w: usize, h: usize, out: io.Writer) !void {
    let mut wr = Writer.{ out: out, buf: [0; 4096], len: 0, acc: 0, n: 0 };
    let mut alpha = false;
    for p in px {
        if (p >> 24) != 255 {
            alpha = true;
        }
    }
    wr.put(0x2f, 8)!;
    wr.put(@truncate(u32, w - 1), 14)!;
    wr.put(@truncate(u32, h - 1), 14)!;
    let mut abit: u32 = 0;
    if alpha {
        abit = 1;
    }
    wr.put(abit, 1)!;
    wr.put(0, 3)!;
    // few colours: a palette
    let mut pal: [256]u32 = [0; 256];
    let mut np: usize = 0;
    for p in px {
        let mut found = false;
        for k in @as(usize, 0)..np {
            if pal[k] == p {
                found = true;
                break;
            }
        }
        if not found {
            if np == 256 {
                np = 257;
                break;
            }
            pal[np] = p;
            np += 1;
        }
    }
    if np <= 256 {
        let mut bits: u32 = 0;
        if np <= 2 {
            bits = 3;
        } else if np <= 4 {
            bits = 2;
        } else if np <= 16 {
            bits = 1;
        }
        wr.put(1, 1)!;
        wr.put(3, 2)!;
        wr.put(@truncate(u32, np - 1), 8)!;
        let mut deltas: [256]u32 = [0; 256];
        deltas[0] = pal[0];
        for k in @as(usize, 1)..np {
            deltas[k] = sub_pixels(pal[k], pal[k - 1]);
        }
        write_image(&mut wr, deltas[..np], np, false)!;
        wr.put(0, 1)!;
        let pw = tiles(w, bits);
        let packed = mem.alloc_zeroed.[u32](pw * h)!;
        defer mem.free(packed);
        let per = @as(usize, 1) << bits;
        let bpp = @as(u32, 8) >> bits;
        for y in @as(usize, 0)..h {
            for x in @as(usize, 0)..w {
                let p = px[(y * w) + x];
                let mut idx: u32 = 0;
                for k in @as(usize, 0)..np {
                    if pal[k] == p {
                        idx = @truncate(u32, k);
                        break;
                    }
                }
                packed[(y * pw) + (x >> bits)] |= idx << ((@truncate(u32, x & (per - 1)) * bpp) + 8);
            }
        }
        for k in @as(usize, 0)..packed.len {
            packed[k] |= 0xff000000;
        }
        write_image(&mut wr, packed, pw, true)!;
    } else {
        // subtract green, then the predictor that suits each 16x16 block
        for k in @as(usize, 0)..px.len {
            let g = (px[k] >> 8) & 0xff;
            px[k] = (px[k] & 0xff00ff00) | (((px[k] | 0xff00ff00) -% ((g << 16) | g)) & 0x00ff00ff);
        }
        let bits: u32 = 4;
        let mw = tiles(w, bits);
        let mh = tiles(h, bits);
        let modes = mem.alloc.[u32](mw * mh)!;
        defer mem.free(modes);
        let res = mem.alloc.[u32](px.len)!;
        defer mem.free(res);
        for by in @as(usize, 0)..mh {
            for bx in @as(usize, 0)..mw {
                let mut best: u32 = 0;
                let mut best_cost: u32 = 0xffffffff;
                for m in @as(u32, 0)..14 {
                    let mut cost: u32 = 0;
                    for y in (by * 16)..@min((by + 1) * 16, h) {
                        for x in (bx * 16)..@min((bx + 1) * 16, w) {
                            cost += residual_cost(sub_pixels(px[(y * w) + x], pred_at(px, w, x, y, m)));
                        }
                    }
                    if cost < best_cost {
                        best_cost = cost;
                        best = m;
                    }
                }
                modes[(by * mw) + bx] = 0xff000000 | (best << 8);
            }
        }
        for y in @as(usize, 0)..h {
            for x in @as(usize, 0)..w {
                let m = (modes[((y >> bits) * mw) + (x >> bits)] >> 8) & 15;
                res[(y * w) + x] = sub_pixels(px[(y * w) + x], pred_at(px, w, x, y, m));
            }
        }
        wr.put(1, 1)!;
        wr.put(2, 2)!;
        wr.put(1, 1)!;
        wr.put(0, 2)!;
        wr.put(bits - 2, 3)!;
        write_image(&mut wr, modes, mw, false)!;
        wr.put(0, 1)!;
        write_image(&mut wr, res, w, true)!;
    }
    wr.flush()!;
}

// the prediction for (x, y) as the decoder makes it
fn pred_at(px: []u32, w: usize, x: usize, y: usize, m: u32) u32 {
    let i = (y * w) + x;
    if y == 0 {
        if x == 0 {
            return 0xff000000;
        }
        return px[i - 1];
    }
    if x == 0 {
        return px[i - w];
    }
    return predict(m, px[i - 1], px[i - w], px[(i - w) + 1], px[(i - w) - 1]);
}

fn argb_of(img: *image.Image) ![]mut u32 {
    let mut src = img.convert(4, image.Depth.U8)!;
    defer src.deinit();
    let n = img.width * img.height;
    let px = mem.alloc.[u32](n)!;
    let d = src.data;
    for i in @as(usize, 0)..n {
        px[i] = (
            ((@as(u32, d[(i * 4) + 3]) << 24) | (@as(u32, d[i * 4]) << 16)) | (
                @as(u32, d[(i * 4) + 1]) << 8)) | @as(u32, d[(i * 4) + 2]);
    }
    return px;
}

fn put_le(o: []mut u8, v: usize, n: usize) void {
    for k in @as(usize, 0)..n {
        o[k] = @truncate(u8, v >> (8 * k));
    }
}

fn check_size(img: *image.Image) !void {
    if (img.width == 0) or (img.height == 0) or (img.width > 16384) or (img.height > 16384) {
        return error.TooLarge;
    }
}

/// A lossless WebP of the image (free it with mem.free).
pub fn encode(img: *image.Image) ![]mut u8 {
    let mut s = strs.String.init();
    encode_to(s.writer(), img) catch |e| {
        s.deinit();
        return e;
    };
    return s.take();
}

/// Writes a lossless WebP of the image to w. The RIFF header holds the
/// size of what follows, so the image's VP8L data is made in memory first.
pub fn encode_to(w: io.Writer, img: *image.Image) !void {
    check_size(img)!;
    let px = argb_of(img)!;
    defer mem.free(px);
    let mut body = strs.String.init();
    defer body.deinit();
    encode_vp8l(px, img.width, img.height, body.writer())!;
    let n = body.len;
    let mut head: [20]u8 = [0; 20];
    @mem_copy(head[..], "RIFF\x00\x00\x00\x00WEBPVP8L\x00\x00\x00\x00");
    put_le(head[4..8], (12 + n) + (n & 1), 4);
    put_le(head[16..20], n, 4);
    w.write_all(head[..])!;
    w.write_all(body.bytes())!;
    if (n & 1) != 0 {
        w.write_all("\x00")!;
    }
}

/// A lossless animation of frames of one size, each shown for its delay;
/// loops: 0 for ever. Free it with mem.free.
pub fn encode_animation(frames: []*image.Image, delays_ms: []u32, loops: u16) ![]mut u8 {
    let mut s = strs.String.init();
    encode_animation_to(s.writer(), frames, delays_ms, loops) catch |e| {
        s.deinit();
        return e;
    };
    return s.take();
}

/// Writes a lossless animation as encode_animation does to w, a frame at a
/// time. The RIFF header holds the size of the whole file, so every frame
/// is encoded twice: once to measure it, once to write it.
pub fn encode_animation_to(w: io.Writer, frames: []*image.Image, delays_ms: []u32, loops: u16) !void {
    if (frames.len == 0) or (delays_ms.len != frames.len) {
        return error.InvalidImage;
    }
    let wd = frames[0].width;
    let h = frames[0].height;
    check_size(frames[0])!;
    for f in frames {
        if (f.width != wd) or (f.height != h) {
            return error.InvalidImage;
        }
    }
    // each frame's VP8L size
    let sizes = mem.alloc.[usize](frames.len)!;
    defer mem.free(sizes);
    let mut total: usize = 4 + 18 + 14;  // "WEBP", VP8X and ANIM
    for i in @as(usize, 0)..frames.len {
        let px = argb_of(frames[i])!;
        defer mem.free(px);
        let mut cw = io.CountingWriter.init(io.discard());
        encode_vp8l(px, wd, h, cw.writer())!;
        sizes[i] = @int_cast(usize, cw.count);
        total += (32 + sizes[i]) + (sizes[i] & 1);
    }
    let mut head: [44]u8 = [0; 44];
    @mem_copy(head[..20], "RIFF\x00\x00\x00\x00WEBPVP8X\x0a\x00\x00\x00");
    put_le(head[4..8], total, 4);
    head[20] = 0x12;  // VP8X flags: animation, alpha
    put_le(head[24..27], wd - 1, 3);
    put_le(head[27..30], h - 1, 3);
    @mem_copy(head[30..38], "ANIM\x06\x00\x00\x00");  // (the background colour is 0)
    put_le(head[42..44], @as(usize, loops), 2);
    w.write_all(head[..])!;
    for i in @as(usize, 0)..frames.len {
        let n = sizes[i];
        let mut fh: [32]u8 = [0; 32];
        @mem_copy(fh[0..4], "ANMF");
        put_le(fh[4..8], (24 + n) + (n & 1), 4);
        // (x and y offsets 0)
        put_le(fh[14..17], wd - 1, 3);
        put_le(fh[17..20], h - 1, 3);
        put_le(fh[20..23], @as(usize, @min(delays_ms[i], 0xffffff)), 3);
        fh[23] = 2;  // no blending, no disposal
        @mem_copy(fh[24..28], "VP8L");
        put_le(fh[28..32], n, 4);
        w.write_all(fh[..])!;
        let px = argb_of(frames[i])!;
        defer mem.free(px);
        let mut cw = io.CountingWriter.init(w);
        encode_vp8l(px, wd, h, cw.writer())!;
        if cw.count != @as(u64, n) {
            return error.InvalidImage;  // (cannot happen: the encoding is deterministic)
        }
        if (n & 1) != 0 {
            w.write_all("\x00")!;
        }
    }
}

// -----------------------------------------------------------------------------
// Tests
// -----------------------------------------------------------------------------

fn same(a: *image.Image, b: *image.Image) bool {
    return (a.width == b.width) and (a.height == b.height) and (a.channels == b.channels) and strs.eq(a.data, b.data);
}

test "files made by libwebp" {
    let list = fs.read_file("std/graphics/image/testdata/webp/cases.txt") catch return;
    defer mem.free(list);
    let mut lines = strs.lines(list);
    let mut path = strs.String.init();
    defer path.deinit();
    loop {
        let line = lines.next() catch break;
        let mut fl = strs.fields(line);
        let name = fl.next() catch continue;
        path.clear();
        path.push_str("std/graphics/image/testdata/webp/")!;
        path.push_str(name)!;
        path.push_str(".webp")!;
        let data = fs.read_file(path.bytes())!;
        defer mem.free(data);
        @expect(is_webp(data));
        let mut a = decode_all(data) catch |e| {
            @expect_eq(name, "");
            return e;
        };
        defer a.deinit();
        for k in @as(usize, 0)..a.frames.len {
            path.clear();
            path.push_str("std/graphics/image/testdata/webp/")!;
            path.push_str(name)!;
            path.push('.')!;
            path.push('0' + @truncate(u8, k))!;
            path.push_str(".png")!;
            let want_bytes = fs.read_file(path.bytes())!;
            defer mem.free(want_bytes);
            let mut want = png.decode(want_bytes)!;
            defer want.deinit();
            let mut f = a.frames.get(k).image;
            if not same(&f, &want) {
                @expect_eq(path.bytes(), "");
            }
        }
    }
}

fn test_image(w: usize, h: usize, ch: usize, colours: usize) !image.Image {
    let mut img = image.Image.init(w, h, ch, image.Depth.U8)!;
    for y in @as(usize, 0)..h {
        for x in @as(usize, 0)..w {
            for c in @as(usize, 0)..ch {
                let mut v = (((x * (3 + c)) + (y * 5)) + ((x / 7) * (y / 5))) & 255;
                if colours > 0 {
                    v = (((x / 3) + (y / 4)) % colours) * 40;
                }
                if (c == 3) and ((x % 11) == 0) {
                    v = 0;
                }
                img.data[(((y * w) + x) * ch) + c] = @truncate(u8, v);
            }
        }
    }
    return img;
}

test "round trips" {
    let sizes: [4][2]usize = [[1, 1], [37, 23], [64, 64], [200, 3]];
    for si in @as(usize, 0)..4 {
        for ch in @as(usize, 1)..5 {
            for colours in [@as(usize, 0), 2, 3, 5, 7]{
                let mut img = test_image(sizes[si][0], sizes[si][1], ch, colours)!;
                defer img.deinit();
                let enc = encode(&img)!;
                defer mem.free(enc);
                let mut back = decode(enc)!;
                defer back.deinit();
                let mut want = img.convert(back.channels, image.Depth.U8)!;
                defer want.deinit();
                @expect(same(&back, &want));
            }
        }
    }
}

test "animation" {
    let mut a = test_image(30, 20, 4, 0)!;
    defer a.deinit();
    let mut b = test_image(30, 20, 4, 3)!;
    defer b.deinit();
    let frames: [2]*image.Image = [&a, &b];
    let delays: [2]u32 = [40, 70];
    let enc = encode_animation(frames[..], delays[..], 0)!;
    defer mem.free(enc);
    let mut back = decode_all(enc)!;
    defer back.deinit();
    @expect_eq(back.frames.len, 2);
    @expect_eq(back.loops, 0);
    @expect_eq(back.frames.get(1).delay_ms, 70);
    let mut f0 = back.frames.get(0).image;
    let mut f1 = back.frames.get(1).image;
    @expect(same(&f0, &a) and same(&f1, &b));
}

fn fails(data: []u8) bool {
    let mut a = decode_all(data) catch return true;
    a.deinit();
    return false;
}

test "damaged files" {
    let names: [4][]u8 = ["std/graphics/image/testdata/webp/lossy_alpha.webp",
                          "std/graphics/image/testdata/webp/lossless_m6.webp",
                          "std/graphics/image/testdata/webp/anim_lossy.webp",
                          "std/graphics/image/testdata/webp/palette16.webp"];
    for n in @as(usize, 0)..4 {
        let data = fs.read_file(names[n]) catch return;
        defer mem.free(data);
        let mut k: usize = 0;
        while k < data.len {
            _ = fails(data[..k]);
            k += 1 + (k / 8);
        }
        let copy = strs.dup(data)!;
        defer mem.free(copy);
        for i in @as(usize, 0)..@min(copy.len, 600) {
            if (i >= 24) and (i < 30) {
                continue;  // (a VP8X canvas size is believed, and may be huge)
            }
            let old = copy[i];
            copy[i] = old ^ @truncate(u8, (i * 37) | 1);
            _ = fails(copy);
            copy[i] = old;
        }
    }
}

test "encode_to" {
    let mut a = test_image(37, 23, 4, 0)!;
    defer a.deinit();
    let mut b = test_image(37, 23, 3, 5)!;
    defer b.deinit();
    let enc = encode(&a)!;
    defer mem.free(enc);
    let mut s = strs.String.init();
    defer s.deinit();
    encode_to(s.writer(), &a)!;
    @expect(strs.eq(s.bytes(), enc));
    let frames: [2]*image.Image = [&a, &b];
    let delays: [2]u32 = [40, 70];
    let movie = encode_animation(frames[..], delays[..], 3)!;
    defer mem.free(movie);
    s.clear();
    encode_animation_to(s.writer(), frames[..], delays[..], 3)!;
    @expect(strs.eq(s.bytes(), movie));
    let mut small: [100]u8 = [0; 100];
    let mut bw = io.BufferWriter.init(small[..]);
    encode_animation_to(bw.writer(), frames[..], delays[..], 3) catch |e| {
        @expect_eq(e, error.NoSpaceLeft);
        return;
    };
    @trap();
}
