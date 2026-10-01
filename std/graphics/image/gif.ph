// =============================================================================
// std/graphics/image/gif.ph - GIF images and animations (read and write).
//
//     let mut img = gif.decode(bytes)!;                // the first frame, RGBA
//     let mut anim = gif.decode_all(bytes)!;           // every frame, composited
//     defer anim.deinit();
//     for f in anim.frames.items() { ... f.image, f.delay_ms ... }
//
//     let out = gif.encode(&img)!;                     // mem.free(out)
//     let movie = gif.encode_animation(frames, delays, 0)!;   // loop for ever
//     gif.encode_to(w, &img)!;                         // or to an io.Writer
//
// Decoding composites each frame onto the canvas as a viewer would
// (disposal methods, transparency, interlacing, local palettes), so every
// frame comes out as a whole RGBA picture. Encoding keeps images of 256
// colours or fewer exact, and reduces others by median cut (without
// dithering); alpha below 128 becomes transparent.
// =============================================================================

import "../../collections/vec.ph" as vec;
import "../../io.ph" as io;
import "../../mem.ph" as mem;
import "../../os/fs.ph" as fs;
import "../../str.ph" as strs;
import "../image.ph" as image;
import "png.ph" as png;

error { InvalidImage, TooLarge, Unsupported, OutOfMemory }

pub fn is_gif(data: []u8) bool {
    return (data.len >= 6) and (strs.eq(data[..6], "GIF87a") or strs.eq(data[..6], "GIF89a"));
}

pub struct Frame {
    image: image.Image,  // the whole canvas, RGBA
    delay_ms: u32,
}

pub struct Animation {
    width: usize,
    height: usize,
    loops: i32,  // 0: for ever; -1: play once (no loop count given)
    frames: vec.Vec.[Frame],

    pub fn deinit(self: *mut Animation) void {
        for f in self.frames.items() {
            let mut img = f.image;
            img.deinit();
        }
        self.frames.deinit();
    }
}

/// The first frame.
pub fn decode(data: []u8) !image.Image {
    let mut a = decode_frames(data, true)!;
    let f = a.frames.get(0);
    a.frames.clear();
    a.deinit();
    return f.image;
}

/// Every frame.
pub fn decode_all(data: []u8) !Animation {
    return decode_frames(data, false)!;
}

fn le16(b: []u8, i: usize) usize {
    return @as(usize, b[i]) | (@as(usize, b[i + 1]) << 8);
}

// the data sub-blocks from i, joined; i moves past the terminator
fn sub_blocks(b: []u8, i: *mut usize, out: *mut strs.String) !void {
    loop {
        if i.* >= b.len {
            return error.InvalidImage;
        }
        let n = @as(usize, b[i.*]);
        i.* += 1;
        if n == 0 {
            return;
        }
        if (i.* + n) > b.len {
            return error.InvalidImage;
        }
        out.push_str(b[i.*..i.* + n])!;
        i.* += n;
    }
}

fn skip_blocks(b: []u8, i: *mut usize) !void {
    loop {
        if i.* >= b.len {
            return error.InvalidImage;
        }
        let n = @as(usize, b[i.*]);
        i.* += 1 + n;
        if n == 0 {
            return;
        }
    }
}

