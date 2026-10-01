// =============================================================================
// std/graphics/svg/path.ph - SVG path data (SVG 1.1 section 8.3): parsing into a
// draw.Path, elliptical arcs as cubic Béziers (appendix F.6), formatting a
// draw.Path as path data; numbers as SVG writes them; tight bounding boxes
// and transformed copies of paths. Part of std/graphics/svg.
//
//     let mut p = draw.Path.init();
//     path.parse("M10 10h80v80h-80z M30 50 a20 20 0 1 0 40 0", &mut p)!;
//     path.write(w, &p)!;                 // "M10 10L90 10L90 90L10 90Z..."
// =============================================================================

import "../../fmt.ph" as fmt;
import "../../io.ph" as io;
import "../../math.ph" as math;
import "../draw.ph" as draw;

error { InvalidPathData, OutOfMemory }

// -----------------------------------------------------------------------------
// Numbers
// -----------------------------------------------------------------------------

/// SVG's white space: space, tab, line feed, carriage return, form feed.
pub fn is_space(c: u8) bool {
    return (c == ' ') or (c == '\t') or (c == '\n') or (c == '\r') or (c == 0x0c);
}

/// Reads text as SVG's number lists, flags and keywords are read: a position
/// in it, skipping white space and commas between items.
pub struct Scanner {
    s: []u8,
    i: usize,

    /// At the start of s.
    pub fn init(s: []u8) Scanner {
        return Scanner.{ s: s, i: 0 };
    }

    /// Skips white space.
    pub fn skip_space(self: *mut Scanner) void {
        while (self.i < self.s.len) and is_space(self.s[self.i]) {
            self.i += 1;
        }
    }

    /// Skips white space with at most one comma in it.
    pub fn skip_separator(self: *mut Scanner) void {
        self.skip_space();
        if (self.i < self.s.len) and (self.s[self.i] == ',') {
            self.i += 1;
            self.skip_space();
        }
    }

    /// Whether only white space is left.
    pub fn at_end(self: *mut Scanner) bool {
        self.skip_space();
        return self.i >= self.s.len;
    }

    /// The next byte (0 at the end).
    pub fn peek(self: *Scanner) u8 {
        if self.i < self.s.len {
            return self.s[self.i];
        }
        return 0;
    }

    /// Whether a number starts here (after white space).
    pub fn number_next(self: *mut Scanner) bool {
        self.skip_space();
        let c = self.peek();
        return ((c >= '0') and (c <= '9')) or (c == '.') or (c == '-') or (c == '+');
    }

    /// A number: [+-] digits [. digits] [e [+-] digits]; "1.5.5" is 1.5 then
    /// .5, and "1-2" is 1 then -2. Leaves the scanner after it (and the
    /// separator that follows).
    pub fn number(self: *mut Scanner) !f64 {
        self.skip_space();
        let v = self.raw_number()!;
        self.skip_separator();
        return v;
    }

    /// A number with nothing skipped after it (for a unit that may follow).
    pub fn raw_number(self: *mut Scanner) !f64 {
        self.skip_space();
        let s = self.s;
        let start = self.i;
        let mut i = self.i;
        if (i < s.len) and ((s[i] == '+') or (s[i] == '-')) {
            i += 1;
        }
        let mut digits: usize = 0;
        while (i < s.len) and (s[i] >= '0') and (s[i] <= '9') {
            i += 1;
            digits += 1;
        }
        if (i < s.len) and (s[i] == '.') {
            i += 1;
            while (i < s.len) and (s[i] >= '0') and (s[i] <= '9') {
                i += 1;
                digits += 1;
            }
        }
        if digits == 0 {
            return error.InvalidPathData;
        }
        // an exponent, unless the "e" begins a unit (em, ex)
        if (i < s.len) and ((s[i] == 'e') or (s[i] == 'E')) {
            let mut j = i + 1;
            if (j < s.len) and ((s[j] == '+') or (s[j] == '-')) {
                j += 1;
            }
            if (j < s.len) and (s[j] >= '0') and (s[j] <= '9') {
                while (j < s.len) and (s[j] >= '0') and (s[j] <= '9') {
                    j += 1;
                }
                i = j;
            }
        }
        let v = fmt.parse_float(s[start..i]) catch return error.InvalidPathData;
        if not math.is_finite(v) {
            return error.InvalidPathData;
        }
        self.i = i;
        return v;
    }

