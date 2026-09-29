// =============================================================================
// std/text.ph - rich text layout: styled runs of text set in lines with
// Unicode line breaking (UAX #14) and bidirectional reordering (UAX #9),
// fonts with fallback, alignment and justification; drawn with std/draw.
//
//     let body = text.Style.init(&serif, 16.0, color.Color.rgb(0.1, 0.1, 0.1));
//     let mut bold = body;
//     bold.font = &serif_bold;
//     let spans: [2]text.Span = [text.Span.{ text: "Hello, ", style: body }, text.Span.{ text: "world", style: bold }];
//     let mut opts = text.Options.init(300.0);    // lines at most 300 px wide
//     opts.align = text.JUSTIFY;
//     let mut l = text.layout(spans[..], opts)!;
//     defer l.deinit();
//     l.draw(&mut canvas, 20.0, 40.0)!;          // top left of the text box
//
// Each code point is drawn with the first font of its style (then its
// fallbacks) that has a glyph for it; glyphs come from the fonts'
// outlines with kerning, without further shaping (no ligatures and no
// contextual forms, so scripts that need them, such as Arabic, show their
// isolated letters).
// =============================================================================

import "color.ph" as color;
import "draw.ph" as draw;
import "font.ph" as font;
import "mem.ph" as mem;
import "str.ph" as strs;
import "unicode.ph" as uni;
import "unicode/bidi.ph" as bidi;
import "unicode/linebreak.ph" as linebreak;
import "vec.ph" as vec;

error { OutOfMemory, Done }

/// Alignments.
pub const START: u8 = 0;  // left in left-to-right paragraphs, right in right-to-left ones
pub const END: u8 = 1;
pub const LEFT: u8 = 2;
pub const RIGHT: u8 = 3;
pub const CENTER: u8 = 4;
pub const JUSTIFY: u8 = 5;  // both edges (the last line of a paragraph as START)

/// How a run of text looks.
pub struct Style {
    font: *font.Font,
    fallbacks: []*font.Font,  // tried in order for characters the font lacks
    size: f64,                // the em, in pixels
    color: color.Color,
    underline: bool,
    strike: bool,
    letter_spacing: f64,  // extra pixels after each character

    pub fn init(f: *font.Font, size: f64, c: color.Color) Style {
        return Style.{
            font: f,
            fallbacks: no_fallbacks(),
            size: size,
            color: c,
            underline: false,
            strike: false,
            letter_spacing: 0.0,
        };
    }
}

fn no_fallbacks() []*font.Font {
    return @slice_from_ptr(@ptr_from_int(**font.Font, 8), 0);
}

/// Some text in one style.
pub struct Span {
    text: []u8,
    style: Style,
}

pub struct Options {
    width: f64,  // the longest a line may be (0: lines break only at newlines)
    align: u8,
    direction: i32,     // bidi.AUTO, bidi.LTR or bidi.RTL, for each paragraph
    line_spacing: f64,  // a multiple of the natural line height
    tab_stops: f64,     // tabs advance to the next multiple of this (0: 4 spaces of the font)

    pub fn init(width: f64) Options {
        return Options.{ width: width, align: START, direction: bidi.AUTO, line_spacing: 1.0, tab_stops: 0.0 };
    }
}

/// A placed glyph.
pub struct Glyph {
    id: u32,
    font: *font.Font,
    x: f64,  // pen position (on the baseline)
    y: f64,
    size: f64,
    span: usize,    // which span it came from
    offset: usize,  // the byte offset of its character in the concatenated text
    advance: f64,
}

/// A line of the layout.
pub struct Line {
    start: usize,  // byte offsets of its text, in the concatenated spans
    end: usize,
    baseline: f64,  // from the top of the layout
    ascent: f64,
    descent: f64,  // positive: below the baseline
    left: f64,     // where its visible content starts and ends
    right: f64,
    first_glyph: usize,
    glyph_count: usize,
    rtl: bool,  // the paragraph's direction
}

/// Laid-out text.
pub struct Layout {
    glyphs: vec.Vec.[Glyph],
    lines: vec.Vec.[Line],
    spans: vec.Vec.[Span],
    width: f64,  // of the widest line (or opts.width when that is set)
    height: f64,