fn decode_frames(b: []u8, first_only: bool) !Animation {
    if not is_gif(b) or (b.len < 13) {
        return error.InvalidImage;
    }
    let w = le16(b, 6);
    let h = le16(b, 8);
    if (w == 0) or (h == 0) {
        return error.InvalidImage;
    }
    if (w * h) > image.MAX_PIXELS {
        return error.TooLarge;
    }
    let flags = b[10];
    let mut i: usize = 13;
    let mut gct: []u8 = b[0..0];
    if (flags & 0x80) != 0 {
        let n = 3 * (@as(usize, 2) << @as(usize, flags & 7));
        if (i + n) > b.len {
            return error.InvalidImage;
        }
        gct = b[i..i + n];
        i += n;
    }
    let mut anim = Animation.{ width: w, height: h, loops: -1, frames: vec.Vec.[Frame].init() };
    let mut canvas = image.Image.init(w, h, 4, image.Depth.U8) catch |e| {
        anim.deinit();
        return e;
    };
    defer canvas.deinit();
    let mut saved = image.Image.init(w, h, 4, image.Depth.U8) catch |e| {
        anim.deinit();
        return e;
    };
    defer saved.deinit();
    let mut lzw = strs.String.init();
    defer lzw.deinit();
    // the graphic control of the next image
    let mut disposal: u8 = 0;
    let mut delay: u32 = 0;
    let mut transparent: i32 = -1;
    // the previous frame's disposal, and where it was
    let mut prev_disposal: u8 = 0;
    let mut prev: [4]usize = [0; 4];
    let mut work: usize = 0;  // pixels decoded, against hostile files
    loop {
        if i >= b.len {
            break;
        }
        let tag = b[i];
        i += 1;
        if tag == 0x3b {
            break;
        }
        if tag == 0x21 {
            if i >= b.len {
                break;
            }
            let label = b[i];
            i += 1;
            if (label == 0xf9) and ((i + 5) < b.len) and (b[i] == 4) {
                let p = b[i + 1];
                disposal = (p >> 2) & 7;
                delay = @as(u32, @truncate(u16, le16(b, i + 2))) * 10;
                transparent = -1;
                if (p & 1) != 0 {
                    transparent = @as(i32, b[i + 4]);
                }
                i += 5;
                skip_blocks(b, &mut i) catch break;
            } else if (label == 0xff) and ((i + 12) <= b.len) and (b[i] == 11) and
                      (strs.eq(b[i + 1..i + 12], "NETSCAPE2.0") or strs.eq(b[i + 1..i + 12], "ANIMEXTS1.0")) {
                i += 12;
                let mut ext = strs.String.init();
                sub_blocks(b, &mut i, &mut ext) catch {
                    ext.deinit();
                    break;
                };
                let e = ext.bytes();
                if (e.len >= 3) and (e[0] == 1) {
                    anim.loops = @as(i32, @truncate(u16, le16(e, 1)));
                }
                ext.deinit();
            } else {
                skip_blocks(b, &mut i) catch break;
            }
            continue;
        }
        if tag != 0x2c {
            break;  // (unknown data ends the file, as viewers treat it)
        }
        if (i + 9) > b.len {
            break;
        }
        let fx = le16(b, i);
        let fy = le16(b, i + 2);
        let fw = le16(b, i + 4);
        let fh = le16(b, i + 6);
        let fflags = b[i + 8];
        i += 9;
        let mut ct = gct;
        if (fflags & 0x80) != 0 {
            let n = 3 * (@as(usize, 2) << @as(usize, fflags & 7));
            if (i + n) > b.len {
                break;
            }
            ct = b[i..i + n];
            i += n;
        }
        if i >= b.len {
            break;
        }
        let min_code = b[i];
        i += 1;
        lzw.clear();
        sub_blocks(b, &mut i, &mut lzw) catch break;
        work += fw * fh;
        if (work > (image.MAX_PIXELS * 4)) or ((fw * fh) > (w * h * 4) + 65536) {
            anim.deinit();
            return error.TooLarge;
        }
        // the previous frame's disposal
        if anim.frames.len > 0 {
            if prev_disposal == 2 {
                clear_rect(&mut canvas, prev);
            } else if prev_disposal == 3 {
                @mem_copy(canvas.data, saved.data);
            }
        }
        if disposal == 3 {
            @mem_copy(saved.data, canvas.data);
        }
        // the indices, then onto the canvas
        let n = fw * fh;
        let idx = mem.alloc.[u8](@max(n, 1)) catch |e| {
            anim.deinit();
            return e;
        };
        @mem_set(idx, 0);
        let got = lzw_decode(lzw.bytes(), min_code, idx[..n]) catch 0;
        let interlaced = (fflags & 0x40) != 0;
        let ncolors = ct.len / 3;
        for k in @as(usize, 0)..@min(got, n) {
            let mut row = k / @max(fw, 1);
            let col = k - (row * fw);
            if interlaced {
                row = interlaced_row(row, fh);
            }
            let x = fx + col;
            let y = fy + row;
            if (x >= w) or (y >= h) {
                continue;
            }
            let c = idx[k];
            if (@as(i32, c) == transparent) or (@as(usize, c) >= ncolors) {
                continue;
            }
            let o = ((y * w) + x) * 4;
            canvas.data[o] = ct[3 * @as(usize, c)];
            canvas.data[o + 1] = ct[(3 * @as(usize, c)) + 1];
            canvas.data[o + 2] = ct[(3 * @as(usize, c)) + 2];
            canvas.data[o + 3] = 255;
        }
        mem.free(idx);
        let copy = canvas.clone() catch |e| {
            anim.deinit();
            return e;
        };
        anim.frames.push(Frame.{ image: copy, delay_ms: delay }) catch |e| {
            let mut c2 = copy;
            c2.deinit();
            anim.deinit();
            return e;
        };
        prev_disposal = disposal;
        prev = [fx, fy, fw, fh];
        disposal = 0;
        delay = 0;
        transparent = -1;
        if first_only {
            break;
        }
    }
    if anim.frames.len == 0 {
        anim.deinit();
        return error.InvalidImage;
    }
    return anim;
}

