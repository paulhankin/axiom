// =============================================================================
// std/encoding/csv.ph - comma-separated values (RFC 4180, and the common variants).
//
//     let mut r = csv.Reader.init(text, csv.Options.default());
//     defer r.deinit();
//     loop {
//         let rec = r.next() catch |e| { if e == error.Done { break; } return e; };
//         for field in rec { ... }         // valid until the next call
//     }
//
//     let mut w = csv.Writer.init(out, csv.Options.default());
//     w.write(["name", "note"][..])!;      // quotes fields only when needed
//
// Records end at LF or CRLF; a quoted field may hold the delimiter, quotes
// (doubled) and line breaks. Set `delimiter` for TSV and friends, `comment`
// to skip lines starting with a character, `lazy_quotes` to accept stray
// quotes, and `trim` to drop spaces around unquoted fields.
// =============================================================================

import "../collections/vec.ph" as vec;
import "../io.ph" as io;
import "../mem.ph" as mem;
import "../str.ph" as strs;

error { Done, InvalidCsv, OutOfMemory }

pub struct Options {
    delimiter: u8,
    quote: u8,
    comment: u8,        // 0: none
    trim: bool,         // spaces around unquoted fields
    lazy_quotes: bool,  // quotes in unquoted fields, and after closing quotes, are text
    max_fields: usize,  // 0: no limit

    pub fn default() Options {
        return Options.{ delimiter: ',', quote: '"', comment: 0, trim: false, lazy_quotes: false, max_fields: 0 };
    }

    pub fn tsv() Options {
        let mut o = Options.default();
        o.delimiter = '\t';
        return o;
    }
}

pub struct Reader {
    s: []u8,
    i: usize,
    opts: Options,
    line: u32,  // of the record being read (from 1)
    nl: u32,    // line breaks before `counted`
    counted: usize,
    buf: strs.String,  // the current record's field bytes
    ends: vec.Vec.[usize],
    fields: vec.Vec.[[]u8],

    pub fn init(text: []u8, opts: Options) Reader {
        let mut t = text;
        if (t.len >= 3) and (t[0] == 0xef) and (t[1] == 0xbb) and (t[2] == 0xbf) {
            t = t[3..];  // a byte-order mark
        }
        return Reader.{ s: t, i: 0, opts: opts, line: 0, nl: 0, counted: 0, buf: strs.String.init(),
                        ends: vec.Vec.[usize].init(), fields: vec.Vec.[[]u8].init() };
    }

    pub fn deinit(self: *mut Reader) void {
        self.buf.deinit();
        self.ends.deinit();
        self.fields.deinit();
    }

    /// The line the last record started on (from 1), for messages.
    pub fn line_number(self: *Reader) u32 {
        return self.line;
    }

    fn at_eol(self: *Reader) bool {
        let c = self.s[self.i];
        return (c == '\n') or ((c == '\r') and ((self.i + 1) < self.s.len) and (self.s[self.i + 1] == '\n'));
    }

    fn skip_eol(self: *mut Reader) void {
        if self.s[self.i] == '\r' {
            self.i += 1;
        }
        self.i += 1;
    }

    /// The next record's fields (valid until the next call), or Done.
    pub fn next(self: *mut Reader) ![][]u8 {
        // skip comment lines
        loop {
            if self.i >= self.s.len {
                return error.Done;
            }
            if (self.opts.comment != 0) and (self.s[self.i] == self.opts.comment) {
                while (self.i < self.s.len) and (self.s[self.i] != '\n') {
                    self.i += 1;
                }
                self.i += 1;
                continue;
            }
            break;
        }
        // (counted from where the last count stopped, to stay linear)
        self.nl +%= @truncate(u32, count_lines(self.s[self.counted..self.i]));
        self.counted = self.i;
        self.line = self.nl + 1;
        self.buf.clear();
        self.ends.clear();
        loop {
            self.field()!;
            if (self.opts.max_fields > 0) and (self.ends.len > self.opts.max_fields) {
                return error.InvalidCsv;
            }
            if self.i >= self.s.len {
                break;
            }
            if self.at_eol() {
                self.skip_eol();
                break;
            }
            // a delimiter: another field follows
            self.i += 1;
            if self.i >= self.s.len {
                self.ends.push(self.buf.len)!;  // a trailing delimiter: one more empty field
                break;
            }
        }
        self.fields.clear();
        let b = self.buf.bytes();
        let mut st: usize = 0;
        for e in self.ends.items() {
            self.fields.push(b[st..e])!;
            st = e;
        }
        return self.fields.items();
    }