    pub fn deinit(self: *mut Layout) void {
        self.glyphs.deinit();
        self.lines.deinit();
        self.spans.deinit();
    }

    /// Draws the text with its top left corner at (x, y).
    pub fn draw(self: *Layout, cv: *mut draw.Canvas, x: f64, y: f64) !void {
        let gs = self.glyphs.items();
        let mut i: usize = 0;
        let mut path = draw.Path.init();
        defer path.deinit();
        while i < gs.len {
            // glyphs of one span share a color: one path each
            let sp = gs[i].span;
            path.clear();
            while (i < gs.len) and (gs[i].span == sp) {
                let g = gs[i];
                let scale = font.scale_for(g.font, g.size);
                g.font.glyph_path(g.id, &mut path, x + g.x, y + g.y, scale)!;
                i += 1;
            }
            cv.fill(&path, draw.Paint.solid(self.spans.get(sp).style.color), draw.NONZERO)!;
        }
        // underlines and strike-throughs
        for g in gs {
            let st = self.spans.get(g.span).style;
            if not st.underline and not st.strike {
                continue;
            }
            let thick = @max(st.size / 16.0, 1.0);
            let paint = draw.Paint.solid(st.color);
            if st.underline {
                cv.fill_rect(x + g.x, y + g.y + (st.size * 0.12), g.advance, thick, paint)!;
            }
            if st.strike {
                cv.fill_rect(x + g.x, (y + g.y) - (st.size * 0.28), g.advance, thick, paint)!;
            }
        }
    }

    /// The byte offset (in the concatenated text) nearest a point, for
    /// placing a caret.
    pub fn offset_at(self: *Layout, x: f64, y: f64) usize {
        let ls = self.lines.items();
        if ls.len == 0 {
            return 0;
        }
        let mut li: usize = ls.len - 1;
        for k in @as(usize, 0)..ls.len {
            if y < (ls[k].baseline + ls[k].descent) {
                li = k;
                break;
            }
        }
        let line = ls[li];
        let gs = self.glyphs.items()[line.first_glyph..line.first_glyph + line.glyph_count];
        if gs.len == 0 {
            return line.start;
        }
        for g in gs {
            if x < (g.x + (g.advance / 2.0)) {
                if line.rtl {
                    return g.offset + 1;
                }
                return g.offset;
            }
        }
        if line.rtl {
            return line.start;
        }
        return line.end;
    }
}

// -----------------------------------------------------------------------------
// Layout
// -----------------------------------------------------------------------------

/// A code point of a paragraph, measured.
struct Cell {
    cp: u32,
    offset: usize,  // byte offset in the concatenated text
    len: usize,
    span: usize,
    gid: u32,
    font: *font.Font,
    advance: f64,
}

fn pick_font(st: *Style, cp: u32, gid: *mut u32) *font.Font {
    let g = st.font.glyph_index(cp);
    if g != 0 {
        gid.* = g;
        return st.font;
    }
    for f in st.fallbacks {
        let h = f.glyph_index(cp);
        if h != 0 {
            gid.* = h;
            return f;
        }
    }
    gid.* = 0;
    return st.font;  // its .notdef
}

fn is_newline(cp: u32) bool {
    return (cp == '\n') or (cp == '\r') or (cp == 0x0b) or (cp == 0x0c) or (cp == 0x85) or (cp == 0x2028) or
           (cp == 0x2029);
}

/// Lays out spans of text.
pub fn layout(spans: []Span, opts: Options) !Layout {
    let mut out = Layout.{
        glyphs: vec.Vec.[Glyph].init(),
        lines: vec.Vec.[Line].init(),
        spans: vec.Vec.[Span].init(),
        width: 0.0,
        height: 0.0,
    };
    layout_into(spans, opts, &mut out) catch |e| {
        out.deinit();
        return e;
    };
    return out;
}

