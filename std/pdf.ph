// =============================================================================
// std/pdf.ph - PDF documents: pages of vector graphics, text and images.
//
//     let mut doc = pdf.Document.init();
//     defer doc.deinit();
//     let mut page = doc.page(pdf.A4_WIDTH, pdf.A4_HEIGHT);
//     defer page.deinit();
//     page.text(&f, 24.0, 72.0, 100.0, "Hello, world", color.Color.rgb(0.0, 0.0, 0.0))!;
//     page.fill(&path, draw.Paint.solid(red), draw.NONZERO)!;
//     page.image(&img, 72.0, 200.0, 200.0, 150.0)!;
//     doc.add_page(&mut page)!;
//     doc.save("out.pdf")!;
//
// Coordinates are in points (1/72 inch) with the origin at the top left of
// the page and y growing downwards, as in std/draw and std/text; paths are
// draw.Paths, paints are draw.Paints (solid colours with alpha, and linear
// and radial gradients, which PDF extends by padding), strokes are
// draw.Strokes. Text is drawn with std/font fonts, TrueType or OpenType
// CFF: each font is embedded once, TrueType fonts reduced to the glyphs the
// document uses, with a ToUnicode map so the text can be searched and
// copied. A std/text Layout draws with draw_layout, in all its fonts,
// colours and directions. Images are embedded losslessly (Flate, with an
// alpha channel as a soft mask), and JPEG files as they are. Pages can
// carry links, and the document bookmarks (an outline) and metadata.
// Content streams and fonts are compressed.
//
// Reading PDF files is std/pdf/read.
// =============================================================================

import "color.ph" as color;
import "crypto/md5.ph" as md5;
import "deflate.ph" as deflate;
import "draw.ph" as draw;
import "font.ph" as font;
import "fs.ph" as fs;
import "image.ph" as image;
import "io.ph" as io;
import "mem.ph" as mem;
import "pdf/read.ph" as read;
import "str.ph" as strs;
import "text.ph" as text;
import "vec.ph" as vec;

error { InvalidFont, InvalidImage, Finished, OutOfMemory }

pub const A4_WIDTH: f64 = 595.2756;
pub const A4_HEIGHT: f64 = 841.8898;
pub const LETTER_WIDTH: f64 = 612.0;
pub const LETTER_HEIGHT: f64 = 792.0;

/// Object numbers of the fixed objects.
const CATALOG: u32 = 1;
const PAGES: u32 = 2;
const RESOURCES: u32 = 3;

// -----------------------------------------------------------------------------
// Writing numbers and strings
// -----------------------------------------------------------------------------

/// A number the way PDF wants it: no exponent, at most 4 decimals.
fn put_num(out: *mut strs.String, v: f64) !void {
    if (v != v) or (@abs(v) > 1.0e12) {
        out.push('0')!;
        return;
    }
    let neg = v < 0.0;
    let scaled = @nearest(@abs(v) * 10000.0);
    let mut ip = @int_from_float(u64, @floor(scaled / 10000.0));
    let mut fp = @int_from_float(u64, scaled - (@floor(scaled / 10000.0) * 10000.0));
    if fp >= 10000 {
        ip += 1;
        fp -= 10000;
    }
    if neg and ((ip != 0) or (fp != 0)) {
        out.push('-')!;
    }
    put_u64(out, ip)!;
    if fp != 0 {
        out.push('.')!;
        let mut d: u64 = 1000;
        let mut f = fp;
        while (f != 0) and (d != 0) {
            out.push(@truncate(u8, f / d) + '0')!;
            f %= d;
            d /= 10;
        }
    }
}

fn put_u64(out: *mut strs.String, v: u64) !void {
    let mut tmp: [20]u8 = [0; 20];
    let mut n: usize = 0;
    let mut x = v;
    while true {
        tmp[n] = @truncate(u8, x % 10) + '0';
        n += 1;
        x /= 10;
        if x == 0 {
            break;
        }
    }
    while n > 0 {
        n -= 1;
        out.push(tmp[n])!;
    }
}

fn put_int(out: *mut strs.String, v: i64) !void {
    if v < 0 {
        out.push('-')!;
        put_u64(out, @bit_cast(u64, -v))!;
    } else {
        put_u64(out, @bit_cast(u64, v))!;
    }
}

fn put_ref(out: *mut strs.String, obj: u32) !void {
    put_u64(out, @as(u64, obj))!;
    out.push_str(" 0 R")!;
}

fn put_hex4(out: *mut strs.String, v: u32) !void {
    let hex = "0123456789ABCDEF";
    let mut k: u32 = 16;
    while k > 0 {
        k -= 4;
        out.push(hex[@as(usize, (v >> k) & 15)])!;
    }
}

/// A text string (for metadata and bookmarks): plain ASCII as a literal
/// string, anything else as UTF-16BE in hex.
fn put_text_string(out: *mut strs.String, s: []u8) !void {
    let mut ascii = true;
    for c in s {
        if (c >= 0x80) or ((c < 32) and (c != '\n')) {
            ascii = false;
        }
    }
    if ascii {
        put_literal(out, s)!;
        return;
    }
    out.push_str("<FEFF")!;
    let mut it = strs.runes(s);
    while true {
        let cp = it.next() catch break;
        put_utf16(out, cp)!;
    }
    out.push('>')!;
}

fn put_utf16(out: *mut strs.String, cp: u32) !void {
    if cp >= 0x10000 {
        let v = cp - 0x10000;
        put_hex4(out, 0xd800 + (v >> 10))!;
        put_hex4(out, 0xdc00 + (v & 0x3ff))!;
    } else {
        put_hex4(out, cp)!;
    }
}

/// A literal string, (...) with \, ( and ) escaped.
fn put_literal(out: *mut strs.String, s: []u8) !void {
    out.push('(')!;
    for c in s {
        if (c == '\\') or (c == '(') or (c == ')') {
            out.push('\\')!;
            out.push(c)!;
        } else if c == '\n' {
            out.push_str("\\n")!;
        } else if c == '\r' {
            out.push_str("\\r")!;
        } else {
            out.push(c)!;
        }
    }
    out.push(')')!;
}

/// A name, /Like#20This.
fn put_name(out: *mut strs.String, s: []u8) !void {
    let hex = "0123456789ABCDEF";
    out.push('/')!;
    for c in s {
        if (c <= 32) or (c >= 127) or (strs.index_of_byte("#()<>[]{}/%", c) catch 99) != 99 {
            out.push('#')!;
            out.push(hex[@as(usize, c >> 4)])!;
            out.push(hex[@as(usize, c & 15)])!;
        } else {
            out.push(c)!;
        }
    }
}

// -----------------------------------------------------------------------------
// The document
// -----------------------------------------------------------------------------

struct FontUse {
    font: *font.Font,
    obj: u32,                // the Type0 font
    used: vec.Vec.[u8],      // per glyph: 1 if drawn
    unicode: vec.Vec.[u32],  // per glyph: the character it was drawn for (0: unknown)
}

struct Mark {
    level: u32,
    title: []mut u8,
    page: u32,  // page object
    y: f64,     // in the page's default space (y up)
}

pub struct Document {
    objs: vec.Vec.[[]mut u8],  // object n's body is objs[n - 1]
    pages: vec.Vec.[u32],
    heights: vec.Vec.[f64],
    fonts: vec.Vec.[FontUse],
    images: vec.Vec.[u32],
    gstates: vec.Vec.[u32],
    alphas: vec.Vec.[u32],  // fill alpha * 1000 << 16 | stroke alpha * 1000, per gstate
    patterns: vec.Vec.[u32],
    marks: vec.Vec.[Mark],
    info: vec.Vec.[[]mut u8],  // key, value, key, value ...
    finished: bool,
    /// Compress content streams and fonts (on by default).
    compress: bool,

    pub fn init() Document {
        let mut d = Document.{
            objs: vec.Vec.[[]mut u8].init(),
            pages: vec.Vec.[u32].init(),
            heights: vec.Vec.[f64].init(),
            fonts: vec.Vec.[FontUse].init(),
            images: vec.Vec.[u32].init(),
            gstates: vec.Vec.[u32].init(),
            alphas: vec.Vec.[u32].init(),
            patterns: vec.Vec.[u32].init(),
            marks: vec.Vec.[Mark].init(),
            info: vec.Vec.[[]mut u8].init(),
            finished: false,
            compress: true,
        };
        return d;
    }

    pub fn deinit(self: *mut Document) void {
        for o in self.objs.items() {
            mem.free(o);
        }
        self.objs.deinit();
        self.pages.deinit();
        self.heights.deinit();
        for f in self.fonts.items() {
            let mut u = f.used;
            u.deinit();
            let mut c = f.unicode;
            c.deinit();
        }
        self.fonts.deinit();
        self.images.deinit();
        self.gstates.deinit();
        self.alphas.deinit();
        self.patterns.deinit();
        for m in self.marks.items() {
            mem.free(m.title);
        }
        self.marks.deinit();
        for s in self.info.items() {
            mem.free(s);
        }
        self.info.deinit();
    }

