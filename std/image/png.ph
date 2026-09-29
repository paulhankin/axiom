// =============================================================================
// std/image/png.ph - PNG (ISO/IEC 15948) decoding and encoding.
//
//     let img = png.decode(bytes)!;           // an image.Image
//     let out = png.encode(&img, png.DEFAULT)!;  // mem.free it
//
// Decoding handles every colour type and bit depth, interlacing (Adam7),
// palettes and tRNS transparency, and checks CRCs. Images come out as U8
// (bit depths 1 to 8; palettes expanded to RGB or RGBA) or U16 (depth 16);
// a transparent colour key adds an alpha channel. Colour-space chunks (gAMA,
// cHRM, iCCP) are ignored: samples are taken as sRGB.
//
// Encoding writes U8 images at depth 8 and U16 at depth 16 (F32 images are
// converted to U16), choosing a filter for each row, with an sRGB chunk.
// =============================================================================

import "../color.ph" as color;
import "../deflate.ph" as deflate;
import "../image.ph" as image;
import "../mem.ph" as mem;
import "../str.ph" as strs;

error { InvalidImage, Unsupported, ChecksumMismatch, OutOfMemory }

/// Compression levels (as std/deflate's).
pub const FAST: u32 = 1;
pub const DEFAULT: u32 = 6;
pub const SMALLEST: u32 = 9;

let SIGNATURE: [8]u8 = [0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a];

/// Whether data starts like a PNG file.
pub fn is_png(data: []u8) bool {
    return (data.len >= 8) and @mem_eq(data[..8], SIGNATURE[..]);
}

fn be32(b: []u8, i: usize) u32 {
    return (((@as(u32, b[i]) << 24) | (@as(u32, b[i + 1]) << 16)) | (@as(u32, b[i + 2]) << 8)) | @as(u32, b[i + 3]);
}

// Adam7: the first column and row of each pass, and the steps
let PASS_X0: [7]usize = [0, 4, 0, 2, 0, 1, 0];
let PASS_Y0: [7]usize = [0, 0, 4, 0, 2, 0, 1];
let PASS_DX: [7]usize = [8, 8, 4, 4, 2, 2, 1];
let PASS_DY: [7]usize = [8, 8, 8, 4, 4, 2, 2];

struct Header {
    width: usize,
    height: usize,
    depth: usize,  // bits per sample
    color_type: u8,
    interlaced: bool,
    samples: usize,  // per pixel in the file
}

// -----------------------------------------------------------------------------
// Decoding
// -----------------------------------------------------------------------------

