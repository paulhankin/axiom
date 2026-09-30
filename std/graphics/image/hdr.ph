// =============================================================================
// std/graphics/image/hdr.ph - Radiance RGBE (.hdr, .pic): HDR images.
//
//     let img = hdr.decode(bytes)!;   // F32 RGB, linear
//     let out = hdr.encode(&img)!;    // mem.free it
//
// Each pixel is three 8-bit mantissas sharing an exponent: about 1% relative
// precision over a huge range. Decoding reads flat, old-style and new-style
// run-length encoded scanlines, top-down or bottom-up; encoding writes
// new-style RLE. Images that are not F32 are converted (to linear light).
// =============================================================================

import "../../fmt.ph" as fmt;
import "../../math.ph" as math;
import "../../mem.ph" as mem;
import "../../str.ph" as strs;
import "../image.ph" as image;

error { InvalidImage, Unsupported, OutOfMemory }

pub fn is_hdr(data: []u8) bool {
    return strs.starts_with(data, "#?RADIANCE") or strs.starts_with(data, "#?RGBE");
}

pub fn decode(data: []u8) !image.Image {
    if not is_hdr(data) {
        return error.InvalidImage;
    }
    // header lines up to an empty one
    let mut pos: usize = 0;
    let mut rle_rgbe = true;
    loop {
        let nl = strs.index_of_byte(data[pos..], '\n') catch return error.InvalidImage;
        let line = data[pos..pos + nl];
        pos += nl + 1;
        if line.len == 0 {
            break;
        }
        if strs.starts_with(line, "FORMAT=") and not @mem_eq(line, "FORMAT=32-bit_rle_rgbe") {
            rle_rgbe = false;
        }
    }
    if not rle_rgbe {
        return error.Unsupported;  // XYZE
    }
    let nl = strs.index_of_byte(data[pos..], '\n') catch return error.InvalidImage;
    let res = data[pos..pos + nl];
    pos += nl + 1;
    let mut parts = strs.fields(res);
    let ya = parts.next() catch return error.InvalidImage;
    let yv = parts.next() catch return error.InvalidImage;
    let xa = parts.next() catch return error.InvalidImage;
    let xv = parts.next() catch return error.InvalidImage;
    if not @mem_eq(xa, "+X") or not (@mem_eq(ya, "-Y") or @mem_eq(ya, "+Y")) {
        return error.Unsupported;  // rotated images
    }
    let bottom_up = @mem_eq(ya, "+Y");
    let h = fmt.parse_int.[usize](yv) catch return error.InvalidImage;
    let w = fmt.parse_int.[usize](xv) catch return error.InvalidImage;
    if (w == 0) or (h == 0) or ((@mul_checked(w, h) catch return error.InvalidImage) > image.MAX_PIXELS) {
        return error.InvalidImage;
    }
    let mut img = image.Image.init(w, h, 3, image.F32)!;
    let line = mem.alloc.[u8](w * 4) catch |e| {
        img.deinit();
        return e;
    };
    defer mem.free(line);
    let out = img.samples32();
    for row in @as(usize, 0)..h {
        pos = read_scanline(data, pos, line, w) catch |e| {
            img.deinit();
            return e;
        };
        let mut y = row;
        if bottom_up {
            y = h - 1 - row;
        }
        for x in @as(usize, 0)..w {
            let p = line[x * 4..(x * 4) + 4];
            let o = ((y * w) + x) * 3;
            if p[3] == 0 {
                out[o] = 0.0;
                out[o + 1] = 0.0;
                out[o + 2] = 0.0;
            } else {
                let f = @float_cast(f32, math.ldexp(1.0, @as(i32, p[3]) - (128 + 8)));
                out[o] = @float_from_int(f32, p[0]) * f;
                out[o + 1] = @float_from_int(f32, p[1]) * f;
                out[o + 2] = @float_from_int(f32, p[2]) * f;
            }
        }
    }
    return img;
}

/// One scanline of RGBE pixels into line; returns the new position.
fn read_scanline(data: []u8, start: usize, line: []mut u8, w: usize) !usize {
    let mut pos = start;
    if (pos + 4) > data.len {
        return error.InvalidImage;
    }
    let new_rle = (w >= 8) and (w < 32768) and (data[pos] == 2) and (data[pos + 1] == 2) and
                  ((data[pos + 2] & 0x80) == 0);
    if new_rle {
        if ((@as(usize, data[pos + 2]) << 8) | @as(usize, data[pos + 3])) != w {
            return error.InvalidImage;
        }
        pos += 4;
        // four channels, each run-length encoded separately
        for c in @as(usize, 0)..4 {
            let mut x: usize = 0;
            while x < w {
                if pos >= data.len {
                    return error.InvalidImage;
                }
                let n = @as(usize, data[pos]);
                pos += 1;
                if n > 128 {
                    let count = n - 128;
                    if ((x + count) > w) or (pos >= data.len) {
                        return error.InvalidImage;
                    }
                    let v = data[pos];
                    pos += 1;
                    for k in @as(usize, 0)..count {
                        line[((x + k) * 4) + c] = v;
                    }
                    x += count;
                } else {
                    if (n == 0) or ((x + n) > w) or ((pos + n) > data.len) {
                        return error.InvalidImage;
                    }
                    for k in @as(usize, 0)..n {
                        line[((x + k) * 4) + c] = data[pos + k];
                    }
                    pos += n;
                    x += n;
                }
            }
        }
        return pos;
    }
    // flat pixels, possibly with old-style runs (1, 1, 1, n repeats the last)
    let mut x: usize = 0;
    let mut shift: u32 = 0;
    while x < w {
        if (pos + 4) > data.len {
            return error.InvalidImage;
        }
        let p = data[pos..pos + 4];
        pos += 4;
        if (p[0] == 1) and (p[1] == 1) and (p[2] == 1) {
            if x == 0 {
                return error.InvalidImage;
            }
            let count = @as(usize, p[3]) << shift;
            if (x + count) > w {
                return error.InvalidImage;
            }
            for k in @as(usize, 0)..count {
                @mem_copy(line[(x + k) * 4..((x + k) * 4) + 4], line[(x - 1) * 4..x * 4]);
            }
            x += count;
            shift += 8;
        } else {
            @mem_copy(line[x * 4..(x * 4) + 4], p);
            x += 1;
            shift = 0;
        }
    }
    return pos;
}