    /// An arc flag: "0" or "1" (which need no separator after them).
    pub fn flag(self: *mut Scanner) !bool {
        self.skip_space();
        let c = self.peek();
        if (c != '0') and (c != '1') {
            return error.InvalidPathData;
        }
        self.i += 1;
        self.skip_separator();
        return c == '1';
    }
}

/// Writes a number briefly: rounded to 6 decimals, shortest form, no
/// exponent for ordinary magnitudes ("12", "0.5", "-3.25").
pub fn write_number(w: io.Writer, v: f64) !void {
    let mut buf: [40]u8 = [0; 40];
    w.write_all(format_number(buf[..], v))!;
}

/// The text write_number writes, in buf (40 bytes suffice).
pub fn format_number(buf: []mut u8, v: f64) []u8 {
    if not math.is_finite(v) {
        buf[0] = '0';
        return buf[..1];
    }
    let mut r = v;
    if @abs(v) < 1.0e15 {
        r = @nearest(v * 1.0e6) / 1.0e6;
    }
    if r == 0.0 {
        r = 0.0;  // no "-0"
        buf[0] = '0';
        return buf[..1];
    }
    let s = fmt.format_float(buf[..], r) catch {
        buf[0] = '0';
        return buf[..1];
    };
    // fmt writes 1e-05 and the like for small values: those are below the
    // rounding, except for magnitudes like 1e-6 themselves
    return s;
}

// -----------------------------------------------------------------------------
// Parsing
// -----------------------------------------------------------------------------

fn is_command(c: u8) bool {
    for k in "MmLlHhVvCcSsQqTtAaZz" {
        if k == c {
            return true;
        }
    }
    return false;
}

struct Pen {
    p: *mut draw.Path,
    x: f64,  // the current point
    y: f64,
    sx: f64,  // the subpath's start
    sy: f64,
    cx: f64,  // the last control point (for S and T)
    cy: f64,
    last: u8,      // the last command, upper case
    closed: bool,  // Z was the last command: the next drawing starts at (sx, sy)

    fn begin(self: *mut Pen) !void {
        if self.closed {
            self.p.move_to(self.sx, self.sy)!;
            self.closed = false;
        }
    }
}

/// Parses path data, appending its subpaths to p. On an error the path holds
/// what came before it (as SVG renders a path "up to the error"), and the
/// error is InvalidPathData.
pub fn parse(d: []u8, p: *mut draw.Path) !void {
    let mut sc = Scanner.init(d);
    let mut pen = Pen.{ p: p, x: 0.0, y: 0.0, sx: 0.0, sy: 0.0, cx: 0.0, cy: 0.0, last: 0, closed: false };
    let mut cmd: u8 = 0;
    loop {
        if sc.at_end() {
            return;
        }
        let c = sc.peek();
        if is_command(c) {
            cmd = c;
            sc.i += 1;
            if (pen.last == 0) and (cmd != 'M') and (cmd != 'm') {
                return error.InvalidPathData;
            }
        } else if (cmd == 0) or (cmd == 'Z') or (cmd == 'z') {
            return error.InvalidPathData;
        } else if cmd == 'M' {
            cmd = 'L';  // more pairs after a moveto are linetos
        } else if cmd == 'm' {
            cmd = 'l';
        }
        segment(&mut sc, &mut pen, cmd)!;
    }
}

