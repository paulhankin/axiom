// =============================================================================
// std/graphics/image/qoi.ph - QOI, the "Quite OK Image" format (qoiformat.org).
//
//     let img = qoi.decode(bytes)!;   // U8 RGB or RGBA
//     let out = qoi.encode(&img)!;    // mem.free it
//
// Lossless and simple: about PNG's size at many times the speed. Gray
// images are written as RGB; U16 and F32 as 8 bits.
// =============================================================================

import "../../mem.ph" as mem;
import "../image.ph" as image;

error { InvalidImage, OutOfMemory }

pub fn is_qoi(data: []u8) bool {
    return (data.len >= 14) and @mem_eq(data[..4], "qoif");
}

fn be32(b: []u8, i: usize) u32 {
    return (((@as(u32, b[i]) << 24) | (@as(u32, b[i + 1]) << 16)) | (@as(u32, b[i + 2]) << 8)) | @as(u32, b[i + 3]);
}

fn hash(p: [4]u8) usize {
    return (
        (((@as(usize, p[0]) * 3) + (@as(usize, p[1]) * 5)) + (@as(usize, p[2]) * 7)) + (@as(usize, p[3]) * 11)) % 64;
}

pub fn decode(data: []u8) !image.Image {
    if not is_qoi(data) {
        return error.InvalidImage;
    }
    let w = @as(usize, be32(data, 4));
    let h = @as(usize, be32(data, 8));
    let ch = @as(usize, data[12]);
    if (w == 0) or (h == 0) or ((ch != 3) and (ch != 4)) or (data[13] > 1) {
        return error.InvalidImage;
    }
    if (@mul_checked(w, h) catch return error.InvalidImage) > image.MAX_PIXELS {
        return error.InvalidImage;
    }
    let mut img = image.Image.init(w, h, ch, image.Depth.U8)!;
    let out = img.samples8();
    let mut index: [64][4]u8 = [[0, 0, 0, 0]; 64];
    let mut px: [4]u8 = [0, 0, 0, 255];
    let mut i: usize = 14;
    let mut run: usize = 0;
    let end = data.len - @min(data.len, 8);  // the padding
    for p in @as(usize, 0)..w * h {
        if run > 0 {
            run -= 1;
        } else if i < end {
            let b = data[i];
            i += 1;
            if b == 0xfe {
                if (i + 3) > data.len {
                    break;
                }
                px[0] = data[i];
                px[1] = data[i + 1];
                px[2] = data[i + 2];
                i += 3;
            } else if b == 0xff {
                if (i + 4) > data.len {
                    break;
                }
                px = [data[i], data[i + 1], data[i + 2], data[i + 3]];
                i += 4;
            } else if (b >> 6) == 0 {
                px = index[@as(usize, b)];
            } else if (b >> 6) == 1 {
                px[0] = px[0] +% (((b >> 4) & 3) -% 2);
                px[1] = px[1] +% (((b >> 2) & 3) -% 2);
                px[2] = px[2] +% ((b & 3) -% 2);
            } else if (b >> 6) == 2 {
                if i >= data.len {
                    break;
                }
                let b2 = data[i];
                i += 1;
                let dg = (b & 63) -% 32;
                px[0] = px[0] +% ((dg -% 8) +% (b2 >> 4));
                px[1] = px[1] +% dg;
                px[2] = px[2] +% ((dg -% 8) +% (b2 & 15));
            } else {
                run = @as(usize, b & 63);
            }
            index[hash(px)] = px;
        }
        for c in @as(usize, 0)..ch {
            out[(p * ch) + c] = px[c];
        }
    }
    return img;
}