pub fn decode(data: []u8) !image.Image {
    if not is_png(data) {
        return error.InvalidImage;
    }
    let mut pos: usize = 8;
    let mut hdr = Header.{ width: 0, height: 0, depth: 0, color_type: 0, interlaced: false, samples: 0 };
    let mut have_header = false;
    let mut palette: [768]u8 = [0; 768];
    let mut npalette: usize = 0;
    let mut palpha: [256]u8 = [0xff; 256];
    let mut has_trns = false;
    let mut key: [3]u16 = [0, 0, 0];  // the transparent gray or RGB value
    let mut idat = strs.String.init();
    defer idat.deinit();
    loop {
        if (pos + 12) > data.len {
            return error.InvalidImage;  // no IEND
        }
        let len = @as(usize, be32(data, pos));
        if len > (data.len - pos - 12) {
            return error.InvalidImage;
        }
        let kind = data[pos + 4..pos + 8];
        let body = data[pos + 8..pos + 8 + len];
        let crc = be32(data, pos + 8 + len);
        if deflate.crc32(0, data[pos + 4..pos + 8 + len]) != crc {
            return error.ChecksumMismatch;
        }
        pos += len + 12;
        if @mem_eq(kind, "IHDR") {
            if have_header or (len != 13) {
                return error.InvalidImage;
            }
            hdr = parse_header(body)!;
            have_header = true;
        } else if not have_header {
            return error.InvalidImage;
        } else if @mem_eq(kind, "PLTE") {
            if ((len % 3) != 0) or (len > 768) or (len == 0) {
                return error.InvalidImage;
            }
            @mem_copy(palette[..len], body);
            npalette = len / 3;
        } else if @mem_eq(kind, "tRNS") {
            has_trns = true;
            if hdr.color_type == 3 {
                if len > 256 {
                    return error.InvalidImage;
                }
                @mem_copy(palpha[..len], body);
            } else if (hdr.color_type == 0) and (len == 2) {
                key[0] = @truncate(u16, (@as(u32, body[0]) << 8) | @as(u32, body[1]));
            } else if (hdr.color_type == 2) and (len == 6) {
                for i in @as(usize, 0)..3 {
                    key[i] = @truncate(u16, (@as(u32, body[2 * i]) << 8) | @as(u32, body[(2 * i) + 1]));
                }
            } else {
                return error.InvalidImage;
            }
        } else if @mem_eq(kind, "IDAT") {
            idat.push_str(body)!;
        } else if @mem_eq(kind, "IEND") {
            break;
        } else if (kind[0] & 0x20) == 0 {
            return error.Unsupported;  // an unknown critical chunk
        }
    }
    if (hdr.color_type == 3) and (npalette == 0) {
        return error.InvalidImage;
    }
    // the filtered scanlines of all passes
    let mut expected: usize = 0;
    for p in @as(usize, 0)..7 {
        let pd = pass_size(&hdr, p);
        if (pd[0] > 0) and (pd[1] > 0) {
            expected += pd[1] * (1 + row_bytes(&hdr, pd[0]));
        }
        if not hdr.interlaced {
            break;
        }
    }
    let raw = deflate.decompress(idat.bytes(), deflate.ZLIB, expected) catch |e| {
        if e == error.TooLarge {
            return error.InvalidImage;
        }
        return e;
    };
    defer mem.free(raw);
    if raw.len < expected {
        return error.InvalidImage;
    }
    // output layout
    let mut channels = hdr.samples;
    if hdr.color_type == 3 {
        channels = 3;
    }
    if has_trns and ((hdr.color_type == 0) or (hdr.color_type == 2) or (hdr.color_type == 3)) {
        channels += 1;
    }
    let mut depth = image.U8;
    if hdr.depth == 16 {
        depth = image.U16;
    }
    let mut img = image.Image.init(hdr.width, hdr.height, channels, depth)!;
    let mut at: usize = 0;
    let mut prev = mem.alloc_zeroed.[u8](row_bytes(&hdr, hdr.width) + 1) catch |e| {
        img.deinit();
        return e;
    };
    defer mem.free(prev);
    let ctx = Expand.{
        hdr: &hdr,
        palette: palette[..npalette * 3],
        palpha: palpha[..],
        has_trns: has_trns,
        key: key,
        channels: channels,
    };
    for p in @as(usize, 0)..7 {
        let pd = pass_size(&hdr, p);
        let pw = pd[0];
        let ph = pd[1];
        if (pw > 0) and (ph > 0) {
            let rb = row_bytes(&hdr, pw);
            let bpp = @max((hdr.depth * hdr.samples) / 8, 1);
            @mem_set(prev[..rb], 0);
            for r in @as(usize, 0)..ph {
                let row = raw[at + 1..at + 1 + rb];
                unfilter(raw[at], row, prev[..rb], bpp) catch |e| {
                    img.deinit();
                    return e;
                };
                @mem_copy(prev[..rb], row);
                let mut x0: usize = 0;
                let mut dx: usize = 1;
                let mut y: usize = r;
                if hdr.interlaced {
                    x0 = PASS_X0[p];
                    dx = PASS_DX[p];
                    y = PASS_Y0[p] + (r * PASS_DY[p]);
                }
                expand_row(&ctx, row, pw, &mut img, x0, dx, y);
                at += rb + 1;
            }
        }
        if not hdr.interlaced {
            break;
        }
    }
    return img;
}

