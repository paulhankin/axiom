// =============================================================================
// std/graphics/image/pnm.ph - the Netpbm formats: PBM, PGM, PPM (P1 to P6), PAM (P7)
// and PFM (floating point, HDR).
//
//     let img = pnm.decode(bytes)!;
//     let out = pnm.encode(&img)!;   // mem.free it
//
// Decoding reads every variant, ASCII and binary, with any maximum value
// (scaled to U8, or U16 when above 255); PFM gives F32 (linear) images.
// Encoding writes P5 or P6 for gray and RGB, P7 when there is alpha (8 or
// 16 bits), and PFM for F32 images (little-endian; alpha is dropped).
// =============================================================================

import "../../fmt.ph" as fmt;
import "../../mem.ph" as mem;
import "../../str.ph" as strs;
import "../image.ph" as image;

error { InvalidImage, Unsupported, OutOfMemory }

pub fn is_pnm(data: []u8) bool {
    return (data.len >= 3) and (data[0] == 'P') and
           (((data[1] >= '1') and (data[1] <= '7')) or (data[1] == 'F') or (data[1] == 'f'));
}

struct Scan {
    d: []u8,
    pos: usize,

    fn skip_space(self: *mut Scan) void {
        while self.pos < self.d.len {
            let c = self.d[self.pos];
            if c == '#' {
                while (self.pos < self.d.len) and (self.d[self.pos] != '\n') {
                    self.pos += 1;
                }
            } else if (c == ' ') or (c == '\t') or (c == '\n') or (c == '\r') or (c == 11) or (c == 12) {
                self.pos += 1;
            } else {
                return;
            }
        }
    }

    fn token(self: *mut Scan) ![]u8 {
        self.skip_space();
        let start = self.pos;
        while (self.pos < self.d.len) and not is_space(self.d[self.pos]) and (self.d[self.pos] != '#') {
            self.pos += 1;
        }
        if self.pos == start {
            return error.InvalidImage;
        }
        return self.d[start..self.pos];
    }

    fn number(self: *mut Scan) !usize {
        let t = self.token()!;
        return fmt.parse_int.[usize](t) catch return error.InvalidImage;
    }

    /// One ASCII bit (P1 allows them unseparated).
    fn bit(self: *mut Scan) !u32 {
        self.skip_space();
        if self.pos >= self.d.len {
            return error.InvalidImage;
        }
        let c = self.d[self.pos];
        self.pos += 1;
        if (c != '0') and (c != '1') {
            return error.InvalidImage;
        }
        return @as(u32, c - '0');
    }
}

fn is_space(c: u8) bool {
    return (c == ' ') or (c == '\t') or (c == '\n') or (c == '\r') or (c == 11) or (c == 12);
}

