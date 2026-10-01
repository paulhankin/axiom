// =============================================================================
// std/text/wrap.ph - wrapping and filling plain text to a width in terminal
// columns, and indenting and dedenting it.
//
//     let lines = wrap.wrap(text, wrap.Options.init(40))!;   // [][]u8
//     defer wrap.free_lines(lines);
//     let mut o = wrap.Options.init(72);
//     o.initial_indent = "  * ";
//     o.subsequent_indent = "    ";
//     let s = wrap.fill(text, o)!;                     // one string; mem.free(s)
//     wrap.wrap_to(io.stdout(), text, o)!;             // the same, written out
//     wrap.width("日本語")                              // 6 columns
//     let d = wrap.dedent("    a\n      b\n")!;         // "a\n  b\n"; mem.free(d)
//
// Widths are display columns as terminals show them: grapheme clusters
// (std/unicode), two columns for East Asian wide and full-width characters
// and emoji, none for combining and other zero-width marks and for control
// characters. Lines break only at the break opportunities of the Unicode
// Line Breaking Algorithm (UAX #14, std/unicode/linebreak): after spaces,
// after hyphens, between ideographs, and so on, never inside a grapheme
// cluster. A word wider than a line is broken between grapheme clusters
// (break_long_words), or set alone on an overlong line. With a Hyphenator
// (std/text/hyphen), a word that does not fit at the end of a line is
// hyphenated if a piece of it fits.
//
// For ASCII text the results are those of Python's textwrap with the same
// options (expand_tabs, replace_whitespace, drop_whitespace,
// break_long_words and break_on_hyphens; checked on generated cases,
// std/text/wrap/testdata, from tools/gen_wrap_tests.py), except where
// UAX #14 and textwrap's word pattern disagree: UAX #14 also breaks after
// a hyphen between single letters ("e-mail") and after a run of hyphens
// ("--option"). Each whitespace character is a space in the output, as in
// textwrap; Whitespace.COLLAPSE instead turns each run into one space and
// trims the ends. With keep_newlines, line breaks in the text are kept and
// each line is wrapped on its own (textwrap turns them into spaces). Only
// ASCII whitespace is dropped at line ends; other spaces (U+00A0, U+3000)
// are part of the words around them.
//
// dedent and indent are Python's textwrap.dedent and textwrap.indent.
// =============================================================================

import "../collections/vec.ph" as vec;
import "../encoding/json.ph" as json;
import "../io.ph" as io;
import "../mem.ph" as mem;
import "../os/fs.ph" as fs;
import "../str.ph" as strs;
import "../unicode.ph" as uni;
import "../unicode/linebreak.ph" as linebreak;
import "hyphen.ph" as hyphen;

error { InvalidWidth, OutOfMemory }

/// How whitespace inside the text is treated.
pub enum Whitespace: u8 {
    KEEP,      // each whitespace character becomes a space (textwrap's replace_whitespace)
    COLLAPSE,  // each run becomes one space; leading and trailing whitespace go
}

/// Hyphenation of words that do not fit at the end of a line.
pub union Hyphenation {
    Off,
    With: *hyphen.Hyphenator,
}

/// What wrap does; Options.init(width) gives textwrap's defaults.
pub struct Options {
    width: usize,             // columns per line, indent included (at least 1)
    initial_indent: []u8,     // before the first line
    subsequent_indent: []u8,  // before the others
    whitespace: Whitespace,
    keep_newlines: bool,     // keep the text's line breaks (else they are spaces)
    drop_whitespace: bool,   // drop spaces at the start (except the first line's) and end of lines
    break_long_words: bool,  // break words longer than a line (else they overflow)
    break_on_hyphens: bool,  // allow breaks after hyphens
    tab_size: usize,         // tabs expand to stops this far apart; 0: a tab is a space
    hyphenation: Hyphenation,

    pub fn init(columns: usize) Options {
        return Options.{
            width: columns,
            initial_indent: "",
            subsequent_indent: "",
            whitespace: Whitespace.KEEP,
            keep_newlines: false,
            drop_whitespace: true,
            break_long_words: true,
            break_on_hyphens: true,
            tab_size: 8,
            hyphenation: Hyphenation.Off,
        };
    }
}

