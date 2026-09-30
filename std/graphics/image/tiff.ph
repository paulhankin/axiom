// =============================================================================
// std/graphics/image/tiff.ph - TIFF images (read and write).
//
//     let mut img = tiff.decode(bytes)!;              // the first page
//     let n = tiff.page_count(bytes)!;
//     let mut p = tiff.decode_page(bytes, 2)!;        // the third
//
//     let out = tiff.encode(&img, tiff.Options.default())!;   // mem.free(out)
//     let book = tiff.encode_pages(pages, tiff.Options.default())!;
//
// Decoding reads classic TIFF and BigTIFF in either byte order: strips or
// tiles, contiguous or planar samples; no compression, LZW, Deflate,
// PackBits, CCITT fax (modified Huffman, group 3 in 1D and 2D, group 4) and
// JPEG; horizontal and floating-point predictors; 1 to 16 bits, 32 and 64
// bits, signed or unsigned integers and 16, 32 or 64-bit floats; gray
// (either polarity), RGB, palettes, CMYK and JPEG's YCbCr; associated
// (premultiplied) or unassociated alpha. Images come out as gray, gray and
// alpha, RGB or RGBA: U8 for depths up to 8 bits (scaled up), U16 for 9 to 16
// (and deeper integers, cut to 16 bits), F32 for floats. CMYK becomes RGB,
// premultiplied alpha becomes straight.
//
// Encoding writes a little-endian TIFF with U8 or U16 integers or F32 floats,
// in strips of about 64 KiB, uncompressed or compressed with LZW, Deflate or
// PackBits, with the predictor (horizontal, or floating-point) if asked.
// =============================================================================

import "../../compress/deflate.ph" as deflate;
import "../../fmt.ph" as fmt;
import "../../mem.ph" as mem;
import "../../os/fs.ph" as fs;
import "../../str.ph" as strs;
import "../image.ph" as image;
import "jpeg.ph" as jpeg;

error { InvalidImage, TooLarge, Unsupported, OutOfMemory }

/// Compressions for encoding (their values are TIFF's).
pub enum Compression: u16 {
    NONE = 1,
    LZW = 5,
    DEFLATE = 8,
    PACKBITS = 32773,
}

pub struct Options {
    compression: Compression,
    predictor: bool,  // difference neighbouring samples first (LZW, Deflate)

    pub fn default() Options {
        return Options.{ compression: Compression.DEFLATE, predictor: true };
    }
}

pub fn is_tiff(data: []u8) bool {
    if data.len < 8 {
        return false;
    }
    if (data[0] == 'I') and (data[1] == 'I') {
        return ((data[2] == 42) or (data[2] == 43)) and (data[3] == 0);
    }
    if (data[0] == 'M') and (data[1] == 'M') {
        return (data[2] == 0) and ((data[3] == 42) or (data[3] == 43));
    }
    return false;
}

// -----------------------------------------------------------------------------
// Directories
// -----------------------------------------------------------------------------

struct File {
    data: []u8,
    be: bool,
    big: bool,  // BigTIFF

    // (callers check the bounds)
    fn u16at(self: *File, i: usize) u64 {
        let b = self.data;
        if self.be {
            return (@as(u64, b[i]) << 8) | @as(u64, b[i + 1]);
        }
        return @as(u64, b[i]) | (@as(u64, b[i + 1]) << 8);
    }

    fn u32at(self: *File, i: usize) u64 {
        if self.be {
            return (self.u16at(i) << 16) | self.u16at(i + 2);
        }
        return self.u16at(i) | (self.u16at(i + 2) << 16);
    }

    fn u64at(self: *File, i: usize) u64 {
        if self.be {
            return (self.u32at(i) << 32) | self.u32at(i + 4);
        }
        return self.u32at(i) | (self.u32at(i + 4) << 32);
    }
}

fn open(data: []u8) !File {
    if not is_tiff(data) {
        return error.InvalidImage;
    }
    let f = File.{ data: data, be: data[0] == 'M', big: (data[2] == 43) or (data[3] == 43) };
    if f.big and ((data.len < 16) or (f.u16at(4) != 8) or (f.u16at(6) != 0)) {
        return error.InvalidImage;
    }
    return f;
}

fn first_ifd(f: *File) u64 {
    if f.big {
        return f.u64at(8);
    }
    return f.u32at(4);
}

// a field's values: count of type typ, at a position in the file
struct Entry {
    typ: u64,
    count: u64,
    at: usize,
}

fn no_entry() Entry {
    return Entry.{ typ: 0, count: 0, at: 0 };
}

fn type_size(t: u64) usize {
    if (t == 1) or (t == 2) or (t == 6) or (t == 7) {
        return 1;
    }
    if (t == 3) or (t == 8) {
        return 2;
    }
    if (t == 4) or (t == 9) or (t == 11) or (t == 13) {
        return 4;
    }
    if (t == 5) or (t == 10) or (t == 12) or (t == 16) or (t == 17) or (t == 18) {
        return 8;
    }
    return 0;
}

// value k of an integer field (k < e.count)
fn elem(f: *File, e: Entry, k: u64) u64 {
    let s = type_size(e.typ);
    let i = e.at + (@truncate(usize, k) * s);
    if s == 1 {
        return @as(u64, f.data[i]);
    }
    if s == 2 {
        return f.u16at(i);
    }
    if s == 4 {
        return f.u32at(i);
    }
    return f.u64at(i);
}

fn first_or(f: *File, e: Entry, default: u64) u64 {
    if e.count == 0 {
        return default;
    }
    return elem(f, e, 0);
}

struct Page {
    width: u64,
    height: u64,
    compression: u64,
    photometric: u64,
    planar: u64,
    predictor: u64,
    fill_order: u64,
    rows_per_strip: u64,
    tile_w: u64,
    tile_h: u64,
    spp: u64,
    t4: u64,
    bits: Entry,
    formats: Entry,
    offsets: Entry,
    counts: Entry,
    colormap: Entry,
    tables: Entry,
    extras: Entry,
    next: u64,  // the next directory's offset, or 0
}

