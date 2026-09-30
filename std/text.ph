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
// fallbacks) that has a glyph for it (a combining mark stays with its
// base's font when it can). Runs of one font, style, direction and script
// are shaped with std/shape: ligatures, kerning, mark positioning, Arabic
// joining and Indic reordering as the fonts' OpenType features say (lines
// are measured shaped, then each line is shaped again as it is placed, so
// forms at line ends are right).
// =============================================================================

import "color.ph" as color;
import "draw.ph" as draw;
import "font.ph" as font;
import "mem.ph" as mem;
import "shape.ph" as shape;
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
    letter_spacing: f64,        // extra pixels after each character
    features: []shape.Feature,  // OpenType feature settings (std/shape), such as shape.feature("-liga")

    pub fn init(f: *font.Font, size: f64, c: color.Color) Style {
        return Style.{
            font: f,
            fallbacks: no_fallbacks(),
            size: size,
            color: c,
            underline: false,
            strike: false,
            letter_spacing: 0.0,
            features: shape.options().features,
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
    length: usize,  // the bytes of text it stands for (several for a ligature; 0 for the
                    // second and later glyphs of a cluster)
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
    run: usize,     // the shaping run it belongs to
    special: bool,  // a tab, newline or control character: not shaped or drawn
}

/// A run of cells shaped together: one span, font, bidi level and script.
struct Run {
    start: usize,  // cells [start, end)
    end: usize,
    level: u8,
    script: u32,
}

/// Font lookups remembered for one layout: the glyphs of ASCII characters
/// for the last style, and the shaping plans.
struct Memo {
    ascii_font: usize,  // the style's font and fallbacks the ASCII table is for
    ascii_fallbacks: usize,
    ascii_nfb: usize,
    ascii_gid: [128]u32,
    ascii_which: [128]u8,  // 0: the style's font; k + 1: fallback k; 255: not looked up
    shaper: shape.Shaper,

    fn pick_font(self: *mut Memo, st: *Style, cp: u32, gid: *mut u32) *font.Font {
        if cp >= 128 {
            return pick_font(st, cp, gid);
        }
        let ff = @int_from_ptr(st.font);
        let fb = @int_from_ptr(st.fallbacks.ptr);
        if (self.ascii_font != ff) or (self.ascii_fallbacks != fb) or (self.ascii_nfb != st.fallbacks.len) {
            self.ascii_font = ff;
            self.ascii_fallbacks = fb;
            self.ascii_nfb = st.fallbacks.len;
            @mem_set(self.ascii_which[..], 255);
        }
        let i = @as(usize, cp);
        let w = self.ascii_which[i];
        if w == 255 {
            let f = pick_font(st, cp, gid);
            self.ascii_gid[i] = gid.*;
            let mut which: u8 = 0;
            for k in @as(usize, 0)..st.fallbacks.len {
                if (@int_from_ptr(f) != ff) and (@int_from_ptr(f) == @int_from_ptr(st.fallbacks[k])) and (k < 254) {
                    which = @truncate(u8, k + 1);
                    break;
                }
            }
            self.ascii_which[i] = which;
            return f;
        }
        gid.* = self.ascii_gid[i];
        if w == 0 {
            return st.font;
        }
        return st.fallbacks[@as(usize, w) - 1];
    }
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

fn is_mark(cp: u32) bool {
    let gc = uni.category(cp);
    return (gc == uni.GC_MN) or (gc == uni.GC_MC) or (gc == uni.GC_ME);
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
    let memo = mem.create(Memo.{
        ascii_font: 0,
        ascii_fallbacks: 0,
        ascii_nfb: 0,
        ascii_gid: [0; 128],
        ascii_which: [255; 128],
        shaper: shape.Shaper.init(),
    })!;
    defer {
        memo.shaper.deinit();
        mem.destroy(memo);
    }
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
        y = paragraph(s, start, end, span_of.items(), spans, opts, y, out, memo)!;
        start = end;
    }
    out.height = y;
    if opts.width > 0.0 {
        out.width = @max(out.width, opts.width);
    }
}

/// Shapes cells [a, b) of run r (their text is s[cs[a].offset..]): the
/// glyphs, in visual order.
fn shape_cells(s: []u8, cs: []Cell, r: Run, a: usize, b: usize, spans: []Span, memo: *mut Memo) ![]mut shape.Glyph {
    let c0 = cs[a];
    let from = c0.offset;
    let to = cs[b - 1].offset + cs[b - 1].len;
    let mut o = shape.options();
    o.script = r.script;
    o.direction = shape.DIR_LTR;
    if (r.level & 1) == 1 {
        o.direction = shape.DIR_RTL;
    }
    o.features = spans[c0.span].style.features;
    return memo.shaper.shape(c0.font, s[from..to], o);
}

/// The cell of cells [a, b) at byte offset off (the last one at or before it).
fn cell_at(cs: []Cell, a: usize, b: usize, off: usize) usize {
    let mut lo = a;
    let mut hi = b;
    while (hi - lo) > 1 {
        let mid = (lo + hi) / 2;
        if cs[mid].offset <= off {
            lo = mid;
        } else {
            hi = mid;
        }
    }
    return lo;
}

/// Lays out the paragraph s[start..end] with its first line's top at y;
/// returns where the next paragraph starts.
fn paragraph(s: []u8, start: usize, end: usize, span_of: []usize, spans: []Span, opts: Options, y0: f64,
             out: *mut Layout, memo: *mut Memo) !f64 {
    // the code points and their fonts
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
        let mut f = memo.pick_font(st, r.value, &mut gid);
        // a mark stays with its base's font when that font has it
        if (cells.len > 0) and is_mark(r.value) {
            let prev = cells.get(cells.len - 1);
            if (prev.span == sp) and not prev.special and (@int_from_ptr(prev.font) != @int_from_ptr(f)) {
                let g = prev.font.glyph_index(r.value);
                if g != 0 {
                    f = prev.font;
                    gid = g;
                }
            }
        }
        let special = (r.value == '\t') or is_newline(r.value) or (uni.category(r.value) == uni.GC_CC);
        let mut adv: f64 = 0.0;
        if r.value == '\t' {
            adv = @float_from_int(f64, f.advance(gid)) * font.scale_for(f, st.size);
        }
        if special and (r.value != '\t') {
            gid = 0;
        }
        cells.push(Cell.{
            cp: r.value,
            offset: i,
            len: r.len,
            span: sp,
            gid: gid,
            font: f,
            advance: adv,
            run: 0,
            special: special,
        })!;
        cps.push(r.value)!;
        i += r.len;
    }
    let cs = cells.items();
    let n = cs.len;
    // bidi levels
    let mut bp = bidi.Paragraph.init(cps.items(), opts.direction)!;
    defer bp.deinit();
    let rtl = (bp.level & 1) == 1;
    // scripts: common and inherited characters take their neighbours'
    let mut scripts = vec.Vec.[u32].init();
    defer scripts.deinit();
    let mut last_script: u32 = 0;
    for k in @as(usize, 0)..n {
        let sc = shape.script_of(cs[k].cp);
        if (sc == shape.tag("Zyyy")) or (sc == shape.tag("Zinh")) or (sc == shape.tag("Zzzz")) {
            scripts.push(last_script)!;
        } else {
            scripts.push(sc)!;
            last_script = sc;
        }
    }
    let scs = scripts.items();
    // leading common characters take the first real script
    {
        let mut first: u32 = 0;
        for k in @as(usize, 0)..n {
            if scs[k] != 0 {
                first = scs[k];
                break;
            }
        }
        for k in @as(usize, 0)..n {
            if scs[k] != 0 {
                break;
            }
            scs[k] = first;
        }
    }
    // runs: one span, font, level and script, without special characters
    let mut runs = vec.Vec.[Run].init();
    defer runs.deinit();
    let mut k: usize = 0;
    while k < n {
        let mut e = k + 1;
        if not cs[k].special {
            while (e < n) and not cs[e].special and (cs[e].span == cs[k].span) and
                  (@int_from_ptr(cs[e].font) == @int_from_ptr(cs[k].font)) and (bp.levels[e] == bp.levels[k]) and
                  (scs[e] == scs[k]) {
                e += 1;
            }
        }
        for j in k..e {
            cs[j].run = runs.len;
        }
        runs.push(Run.{ start: k, end: e, level: bp.levels[k], script: scs[k] })!;
        k = e;
    }
    // measure: each cluster's advance goes to its first cell
    for r in runs.items() {
        if cs[r.start].special {
            continue;
        }
        let gs = shape_cells(s, cs, r, r.start, r.end, spans, memo)!;
        defer mem.free(gs);
        let f = cs[r.start].font;
        let scale = font.scale_for(f, spans[cs[r.start].span].style.size);
        for g in gs {
            let c = cell_at(cs, r.start, r.end, cs[r.start].offset + @as(usize, g.cluster));
            cs[c].advance += @float_from_int(f64, g.x_advance) * scale;
        }
    }
    for j in @as(usize, 0)..n {
        if not (is_newline(cs[j].cp) or (uni.category(cs[j].cp) == uni.GC_CC)) {
            cs[j].advance += spans[cs[j].span].style.letter_spacing;
        }
    }
    // tabs: to the next stop (measured from the line start when placed)
    let tab = opts.tab_stops;
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
        y = place_line(s, cs, runs.items(), &bp, line_start, line_end, (line_end == n), rtl, spans, opts, y, tab, out,
                       memo)!;
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

/// Where the line starting at cell `a` ends: the last break opportunity
/// that fits (or a mandatory one), else a grapheme boundary.
fn fit(cs: []Cell, a: usize, bs: []usize, bi: *mut usize, width: f64, tab: f64) !usize {
    // skip opportunities at or before the start
    while (bi.* < bs.len) and (bs[bi.*] <= a) {
        bi.* += 1;
    }
    let mut best: usize = 0;
    let mut k = bi.*;
    // the width of [a, b) without trailing white space, carried forward
    // break by break (recomputing it from a each time is quadratic)
    let mut x: f64 = 0.0;        // pen position after cells [a, done)
    let mut visible: f64 = 0.0;  // x after the last cell that is not trailing space
    let mut done = a;
    while k < bs.len {
        let b = bs[k];
        while done < b {
            let c = cs[done];
            if (c.cp == '\t') and (tab > 0.0) {
                x = (@floor(x / tab) + 1.0) * tab;
            } else {
                x += c.advance;
            }
            if not (trailing_space(c.cp) or is_newline(c.cp)) {
                visible = x;
            }
            done += 1;
        }
        if visible > width {
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
        // keep combining marks (and the rest of a cluster) with their base
        while (e < limit) and (((uni.width(cs[e].cp) == 0) and (cs[e].advance == 0.0)) or is_mark(cs[e].cp)) {
            e += 1;
        }
    }
    return e;
}

/// A glyph of a line being placed: positions relative to the pen.
struct Placed {
    id: u32,
    font: *font.Font,
    span: usize,
    offset: usize,
    length: usize,
    advance: f64,  // with letter spacing
    dx: f64,
    dy: f64,      // y down
    space: bool,  // a space justification may widen
    tab: bool,    // a tab: advances to the next stop
    draw: bool,
}

/// Shapes the line's cells [a, e) in visual order.
fn shape_line(s: []u8, cs: []Cell, runs: []Run, order: []usize, e: usize, spans: []Span, memo: *mut Memo,
              placed: *mut vec.Vec.[Placed]) !void {
    let mut i: usize = 0;
    while i < order.len {
        let k = order[i];
        if k >= e {
            i += 1;
            continue;  // trailing white space
        }
        let c = cs[k];
        if c.special {
            if c.cp == '\t' {
                placed.push(Placed.{
                    id: 0,
                    font: c.font,
                    span: c.span,
                    offset: c.offset,
                    length: c.len,
                    advance: c.advance,
                    dx: 0.0,
                    dy: 0.0,
                    space: false,
                    tab: true,
                    draw: false,
                })!;
            }
            i += 1;
            continue;
        }
        // the cells of this run that follow in visual order
        let mut lo = k;
        let mut hi = k;
        let mut j = i + 1;
        while (j < order.len) and (order[j] < e) and not cs[order[j]].special and (cs[order[j]].run == c.run) {
            lo = @min(lo, order[j]);
            hi = @max(hi, order[j]);
            j += 1;
        }
        let r = runs[c.run];
        let gs = shape_cells(s, cs, r, lo, hi + 1, spans, memo)!;
        defer mem.free(gs);
        let st = spans[c.span].style;
        let scale = font.scale_for(c.font, st.size);
        let base = cs[lo].offset;
        for g in gs {
            let cell = cell_at(cs, lo, hi + 1, base + @as(usize, g.cluster));
            placed.push(Placed.{
                id: g.id,
                font: c.font,
                span: c.span,
                offset: base + @as(usize, g.cluster),
                length: 0,
                advance: @float_from_int(f64, g.x_advance) * scale,
                dx: @float_from_int(f64, g.x_offset) * scale,
                dy: -@float_from_int(f64, g.y_offset) * scale,
                space: (cs[cell].cp == ' ') or (cs[cell].cp == 0x3000),
                tab: false,
                draw: g.id != 0,
            })!;
        }
        // the text each cluster stands for (from its start to the next
        // cluster's), given to its first glyph
        {
            let ps = placed.items();
            let first = ps.len - gs.len;
            let seg_end = cs[hi].offset + cs[hi].len;
            for m in first..ps.len {
                let mut seen = false;
                for q in first..m {
                    if ps[q].offset == ps[m].offset {
                        seen = true;
                    }
                }
                if seen {
                    continue;
                }
                let mut next = seg_end;
                for q in first..ps.len {
                    if (ps[q].offset > ps[m].offset) and (ps[q].offset < next) {
                        next = ps[q].offset;
                    }
                }
                ps[m].length = next - ps[m].offset;
            }
        }
        // letter spacing: after each cluster, per character in it
        if st.letter_spacing != 0.0 {
            let ps = placed.items();
            let first = ps.len - gs.len;
            for m in first..ps.len {
                let mut last_of_cluster = true;
                if (m + 1) < ps.len {
                    last_of_cluster = ps[m + 1].offset != ps[m].offset;
                }
                if last_of_cluster {
                    // the characters from this cluster's start to the next larger cluster start
                    let mut next = cs[hi].offset + cs[hi].len;
                    for q in first..ps.len {
                        if (ps[q].offset > ps[m].offset) and (ps[q].offset < next) {
                            next = ps[q].offset;
                        }
                    }
                    let mut count: usize = 0;
                    for q in lo..(hi + 1) {
                        if (cs[q].offset >= ps[m].offset) and (cs[q].offset < next) {
                            count += 1;
                        }
                    }
                    ps[m].advance += st.letter_spacing * @float_from_int(f64, count);
                }
            }
        }
        i = j;
    }
}

/// The width of placed glyphs with tabs and extra space per space glyph.
fn placed_width(ps: []Placed, tab: f64, extra: f64) f64 {
    let mut x: f64 = 0.0;
    for p in ps {
        if p.tab and (tab > 0.0) {
            x = (@floor(x / tab) + 1.0) * tab;
        } else {
            x += p.advance;
        }
        if p.space {
            x += extra;
        }
    }
    return x;
}

/// Places cells [a, b) as a line whose top is at y; returns the next line's top.
fn place_line(s: []u8, cs: []Cell, runs: []Run, bp: *bidi.Paragraph, a: usize, b: usize, last: bool, rtl: bool,
              spans: []Span, opts: Options, y: f64, tab: f64, out: *mut Layout, memo: *mut Memo) !f64 {
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
    // the line's glyphs, shaped, in visual order
    let order = bp.visual_order(a, b)!;
    defer mem.free(order);
    let mut placed = vec.Vec.[Placed].init();
    defer placed.deinit();
    shape_line(s, cs, runs, order, e, spans, memo, &mut placed)!;
    let ps = placed.items();
    let lw = placed_width(ps, tab, 0.0);
    // justification: extra room shared among the spaces
    let mut extra: f64 = 0.0;
    let ends_para = last or ((b > a) and is_newline(cs[b - 1].cp));
    if (opts.align == JUSTIFY) and not ends_para and (opts.width > lw) {
        let mut spaces: usize = 0;
        for p in ps {
            if p.space {
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
    for p in ps {
        let st = spans[p.span].style;
        let mut adv = p.advance;
        if p.tab and (tab > 0.0) {
            adv = ((@floor((x - left) / tab) + 1.0) * tab) - (x - left);
        }
        if p.space {
            adv += extra;
        }
        if p.draw {
            out.glyphs.push(Glyph.{
                id: p.id,
                font: p.font,
                x: x + p.dx,
                y: baseline + p.dy,
                size: st.size,
                span: p.span,
                offset: p.offset,
                length: p.length,
                advance: adv,
            })!;
        }
        x += adv;
    }
    let mut le = cs[a].offset;
    if b > a {
        le = cs[b - 1].offset + cs[b - 1].len;
    }
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

test "shaped text" {
    let mut f = try_font("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf") catch return;
    defer f.deinit();
    let st = Style.init(&f, 20.0, color.Color.rgb(0.0, 0.0, 0.0));
    // ligatures: "ffi" is one glyph, placed as std/shape places it
    let sp: [1]Span = [Span.{ text: "office", style: st }];
    let mut l = layout(sp[..], Options.init(0.0))!;
    defer l.deinit();
    let gs = l.glyphs.items();
    @expect_eq(gs.len, 4);
    let sh = shape.shape(&f, "office", shape.options())!;
    defer mem.free(sh);
    let scale = font.scale_for(&f, 20.0);
    let mut pen: f64 = 0.0;
    for k in @as(usize, 0)..4 {
        @expect_eq(gs[k].id, sh[k].id);
        @expect(@abs(gs[k].x - pen) < 0.0001);
        pen += @float_from_int(f64, sh[k].x_advance) * scale;
    }
    // features: no ligatures
    let mut plain = st;
    let noliga: [1]shape.Feature = [shape.feature("-liga")!];
    plain.features = noliga[..];
    let ps: [1]Span = [Span.{ text: "office", style: plain }];
    let mut p = layout(ps[..], Options.init(0.0))!;
    defer p.deinit();
    @expect_eq(p.glyphs.len, 6);
    // letter spacing goes after each character, ligated or not
    let mut spaced = st;
    spaced.letter_spacing = 2.0;
    let ss: [1]Span = [Span.{ text: "office", style: spaced }];
    let mut sl = layout(ss[..], Options.init(0.0))!;
    defer sl.deinit();
    @expect(@abs((sl.lines.get(0).right - l.lines.get(0).right) - 12.0) < 0.0001);
    // Arabic joins, right to left: lam alef is one glyph
    let ar: [1]Span = [Span.{ text: "\u{633}\u{644}\u{627}\u{645}", style: st }];
    let mut a = layout(ar[..], Options.init(0.0))!;
    defer a.deinit();
    @expect(a.lines.get(0).rtl);
    let ags = a.glyphs.items();
    @expect_eq(ags.len, 3);
    @expect_eq(ags[0].offset, 6);                // meem, at the left
    @expect_eq(ags[2].offset, 0);                // seen, at the right
    @expect(ags[2].id != f.glyph_index(0x633));  // its initial form
    // a line break between Arabic words reshapes each line
    let words: [1]Span = [Span.{ text: "\u{628}\u{628} \u{628}\u{628}", style: st }];
    let whole = measure(words[..], Options.init(0.0))!;
    let mut w = layout(words[..], Options.init(whole[0] * 0.7))!;
    defer w.deinit();
    @expect_eq(w.lines.len, 2);
    // each line ends in a final form (as its own word would): the same
    // glyphs as the first word shaped alone
    let one = shape.shape(&f, "\u{628}\u{628}", shape.options())!;
    defer mem.free(one);
    let wg = w.glyphs.items();
    @expect_eq(wg.len, 4);
    @expect_eq(wg[0].id, one[0].id);
    @expect_eq(wg[1].id, one[1].id);
    @expect_eq(wg[2].id, one[0].id);
}

test "Indic reordering" {
    let mut f = try_font("/usr/share/fonts/truetype/freefont/FreeSerif.ttf") catch return;
    defer f.deinit();
    let st = Style.init(&f, 20.0, color.Color.rgb(0.0, 0.0, 0.0));
    // "ki": the vowel sign i is drawn before (left of) ka
    let sp: [1]Span = [Span.{ text: "\u{915}\u{93f}", style: st }];
    let mut l = layout(sp[..], Options.init(0.0))!;
    defer l.deinit();
    let gs = l.glyphs.items();
    @expect_eq(gs.len, 2);
    @expect(gs[0].id == f.glyph_index(0x93f));
    @expect(gs[1].id == f.glyph_index(0x915));
    @expect(gs[0].x < gs[1].x);
}