pub fn encode(img: *image.Image) ![]mut u8 {
    if (img.depth != image.Depth.U8) or (img.channels < 3) {
        let mut ch: usize = 3;
        if img.has_alpha() {
            ch = 4;
        }
        let mut c = img.convert(ch, image.Depth.U8)!;
        defer c.deinit();
        return encode(&c);
    }
    let w = img.width;
    let h = img.height;
    let ch = img.channels;
    let npx = w * h;
    let out = mem.alloc.[u8](((npx * (ch + 1)) + 14) + 8)!;
    let hdr: [14]u8 = [
        'q',
        'o',
        'i',
        'f',
        @truncate(u8, w >> 24),
        @truncate(u8, w >> 16),
        @truncate(u8, w >> 8),
        @truncate(u8, w),
        @truncate(u8, h >> 24),
        @truncate(u8, h >> 16),
        @truncate(u8, h >> 8),
        @truncate(u8, h),
        @truncate(u8, ch),
        0,
    ];
    @mem_copy(out[..14], hdr[..]);
    let mut o: usize = 14;
    let mut index: [64][4]u8 = [[0, 0, 0, 0]; 64];
    let mut prev: [4]u8 = [0, 0, 0, 255];
    let mut run: u8 = 0;
    let src = img.samples8();
    for p in @as(usize, 0)..npx {
        let mut px: [4]u8 = [src[p * ch], src[(p * ch) + 1], src[(p * ch) + 2], 255];
        if ch == 4 {
            px[3] = src[(p * ch) + 3];
        }
        if @mem_eq(px[..], prev[..]) {
            run += 1;
            if (run == 62) or (p == (npx - 1)) {
                out[o] = 0xc0 | (run - 1);
                o += 1;
                run = 0;
            }
            continue;
        }
        if run > 0 {
            out[o] = 0xc0 | (run - 1);
            o += 1;
            run = 0;
        }
        let hi = hash(px);
        if @mem_eq(index[hi][..], px[..]) {
            out[o] = @truncate(u8, hi);
            o += 1;
        } else {
            index[hi] = px;
            if px[3] == prev[3] {
                let dr = @as(i32, @bit_cast(i8, px[0] -% prev[0]));
                let dg = @as(i32, @bit_cast(i8, px[1] -% prev[1]));
                let db = @as(i32, @bit_cast(i8, px[2] -% prev[2]));
                let dr_dg = dr - dg;
                let db_dg = db - dg;
                if (dr > -3) and (dr < 2) and (dg > -3) and (dg < 2) and (db > -3) and (db < 2) {
                    out[o] = @truncate(u8, @bit_cast(u32, ((0x40 | ((dr + 2) << 4)) | ((dg + 2) << 2)) | (db + 2)));
                    o += 1;
                } else if (dg > -33) and (dg < 32) and (dr_dg > -9) and (dr_dg < 8) and (db_dg > -9) and (db_dg < 8) {
                    out[o] = @truncate(u8, @bit_cast(u32, 0x80 | (dg + 32)));
                    out[o + 1] = @truncate(u8, @bit_cast(u32, ((dr_dg + 8) << 4) | (db_dg + 8)));
                    o += 2;
                } else {
                    out[o] = 0xfe;
                    out[o + 1] = px[0];
                    out[o + 2] = px[1];
                    out[o + 3] = px[2];
                    o += 4;
                }
            } else {
                out[o] = 0xff;
                @mem_copy(out[o + 1..o + 5], px[..]);
                o += 5;
            }
        }
        prev = px;
    }
    let pad: [8]u8 = [0, 0, 0, 0, 0, 0, 0, 1];
    @mem_copy(out[o..o + 8], pad[..]);
    o += 8;
    return mem.realloc(out, o) catch |e| {
        mem.free(out);
        return e;
    };
}

test "round trips" {
    let chans: [2]usize = [3, 4];
    for ch in chans {
        let mut img = image.Image.init(40, 30, ch, image.Depth.U8)!;
        defer img.deinit();
        let px = img.samples8();
        for i in @as(usize, 0)..px.len {
            // runs, small steps and jumps
            px[i] = @truncate(u8, ((i / 97) * 13) + ((i % ch) * ((i / 500) % 3)));
        }
        let enc = encode(&img)!;
        defer mem.free(enc);
        let mut back = decode(enc)!;
        defer back.deinit();
        @expect_eq(back.data[..], img.data[..]);
    }
}