fn clear_rect(img: *mut image.Image, r: [4]usize) void {
    for y in r[1]..@min(r[1] + r[3], img.height) {
        for x in r[0]..@min(r[0] + r[2], img.width) {
            let o = ((y * img.width) + x) * 4;
            img.data[o] = 0;
            img.data[o + 1] = 0;
            img.data[o + 2] = 0;
            img.data[o + 3] = 0;
        }
    }
}

// The row an interlaced image's k-th row goes to: rows 0, 8, 16... then 4,
// 12..., then 2, 6..., then 1, 3...
fn interlaced_row(k: usize, h: usize) usize {
    let p1 = (h + 7) / 8;
    if k < p1 {
        return k * 8;
    }
    let p2 = (h + 3) / 8;
    if k < (p1 + p2) {
        return ((k - p1) * 8) + 4;
    }
    let p3 = (h + 1) / 4;
    if k < (p1 + p2 + p3) {
        return ((k - p1 - p2) * 4) + 2;
    }
    return ((k - p1 - p2 - p3) * 2) + 1;
}

// LZW, as GIF has it; returns how many indices it produced (the rest of
// out stays as it was when the data runs short).
fn lzw_decode(data: []u8, min_code: u8, out: []mut u8) !usize {
    if (min_code < 2) or (min_code > 11) {
        return error.InvalidImage;
    }
    let clear = @as(u32, 1) << @as(u32, min_code);
    let eoi = clear + 1;
    // each code: its prefix code and last byte; lengths for copying
    let mut prefix: [4096]u16 = [0; 4096];
    let mut suffix: [4096]u8 = [0; 4096];
    let mut first: [4096]u8 = [0; 4096];
    let mut length: [4096]u16 = [0; 4096];
    for k in @as(u32, 0)..clear {
        suffix[@as(usize, k)] = @truncate(u8, k);
        first[@as(usize, k)] = @truncate(u8, k);
        length[@as(usize, k)] = 1;
    }
    let mut next = eoi + 1;
    let mut size = @as(u32, min_code) + 1;
    let mut old: u32 = 0xffff;
    let mut acc: u32 = 0;
    let mut nbits: u32 = 0;
    let mut i: usize = 0;
    let mut n: usize = 0;
    loop {
        while nbits < size {
            if i >= data.len {
                return n;
            }
            acc |= @as(u32, data[i]) << nbits;
            nbits += 8;
            i += 1;
        }
        let code = acc & ((@as(u32, 1) << size) - 1);
        acc >>= size;
        nbits -= size;
        if code == clear {
            next = eoi + 1;
            size = @as(u32, min_code) + 1;
            old = 0xffff;
            continue;
        }
        if code == eoi {
            return n;
        }
        if old == 0xffff {
            if code >= clear {
                return error.InvalidImage;
            }
            if n < out.len {
                out[n] = @truncate(u8, code);
                n += 1;
            }
            old = code;
            continue;
        }
        let mut c = code;
        let mut extra: u8 = 0;
        let mut add_self = false;
        if code >= next {
            if code != next {
                return error.InvalidImage;
            }
            // the KwKwK case: old's string and its first byte
            c = old;
            extra = first[@as(usize, old)];
            add_self = true;
        }
        // write c's string backwards
        let len = @as(usize, length[@as(usize, c)]);
        let total = len + b2u(add_self);
        if (n + total) > out.len {
            return n;
        }
        let mut k = c;
        let mut at = n + len;
        while at > n {
            at -= 1;
            out[at] = suffix[@as(usize, k)];
            k = @as(u32, prefix[@as(usize, k)]);
        }
        if add_self {
            out[n + len] = extra;
        }
        n += total;
        if next < 4096 {
            prefix[@as(usize, next)] = @truncate(u16, old);
            suffix[@as(usize, next)] = first[@as(usize, c)];
            first[@as(usize, next)] = first[@as(usize, old)];
            length[@as(usize, next)] = length[@as(usize, old)] + 1;
            next += 1;
            if (next == (@as(u32, 1) << size)) and (size < 12) {
                size += 1;
            }
        }
        old = code;
    }
}