    /// Reserves an object number (its body is set later).
    fn reserve(self: *mut Document) !u32 {
        if self.objs.len == 0 {
            // the catalog, the page tree and the shared resources
            for _k in @as(usize, 0)..3 {
                self.objs.push(mem.alloc.[u8](0)!)!;
            }
        }
        self.objs.push(mem.alloc.[u8](0)!)!;
        return @truncate(u32, self.objs.len);
    }

    fn set(self: *mut Document, obj: u32, body: *mut strs.String) !void {
        let i = @as(usize, obj) - 1;
        mem.free(self.objs.get(i));
        self.objs.set(i, body.take()!);
    }

    fn add_obj(self: *mut Document, body: *mut strs.String) !u32 {
        let n = self.reserve()!;
        self.set(n, body)!;
        return n;
    }

    /// A stream object: the dictionary entries (without << >> and /Length)
    /// and the data, compressed if it is worth it.
    fn add_stream(self: *mut Document, dict: []u8, data: []u8, may_compress: bool) !u32 {
        let mut body = strs.String.init();
        defer body.deinit();
        let mut packed: []mut u8 = mem.alloc.[u8](0)!;
        defer mem.free(packed);
        let mut payload = data;
        let mut flate = false;
        if may_compress and self.compress and (data.len > 64) {
            mem.free(packed);
            packed = deflate.compress(data, deflate.ZLIB, 6)!;
            if packed.len < data.len {
                payload = packed;
                flate = true;
            }
        }
        body.push_str("<< ")!;
        body.push_str(dict)!;
        if flate {
            body.push_str(" /Filter /FlateDecode")!;
        }
        body.push_str(" /Length ")!;
        put_u64(&mut body, @as(u64, payload.len))!;
        body.push_str(" >>\nstream\n")!;
        body.push_str(payload)!;
        body.push_str("\nendstream")!;
        return self.add_obj(&mut body);
    }

    /// Sets a metadata entry: "Title", "Author", "Subject", "Keywords",
    /// "Creator" or "Producer".
    pub fn set_info(self: *mut Document, key: []u8, value: []u8) !void {
        self.info.push(strs.dup(key)!)!;
        self.info.push(strs.dup(value)!)!;
    }

    /// A new page, width by height points; draw on it, then add_page.
    pub fn page(self: *mut Document, width: f64, height: f64) !Page {
        let mut p = Page.{
            doc: self,
            width: width,
            height: height,
            content: strs.String.init(),
            annots: vec.Vec.[u32].init(),
            ctm: draw.Affine.{ a: 1.0, b: 0.0, c: 0.0, d: -1.0, e: 0.0, f: height },
            stack: vec.Vec.[draw.Affine].init(),
            path_tmp: draw.Path.init(),
            page_obj: 0,
        };
        // y down: flip the page
        p.content.push_str("1 0 0 -1 0 ")!;
        put_num(&mut p.content, height)!;
        p.content.push_str(" cm\n")!;
        return p;
    }

    /// Adds a finished page to the end of the document.
    pub fn add_page(self: *mut Document, p: *mut Page) !void {
        if self.finished {
            return error.Finished;
        }
        let contents = self.add_stream("", p.content.bytes(), true)!;
        let obj = p.obj()!;
        let mut body = strs.String.init();
        defer body.deinit();
        body.push_str("<< /Type /Page /Parent 2 0 R /Resources 3 0 R /MediaBox [0 0 ")!;
        put_num(&mut body, p.width)!;
        body.push(' ')!;
        put_num(&mut body, p.height)!;
        body.push_str("] /Contents ")!;
        put_ref(&mut body, contents)!;
        if p.annots.len > 0 {
            body.push_str(" /Annots [")!;
            for a in p.annots.items() {
                body.push(' ')!;
                put_ref(&mut body, a)!;
            }
            body.push_str(" ]")!;
        }
        body.push_str(" >>")!;
        self.set(obj, &mut body)!;
        self.pages.push(obj)!;
        self.heights.push(p.height)!;
        p.content.clear();
        p.annots.clear();
        p.page_obj = 0;
    }

    pub fn page_count(self: *Document) usize {
        return self.pages.len;
    }

    /// A bookmark in the outline, at `level` (0 for the top), pointing at
    /// height y (from the top) of page `index`.
    pub fn bookmark(self: *mut Document, level: u32, title: []u8, index: usize, y: f64) !void {
        if index >= self.pages.len {
            return;
        }
        let obj = self.pages.get(index);
        let top = self.heights.get(index) - y;
        self.marks.push(Mark.{ level: level, title: strs.dup(title)!, page: obj, y: top })!;
    }

    fn font_use(self: *mut Document, f: *font.Font) !usize {
        for i in 0..self.fonts.len {
            if @int_from_ptr(self.fonts.get(i).font) == @int_from_ptr(f) {
                return i;
            }
        }
        let obj = self.reserve()!;
        let mut used = vec.Vec.[u8].init();
        let mut uni = vec.Vec.[u32].init();
        let n = @as(usize, f.num_glyphs);
        for _k in 0..n {
            used.push(0)!;
            uni.push(0)!;
        }
        self.fonts.push(FontUse.{ font: f, obj: obj, used: used, unicode: uni })!;
        return self.fonts.len - 1;
    }

    fn gstate(self: *mut Document, fill_alpha: f32, stroke_alpha: f32) !usize {
        let key = (@as(u32, @int_from_float(u16, @nearest(@min(@max(fill_alpha, 0.0), 1.0) * 1000.0))) << 16) | @as(
            u32, @int_from_float(u16, @nearest(@min(@max(stroke_alpha, 0.0), 1.0) * 1000.0)));
        for i in 0..self.alphas.len {
            if self.alphas.get(i) == key {
                return i;
            }
        }
        let mut body = strs.String.init();
        defer body.deinit();
        body.push_str("<< /Type /ExtGState /ca ")!;
        put_num(&mut body, @as(f64, (key >> 16)) / 1000.0)!;
        body.push_str(" /CA ")!;
        put_num(&mut body, @as(f64, key & 0xffff) / 1000.0)!;
        body.push_str(" >>")!;
        let obj = self.add_obj(&mut body)!;
        self.gstates.push(obj)!;
        self.alphas.push(key)!;
        return self.gstates.len - 1;
    }

    /// Writes the PDF file.
    pub fn save(self: *mut Document, path: []u8) !void {
        let b = self.bytes()!;
        defer mem.free(b);
        fs.write_file(path, b)!;
    }

    /// Writes the PDF to a Writer.
    pub fn write(self: *mut Document, w: io.Writer) !void {
        let b = self.bytes()!;
        defer mem.free(b);
        w.write_all(b)!;
    }

    /// The PDF file. The document is finished: no pages can be added after.
    pub fn bytes(self: *mut Document) ![]mut u8 {
        if not self.finished {
            self.finish()!;
            self.finished = true;
        }
        return self.serialize();
    }

    fn finish(self: *mut Document) !void {
        if self.objs.len == 0 {
            _ = self.reserve()!;
        }
        for i in 0..self.fonts.len {
            embed_font(self, i)!;
        }
        // shared resources
        let mut r = strs.String.init();
        defer r.deinit();
        r.push_str("<< /ProcSet [/PDF /Text /ImageB /ImageC]")!;
        if self.fonts.len > 0 {
            r.push_str(" /Font <<")!;
            for i in 0..self.fonts.len {
                r.push_str(" /F")!;
                put_u64(&mut r, @as(u64, i + 1))!;
                r.push(' ')!;
                put_ref(&mut r, self.fonts.get(i).obj)!;
            }
            r.push_str(" >>")!;
        }
        if self.images.len > 0 {
            r.push_str(" /XObject <<")!;
            for i in 0..self.images.len {
                r.push_str(" /Im")!;
                put_u64(&mut r, @as(u64, i + 1))!;
                r.push(' ')!;
                put_ref(&mut r, self.images.get(i))!;
            }
            r.push_str(" >>")!;
        }
        if self.gstates.len > 0 {
            r.push_str(" /ExtGState <<")!;
            for i in 0..self.gstates.len {
                r.push_str(" /G")!;
                put_u64(&mut r, @as(u64, i + 1))!;
                r.push(' ')!;
                put_ref(&mut r, self.gstates.get(i))!;
            }
            r.push_str(" >>")!;
        }
        if self.patterns.len > 0 {
            r.push_str(" /Pattern <<")!;
            for i in 0..self.patterns.len {
                r.push_str(" /P")!;
                put_u64(&mut r, @as(u64, i + 1))!;
                r.push(' ')!;
                put_ref(&mut r, self.patterns.get(i))!;
            }
            r.push_str(" >>")!;
        }
        r.push_str(" >>")!;
        self.set(RESOURCES, &mut r)!;
        // the page tree
        let mut t = strs.String.init();
        defer t.deinit();
        t.push_str("<< /Type /Pages /Kids [")!;
        for p in self.pages.items() {
            t.push(' ')!;
            put_ref(&mut t, p)!;
        }
        t.push_str(" ] /Count ")!;
        put_u64(&mut t, @as(u64, self.pages.len))!;
        t.push_str(" >>")!;
        self.set(PAGES, &mut t)!;
        let outline = write_outline(self)!;
        // the catalog
        let mut c = strs.String.init();
        defer c.deinit();
        c.push_str("<< /Type /Catalog /Pages 2 0 R")!;
        if outline != 0 {
            c.push_str(" /Outlines ")!;
            put_ref(&mut c, outline)!;
            c.push_str(" /PageMode /UseOutlines")!;
        }
        c.push_str(" >>")!;
        self.set(CATALOG, &mut c)!;
    }