fn segment(sc: *mut Scanner, pen: *mut Pen, cmd: u8) !void {
    let rel = (cmd >= 'a') and (cmd <= 'z');
    let mut ox: f64 = 0.0;
    let mut oy: f64 = 0.0;
    if rel {
        ox = pen.x;
        oy = pen.y;
    }
    let up = cmd & 0xdf;
    // a control point to reflect for S and T, if the last command had one
    let mut rx = pen.x;
    let mut ry = pen.y;
    if ((up == 'S') and ((pen.last == 'C') or (pen.last == 'S'))) or
       ((up == 'T') and ((pen.last == 'Q') or (pen.last == 'T'))) {
        rx = (2.0 * pen.x) - pen.cx;
        ry = (2.0 * pen.y) - pen.cy;
    }
    if up == 'M' {
        let x = sc.number()! + ox;
        let y = sc.number()! + oy;
        pen.p.move_to(x, y)!;
        pen.x = x;
        pen.y = y;
        pen.sx = x;
        pen.sy = y;
        pen.closed = false;
    } else if up == 'Z' {
        pen.p.close()!;
        pen.x = pen.sx;
        pen.y = pen.sy;
        pen.closed = true;
    } else if up == 'L' {
        let x = sc.number()! + ox;
        let y = sc.number()! + oy;
        pen.begin()!;
        pen.p.line_to(x, y)!;
        pen.x = x;
        pen.y = y;
    } else if up == 'H' {
        let x = sc.number()! + ox;
        pen.begin()!;
        pen.p.line_to(x, pen.y)!;
        pen.x = x;
    } else if up == 'V' {
        let y = sc.number()! + oy;
        pen.begin()!;
        pen.p.line_to(pen.x, y)!;
        pen.y = y;
    } else if (up == 'C') or (up == 'S') {
        let mut c1x = rx;
        let mut c1y = ry;
        if up == 'C' {
            c1x = sc.number()! + ox;
            c1y = sc.number()! + oy;
        }
        let c2x = sc.number()! + ox;
        let c2y = sc.number()! + oy;
        let x = sc.number()! + ox;
        let y = sc.number()! + oy;
        pen.begin()!;
        pen.p.cubic_to(c1x, c1y, c2x, c2y, x, y)!;
        pen.cx = c2x;
        pen.cy = c2y;
        pen.x = x;
        pen.y = y;
    } else if (up == 'Q') or (up == 'T') {
        let mut qx = rx;
        let mut qy = ry;
        if up == 'Q' {
            qx = sc.number()! + ox;
            qy = sc.number()! + oy;
        }
        let x = sc.number()! + ox;
        let y = sc.number()! + oy;
        pen.begin()!;
        pen.p.quad_to(qx, qy, x, y)!;
        pen.cx = qx;
        pen.cy = qy;
        pen.x = x;
        pen.y = y;
    } else if up == 'A' {
        let arx = sc.number()!;
        let ary = sc.number()!;
        let rot = sc.number()!;
        let large = sc.flag()!;
        let sweep = sc.flag()!;
        let x = sc.number()! + ox;
        let y = sc.number()! + oy;
        pen.begin()!;
        arc_to(pen.p, pen.x, pen.y, arx, ary, rot, large, sweep, x, y)!;
        pen.x = x;
        pen.y = y;
    } else {
        return error.InvalidPathData;
    }
    pen.last = up;
}

