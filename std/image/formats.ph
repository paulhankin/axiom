// =============================================================================
// std/image/formats.ph - reading and writing images in any supported format.
//
//     let mut img = formats.load("photo.jpg")!;       // the format from the content
//     defer img.deinit();
//     formats.save("photo.png", &img, formats.Options.default())!;  // from the extension
//
//     let f = formats.detect(bytes);                   // PNG, JPEG, ... or UNKNOWN
//     let img2 = formats.decode(bytes)!;
//     let out = formats.encode(&img, formats.PNG, formats.Options.default())!;
//
// Formats: PNG, JPEG, BMP, QOI, PNM (PBM, PGM, PPM, PAM, PFM), Radiance HDR,
// OpenEXR and GIF. HDR and EXR hold linear floating point (F32) images; the
// others 8 or 16 bits per sample. Encoding converts as the format needs.
// =============================================================================

import "../image.ph" as image;
import "../io.ph" as io;
import "../mem.ph" as mem;
import "../str.ph" as strs;
import "bmp.ph" as bmp;
import "exr.ph" as exr;
import "gif.ph" as gif;
import "hdr.ph" as hdr;
import "jpeg.ph" as jpeg;
import "png.ph" as png;
import "pnm.ph" as pnm;
import "qoi.ph" as qoi;

error { Unsupported, InvalidImage, OutOfMemory }

pub const UNKNOWN: u8 = 0;
pub const PNG: u8 = 1;
pub const JPEG: u8 = 2;
pub const BMP: u8 = 3;
pub const QOI: u8 = 4;
pub const PNM: u8 = 5;
pub const HDR: u8 = 6;
pub const EXR: u8 = 7;
pub const GIF: u8 = 8;

/// Settings for encoders (each uses the ones that apply to it).
pub struct Options {
    quality: u32,   // JPEG, 1-100
    subsample: u8,  // JPEG chroma: jpeg.SUBSAMPLE_444, _422 or _420
    level: u32,     // PNG compression, 0-9
    exr: exr.Options,

    pub fn default() Options {
        return Options.{ quality: 90, subsample: jpeg.SUBSAMPLE_420, level: png.DEFAULT, exr: exr.Options.default() };
    }
}

/// The format of encoded data, from its first bytes.
pub fn detect(data: []u8) u8 {
    if png.is_png(data) {
        return PNG;
    }
    if jpeg.is_jpeg(data) {
        return JPEG;
    }
    if exr.is_exr(data) {
        return EXR;
    }
    if gif.is_gif(data) {
        return GIF;
    }
    if qoi.is_qoi(data) {
        return QOI;
    }
    if hdr.is_hdr(data) {
        return HDR;
    }
    if bmp.is_bmp(data) {
        return BMP;
    }
    if pnm.is_pnm(data) {
        return PNM;
    }
    return UNKNOWN;
}

/// The format for a file name's extension (case-insensitive), or UNKNOWN.
pub fn from_extension(path: []u8) u8 {
    let dot = strs.last_index_of_byte(path, '.') catch return UNKNOWN;
    let ext = path[dot + 1..];
    let mut buf: [8]u8 = [0; 8];
    if ext.len > buf.len {
        return UNKNOWN;
    }
    for i in @as(usize, 0)..ext.len {
        buf[i] = strs.to_lower_byte(ext[i]);
    }
    let e = buf[..ext.len];
    if @mem_eq(e, "png") {
        return PNG;
    }
    if @mem_eq(e, "jpg") or @mem_eq(e, "jpeg") or @mem_eq(e, "jpe") or @mem_eq(e, "jfif") {
        return JPEG;
    }
    if @mem_eq(e, "bmp") or @mem_eq(e, "dib") {
        return BMP;
    }
    if @mem_eq(e, "qoi") {
        return QOI;
    }
    if @mem_eq(e, "pbm") or @mem_eq(e, "pgm") or @mem_eq(e, "ppm") or @mem_eq(e, "pnm") or @mem_eq(e, "pam") or
       @mem_eq(e, "pfm") {
        return PNM;
    }
    if @mem_eq(e, "hdr") or @mem_eq(e, "pic") or @mem_eq(e, "rgbe") {
        return HDR;
    }
    if @mem_eq(e, "exr") {
        return EXR;
    }
    if @mem_eq(e, "gif") {
        return GIF;
    }
    return UNKNOWN;
}

/// Decodes an image in any supported format.
pub fn decode(data: []u8) !image.Image {
    let f = detect(data);
    if f == PNG {
        return png.decode(data);
    }
    if f == JPEG {
        return jpeg.decode(data);
    }
    if f == BMP {
        return bmp.decode(data);
    }
    if f == QOI {
        return qoi.decode(data);
    }
    if f == PNM {
        return pnm.decode(data);
    }
    if f == HDR {
        return hdr.decode(data);
    }
    if f == EXR {
        return exr.decode(data);
    }
    if f == GIF {
        return gif.decode(data);
    }
    return error.Unsupported;
}

/// Encodes an image (free the result with mem.free).
pub fn encode(img: *image.Image, format: u8, opts: Options) ![]mut u8 {
    if format == PNG {
        return png.encode(img, opts.level);
    }
    if format == JPEG {
        return jpeg.encode(img, opts.quality, opts.subsample);
    }
    if format == BMP {
        return bmp.encode(img);
    }
    if format == QOI {
        return qoi.encode(img);
    }
    if format == PNM {
        return pnm.encode(img);
    }
    if format == HDR {
        return hdr.encode(img);
    }
    if format == EXR {
        return exr.encode(img, opts.exr);
    }
    if format == GIF {
        return gif.encode(img);
    }
    return error.Unsupported;
}

/// Reads and decodes an image file.
pub fn load(path: []u8) !image.Image {
    let f = io.open(path)!;
    defer f.close();
    let data = f.reader().read_all()!;
    defer mem.free(data);
    return decode(data);
}

/// Encodes an image in the format its file name's extension names, and
/// writes the file.
pub fn save(path: []u8, img: *image.Image, opts: Options) !void {
    let format = from_extension(path);
    if format == UNKNOWN {
        return error.Unsupported;
    }
    let data = encode(img, format, opts)!;
    defer mem.free(data);
    let f = io.create(path)!;
    defer f.close();
    f.writer().write_all(data)!;
}

test "every format" {
    let mut img = image.Image.init(10, 6, 3, image.U8)!;
    defer img.deinit();
    let px = img.samples8();
    for i in @as(usize, 0)..px.len {
        px[i] = @truncate(u8, (i % 30) * 8);
    }
    let formats: [8]u8 = [PNG, JPEG, BMP, QOI, PNM, HDR, EXR, GIF];
    for f in formats {
        let enc = encode(&img, f, Options.default())!;
        defer mem.free(enc);
        @expect_eq(detect(enc), f);
        let mut back = decode(enc)!;
        defer back.deinit();
        @expect_eq(back.width, 10);
        @expect_eq(back.height, 6);
        if (f != JPEG) and (f != HDR) {
            @expect_eq(back.get(3, 2).to_rgba8().to_u32(), img.get(3, 2).to_rgba8().to_u32());
        }
    }
    @expect_eq(from_extension("a/b.JPG"), JPEG);
    @expect_eq(from_extension("x.exr"), EXR);
    @expect_eq(from_extension("noext"), UNKNOWN);
}