fn b2u(b: bool) usize {
    if b {
        return 1;
    }
    return 0;
}

// -----------------------------------------------------------------------------
// Encoding
// -----------------------------------------------------------------------------

/// Encodes an image (any channels and depth; alpha below half is
/// transparent). The caller frees the result.
pub fn encode(img: *image.Image) ![]mut u8 {
    let mut s = strs.String.init();
    encode_to(s.writer(), img) catch |e| {
        s.deinit();
        return e;
    };
    return s.take();
}

/// Encodes an image as encode does, to w (as the LZW codes are made).
pub fn encode_to(w: io.Writer, img: *image.Image) !void {
    let one: [1]*image.Image = [img];
    let d: [1]u32 = [0];
    encode_frames(w, one[..], d[..], -1)!;
}

/// Encodes frames (all the same size) with their delays in milliseconds;
/// loops: 0 for ever, n more times, or -1 to play once. The caller frees
/// the result.
pub fn encode_animation(frames: []*image.Image, delays_ms: []u32, loops: i32) ![]mut u8 {
    let mut s = strs.String.init();
    encode_animation_to(s.writer(), frames, delays_ms, loops) catch |e| {
        s.deinit();
        return e;
    };
    return s.take();
}

/// Encodes an animation as encode_animation does, to w, a frame at a time.
pub fn encode_animation_to(w: io.Writer, frames: []*image.Image, delays_ms: []u32, loops: i32) !void {
    if (frames.len == 0) or (delays_ms.len != frames.len) {
        return error.InvalidImage;
    }
    encode_frames(w, frames, delays_ms, loops)!;
}

fn put16(s: *mut strs.String, v: usize) !void {
    s.push(@truncate(u8, v))!;
    s.push(@truncate(u8, v >> 8))!;
}

const FLUSH_AT: usize = 16384;  // write out what is gathered at this size

// Image data as sub-blocks of up to 255 bytes, gathered in o and written
// to out a few KiB at a time.
struct SubBlocks {
    out: io.Writer,
    o: *mut strs.String,
    blk: [255]u8,
    n: usize,

    fn push(self: *mut SubBlocks, b: u8) !void {
        self.blk[self.n] = b;
        self.n += 1;
        if self.n == 255 {
            self.end_block()!;
        }
    }

    fn end_block(self: *mut SubBlocks) !void {
        if self.n == 0 {
            return;
        }
        self.o.push(@truncate(u8, self.n))!;
        self.o.push_str(self.blk[..self.n])!;
        self.n = 0;
        if self.o.len >= FLUSH_AT {
            self.out.write_all(self.o.bytes())!;
            self.o.clear();
        }
    }
}