/// The columns s takes in a terminal (see the module comment).
pub fn width(s: []u8) usize {
    return uni.string_width(s);
}

// -----------------------------------------------------------------------------
// Wrapping
// -----------------------------------------------------------------------------

/// The lines of text wrapped to opts.width, without line endings (none for
/// text that is empty or all whitespace). Free them with free_lines.
pub fn wrap(text: []u8, opts: Options) ![]mut []mut u8 {
    let mut lines = vec.Vec.[[]mut u8].init();
    let mut out = Out.{ sink: Sink.Collect(&mut lines), count: 0 };
    run(text, opts, &mut out) catch |e| {
        for l in lines.items() {
            mem.free(l);
        }
        lines.deinit();
        return e;
    };
    let items = lines.items();
    let res = mem.alloc.[[]mut u8](items.len) catch |e| {
        for l in items {
            mem.free(l);
        }
        lines.deinit();
        return e;
    };
    @mem_copy(res, items);
    lines.deinit();
    return res;
}

/// Frees what wrap returned.
pub fn free_lines(lines: []mut []mut u8) void {
    for l in lines {
        mem.free(l);
    }
    mem.free(lines);
}

/// Writes the wrapped lines to w, separated by newlines (no newline after
/// the last): what fill returns.
pub fn wrap_to(w: io.Writer, text: []u8, opts: Options) !void {
    let mut out = Out.{ sink: Sink.Write(w), count: 0 };
    run(text, opts, &mut out)!;
}

/// The wrapped lines joined by newlines, in new memory (free with mem.free).
pub fn fill(text: []u8, opts: Options) ![]mut u8 {
    let mut s = strs.String.init();
    wrap_to(s.writer(), text, opts) catch |e| {
        s.deinit();
        return e;
    };
    return s.take();
}

union Sink {
    Collect: *mut vec.Vec.[[]mut u8],
    Write: io.Writer,
}

struct Out {
    sink: Sink,
    count: usize,  // lines so far

    fn emit(self: *mut Out, line: []u8) !void {
        match self.sink {
            .Collect(v) => {
                let copy = mem.dup(line)!;
                v.push(copy) catch |e| {
                    mem.free(copy);
                    return e;
                };
            }
            .Write(w) => {
                if self.count > 0 {
                    w.write_all("\n")!;
                }
                w.write_all(line)!;
            }
        }
        self.count += 1;
    }
}

/// A piece of a line: a word or a run of spaces (or an added hyphen).
struct Piece {
    s: []u8,
    width: usize,
    space: bool,
    whole: []u8,  // the chunk s is (the rest of), for hyphenation
}

fn is_ascii_space(c: u8) bool {
    return (c == ' ') or (c == '\t') or (c == '\n') or (c == '\r') or (c == 0x0b) or (c == 0x0c);
}

// The length of a line separator at s[i..] (in keep_newlines mode), or 0.
fn newline_len(s: []u8, i: usize) usize {
    let c = s[i];
    if c == '\n' {
        return 1;
    }
    if c == '\r' {
        if ((i + 1) < s.len) and (s[i + 1] == '\n') {
            return 2;
        }
        return 1;
    }
    // U+0085, U+2028, U+2029
    if ((c == 0xc2) and ((i + 1) < s.len)) and (s[i + 1] == 0x85) {
        return 2;
    }
    if ((c == 0xe2) and ((i + 2) < s.len)) and ((s[i + 1] == 0x80) and ((s[i + 2] == 0xa8) or (s[i + 2] == 0xa9))) {
        return 3;
    }
    return 0;
}