fn read_page(f: *File, off: u64) !Page {
    let len = @as(u64, f.data.len);
    let mut csize: u64 = 2;
    let mut esize: u64 = 12;
    let mut lsize: u64 = 4;
    if f.big {
        csize = 8;
        esize = 20;
        lsize = 8;
    }
    if (off < 8) or (off > len) or ((len - off) < csize) {
        return error.InvalidImage;
    }
    let o = @truncate(usize, off);
    let mut n = f.u16at(o);
    if f.big {
        n = f.u64at(o);
    }
    if (n > (len / esize)) or ((len - off) < ((csize + (n * esize)) + lsize)) {
        return error.InvalidImage;
    }
    let mut p = Page.{
        width: 0,
        height: 0,
        compression: 1,
        photometric: 99,
        planar: 1,
        predictor: 1,
        fill_order: 1,
        rows_per_strip: 0,
        tile_w: 0,
        tile_h: 0,
        spp: 1,
        t4: 0,
        bits: no_entry(),
        formats: no_entry(),
        offsets: no_entry(),
        counts: no_entry(),
        colormap: no_entry(),
        tables: no_entry(),
        extras: no_entry(),
        next: 0,
    };
    let start = o + @truncate(usize, csize);
    for k in @as(usize, 0)..@truncate(usize, n) {
        let at = start + (k * @truncate(usize, esize));
        let tag = f.u16at(at);
        let typ = f.u16at(at + 2);
        let mut count = f.u32at(at + 4);
        let mut vpos = at + 8;
        if f.big {
            count = f.u64at(at + 4);
            vpos = at + 12;
        }
        let s = @as(u64, type_size(typ));
        if (s == 0) or (count == 0) or (count > len) {
            continue;
        }
        let total = count * s;
        let mut vat = @as(u64, vpos);
        if total > lsize {
            vat = f.u32at(vpos);
            if f.big {
                vat = f.u64at(vpos);
            }
        }
        if (vat > len) or ((len - vat) < total) {
            continue;  // (a field out of the file is ignored)
        }
        let e = Entry.{ typ: typ, count: count, at: @truncate(usize, vat) };
        if tag == 256 {
            p.width = elem(f, e, 0);
        } else if tag == 257 {
            p.height = elem(f, e, 0);
        } else if tag == 258 {
            p.bits = e;
        } else if tag == 259 {
            p.compression = elem(f, e, 0);
        } else if tag == 262 {
            p.photometric = elem(f, e, 0);
        } else if tag == 266 {
            p.fill_order = elem(f, e, 0);
        } else if (tag == 273) or (tag == 324) {
            p.offsets = e;
        } else if tag == 277 {
            p.spp = elem(f, e, 0);
        } else if tag == 278 {
            p.rows_per_strip = elem(f, e, 0);
        } else if (tag == 279) or (tag == 325) {
            p.counts = e;
        } else if tag == 284 {
            p.planar = elem(f, e, 0);
        } else if tag == 292 {
            p.t4 = elem(f, e, 0);
        } else if tag == 317 {
            p.predictor = elem(f, e, 0);
        } else if tag == 320 {
            p.colormap = e;
        } else if tag == 322 {
            p.tile_w = elem(f, e, 0);
        } else if tag == 323 {
            p.tile_h = elem(f, e, 0);
        } else if tag == 338 {
            p.extras = e;
        } else if tag == 339 {
            p.formats = e;
        } else if tag == 347 {
            p.tables = e;
        }
    }
    let after = start + (@truncate(usize, n) * @truncate(usize, esize));
    p.next = f.u32at(after);
    if f.big {
        p.next = f.u64at(after);
    }
    return p;
}

// the directory of page n
fn page_at(f: *File, n: usize) !Page {
    let mut off = first_ifd(f);
    let mut k: usize = 0;
    loop {
        let p = read_page(f, off)!;
        if k == n {
            return p;
        }
        if (p.next == 0) or (k > 65536) {
            return error.InvalidImage;
        }
        off = p.next;
        k += 1;
    }
}

/// The number of pages (images) in the file.
pub fn page_count(data: []u8) !usize {
    let f = open(data)!;
    let mut off = first_ifd(&f);
    let mut k: usize = 0;
    loop {
        let p = read_page(&f, off)!;
        k += 1;
        if p.next == 0 {
            return k;
        }
        if k > 65536 {
            return error.InvalidImage;
        }
        off = p.next;
    }
}

/// The first page.
pub fn decode(data: []u8) !image.Image {
    return decode_page(data, 0);
}

/// Page n (from 0).
pub fn decode_page(data: []u8, n: usize) !image.Image {
    let f = open(data)!;
    let p = page_at(&f, n)!;
    return decode_ifd(&f, &p);
}

// -----------------------------------------------------------------------------
// Decoding
// -----------------------------------------------------------------------------

fn decode_ifd(f: *File, p: *Page) !image.Image {
    if (p.width == 0) or (p.height == 0) or (p.width > 0x10000000) or (p.height > 0x10000000) {
        return error.InvalidImage;
    }
    let w = @truncate(usize, p.width);
    let h = @truncate(usize, p.height);
    let px = @mul_checked(w, h) catch return error.TooLarge;
    if px > image.MAX_PIXELS {
        return error.TooLarge;
    }
    if (p.spp == 0) or (p.spp > 16) {
        return error.Unsupported;
    }
    let spp = @truncate(usize, p.spp);
    let bps = first_or(f, p.bits, 1);
    let mut bk: u64 = 1;
    while bk < @min(p.bits.count, p.spp) {
        if elem(f, p.bits, bk) != bps {
            return error.Unsupported;
        }
        bk += 1;
    }
    let format = first_or(f, p.formats, 1);
    let float = format == 3;
    let signed = format == 2;
    if float {
        if (bps != 16) and (bps != 32) and (bps != 64) {
            return error.Unsupported;
        }
    } else if (format != 1) and (format != 2) and (format != 4) {
        return error.Unsupported;
    } else if ((bps == 0) or (bps > 16)) and (bps != 32) and (bps != 64) {
        return error.Unsupported;
    }
    let comp = p.compression;
    let mut photo = p.photometric;
    if photo == 99 {
        photo = 1;
        if spp >= 3 {
            photo = 2;
        }
    }
    let mut colors: usize = 1;
    if (photo == 2) or ((photo == 6) and (comp == 7)) {
        colors = 3;
    } else if photo == 5 {
        colors = 4;
    } else if (photo != 0) and (photo != 1) and (photo != 3) {
        return error.Unsupported;
    }
    if spp < colors {
        return error.InvalidImage;
    }
    let mut alpha: u64 = 0;  // 1 associated, 2 unassociated
    if (spp > colors) and (p.extras.count > 0) {
        let e = elem(f, p.extras, 0);
        if (e == 1) or (e == 2) {
            alpha = e;
        }
    }
    if photo == 3 {
        if float or (bps > 16) or (p.colormap.count < (@as(u64, 3) << bps)) {
            return error.InvalidImage;
        }
    }
    let fax = (comp == 2) or (comp == 3) or (comp == 4);
    if fax and ((bps != 1) or (spp != 1)) {
        return error.Unsupported;
    }
    if (comp == 7) and (bps != 8) {
        return error.Unsupported;
    }
    if (comp != 1) and not fax and (comp != 5) and (comp != 7) and (comp != 8) and (comp != 32946) and (comp != 32773) {
        return error.Unsupported;
    }
    let pred = p.predictor;
    if (pred == 2) and (float or ((bps != 8) and (bps != 16) and (bps != 32) and (bps != 64))) {
        return error.Unsupported;
    }
    if (pred == 3) and not float {
        return error.Unsupported;
    }
    if (pred != 1) and (pred != 2) and (pred != 3) {
        return error.Unsupported;
    }
    // the chunks: strips or tiles
    let tiled = p.tile_w > 0;
    let mut cw = w;
    let mut chh = h;
    if (p.rows_per_strip > 0) and (p.rows_per_strip < p.height) {
        chh = @truncate(usize, p.rows_per_strip);
    }
    if tiled {
        if (p.tile_h == 0) or (p.tile_w > 65536) or (p.tile_h > 65536) {
            return error.InvalidImage;
        }
        cw = @truncate(usize, p.tile_w);
        chh = @truncate(usize, p.tile_h);
    }
    let across = (w + (cw - 1)) / cw;
    let down = (h + (chh - 1)) / chh;
    let mut planes: usize = 1;
    let mut spc = spp;
    if p.planar == 2 {
        planes = spp;
        spc = 1;
    }
    let per_plane = across * down;
    let nchunks = per_plane * planes;
    if (p.offsets.count < @as(u64, nchunks)) or (p.counts.count < @as(u64, nchunks)) {
        return error.InvalidImage;
    }
    let row_bits = @mul_checked(
        @mul_checked(cw, spc) catch return error.TooLarge, @truncate(usize, bps)) catch return error.TooLarge;
    let rowbytes = (row_bits + 7) / 8;
    let chunk_bytes = @mul_checked(rowbytes, chh) catch return error.TooLarge;
    if chunk_bytes > 0x40000000 {
        return error.TooLarge;
    }
    let mut esz: usize = 1;
    if float {
        esz = 4;
    } else if bps > 8 {
        esz = 2;
    }
    let raw_len = @mul_checked(@mul_checked(px, spp) catch return error.TooLarge, esz) catch return error.TooLarge;
    let raw = mem.alloc_zeroed.[u8](raw_len)!;
    defer mem.free(raw);
    let buf = mem.alloc.[u8](@max(chunk_bytes, 1))!;
    defer mem.free(buf);
    let vals = mem.alloc.[u64](cw * spc)!;
    defer mem.free(vals);
    let tmp = mem.alloc.[u8](@max(rowbytes, 1))!;
    defer mem.free(tmp);
    let mut src_copy = strs.String.init();
    defer src_copy.deinit();
    let mut fx = Fax.{ white: @slice_from_ptr(@ptr_from_int(*mut u32, 4), 0),
                       black: @slice_from_ptr(@ptr_from_int(*mut u32, 4), 0) };
    defer fx.deinit();
    if fax {
        fx.init()!;
    }
    let len = @as(u64, f.data.len);
    for ci in @as(usize, 0)..nchunks {
        let plane = ci / per_plane;
        let rest = ci % per_plane;
        let x0 = (rest % across) * cw;
        let y0 = (rest / across) * chh;
        let mut rows = chh;
        if not tiled {
            rows = @min(chh, h - y0);
        }
        let off = elem(f, p.offsets, @as(u64, ci));
        let mut n = elem(f, p.counts, @as(u64, ci));
        if off > len {
            return error.InvalidImage;
        }
        n = @min(n, len - off);
        let mut src = f.data[@truncate(usize, off)..@truncate(usize, off + n)];
        if p.fill_order == 2 {
            src_copy.clear();
            src_copy.push_str(src)!;
            let s = src_copy.bytes();
            for i in @as(usize, 0)..s.len {
                s[i] = reverse_bits(s[i]);
            }
            src = s;
        }
        let out = buf[..rowbytes * rows];
        @mem_set(out, 0);
        if comp == 1 {
            let k = @min(src.len, out.len);
            @mem_copy(out[..k], src[..k]);
        } else if comp == 32773 {
            unpackbits(src, out);
        } else if comp == 5 {
            _ = lzw_decode(src, out)!;
        } else if (comp == 8) or (comp == 32946) {
            let z = deflate.decompress(src, deflate.Format.ZLIB, out.len + 65536) catch |e| {
                if e == error.OutOfMemory {
                    return e;
                }
                return error.InvalidImage;
            };
            let k = @min(z.len, out.len);
            @mem_copy(out[..k], z[..k]);
            mem.free(z);
        } else if fax {
            fx.decode(src, out, cw, rows, rowbytes, comp, p.t4)!;
        } else {
            jpeg_chunk(f, p, src, out, cw, rows, spc)!;
        }
        for r in @as(usize, 0)..rows {
            let y = y0 + r;
            if y >= h {
                break;
            }
            unpack_row(f, out[r * rowbytes..(r + 1) * rowbytes], vals, spc, bps, pred, tmp);
            let xn = @min(cw, w - x0);
            for c in @as(usize, 0)..xn {
                let base = ((y * w) + x0 + c) * spp;
                for s in @as(usize, 0)..spc {
                    let v = vals[(c * spc) + s];
                    let mut idx = base + s;
                    if planes > 1 {
                        idx = base + plane;
                    }
                    put_raw(raw, idx, v, float, signed, bps);
                }
            }
        }
    }
    return finish(f, p, raw, w, h, spp, colors, photo, alpha, float, bps);
}