fn encode_frames(wr: io.Writer, frames: []*image.Image, delays: []u32, loops: i32) !void {
    let w = frames[0].width;
    let h = frames[0].height;
    if (w > 65535) or (h > 65535) or (w == 0) or (h == 0) {
        return error.TooLarge;
    }
    let mut out = strs.String.init();
    defer out.deinit();
    out.push_str("GIF89a")!;
    put16(&mut out, w)!;
    put16(&mut out, h)!;
    out.push(0x70)!;  // no global table; 8 bits of colour resolution
    out.push(0)!;
    out.push(0)!;
    if (loops >= 0) and (frames.len > 1) {
        out.push_str("\x21\xff\x0bNETSCAPE2.0\x03\x01")!;
        put16(&mut out, @as(usize, @bit_cast(u32, loops)))!;
        out.push(0)!;
    }
    for k in @as(usize, 0)..frames.len {
        let f = frames[k];
        if (f.width != w) or (f.height != h) {
            return error.InvalidImage;
        }
        let mut rgba = f.convert(4, image.Depth.U8)!;
        defer rgba.deinit();
        let mut pal = Palette.{ colors: [0; 768], n: 0, transparent: false };
        let idx = mem.alloc.[u8](w * h)!;
        defer mem.free(idx);
        index_image(&rgba, &mut pal, idx)!;
        // graphic control: delay, disposal "none", the transparent index
        out.push_str("\x21\xf9\x04")!;
        let mut packed: u8 = 4;  // disposal 1: leave in place
        if pal.transparent {
            packed |= 1;
        }
        out.push(packed)!;
        put16(&mut out, @as(usize, (delays[k] + 5) / 10))!;
        out.push(0)!;  // the transparent index is 0
        out.push(0)!;
        // the image, with its own table
        let mut bits: u32 = 1;
        while (@as(usize, 1) << @as(usize, bits)) < @max(pal.n, 2) {
            bits += 1;
        }
        out.push(0x2c)!;
        put16(&mut out, 0)!;
        put16(&mut out, 0)!;
        put16(&mut out, w)!;
        put16(&mut out, h)!;
        out.push(0x80 | @truncate(u8, bits - 1))!;
        for c in @as(usize, 0)..(@as(usize, 1) << @as(usize, bits)) {
            out.push(pal.colors[3 * c])!;
            out.push(pal.colors[(3 * c) + 1])!;
            out.push(pal.colors[(3 * c) + 2])!;
        }
        let min_code = @max(bits, 2);
        out.push(@truncate(u8, min_code))!;
        let mut data = SubBlocks.{ out: wr, o: &mut out, blk: [0; 255], n: 0 };
        lzw_encode(idx, min_code, &mut data)!;
        data.end_block()!;
        out.push(0)!;
    }
    out.push(0x3b)!;
    wr.write_all(out.bytes())!;
}

struct Palette {
    colors: [768]u8,
    n: usize,
    transparent: bool,  // (then entry 0)
}

// The palette and each pixel's index: exact for 256 colours or fewer,
// median cut otherwise.
fn index_image(img: *image.Image, pal: *mut Palette, idx: []mut u8) !void {
    let px = img.data;
    let n = img.width * img.height;
    // the distinct colours, as 24-bit keys (and whether any is transparent)
    let keys = mem.alloc.[u32](n)!;
    defer mem.free(keys);
    let mut m: usize = 0;
    for k in @as(usize, 0)..n {
        if px[(4 * k) + 3] < 128 {
            pal.transparent = true;
            continue;
        }
        keys[m] = ((@as(u32, px[4 * k]) << 16) | (@as(u32, px[(4 * k) + 1]) << 8)) | @as(u32, px[(4 * k) + 2]);
        m += 1;
    }
    let colors = keys[..m];
    vec.sort.[u32](colors, less32);
    let mut distinct: usize = 0;
    for k in @as(usize, 0)..m {
        if (k == 0) or (colors[k] != colors[k - 1]) {
            colors[distinct] = colors[k];
            distinct += 1;
        }
    }
    let room = 256 - b2u(pal.transparent);
    let first = b2u(pal.transparent);
    if distinct <= room {
        for k in @as(usize, 0)..distinct {
            set_color(pal, first + k, colors[k]);
        }
        pal.n = first + distinct;
        for k in @as(usize, 0)..n {
            if px[(4 * k) + 3] < 128 {
                idx[k] = 0;
                continue;
            }
            let key = ((@as(u32, px[4 * k]) << 16) | (@as(u32, px[(4 * k) + 1]) << 8)) | @as(u32, px[(4 * k) + 2]);
            idx[k] = @truncate(u8, first + find32(colors[..distinct], key));
        }
        return;
    }
    // median cut over the distinct colours (each counted once)
    let mut boxes: [256][2]usize = [[0, 0]; 256];
    boxes[0] = [0, distinct];
    let mut nb: usize = 1;
    while nb < room {
        // split the box with the widest channel range
        let mut best: usize = 256;
        let mut best_range: u32 = 0;
        let mut best_ch: u32 = 0;
        for k in @as(usize, 0)..nb {
            if (boxes[k][1] - boxes[k][0]) < 2 {
                continue;
            }
            for ch in @as(u32, 0)..3 {
                let r = channel_range(colors[boxes[k][0]..boxes[k][1]], ch);
                if r > best_range {
                    best_range = r;
                    best = k;
                    best_ch = ch;
                }
            }
        }
        if best == 256 {
            break;
        }
        let bx = boxes[best];
        sort_by_channel(colors[bx[0]..bx[1]], best_ch);
        let mid = bx[0] + ((bx[1] - bx[0]) / 2);
        boxes[best] = [bx[0], mid];
        boxes[nb] = [mid, bx[1]];
        nb += 1;
    }
    for k in @as(usize, 0)..nb {
        // the box's average colour
        let mut sum: [3]u64 = [0; 3];
        let bx = boxes[k];
        for c in colors[bx[0]..bx[1]] {
            sum[0] += @as(u64, c >> 16);
            sum[1] += @as(u64, (c >> 8) & 255);
            sum[2] += @as(u64, c & 255);
        }
        let cnt = @as(u64, @max(bx[1] - bx[0], 1));
        let avg = ((@truncate(u32, sum[0] / cnt) << 16) | (@truncate(u32, sum[1] / cnt) << 8)) | @truncate(
            u32, sum[2] / cnt);
        set_color(pal, first + k, avg);
    }
    pal.n = first + nb;
    // nearest palette entry, cached by 15-bit colour
    let mut cache: [32768]i16 = [-1; 32768];
    for k in @as(usize, 0)..n {
        if px[(4 * k) + 3] < 128 {
            idx[k] = 0;
            continue;
        }
        let r = px[4 * k];
        let g = px[(4 * k) + 1];
        let b = px[(4 * k) + 2];
        let q = ((@as(usize, r >> 3) << 10) | (@as(usize, g >> 3) << 5)) | @as(usize, b >> 3);
        if cache[q] < 0 {
            cache[q] = @truncate(i16, @as(isize, @bit_cast(isize, nearest(pal, first, r, g, b))));
        }
        idx[k] = @truncate(u8, @as(u16, @bit_cast(u16, cache[q])));
    }
}