/// Appends the elliptical arc from (x1, y1) (the current point) to (x2, y2)
/// with radii rx and ry, the ellipse's x axis rotated by `rotation` degrees,
/// as SVG's arc command draws it: cubic Béziers of at most a quarter turn
/// each. Radii too small for the endpoints are scaled up; a zero radius
/// makes a line.
pub fn arc_to(p: *mut draw.Path, x1: f64, y1: f64, rx0: f64, ry0: f64, rotation: f64, large: bool, sweep: bool, x2: f64,
              y2: f64) !void {
    if (x1 == x2) and (y1 == y2) {
        return;
    }
    let mut rx = @abs(rx0);
    let mut ry = @abs(ry0);
    if not (rx > 0.0) or not (ry > 0.0) {
        return p.line_to(x2, y2);
    }
    let sc = math.sincos(@rem(rotation, 360.0) * (math.PI / 180.0));
    let cphi = sc.cos;
    let sphi = sc.sin;
    // F.6.5: the centre
    let dx2 = (x1 - x2) / 2.0;
    let dy2 = (y1 - y2) / 2.0;
    let x1p = (cphi * dx2) + (sphi * dy2);
    let y1p = (cphi * dy2) - (sphi * dx2);
    // F.6.6: radii too small are scaled up
    let lambda = ((x1p * x1p) / (rx * rx)) + ((y1p * y1p) / (ry * ry));
    if lambda > 1.0 {
        let k = @sqrt(lambda);
        rx = rx * k;
        ry = ry * k;
    }
    let rx2 = rx * rx;
    let ry2 = ry * ry;
    let num = ((rx2 * ry2) - (rx2 * (y1p * y1p))) - (ry2 * (x1p * x1p));
    let den = (rx2 * (y1p * y1p)) + (ry2 * (x1p * x1p));
    let mut coef: f64 = 0.0;
    if den > 0.0 {
        coef = @sqrt(@max(num, 0.0) / den);
    }
    if large == sweep {
        coef = -coef;
    }
    let cxp = (coef * (rx * y1p)) / ry;
    let cyp = (coef * (-ry * x1p)) / rx;
    let cx = ((cphi * cxp) - (sphi * cyp)) + ((x1 + x2) / 2.0);
    let cy = ((sphi * cxp) + (cphi * cyp)) + ((y1 + y2) / 2.0);
    let ux = (x1p - cxp) / rx;
    let uy = (y1p - cyp) / ry;
    let vx = (-x1p - cxp) / rx;
    let vy = (-y1p - cyp) / ry;
    let theta1 = math.atan2(uy, ux);
    let mut dtheta = math.atan2((ux * vy) - (uy * vx), (ux * vx) + (uy * vy));
    if not sweep and (dtheta > 0.0) {
        dtheta -= math.TAU;
    } else if sweep and (dtheta < 0.0) {
        dtheta += math.TAU;
    }
    if not math.is_finite(dtheta) or not math.is_finite(cx) or not math.is_finite(cy) {
        return p.line_to(x2, y2);
    }
    let n = @max(@ceil((@abs(dtheta) / (math.PI / 2.0)) - 1.0e-9), 1.0);
    let step = dtheta / n;
    let k = (4.0 / 3.0) * math.tan(step / 4.0);
    let count = @int_from_float(usize, n);
    let mut a = theta1;
    for i in @as(usize, 0)..count {
        let s0 = math.sincos(a);
        let s1 = math.sincos(a + step);
        // on the unit circle, then scaled, rotated and moved
        let p1x = s0.cos - (k * s0.sin);
        let p1y = s0.sin + (k * s0.cos);
        let p2x = s1.cos + (k * s1.sin);
        let p2y = s1.sin - (k * s1.cos);
        let mut ex = cx + ((cphi * (rx * s1.cos)) - (sphi * (ry * s1.sin)));
        let mut ey = cy + ((sphi * (rx * s1.cos)) + (cphi * (ry * s1.sin)));
        if (i + 1) == count {
            ex = x2;
            ey = y2;
        }
        p.cubic_to(cx + ((cphi * (rx * p1x)) - (sphi * (ry * p1y))), cy + ((sphi * (rx * p1x)) + (cphi * (ry * p1y))),
                   cx + ((cphi * (rx * p2x)) - (sphi * (ry * p2y))), cy + ((sphi * (rx * p2x)) + (cphi * (ry * p2y))),
                   ex, ey)!;
        a = a + step;
    }
}

// -----------------------------------------------------------------------------
// Formatting
// -----------------------------------------------------------------------------

/// Writes a path as path data with absolute commands (M, L, Q, C, Z).
pub fn write(w: io.Writer, p: *draw.Path) !void {
    let verbs = p.verbs.items();
    let pts = p.pts.items();
    let mut k: usize = 0;
    for v in verbs {
        let mut n: usize = 0;
        match v {
            .MOVE => {
                w.write_all("M")!;
                n = 2;
            }
            .LINE => {
                w.write_all("L")!;
                n = 2;
            }
            .QUAD => {
                w.write_all("Q")!;
                n = 4;
            }
            .CUBIC => {
                w.write_all("C")!;
                n = 6;
            }
            .CLOSE => { w.write_all("Z")!; }
        }
        for j in @as(usize, 0)..n {
            if j > 0 {
                w.write_all(" ")!;
            }
            write_number(w, pts[k + j])!;
        }
        k += n;
    }
}

