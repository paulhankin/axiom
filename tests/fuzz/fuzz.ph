// A driver for fuzzing the standard library's parsers: `fuzz <target> <file>`
// runs one target on the file's bytes. Errors are fine (bad input is
// expected); a trap, a hang or runaway memory is a bug. tools/fuzz.py
// mutates seed files and runs this.
import "std/audio" as audio;
import "std/audio/formats" as audio_formats;
import "std/base64" as base64;
import "std/color" as color;
import "std/crypto/x509" as x509;
import "std/deflate" as deflate;
import "std/draw" as draw;
import "std/fmt" as fmt;
import "std/font" as font;
import "std/fs" as fs;
import "std/http" as http;
import "std/image" as image;
import "std/image/formats" as image_formats;
import "std/io" as io;
import "std/json" as json;
import "std/markdown" as markdown;
import "std/mem" as mem;
import "std/os" as os;
import "std/pdf" as pdf;
import "std/pdf/read" as pdf_read;
import "std/regex" as regex;
import "std/str" as strs;
import "std/time" as time;
import "std/unicode" as uni;
import "std/unicode/bidi" as bidi;
import "std/unicode/linebreak" as linebreak;

export fn main() i32 {
    let target = os.arg(1) catch return 2;
    let path = os.arg(2) catch return 2;
    if strs.eq(target, "seeds") {
        seeds(path) catch return 1;
        return 0;
    }
    let data = fs.read_file(path) catch return 2;
    defer mem.free(data);
    run(target, data) catch return 1;
    return 0;
}

fn run(t: []u8, d: []u8) !void {
    if strs.eq(t, "json") {
        let mut doc = json.parse(d)!;
        defer doc.deinit();
        let s = json.stringify(doc.root)!;
        mem.free(s);
    } else if strs.eq(t, "regex") {
        // pattern NUL subject
        let cut = strs.index_of_byte(d, 0) catch d.len;
        let mut re = regex.compile(d[..cut])!;
        defer re.deinit();
        let subject = d[@min(cut + 1, d.len)..];
        _ = re.is_match(subject)!;
        let mut caps = re.captures(subject) catch return;
        caps.deinit();
        let mut it = re.find_all(subject);
        let mut n: usize = 0;
        while n < 10000 {
            _ = it.next() catch break;
            n += 1;
        }
    } else if strs.eq(t, "image") {
        let mut img = image_formats.decode(d)!;
        img.deinit();
    } else if strs.eq(t, "audio") {
        let mut s = audio_formats.decode(d)!;
        s.deinit();
    } else if strs.eq(t, "deflate") {
        let formats: [3]u8 = [deflate.RAW, deflate.ZLIB, deflate.GZIP];
        for f in formats {
            let out = deflate.decompress(d, f, 1 << 24) catch continue;
            mem.free(out);
        }
        let mut sr = io.SliceReader.init(d);
        let dec = deflate.Decompressor.init(sr.reader(), deflate.GZIP)!;
        let mut buf: [4096]u8 = [0; 4096];
        let mut total: usize = 0;
        while total < (1 << 24) {
            let n = dec.read(buf[..]) catch break;
            if n == 0 {
                break;
            }
            total += n;
        }
        dec.deinit();
    } else if strs.eq(t, "font") {
        let f = font.Font.init(d)!;
        let mut path = draw.Path.init();
        defer path.deinit();
        let n = @min(f.num_glyphs, 300);
        for g in @as(u32, 0)..n {
            path.clear();
            f.glyph_path(g, &mut path, 0.0, 0.0, 0.01) catch continue;
            _ = f.advance(g);
        }
        for cp in @as(u32, 0)..300 {
            let a = f.glyph_index(cp);
            _ = f.kerning(a, f.glyph_index(cp + 1));
        }
        _ = f.glyph_index(0x10ffff);
        _ = f.glyph_index(0x4e2d);
    } else if strs.eq(t, "markdown") {
        let a = markdown.to_html(d, markdown.Options.init())!;
        mem.free(a);
        let b = markdown.to_html(d, markdown.Options.commonmark())!;
        mem.free(b);
    } else if strs.eq(t, "pdf") {
        let mut r = pdf_read.Reader.open(d)!;
        defer r.deinit();
        for i in 0..@min(r.page_count(), 50) {
            let s = r.page_text(i) catch continue;
            mem.free(s);
        }
        let ti = r.info("Title")!;
        mem.free(ti);
    } else if strs.eq(t, "x509") {
        let mut it = x509.pem_blocks(d);
        while true {
            let b = it.next() catch break;
            let der = x509.pem_decode(b) catch continue;
            defer mem.free(der);
            let mut c = x509.Certificate.parse(der) catch continue;
            c.deinit();
        }
        let mut c2 = x509.Certificate.parse(d) catch return;
        c2.deinit();
    } else if strs.eq(t, "text") {
        // everything that takes arbitrary text
        let b = base64.decode_alloc(d, base64.STANDARD) catch mem.alloc.[u8](0)!;
        mem.free(b);
        _ = time.parse_rfc3339(d) catch time.Time.{ sec: 0, nsec: 0 };
        _ = time.parse_http(d) catch time.Time.{ sec: 0, nsec: 0 };
        _ = fmt.parse_int.[i64](d) catch 0;
        _ = fmt.parse_float(d) catch 0.0;
        _ = http.parse_url(d) catch return;
        let n1 = uni.nfkc(d)!;
        mem.free(n1);
        let n2 = uni.nfd(d)!;
        mem.free(n2);
        let u = uni.upper(d)!;
        mem.free(u);
        let cf = uni.case_fold(d)!;
        mem.free(cf);
        let mut g = uni.graphemes(d);
        while true {
            _ = g.next() catch break;
        }
        let mut w = uni.words(d);
        while true {
            _ = w.next() catch break;
        }
        let mut s = uni.sentences(d);
        while true {
            _ = s.next() catch break;
        }
        let mut lb = linebreak.Breaks.init(d)!;
        while true {
            _ = lb.next() catch break;
        }
        lb.deinit();
        let mut v = mem.alloc.[u32](strs.rune_count(d))!;
        defer mem.free(v);
        let mut it = strs.runes(d);
        let mut k: usize = 0;
        while k < v.len {
            v[k] = it.next() catch break;
            k += 1;
        }
        let mut p = bidi.Paragraph.init(v, bidi.AUTO)!;
        let order = p.visual_order(0, v.len)!;
        mem.free(order);
        p.deinit();
    }
}