fn less32(a: u32, b: u32) bool {
    return a < b;
}

fn find32(xs: []u32, key: u32) usize {
    let mut lo: usize = 0;
    let mut hi = xs.len;
    while lo < hi {
        let mid = lo + ((hi - lo) / 2);
        if xs[mid] < key {
            lo = mid + 1;
        } else {
            hi = mid;
        }
    }
    return lo;
}

fn set_color(pal: *mut Palette, i: usize, c: u32) void {
    pal.colors[3 * i] = @truncate(u8, c >> 16);
    pal.colors[(3 * i) + 1] = @truncate(u8, c >> 8);
    pal.colors[(3 * i) + 2] = @truncate(u8, c);
}

fn channel(c: u32, ch: u32) u32 {
    return (c >> (16 - (8 * ch))) & 255;
}

fn channel_range(xs: []u32, ch: u32) u32 {
    let mut lo: u32 = 255;
    let mut hi: u32 = 0;
    for c in xs {
        let v = channel(c, ch);
        lo = @min(lo, v);
        hi = @max(hi, v);
    }
    return hi - lo;
}

// (a counting sort, by one channel)
fn sort_by_channel(xs: []mut u32, ch: u32) void {
    let mut count: [257]usize = [0; 257];
    for c in xs {
        count[@as(usize, channel(c, ch)) + 1] += 1;
    }
    for k in @as(usize, 1)..257 {
        count[k] += count[k - 1];
    }
    let tmp = mem.alloc.[u32](xs.len) catch return;
    defer mem.free(tmp);
    for c in xs {
        let v = @as(usize, channel(c, ch));
        tmp[count[v]] = c;
        count[v] += 1;
    }
    @mem_copy(xs, tmp);
}

fn nearest(pal: *Palette, first: usize, r: u8, g: u8, b: u8) usize {
    let mut best = first;
    let mut best_d: u32 = 0xffffffff;
    for k in first..pal.n {
        let dr = @as(i32, pal.colors[3 * k]) - @as(i32, r);
        let dg = @as(i32, pal.colors[(3 * k) + 1]) - @as(i32, g);
        let db = @as(i32, pal.colors[(3 * k) + 2]) - @as(i32, b);
        let d = @bit_cast(u32, ((dr * dr) + (dg * dg)) + (db * db));
        if d < best_d {
            best_d = d;
            best = k;
        }
    }
    return best;
}