// -----------------------------------------------------------------------------
// Geometry
// -----------------------------------------------------------------------------

/// A box: (x0, y0) to (x1, y1); empty if x0 > x1.
pub struct Box {
    x0: f64,
    y0: f64,
    x1: f64,
    y1: f64,

    /// A box with nothing in it.
    pub fn empty() Box {
        return Box.{ x0: 1.0e300, y0: 1.0e300, x1: -1.0e300, y1: -1.0e300 };
    }

    /// Whether nothing has been added.
    pub fn is_empty(self: Box) bool {
        return not (self.x0 <= self.x1) or not (self.y0 <= self.y1);
    }

    /// Grows the box to hold (x, y).
    pub fn add(self: *mut Box, x: f64, y: f64) void {
        self.x0 = @min(self.x0, x);
        self.y0 = @min(self.y0, y);
        self.x1 = @max(self.x1, x);
        self.y1 = @max(self.y1, y);
    }
}

/// The value at t of the cubic (or quadratic, with d = c = b...) coordinate polynomial.
fn cubic_at(a: f64, b: f64, c: f64, d: f64, t: f64) f64 {
    let u = 1.0 - t;
    return ((((u * u) * u) * a) + (((3.0 * (u * u)) * t) * b)) + ((((3.0 * u) * (t * t)) * c) + (((t * t) * t) * d));
}

/// Adds a cubic coordinate's extremes in (0, 1) to xs (at most 2), returning how many.
fn cubic_extrema(a: f64, b: f64, c: f64, d: f64, ts: []mut f64) usize {
    // derivative / 3: (b - a)(1-t)^2 + 2(c - b)(1-t)t + (d - c)t^2 = A t^2 + B t + C
    let qa = ((d - (3.0 * c)) + (3.0 * b)) - a;
    let qb = 2.0 * ((a - (2.0 * b)) + c);
    let qc = b - a;
    let mut n: usize = 0;
    if @abs(qa) < 1.0e-12 {
        if @abs(qb) > 1.0e-12 {
            let t = -qc / qb;
            if (t > 0.0) and (t < 1.0) {
                ts[n] = t;
                n += 1;
            }
        }
        return n;
    }
    let disc = (qb * qb) - ((4.0 * qa) * qc);
    if disc < 0.0 {
        return 0;
    }
    let sq = @sqrt(disc);
    let t1 = (-qb + sq) / (2.0 * qa);
    let t2 = (-qb - sq) / (2.0 * qa);
    if (t1 > 0.0) and (t1 < 1.0) {
        ts[n] = t1;
        n += 1;
    }
    if (t2 > 0.0) and (t2 < 1.0) {
        ts[n] = t2;
        n += 1;
    }
    return n;
}

/// The tight bounding box of a path (curves by their extremes, not their
/// control points).
pub fn bounds(p: *draw.Path) Box {
    let mut b = Box.empty();
    let verbs = p.verbs.items();
    let pts = p.pts.items();
    let mut k: usize = 0;
    let mut cx: f64 = 0.0;
    let mut cy: f64 = 0.0;
    let mut ts: [2]f64 = [0.0; 2];
    for v in verbs {
        match v {
            .MOVE => {
                cx = pts[k];
                cy = pts[k + 1];
                b.add(cx, cy);
                k += 2;
            }
            .LINE => {
                cx = pts[k];
                cy = pts[k + 1];
                b.add(cx, cy);
                k += 2;
            }
            .QUAD => {
                // as the cubic it equals
                let qx = pts[k];
                let qy = pts[k + 1];
                let ex = pts[k + 2];
                let ey = pts[k + 3];
                let c1x = cx + ((2.0 / 3.0) * (qx - cx));
                let c1y = cy + ((2.0 / 3.0) * (qy - cy));
                let c2x = ex + ((2.0 / 3.0) * (qx - ex));
                let c2y = ey + ((2.0 / 3.0) * (qy - ey));
                add_cubic(&mut b, cx, cy, c1x, c1y, c2x, c2y, ex, ey, ts[..]);
                cx = ex;
                cy = ey;
                k += 4;
            }
            .CUBIC => {
                add_cubic(&mut b, cx, cy, pts[k], pts[k + 1], pts[k + 2], pts[k + 3], pts[k + 4], pts[k + 5], ts[..]);
                cx = pts[k + 4];
                cy = pts[k + 5];
                k += 6;
            }
            .CLOSE => {}
        }
    }
    return b;
}