pub fn decode(data: []u8) !image.Image {
    if not is_pnm(data) {
        return error.InvalidImage;
    }
    let kind = data[1];
    let mut s = Scan.{ d: data, pos: 2 };
    if (kind == 'F') or (kind == 'f') {
        return decode_pfm(&mut s, kind == 'F');
    }
    let mut w: usize = 0;
    let mut h: usize = 0;
    let mut depth: usize = 1;
    let mut maxval: usize = 1;
    if kind == '7' {
        loop {
            let t = s.token()!;
            if @mem_eq(t, "ENDHDR") {
                break;
            }
            if @mem_eq(t, "TUPLTYPE") {
                _ = s.token()!;
                continue;
            }
            let v = s.number()!;
            if @mem_eq(t, "WIDTH") {
                w = v;
            } else if @mem_eq(t, "HEIGHT") {
                h = v;
            } else if @mem_eq(t, "DEPTH") {
                depth = v;
            } else if @mem_eq(t, "MAXVAL") {
                maxval = v;
            } else {
                return error.InvalidImage;
            }
        }
    } else {
        w = s.number()!;
        h = s.number()!;
        if (kind != '1') and (kind != '4') {
            maxval = s.number()!;
        }
        if (kind == '3') or (kind == '6') {
            depth = 3;
        }
    }
    if (w == 0) or (h == 0) or (maxval == 0) or (maxval > 65535) or (depth < 1) or (depth > 4) {
        return error.InvalidImage;
    }
    if (@mul_checked(w, h) catch return error.InvalidImage) > image.MAX_PIXELS {
        return error.InvalidImage;
    }
    // binary data starts after exactly one whitespace byte
    if (kind >= '4') and (s.pos < data.len) {
        s.pos += 1;
    }
    let mut out_depth = image.Depth.U8;
    if maxval > 255 {
        out_depth = image.Depth.U16;
    }
    let mut img = image.Image.init(w, h, depth, out_depth)!;
    let n = (w * h) * depth;
    let bits = (kind == '1') or (kind == '4');
    let ascii = kind <= '3';
    let wide = maxval > 255;
    for i in @as(usize, 0)..n {
        let mut v: u32 = 0;
        if bits {
            let x = i % w;
            let y = i / w;
            if ascii {
                v = s.bit() catch |e| {
                    img.deinit();
                    return e;
                };
            } else {
                let at = s.pos + (y * ((w + 7) / 8)) + (x / 8);
                if at >= data.len {
                    img.deinit();
                    return error.InvalidImage;
                }
                v = (@as(u32, data[at]) >> @truncate(u32, 7 - (x % 8))) & 1;
            }
            // PBM: 1 is black
            img.data[i] = @truncate(u8, (1 - v) * 255);
            continue;
        }
        if ascii {
            v = @truncate(u32, s.number() catch |e| {
                img.deinit();
                return e;
            });
        } else if wide {
            let at = s.pos + (2 * i);
            if (at + 2) > data.len {
                img.deinit();
                return error.InvalidImage;
            }
            v = (@as(u32, data[at]) << 8) | @as(u32, data[at + 1]);
        } else {
            let at = s.pos + i;
            if at >= data.len {
                img.deinit();
                return error.InvalidImage;
            }
            v = @as(u32, data[at]);
        }
        let m = @truncate(u32, maxval);
        v = @min(v, m);
        if wide {
            img.samples16()[i] = @truncate(u16, ((v * 65535) + (m / 2)) / m);
        } else {
            img.data[i] = @truncate(u8, ((v * 255) + (m / 2)) / m);
        }
    }
    return img;
}

fn decode_pfm(s: *mut Scan, color: bool) !image.Image {
    let w = s.number()!;
    let h = s.number()!;
    let scale_text = s.token()!;
    let scale = fmt.parse_float(scale_text) catch return error.InvalidImage;
    if s.pos < s.d.len {
        s.pos += 1;
    }
    let mut ch: usize = 1;
    if color {
        ch = 3;
    }
    if (w == 0) or (h == 0) or ((@mul_checked(w, h) catch return error.InvalidImage) > image.MAX_PIXELS) {
        return error.InvalidImage;
    }
    let n = (w * h) * ch;
    if (s.pos + (n * 4)) > s.d.len {
        return error.InvalidImage;
    }
    let little = scale < 0.0;
    let mut img = image.Image.init(w, h, ch, image.Depth.F32)!;
    let out = img.samples32();
    // rows are stored bottom to top
    for y in @as(usize, 0)..h {
        let src_row = h - 1 - y;
        for i in @as(usize, 0)..w * ch {
            let at = s.pos + (((src_row * w * ch) + i) * 4);
            let b = s.d[at..at + 4];
            let mut v: u32 = 0;
            if little {
                v = (((@as(u32, b[3]) << 24) | (@as(u32, b[2]) << 16)) | (@as(u32, b[1]) << 8)) | @as(u32, b[0]);
            } else {
                v = (((@as(u32, b[0]) << 24) | (@as(u32, b[1]) << 16)) | (@as(u32, b[2]) << 8)) | @as(u32, b[3]);
            }
            out[(y * w * ch) + i] = @bit_cast(f32, v);
        }
    }
    return img;
}