fn parse_header(b: []u8) !Header {
    let w = @as(usize, be32(b, 0));
    let h = @as(usize, be32(b, 4));
    let depth = @as(usize, b[8]);
    let ct = b[9];
    if (w == 0) or (h == 0) or (b[10] != 0) or (b[11] != 0) or (b[12] > 1) {
        return error.InvalidImage;
    }
    if (@mul_checked(w, h) catch return error.InvalidImage) > image.MAX_PIXELS {
        return error.InvalidImage;
    }
    let mut samples: usize = 0;
    let mut ok = false;
    if ct == 0 {
        samples = 1;
        ok = (depth == 1) or (depth == 2) or (depth == 4) or (depth == 8) or (depth == 16);
    } else if ct == 2 {
        samples = 3;
        ok = (depth == 8) or (depth == 16);
    } else if ct == 3 {
        samples = 1;
        ok = (depth == 1) or (depth == 2) or (depth == 4) or (depth == 8);
    } else if ct == 4 {
        samples = 2;
        ok = (depth == 8) or (depth == 16);
    } else if ct == 6 {
        samples = 4;
        ok = (depth == 8) or (depth == 16);
    }
    if not ok {
        return error.InvalidImage;
    }
    return Header.{ width: w, height: h, depth: depth, color_type: ct, interlaced: b[12] == 1, samples: samples };
}

/// The width and height of a pass (the whole image if not interlaced).
fn pass_size(h: *Header, p: usize) [2]usize {
    if not h.interlaced {
        return [h.width, h.height];
    }
    let w = (h.width + PASS_DX[p] - 1 - PASS_X0[p]) / PASS_DX[p];
    let ht = (h.height + PASS_DY[p] - 1 - PASS_Y0[p]) / PASS_DY[p];
    if (h.width <= PASS_X0[p]) or (h.height <= PASS_Y0[p]) {
        return [0, 0];
    }
    return [w, ht];
}

fn row_bytes(h: *Header, w: usize) usize {
    return ((w * h.samples * h.depth) + 7) / 8;
}

fn paeth(a: u8, b: u8, c: u8) u8 {
    let p = (@as(i32, a) + @as(i32, b)) - @as(i32, c);
    let pa = @abs(p - @as(i32, a));
    let pb = @abs(p - @as(i32, b));
    let pc = @abs(p - @as(i32, c));
    if (pa <= pb) and (pa <= pc) {
        return a;
    }
    if pb <= pc {
        return b;
    }
    return c;
}

fn unfilter(f: u8, row: []mut u8, prev: []u8, bpp: usize) !void {
    let n = row.len;
    if f == 0 {
        return;
    }
    if f == 1 {
        for i in bpp..n {
            row[i] = row[i] +% row[i - bpp];
        }
    } else if f == 2 {
        for i in @as(usize, 0)..n {
            row[i] = row[i] +% prev[i];
        }
    } else if f == 3 {
        for i in @as(usize, 0)..n {
            let mut left: u32 = 0;
            if i >= bpp {
                left = @as(u32, row[i - bpp]);
            }
            row[i] = row[i] +% @truncate(u8, (left + @as(u32, prev[i])) / 2);
        }
    } else if f == 4 {
        for i in @as(usize, 0)..n {
            let mut left: u8 = 0;
            let mut ul: u8 = 0;
            if i >= bpp {
                left = row[i - bpp];
                ul = prev[i - bpp];
            }
            row[i] = row[i] +% paeth(left, prev[i], ul);
        }
    } else {
        return error.InvalidImage;
    }
}

struct Expand {
    hdr: *Header,
    palette: []u8,
    palpha: []u8,
    has_trns: bool,
    key: [3]u16,
    channels: usize,
}

/// Sample i of a row of depth < 8.
fn small_sample(row: []u8, i: usize, depth: usize) u32 {
    let bit = i * depth;
    let byte = row[bit / 8];
    let shift = @truncate(u32, 8 - depth - (bit % 8));
    return (@as(u32, byte) >> shift) & ((@as(u32, 1) << @truncate(u32, depth)) - 1);
}