    fn serialize(self: *mut Document) ![]mut u8 {
        let mut out = strs.String.init();
        let r = self.serialize_into(&mut out);
        r catch |e| {
            out.deinit();
            return e;
        };
        return out.take();
    }

    fn serialize_into(self: *mut Document, out: *mut strs.String) !void {
        out.push_str("%PDF-1.7\n%\xe2\xe3\xcf\xd3\n")!;
        let mut offsets = vec.Vec.[usize].init();
        defer offsets.deinit();
        for i in 0..self.objs.len {
            offsets.push(out.len)!;
            put_u64(out, @as(u64, i + 1))!;
            out.push_str(" 0 obj\n")!;
            out.push_str(self.objs.get(i))!;
            out.push_str("\nendobj\n")!;
        }
        // metadata
        let mut info: u32 = 0;
        if self.info.len > 0 {
            let mut b = strs.String.init();
            defer b.deinit();
            b.push_str("<<")!;
            let mut k: usize = 0;
            while k + 1 < self.info.len {
                b.push(' ')!;
                put_name(&mut b, self.info.get(k))!;
                b.push(' ')!;
                put_text_string(&mut b, self.info.get(k + 1))!;
                k += 2;
            }
            b.push_str(" >>")!;
            offsets.push(out.len)!;
            info = @truncate(u32, offsets.len);
            put_u64(out, @as(u64, info))!;
            out.push_str(" 0 obj\n")!;
            out.push_str(b.bytes())!;
            out.push_str("\nendobj\n")!;
        }
        let xref = out.len;
        out.push_str("xref\n0 ")!;
        put_u64(out, @as(u64, offsets.len + 1))!;
        out.push_str("\n0000000000 65535 f \n")!;
        for off in offsets.items() {
            let mut digits: [10]u8 = [0; 10];
            let mut v = off;
            let mut k: usize = 10;
            while k > 0 {
                k -= 1;
                digits[k] = @truncate(u8, v % 10) + '0';
                v /= 10;
            }
            out.push_str(digits[..])!;
            out.push_str(" 00000 n \n")!;
        }
        // a file identifier from the contents
        let mut id: [16]u8 = [0; 16];
        md5.md5(out.bytes(), id[..]);
        out.push_str("trailer\n<< /Size ")!;
        put_u64(out, @as(u64, offsets.len + 1))!;
        out.push_str(" /Root 1 0 R")!;
        if info != 0 {
            out.push_str(" /Info ")!;
            put_ref(out, info)!;
        }
        out.push_str(" /ID [<")!;
        for b in id {
            put_hex2(out, b)!;
        }
        out.push_str("> <")!;
        for b in id {
            put_hex2(out, b)!;
        }
        out.push_str(">] >>\nstartxref\n")!;
        put_u64(out, @as(u64, xref))!;
        out.push_str("\n%%EOF\n")!;
    }
}

fn put_hex2(out: *mut strs.String, b: u8) !void {
    let hex = "0123456789ABCDEF";
    out.push(hex[@as(usize, b >> 4)])!;
    out.push(hex[@as(usize, b & 15)])!;
}

/// The outline (bookmarks) as a tree by level; its root object, or 0.
fn write_outline(doc: *mut Document) !u32 {
    let n = doc.marks.len;
    if n == 0 {
        return 0;
    }
    let root = doc.reserve()!;
    let mut objs = vec.Vec.[u32].init();
    defer objs.deinit();
    for _k in 0..n {
        objs.push(doc.reserve()!)!;
    }
    // parent of each: the nearest earlier mark with a lower level
    let mut parent = vec.Vec.[usize].init();  // n means the root
    defer parent.deinit();
    let mut stack = vec.Vec.[usize].init();
    defer stack.deinit();
    for i in 0..n {
        let lv = doc.marks.get(i).level;
        while (stack.len > 0) and (doc.marks.get(stack.get(stack.len - 1)).level >= lv) {
            _ = stack.pop()!;
        }
        if stack.len > 0 {
            parent.push(stack.get(stack.len - 1))!;
        } else {
            parent.push(n)!;
        }
        stack.push(i)!;
    }
    for i in 0..n {
        let m = doc.marks.get(i);
        let mut b = strs.String.init();
        defer b.deinit();
        b.push_str("<< /Title ")!;
        put_text_string(&mut b, m.title)!;
        b.push_str(" /Parent ")!;
        let p = parent.get(i);
        if p == n {
            put_ref(&mut b, root)!;
        } else {
            put_ref(&mut b, objs.get(p))!;
        }
        // siblings and children
        let mut prev: usize = n;
        let mut k = i;
        while k > 0 {
            k -= 1;
            if parent.get(k) == p {
                prev = k;
                break;
            }
            if k == p {
                break;
            }
        }
        if prev != n {
            b.push_str(" /Prev ")!;
            put_ref(&mut b, objs.get(prev))!;
        }
        let mut next: usize = n;
        let mut first: usize = n;
        let mut last: usize = n;
        let mut count: u64 = 0;
        for j in i + 1..n {
            if (parent.get(j) == p) and (next == n) {
                next = j;
            }
            if parent.get(j) == i {
                if first == n {
                    first = j;
                }
                last = j;
                count += 1;
            }
            if doc.marks.get(j).level <= m.level {
                if parent.get(j) == p {
                    next = j;
                }
                break;
            }
        }
        if next != n {
            b.push_str(" /Next ")!;
            put_ref(&mut b, objs.get(next))!;
        }
        if first != n {
            b.push_str(" /First ")!;
            put_ref(&mut b, objs.get(first))!;
            b.push_str(" /Last ")!;
            put_ref(&mut b, objs.get(last))!;
            b.push_str(" /Count ")!;
            put_u64(&mut b, count)!;
        }
        b.push_str(" /Dest [")!;
        put_ref(&mut b, m.page)!;
        b.push_str(" /XYZ null ")!;
        put_num(&mut b, m.y)!;
        b.push_str(" null] >>")!;
        doc.set(objs.get(i), &mut b)!;
    }
    let mut b = strs.String.init();
    defer b.deinit();
    b.push_str("<< /Type /Outlines")!;
    let mut first: usize = n;
    let mut last: usize = n;
    let mut count: u64 = 0;
    for i in 0..n {
        if parent.get(i) == n {
            if first == n {
                first = i;
            }
            last = i;
            count += 1;
        }
    }
    b.push_str(" /First ")!;
    put_ref(&mut b, objs.get(first))!;
    b.push_str(" /Last ")!;
    put_ref(&mut b, objs.get(last))!;
    b.push_str(" /Count ")!;
    put_u64(&mut b, count)!;
    b.push_str(" >>")!;
    doc.set(root, &mut b)!;
    return root;
}

// -----------------------------------------------------------------------------
// Pages
// -----------------------------------------------------------------------------

pub struct Page {
    doc: *mut Document,
    width: f64,
    height: f64,
    content: strs.String,
    annots: vec.Vec.[u32],
    ctm: draw.Affine,  // user space to the page's default space
    stack: vec.Vec.[draw.Affine],
    path_tmp: draw.Path,
    page_obj: u32,

    pub fn deinit(self: *mut Page) void {
        self.content.deinit();
        self.annots.deinit();
        self.stack.deinit();
        self.path_tmp.deinit();
    }

    /// The page's object number (reserved on first use, for links to it).
    fn obj(self: *mut Page) !u32 {
        if self.page_obj == 0 {
            self.page_obj = self.doc.reserve()!;
        }
        return self.page_obj;
    }

    fn op(self: *mut Page, s: []u8) !void {
        self.content.push_str(s)!;
    }

    fn num(self: *mut Page, v: f64) !void {
        put_num(&mut self.content, v)!;
        self.content.push(' ')!;
    }

    // --- graphics state ---

    /// Saves the graphics state (transform, clip, ...) until restore.
    pub fn save(self: *mut Page) !void {
        self.stack.push(self.ctm)!;
        self.op("q\n")!;
    }

    pub fn restore(self: *mut Page) !void {
        let t = self.stack.pop() catch return;
        self.ctm = t;
        self.op("Q\n")!;
    }