/// A linear RGB value as RGBE.
fn rgbe(r: f32, g: f32, b: f32) [4]u8 {
    let v = @max(@max(r, g), b);
    if not (v > 1.0e-32) {
        return [0, 0, 0, 0];
    }
    let fr = math.frexp(@as(f64, v));
    if fr.exponent > 127 {
        return [255, 255, 255, 255];
    }
    let scale = (fr.mantissa * 256.0) / @as(f64, v);
    return [
        byte(@as(f64, r) * scale),
        byte(@as(f64, g) * scale),
        byte(@as(f64, b) * scale),
        @truncate(u8, @bit_cast(u32, fr.exponent + 128)),
    ];
}

fn byte(v: f64) u8 {
    return @truncate(u8, @int_from_float(u32, @min(@max(v, 0.0), 255.0)));
}

pub fn encode(img: *image.Image) ![]mut u8 {
    if (img.depth != image.F32) or (img.channels != 3) {
        let mut c = img.convert(3, image.F32)!;
        defer c.deinit();
        return encode(&c);
    }
    let mut o = strs.String.init();
    write_file(&mut o, img) catch |e| {
        o.deinit();
        return e;
    };
    return o.take();
}

fn write_file(o: *mut strs.String, img: *image.Image) !void {
    let w = img.width;
    let h = img.height;
    fmt.write(o.writer(), "#?RADIANCE\nFORMAT=32-bit_rle_rgbe\n\n-Y {} +X {}\n", [fmt.int(h), fmt.int(w)])!;
    let line = mem.alloc.[u8](w * 4)!;
    defer mem.free(line);
    let px = img.samples32();
    for y in @as(usize, 0)..h {
        for x in @as(usize, 0)..w {
            let i = ((y * w) + x) * 3;
            let e = rgbe(px[i], px[i + 1], px[i + 2]);
            @mem_copy(line[x * 4..(x * 4) + 4], e[..]);
        }
        if (w < 8) or (w >= 32768) {
            o.push_str(line)!;
            continue;
        }
        let hdr: [4]u8 = [2, 2, @truncate(u8, w >> 8), @truncate(u8, w)];
        o.push_str(hdr[..])!;
        for c in @as(usize, 0)..4 {
            write_channel(o, line, c, w)!;
        }
    }
}

/// One channel of a scanline: runs of 3 or more as (128 + n, v), the
/// rest as literal stretches (n, bytes...), n <= 127.
fn write_channel(o: *mut strs.String, line: []u8, c: usize, w: usize) !void {
    let mut x: usize = 0;
    while x < w {
        // find the next run of at least 3
        let mut run_start = x;
        let mut run_len: usize = 0;
        while run_start < w {
            run_len = 1;
            while ((run_start + run_len) < w) and (run_len < 127) and
                  (line[((run_start + run_len) * 4) + c] == line[(run_start * 4) + c]) {
                run_len += 1;
            }
            if run_len >= 3 {
                break;
            }
            run_start += run_len;
        }
        if run_start > w {
            run_start = w;
        }
        // literals before it
        while x < run_start {
            let n = @min(run_start - x, 127);
            o.push(@truncate(u8, n))!;
            for k in @as(usize, 0)..n {
                o.push(line[((x + k) * 4) + c])!;
            }
            x += n;
        }
        if (run_start < w) and (run_len >= 3) {
            o.push(@truncate(u8, 128 + run_len))!;
            o.push(line[(run_start * 4) + c])!;
            x = run_start + run_len;
        }
    }
}

test "round trip" {
    let widths: [3]usize = [5, 40, 300];
    for w in widths {
        let mut img = image.Image.init(w, 3, 3, image.F32)!;
        defer img.deinit();
        let px = img.samples32();
        for i in @as(usize, 0)..px.len {
            let v = @float_from_int(f32, (i / 9) % 7);
            px[i] = (v * v) * 13.5;  // runs, zeros and a wide range
        }
        let enc = encode(&img)!;
        defer mem.free(enc);
        let mut back = decode(enc)!;
        defer back.deinit();
        let bp = back.samples32();
        for i in @as(usize, 0)..px.len {
            @expect(@abs(bp[i] - px[i]) <= (@max(px[i], 0.0) * 0.01));
        }
    }
    // old-style runs and bottom-up rows
    let mut b =
        decode("#?RADIANCE\n\n+Y 2 +X 3\n\x80\x40\x20\x81\x01\x01\x01\x02\x00\x00\x00\x00\x00\x00\x00\x00\x10\x10\x10\x80")!;
    defer b.deinit();
    @expect_eq(b.get(2, 1).r, 1.0);
    @expect_eq(b.get(2, 1).b, 0.25);
    @expect_eq(b.get(2, 0).g, 0.0625);
}