fn layout_into(spans: []Span, opts: Options, out: *mut Layout) !void {
    out.spans.extend(spans)!;
    // the whole text, and each byte's span
    let mut all = strs.String.init();
    defer all.deinit();
    let mut span_of = vec.Vec.[usize].init();
    defer span_of.deinit();
    for k in @as(usize, 0)..spans.len {
        all.push_str(spans[k].text)!;
        for _b in spans[k].text {
            span_of.push(k)!;
        }
    }
    let s = all.bytes();
    // paragraphs end after a paragraph or line separator
    let mut y: f64 = 0.0;
    let mut start: usize = 0;
    if s.len == 0 {
        return;
    }
    while start < s.len {
        let mut end = start;
        while end < s.len {
            let r = strs.decode_rune(s[end..]);
            end += r.len;
            if (r.value == '\n') or (r.value == 0x2029) or (r.value == 0x85) or
               ((r.value == '\r') and not ((end < s.len) and (s[end] == '\n'))) {
                break;
            }
        }
        y = paragraph(s, start, end, span_of.items(), spans, opts, y, out)!;
        start = end;
    }
    out.height = y;
    if opts.width > 0.0 {
        out.width = @max(out.width, opts.width);
    }
}

/// Lays out the paragraph s[start..end] with its first line's top at y;
/// returns where the next paragraph starts.
fn paragraph(s: []u8, start: usize, end: usize, span_of: []usize, spans: []Span, opts: Options, y0: f64,
             out: *mut Layout) !f64 {
    // measure each code point
    let mut cells = vec.Vec.[Cell].init();
    defer cells.deinit();
    let mut cps = vec.Vec.[u32].init();
    defer cps.deinit();
    let mut i = start;
    while i < end {
        let r = strs.decode_rune(s[i..]);
        let sp = span_of[i];
        let st = &spans[sp].style;
        let mut gid: u32 = 0;
        let f = pick_font(st, r.value, &mut gid);
        let scale = font.scale_for(f, st.size);
        let mut adv = (@float_from_int(f64, f.advance(gid)) * scale) + st.letter_spacing;
        if is_newline(r.value) or (uni.category(r.value) == uni.GC_CC) {
            adv = 0.0;
            gid = 0;
        }
        cells.push(Cell.{ cp: r.value, offset: i, len: r.len, span: sp, gid: gid, font: f, advance: adv })!;
        cps.push(r.value)!;
        i += r.len;
    }
    let cs = cells.items();
    let n = cs.len;
    // bidi levels
    let mut bp = bidi.Paragraph.init(cps.items(), opts.direction)!;
    defer bp.deinit();
    let rtl = (bp.level & 1) == 1;
    // mirrored glyphs in right-to-left runs
    for k in @as(usize, 0)..n {
        if ((bp.levels[k] & 1) == 1) and uni.is_bidi_mirrored(cs[k].cp) {
            let m = uni.mirror(cs[k].cp);
            let g = cs[k].font.glyph_index(m);
            if g != 0 {
                cs[k].gid = g;
            }
        }
    }
    // tabs: to the next stop (measured from the line start when placed)
    let mut tab = opts.tab_stops;
    // break opportunities, as code point indices where a line may end
    let mut breaks = vec.Vec.[usize].init();
    defer breaks.deinit();
    let mut lb = linebreak.Breaks.init(s[start..end])!;
    defer lb.deinit();
    let mut ci: usize = 0;
    loop {
        let b = lb.next() catch break;
        while (ci < n) and ((cs[ci].offset - start) < b.pos) {
            ci += 1;
        }
        breaks.push(ci)!;
    }
    // greedy line filling
    let mut line_start: usize = 0;
    let mut y = y0;
    let mut bi: usize = 0;
    let bs = breaks.items();
    while line_start < n {
        let mut line_end = n;
        if opts.width > 0.0 {
            line_end = fit(cs, line_start, bs, &mut bi, opts.width, tab)!;
        }
        y = place_line(cs, &bp, line_start, line_end, (line_end == n), rtl, spans, opts, y, tab, out)!;
        line_start = line_end;
    }
    if n == 0 {
        // an empty paragraph: a blank line
        y = y + blank_height(spans, span_of, start, opts);
    }
    return y;
}

fn blank_height(spans: []Span, span_of: []usize, at: usize, opts: Options) f64 {
    let mut sp: usize = 0;
    if at < span_of.len {
        sp = span_of[at];
    } else if span_of.len > 0 {
        sp = span_of[span_of.len - 1];
    }
    let st = spans[sp].style;
    return font.line_height(st.font, st.size) * opts.line_spacing;
}

fn trailing_space(cp: u32) bool {
    return uni.is_space(cp) and (cp != 0xa0) and (cp != 0x202f);
}

