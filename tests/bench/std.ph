// Throughput of the standard library's heavy operations. Writes its inputs
// to build/bench/ so tools/bench_std.py can time the same work in
// reference implementations (zlib, hashlib, json, re, Pillow, cmark-gfm,
// OpenSSL, ...). Build with -O2:
//
//     phalange build --target=native -O2 tests/bench/std.ph -o build/bench/std && build/bench/std [filter]
import "std/base64" as base64;
import "std/bigint" as bigint;
import "std/color" as color;
import "std/crypto/aes_gcm" as aes_gcm;
import "std/crypto/chacha20poly1305" as chacha;
import "std/crypto/md5" as md5;
import "std/crypto/sha2" as sha2;
import "std/deflate" as deflate;
import "std/draw" as draw;
import "std/fmt" as fmt;
import "std/font" as font;
import "std/fs" as fs;
import "std/hash" as hash;
import "std/image" as image;
import "std/image/jpeg" as jpeg;
import "std/image/png" as png;
import "std/json" as json;
import "std/map" as maps;
import "std/markdown" as markdown;
import "std/mem" as mem;
import "std/os" as os;
import "std/pdf" as pdf;
import "std/pdf/read" as pdf_read;
import "std/regex" as regex;
import "std/str" as strs;
import "std/text" as text;
import "std/time" as time;
import "std/unicode" as uni;
import "std/vec" as vec;

let mut filter: []u8 = "";
let mut rng: u64 = 0x2545f4914f6cdd1d;

fn rand() u64 {
    rng ^= rng << 13;
    rng ^= rng >> 7;
    rng ^= rng << 17;
    return rng;
}

fn wanted(name: []u8) bool {
    return (filter.len == 0) or strs.contains(name, filter);
}

/// Prints a result: MB/s of `bytes` processed in the time since `t0`.
fn report(name: []u8, bytes: usize, t0: time.Instant) void {
    let s = t0.elapsed().as_seconds();
    let mbs = (@float_from_int(f64, bytes) / 1.0e6) / @max(s, 1.0e-9);
    fmt.print("{:<28} {:>10.1} MB/s  ({:.3} s)\n", [fmt.str(name), fmt.float(mbs), fmt.float(s)]) catch {};
}

fn report_ops(name: []u8, ops: usize, t0: time.Instant) void {
    let s = t0.elapsed().as_seconds();
    let r = @float_from_int(f64, ops) / @max(s, 1.0e-9);
    fmt.print("{:<28} {:>10.3} Mop/s  ({:.3} s)\n", [fmt.str(name), fmt.float(r / 1.0e6), fmt.float(s)]) catch {};
}

let WORDS: [24][]u8 = [
    "the",
    "quick",
    "brown",
    "fox",
    "jumps",
    "over",
    "lazy",
    "dog",
    "lorem",
    "ipsum",
    "dolor",
    "sit",
    "amet",
    "consectetur",
    "adipiscing",
    "elit",
    "sed",
    "do",
    "eiusmod",
    "tempor",
    "incididunt",
    "labore",
    "magna",
    "aliqua",
];

/// English-like text with some structure (and a few emails and numbers).
fn corpus(size: usize) ![]mut u8 {
    let mut s = strs.String.init();
    while s.len < size {
        let r = rand();
        if (r % 50) == 0 {
            s.push_str("someone")!;
            s.push_str(WORDS[@truncate(usize, (r >> 8) % 24)])!;
            s.push_str("@example.com")!;
        } else if (r % 23) == 0 {
            fmt.write(s.writer(), "{}", [fmt.int((r >> 16) % 100000)])!;
        } else {
            s.push_str(WORDS[@truncate(usize, (r >> 8) % 24)])!;
        }
        if (r % 13) == 0 {
            s.push_str(".\n")!;
        } else {
            s.push(' ')!;
        }
    }
    return s.take();
}

fn save(name: []u8, data: []u8) !void {
    let mut p = strs.String.init();
    defer p.deinit();
    p.push_str("build/bench/")!;
    p.push_str(name)!;
    fs.write_file(p.bytes(), data)!;
}

export fn main() i32 {
    filter = os.arg(1) catch no_filter();
    run() catch |e| {
        fmt.print("error: {}\n", [fmt.str(@error_name(e))]) catch {};
        return 1;
    };
    return 0;
}