pub fn encode(img: *image.Image) ![]mut u8 {
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
    let wr = o.writer();
    if img.depth == image.Depth.F32 {
        let mut ch: usize = 3;
        if img.channels <= 2 {
            ch = 1;
        }
        let mut c = img.convert(ch, image.Depth.F32)!;
        defer c.deinit();
        let mut tag = "PF";
        if ch == 1 {
            tag = "Pf";
        }
        fmt.write(wr, "{}\n{} {}\n-1.0\n", [fmt.str(tag), fmt.int(w), fmt.int(h)])!;
        let src = c.samples32();
        for y in @as(usize, 0)..h {
            let row = h - 1 - y;
            for i in @as(usize, 0)..w * ch {
                let v = @bit_cast(u32, src[(row * w * ch) + i]);
                let b: [4]u8 = [
                    @truncate(u8, v), @truncate(u8, v >> 8), @truncate(u8, v >> 16), @truncate(u8, v >> 24)];
                o.push_str(b[..])!;
            }
        }
        return;
    }
    let mut maxval: u32 = 255;
    if img.depth == image.Depth.U16 {
        maxval = 65535;
    }
    if img.has_alpha() {
        let mut tt = "GRAYSCALE_ALPHA";
        if img.channels == 4 {
            tt = "RGB_ALPHA";
        }
        fmt.write(
            wr, "P7\nWIDTH {}\nHEIGHT {}\nDEPTH {}\nMAXVAL {}\nTUPLTYPE {}\nENDHDR\n", [
                fmt.int(w), fmt.int(h), fmt.int(img.channels), fmt.int(maxval), fmt.str(tt)])!;
    } else {
        let mut tag = "P6";
        if img.channels == 1 {
            tag = "P5";
        }
        fmt.write(wr, "{}\n{} {}\n{}\n", [fmt.str(tag), fmt.int(w), fmt.int(h), fmt.int(maxval)])!;
    }
    if img.depth == image.Depth.U8 {
        o.push_str(img.samples8())!;
        return;
    }
    for v in img.samples16() {
        o.push(@truncate(u8, v >> 8))!;
        o.push(@truncate(u8, v))!;
    }
}

test "formats" {
    let mut a = decode("P1\n# a comment\n3 2\n1 0 1\n010")!;
    defer a.deinit();
    @expect_eq(a.data[..6], "\x00\xff\x00\xff\x00\xff");
    let mut b = decode("P2 2 1 10 0 10")!;
    defer b.deinit();
    @expect_eq(b.data[..2], "\x00\xff");
    let mut c = decode("P4\n9 1\n\x80\x80")!;
    defer c.deinit();
    @expect_eq(c.data[0], 0);
    @expect_eq(c.data[8], 0);
    @expect_eq(c.data[1], 255);
    let mut d = decode("P5 1 1 65535\n\x12\x34")!;
    defer d.deinit();
    @expect_eq(d.samples16()[0], 0x1234);
    let kinds: [3]image.Depth = [image.Depth.U8, image.Depth.U16, image.Depth.F32];
    let chans: [4]usize = [1, 2, 3, 4];
    for k in kinds {
        for ch in chans {
            let mut img = image.Image.init(5, 3, ch, k)!;
            defer img.deinit();
            for i in @as(usize, 0)..img.data.len {
                img.data[i] = @truncate(u8, (i * 7) + 1);
            }
            if k == image.Depth.F32 {
                let f = img.samples32();
                for i in @as(usize, 0)..f.len {
                    f[i] = @float_from_int(f32, i) * 0.25;
                }
            }
            let enc = encode(&img)!;
            defer mem.free(enc);
            let mut back = decode(enc)!;
            defer back.deinit();
            if (k == image.Depth.F32) and ((ch == 2) or (ch == 4)) {
                @expect_eq(back.channels, ch - 1);  // PFM has no alpha
                continue;
            }
            @expect_eq(back.channels, ch);
            @expect_eq(back.data[..], img.data[..]);
        }
    }
}