/// The width of cells [a, b) without trailing white space.
fn visible_width(cs: []Cell, a: usize, b: usize, tab: f64) f64 {
    let mut e = b;
    while (e > a) and (trailing_space(cs[e - 1].cp) or is_newline(cs[e - 1].cp)) {
        e -= 1;
    }
    let mut w: f64 = 0.0;
    for k in a..e {
        if (cs[k].cp == '\t') and (tab > 0.0) {
            w = (@floor(w / tab) + 1.0) * tab;
        } else {
            w += cs[k].advance;
        }
    }
    return w;
}

/// Where the line starting at cell `a` ends: the last break opportunity
/// that fits (or a mandatory one), else a grapheme boundary.
fn fit(cs: []Cell, a: usize, bs: []usize, bi: *mut usize, width: f64, tab: f64) !usize {
    // skip opportunities at or before the start
    while (bi.* < bs.len) and (bs[bi.*] <= a) {
        bi.* += 1;
    }
    let mut best: usize = 0;
    let mut k = bi.*;
    while k < bs.len {
        let b = bs[k];
        if visible_width(cs, a, b, tab) > width {
            break;
        }
        best = b;
        k += 1;
        // a newline ends the line
        if (b > 0) and is_newline(cs[b - 1].cp) {
            break;
        }
    }
    if best > a {
        bi.* = k;
        return best;
    }
    // nothing fits: break inside the first word, between characters
    let limit = bs[@min(bi.*, bs.len - 1)];
    let mut w: f64 = 0.0;
    let mut e = a;
    while e < limit {
        let nw = w + cs[e].advance;
        if (nw > width) and (e > a) {
            break;
        }
        w = nw;
        e += 1;
        // keep combining marks with their base
        while (e < limit) and (uni.width(cs[e].cp) == 0) and (cs[e].advance == 0.0) {
            e += 1;
        }
    }
    return e;
}

/// The width of a line's cells in visual order, with kerning and tabs,
/// leaving out the trailing white space from cell `e` on.
fn kerned_width(cs: []Cell, bp: *bidi.Paragraph, order: []usize, e: usize, spans: []Span, tab: f64) f64 {
    let mut x: f64 = 0.0;
    let mut prev_font: usize = 0;
    let mut prev_gid: u32 = 0;
    let mut prev_span: usize = @max_value(usize);
    let mut prev_level: u8 = 255;
    for k in order {
        let c = cs[k];
        if k >= e {
            continue;
        }
        if (prev_span == c.span) and (prev_font == @int_from_ptr(c.font)) and (prev_level == bp.levels[k]) and
           (prev_gid != 0) and (c.gid != 0) {
            x += @float_from_int(f64, c.font.kerning(prev_gid, c.gid)) * font.scale_for(
                c.font, spans[c.span].style.size);
        }
        if (c.cp == '\t') and (tab > 0.0) {
            x = (@floor(x / tab) + 1.0) * tab;
        } else {
            x += c.advance;
        }
        prev_font = @int_from_ptr(c.font);
        prev_gid = c.gid;
        prev_span = c.span;
        prev_level = bp.levels[k];
    }
    return x;
}