fn add_cubic(b: *mut Box, x0: f64, y0: f64, x1: f64, y1: f64, x2: f64, y2: f64, x3: f64, y3: f64, ts: []mut f64) void {
    b.add(x3, y3);
    let nx = cubic_extrema(x0, x1, x2, x3, ts);
    for i in @as(usize, 0)..nx {
        b.add(cubic_at(x0, x1, x2, x3, ts[i]), cubic_at(y0, y1, y2, y3, ts[i]));
    }
    let ny = cubic_extrema(y0, y1, y2, y3, ts);
    for i in @as(usize, 0)..ny {
        b.add(cubic_at(x0, x1, x2, x3, ts[i]), cubic_at(y0, y1, y2, y3, ts[i]));
    }
}

/// Appends src mapped through t to dst.
pub fn append_transformed(dst: *mut draw.Path, src: *draw.Path, t: draw.Affine) !void {
    let pts = src.pts.items();
    let mut k: usize = 0;
    for v in src.verbs.items() {
        match v {
            .MOVE => {
                let q = t.apply(pts[k], pts[k + 1]);
                dst.move_to(q.x, q.y)!;
                k += 2;
            }
            .LINE => {
                let q = t.apply(pts[k], pts[k + 1]);
                dst.line_to(q.x, q.y)!;
                k += 2;
            }
            .QUAD => {
                let c = t.apply(pts[k], pts[k + 1]);
                let q = t.apply(pts[k + 2], pts[k + 3]);
                dst.quad_to(c.x, c.y, q.x, q.y)!;
                k += 4;
            }
            .CUBIC => {
                let c1 = t.apply(pts[k], pts[k + 1]);
                let c2 = t.apply(pts[k + 2], pts[k + 3]);
                let q = t.apply(pts[k + 4], pts[k + 5]);
                dst.cubic_to(c1.x, c1.y, c2.x, c2.y, q.x, q.y)!;
                k += 6;
            }
            .CLOSE => { dst.close()!; }
        }
    }
}

// -----------------------------------------------------------------------------
// Tests (and std/graphics/svg's)
// -----------------------------------------------------------------------------

test "numbers" {
    let mut buf: [40]u8 = [0; 40];
    @expect_eq(format_number(buf[..], 0.1 + 0.2), "0.3");
    @expect_eq(format_number(buf[..], -0.0000001), "0");
    @expect_eq(format_number(buf[..], 1234.5678901), "1234.56789");
    @expect_eq(format_number(buf[..], -12.0), "-12");
    let mut sc = Scanner.init(" 1.5.5-2e1 1e 3,");
    @expect_eq(sc.number()!, 1.5);
    @expect_eq(sc.number()!, 0.5);
    @expect_eq(sc.number()!, -20.0);
    @expect_eq(sc.raw_number()!, 1.0);  // "1e" with no digits: 1, then a unit "e"
    @expect_eq(sc.peek(), 'e');
}

test "bounds" {
    let mut p = draw.Path.init();
    defer p.deinit();
    parse("M0 0 C0 10 10 10 10 0 Q5 -10 0 0", &mut p)!;
    let b = bounds(&p);
    @expect_eq(b.x0, 0.0);
    @expect_eq(b.x1, 10.0);
    @expect(@abs(b.y1 - 7.5) < 1.0e-12);
    @expect(@abs(b.y0 + 5.0) < 1.0e-12);
    let mut q = draw.Path.init();
    defer q.deinit();
    append_transformed(&mut q, &p, draw.Affine.translation(1.0, 2.0))!;
    let c = bounds(&q);
    @expect_eq(c.x0, 1.0);
    @expect(@abs(c.y1 - 9.5) < 1.0e-12);
    @expect(Box.empty().is_empty());
}