    /// Transforms what follows by t (in user space, before the current transform).
    pub fn concat(self: *mut Page, t: draw.Affine) !void {
        self.ctm = t.then(self.ctm);
        self.num(t.a)!;
        self.num(t.b)!;
        self.num(t.c)!;
        self.num(t.d)!;
        self.num(t.e)!;
        self.num(t.f)!;
        self.op("cm\n")!;
    }

    pub fn translate(self: *mut Page, tx: f64, ty: f64) !void {
        self.concat(draw.Affine.translation(tx, ty))!;
    }

    pub fn scale(self: *mut Page, sx: f64, sy: f64) !void {
        self.concat(draw.Affine.scaling(sx, sy))!;
    }

    pub fn rotate(self: *mut Page, radians: f64) !void {
        self.concat(draw.Affine.rotation(radians))!;
    }

    // --- paths ---

    fn path(self: *mut Page, p: *draw.Path) !void {
        let verbs = p.verbs.items();
        let pts = p.pts.items();
        let mut k: usize = 0;
        let mut cx: f64 = 0.0;
        let mut cy: f64 = 0.0;
        for v in verbs {
            if v == draw.V_MOVE {
                cx = pts[k];
                cy = pts[k + 1];
                self.num(cx)!;
                self.num(cy)!;
                self.op("m\n")!;
                k += 2;
            } else if v == draw.V_LINE {
                cx = pts[k];
                cy = pts[k + 1];
                self.num(cx)!;
                self.num(cy)!;
                self.op("l\n")!;
                k += 2;
            } else if v == draw.V_QUAD {
                // as a cubic
                let qx = pts[k];
                let qy = pts[k + 1];
                let x = pts[k + 2];
                let y = pts[k + 3];
                self.num(cx + ((2.0 / 3.0) * (qx - cx)))!;
                self.num(cy + ((2.0 / 3.0) * (qy - cy)))!;
                self.num(x + ((2.0 / 3.0) * (qx - x)))!;
                self.num(y + ((2.0 / 3.0) * (qy - y)))!;
                self.num(x)!;
                self.num(y)!;
                self.op("c\n")!;
                cx = x;
                cy = y;
                k += 4;
            } else if v == draw.V_CUBIC {
                for j in @as(usize, 0)..6 {
                    self.num(pts[k + j])!;
                }
                cx = pts[k + 4];
                cy = pts[k + 5];
                self.op("c\n")!;
                k += 6;
            } else if v == draw.V_CLOSE {
                self.op("h\n")!;
            }
        }
    }

    /// Sets the fill (or stroke) paint; returns false if nothing would show.
    fn paint(self: *mut Page, p: draw.Paint, stroke: bool) !bool {
        if p.kind == draw.PAINT_SOLID {
            let c = p.color;
            if c.a <= 0.0 {
                return false;
            }
            self.alpha(c.a, stroke)!;
            self.num(@as(f64, color.srgb_encode(@min(@max(c.r, 0.0), 1.0))))!;
            self.num(@as(f64, color.srgb_encode(@min(@max(c.g, 0.0), 1.0))))!;
            self.num(@as(f64, color.srgb_encode(@min(@max(c.b, 0.0), 1.0))))!;
            if stroke {
                self.op("RG\n")!;
            } else {
                self.op("rg\n")!;
            }
            return true;
        }
        if (p.kind == draw.PAINT_LINEAR) or (p.kind == draw.PAINT_RADIAL) {
            if p.stops.len == 0 {
                return false;
            }
            let idx = gradient(self.doc, p, self.ctm)!;
            self.alpha(1.0, stroke)!;
            if stroke {
                self.op("/Pattern CS /P")!;
            } else {
                self.op("/Pattern cs /P")!;
            }
            put_u64(&mut self.content, @as(u64, idx + 1))!;
            if stroke {
                self.op(" SCN\n")!;
            } else {
                self.op(" scn\n")!;
            }
            return true;
        }
        return false;  // image paints are not supported
    }

    fn alpha(self: *mut Page, a: f32, stroke: bool) !void {
        // one graphics state per alpha; opaque drawing resets it
        let mut fa: f32 = 1.0;
        let mut sa: f32 = 1.0;
        if stroke {
            sa = a;
        } else {
            fa = a;
        }
        let g = self.doc.gstate(fa, sa)!;
        self.op("/G")!;
        put_u64(&mut self.content, @as(u64, g + 1))!;
        self.op(" gs\n")!;
    }

    /// Fills a path (rule: draw.NONZERO or draw.EVEN_ODD).
    pub fn fill(self: *mut Page, p: *draw.Path, paint: draw.Paint, rule: u8) !void {
        if not self.paint(paint, false)! {
            return;
        }
        self.path(p)!;
        if rule == draw.EVEN_ODD {
            self.op("f*\n")!;
        } else {
            self.op("f\n")!;
        }
    }

    /// Strokes a path.
    pub fn stroke(self: *mut Page, p: *draw.Path, paint: draw.Paint, style: draw.Stroke) !void {
        if not self.paint(paint, true)! {
            return;
        }
        self.num(style.width)!;
        self.op("w ")!;
        put_u64(&mut self.content, @as(u64, style.cap))!;
        self.op(" J ")!;
        put_u64(&mut self.content, @as(u64, style.join))!;
        self.op(" j ")!;
        self.num(@max(style.miter_limit, 1.0))!;
        self.op("M [")!;
        for d in style.dash {
            self.num(d)!;
        }
        self.op("] ")!;
        self.num(style.dash_offset)!;
        self.op("d\n")!;
        self.path(p)!;
        self.op("S\n")!;
    }

    pub fn fill_rect(self: *mut Page, x: f64, y: f64, w: f64, h: f64, paint: draw.Paint) !void {
        self.path_tmp.clear();
        self.path_tmp.rect(x, y, w, h)!;
        let tmp = self.path_tmp;
        self.fill(&tmp, paint, draw.NONZERO)!;
    }

    /// Limits what follows (until restore) to the inside of a path.
    pub fn clip(self: *mut Page, p: *draw.Path, rule: u8) !void {
        self.path(p)!;
        if rule == draw.EVEN_ODD {
            self.op("W* n\n")!;
        } else {
            self.op("W n\n")!;
        }
    }

    // --- text ---

    /// Draws a line of text with its baseline starting at (x, y).
    pub fn text(self: *mut Page, f: *font.Font, size: f64, x: f64, y: f64, s: []u8, c: color.Color) !void {
        if not self.paint(draw.Paint.solid(c), false)! {
            return;
        }
        let fi = self.doc.font_use(f)!;
        self.op("BT /F")!;
        put_u64(&mut self.content, @as(u64, fi + 1))!;
        self.op(" ")!;
        self.num(size)!;
        self.op("Tf 1 0 0 -1 ")!;
        self.num(x)!;
        self.num(y)!;
        self.op("Tm [<")!;
        let upem = @float_from_int(f64, f.units_per_em);
        let mut prev: u32 = 0;
        let mut it = strs.runes(s);
        while true {
            let cp = it.next() catch break;
            let g = f.glyph_index(cp);
            if prev != 0 {
                let k = f.kerning(prev, g);
                if k != 0 {
                    self.op("> ")!;
                    self.num(-@float_from_int(f64, k) * 1000.0 / upem)!;
                    self.op("<")!;
                }
            }
            self.glyph(fi, g, cp)!;
            prev = g;
        }
        self.op(">] TJ ET\n")!;
    }

    fn glyph(self: *mut Page, fi: usize, g: u32, cp: u32) !void {
        let fu = self.doc.fonts.get_ptr(fi);
        if @as(usize, g) < fu.used.len {
            fu.used.set(@as(usize, g), 1);
            if fu.unicode.get(@as(usize, g)) == 0 {
                fu.unicode.set(@as(usize, g), cp);
            }
        }
        put_hex4(&mut self.content, g & 0xffff)!;
    }