fn run(text: []u8, opts: Options, out: *mut Out) !void {
    if opts.width == 0 {
        return error.InvalidWidth;
    }
    let mut para = strs.String.init();
    defer para.deinit();
    let mut line = strs.String.init();
    defer line.deinit();
    if not opts.keep_newlines {
        prepare(text, opts, &mut para)!;
        wrap_paragraph(para.bytes(), opts, out, &mut line)!;
        return;
    }
    let mut start: usize = 0;
    let mut i: usize = 0;
    while i < text.len {
        let nl = newline_len(text, i);
        if nl == 0 {
            i += 1;
            continue;
        }
        para.clear();
        prepare(text[start..i], opts, &mut para)!;
        let before = out.count;
        wrap_paragraph(para.bytes(), opts, out, &mut line)!;
        if out.count == before {
            out.emit("")!;  // an empty (or blank) line stays a line
        }
        i += nl;
        start = i;
    }
    if start < text.len {
        para.clear();
        prepare(text[start..], opts, &mut para)!;
        let before = out.count;
        wrap_paragraph(para.bytes(), opts, out, &mut line)!;
        if out.count == before {
            out.emit("")!;
        }
    }
}

// Expands tabs and turns whitespace into spaces (collapsing it if asked).
fn prepare(text: []u8, opts: Options, out: *mut strs.String) !void {
    let mut col: usize = 0;
    let mut i: usize = 0;
    let collapse = opts.whitespace == Whitespace.COLLAPSE;
    let mut pending_space = false;  // COLLAPSE: a space to write before the next word
    while i < text.len {
        let c = text[i];
        if is_ascii_space(c) {
            let mut n: usize = 1;
            if (c == '\t') and (opts.tab_size > 0) {
                n = opts.tab_size - (col % opts.tab_size);
            }
            if (c == '\n') or (c == '\r') {
                col = 0;
            } else {
                col += n;
            }
            if collapse {
                pending_space = out.len > 0;
            } else {
                for k in @as(usize, 0)..n {
                    out.push(' ')!;
                }
            }
            i += 1;
            continue;
        }
        if pending_space {
            out.push(' ')!;
            pending_space = false;
        }
        let r = strs.decode_rune(text[i..]);
        let w = uni.width(r.value);
        if w > 0 {
            col += @as(usize, @bit_cast(u32, w));
        }
        out.push_str(text[i..i + r.len])!;
        i += r.len;
    }
}

// Splits a prepared paragraph into pieces at its break opportunities and
// sets them into lines, as textwrap's _wrap_chunks.
fn wrap_paragraph(t: []u8, opts: Options, out: *mut Out, line: *mut strs.String) !void {
    let mut chunks = vec.Vec.[Piece].init();
    defer chunks.deinit();
    if t.len == 0 {
        return;
    }
    let mut br = linebreak.Breaks.init(t)!;
    defer br.deinit();
    let mut start: usize = 0;
    loop {
        let b = br.next() catch break;
        let p = b.pos;
        if (p < t.len) and ((not opts.break_on_hyphens) and (t[p - 1] == '-')) {
            continue;
        }
        // the word, then its trailing spaces
        let mut e = p;
        while (e > start) and (t[e - 1] == ' ') {
            e -= 1;
        }
        if e > start {
            chunks.push(Piece.{ s: t[start..e], width: width(t[start..e]), space: false, whole: t[start..e] })!;
        }
        if p > e {
            chunks.push(Piece.{ s: t[e..p], width: p - e, space: true, whole: t[e..p] })!;
        }
        start = p;
    }
    let cs = chunks.items();
    let mut pieces = vec.Vec.[Piece].init();
    defer pieces.deinit();
    let mut ci: usize = 0;
    let mut lines_here: usize = 0;
    while ci < cs.len {
        pieces.clear();
        let mut lead = opts.subsequent_indent;
        if out.count == 0 {
            lead = opts.initial_indent;
        }
        // the columns left after the indent (may be negative)
        let avail = @int_cast(i64, opts.width) - @int_cast(i64, width(lead));
        let mut cur: i64 = 0;
        if opts.drop_whitespace and cs[ci].space and (lines_here > 0) {
            ci += 1;
        }
        while (ci < cs.len) and ((cur + @int_cast(i64, cs[ci].width)) <= avail) {
            pieces.push(cs[ci])!;
            cur += @int_cast(i64, cs[ci].width);
            ci += 1;
        }
        // with hyphenation, a word that does not fit is hyphenated, or a
        // long one starts the next line (to be hyphenated there) rather than
        // filling this one
        let mut hyphenated = false;
        if (ci < cs.len) and not cs[ci].space {
            match opts.hyphenation {
                .With(h) => {
                    hyphenated = try_hyphenate(h, &mut cs[ci], avail - cur, &mut pieces)! or (pieces.len > 0);
                }
                .Off => {}
            }
        }
        if ((not hyphenated) and (ci < cs.len)) and (@int_cast(i64, cs[ci].width) > avail) {
            if long_word(&mut cs[ci], opts, avail, cur, &mut pieces)! {
                ci += 1;
            }
        }
        let ps = pieces.items();
        let mut np = ps.len;
        if (opts.drop_whitespace and (np > 0)) and ps[np - 1].space {
            np -= 1;
        }
        if np > 0 {
            line.clear();
            line.push_str(lead)!;
            for k in @as(usize, 0)..np {
                line.push_str(ps[k].s)!;
            }
            out.emit(line.bytes())!;
            lines_here += 1;
        }
    }
}