fn run() !void {
    fs.mkdir_all("build/bench")!;
    let text4 = corpus(4 << 20)!;
    defer mem.free(text4);
    save("text.txt", text4)!;

    if wanted("deflate") {
        let t0 = time.Instant.now();
        let z = deflate.compress(text4, deflate.ZLIB, 6)!;
        report("deflate compress -6", text4.len, t0);
        save("text.z", z)!;
        let t1 = time.Instant.now();
        for _k in @as(usize, 0)..4 {
            let d = deflate.decompress(z, deflate.ZLIB, 1 << 30)!;
            mem.free(d);
        }
        report("deflate decompress", text4.len * 4, t1);
        mem.free(z);
    }
    if wanted("hash") {
        let mut out: [64]u8 = [0; 64];
        let t0 = time.Instant.now();
        for _k in @as(usize, 0)..4 {
            sha2.sha256(text4, out[..32]);
        }
        report("sha256", text4.len * 4, t0);
        let t1 = time.Instant.now();
        for _k in @as(usize, 0)..4 {
            sha2.sha512(text4, out[..]);
        }
        report("sha512", text4.len * 4, t1);
        let t2 = time.Instant.now();
        for _k in @as(usize, 0)..4 {
            md5.md5(text4, out[..16]);
        }
        report("md5", text4.len * 4, t2);
        let t3 = time.Instant.now();
        let mut h: u64 = 0;
        for k in @as(usize, 0)..16 {
            h ^= hash.hash_bytes(text4[k..]);
        }
        report("hash_bytes", text4.len * 16, t3);
        if h == 42 {
            fmt.print("{}", [fmt.int(h)]) catch {};
        }
    }
    if wanted("aead") {
        let key: [32]u8 = [7; 32];
        let nonce: [12]u8 = [1; 12];
        let mut out = mem.alloc.[u8](text4.len + 16)!;
        defer mem.free(out);
        let g = aes_gcm.AesGcm.init(key[..16])!;
        let t0 = time.Instant.now();
        for _k in @as(usize, 0)..4 {
            g.seal(nonce[..], "", text4, out)!;
        }
        report("aes-128-gcm seal", text4.len * 4, t0);
        let t1 = time.Instant.now();
        for _k in @as(usize, 0)..4 {
            chacha.seal(key[..], nonce[..], "", text4, out)!;
        }
        report("chacha20-poly1305 seal", text4.len * 4, t1);
    }
    if wanted("base64") {
        let enc = base64.encode_alloc(text4, base64.STANDARD)!;
        defer mem.free(enc);
        let t0 = time.Instant.now();
        for _k in @as(usize, 0)..8 {
            let e = base64.encode_alloc(text4, base64.STANDARD)!;
            mem.free(e);
        }
        report("base64 encode", text4.len * 8, t0);
        let t1 = time.Instant.now();
        for _k in @as(usize, 0)..8 {
            let d = base64.decode_alloc(enc, base64.STANDARD)!;
            mem.free(d);
        }
        report("base64 decode", enc.len * 8, t1);
    }
    if wanted("json") {
        let mut s = strs.String.init();
        defer s.deinit();
        s.push('[')!;
        let mut i: usize = 0;
        while s.len < (4 << 20) {
            if i > 0 {
                s.push(',')!;
            }
            fmt.write(s.writer(),
                      "{{\"id\":{},\"name\":\"{}\",\"score\":{},\"tags\":[\"a\",\"b\\n\"],\"ok\":true,\"x\":null}}",
                      [fmt.int(i), fmt.str(WORDS[i % 24]), fmt.float(@float_from_int(f64, i) * 1.25)])!;
            i += 1;
        }
        s.push(']')!;
        save("data.json", s.bytes())!;
        let t0 = time.Instant.now();
        for _k in @as(usize, 0)..4 {
            let mut d = json.parse(s.bytes())!;
            d.deinit();
        }
        report("json parse", s.len * 4, t0);
        let mut d = json.parse(s.bytes())!;
        defer d.deinit();
        let t1 = time.Instant.now();
        for _k in @as(usize, 0)..4 {
            let o = json.stringify(d.root)!;
            mem.free(o);
        }
        report("json stringify", s.len * 4, t1);
    }
    if wanted("regex") {
        let mut re = regex.compile("[a-z]+@[a-z]+\\.com")!;
        defer re.deinit();
        let t0 = time.Instant.now();
        let mut n: usize = 0;
        let mut it = re.find_all(text4);
        while true {
            _ = it.next() catch break;
            n += 1;
        }
        report("regex find_all (emails)", text4.len, t0);
        let mut re2 = regex.compile("(\\w+) (\\w+)ing")!;
        defer re2.deinit();
        let t1 = time.Instant.now();
        let mut it2 = re2.find_all(text4);
        while true {
            _ = it2.next() catch break;
            n += 1;
        }
        report("regex find_all (words)", text4.len, t1);
        let t2 = time.Instant.now();
        for _k in @as(usize, 0)..16 {
            _ = strs.index_of(text4, "not in the text") catch 0;
        }
        report("str.index_of (miss)", text4.len * 16, t2);
        _ = n;
    }
    if wanted("markdown") {
        let mut s = strs.String.init();
        defer s.deinit();
        while s.len < (2 << 20) {
            s.push_str("# Heading *one*\n\nSome **bold** and _emphasis_, `code`, a [link](http://x.com \"t\") and www.example.com.\n\n- item one\n- item [two][r]\n  1. nested\n\n> quote\n> more\n\n```\ncode block\n```\n\n| a | b |\n|---|---|\n| 1 | 2 |\n\n[r]: /url\n\n")!;
        }
        save("doc.md", s.bytes())!;
        let t0 = time.Instant.now();
        for _k in @as(usize, 0)..4 {
            let h = markdown.to_html(s.bytes(), markdown.Options.init())!;
            mem.free(h);
        }
        report("markdown to_html (gfm)", s.len * 4, t0);
    }
    if wanted("image") {
        let w: usize = 1024;
        let h: usize = 768;
        let mut img = image.Image.init(w, h, 4, image.U8)!;
        defer img.deinit();
        for y in @as(usize, 0)..h {
            for x in @as(usize, 0)..w {
                let o = ((y * w) + x) * 4;
                img.data[o] = @truncate(u8, x ^ y);
                img.data[o + 1] = @truncate(u8, (x * 3) + y);
                img.data[o + 2] = @truncate(u8, rand() & 31);
                img.data[o + 3] = 255;
            }
        }
        let raw = w * h * 4;
        let t0 = time.Instant.now();
        let p = png.encode(&img, png.DEFAULT)!;
        report("png encode", raw, t0);
        save("image.png", p)!;
        let t1 = time.Instant.now();
        for _k in @as(usize, 0)..4 {
            let mut d = png.decode(p)!;
            d.deinit();
        }
        report("png decode", raw * 4, t1);
        mem.free(p);
        let t2 = time.Instant.now();
        let j = jpeg.encode(&img, 90, jpeg.SUBSAMPLE_420)!;
        report("jpeg encode q90", raw, t2);
        save("image.jpg", j)!;
        let t3 = time.Instant.now();
        for _k in @as(usize, 0)..4 {
            let mut d = jpeg.decode(j)!;
            d.deinit();
        }
        report("jpeg decode", raw * 4, t3);
        mem.free(j);
        let mut small = img.resize(w / 3, h / 3)!;
        let t4 = time.Instant.now();
        let mut big = small.resize(w, h)!;
        report("image resize (upscale)", raw, t4);
        small.deinit();
        big.deinit();
    }
    if wanted("draw") {
        let mut img = image.Image.init(800, 600, 4, image.U8)!;
        defer img.deinit();
        let mut cv = draw.Canvas.init(&mut img);
        defer cv.deinit();
        let mut path = draw.Path.init();
        defer path.deinit();
        let t0 = time.Instant.now();
        let n: usize = 2000;
        for i in @as(usize, 0)..n {
            path.clear();
            let x = @float_from_int(f64, rand() % 800);
            let y = @float_from_int(f64, rand() % 600);
            path.circle(x, y, 5.0 + @float_from_int(f64, i % 40))!;
            cv.fill(&path, draw.Paint.solid(color.Color.rgb(0.2, 0.4, 0.8)), draw.NONZERO)!;
        }
        report_ops("draw fill circles", n, t0);
        let t1 = time.Instant.now();
        for _i in @as(usize, 0)..n {
            path.clear();
            path.move_to(@float_from_int(f64, rand() % 800), @float_from_int(f64, rand() % 600))!;
            path.cubic_to(100.0, 100.0, 700.0, 500.0, @float_from_int(f64, rand() % 800), 300.0)!;
            cv.stroke(&path, draw.Paint.solid(color.Color.rgb(0.0, 0.0, 0.0)), draw.Stroke.width(3.0))!;
        }
        report_ops("draw stroke curves", n, t1);
    }
    if wanted("text") {
        let mut f = font.Font.load("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf") catch return;
        defer f.deinit();
        let st = text.Style.init(&f, 12.0, color.Color.rgb(0.0, 0.0, 0.0));
        let body = text4[..200000];
        let sp: [1]text.Span = [text.Span.{ text: body, style: st }];
        let t0 = time.Instant.now();
        let mut l = text.layout(sp[..], text.Options.init(400.0))!;
        report("text layout (wrap 400pt)", body.len, t0);
        l.deinit();
        let t1 = time.Instant.now();
        let nfc = uni.nfc(text4)!;
        report("unicode nfc (ascii)", text4.len, t1);
        mem.free(nfc);
        let t2 = time.Instant.now();
        let mut g = uni.graphemes(text4);
        let mut n: usize = 0;
        while true {
            _ = g.next() catch break;
            n += 1;
        }
        report("unicode graphemes", text4.len, t2);
        let t3 = time.Instant.now();
        let up = uni.upper(text4)!;
        report("unicode upper", text4.len, t3);
        mem.free(up);
        // PDF: write the text as pages, then read it back
        let t4 = time.Instant.now();
        let mut doc = pdf.Document.init();
        defer doc.deinit();
        let mut ll = text.layout(sp[..], text.Options.init(450.0))!;
        defer ll.deinit();
        let mut page = doc.page(pdf.A4_WIDTH, pdf.A4_HEIGHT)!;
        defer page.deinit();
        page.draw_layout(&ll, 50.0, 50.0)!;
        doc.add_page(&mut page)!;
        let bytes = doc.bytes()!;
        defer mem.free(bytes);
        report("pdf write (layout)", body.len, t4);
        save("text.pdf", bytes)!;
        let t5 = time.Instant.now();
        let mut r = pdf_read.Reader.open(bytes)!;
        defer r.deinit();
        let tx = r.page_text(0)!;
        report("pdf read text", body.len, t5);
        mem.free(tx);
    }
    if wanted("fmt") {
        let n: usize = 1000000;
        let mut s = strs.String.init();
        defer s.deinit();
        let t0 = time.Instant.now();
        for i in @as(usize, 0)..n {
            fmt.write(s.writer(), "{} ", [fmt.float(@float_from_int(f64, i) * 1.37e-3)])!;
        }
        report_ops("fmt float (shortest)", n, t0);
        save("floats.txt", s.bytes())!;
        let t1 = time.Instant.now();
        let mut f = strs.fields(s.bytes());
        let mut sum: f64 = 0.0;
        while true {
            let w = f.next() catch break;
            sum += fmt.parse_float(w)!;
        }
        report_ops("fmt parse_float", n, t1);
        _ = sum;
    }
    if wanted("containers") {
        let n: usize = 1000000;
        let mut m = maps.Map.[u64, u64].init(hash.hash_u64, hash.eq_u64);
        defer m.deinit();
        let t0 = time.Instant.now();
        for i in @as(usize, 0)..n {
            m.put(@as(u64, i) *% 0x9e3779b97f4a7c15, @as(u64, i))!;
        }
        report_ops("map insert u64", n, t0);
        let t1 = time.Instant.now();
        let mut hits: usize = 0;
        for i in @as(usize, 0)..n {
            if m.contains(@as(u64, i) *% 0x9e3779b97f4a7c15) {
                hits += 1;
            }
        }
        report_ops("map lookup u64", n, t1);
        let mut v = vec.Vec.[u64].init();
        defer v.deinit();
        for _i in @as(usize, 0)..n {
            v.push(rand())!;
        }
        let t2 = time.Instant.now();
        v.sort(less_u64);
        report_ops("sort u64", n, t2);
        let t3 = time.Instant.now();
        let mut total: usize = 0;
        for _i in @as(usize, 0)..n {
            let b = mem.alloc.[u8](16 + @truncate(usize, rand() % 200))!;
            total += b.len;
            mem.free(b);
        }
        report_ops("alloc/free small", n, t3);
        _ = hits;
        _ = total;
    }
    if wanted("bigint") {
        let mut a = bigint.BigInt.parse("1", 10)!;
        defer a.deinit();
        let mut r = bigint.BigInt.parse("1", 10)!;
        defer r.deinit();
        let mut bytes: [256]u8 = [0; 256];
        for k in @as(usize, 0)..256 {
            bytes[k] = @truncate(u8, rand());
        }
        let mut x = bigint.BigInt.from_bytes(bytes[..])!;
        defer x.deinit();
        let mut m = bigint.BigInt.from_bytes(bytes[..])!;
        defer m.deinit();
        let mut e = bigint.BigInt.from_bytes(bytes[..])!;
        defer e.deinit();
        let t0 = time.Instant.now();
        for _k in @as(usize, 0)..20000 {
            r.mul(&x, &m)!;
        }
        report_ops("bigint mul 2048x2048", 20000, t0);
        m.set_bit(0, 1)!;
        let t1 = time.Instant.now();
        for _k in @as(usize, 0)..10 {
            r.mod_pow(&x, &e, &m)!;
        }
        report_ops("bigint mod_pow 2048", 10, t1);
        _ = a;
    }
}

fn no_filter() []mut u8 {
    return @slice_from_ptr(@ptr_from_int(*mut u8, 1), 0);
}

fn less_u64(a: u64, b: u64) bool {
    return a < b;
}