    /// Draws a std/text Layout with its top left at (x, y).
    pub fn draw_layout(self: *mut Page, l: *text.Layout, x: f64, y: f64) !void {
        let gs = l.glyphs.items();
        let spans = l.spans.items();
        // where each span starts in the concatenated text
        let mut starts = vec.Vec.[usize].init();
        defer starts.deinit();
        let mut total: usize = 0;
        for sp in spans {
            starts.push(total)!;
            total += sp.text.len;
        }
        let mut i: usize = 0;
        while i < gs.len {
            // a run: one span (font, size, colour) on one baseline
            let g0 = gs[i];
            let st = spans[g0.span].style;
            let f = g0.font;
            let fi = self.doc.font_use(f)!;
            if not self.paint(draw.Paint.solid(st.color), false)! {
                i += 1;
                continue;
            }
            let scale = g0.size / @float_from_int(f64, f.units_per_em);
            self.op("BT /F")!;
            put_u64(&mut self.content, @as(u64, fi + 1))!;
            self.op(" ")!;
            self.num(g0.size)!;
            self.op("Tf 1 0 0 -1 ")!;
            self.num(x + g0.x)!;
            self.num(y + g0.y)!;
            self.op("Tm [<")!;
            let mut pen = g0.x;
            while i < gs.len {
                let g = gs[i];
                if (g.span != g0.span) or (@int_from_ptr(g.font) != @int_from_ptr(f)) or (g.y != g0.y) or
                   (g.size != g0.size) {
                    break;
                }
                let d = g.x - pen;
                if @abs(d) > 0.001 {
                    self.op("> ")!;
                    self.num(-d * 1000.0 / g.size)!;
                    self.op("<")!;
                }
                let sp = spans[g.span];
                let off = g.offset - starts.get(g.span);
                let mut cp: u32 = 0;
                if off < sp.text.len {
                    cp = strs.decode_rune(sp.text[off..]).value;
                }
                self.glyph(fi, g.id, cp)!;
                pen = g.x + (@float_from_int(f64, f.advance(g.id)) * scale);
                i += 1;
            }
            self.op(">] TJ ET\n")!;
        }
        // underlines and strike-throughs
        for g in gs {
            let st = spans[g.span].style;
            if not st.underline and not st.strike {
                continue;
            }
            let thick = @max(st.size / 16.0, 0.5);
            let paint = draw.Paint.solid(st.color);
            if st.underline {
                self.fill_rect(x + g.x, y + g.y + (st.size * 0.12), g.advance, thick, paint)!;
            }
            if st.strike {
                self.fill_rect(x + g.x, (y + g.y) - (st.size * 0.28), g.advance, thick, paint)!;
            }
        }
    }

    // --- images ---

    /// Draws an image into the rectangle with top left (x, y), w by h.
    pub fn image(self: *mut Page, img: *image.Image, x: f64, y: f64, w: f64, h: f64) !void {
        let idx = add_image(self.doc, img)!;
        self.place_image(idx, x, y, w, h)!;
    }

    /// Draws a JPEG file (embedded as it is) into the rectangle.
    pub fn jpeg(self: *mut Page, data: []u8, x: f64, y: f64, w: f64, h: f64) !void {
        let idx = add_jpeg(self.doc, data)!;
        self.place_image(idx, x, y, w, h)!;
    }

    fn place_image(self: *mut Page, idx: usize, x: f64, y: f64, w: f64, h: f64) !void {
        self.alpha(1.0, false)!;
        self.op("q ")!;
        self.num(w)!;
        self.op("0 0 ")!;
        self.num(-h)!;
        self.num(x)!;
        self.num(y + h)!;
        self.op("cm /Im")!;
        put_u64(&mut self.content, @as(u64, idx + 1))!;
        self.op(" Do Q\n")!;
    }

    // --- links ---

    /// Makes a rectangle of the page a link to a URL.
    pub fn link(self: *mut Page, x: f64, y: f64, w: f64, h: f64, url: []u8) !void {
        let mut b = strs.String.init();
        defer b.deinit();
        b.push_str("<< /Type /Annot /Subtype /Link /Border [0 0 0] /Rect [")!;
        self.put_rect(&mut b, x, y, w, h)!;
        b.push_str("] /A << /S /URI /URI ")!;
        put_literal(&mut b, url)!;
        b.push_str(" >> >>")!;
        let a = self.doc.add_obj(&mut b)!;
        self.annots.push(a)!;
    }

    /// Makes a rectangle of the page a link to height y (from the top) of
    /// an earlier page.
    pub fn link_to_page(self: *mut Page, x: f64, y: f64, w: f64, h: f64, index: usize, target_y: f64) !void {
        if index >= self.doc.pages.len {
            return;
        }
        let mut b = strs.String.init();
        defer b.deinit();
        b.push_str("<< /Type /Annot /Subtype /Link /Border [0 0 0] /Rect [")!;
        self.put_rect(&mut b, x, y, w, h)!;
        b.push_str("] /Dest [")!;
        put_ref(&mut b, self.doc.pages.get(index))!;
        b.push_str(" /XYZ null ")!;
        put_num(&mut b, self.doc.heights.get(index) - target_y)!;
        b.push_str(" null] >>")!;
        let a = self.doc.add_obj(&mut b)!;
        self.annots.push(a)!;
    }

    /// The bounding box of a user-space rectangle in the page's default space.
    fn put_rect(self: *mut Page, b: *mut strs.String, x: f64, y: f64, w: f64, h: f64) !void {
        let p0 = self.ctm.apply(x, y);
        let p1 = self.ctm.apply(x + w, y);
        let p2 = self.ctm.apply(x, y + h);
        let p3 = self.ctm.apply(x + w, y + h);
        put_num(b, @min(@min(p0.x, p1.x), @min(p2.x, p3.x)))!;
        b.push(' ')!;
        put_num(b, @min(@min(p0.y, p1.y), @min(p2.y, p3.y)))!;
        b.push(' ')!;
        put_num(b, @max(@max(p0.x, p1.x), @max(p2.x, p3.x)))!;
        b.push(' ')!;
        put_num(b, @max(@max(p0.y, p1.y), @max(p2.y, p3.y)))!;
    }
}

// -----------------------------------------------------------------------------
// Gradients
// -----------------------------------------------------------------------------

fn put_color_components(b: *mut strs.String, c: color.Color) !void {
    b.push('[')!;
    put_num(b, @as(f64, color.srgb_encode(@min(@max(c.r, 0.0), 1.0))))!;
    b.push(' ')!;
    put_num(b, @as(f64, color.srgb_encode(@min(@max(c.g, 0.0), 1.0))))!;
    b.push(' ')!;
    put_num(b, @as(f64, color.srgb_encode(@min(@max(c.b, 0.0), 1.0))))!;
    b.push(']')!;
}

/// A shading pattern for a gradient paint; its index.
fn gradient(doc: *mut Document, p: draw.Paint, ctm: draw.Affine) !usize {
    let mut b = strs.String.init();
    defer b.deinit();
    // the colour function: stitched linear pieces between the stops
    let stops = p.stops;
    let mut fun = strs.String.init();
    defer fun.deinit();
    if stops.len == 1 {
        fun.push_str("<< /FunctionType 2 /Domain [0 1] /N 1 /C0 ")!;
        put_color_components(&mut fun, stops[0].color)!;
        fun.push_str(" /C1 ")!;
        put_color_components(&mut fun, stops[0].color)!;
        fun.push_str(" >>")!;
    } else {
        fun.push_str("<< /FunctionType 3 /Domain [0 1] /Functions [")!;
        for i in 0..stops.len - 1 {
            fun.push_str(" << /FunctionType 2 /Domain [0 1] /N 1 /C0 ")!;
            put_color_components(&mut fun, stops[i].color)!;
            fun.push_str(" /C1 ")!;
            put_color_components(&mut fun, stops[i + 1].color)!;
            fun.push_str(" >>")!;
        }
        fun.push_str(" ] /Bounds [")!;
        for i in 1..stops.len - 1 {
            fun.push(' ')!;
            put_num(&mut fun, @min(@max(stops[i].offset, 0.0), 1.0))!;
        }
        fun.push_str(" ] /Encode [")!;
        for _k in 0..stops.len - 1 {
            fun.push_str(" 0 1")!;
        }
        fun.push_str(" ] >>")!;
    }
    b.push_str("<< /PatternType 2 /Matrix [")!;
    let m = p.transform.then(ctm);
    put_num(&mut b, m.a)!;
    b.push(' ')!;
    put_num(&mut b, m.b)!;
    b.push(' ')!;
    put_num(&mut b, m.c)!;
    b.push(' ')!;
    put_num(&mut b, m.d)!;
    b.push(' ')!;
    put_num(&mut b, m.e)!;
    b.push(' ')!;
    put_num(&mut b, m.f)!;
    b.push_str("] /Shading << /ColorSpace /DeviceRGB /Extend [true true] ")!;
    if p.kind == draw.PAINT_LINEAR {
        b.push_str("/ShadingType 2 /Coords [")!;
        put_num(&mut b, p.x0)!;
        b.push(' ')!;
        put_num(&mut b, p.y0)!;
        b.push(' ')!;
        put_num(&mut b, p.x1)!;
        b.push(' ')!;
        put_num(&mut b, p.y1)!;
    } else {
        b.push_str("/ShadingType 3 /Coords [")!;
        put_num(&mut b, p.x0)!;
        b.push(' ')!;
        put_num(&mut b, p.y0)!;
        b.push_str(" 0 ")!;
        put_num(&mut b, p.x0)!;
        b.push(' ')!;
        put_num(&mut b, p.y0)!;
        b.push(' ')!;
        put_num(&mut b, p.r)!;
    }
    b.push_str("] /Function ")!;
    b.push_str(fun.bytes())!;
    b.push_str(" >> >>")!;
    let obj = doc.add_obj(&mut b)!;
    doc.patterns.push(obj)!;
    return doc.patterns.len - 1;
}