fn reverse_bits(b: u8) u8 {
    let mut v = b;
    v = ((v & 0xf0) >> 4) | ((v & 0x0f) << 4);
    v = ((v & 0xcc) >> 2) | ((v & 0x33) << 2);
    return ((v & 0xaa) >> 1) | ((v & 0x55) << 1);
}

// the samples of one row of a chunk, undoing the predictor
fn unpack_row(f: *File, row: []u8, vals: []mut u64, spc: usize, bps: u64, pred: u64, tmp: []mut u8) void {
    let n = vals.len;
    if pred == 3 {
        // bytes of each sample split into planes (most significant first),
        // then differenced
        let k = @truncate(usize, bps / 8);
        let nb = n * k;
        @mem_copy(tmp[..nb], row[..nb]);
        for i in spc..nb {
            tmp[i] = tmp[i] +% tmp[i - spc];
        }
        for j in @as(usize, 0)..n {
            let mut v: u64 = 0;
            for b in @as(usize, 0)..k {
                v = (v << 8) | @as(u64, tmp[(b * n) + j]);
            }
            vals[j] = v;
        }
        return;
    }
    if (bps % 8) == 0 {
        let k = @truncate(usize, bps / 8);
        for j in @as(usize, 0)..n {
            let at = j * k;
            let mut v: u64 = 0;
            if f.be {
                for b in @as(usize, 0)..k {
                    v = (v << 8) | @as(u64, row[at + b]);
                }
            } else {
                for b in @as(usize, 0)..k {
                    v |= @as(u64, row[at + b]) << @as(u64, 8 * b);
                }
            }
            vals[j] = v;
        }
    } else {
        let nbits = @truncate(usize, bps);
        let mut pos: usize = 0;
        for j in @as(usize, 0)..n {
            let mut v: u64 = 0;
            for _q in @as(usize, 0)..nbits {
                v = (v << 1) | @as(u64, (row[pos >> 3] >> @truncate(u8, 7 - (pos & 7))) & 1);
                pos += 1;
            }
            vals[j] = v;
        }
    }
    if pred == 2 {
        let mut mask: u64 = 0xffffffffffffffff;
        if bps < 64 {
            mask = (@as(u64, 1) << bps) - 1;
        }
        for j in spc..n {
            vals[j] = (vals[j] +% vals[j - spc]) & mask;
        }
    }
}

// a sample into the raw buffer: u8 (up to 8 bits), u16 (up to 16 bits;
// deeper integers keep their top 16) or f32
fn put_raw(raw: []mut u8, idx: usize, v: u64, float: bool, signed: bool, bps: u64) void {
    if float {
        let mut x: f32 = 0.0;
        if bps == 16 {
            x = image.half_to_f32(@truncate(u16, v));
        } else if bps == 32 {
            x = @bit_cast(f32, @truncate(u32, v));
        } else {
            x = @float_cast(f32, @bit_cast(f64, v));
        }
        @cast_slice(f32, raw)[idx] = x;
        return;
    }
    let mut u = v;
    if signed {
        u ^= @as(u64, 1) << (bps - 1);
    }
    if bps <= 8 {
        raw[idx] = @truncate(u8, u);
    } else {
        if bps > 16 {
            u >>= bps - 16;
        }
        @cast_slice(u16, raw)[idx] = @truncate(u16, u);
    }
}