// -----------------------------------------------------------------------------
// Seeds: small valid inputs for each target, made with std's own encoders
// -----------------------------------------------------------------------------

fn put(dir: []u8, target: []u8, name: []u8, data: []u8) !void {
    let mut p = strs.String.init();
    defer p.deinit();
    p.push_str(dir)!;
    p.push('/')!;
    p.push_str(target)!;
    fs.mkdir_all(p.bytes())!;
    p.push('/')!;
    p.push_str(name)!;
    fs.write_file(p.bytes(), data)!;
}

fn seeds(dir: []u8) !void {
    // images: gray, gray+alpha, RGB, RGBA; 8 and 16 bits; float for HDR/EXR
    let chans: [4]usize = [1, 2, 3, 4];
    let names: [7][]u8 = ["png", "jpeg", "bmp", "qoi", "pnm", "hdr", "exr"];
    for c in chans {
        let mut img = image.Image.init(13, 7, c, image.U8)!;
        defer img.deinit();
        for y in @as(usize, 0)..7 {
            for x in @as(usize, 0)..13 {
                let mut col = color.Color.rgb(@float_from_int(f32, x) / 13.0, @float_from_int(f32, y) / 7.0, 0.5);
                col.a = @float_from_int(f32, x + y) / 20.0;
                img.set(x, y, col);
            }
        }
        for f in @as(u8, 1)..8 {
            let mut src = img.clone()!;
            defer src.deinit();
            if f >= image_formats.HDR {
                let conv = src.convert(c, image.F32)!;
                src.deinit();
                src = conv;
            }
            let b = image_formats.encode(&src, f, image_formats.Options.default()) catch continue;
            defer mem.free(b);
            let mut n = strs.String.init();
            defer n.deinit();
            n.push_str(names[@as(usize, f - 1)])!;
            n.push('0' + @truncate(u8, c))!;
            put(dir, "image", n.bytes(), b)!;
        }
    }
    // sounds
    let rates: [2]u32 = [8000, 44100];
    let fmts: [3]u8 = [audio_formats.WAV, audio_formats.AIFF, audio_formats.FLAC];
    let fnames: [3][]u8 = ["wav", "aiff", "flac"];
    for r in rates {
        let mut snd = audio.Sound.init(300, 2, r, audio.S16)!;
        defer snd.deinit();
        for i in @as(usize, 0)..300 {
            snd.set(i, 0, (@float_from_int(f64, i % 37) / 40.0) - 0.4);
            snd.set(i, 1, (@float_from_int(f64, i % 11) / 12.0) - 0.4);
        }
        for k in @as(usize, 0)..3 {
            let b = audio_formats.encode(&snd, fmts[k]) catch continue;
            defer mem.free(b);
            let mut n = strs.String.init();
            defer n.deinit();
            n.push_str(fnames[k])!;
            n.push('0' + @truncate(u8, r % 10))!;
            put(dir, "audio", n.bytes(), b)!;
        }
    }
    // compressed data
    let text =
        "The quick brown fox jumps over the lazy dog. The quick brown fox jumps over the lazy dog again, and again: 0123456789 abcdefghijklmnopqrstuvwxyz";
    let dfmts: [3]u8 = [deflate.RAW, deflate.ZLIB, deflate.GZIP];
    let dnames: [3][]u8 = ["raw", "zlib", "gzip"];
    for k in @as(usize, 0)..3 {
        let levels: [2]u32 = [1, 9];
        for lv in levels {
            let b = deflate.compress(text, dfmts[k], lv)!;
            defer mem.free(b);
            let mut n = strs.String.init();
            defer n.deinit();
            n.push_str(dnames[k])!;
            n.push('0' + @truncate(u8, lv))!;
            put(dir, "deflate", n.bytes(), b)!;
        }
    }
    put(dir, "json", "a",
        "{\"a\": [1, 2.5e10, -0.0, true, false, null, \"x\\u00e9\\ud83d\\ude00\"], \"b\": {\"c\": {}}}")!;
    put(dir, "json", "b", "[[[[[[1]]]]], \"\\n\\t\\\"\", 123456789012345678901234567890, 1e400]")!;
    put(dir, "markdown", "a",
        "# Title *em* **strong**\n\n- [x] a\n- b <http://x.y>\n\n> quote `code` [link](/u \"t\") ![i](j)\n\n```go\nx\n```\n\n| a | b |\n|---|:-:|\n| 1 | 2 |\n\n[r]: /url\n\n<div>\nhtml\n</div>\n~~del~~ www.x.com a@b.co &amp; &#123; \\* _x_\n")!;
    put(dir, "regex", "a", "(a|b)*c{2,5}[^x-z]\\d+(?P<n>foo)?$\x00aaabccc123foo")!;
    put(dir, "regex", "b", "(?i)^(\\w+)\\s*=\\s*(.*?)(?:#.*)?$\x00Key = value # comment")!;
    put(dir, "text", "a", "2024-02-29T23:59:60.123+05:30")!;
    put(dir, "text", "b", "Sun, 06 Nov 1994 08:49:37 GMT")!;
    put(dir, "text", "c", "https://user:pw@example.com:8080/p/a%20th?q=1&r=%zz#frag")!;
    put(dir, "text", "d",
        "e\xcc\x81\xef\xac\x81 \xd7\xa9\xd7\x9c\xd7\x95\xd7\x9d (abc) \xf0\x9f\x91\xa9\xe2\x80\x8d\xf0\x9f\x91\xa9 SGVsbG8= -12.5e-3")!;
    // a PDF
    let mut doc = pdf.Document.init();
    defer doc.deinit();
    let mut pg = doc.page(200.0, 100.0)!;
    defer pg.deinit();
    let mut path = draw.Path.init();
    defer path.deinit();
    path.rect(10.0, 10.0, 50.0, 20.0)!;
    pg.fill(&path, draw.Paint.solid(color.Color.rgb(1.0, 0.0, 0.0)), draw.NONZERO)!;
    let mut f = font.Font.load("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf") catch {
        doc.add_page(&mut pg)!;
        let b = doc.bytes()!;
        defer mem.free(b);
        put(dir, "pdf", "a", b)!;
        return;
    };
    defer f.deinit();
    pg.text(&f, 10.0, 10.0, 50.0, "Hello PDF", color.Color.rgb(0.0, 0.0, 0.0))!;
    doc.add_page(&mut pg)!;
    let b = doc.bytes()!;
    defer mem.free(b);
    put(dir, "pdf", "a", b)!;
}