// -----------------------------------------------------------------------------
// Images
// -----------------------------------------------------------------------------

fn add_image(doc: *mut Document, img: *image.Image) !usize {
    let alpha = (img.channels == 2) or (img.channels == 4);
    let gray = img.channels <= 2;
    let mut conv = img.convert(img.channels, image.U8)!;
    defer conv.deinit();
    let n = img.width * img.height;
    let cc: usize = if_gray(gray);
    let mut colour = mem.alloc.[u8](n * cc)!;
    defer mem.free(colour);
    let mut mask = mem.alloc.[u8](n * @as(usize, bool_usize(alpha)))!;
    defer mem.free(mask);
    let src = conv.data;
    let ch = img.channels;
    for p in 0..n {
        for c in 0..cc {
            colour[(p * cc) + c] = src[(p * ch) + c];
        }
        if alpha {
            mask[p] = src[(p * ch) + ch - 1];
        }
    }
    let mut smask: u32 = 0;
    if alpha {
        let mut d = strs.String.init();
        defer d.deinit();
        d.push_str("/Type /XObject /Subtype /Image /Width ")!;
        put_u64(&mut d, @as(u64, img.width))!;
        d.push_str(" /Height ")!;
        put_u64(&mut d, @as(u64, img.height))!;
        d.push_str(" /ColorSpace /DeviceGray /BitsPerComponent 8")!;
        smask = doc.add_stream(d.bytes(), mask, true)!;
    }
    let mut d = strs.String.init();
    defer d.deinit();
    d.push_str("/Type /XObject /Subtype /Image /Width ")!;
    put_u64(&mut d, @as(u64, img.width))!;
    d.push_str(" /Height ")!;
    put_u64(&mut d, @as(u64, img.height))!;
    if gray {
        d.push_str(" /ColorSpace /DeviceGray")!;
    } else {
        d.push_str(" /ColorSpace /DeviceRGB")!;
    }
    d.push_str(" /BitsPerComponent 8")!;
    if smask != 0 {
        d.push_str(" /SMask ")!;
        put_ref(&mut d, smask)!;
    }
    let obj = doc.add_stream(d.bytes(), colour, true)!;
    doc.images.push(obj)!;
    return doc.images.len - 1;
}

fn if_gray(gray: bool) usize {
    if gray {
        return 1;
    }
    return 3;
}

fn bool_usize(b: bool) usize {
    if b {
        return 1;
    }
    return 0;
}

/// Width, height and components from a JPEG's frame header.
fn jpeg_info(data: []u8, w: *mut usize, h: *mut usize, comps: *mut usize, adobe: *mut bool) bool {
    if (data.len < 4) or (data[0] != 0xff) or (data[1] != 0xd8) {
        return false;
    }
    let mut i: usize = 2;
    while i + 4 <= data.len {
        if data[i] != 0xff {
            return false;
        }
        let m = data[i + 1];
        if m == 0xff {
            i += 1;
            continue;
        }
        let len = (@as(usize, data[i + 2]) << 8) | @as(usize, data[i + 3]);
        if len < 2 {
            return false;
        }
        if (m == 0xee) and (i + 9 <= data.len) and @mem_eq(data[i + 4..i + 9], "Adobe") {
            adobe.* = true;
        }
        if ((m >= 0xc0) and (m <= 0xcf)) and (m != 0xc4) and (m != 0xc8) and (m != 0xcc) {
            if i + 10 > data.len {
                return false;
            }
            h.* = (@as(usize, data[i + 5]) << 8) | @as(usize, data[i + 6]);
            w.* = (@as(usize, data[i + 7]) << 8) | @as(usize, data[i + 8]);
            comps.* = @as(usize, data[i + 9]);
            return (w.* > 0) and (h.* > 0) and ((comps.* == 1) or (comps.* == 3) or (comps.* == 4));
        }
        i += 2 + len;
    }
    return false;
}

fn add_jpeg(doc: *mut Document, data: []u8) !usize {
    let mut w: usize = 0;
    let mut h: usize = 0;
    let mut comps: usize = 0;
    let mut adobe = false;
    if not jpeg_info(data, &mut w, &mut h, &mut comps, &mut adobe) {
        return error.InvalidImage;
    }
    let mut d = strs.String.init();
    defer d.deinit();
    d.push_str("/Type /XObject /Subtype /Image /Width ")!;
    put_u64(&mut d, @as(u64, w))!;
    d.push_str(" /Height ")!;
    put_u64(&mut d, @as(u64, h))!;
    if comps == 1 {
        d.push_str(" /ColorSpace /DeviceGray")!;
    } else if comps == 3 {
        d.push_str(" /ColorSpace /DeviceRGB")!;
    } else {
        d.push_str(" /ColorSpace /DeviceCMYK")!;
        if adobe {
            d.push_str(" /Decode [1 0 1 0 1 0 1 0]")!;
        }
    }
    d.push_str(" /BitsPerComponent 8 /Filter /DCTDecode")!;
    let obj = doc.add_stream(d.bytes(), data, false)!;
    doc.images.push(obj)!;
    return doc.images.len - 1;
}

// -----------------------------------------------------------------------------
// Fonts
// -----------------------------------------------------------------------------

fn be16(d: []u8, i: usize) u32 {
    if i + 2 > d.len {
        return 0;
    }
    return (@as(u32, d[i]) << 8) | @as(u32, d[i + 1]);
}

fn be32(d: []u8, i: usize) u32 {
    return (be16(d, i) << 16) | be16(d, i + 2);
}

fn sbe16(d: []u8, i: usize) i64 {
    return @as(i64, @bit_cast(i16, @truncate(u16, be16(d, i))));
}

fn push16(b: *mut strs.String, v: u32) !void {
    b.push(@truncate(u8, v >> 8))!;
    b.push(@truncate(u8, v))!;
}

fn push32(b: *mut strs.String, v: u32) !void {
    push16(b, v >> 16)!;
    push16(b, v & 0xffff)!;
}

/// The font's PostScript name (name ID 6), made safe for a PDF name.
fn postscript_name(f: *font.Font, out: *mut strs.String) !void {
    let t = f.table("name");
    let count = @as(usize, be16(t, 2));
    let strings = @as(usize, be16(t, 4));
    let start = out.len;
    for i in 0..count {
        let r = 6 + (12 * i);
        if r + 12 > t.len {
            break;
        }
        let platform = be16(t, r);
        let id = be16(t, r + 6);
        let len = @as(usize, be16(t, r + 8));
        let off = strings + @as(usize, be16(t, r + 10));
        if (id != 6) or (off + len > t.len) {
            continue;
        }
        let s = t[off..off + len];
        let mut k: usize = 0;
        let wide = (platform == 0) or (platform == 3);
        while k < s.len {
            let mut c = s[k];
            if wide {
                c = s[@min(k + 1, s.len - 1)];
                if s[k] != 0 {
                    c = '_';
                }
                k += 2;
            } else {
                k += 1;
            }
            if (c > 32) and (c < 127) and (strs.index_of_byte("#()<>[]{}/%", c) catch 99) == 99 {
                out.push(c)!;
            }
        }
        if out.len > start {
            return;
        }
    }
    out.push_str("Font")!;
}

/// Glyphs referenced by composite glyphs of the used ones become used too.
fn close_composites(f: *font.Font, used: []mut u8) void {
    let glyf = f.table("glyf");
    let loca = f.table("loca");
    let long = f.loc_long;
    let n = used.len;
    // components can nest; repeat until nothing changes (bounded)
    let mut changed = true;
    let mut rounds: u32 = 0;
    while changed and (rounds < 16) {
        changed = false;
        rounds += 1;
        for g in 0..n {
            if used[g] == 0 {
                continue;
            }
            let mut a: usize = 0;
            let mut b: usize = 0;
            if not glyph_range(loca, long, g, &mut a, &mut b) or (b > glyf.len) or (b - a < 10) {
                continue;
            }
            if sbe16(glyf, a) >= 0 {
                continue;  // simple glyph
            }
            let mut p = a + 10;
            while p + 4 <= b {
                let flags = be16(glyf, p);
                let comp = @as(usize, be16(glyf, p + 2));
                if (comp < n) and (used[comp] == 0) {
                    used[comp] = 1;
                    changed = true;
                }
                p += 4;
                if (flags & 1) != 0 {
                    p += 4;
                } else {
                    p += 2;
                }
                if (flags & 8) != 0 {
                    p += 2;
                } else if (flags & 0x40) != 0 {
                    p += 4;
                } else if (flags & 0x80) != 0 {
                    p += 8;
                }
                if (flags & 0x20) == 0 {
                    break;
                }
            }
        }
    }
}