// raw samples to an image: photometric interpretation, palettes, scaling,
// alpha
fn finish(f: *File, p: *Page, raw: []u8, w: usize, h: usize, spp: usize, colors: usize, photo: u64, alpha: u64,
          float: bool, bps: u64) !image.Image {
    let mut outc: usize = 3;
    if (photo == 0) or (photo == 1) {
        outc = 1;
    }
    if alpha != 0 {
        outc += 1;
    }
    let px = w * h;
    if float {
        let mut img = image.Image.init(w, h, outc, image.Depth.F32)!;
        let o = img.samples32();
        let r = @cast_slice(f32, raw[..(px * spp) * 4]);
        for i in @as(usize, 0)..px {
            let s = r[i * spp..(i + 1) * spp];
            let mut c: [4]f32 = [0.0; 4];
            if photo == 0 {
                c[0] = 1.0 - s[0];
            } else if photo == 5 {
                for k in @as(usize, 0)..3 {
                    c[k] = (1.0 - s[k]) * (1.0 - s[3]);
                }
            } else {
                for k in @as(usize, 0)..@min(colors, 3) {
                    c[k] = s[k];
                }
            }
            let nc = outc - @truncate(usize, @min(alpha, 1));
            if alpha != 0 {
                let a = s[colors];
                if (alpha == 1) and (a != 0.0) {
                    for k in @as(usize, 0)..nc {
                        c[k] = c[k] / a;
                    }
                }
                c[nc] = a;
            }
            for k in @as(usize, 0)..outc {
                o[(i * outc) + k] = c[k];
            }
        }
        return img;
    }
    let mut eff = bps;
    if eff > 16 {
        eff = 16;
    }
    let maxv = (@as(u64, 1) << eff) - 1;
    let mut outmax: u64 = 255;
    let mut depth = image.Depth.U8;
    if bps > 8 {
        outmax = 65535;
        depth = image.Depth.U16;
    }
    let mut img = image.Image.init(w, h, outc, depth)!;
    let o8 = img.data;
    let mut o16 = @slice_from_ptr(@ptr_from_int(*mut u16, 2), 0);
    let mut r16: []u16 = o16;
    if depth == image.Depth.U16 {
        o16 = img.samples16();
        r16 = @cast_slice(u16, raw[..(px * spp) * 2]);
    }
    let ncol = @as(u64, 1) << bps;
    for i in @as(usize, 0)..px {
        let mut s: [5]u64 = [0; 5];
        for k in @as(usize, 0)..@min(spp, 5) {
            if depth == image.Depth.U16 {
                s[k] = @as(u64, r16[(i * spp) + k]);
            } else {
                s[k] = @as(u64, raw[(i * spp) + k]);
            }
        }
        let mut c: [4]u64 = [0; 4];
        if photo == 3 {
            let ix = s[0];
            if ix < ncol {
                for k in @as(usize, 0)..3 {
                    c[k] = elem(f, p.colormap, (@as(u64, k) * ncol) + ix);
                    if depth == image.Depth.U8 {
                        c[k] = c[k] >> 8;
                    }
                }
            }
        } else {
            for k in @as(usize, 0)..@min(spp, 5) {
                s[k] = scale(s[k], maxv, outmax);
            }
            if photo == 0 {
                c[0] = outmax - s[0];
            } else if photo == 1 {
                c[0] = s[0];
            } else if photo == 5 {
                for k in @as(usize, 0)..3 {
                    c[k] = (((outmax - s[k]) * (outmax - s[3])) + (outmax / 2)) / outmax;
                }
            } else {
                c[0] = s[0];
                c[1] = s[1];
                c[2] = s[2];
            }
        }
        let nc = outc - @truncate(usize, @min(alpha, 1));
        if alpha != 0 {
            let mut a = s[colors];
            if photo == 3 {
                a = scale(a, maxv, outmax);
            }
            if alpha == 1 {
                for k in @as(usize, 0)..nc {
                    if a == 0 {
                        c[k] = 0;
                    } else {
                        c[k] = @min(outmax, ((c[k] * outmax) + (a / 2)) / a);
                    }
                }
            }
            c[nc] = a;
        }
        for k in @as(usize, 0)..outc {
            if depth == image.Depth.U16 {
                o16[(i * outc) + k] = @truncate(u16, c[k]);
            } else {
                o8[(i * outc) + k] = @truncate(u8, c[k]);
            }
        }
    }
    return img;
}

// v of 0..maxv to 0..outmax
fn scale(v: u64, maxv: u64, outmax: u64) u64 {
    if maxv == outmax {
        return v;
    }
    return ((@min(v, maxv) * outmax) + (maxv / 2)) / maxv;
}

// a JPEG strip or tile, with the tables shared by the page
fn jpeg_chunk(f: *File, p: *Page, src: []u8, out: []mut u8, cw: usize, rows: usize, spc: usize) !void {
    let mut js = strs.String.init();
    defer js.deinit();
    let t = p.tables;
    if (t.count >= 4) and (type_size(t.typ) == 1) and (src.len >= 2) {
        let tb = f.data[t.at..t.at + @truncate(usize, t.count)];
        js.push_str(src[..2])!;
        let mut end = tb.len;
        if (tb[end - 2] == 0xff) and (tb[end - 1] == 0xd9) {
            end -= 2;
        }
        js.push_str(tb[2..end])!;
        js.push_str(src[2..])!;
    } else {
        js.push_str(src)!;
    }
    let mut im = jpeg.decode(js.bytes()) catch |e| {
        if e == error.OutOfMemory {
            return e;
        }
        if e == error.Unsupported {
            return e;
        }
        return error.InvalidImage;
    };
    defer im.deinit();
    if im.channels != spc {
        return error.Unsupported;
    }
    let rowbytes = cw * spc;
    let n = @min(cw, im.width) * spc;
    for y in @as(usize, 0)..@min(rows, im.height) {
        @mem_copy(out[y * rowbytes..(y * rowbytes) + n], im.data[(y * im.width) * spc..((y * im.width) * spc) + n]);
    }
}

// -----------------------------------------------------------------------------
// PackBits and LZW
// -----------------------------------------------------------------------------

fn unpackbits(src: []u8, out: []mut u8) void {
    let mut i: usize = 0;
    let mut o: usize = 0;
    while (i < src.len) and (o < out.len) {
        let n = @as(usize, src[i]);
        i += 1;
        if n < 128 {
            let k = @min(@min(n + 1, src.len - i), out.len - o);
            @mem_copy(out[o..o + k], src[i..i + k]);
            i += n + 1;
            o += k;
        } else if n > 128 {
            if i >= src.len {
                return;
            }
            let k = @min(257 - n, out.len - o);
            @mem_set(out[o..o + k], src[i]);
            i += 1;
            o += k;
        }
    }
}