// Breaks a word too long for a line at grapheme boundaries, putting what
// fits in the rest of this line into pieces (textwrap's _handle_long_word).
// Returns whether the whole chunk was used.
fn long_word(c: *mut Piece, opts: Options, avail: i64, cur: i64, pieces: *mut vec.Vec.[Piece]) !bool {
    let mut space_left: i64 = 1;
    if avail >= 1 {
        space_left = avail - cur;
    }
    if not opts.break_long_words {
        if pieces.len == 0 {
            pieces.push(c.*)!;
            return true;
        }
        return false;
    }
    let s = c.s;
    let mut g = uni.graphemes(s);
    let mut used: usize = 0;
    let mut w: i64 = 0;
    loop {
        let cl = g.next() catch break;
        let cw = @int_cast(i64, width(cl));
        if (w + cw) > space_left {
            if (used == 0) and (pieces.len == 0) {
                used = cl.len;  // nothing fits: one cluster anyway
                w = cw;
            }
            break;
        }
        used += cl.len;
        w += cw;
    }
    // textwrap appends the (possibly empty) piece; an empty one counts as
    // whitespace, so drop_whitespace removes it rather than a space before it
    pieces.push(Piece.{ s: s[..used], width: @int_cast(usize, w), space: c.space or (used == 0), whole: c.whole })!;
    c.s = s[used..];
    c.width = width(c.s);
    return c.s.len == 0;
}

// If a hyphenated piece of word c fits in room columns, adds it and a
// hyphen to pieces, leaves the rest in c, and returns true. The places come
// from the whole word, even when c is what is left of it.
fn try_hyphenate(h: *hyphen.Hyphenator, c: *mut Piece, room: i64, pieces: *mut vec.Vec.[Piece]) !bool {
    let s = c.s;
    let whole = c.whole;
    let done = @int_from_ptr(s.ptr) - @int_from_ptr(whole.ptr);  // bytes of whole already set
    // the letters of the word, without punctuation around them
    let mut a: usize = 0;
    while (a < whole.len) and not uni.is_letter(strs.decode_rune(whole[a..]).value) {
        a += strs.decode_rune(whole[a..]).len;
    }
    let mut b = a;
    while b < whole.len {
        let r = strs.decode_rune(whole[b..]);
        if not uni.is_letter(r.value) {
            break;
        }
        b += r.len;
    }
    if (b - a) > hyphen.MAX_WORD {
        return false;
    }
    let mut at: [256]usize = [0; 256];
    let n = h.breaks(whole[a..b], at[..])!;
    let hw = @int_cast(i64, width("-"));
    let mut j = n;
    while j > 0 {
        j -= 1;
        let p = a + at[j];
        if p <= done {
            break;
        }
        let w = @int_cast(i64, width(whole[done..p]));
        if (w + hw) <= room {
            pieces.push(Piece.{ s: whole[done..p], width: @int_cast(usize, w), space: false, whole: whole })!;
            pieces.push(Piece.{ s: "-", width: 1, space: false, whole: whole })!;
            c.s = whole[p..];
            c.width = width(c.s);
            return true;
        }
    }
    return false;
}