/// Writes w pixels of an unfiltered row into the image at (x0 + k dx, y).
fn expand_row(e: *Expand, row: []u8, w: usize, img: *mut image.Image, x0: usize, dx: usize, y: usize) void {
    let h = e.hdr;
    let ch = e.channels;
    if h.depth == 16 {
        let out = img.samples16();
        for k in @as(usize, 0)..w {
            let o = ((y * img.width) + x0 + (k * dx)) * ch;
            let mut is_key = true;
            for s in @as(usize, 0)..h.samples {
                let i = ((k * h.samples) + s) * 2;
                let v = @truncate(u16, (@as(u32, row[i]) << 8) | @as(u32, row[i + 1]));
                out[o + s] = v;
                if (s >= 3) or (v != e.key[s]) {
                    is_key = false;
                }
            }
            if ch > h.samples {
                out[o + h.samples] = 0xffff;
                if is_key {
                    out[o + h.samples] = 0;
                }
            }
        }
        return;
    }
    let out = img.samples8();
    for k in @as(usize, 0)..w {
        let o = ((y * img.width) + x0 + (k * dx)) * ch;
        if h.color_type == 3 {
            let mut idx: usize = 0;
            if h.depth == 8 {
                idx = @as(usize, row[k]);
            } else {
                idx = @as(usize, small_sample(row, k, h.depth));
            }
            if (idx * 3) < e.palette.len {
                out[o] = e.palette[idx * 3];
                out[o + 1] = e.palette[(idx * 3) + 1];
                out[o + 2] = e.palette[(idx * 3) + 2];
            }
            if ch == 4 {
                out[o + 3] = e.palpha[idx];
            }
            continue;
        }
        if h.depth < 8 {
            let v = small_sample(row, k, h.depth);
            let scale = 255 / ((@as(u32, 1) << @truncate(u32, h.depth)) - 1);
            out[o] = @truncate(u8, v * scale);
            if ch == 2 {
                out[o + 1] = 0xff;
                if v == @as(u32, e.key[0]) {
                    out[o + 1] = 0;
                }
            }
            continue;
        }
        let mut is_key = true;
        for s in @as(usize, 0)..h.samples {
            let v = row[(k * h.samples) + s];
            out[o + s] = v;
            if (s >= 3) or (@as(u16, v) != e.key[s]) {
                is_key = false;
            }
        }
        if ch > h.samples {
            out[o + h.samples] = 0xff;
            if is_key {
                out[o + h.samples] = 0;
            }
        }
    }
}

// -----------------------------------------------------------------------------
// Encoding
// -----------------------------------------------------------------------------

/// Encodes an image as PNG (free the result with mem.free).
pub fn encode(img: *image.Image, level: u32) ![]mut u8 {
    if img.depth == image.F32 {
        let mut c = img.convert(img.channels, image.U16)!;
        defer c.deinit();
        return encode(&c, level);
    }
    let mut ct: u8 = 0;
    if img.channels == 2 {
        ct = 4;
    } else if img.channels == 3 {
        ct = 2;
    } else if img.channels == 4 {
        ct = 6;
    }
    let bps = image.bytes_per_sample(img.depth);
    let bpp = img.channels * bps;
    let rb = img.width * bpp;
    // the filtered scanlines
    let mut raw = strs.String.init();
    defer raw.deinit();
    raw.reserve(img.height * (rb + 1))!;
    let cur = mem.alloc.[u8](rb)!;
    defer mem.free(cur);
    let prev = mem.alloc_zeroed.[u8](rb)!;
    defer mem.free(prev);
    let trial = mem.alloc.[u8](rb)!;
    defer mem.free(trial);
    let best = mem.alloc.[u8](rb)!;
    defer mem.free(best);
    for y in @as(usize, 0)..img.height {
        // big-endian samples
        let src = img.data[y * rb..(y + 1) * rb];
        if bps == 2 {
            let mut i: usize = 0;
            while i < rb {
                cur[i] = src[i + 1];
                cur[i + 1] = src[i];
                i += 2;
            }
        } else {
            @mem_copy(cur, src);
        }
        // the filter with the smallest sum of absolute (signed) values
        let mut best_f: u8 = 0;
        let mut best_cost: u64 = 0xffffffffffffffff;
        let mut last: u8 = 4;
        if level == 0 {
            last = 0;
        }
        for f in @as(u8, 0)..last + 1 {
            let cost = filter_row(f, cur, prev, trial, bpp);
            if cost < best_cost {
                best_cost = cost;
                best_f = f;
                @mem_copy(best, trial);
            }
        }
        raw.push(best_f)!;
        raw.push_str(best)!;
        @mem_copy(prev, cur);
    }
    let z = deflate.compress(raw.bytes(), deflate.ZLIB, level)!;
    defer mem.free(z);
    let mut out = strs.String.init();
    out.push_str(SIGNATURE[..]) catch |e| {
        out.deinit();
        return e;
    };
    let mut ihdr: [13]u8 = [0; 13];
    put_be32(ihdr[0..4], @truncate(u32, img.width));
    put_be32(ihdr[4..8], @truncate(u32, img.height));
    ihdr[8] = @truncate(u8, bps * 8);
    ihdr[9] = ct;
    write_chunk(&mut out, "IHDR", ihdr[..]) catch |e| {
        out.deinit();
        return e;
    };
    let srgb: [1]u8 = [0];  // perceptual rendering intent
    write_chunk(&mut out, "sRGB", srgb[..]) catch |e| {
        out.deinit();
        return e;
    };
    // IDAT chunks of at most 1 MiB
    let mut rest = z[..];
    while rest.len > 0 {
        let n = @min(rest.len, 1048576);
        write_chunk(&mut out, "IDAT", rest[..n]) catch |e| {
            out.deinit();
            return e;
        };
        rest = rest[n..];
    }
    write_chunk(&mut out, "IEND", "") catch |e| {
        out.deinit();
        return e;
    };
    return out.take();
}