/// Places cells [a, b) as a line whose top is at y; returns the next line's top.
fn place_line(cs: []Cell, bp: *bidi.Paragraph, a: usize, b: usize, last: bool, rtl: bool, spans: []Span, opts: Options,
              y: f64, tab: f64, out: *mut Layout) !f64 {
    // metrics: the largest of the fonts on the line
    let mut asc: f64 = 0.0;
    let mut desc: f64 = 0.0;
    let mut gap: f64 = 0.0;
    for k in a..b {
        let c = cs[k];
        let st = spans[c.span].style;
        let sc = font.scale_for(c.font, st.size);
        asc = @max(asc, @float_from_int(f64, c.font.ascent) * sc);
        desc = @max(desc, @float_from_int(f64, -c.font.descent) * sc);
        gap = @max(gap, @float_from_int(f64, c.font.line_gap) * sc);
    }
    let natural = (asc + desc + gap) * opts.line_spacing;
    let baseline = y + (gap * opts.line_spacing * 0.5) + asc + ((opts.line_spacing - 1.0) * (asc + desc) * 0.5);
    // trailing white space takes no room
    let mut e = b;
    while (e > a) and (trailing_space(cs[e - 1].cp) or is_newline(cs[e - 1].cp)) {
        e -= 1;
    }
    // visual order
    let order = bp.visual_order(a, b)!;
    defer mem.free(order);
    let lw = kerned_width(cs, bp, order, e, spans, tab);
    // justification: extra room shared among the spaces
    let mut extra: f64 = 0.0;
    let ends_para = last or ((b > a) and is_newline(cs[b - 1].cp));
    if (opts.align == JUSTIFY) and not ends_para and (opts.width > lw) {
        let mut spaces: usize = 0;
        for k in a..e {
            if (cs[k].cp == ' ') or (cs[k].cp == 0x3000) {
                spaces += 1;
            }
        }
        if spaces > 0 {
            extra = (opts.width - lw) / @float_from_int(f64, spaces);
        }
    }
    let mut x: f64 = 0.0;
    let mut align = opts.align;
    if align == JUSTIFY {
        align = START;
        if extra > 0.0 {
            align = LEFT;
        }
    }
    if align == START {
        align = LEFT;
        if rtl {
            align = RIGHT;
        }
    } else if align == END {
        align = RIGHT;
        if rtl {
            align = LEFT;
        }
    }
    if opts.width > 0.0 {
        if align == RIGHT {
            x = opts.width - lw;
        } else if align == CENTER {
            x = (opts.width - lw) / 2.0;
        }
    }
    let left = x;
    let first = out.glyphs.len;
    let mut prev_font: usize = 0;
    let mut prev_gid: u32 = 0;
    let mut prev_span: usize = @max_value(usize);
    let mut prev_level: u8 = 255;
    for k in order {
        let c = cs[k];
        if k >= e {
            continue;  // trailing white space
        }
        let st = spans[c.span].style;
        // kerning with the glyph to the left, within a run of one font and direction
        if (prev_span == c.span) and (prev_font == @int_from_ptr(c.font)) and (prev_level == bp.levels[k]) and
           (prev_gid != 0) and (c.gid != 0) {
            x += @float_from_int(f64, c.font.kerning(prev_gid, c.gid)) * font.scale_for(c.font, st.size);
        }
        let mut adv = c.advance;
        if (c.cp == '\t') and (tab > 0.0) {
            adv = ((@floor((x - left) / tab) + 1.0) * tab) - (x - left);
        }
        if ((c.cp == ' ') or (c.cp == 0x3000)) and (extra > 0.0) {
            adv += extra;
        }
        if (c.gid != 0) and (c.cp != '\t') and not is_newline(c.cp) {
            out.glyphs.push(Glyph.{
                id: c.gid,
                font: c.font,
                x: x,
                y: baseline,
                size: st.size,
                span: c.span,
                offset: c.offset,
                advance: adv,
            })!;
        }
        x += adv;
        prev_font = @int_from_ptr(c.font);
        prev_gid = c.gid;
        prev_span = c.span;
        prev_level = bp.levels[k];
    }
    let mut ls = cs[a].offset;
    let mut le = ls;
    if b > a {
        le = cs[b - 1].offset + cs[b - 1].len;
    }
    _ = ls;
    out.lines.push(Line.{
        start: cs[a].offset,
        end: le,
        baseline: baseline,
        ascent: asc,
        descent: desc,
        left: left,
        right: x,
        first_glyph: first,
        glyph_count: out.glyphs.len - first,
        rtl: rtl,
    })!;
    out.width = @max(out.width, x);
    ls = 0;
    return y + natural;
}

/// The size a text would take: [width, height].
pub fn measure(spans: []Span, opts: Options) ![2]f64 {
    let mut l = layout(spans, opts)!;
    defer l.deinit();
    return [l.width, l.height];
}

/// Lays out and draws text in one style with its top left at (x, y).
pub fn draw_text(cv: *mut draw.Canvas, s: []u8, st: Style, x: f64, y: f64, opts: Options) !void {
    let sp: [1]Span = [Span.{ text: s, style: st }];
    let mut l = layout(sp[..], opts)!;
    defer l.deinit();
    l.draw(cv, x, y)!;
}

// -----------------------------------------------------------------------------
// Tests (with the system's fonts, when there are any)
// -----------------------------------------------------------------------------

import "image.ph" as image;