// -----------------------------------------------------------------------------
// Dedent and indent
// -----------------------------------------------------------------------------

// The end of the line starting at i (before its "\n"), as Python's
// MULTILINE regular expressions see lines.
fn line_end(s: []u8, i: usize) usize {
    let mut e = i;
    while (e < s.len) and (s[e] != '\n') {
        e += 1;
    }
    return e;
}

/// Writes text without the whitespace (spaces and tabs) common to the
/// start of all its lines that have anything else, as Python's
/// textwrap.dedent; lines of only spaces and tabs become empty.
pub fn dedent_to(w: io.Writer, text: []u8) !void {
    // the margin: the longest common prefix of the non-blank lines' indents
    let mut margin = text[0..0];
    let mut have = false;
    let mut i: usize = 0;
    while i <= text.len {
        let e = line_end(text, i);
        let l = text[i..e];
        let mut k: usize = 0;
        while (k < l.len) and ((l[k] == ' ') or (l[k] == '\t')) {
            k += 1;
        }
        if k < l.len {
            let ind = l[..k];
            if not have {
                margin = ind;
                have = true;
            } else {
                let mut c: usize = 0;
                while ((c < margin.len) and (c < ind.len)) and (margin[c] == ind[c]) {
                    c += 1;
                }
                margin = margin[..c];
            }
        }
        if e == text.len {
            break;
        }
        i = e + 1;
    }
    i = 0;
    while i <= text.len {
        let e = line_end(text, i);
        let l = text[i..e];
        let mut k: usize = 0;
        while (k < l.len) and ((l[k] == ' ') or (l[k] == '\t')) {
            k += 1;
        }
        if k == l.len {
            // blank: nothing but the newline
        } else if strs.starts_with(l, margin) {
            w.write_all(l[margin.len..])!;
        } else {
            w.write_all(l)!;
        }
        if e == text.len {
            break;
        }
        w.write_all("\n")!;
        i = e + 1;
    }
}

/// dedent_to's text in new memory (free with mem.free).
pub fn dedent(text: []u8) ![]mut u8 {
    let mut s = strs.String.init();
    dedent_to(s.writer(), text) catch |e| {
        s.deinit();
        return e;
    };
    return s.take();
}

// The length of the line separator at s[i..] as Python's str.splitlines
// sees them, or 0.
fn splitlines_sep(s: []u8, i: usize) usize {
    let c = s[i];
    if (c == '\n') or (c == 0x0b) or (c == 0x0c) or (c == 0x1c) or (c == 0x1d) or (c == 0x1e) {
        return 1;
    }
    return newline_len(s, i);
}

fn is_blank(l: []u8) bool {
    let mut i: usize = 0;
    while i < l.len {
        let r = strs.decode_rune(l[i..]);
        if not (uni.is_space(r.value) or ((r.value >= 0x1c) and (r.value <= 0x1f))) {
            return false;
        }
        i += r.len;
    }
    return true;
}

/// Writes text with prefix before each line that is not blank (empty or
/// all whitespace), as Python's textwrap.indent; lines end as
/// str.splitlines sees them.
pub fn indent_to(w: io.Writer, text: []u8, prefix: []u8) !void {
    let mut start: usize = 0;
    let mut i: usize = 0;
    while start < text.len {
        let mut sep: usize = 0;
        while i < text.len {
            sep = splitlines_sep(text, i);
            if sep > 0 {
                break;
            }
            i += 1;
        }
        let end = @min(i + sep, text.len);
        let l = text[start..end];
        if not is_blank(l) {
            w.write_all(prefix)!;
        }
        w.write_all(l)!;
        i = end;
        start = end;
    }
}

/// indent_to's text in new memory (free with mem.free).
pub fn indent(text: []u8, prefix: []u8) ![]mut u8 {
    let mut s = strs.String.init();
    indent_to(s.writer(), text, prefix) catch |e| {
        s.deinit();
        return e;
    };
    return s.take();
}

// -----------------------------------------------------------------------------
// Tests
// -----------------------------------------------------------------------------