fn put_be32(b: []mut u8, v: u32) void {
    b[0] = @truncate(u8, v >> 24);
    b[1] = @truncate(u8, v >> 16);
    b[2] = @truncate(u8, v >> 8);
    b[3] = @truncate(u8, v);
}

fn write_chunk(out: *mut strs.String, kind: []u8, body: []u8) !void {
    let mut lb: [4]u8 = [0; 4];
    put_be32(lb[..], @truncate(u32, body.len));
    out.push_str(lb[..])!;
    out.push_str(kind)!;
    out.push_str(body)!;
    put_be32(lb[..], deflate.crc32(deflate.crc32(0, kind), body));
    out.push_str(lb[..])!;
}

/// Filters cur (with prev above) into out; returns the heuristic cost.
fn filter_row(f: u8, cur: []u8, prev: []u8, out: []mut u8, bpp: usize) u64 {
    let mut cost: u64 = 0;
    for i in @as(usize, 0)..cur.len {
        let mut left: u8 = 0;
        let mut ul: u8 = 0;
        if i >= bpp {
            left = cur[i - bpp];
            ul = prev[i - bpp];
        }
        let mut v = cur[i];
        if f == 1 {
            v = cur[i] -% left;
        } else if f == 2 {
            v = cur[i] -% prev[i];
        } else if f == 3 {
            v = cur[i] -% @truncate(u8, (@as(u32, left) + @as(u32, prev[i])) / 2);
        } else if f == 4 {
            v = cur[i] -% paeth(left, prev[i], ul);
        }
        out[i] = v;
        let s = @as(i32, @bit_cast(i8, v));
        cost += @as(u64, @bit_cast(u32, @abs(s)));
    }
    return cost;
}

// -----------------------------------------------------------------------------
// Tests
// -----------------------------------------------------------------------------

test "round trips" {
    let kinds: [4]usize = [1, 2, 3, 4];
    for ch in kinds {
        for d in @as(u8, 1)..3 {
            let mut img = image.Image.init(37, 21, ch, d)!;
            defer img.deinit();
            for y in @as(usize, 0)..21 {
                for x in @as(usize, 0)..37 {
                    let v = @float_from_int(f32, (x * 7) + (y * 3)) / 300.0;
                    img.set(x, y, color.Color.rgba(v, 1.0 - v, v * v, 1.0 - (v / 2.0)));
                }
            }
            let enc = encode(&img, DEFAULT)!;
            defer mem.free(enc);
            @expect(is_png(enc));
            let mut back = decode(enc)!;
            defer back.deinit();
            @expect_eq(back.channels, ch);
            @expect_eq(back.depth, d);
            @expect_eq(back.data[..], img.data[..]);
        }
    }
}

test "rejects" {
    let mut img = image.Image.init(4, 4, 3, image.U8)!;
    defer img.deinit();
    let enc = encode(&img, FAST)!;
    defer mem.free(enc);
    let bad = mem.dup(enc[..])!;
    defer mem.free(bad);
    bad[20] ^= 1;  // inside IHDR: its CRC fails
    _ = decode(bad) catch |e| {
        @expect_eq(e, error.ChecksumMismatch);
        _ = decode(enc[..enc.len - 12]) catch return;  // no IEND
        @trap();
    };
    @trap();
}