fn lzw_decode(src: []u8, out: []mut u8) !usize {
    if (src.len >= 2) and (src[0] == 0) and ((src[1] & 1) != 0) {
        return error.Unsupported;  // the old, LSB-first LZW
    }
    let prefix = mem.alloc.[u16](4096)!;
    defer mem.free(prefix);
    let suffix = mem.alloc.[u8](4096)!;
    defer mem.free(suffix);
    let first = mem.alloc.[u8](4096)!;
    defer mem.free(first);
    let lens = mem.alloc.[u16](4096)!;
    defer mem.free(lens);
    for i in @as(usize, 0)..256 {
        prefix[i] = 0;
        suffix[i] = @truncate(u8, i);
        first[i] = @truncate(u8, i);
        lens[i] = 1;
    }
    let mut acc: u64 = 0;
    let mut nacc: u64 = 0;
    let mut ip: usize = 0;
    let mut width: u64 = 9;
    let mut next: usize = 258;
    let mut prev: usize = 4096;  // none
    let mut o: usize = 0;
    loop {
        while nacc < width {
            if ip >= src.len {
                return o;
            }
            acc = (acc << 8) | @as(u64, src[ip]);
            ip += 1;
            nacc += 8;
        }
        let code = @truncate(usize, (acc >> (nacc - width)) & ((@as(u64, 1) << width) - 1));
        nacc -= width;
        acc &= (@as(u64, 1) << nacc) - 1;
        if code == 257 {
            return o;
        }
        if code == 256 {
            width = 9;
            next = 258;
            prev = 4096;
            continue;
        }
        if prev == 4096 {
            if code > 255 {
                return error.InvalidImage;
            }
        } else {
            if code > next {
                return error.InvalidImage;
            }
            if next < 4096 {
                let mut fb = first[prev];
                if code < next {
                    fb = first[code];
                }
                prefix[next] = @truncate(u16, prev);
                suffix[next] = fb;
                first[next] = first[prev];
                lens[next] = lens[prev] + 1;
                next += 1;
                if (next >= ((@as(usize, 1) << @truncate(usize, width)) - 1)) and (width < 12) {
                    width += 1;
                }
            } else if code >= 4096 {
                return error.InvalidImage;
            }
        }
        // the string, written backwards
        let n = @as(usize, lens[code]);
        let mut k = code;
        let mut j = n;
        while j > 0 {
            j -= 1;
            if (o + j) < out.len {
                out[o + j] = suffix[k];
            }
            k = @as(usize, prefix[k]);
        }
        o += n;
        if o >= out.len {
            return out.len;
        }
        prev = code;
    }
}

struct LzwOut {
    out: *mut strs.String,
    acc: u64,
    n: u64,
    width: u64,
    next: u64,  // the decoder's next code
    first: bool,

    fn bits(self: *mut LzwOut, code: u64) !void {
        self.acc = (self.acc << self.width) | code;
        self.n += self.width;
        while self.n >= 8 {
            self.out.push(@truncate(u8, self.acc >> (self.n - 8)))!;
            self.n -= 8;
        }
        self.acc &= (@as(u64, 1) << self.n) - 1;
    }

    // a code, tracking the decoder's code width
    fn data(self: *mut LzwOut, code: u64) !void {
        self.bits(code)!;
        if self.first {
            self.first = false;
        } else {
            self.next += 1;
            if (self.next >= ((@as(u64, 1) << self.width) - 1)) and (self.width < 12) {
                self.width += 1;
            }
        }
    }

    fn clear(self: *mut LzwOut) !void {
        self.bits(256)!;
        self.width = 9;
        self.next = 258;
        self.first = true;
    }

    fn finish(self: *mut LzwOut) !void {
        self.bits(257)!;
        if self.n > 0 {
            self.out.push(@truncate(u8, self.acc << (8 - self.n)))!;
        }
    }
}

const HASH_SIZE: usize = 16384;

fn lzw_encode(src: []u8, out: *mut strs.String) !void {
    let mut lo = LzwOut.{ out: out, acc: 0, n: 0, width: 9, next: 258, first: true };
    lo.clear()!;
    if src.len == 0 {
        lo.finish()!;
        return;
    }
    let keys = mem.alloc_zeroed.[u32](HASH_SIZE)!;
    defer mem.free(keys);
    let codes = mem.alloc.[u16](HASH_SIZE)!;
    defer mem.free(codes);
    let mut next: u32 = 258;
    let mut prefix = @as(u32, src[0]);
    for i in @as(usize, 1)..src.len {
        let key = ((prefix << 8) | @as(u32, src[i])) + 1;
        let mut hsh = @as(usize, (key *% 2654435761) >> 18) & (HASH_SIZE - 1);
        let mut found = false;
        while keys[hsh] != 0 {
            if keys[hsh] == key {
                found = true;
                break;
            }
            hsh = (hsh + 1) & (HASH_SIZE - 1);
        }
        if found {
            prefix = @as(u32, codes[hsh]);
            continue;
        }
        lo.data(@as(u64, prefix))!;
        keys[hsh] = key;
        codes[hsh] = @truncate(u16, next);
        next += 1;
        if next == 4094 {
            lo.clear()!;
            @mem_set(keys, 0);
            next = 258;
        }
        prefix = @as(u32, src[i]);
    }
    lo.data(@as(u64, prefix))!;
    lo.finish()!;
}

fn packbits_row(row: []u8, o: *mut strs.String) !void {
    let n = row.len;
    let mut i: usize = 0;
    while i < n {
        let mut r: usize = 1;
        while ((i + r) < n) and (r < 128) and (row[i + r] == row[i]) {
            r += 1;
        }
        if r >= 2 {
            o.push(@truncate(u8, 257 - r))!;
            o.push(row[i])!;
            i += r;
            continue;
        }
        let start = i;
        while (i < n) and ((i - start) < 128) {
            if ((i + 1) < n) and (row[i] == row[i + 1]) {
                break;
            }
            i += 1;
        }
        o.push(@truncate(u8, (i - start) - 1))!;
        o.push_str(row[start..i])!;
    }
}

// -----------------------------------------------------------------------------
// CCITT fax: modified Huffman, T.4 (group 3) and T.6 (group 4)
// -----------------------------------------------------------------------------

// The run-length codes: terminating (0-63), make-up (64-1728) and the
// extended make-up codes both colours share (1792-2560).
let WHITE_CODES: [104][]u8 = ["00110101", "000111", "0111", "1000", "1011", "1100", "1110", "1111", "10011", "10100",
                              "00111", "01000", "001000", "000011", "110100", "110101", "101010", "101011", "0100111",
                              "0001100", "0001000", "0010111", "0000011", "0000100", "0101000", "0101011", "0010011",
                              "0100100", "0011000", "00000010", "00000011", "00011010", "00011011", "00010010",
                              "00010011", "00010100", "00010101", "00010110", "00010111", "00101000", "00101001",
                              "00101010", "00101011", "00101100", "00101101", "00000100", "00000101", "00001010",
                              "00001011", "01010010", "01010011", "01010100", "01010101", "00100100", "00100101",
                              "01011000", "01011001", "01011010", "01011011", "01001010", "01001011", "00110010",
                              "00110011", "00110100", "11011", "10010", "010111", "0110111", "00110110", "00110111",
                              "01100100", "01100101", "01101000", "01100111", "011001100", "011001101", "011010010",
                              "011010011", "011010100", "011010101", "011010110", "011010111", "011011000", "011011001",
                              "011011010", "011011011", "010011000", "010011001", "010011010", "011000", "010011011",
                              "00000001000", "00000001100", "00000001101", "000000010010", "000000010011",
                              "000000010100", "000000010101", "000000010110", "000000010111", "000000011100",
                              "000000011101", "000000011110", "000000011111"];