fn lzw_encode(idx: []u8, min_code: u32, out: *mut SubBlocks) !void {
    let clear = @as(u32, 1) << min_code;
    let eoi = clear + 1;
    // the dictionary: (prefix code << 8 | byte) -> code, open addressing
    let mut keys: [8192]u32 = [0xffffffff; 8192];
    let mut vals: [8192]u16 = [0; 8192];
    let mut next = eoi + 1;
    let mut size = min_code + 1;
    let mut acc: u32 = 0;
    let mut nbits: u32 = 0;
    emit(out, &mut acc, &mut nbits, clear, size)!;
    if idx.len == 0 {
        emit(out, &mut acc, &mut nbits, eoi, size)!;
        flush_bits(out, &mut acc, &mut nbits)!;
        return;
    }
    let mut cur = @as(u32, idx[0]);
    for k in @as(usize, 1)..idx.len {
        let c = idx[k];
        let key = (cur << 8) | @as(u32, c);
        let mut slot = @as(usize, (key *% 2654435761) >> 19);
        let mut found = false;
        while keys[slot] != 0xffffffff {
            if keys[slot] == key {
                found = true;
                break;
            }
            slot = (slot + 1) & 8191;
        }
        if found {
            cur = @as(u32, vals[slot]);
            continue;
        }
        emit(out, &mut acc, &mut nbits, cur, size)!;
        if next < 4096 {
            keys[slot] = key;
            vals[slot] = @truncate(u16, next);
            // (the decoder widens its codes one code later than this)
            if next == (@as(u32, 1) << size) {
                size += 1;
            }
            next += 1;
        } else {
            // the table is full: start again
            emit(out, &mut acc, &mut nbits, clear, size)!;
            for j in @as(usize, 0)..8192 {
                keys[j] = 0xffffffff;
            }
            next = eoi + 1;
            size = min_code + 1;
        }
        cur = @as(u32, c);
    }
    emit(out, &mut acc, &mut nbits, cur, size)!;
    emit(out, &mut acc, &mut nbits, eoi, size)!;
    flush_bits(out, &mut acc, &mut nbits)!;
}

fn emit(out: *mut SubBlocks, acc: *mut u32, nbits: *mut u32, code: u32, size: u32) !void {
    acc.* |= code << nbits.*;
    nbits.* += size;
    while nbits.* >= 8 {
        out.push(@truncate(u8, acc.*))!;
        acc.* >>= 8;
        nbits.* -= 8;
    }
}

fn flush_bits(out: *mut SubBlocks, acc: *mut u32, nbits: *mut u32) !void {
    if nbits.* > 0 {
        out.push(@truncate(u8, acc.*))!;
        acc.* = 0;
        nbits.* = 0;
    }
}

// -----------------------------------------------------------------------------
// Tests
// -----------------------------------------------------------------------------

fn same(a: *image.Image, b: *image.Image) bool {
    if (a.width != b.width) or (a.height != b.height) {
        return false;
    }
    let mut x = a.convert(4, image.Depth.U8) catch return false;
    defer x.deinit();
    let mut y = b.convert(4, image.Depth.U8) catch return false;
    defer y.deinit();
    // (fully transparent pixels match whatever their colour)
    for k in @as(usize, 0)..(a.width * a.height) {
        let o = 4 * k;
        if (x.data[o + 3] == 0) and (y.data[o + 3] == 0) {
            continue;
        }
        if not @mem_eq(x.data[o..o + 4], y.data[o..o + 4]) {
            return false;
        }
    }
    return true;
}

test "files made by Pillow" {
    let names: [5][]u8 = ["still", "interlaced", "four", "transparent", "anim"];
    let counts: [5]usize = [1, 1, 1, 1, 5];
    for n in @as(usize, 0)..5 {
        let mut p = strs.String.init();
        defer p.deinit();
        p.push_str("std/graphics/image/testdata/gif/")!;
        p.push_str(names[n])!;
        p.push_str(".gif")!;
        let data = fs.read_file(p.bytes()) catch return;
        defer mem.free(data);
        @expect(is_gif(data));
        let mut a = decode_all(data)!;
        defer a.deinit();
        @expect(a.frames.len == counts[n]);
        for k in @as(usize, 0)..a.frames.len {
            p.clear();
            p.push_str("std/graphics/image/testdata/gif/")!;
            p.push_str(names[n])!;
            p.push('.')!;
            p.push('0' + @truncate(u8, k))!;
            p.push_str(".png")!;
            let want_bytes = fs.read_file(p.bytes())!;
            defer mem.free(want_bytes);
            let mut want = png.decode(want_bytes)!;
            defer want.deinit();
            let mut f = a.frames.get(k).image;
            if not same(&f, &want) {
                @expect_eq(p.bytes(), "");
            }
        }
    }
}