fn glyph_range(loca: []u8, long: bool, g: usize, a: *mut usize, b: *mut usize) bool {
    if long {
        if (g + 2) * 4 > loca.len {
            return false;
        }
        a.* = @as(usize, be32(loca, g * 4));
        b.* = @as(usize, be32(loca, (g + 1) * 4));
    } else {
        if (g + 2) * 2 > loca.len {
            return false;
        }
        a.* = @as(usize, be16(loca, g * 2)) * 2;
        b.* = @as(usize, be16(loca, (g + 1) * 2)) * 2;
    }
    return a.* <= b.*;
}

struct TableOut {
    tag: []u8,
    data: []u8,
}

/// A standalone font file: for TrueType the tables a PDF needs, with the
/// unused glyphs emptied; for CFF all tables.
fn build_font_file(f: *font.Font, used: []u8, out: *mut strs.String) !void {
    let mut tables = vec.Vec.[TableOut].init();
    defer tables.deinit();
    let mut glyf = strs.String.init();
    defer glyf.deinit();
    let mut loca = strs.String.init();
    defer loca.deinit();
    let mut head = strs.String.init();
    defer head.deinit();
    if not f.is_cff {
        let og = f.table("glyf");
        let ol = f.table("loca");
        let n = @as(usize, f.num_glyphs);
        for g in 0..n {
            push32(&mut loca, @truncate(u32, glyf.len))!;
            let mut a: usize = 0;
            let mut b: usize = 0;
            if (g < used.len) and (used[g] != 0) and glyph_range(ol, f.loc_long, g, &mut a, &mut b) and (b <= og.len) {
                glyf.push_str(og[a..b])!;
                while (glyf.len % 4) != 0 {
                    glyf.push(0)!;
                }
            }
        }
        push32(&mut loca, @truncate(u32, glyf.len))!;
        head.push_str(f.table("head"))!;
        if head.len >= 54 {
            // long offsets; the checksum adjustment is recomputed below
            head.buf[50] = 0;
            head.buf[51] = 1;
            for k in @as(usize, 8)..12 {
                head.buf[k] = 0;
            }
        }
        let keep: [7][]u8 = ["cvt ", "fpgm", "hhea", "hmtx", "maxp", "prep", "OS/2"];
        tables.push(TableOut.{ tag: "glyf", data: glyf.bytes() })!;
        tables.push(TableOut.{ tag: "head", data: head.bytes() })!;
        tables.push(TableOut.{ tag: "loca", data: loca.bytes() })!;
        for t in keep {
            let d = f.table(t);
            if d.len > 0 {
                tables.push(TableOut.{ tag: t, data: d })!;
            }
        }
    } else {
        for i in 0..f.table_count() {
            let tag = f.table_tag(i);
            let d = f.table(tag);
            if (tag.len == 4) and (d.len > 0) {
                tables.push(TableOut.{ tag: tag, data: d })!;
            }
        }
    }
    // sorted by tag
    let ts = tables.items();
    for i in 1..ts.len {
        let mut j = i;
        while (j > 0) and (strs.compare(ts[j - 1].tag, ts[j].tag) > 0) {
            let t = ts[j];
            ts[j] = ts[j - 1];
            ts[j - 1] = t;
            j -= 1;
        }
    }
    let count = @truncate(u32, ts.len);
    let mut sel: u32 = 0;
    while (@as(u32, 1) << (sel + 1)) <= count {
        sel += 1;
    }
    let range = (@as(u32, 1) << sel) * 16;
    if f.is_cff {
        out.push_str("OTTO")!;
    } else {
        push32(out, 0x00010000)!;
    }
    push16(out, count)!;
    push16(out, range)!;
    push16(out, sel)!;
    push16(out, (count * 16) - range)!;
    let dir = out.len;
    let mut off = 12 + (16 * ts.len);
    for t in ts {
        out.push_str(t.tag)!;
        push32(out, checksum(t.data))!;
        push32(out, @truncate(u32, off))!;
        push32(out, @truncate(u32, t.data.len))!;
        off += (t.data.len + 3) & ~@as(usize, 3);
    }
    _ = dir;
    let mut head_at: usize = 0;
    for t in ts {
        if strs.eq(t.tag, "head") {
            head_at = out.len;
        }
        out.push_str(t.data)!;
        while (out.len % 4) != 0 {
            out.push(0)!;
        }
    }
    if (head_at != 0) and (out.len >= head_at + 12) {
        let adj = 0xb1b0afba -% checksum(out.bytes());
        out.buf[head_at + 8] = @truncate(u8, adj >> 24);
        out.buf[head_at + 9] = @truncate(u8, adj >> 16);
        out.buf[head_at + 10] = @truncate(u8, adj >> 8);
        out.buf[head_at + 11] = @truncate(u8, adj);
    }
}

fn checksum(d: []u8) u32 {
    let mut sum: u32 = 0;
    let mut i: usize = 0;
    while i < d.len {
        let mut w: u32 = 0;
        for k in @as(usize, 0)..4 {
            w <<= 8;
            if i + k < d.len {
                w |= @as(u32, d[i + k]);
            }
        }
        sum +%= w;
        i += 4;
    }
    return sum;
}

fn embed_font(doc: *mut Document, i: usize) !void {
    let fu = doc.fonts.get(i);
    let f = fu.font;
    let used = fu.used.items();
    used[0] = 1;  // .notdef
    if not f.is_cff {
        close_composites(f, used);
    }
    // a subset tag from the glyphs used
    let mut hsh: u32 = 2166136261;
    for g in 0..used.len {
        if used[g] != 0 {
            hsh = (hsh ^ @truncate(u32, g)) *% 16777619;
        }
    }
    let mut name = strs.String.init();
    defer name.deinit();
    if not f.is_cff {
        for _k in @as(usize, 0)..6 {
            name.push('A' + @truncate(u8, hsh % 26))!;
            hsh /= 26;
        }
        name.push('+')!;
    }
    postscript_name(f, &mut name)!;
    // the font file
    let mut file = strs.String.init();
    defer file.deinit();
    build_font_file(f, used, &mut file)!;
    let mut fd = strs.String.init();
    defer fd.deinit();
    let file_obj: u32 = 0;
    let mut ff: u32 = file_obj;
    if f.is_cff {
        ff = doc.add_stream("/Subtype /OpenType", file.bytes(), true)!;
    } else {
        let mut d = strs.String.init();
        defer d.deinit();
        d.push_str("/Length1 ")!;
        put_u64(&mut d, @as(u64, file.len))!;
        ff = doc.add_stream(d.bytes(), file.bytes(), true)!;
    }
    let upem = @float_from_int(f64, f.units_per_em);
    let k = 1000.0 / upem;
    let head = f.table("head");
    fd.push_str("<< /Type /FontDescriptor /FontName ")!;
    put_name(&mut fd, name.bytes())!;
    fd.push_str(" /Flags 4 /FontBBox [")!;
    for j in @as(usize, 0)..4 {
        if j > 0 {
            fd.push(' ')!;
        }
        put_num(&mut fd, @float_from_int(f64, sbe16(head, 36 + (2 * j))) * k)!;
    }
    fd.push_str("] /ItalicAngle ")!;
    let post = f.table("post");
    let mut italic: f64 = 0.0;
    if post.len >= 8 {
        italic = @float_from_int(f64, @as(i64, @bit_cast(i32, be32(post, 4)))) / 65536.0;
    }
    put_num(&mut fd, italic)!;
    fd.push_str(" /Ascent ")!;
    put_num(&mut fd, @float_from_int(f64, f.ascent) * k)!;
    fd.push_str(" /Descent ")!;
    put_num(&mut fd, @float_from_int(f64, f.descent) * k)!;
    let os2 = f.table("OS/2");
    let mut cap = @float_from_int(f64, f.ascent) * k;
    if (os2.len >= 90) and (be16(os2, 0) >= 2) {
        cap = @float_from_int(f64, sbe16(os2, 88)) * k;
    }
    fd.push_str(" /CapHeight ")!;
    put_num(&mut fd, cap)!;
    fd.push_str(" /StemV 80 ")!;
    if f.is_cff {
        fd.push_str("/FontFile3 ")!;
    } else {
        fd.push_str("/FontFile2 ")!;
    }
    put_ref(&mut fd, ff)!;
    fd.push_str(" >>")!;
    let desc = doc.add_obj(&mut fd)!;
    // the CIDFont with the widths of the glyphs used
    let mut cid = strs.String.init();
    defer cid.deinit();
    cid.push_str("<< /Type /Font /Subtype ")!;
    if f.is_cff {
        cid.push_str("/CIDFontType0")!;
    } else {
        cid.push_str("/CIDFontType2 /CIDToGIDMap /Identity")!;
    }
    cid.push_str(" /BaseFont ")!;
    put_name(&mut cid, name.bytes())!;
    cid.push_str(" /CIDSystemInfo << /Registry (Adobe) /Ordering (Identity) /Supplement 0 >> /FontDescriptor ")!;
    put_ref(&mut cid, desc)!;
    cid.push_str(" /W [")!;
    let mut g: usize = 0;
    while g < used.len {
        if used[g] == 0 {
            g += 1;
            continue;
        }
        cid.push(' ')!;
        put_u64(&mut cid, @as(u64, g))!;
        cid.push_str(" [")!;
        while (g < used.len) and (used[g] != 0) {
            cid.push(' ')!;
            put_num(&mut cid, @float_from_int(f64, f.advance(@truncate(u32, g))) * k)!;
            g += 1;
        }
        cid.push_str(" ]")!;
    }
    cid.push_str(" ] >>")!;
    let cidobj = doc.add_obj(&mut cid)!;
    // ToUnicode
    let mut cm = strs.String.init();
    defer cm.deinit();
    cm.push_str("/CIDInit /ProcSet findresource begin\n12 dict begin\nbegincmap\n/CIDSystemInfo << /Registry (Adobe) /Ordering (UCS) /Supplement 0 >> def\n/CMapName /Adobe-Identity-UCS def\n/CMapType 2 def\n1 begincodespacerange\n<0000> <FFFF>\nendcodespacerange\n")!;
    let uni = fu.unicode.items();
    let mut pending: usize = 0;
    let mut block = strs.String.init();
    defer block.deinit();
    for gi in 0..uni.len {
        if (uni[gi] == 0) or (used[gi] == 0) or (gi > 0xffff) {
            continue;
        }
        block.push('<')!;
        put_hex4(&mut block, @truncate(u32, gi))!;
        block.push_str("> <")!;
        put_utf16(&mut block, uni[gi])!;
        block.push_str(">\n")!;
        pending += 1;
        if pending == 100 {
            put_u64(&mut cm, 100)!;
            cm.push_str(" beginbfchar\n")!;
            cm.push_str(block.bytes())!;
            cm.push_str("endbfchar\n")!;
            block.clear();
            pending = 0;
        }
    }
    if pending > 0 {
        put_u64(&mut cm, @as(u64, pending))!;
        cm.push_str(" beginbfchar\n")!;
        cm.push_str(block.bytes())!;
        cm.push_str("endbfchar\n")!;
    }
    cm.push_str("endcmap\nCMapName currentdict /CMap defineresource pop\nend\nend\n")!;
    let tu = doc.add_stream("", cm.bytes(), true)!;
    // the Type0 font
    let mut t0 = strs.String.init();
    defer t0.deinit();
    t0.push_str("<< /Type /Font /Subtype /Type0 /BaseFont ")!;
    put_name(&mut t0, name.bytes())!;
    t0.push_str(" /Encoding /Identity-H /DescendantFonts [")!;
    put_ref(&mut t0, cidobj)!;
    t0.push_str("] /ToUnicode ")!;
    put_ref(&mut t0, tu)!;
    t0.push_str(" >>")!;
    doc.set(fu.obj, &mut t0)!;
}