let BLACK_CODES: [104][]u8 = ["0000110111", "010", "11", "10", "011", "0011", "0010", "00011", "000101", "000100",
                              "0000100", "0000101", "0000111", "00000100", "00000111", "000011000", "0000010111",
                              "0000011000", "0000001000", "00001100111", "00001101000", "00001101100", "00000110111",
                              "00000101000", "00000010111", "00000011000", "000011001010", "000011001011",
                              "000011001100", "000011001101", "000001101000", "000001101001", "000001101010",
                              "000001101011", "000011010010", "000011010011", "000011010100", "000011010101",
                              "000011010110", "000011010111", "000001101100", "000001101101", "000011011010",
                              "000011011011", "000001010100", "000001010101", "000001010110", "000001010111",
                              "000001100100", "000001100101", "000001010010", "000001010011", "000000100100",
                              "000000110111", "000000111000", "000000100111", "000000101000", "000001011000",
                              "000001011001", "000000101011", "000000101100", "000001011010", "000001100110",
                              "000001100111", "0000001111", "000011001000", "000011001001", "000001011011",
                              "000000110011", "000000110100", "000000110101", "0000001101100", "0000001101101",
                              "0000001001010", "0000001001011", "0000001001100", "0000001001101", "0000001110010",
                              "0000001110011", "0000001110100", "0000001110101", "0000001110110", "0000001110111",
                              "0000001010010", "0000001010011", "0000001010100", "0000001010101", "0000001011010",
                              "0000001011011", "0000001100100", "0000001100101", "00000001000", "00000001100",
                              "00000001101", "000000010010", "000000010011", "000000010100", "000000010101",
                              "000000010110", "000000010111", "000000011100", "000000011101", "000000011110",
                              "000000011111"];

struct Bits {
    src: []u8,
    pos: usize,  // in bits

    fn peek(self: *Bits, n: usize) u32 {
        let mut v: u32 = 0;
        let mut p = self.pos;
        for _q in @as(usize, 0)..n {
            let mut bit: u32 = 0;
            if (p >> 3) < self.src.len {
                bit = @as(u32, (self.src[p >> 3] >> @truncate(u8, 7 - (p & 7))) & 1);
            }
            v = (v << 1) | bit;
            p += 1;
        }
        return v;
    }

    fn done(self: *Bits) bool {
        return self.pos >= (self.src.len * 8);
    }
}

// a 13-bit lookup: (code length << 16) | run, or 0
fn fax_table(codes: *[104][]u8) ![]mut u32 {
    let t = mem.alloc_zeroed.[u32](8192)!;
    for k in @as(usize, 0)..104 {
        let mut run = k;
        if (k >= 64) and (k < 91) {
            run = (k - 63) * 64;
        } else if k >= 91 {
            run = 1792 + ((k - 91) * 64);
        }
        let c = codes[k];
        let mut v: usize = 0;
        for i in @as(usize, 0)..c.len {
            v = (v << 1) | @as(usize, c[i] - '0');
        }
        let shift = 13 - c.len;
        let base = v << shift;
        for j in @as(usize, 0)..(@as(usize, 1) << shift) {
            t[base + j] = @truncate(u32, (c.len << 16) | run);
        }
    }
    return t;
}

struct Fax {
    white: []mut u32,
    black: []mut u32,

    fn init(self: *mut Fax) !void {
        self.white = fax_table(&WHITE_CODES)!;
        self.black = fax_table(&BLACK_CODES)!;
    }

    fn deinit(self: *mut Fax) void {
        if self.white.len > 0 {
            mem.free(self.white);
        }
        if self.black.len > 0 {
            mem.free(self.black);
        }
    }

    fn run(self: *Fax, b: *mut Bits, color: usize) !usize {
        let mut t = self.white;
        if color != 0 {
            t = self.black;
        }
        let mut total: usize = 0;
        loop {
            let e = t[@as(usize, b.peek(13))];
            if e == 0 {
                return error.InvalidImage;
            }
            b.pos += @as(usize, e >> 16);
            let r = @as(usize, e & 0xffff);
            total += r;
            if r < 64 {
                return total;
            }
            if b.done() {
                return error.InvalidImage;
            }
        }
    }

    fn line_1d(self: *Fax, b: *mut Bits, cur: []mut usize, w: usize) !usize {
        let mut n: usize = 0;
        let mut a0: usize = 0;
        let mut color: usize = 0;
        while a0 < w {
            a0 = @min(a0 + self.run(b, color)!, w);
            if n >= cur.len {
                return error.InvalidImage;
            }
            cur[n] = a0;
            n += 1;
            color ^= 1;
        }
        return n;
    }

    // ref holds the reference line's changes, then w at least three times
    fn line_2d(self: *Fax, b: *mut Bits, rf: []usize, nref: usize, cur: []mut usize, w: usize) !usize {
        let mut n: usize = 0;
        let mut a0: usize = 0;
        let mut start = true;  // a0 is before the first pixel
        let mut color: usize = 0;
        let mut bi: usize = 0;
        while start or (a0 < w) {
            if not start {
                while (bi < nref) and (rf[bi] <= a0) {
                    bi += 1;
                }
            }
            let mut i = bi;
            if (i & 1) != color {
                i += 1;
            }
            let b1 = rf[i];
            let b2 = rf[i + 1];
            let m = b.peek(7);
            let mut d: usize = 0;
            let mut left = false;
            if m >= 64 {
                b.pos += 1;
            } else if (m >> 4) == 3 {
                b.pos += 3;
                d = 1;
            } else if (m >> 4) == 2 {
                b.pos += 3;
                d = 1;
                left = true;
            } else if (m >> 4) == 1 {
                // horizontal: two runs
                b.pos += 3;
                let a1 = @min(a0 + self.run(b, color)!, w);
                let a2 = @min(a1 + self.run(b, color ^ 1)!, w);
                if (n + 2) > cur.len {
                    return error.InvalidImage;
                }
                cur[n] = a1;
                cur[n + 1] = a2;
                n += 2;
                a0 = a2;
                start = false;
                continue;
            } else if (m >> 3) == 1 {
                // pass
                b.pos += 4;
                a0 = b2;
                start = false;
                continue;
            } else if (m >> 1) == 3 {
                b.pos += 6;
                d = 2;
            } else if (m >> 1) == 2 {
                b.pos += 6;
                d = 2;
                left = true;
            } else if m == 3 {
                b.pos += 7;
                d = 3;
            } else if m == 2 {
                b.pos += 7;
                d = 3;
                left = true;
            } else {
                return error.InvalidImage;  // an extension or an early end of line
            }
            if left and (b1 < d) {
                return error.InvalidImage;
            }
            let mut a1 = b1 + d;
            if left {
                a1 = b1 - d;
            }
            if (a1 > w) or ((not start) and (a1 < a0)) or (n >= cur.len) {
                return error.InvalidImage;
            }
            cur[n] = a1;
            n += 1;
            a0 = a1;
            color ^= 1;
            start = false;
        }
        return n;
    }

    fn decode(self: *Fax, src: []u8, out: []mut u8, w: usize, rows: usize, rowbytes: usize, comp: u64, t4: u64) !void {
        let cap = (2 * w) + 8;
        let rf = mem.alloc.[usize](cap + 3)!;
        defer mem.free(rf);
        let cur = mem.alloc.[usize](cap)!;
        defer mem.free(cur);
        let mut nref: usize = 0;
        rf[0] = w;
        rf[1] = w;
        rf[2] = w;
        let mut b = Bits.{ src: src, pos: 0 };
        for y in @as(usize, 0)..rows {
            let mut two_d = comp == 4;
            if comp == 2 {
                b.pos = (b.pos + 7) & ~@as(usize, 7);
            } else if comp == 3 {
                // fill bits and the end of line
                while (not b.done()) and (b.peek(12) == 0) {
                    b.pos += 1;
                }
                if b.peek(12) == 1 {
                    b.pos += 12;
                }
                if (t4 & 1) != 0 {
                    two_d = b.peek(1) == 0;
                    b.pos += 1;
                }
            }
            let mut n: usize = 0;
            if two_d {
                n = self.line_2d(&mut b, rf, nref, cur, w)!;
            } else {
                n = self.line_1d(&mut b, cur, w)!;
            }
            // black runs are set bits
            let o = out[y * rowbytes..(y + 1) * rowbytes];
            let mut i: usize = 0;
            while i < n {
                let mut e = w;
                if (i + 1) < n {
                    e = cur[i + 1];
                }
                for x in cur[i]..e {
                    o[x >> 3] |= @truncate(u8, @as(usize, 0x80) >> (x & 7));
                }
                i += 2;
            }
            @mem_copy(rf[..n], cur[..n]);
            nref = n;
            rf[n] = w;
            rf[n + 1] = w;
            rf[n + 2] = w;
        }
    }
}