    // One field, up to (not including) the delimiter or line end.
    fn field(self: *mut Reader) !void {
        let o = &self.opts;
        let mut i = self.i;
        if o.trim {
            while (i < self.s.len) and ((self.s[i] == ' ') or (self.s[i] == '\t')) and (self.s[i] != o.delimiter) {
                i += 1;
            }
        }
        if (i < self.s.len) and (self.s[i] == o.quote) {
            // quoted
            i += 1;
            loop {
                if i >= self.s.len {
                    if o.lazy_quotes {
                        break;
                    }
                    self.i = i;
                    return error.InvalidCsv;  // unterminated
                }
                let c = self.s[i];
                if c == o.quote {
                    if ((i + 1) < self.s.len) and (self.s[i + 1] == o.quote) {
                        self.buf.push(o.quote)!;
                        i += 2;
                        continue;
                    }
                    i += 1;
                    // after the closing quote: the end of the field
                    let mut j = i;
                    if o.trim {
                        while (j < self.s.len) and ((self.s[j] == ' ') or (self.s[j] == '\t')) and
                              (self.s[j] != o.delimiter) {
                            j += 1;
                        }
                    }
                    self.i = j;
                    if (j >= self.s.len) or (self.s[j] == o.delimiter) or self.at_eol() {
                        break;
                    }
                    if not o.lazy_quotes {
                        return error.InvalidCsv;
                    }
                    self.buf.push(o.quote)!;
                    continue;
                }
                if (c == '\r') and ((i + 1) < self.s.len) and (self.s[i + 1] == '\n') {
                    self.buf.push('\n')!;  // line breaks in fields become LF
                    i += 2;
                    continue;
                }
                self.buf.push(c)!;
                i += 1;
            }
            if i >= self.s.len {
                self.i = i;
            }
            self.ends.push(self.buf.len)!;
            return;
        }
        // unquoted
        let st = i;
        while i < self.s.len {
            let c = self.s[i];
            if c == o.delimiter {
                break;
            }
            if (c == '\n') or ((c == '\r') and ((i + 1) < self.s.len) and (self.s[i + 1] == '\n')) {
                break;
            }
            if (c == o.quote) and not o.lazy_quotes {
                self.i = i;
                return error.InvalidCsv;  // a quote inside an unquoted field
            }
            i += 1;
        }
        let mut e = i;
        if o.trim {
            while (e > st) and ((self.s[e - 1] == ' ') or (self.s[e - 1] == '\t')) {
                e -= 1;
            }
        }
        self.buf.push_str(self.s[st..e])!;
        self.ends.push(self.buf.len)!;
        self.i = i;
    }
}

fn count_lines(s: []u8) usize {
    let mut n: usize = 0;
    for c in s {
        if c == '\n' {
            n += 1;
        }
    }
    return n;
}

pub struct Writer {
    out: io.Writer,
    opts: Options,
    crlf: bool,  // end records with CRLF (RFC 4180) instead of LF

    pub fn init(out: io.Writer, opts: Options) Writer {
        return Writer.{ out: out, opts: opts, crlf: false };
    }

    /// Writes one record.
    pub fn write(self: *mut Writer, fields: [][]u8) !void {
        for k in @as(usize, 0)..fields.len {
            if k > 0 {
                let d: [1]u8 = [self.opts.delimiter];
                self.out.write_all(d[..])!;
            }
            self.field(fields[k])!;
        }
        if self.crlf {
            self.out.write_all("\r\n")!;
        } else {
            self.out.write_all("\n")!;
        }
    }