// -----------------------------------------------------------------------------
// Tests
// -----------------------------------------------------------------------------

fn num_str(v: f64) ![]mut u8 {
    let mut s = strs.String.init();
    put_num(&mut s, v)!;
    return s.take();
}

fn expect_num(v: f64, want: []u8) !void {
    let s = num_str(v)!;
    defer mem.free(s);
    @expect(strs.eq(s, want));
}

test "numbers and strings" {
    expect_num(0.0, "0")!;
    expect_num(12.0, "12")!;
    expect_num(-1.5, "-1.5")!;
    expect_num(1234.56789, "1234.5679")!;
    expect_num(0.00004, "0")!;
    expect_num(-0.00004, "0")!;
    expect_num(0.99999, "1")!;
    expect_num(1.0e300, "0")!;
    let mut s = strs.String.init();
    defer s.deinit();
    put_literal(&mut s, "a(b)\\c")!;
    put_name(&mut s, "A B#")!;
    put_text_string(&mut s, "\xc3\xa9\xf0\x9f\x98\x80")!;
    @expect(strs.eq(s.bytes(), "(a\\(b\\)\\\\c)/A#20B#23<FEFF00E9D83DDE00>"));
}

test "documents read back" {
    let mut doc = Document.init();
    defer doc.deinit();
    doc.set_info("Title", "Caf\xc3\xa9 report")!;
    let mut p = doc.page(A4_WIDTH, A4_HEIGHT)!;
    defer p.deinit();
    let mut path = draw.Path.init();
    defer path.deinit();
    path.circle(100.0, 100.0, 50.0)!;
    p.fill(&path, draw.Paint.solid(color.Color.rgb(1.0, 0.0, 0.0)), draw.NONZERO)!;
    p.stroke(&path, draw.Paint.solid(color.Color.rgb(0.0, 0.0, 0.0)), draw.Stroke.width(2.0))!;
    let stops: [2]draw.Stop = [
        draw.Stop.{ offset: 0.0, color: color.Color.rgb(0.0, 0.0, 1.0) },
        draw.Stop.{ offset: 1.0, color: color.Color.rgb(1.0, 1.0, 1.0) },
    ];
    p.fill_rect(200.0, 50.0, 100.0, 100.0, draw.Paint.linear(200.0, 0.0, 300.0, 0.0, stops[..]))!;
    let mut img = image.Image.init(4, 3, 4, image.U8)!;
    defer img.deinit();
    p.image(&img, 10.0, 300.0, 40.0, 30.0)!;
    p.link(10.0, 10.0, 50.0, 20.0, "https://example.com/(x)")!;
    doc.add_page(&mut p)!;
    let mut p2 = doc.page(LETTER_WIDTH, LETTER_HEIGHT)!;
    defer p2.deinit();
    doc.add_page(&mut p2)!;
    doc.bookmark(0, "One", 0, 0.0)!;
    doc.bookmark(1, "One point one", 0, 100.0)!;
    doc.bookmark(0, "Two", 1, 0.0)!;
    let bytes = doc.bytes()!;
    defer mem.free(bytes);
    @expect(strs.starts_with(bytes, "%PDF-1.7"));
    let mut r = read.Reader.open(bytes)!;
    defer r.deinit();
    @expect_eq(r.page_count(), 2);
    let s1 = r.page_size(0)!;
    @expect(@abs(s1[0] - A4_WIDTH) < 0.001);
    let s2 = r.page_size(1)!;
    @expect_eq(s2[1], LETTER_HEIGHT);
    let t = r.info("Title")!;
    defer mem.free(t);
    @expect(strs.eq(t, "Caf\xc3\xa9 report"));
    // a finished document takes no more pages
    let mut p3 = doc.page(10.0, 10.0)!;
    defer p3.deinit();
    @expect(failed(doc.add_page(&mut p3)));
}

fn failed(r: !void) bool {
    r catch return true;
    return false;
}

test "text in embedded fonts reads back" {
    let mut f = font.Font.load("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf") catch return;
    defer f.deinit();
    let mut doc = Document.init();
    defer doc.deinit();
    let mut p = doc.page(400.0, 200.0)!;
    defer p.deinit();
    let black = color.Color.rgb(0.0, 0.0, 0.0);
    p.text(&f, 12.0, 20.0, 40.0, "Hello, AVA world \xe2\x9c\x93", black)!;
    let st = text.Style.init(&f, 10.0, black);
    let sp: [1]text.Span = [text.Span.{ text: "one two three four five six seven", style: st }];
    let mut l = text.layout(sp[..], text.Options.init(80.0))!;
    defer l.deinit();
    p.draw_layout(&l, 20.0, 80.0)!;
    doc.add_page(&mut p)!;
    let bytes = doc.bytes()!;
    defer mem.free(bytes);
    let mut r = read.Reader.open(bytes)!;
    defer r.deinit();
    let t = r.page_text(0)!;
    defer mem.free(t);
    @expect(strs.eq(t, "Hello, AVA world \xe2\x9c\x93\none two three\nfour five six\nseven"));
    // the font is embedded, reduced to the glyphs used
    @expect(bytes.len < 40000);
}

test "font subsets are fonts" {
    let mut f = font.Font.load("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf") catch return;
    defer f.deinit();
    let mut used = mem.alloc_zeroed.[u8](@as(usize, f.num_glyphs))!;
    defer mem.free(used);
    let a = f.glyph_index('A');
    let e = f.glyph_index(0xe9);  // é: a composite glyph
    used[@as(usize, a)] = 1;
    used[@as(usize, e)] = 1;
    close_composites(&f, used);
    let mut file = strs.String.init();
    defer file.deinit();
    build_font_file(&f, used, &mut file)!;
    @expect(file.len < 60000);
    let sub = font.Font.init(file.bytes())!;
    @expect_eq(sub.num_glyphs, f.num_glyphs);
    let mut path = draw.Path.init();
    defer path.deinit();
    sub.glyph_path(e, &mut path, 0.0, 0.0, 1.0)!;
    @expect(path.verbs.len > 10);  // é and its components are there
    path.clear();
    sub.glyph_path(f.glyph_index('B'), &mut path, 0.0, 0.0, 1.0)!;
    @expect_eq(path.verbs.len, 0);  // B is not
    @expect_eq(checksum(file.bytes()), 0xb1b0afba);
}