// -----------------------------------------------------------------------------
// Encoding
// -----------------------------------------------------------------------------

/// Writes one image.
pub fn encode(img: *image.Image, opts: Options) ![]mut u8 {
    let one: [1]*image.Image = [img];
    return encode_pages(one[..], opts);
}

fn put16(o: *mut strs.String, v: u64) !void {
    o.push(@truncate(u8, v))!;
    o.push(@truncate(u8, v >> 8))!;
}

fn put32(o: *mut strs.String, v: u64) !void {
    put16(o, v & 0xffff)!;
    put16(o, v >> 16)!;
}

fn set32(o: *mut strs.String, at: usize, v: u64) void {
    let b = o.bytes();
    for k in @as(usize, 0)..4 {
        b[at + k] = @truncate(u8, v >> @as(u64, 8 * k));
    }
}

fn align2(o: *mut strs.String) !void {
    if (o.len & 1) != 0 {
        o.push(0)!;
    }
}

// a field: SHORTs or LONGs, inline when they fit
fn field(o: *mut strs.String, tag: u64, typ: u64, count: u64, value: u64) !void {
    put16(o, tag)!;
    put16(o, typ)!;
    put32(o, count)!;
    put32(o, value)!;
}

/// Writes images as the pages of one file.
pub fn encode_pages(imgs: []*image.Image, opts: Options) ![]mut u8 {
    let comp = opts.compression;
    if imgs.len == 0 {
        return error.InvalidImage;
    }
    let mut o = strs.String.init();
    defer o.deinit();
    o.push_str("II*\x00")!;
    put32(&mut o, 0)!;
    let mut link: usize = 4;
    let mut strip = strs.String.init();
    defer strip.deinit();
    let mut packed = strs.String.init();
    defer packed.deinit();
    let mut offs = strs.String.init();  // u32s
    defer offs.deinit();
    for pi in @as(usize, 0)..imgs.len {
        let img = imgs[pi];
        let ch = img.channels;
        let w = img.width;
        let h = img.height;
        let mut bps: usize = 8;
        let mut format: u64 = 1;
        if img.depth == image.Depth.U16 {
            bps = 16;
        } else if img.depth == image.Depth.F32 {
            bps = 32;
            format = 3;
        }
        let bytes = bps / 8;
        let rowbytes = (w * ch) * bytes;
        let rps = @min(@max(65536 / @max(rowbytes, 1), 1), h);
        let nstrips = (h + (rps - 1)) / rps;
        let use_pred = opts.predictor and ((comp == Compression.LZW) or (comp == Compression.DEFLATE));
        let mut pred: u64 = 1;
        if use_pred {
            pred = 2;
            if format == 3 {
                pred = 3;
            }
        }
        offs.clear();
        for si in @as(usize, 0)..nstrips {
            let y0 = si * rps;
            let y1 = @min(y0 + rps, h);
            strip.clear();
            strip.push_str(img.data[y0 * rowbytes..y1 * rowbytes])!;
            let sb = strip.bytes();
            if use_pred {
                for y in @as(usize, 0)..(y1 - y0) {
                    predict(sb[y * rowbytes..(y + 1) * rowbytes], ch, bps, pred, &mut packed)!;
                }
            }
            packed.clear();
            if comp == Compression.NONE {
                packed.push_str(sb)!;
            } else if comp == Compression.PACKBITS {
                for y in @as(usize, 0)..(y1 - y0) {
                    packbits_row(sb[y * rowbytes..(y + 1) * rowbytes], &mut packed)!;
                }
            } else if comp == Compression.LZW {
                lzw_encode(sb, &mut packed)!;
            } else {
                let z = deflate.compress(sb, deflate.Format.ZLIB, deflate.DEFAULT_LEVEL)!;
                defer mem.free(z);
                packed.push_str(z)!;
            }
            align2(&mut o)!;
            put32(&mut offs, @as(u64, o.len))!;
            put32(&mut offs, @as(u64, packed.len))!;
            o.push_str(packed.bytes())!;
        }
        // the arrays too long to go inline
        align2(&mut o)!;
        let mut bits_at: u64 = 0;
        let mut format_at: u64 = 0;
        let mut offsets_at: u64 = 0;
        let mut counts_at: u64 = 0;
        let ob = offs.bytes();
        if ch > 2 {
            bits_at = @as(u64, o.len);
            for _q in @as(usize, 0)..ch {
                put16(&mut o, @as(u64, bps))!;
            }
            format_at = @as(u64, o.len);
            for _q in @as(usize, 0)..ch {
                put16(&mut o, format)!;
            }
        }
        if nstrips > 1 {
            offsets_at = @as(u64, o.len);
            for si in @as(usize, 0)..nstrips {
                o.push_str(ob[si * 8..(si * 8) + 4])!;
            }
            counts_at = @as(u64, o.len);
            for si in @as(usize, 0)..nstrips {
                o.push_str(ob[(si * 8) + 4..(si * 8) + 8])!;
            }
        }
        // the directory
        align2(&mut o)!;
        set32(&mut o, link, @as(u64, o.len));
        let mut nf: u64 = 10;
        if pred != 1 {
            nf += 1;
        }
        if (ch == 2) or (ch == 4) {
            nf += 1;
        }
        if format != 1 {
            nf += 1;
        }
        put16(&mut o, nf)!;
        field(&mut o, 256, 4, 1, @as(u64, w))!;
        field(&mut o, 257, 4, 1, @as(u64, h))!;
        if ch > 2 {
            field(&mut o, 258, 3, @as(u64, ch), bits_at)!;
        } else {
            field(&mut o, 258, 3, @as(u64, ch), (@as(u64, bps) << 16) | @as(u64, bps))!;
        }
        field(&mut o, 259, 3, 1, @as(u64, @int_from_enum(comp)))!;
        let mut photo: u64 = 1;
        if ch >= 3 {
            photo = 2;
        }
        field(&mut o, 262, 3, 1, photo)!;
        if nstrips > 1 {
            field(&mut o, 273, 4, @as(u64, nstrips), offsets_at)!;
        } else {
            field(&mut o, 273, 4, 1, word(ob, 0))!;
        }
        field(&mut o, 277, 3, 1, @as(u64, ch))!;
        field(&mut o, 278, 4, 1, @as(u64, rps))!;
        if nstrips > 1 {
            field(&mut o, 279, 4, @as(u64, nstrips), counts_at)!;
        } else {
            field(&mut o, 279, 4, 1, word(ob, 4))!;
        }
        field(&mut o, 284, 3, 1, 1)!;
        if pred != 1 {
            field(&mut o, 317, 3, 1, pred)!;
        }
        if (ch == 2) or (ch == 4) {
            field(&mut o, 338, 3, 1, 2)!;
        }
        if format != 1 {
            if ch > 2 {
                field(&mut o, 339, 3, @as(u64, ch), format_at)!;
            } else {
                field(&mut o, 339, 3, @as(u64, ch), (format << 16) | format)!;
            }
        }
        link = o.len;
        put32(&mut o, 0)!;
        if o.len > 0xffffffff {
            return error.TooLarge;
        }
    }
    return o.take();
}