fn wrapped(text: []u8, opts: Options) ![]mut u8 {
    let lines = wrap(text, opts)!;
    defer free_lines(lines);
    let mut s = strs.String.init();
    for l in lines {
        s.push_str("[")!;
        s.push_str(l)!;
        s.push_str("]")!;
    }
    return s.take();
}

fn check(text: []u8, opts: Options, want: []u8) !void {
    let got = wrapped(text, opts)!;
    defer mem.free(got);
    @expect_eq(got, want);
}

test "basics" {
    let o = Options.init(10);
    check("The quick brown fox jumps over the lazy dog", o, "[The quick][brown fox][jumps over][the lazy][dog]")!;
    check("", o, "")!;
    check("   ", o, "")!;
    check("  indented text here", o, "[  indented][text here]")!;
    check("a  b   c", Options.init(80), "[a  b   c]")!;
    // long words: broken, filling the line first, or left whole
    check("ab cdefghijklmnop", Options.init(5), "[ab cd][efghi][jklmn][op]")!;
    let mut nb = Options.init(5);
    nb.break_long_words = false;
    check("ab cdefghijklmnop q", nb, "[ab][cdefghijklmnop][q]")!;
    // hyphens are break opportunities
    check("a well-known example", Options.init(12), "[a well-known][example]")!;
    check("a well-known example", Options.init(10), "[a well-][known][example]")!;
    let mut nh = Options.init(10);
    nh.break_on_hyphens = false;
    check("a well-known example", nh, "[a][well-known][example]")!;
    _ = wrap("x", Options.init(0)) catch |e| {
        @expect(e == error.InvalidWidth);
        return;
    };
    @trap();
}

test "indents and whitespace" {
    let mut o = Options.init(12);
    o.initial_indent = "* ";
    o.subsequent_indent = "  ";
    check("one two three four five", o, "[* one two][  three four][  five]")!;
    let mut c = Options.init(20);
    c.whitespace = Whitespace.COLLAPSE;
    check("  lots   of\t\tspace \n here  ", c, "[lots of space here]")!;
    let mut k = Options.init(20);
    k.keep_newlines = true;
    check("first line\n\nthird line which is long\n", k, "[first line][][third line which is][long]")!;
    // tabs expand to stops of 8 (then are spaces)
    check("a\tb", Options.init(20), "[a       b]")!;
    let mut t0 = Options.init(20);
    t0.tab_size = 0;
    check("a\tb", t0, "[a b]")!;
    let mut kd = Options.init(5);
    kd.drop_whitespace = false;
    check("abc   def", kd, "[abc][   ][def]")!;
}

test "wide characters" {
    @expect(width("日本語") == 6);
    @expect(width("e\u{301}") == 1);
    @expect(width("🇫🇷") == 2);
    @expect(width("👩\u{200d}💻") == 2);
    // ideographs break anywhere (except before closing punctuation)
    check("日本語のテキストです。", Options.init(8), "[日本語の][テキスト][です。]")!;
    check("日本語のテキストです。", Options.init(7), "[日本語][のテキ][ストで][す。]")!;
    // mixed scripts; a wide character never hangs past the edge
    check("abc 日本語", Options.init(6), "[abc 日][本語]")!;
    // combining marks stay with their letters
    check("cafe\u{301} cafe\u{301}", Options.init(5), "[cafe\u{301}][cafe\u{301}]")!;
    check("e\u{301}e\u{301}e\u{301}e\u{301}", Options.init(3), "[e\u{301}e\u{301}e\u{301}][e\u{301}]")!;
    // emoji: two columns each, sequences kept whole
    check("🙂🙂🙂 👩\u{200d}💻👩\u{200d}💻", Options.init(4), "[🙂🙂][🙂][👩\u{200d}💻👩\u{200d}💻]")!;
    // a wide character in a one-column line still makes progress
    check("日本", Options.init(1), "[日][本]")!;
}