fn try_font(p: []u8) !font.Font {
    return font.Font.load(p);
}

test "lines, alignment and bidi" {
    let mut f = try_font("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf") catch return;
    defer f.deinit();
    let st = Style.init(&f, 20.0, color.Color.rgb(0.0, 0.0, 0.0));
    let sp: [1]Span = [Span.{ text: "The quick brown fox jumps over the lazy dog.", style: st }];
    let mut opts = Options.init(150.0);
    let mut l = layout(sp[..], opts)!;
    defer l.deinit();
    @expect(l.lines.len >= 3);
    for line in l.lines.items() {
        @expect((line.right - line.left) <= 150.001);
    }
    // the lines cover the text in order
    @expect_eq(l.lines.get(0).start, 0);
    @expect_eq(l.lines.get(l.lines.len - 1).end, sp[0].text.len);
    // right alignment puts each line's end at the width
    opts.align = RIGHT;
    let mut r = layout(sp[..], opts)!;
    defer r.deinit();
    for line in r.lines.items() {
        @expect(@abs(line.right - 150.0) < 0.001);
    }
    // justification fills every line but the last
    opts.align = JUSTIFY;
    let mut j = layout(sp[..], opts)!;
    defer j.deinit();
    let jl = j.lines.items();
    for k in @as(usize, 0)..jl.len - 1 {
        @expect(@abs(jl[k].right - 150.0) < 0.01);
    }
    // newlines end lines
    let two: [1]Span = [Span.{ text: "one\ntwo", style: st }];
    let mut nl = layout(two[..], Options.init(0.0))!;
    defer nl.deinit();
    @expect_eq(nl.lines.len, 2);
    @expect(nl.height > (2.0 * 20.0));
    // Hebrew is set right to left: its first letter is at the right
    let heb: [1]Span = [Span.{ text: "\xd7\x90\xd7\x91\xd7\x92 abc", style: st }];
    let mut h = layout(heb[..], Options.init(200.0))!;
    defer h.deinit();
    @expect(h.lines.get(0).rtl);
    let gs = h.glyphs.items();
    let mut alef_x: f64 = -1.0;
    let mut a_x: f64 = -1.0;
    for g in gs {
        if g.offset == 0 {
            alef_x = g.x;
        }
        if g.offset == 7 {
            a_x = g.x;
        }
    }
    @expect(alef_x > a_x);  // abc (on the left) then the Hebrew, read from the right
    @expect(@abs(h.lines.get(0).right - 200.0) < 0.001);  // START is the right edge
    // hit testing
    @expect_eq(l.offset_at(0.0, 5.0), 0);
    // drawing puts ink down
    let mut img = image.Image.init(160, 120, 1, image.U8)!;
    defer img.deinit();
    let mut cv = draw.Canvas.init(&mut img);
    defer cv.deinit();
    let mut white = st;
    white.color = color.Color.rgb(1.0, 1.0, 1.0);
    white.underline = true;
    let ws: [1]Span = [Span.{ text: "Hello there", style: white }];
    let mut wl = layout(ws[..], Options.init(150.0))!;
    defer wl.deinit();
    wl.draw(&mut cv, 5.0, 5.0)!;
    let mut ink: u64 = 0;
    for v in img.samples8() {
        ink += @as(u64, v);
    }
    @expect(ink > 10000);
}

test "font fallback" {
    let mut latin = try_font("/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf") catch return;
    defer latin.deinit();
    let mut cjk = try_font("/usr/share/fonts/truetype/wqy/wqy-zenhei.ttc") catch return;
    defer cjk.deinit();
    let fbs: [1]*font.Font = [&cjk];
    let mut st = Style.init(&latin, 16.0, color.Color.rgb(0.0, 0.0, 0.0));
    st.fallbacks = fbs[..];
    let sp: [1]Span = [Span.{ text: "Hi \xe4\xb8\xad\xe6\x96\x87", style: st }];
    let mut l = layout(sp[..], Options.init(0.0))!;
    defer l.deinit();
    let gs = l.glyphs.items();
    @expect_eq(gs.len, 5);  // H, i, space, 中, 文
    @expect_eq(@int_from_ptr(gs[0].font), @int_from_ptr(&latin));
    @expect_eq(@int_from_ptr(gs[3].font), @int_from_ptr(&cjk));
}