    fn field(self: *mut Writer, f: []u8) !void {
        let q = self.opts.quote;
        let mut needs = (f.len > 0) and ((f[0] == ' ') or (f[f.len - 1] == ' ') or (f[0] == self.opts.comment));
        for c in f {
            if (c == self.opts.delimiter) or (c == q) or (c == '\n') or (c == '\r') {
                needs = true;
            }
        }
        if not needs {
            self.out.write_all(f)!;
            return;
        }
        let qq: [2]u8 = [q, q];
        self.out.write_all(qq[..1])!;
        let mut st: usize = 0;
        for k in @as(usize, 0)..f.len {
            if f[k] == q {
                self.out.write_all(f[st..k])!;
                self.out.write_all(qq[..])!;
                st = k + 1;
            }
        }
        self.out.write_all(f[st..])!;
        self.out.write_all(qq[..1])!;
    }
}

// -----------------------------------------------------------------------------
// Tests
// -----------------------------------------------------------------------------

fn all_records(text: []u8, o: Options, out: *mut strs.String) !void {
    let mut r = Reader.init(text, o);
    defer r.deinit();
    loop {
        let rec = r.next() catch |e| {
            if e == error.Done {
                return;
            }
            return e;
        };
        for k in @as(usize, 0)..rec.len {
            if k > 0 {
                out.push('|')!;
            }
            out.push_str(rec[k])!;
        }
        out.push(';')!;
    }
}

fn read_as(text: []u8, o: Options) ![]mut u8 {
    let mut s = strs.String.init();
    all_records(text, o, &mut s) catch |e| {
        s.deinit();
        return e;
    };
    return s.take();
}

test "reading" {
    let d = Options.default();
    let cases: [9][2][]u8 = [
        ["a,b,c\n1,2,3\n", "a|b|c;1|2|3;"],
        ["a,b\r\n\"x,y\",\"say \"\"hi\"\"\"\r\n", "a|b;x,y|say \"hi\";"],
        ["\"multi\nline\",z", "multi\nline|z;"],
        ["a,,c,\n", "a||c|;"],
        ["\xef\xbb\xbfbom,x", "bom|x;"],
        ["one\n\ntwo", "one;;two;"],
        ["\"\"", ";"],
        ["x,\"\"\n", "x|;"],
        ["a,b,", "a|b|;"],
    ];
    for c in cases {
        let got = read_as(c[0], d)!;
        defer mem.free(got);
        @expect_eq(got, c[1]);
    }
    let bad: [3][]u8 = ["a\"b", "\"unterminated", "\"x\"y"];
    for b in bad {
        @expect(fails(read_as(b, d)));
    }
    let mut lazy = d;
    lazy.lazy_quotes = true;
    let l = read_as("a\"b,\"x\"y\n", lazy)!;
    defer mem.free(l);
    @expect_eq(l, "a\"b|x\"y\n;");  // (the stray quote leaves the field quoted: the line break is in it)
    let mut t = Options.tsv();
    t.trim = true;
    t.comment = '#';
    let tv = read_as("# header\n a \t \"b\" \nc\td", t)!;
    defer mem.free(tv);
    @expect_eq(tv, "a|b;c|d;");
    let mut r = Reader.init("a\nb\n\"c\nd\"\ne", d);
    defer r.deinit();
    for want in [@as(u32, 1), 2, 3, 5]{
        _ = r.next()!;
        @expect_eq(r.line_number(), want);
    }
    let mut lim = d;
    lim.max_fields = 2;
    @expect(fails(read_as("1,2,3", lim)));
}

fn fails(r: ![]mut u8) bool {
    _ = r catch return true;
    return false;
}

test "writing round trips" {
    let mut s = strs.String.init();
    defer s.deinit();
    let mut w = Writer.init(s.writer(), Options.default());
    let r1: [4][]u8 = ["plain", "with,comma", "say \"hi\"", " padded"];
    let r2: [2][]u8 = ["line\nbreak", ""];
    w.write(r1[..])!;
    w.write(r2[..])!;
    @expect_eq(s.bytes(), "plain,\"with,comma\",\"say \"\"hi\"\"\",\" padded\"\n\"line\nbreak\",\n");
    let back = read_as(s.bytes(), Options.default())!;
    defer mem.free(back);
    @expect_eq(back, "plain|with,comma|say \"hi\"| padded;line\nbreak|;");
}