test "round trips" {
    // few colours: exact
    let mut img = image.Image.init(37, 23, 4, image.Depth.U8)!;
    defer img.deinit();
    for y in @as(usize, 0)..23 {
        for x in @as(usize, 0)..37 {
            let o = ((y * 37) + x) * 4;
            img.data[o] = @truncate(u8, (x / 5) * 40);
            img.data[o + 1] = @truncate(u8, (y / 4) * 50);
            img.data[o + 2] = 77;
            img.data[o + 3] = 255;
            if ((x + y) % 11) == 0 {
                img.data[o + 3] = 0;
            }
        }
    }
    let enc = encode(&img)!;
    defer mem.free(enc);
    let mut back = decode(enc)!;
    defer back.deinit();
    @expect(same(&img, &back));
    // many colours: close
    let mut rich = image.Image.init(64, 64, 3, image.Depth.U8)!;
    defer rich.deinit();
    for k in @as(usize, 0)..(64 * 64) {
        rich.data[3 * k] = @truncate(u8, k);
        rich.data[(3 * k) + 1] = @truncate(u8, k >> 4);
        rich.data[(3 * k) + 2] = @truncate(u8, k * 7);
    }
    let e2 = encode(&rich)!;
    defer mem.free(e2);
    let mut b2 = decode(e2)!;
    defer b2.deinit();
    let mut err: u64 = 0;
    for k in @as(usize, 0)..(64 * 64) {
        for c in @as(usize, 0)..3 {
            let d = @as(i32, rich.data[(3 * k) + c]) - @as(i32, b2.data[(4 * k) + c]);
            err += @as(u64, @bit_cast(u32, d * d));
        }
    }
    @expect((err / (64 * 64 * 3)) < 200);
    // an animation
    let mut f2 = img.clone()!;
    defer f2.deinit();
    f2.data[0] = 1;
    let frames: [2]*image.Image = [&img, &f2];
    let delays: [2]u32 = [100, 250];
    let movie = encode_animation(frames[..], delays[..], 0)!;
    defer mem.free(movie);
    let mut a = decode_all(movie)!;
    defer a.deinit();
    @expect((a.frames.len == 2) and (a.loops == 0) and (a.frames.get(1).delay_ms == 250));
    let mut g1 = a.frames.get(1).image;
    @expect(same(&g1, &f2));
    // hostile: truncated, and a huge canvas
    @expect(invalid(decode(movie[..20])));
    @expect(invalid(decode("GIF89a\xff\xff\xff\xff\x00\x00\x00;")));
}

fn invalid(r: !image.Image) bool {
    let mut x = r catch return true;
    x.deinit();
    return false;
}

test "encode_to" {
    let mut a = image.Image.init(120, 90, 3, image.Depth.U8)!;
    defer a.deinit();
    for i in @as(usize, 0)..a.data.len {
        a.data[i] = @truncate(u8, (i * 31) ^ (i / 7));
    }
    let mut b = a.clone()!;
    defer b.deinit();
    b.data[5] = 9;
    let enc = encode(&a)!;
    defer mem.free(enc);
    let mut s = strs.String.init();
    defer s.deinit();
    encode_to(s.writer(), &a)!;
    @expect_eq(s.bytes(), enc[..]);
    let frames: [2]*image.Image = [&a, &b];
    let delays: [2]u32 = [100, 250];
    let movie = encode_animation(frames[..], delays[..], 2)!;
    defer mem.free(movie);
    s.clear();
    encode_animation_to(s.writer(), frames[..], delays[..], 2)!;
    @expect_eq(s.bytes(), movie[..]);
    let mut small: [100]u8 = [0; 100];
    let mut bw = io.BufferWriter.init(small[..]);
    encode_to(bw.writer(), &a) catch |e| {
        @expect_eq(e, error.NoSpaceLeft);
        return;
    };
    @trap();
}