fn word(b: []u8, i: usize) u64 {
    return ((@as(u64, b[i]) | (@as(u64, b[i + 1]) << 8)) | (@as(u64, b[i + 2]) << 16)) | (@as(u64, b[i + 3]) << 24);
}

// differences a row in place: horizontal (pred 2) on samples, or
// floating-point (pred 3) on the bytes of the samples split into planes
fn predict(row: []mut u8, ch: usize, bps: usize, pred: u64, tmp: *mut strs.String) !void {
    if pred == 3 {
        let n = row.len / 4;
        tmp.clear();
        tmp.push_str(row)!;
        let t = tmp.bytes();
        for j in @as(usize, 0)..n {
            for b in @as(usize, 0)..4 {
                row[(b * n) + j] = t[(j * 4) + (3 - b)];
            }
        }
        let mut i = row.len;
        while i > ch {
            i -= 1;
            row[i] = row[i] -% row[i - ch];
        }
        return;
    }
    if bps == 8 {
        let mut i = row.len;
        while i > ch {
            i -= 1;
            row[i] = row[i] -% row[i - ch];
        }
        return;
    }
    let s = @cast_slice(u16, row);
    let mut i = s.len;
    while i > ch {
        i -= 1;
        s[i] = s[i] -% s[i - ch];
    }
}

// -----------------------------------------------------------------------------
// Tests
// -----------------------------------------------------------------------------

fn num(fs_: *mut strs.Fields) usize {
    let t = fs_.next() catch return 0;
    return fmt.parse_int.[usize](t) catch 0;
}

test "files made by tifffile and libtiff" {
    let list = fs.read_file("std/graphics/image/testdata/tiff/cases.txt") catch return;
    defer mem.free(list);
    let mut lines = strs.lines(list);
    let mut path = strs.String.init();
    defer path.deinit();
    let mut cases: usize = 0;
    loop {
        let line = lines.next() catch break;
        let mut fl = strs.fields(line);
        let name = fl.next() catch continue;
        let rawname = fl.next()!;
        let w = num(&mut fl);
        let h = num(&mut fl);
        let c = num(&mut fl);
        let depth_name = fl.next()!;
        let page = num(&mut fl);
        let tol = num(&mut fl);
        path.clear();
        path.push_str("std/graphics/image/testdata/tiff/")!;
        path.push_str(name)!;
        let data = fs.read_file(path.bytes())!;
        defer mem.free(data);
        @expect(is_tiff(data));
        path.clear();
        path.push_str("std/graphics/image/testdata/tiff/")!;
        path.push_str(rawname)!;
        let want = fs.read_file(path.bytes())!;
        defer mem.free(want);
        let mut img = decode_page(data, page) catch |e| {
            @expect_eq(name, "");
            return e;
        };
        defer img.deinit();
        let mut depth = image.Depth.U8;
        if strs.eq(depth_name, "u16") {
            depth = image.Depth.U16;
        } else if strs.eq(depth_name, "f32") {
            depth = image.Depth.F32;
        }
        if (img.width != w) or (img.height != h) or (img.channels != c) or (img.depth != depth) {
            @expect_eq(name, "");
        }
        @expect(img.data.len >= want.len);
        let mut worst: usize = 0;
        for i in @as(usize, 0)..want.len {
            let a = @as(usize, img.data[i]);
            let b = @as(usize, want[i]);
            worst = @max(worst, @max(a, b) - @min(a, b));
        }
        if worst > tol {
            @expect_eq(name, "");
        }
        cases += 1;
    }
    @expect(cases > 30);
}

fn test_image(w: usize, h: usize, ch: usize, depth: image.Depth) !image.Image {
    let mut img = image.Image.init(w, h, ch, depth)!;
    for y in @as(usize, 0)..h {
        for x in @as(usize, 0)..w {
            for c in @as(usize, 0)..ch {
                let i = (((y * w) + x) * ch) + c;
                let v = ((x * (7 + c)) + (y * 3)) + ((x / 5) * (y / 3));
                if depth == image.Depth.U8 {
                    img.data[i] = @truncate(u8, v);
                    if (x / 9) == 2 {
                        img.data[i] = 200;  // a run for PackBits
                    }
                } else if depth == image.Depth.U16 {
                    img.samples16()[i] = @truncate(u16, v * 263);
                } else {
                    img.samples32()[i] = (@float_from_int(f32, v) / 37.0) - 1.5;
                }
            }
        }
    }
    return img;
}

test "round trips" {
    let comps: [4]Compression = [Compression.NONE, Compression.LZW, Compression.DEFLATE, Compression.PACKBITS];
    let depths: [3]image.Depth = [image.Depth.U8, image.Depth.U16, image.Depth.F32];
    for d in @as(usize, 0)..3 {
        for ch in @as(usize, 1)..5 {
            let mut img = test_image(301, 250, ch, depths[d])!;
            defer img.deinit();
            for k in @as(usize, 0)..4 {
                for pr in @as(usize, 0)..2 {
                    let enc = encode(&img, Options.{ compression: comps[k], predictor: pr == 1 })!;
                    defer mem.free(enc);
                    let mut back = decode(enc)!;
                    defer back.deinit();
                    @expect((back.width == 301) and (back.height == 250) and (back.channels == ch));
                    @expect(back.depth == depths[d]);
                    @expect(strs.eq(back.data, img.data));
                }
            }
        }
    }
}

test "pages" {
    let mut a = test_image(20, 10, 3, image.Depth.U8)!;
    defer a.deinit();
    let mut b = test_image(7, 30, 1, image.Depth.U16)!;
    defer b.deinit();
    let both: [2]*image.Image = [&a, &b];
    let enc = encode_pages(both[..], Options.default())!;
    defer mem.free(enc);
    @expect_eq(page_count(enc)!, 2);
    let mut p1 = decode_page(enc, 1)!;
    defer p1.deinit();
    @expect(strs.eq(p1.data, b.data));
    @expect(fails(decode_page(enc, 2)));
}

fn fails(r: !image.Image) bool {
    let mut img = r catch return true;
    img.deinit();
    return false;
}

test "damaged files" {
    let names: [4][]u8 = ["std/graphics/image/testdata/tiff/fax_group4.tif",
                          "std/graphics/image/testdata/tiff/rgb8_lzw_be.tif",
                          "std/graphics/image/testdata/tiff/jpeg.tif",
                          "std/graphics/image/testdata/tiff/rgba8_tiled.tif"];
    for n in @as(usize, 0)..4 {
        let data = fs.read_file(names[n]) catch return;
        defer mem.free(data);
        // every truncation, and bytes changed here and there
        let mut k: usize = 0;
        while k < data.len {
            _ = fails(decode(data[..k]));
            k += 1 + (k / 16);
        }
        let copy = strs.dup(data)!;
        defer mem.free(copy);
        for i in @as(usize, 0)..copy.len {
            let old = copy[i];
            copy[i] = old ^ @truncate(u8, (i * 37) | 1);
            _ = fails(decode(copy));
            copy[i] = old;
            if i > 400 {
                break;
            }
        }
    }
}