test "hyphenation" {
    let mut h = hyphen.Hyphenator.en_us()!;
    defer h.deinit();
    let mut o = Options.init(12);
    o.hyphenation = Hyphenation.With(&h);
    check("the hyphenation of words", o, "[the hyphen-][ation of][words]")!;
    // pieces of a long word, at the whole word's places (su-per-cal-ifrag-ilis-tic-ex-pi-ali-do-cious)
    o.width = 16;
    check("a supercalifragilisticexpialidocious word", o, "[a supercalifrag-][ilisticexpiali-][docious word]")!;
    o.width = 10;
    check("supercalifragilisticexpialidocious", o, "[supercal-][ifragilis-][ticexpi-][alidocious]")!;
    // punctuation stays with the second half
    o.width = 14;
    check("see hyphenation, again", o, "[see hyphen-][ation, again]")!;
    // no place fits: as without hyphenation
    o.width = 5;
    check("ab hyphenation", o, "[ab][hy-][phen-][ation]")!;
}

test "fill and wrap_to" {
    let s = fill("aaa bbb ccc", Options.init(7))!;
    defer mem.free(s);
    @expect_eq(s, "aaa bbb\nccc");
    let mut b = strs.String.init();
    defer b.deinit();
    wrap_to(b.writer(), "aaa bbb ccc", Options.init(3))!;
    @expect_eq(b.bytes(), "aaa\nbbb\nccc");
}

test "dedent and indent" {
    let a = dedent("    hello\n      world\n")!;
    defer mem.free(a);
    @expect_eq(a, "hello\n  world\n");
    let b = dedent("  a\n\t b\n")!;
    defer mem.free(b);
    @expect_eq(b, "  a\n\t b\n");
    let c = dedent("   x\n  \n    y")!;
    defer mem.free(c);
    @expect_eq(c, "x\n\n y");
    let d = indent("a\n\n  \nb\r\nc", "> ")!;
    defer mem.free(d);
    @expect_eq(d, "> a\n\n  \n> b\r\n> c");
}

test "against Python's textwrap" {
    let data = fs.read_file("std/text/wrap/testdata/cases.json") catch return;
    defer mem.free(data);
    let mut doc = json.parse(data)!;
    defer doc.deinit();
    let cases = doc.root.as_array()!;
    let mut n: usize = 0;
    for c in cases {
        let kind = c.get("op")!.as_str()!;
        let text = c.get("text")!.as_str()!;
        let want = c.get("want")!;
        if strs.eq(kind, "dedent") {
            let got = dedent(text)!;
            defer mem.free(got);
            if not strs.eq(got, want.as_str()!) {
                @expect_eq(got, want.as_str()!);
            }
            n += 1;
            continue;
        }
        if strs.eq(kind, "indent") {
            let got = indent(text, c.get("prefix")!.as_str()!)!;
            defer mem.free(got);
            if not strs.eq(got, want.as_str()!) {
                @expect_eq(got, want.as_str()!);
            }
            n += 1;
            continue;
        }
        let mut o = Options.init(@int_cast(usize, c.get("width")!.as_i64()!));
        o.initial_indent = c.get("initial_indent")!.as_str()!;
        o.subsequent_indent = c.get("subsequent_indent")!.as_str()!;
        o.drop_whitespace = c.get("drop_whitespace")!.as_bool()!;
        o.break_long_words = c.get("break_long_words")!.as_bool()!;
        o.break_on_hyphens = c.get("break_on_hyphens")!.as_bool()!;
        o.tab_size = @int_cast(usize, c.get("tabsize")!.as_i64()!);
        let lines = wrap(text, o)!;
        defer free_lines(lines);
        let ws = want.as_array()!;
        let mut same = lines.len == ws.len;
        for k in @as(usize, 0)..@min(lines.len, ws.len) {
            same = same and strs.eq(lines[k], ws[k].as_str()!);
        }
        if not same {
            let got = fill(text, o)!;
            defer mem.free(got);
            let mut joined = strs.String.init();
            defer joined.deinit();
            for k in @as(usize, 0)..ws.len {
                if k > 0 {
                    joined.push_str("\n")!;
                }
                joined.push_str(ws[k].as_str()!)!;
            }
            @expect_eq(got, joined.bytes());
        }
        n += 1;
    }
    @expect(n >= 400);
}
