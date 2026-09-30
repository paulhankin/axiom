// =============================================================================
// std/graphics/draw.ph - 2D vector drawing with anti-aliasing, onto a std/graphics/image Image.
//
//     let mut img = image.Image.init(400, 300, 4, image.Depth.U8)!;
//     let mut cv = draw.Canvas.init(&mut img);
//     defer cv.deinit();
//     cv.clear(color.Color.rgb(1.0, 1.0, 1.0));
//     let mut p = draw.Path.init();
//     defer p.deinit();
//     p.move_to(20.0, 20.0)!;
//     p.cubic_to(100.0, 0.0, 200.0, 150.0, 380.0, 60.0)!;
//     cv.stroke(&p, draw.Paint.solid(red), draw.Stroke.width(4.0))!;
//     p.clear();
//     p.circle(200.0, 180.0, 80.0)!;
//     cv.fill(&p, draw.Paint.linear(0.0, 100.0, 0.0, 260.0, stops[..]), draw.FillRule.NONZERO)!;
//
// Paths are lines, quadratic and cubic Béziers and arcs, in user space; the
// canvas transform (translate, scale, rotate, or any affine matrix) maps
// them to pixels. Fills use the nonzero or even-odd rule; strokes have a
// width, joins (miter, round, bevel), caps (butt, round, square) and dashes.
// Paints are solid colours, linear and radial gradients, and images.
//
// Coverage is exact: each pixel gets the area of it the shape covers, so
// edges are anti-aliased without supersampling. Colours are composited in
// linear light (as std/graphics/color's Color is), with Porter-Duff operators and
// blend modes, global alpha, and clipping to paths.
// =============================================================================

import "../collections/vec.ph" as vec;
import "../math.ph" as math;
import "../mem.ph" as mem;
import "../sync.ph" as sync;
import "color.ph" as color;
import "image.ph" as image;

error { OutOfMemory, InvalidPath }

// -----------------------------------------------------------------------------
// Geometry
// -----------------------------------------------------------------------------

pub struct Point {
    x: f64,
    y: f64,
}

/// An affine transform: x' = a x + c y + e, y' = b x + d y + f (the matrix
/// of SVG and HTML canvas).
pub struct Affine {
    a: f64,
    b: f64,
    c: f64,
    d: f64,
    e: f64,
    f: f64,

    pub fn identity() Affine {
        return Affine.{ a: 1.0, b: 0.0, c: 0.0, d: 1.0, e: 0.0, f: 0.0 };
    }

    pub fn translation(tx: f64, ty: f64) Affine {
        return Affine.{ a: 1.0, b: 0.0, c: 0.0, d: 1.0, e: tx, f: ty };
    }

    pub fn scaling(sx: f64, sy: f64) Affine {
        return Affine.{ a: sx, b: 0.0, c: 0.0, d: sy, e: 0.0, f: 0.0 };
    }

    /// A rotation by `radians` (clockwise on screen, where y points down).
    pub fn rotation(radians: f64) Affine {
        let sc = math.sincos(radians);
        return Affine.{ a: sc.cos, b: sc.sin, c: -sc.sin, d: sc.cos, e: 0.0, f: 0.0 };
    }

    /// self, then other.
    pub fn then(self: Affine, o: Affine) Affine {
        return Affine.{
            a: (o.a * self.a) + (o.c * self.b),
            b: (o.b * self.a) + (o.d * self.b),
            c: (o.a * self.c) + (o.c * self.d),
            d: (o.b * self.c) + (o.d * self.d),
            e: ((o.a * self.e) + (o.c * self.f)) + o.e,
            f: ((o.b * self.e) + (o.d * self.f)) + o.f,
        };
    }

    pub fn apply(self: Affine, x: f64, y: f64) Point {
        return Point.{ x: ((self.a * x) + (self.c * y)) + self.e, y: ((self.b * x) + (self.d * y)) + self.f };
    }

    pub fn determinant(self: Affine) f64 {
        return (self.a * self.d) - (self.b * self.c);
    }

    /// The inverse (the identity if this one is singular).
    pub fn invert(self: Affine) Affine {
        let det = self.determinant();
        if @abs(det) < 1.0e-12 {
            return Affine.identity();
        }
        let id = 1.0 / det;
        return Affine.{
            a: self.d * id,
            b: -self.b * id,
            c: -self.c * id,
            d: self.a * id,
            e: ((self.c * self.f) - (self.d * self.e)) * id,
            f: ((self.b * self.e) - (self.a * self.f)) * id,
        };
    }

    /// How much the transform scales lengths (on average).
    fn scale_factor(self: Affine) f64 {
        return @max(@sqrt(@abs(self.determinant())), 1.0e-9);
    }
}

// -----------------------------------------------------------------------------
// Paths
// -----------------------------------------------------------------------------

/// A Path's verbs, for code that walks `verbs` and `pts` (move and line
/// take one point, quad two, cubic three, close none).
pub enum Verb: u8 {
    MOVE,
    LINE,
    QUAD,
    CUBIC,
    CLOSE,
}

/// A sequence of subpaths, each starting with move_to.
pub struct Path {
    verbs: vec.Vec.[Verb],
    pts: vec.Vec.[f64],  // x, y pairs
    start_x: f64,        // the current subpath's first point
    start_y: f64,
    cur_x: f64,
    cur_y: f64,
    open: bool,  // a subpath has begun

    pub fn init() Path {
        return Path.{
            verbs: vec.Vec.[Verb].init(),
            pts: vec.Vec.[f64].init(),
            start_x: 0.0,
            start_y: 0.0,
            cur_x: 0.0,
            cur_y: 0.0,
            open: false,
        };
    }

    pub fn deinit(self: *mut Path) void {
        self.verbs.deinit();
        self.pts.deinit();
    }

    pub fn clear(self: *mut Path) void {
        self.verbs.clear();
        self.pts.clear();
        self.open = false;
    }

    fn pt(self: *mut Path, x: f64, y: f64) !void {
        self.pts.push(x)!;
        self.pts.push(y)!;
        self.cur_x = x;
        self.cur_y = y;
    }

    /// Starts a subpath at (x, y) if none is open (for line_to and curves
    /// with no current point).
    fn ensure(self: *mut Path, x: f64, y: f64) !void {
        if not self.open {
            self.move_to(x, y)!;
        }
    }

    pub fn move_to(self: *mut Path, x: f64, y: f64) !void {
        self.verbs.push(Verb.MOVE)!;
        self.pt(x, y)!;
        self.start_x = x;
        self.start_y = y;
        self.open = true;
    }

    pub fn line_to(self: *mut Path, x: f64, y: f64) !void {
        if not self.open {
            return self.move_to(x, y);
        }
        self.verbs.push(Verb.LINE)!;
        self.pt(x, y)!;
    }

    /// A quadratic Bézier with control point (cx, cy).
    pub fn quad_to(self: *mut Path, cx: f64, cy: f64, x: f64, y: f64) !void {
        self.ensure(cx, cy)!;
        self.verbs.push(Verb.QUAD)!;
        self.pt(cx, cy)!;
        self.pt(x, y)!;
    }

    /// A cubic Bézier with control points (c1x, c1y) and (c2x, c2y).
    pub fn cubic_to(self: *mut Path, c1x: f64, c1y: f64, c2x: f64, c2y: f64, x: f64, y: f64) !void {
        self.ensure(c1x, c1y)!;
        self.verbs.push(Verb.CUBIC)!;
        self.pt(c1x, c1y)!;
        self.pt(c2x, c2y)!;
        self.pt(x, y)!;
    }

    /// Ends the subpath with a line back to its start.
    pub fn close(self: *mut Path) !void {
        if not self.open {
            return;
        }
        self.verbs.push(Verb.CLOSE)!;
        self.cur_x = self.start_x;
        self.cur_y = self.start_y;
        self.open = false;
    }

    /// An arc of the circle at (cx, cy) with radius r, from angle a0 to a1
    /// (radians, clockwise on screen; counterclockwise if ccw), joined to
    /// the current point by a line (as in HTML canvas).
    pub fn arc(self: *mut Path, cx: f64, cy: f64, r: f64, a0: f64, a1: f64, ccw: bool) !void {
        self.elliptic_arc(cx, cy, r, r, a0, a1, ccw)!;
    }

    pub fn elliptic_arc(self: *mut Path, cx: f64, cy: f64, rx: f64, ry: f64, a0: f64, a1: f64, ccw: bool) !void {
        let mut sweep = a1 - a0;
        let tau = math.TAU;
        if ccw {
            if sweep > 0.0 {
                sweep = sweep - (tau * @ceil(sweep / tau));
            }
            sweep = @max(sweep, -tau);
        } else {
            if sweep < 0.0 {
                sweep = sweep + (tau * @ceil(-sweep / tau));
            }
            sweep = @min(sweep, tau);
        }
        let sc0 = math.sincos(a0);
        let x0 = cx + (rx * sc0.cos);
        let y0 = cy + (ry * sc0.sin);
        if self.open {
            self.line_to(x0, y0)!;
        } else {
            self.move_to(x0, y0)!;
        }
        // cubic segments of at most a quarter turn
        let n = @max(@ceil(@abs(sweep) / (math.PI / 2.0)), 1.0);
        let step = sweep / n;
        let k = (4.0 / 3.0) * math.tan(step / 4.0);
        let mut a = a0;
        let mut i: f64 = 0.0;
        while i < n {
            let s0 = math.sincos(a);
            let s1 = math.sincos(a + step);
            self.cubic_to(cx + (rx * (s0.cos - (k * s0.sin))), cy + (ry * (s0.sin + (k * s0.cos))),
                          cx + (rx * (s1.cos + (k * s1.sin))), cy + (ry * (s1.sin - (k * s1.cos))), cx + (rx * s1.cos),
                          cy + (ry * s1.sin))!;
            a = a + step;
            i += 1.0;
        }
    }

    pub fn rect(self: *mut Path, x: f64, y: f64, w: f64, h: f64) !void {
        self.move_to(x, y)!;
        self.line_to(x + w, y)!;
        self.line_to(x + w, y + h)!;
        self.line_to(x, y + h)!;
        self.close()!;
    }

    /// A rectangle with corners rounded to radius r.
    pub fn rounded_rect(self: *mut Path, x: f64, y: f64, w: f64, h: f64, r: f64) !void {
        let rr = @min(@max(r, 0.0), @min(@abs(w), @abs(h)) / 2.0);
        if rr == 0.0 {
            return self.rect(x, y, w, h);
        }
        let hp = math.PI / 2.0;
        self.move_to(x + rr, y)!;
        self.line_to((x + w) - rr, y)!;
        self.arc((x + w) - rr, y + rr, rr, -hp, 0.0, false)!;
        self.line_to(x + w, (y + h) - rr)!;
        self.arc((x + w) - rr, (y + h) - rr, rr, 0.0, hp, false)!;
        self.line_to(x + rr, y + h)!;
        self.arc(x + rr, (y + h) - rr, rr, hp, math.PI, false)!;
        self.line_to(x, y + rr)!;
        self.arc(x + rr, y + rr, rr, math.PI, math.PI + hp, false)!;
        self.close()!;
    }

    pub fn ellipse(self: *mut Path, cx: f64, cy: f64, rx: f64, ry: f64) !void {
        self.move_to(cx + rx, cy)!;
        self.elliptic_arc(cx, cy, rx, ry, 0.0, math.TAU, false)!;
        self.close()!;
    }

    pub fn circle(self: *mut Path, cx: f64, cy: f64, r: f64) !void {
        self.ellipse(cx, cy, r, r)!;
    }

    /// A closed polygon (x, y pairs).
    pub fn polygon(self: *mut Path, xy: []f64) !void {
        let mut i: usize = 0;
        while (i + 1) < xy.len {
            if i == 0 {
                self.move_to(xy[0], xy[1])!;
            } else {
                self.line_to(xy[i], xy[i + 1])!;
            }
            i += 2;
        }
        self.close()!;
    }
}

// -----------------------------------------------------------------------------
// Flattening: paths to polylines
// -----------------------------------------------------------------------------

/// Polylines: points (x, y pairs), where each one starts, and whether it is
/// closed.
struct Poly {
    pts: vec.Vec.[f64],
    starts: vec.Vec.[usize],  // index of each polyline's first point (in points)
    closed: vec.Vec.[bool],

    fn init() Poly {
        return Poly.{ pts: vec.Vec.[f64].init(), starts: vec.Vec.[usize].init(), closed: vec.Vec.[bool].init() };
    }

    fn deinit(self: *mut Poly) void {
        self.pts.deinit();
        self.starts.deinit();
        self.closed.deinit();
    }

    fn clear(self: *mut Poly) void {
        self.pts.clear();
        self.starts.clear();
        self.closed.clear();
    }

    fn npoints(self: *Poly) usize {
        return self.pts.len / 2;
    }

    fn begin(self: *mut Poly) !void {
        self.starts.push(self.npoints())!;
        self.closed.push(false)!;
    }

    fn add(self: *mut Poly, x: f64, y: f64) !void {
        self.pts.push(x)!;
        self.pts.push(y)!;
    }

    fn close_last(self: *mut Poly) void {
        if self.closed.len > 0 {
            self.closed.set(self.closed.len - 1, true);
        }
    }

    /// Points [first, end) of polyline i.
    fn range(self: *Poly, i: usize) [2]usize {
        let first = self.starts.get(i);
        let mut end = self.npoints();
        if (i + 1) < self.starts.len {
            end = self.starts.get(i + 1);
        }
        return [first, end];
    }

    fn x(self: *Poly, i: usize) f64 {
        return self.pts.get(2 * i);
    }

    fn y(self: *Poly, i: usize) f64 {
        return self.pts.get((2 * i) + 1);
    }
}

/// The path through t as polylines, with curves split finely enough that
/// they stray at most tol from the curve (in t's output space).
fn flatten(p: *Path, t: Affine, tol: f64, out: *mut Poly) !void {
    let verbs = p.verbs.items();
    let pts = p.pts.items();
    let mut k: usize = 0;
    let mut cx: f64 = 0.0;
    let mut cy: f64 = 0.0;
    for v in verbs {
        if v == Verb.MOVE {
            let q = t.apply(pts[k], pts[k + 1]);
            k += 2;
            out.begin()!;
            out.add(q.x, q.y)!;
            cx = q.x;
            cy = q.y;
        } else if v == Verb.LINE {
            let q = t.apply(pts[k], pts[k + 1]);
            k += 2;
            out.add(q.x, q.y)!;
            cx = q.x;
            cy = q.y;
        } else if v == Verb.QUAD {
            let c = t.apply(pts[k], pts[k + 1]);
            let e = t.apply(pts[k + 2], pts[k + 3]);
            k += 4;
            let ddx = (cx - (2.0 * c.x)) + e.x;
            let ddy = (cy - (2.0 * c.y)) + e.y;
            let n = segments(@sqrt((ddx * ddx) + (ddy * ddy)) / (4.0 * tol));
            for i in @as(usize, 1)..n + 1 {
                let s = @float_from_int(f64, i) / @float_from_int(f64, n);
                let u = 1.0 - s;
                out.add(((u * u) * cx) + ((2.0 * u * s) * c.x) + ((s * s) * e.x),
                        ((u * u) * cy) + ((2.0 * u * s) * c.y) + ((s * s) * e.y))!;
            }
            cx = e.x;
            cy = e.y;
        } else if v == Verb.CUBIC {
            let c1 = t.apply(pts[k], pts[k + 1]);
            let c2 = t.apply(pts[k + 2], pts[k + 3]);
            let e = t.apply(pts[k + 4], pts[k + 5]);
            k += 6;
            let d1x = (cx - (2.0 * c1.x)) + c2.x;
            let d1y = (cy - (2.0 * c1.y)) + c2.y;
            let d2x = (c1.x - (2.0 * c2.x)) + e.x;
            let d2y = (c1.y - (2.0 * c2.y)) + e.y;
            let m = @max((d1x * d1x) + (d1y * d1y), (d2x * d2x) + (d2y * d2y));
            let n = segments((0.75 * @sqrt(m)) / tol);
            for i in @as(usize, 1)..n + 1 {
                let s = @float_from_int(f64, i) / @float_from_int(f64, n);
                let u = 1.0 - s;
                let b0 = (u * u) * u;
                let b1 = (3.0 * (u * u)) * s;
                let b2 = (3.0 * u) * (s * s);
                let b3 = (s * s) * s;
                out.add((((b0 * cx) + (b1 * c1.x)) + (b2 * c2.x)) + (b3 * e.x),
                        (((b0 * cy) + (b1 * c1.y)) + (b2 * c2.y)) + (b3 * e.y))!;
            }
            cx = e.x;
            cy = e.y;
        } else {
            out.close_last();
        }
    }
}

/// Wang's bound: sqrt(x) segments, at least 1.
fn segments(x: f64) usize {
    let n = @ceil(@sqrt(@max(x, 0.0)));
    if not (n >= 1.0) {
        return 1;
    }
    return @int_from_float(usize, @min(n, 1000.0));
}

// -----------------------------------------------------------------------------
// Rasterization
//
// Each pixel row is sampled on SUB horizontal lines. On each, the edges'
// crossings are sorted and walked with their winding numbers; the spans the
// fill rule includes are added to the row with exact fractional ends. So
// coverage is exact across (to a small fraction of a pixel) and 1/SUB down,
// and overlapping contours (a self-intersecting path, the pieces of a
// stroke) combine by their true winding, not by adding up.
// -----------------------------------------------------------------------------

/// Which points a fill covers: those with a nonzero winding number, or an odd one.
pub enum FillRule: u8 {
    NONZERO,
    EVEN_ODD,
}

const BAND: usize = 16;  // rows prepared at a time
const SUB: usize = 16;   // sample lines per row

struct Edge {
    x0: f64,  // at y0
    y0: f64,
    y1: f64,  // y0 < y1
    dxdy: f64,
    dir: i32,  // +1 downward, -1 upward
}

struct Crossing {
    x: f64,
    dir: i32,
    e: usize,  // the edge
}

const FAR: f64 = 1.0e300;  // the x of a crossing that isn't there

/// The pixel rectangle [x0, x1) x [y0, y1) to rasterize: the polygons' bounds
/// within the canvas.
struct Bounds {
    x0: usize,
    y0: usize,
    x1: usize,
    y1: usize,
}

fn bounds_of(p: *Poly, w: usize, h: usize) Bounds {
    let n = p.npoints();
    let mut minx: f64 = 1.0e300;
    let mut miny: f64 = 1.0e300;
    let mut maxx: f64 = -1.0e300;
    let mut maxy: f64 = -1.0e300;
    for i in @as(usize, 0)..n {
        minx = @min(minx, p.x(i));
        maxx = @max(maxx, p.x(i));
        miny = @min(miny, p.y(i));
        maxy = @max(maxy, p.y(i));
    }
    let wf = @float_from_int(f64, w);
    let hf = @float_from_int(f64, h);
    if (n == 0) or not (maxx > 0.0) or not (maxy > 0.0) or not (minx < wf) or not (miny < hf) {
        return Bounds.{ x0: 0, y0: 0, x1: 0, y1: 0 };
    }
    return Bounds.{
        x0: @int_from_float(usize, @floor(@max(minx, 0.0))),
        y0: @int_from_float(usize, @floor(@max(miny, 0.0))),
        x1: @int_from_float(usize, @ceil(@min(maxx, wf))),
        y1: @int_from_float(usize, @ceil(@min(maxy, hf))),
    };
}

/// Adds weight w of coverage over [xa, xb) (pixels relative to the row's
/// start, clamped to [0, n]) to a row: partial pixels into cov, whole runs
/// as differences into run.
fn add_span(cov: []mut f32, run: []mut f32, n: usize, xa0: f64, xb0: f64, w: f32) void {
    let nf = @float_from_int(f64, n);
    let xa = @min(@max(xa0, 0.0), nf);
    let xb = @min(@max(xb0, 0.0), nf);
    if xb <= xa {
        return;
    }
    let ia = @int_from_float(usize, @floor(xa));
    let ib = @int_from_float(usize, @floor(xb));
    if ia == ib {
        cov[ia] += w * @float_cast(f32, xb - xa);
        return;
    }
    cov[ia] += w * @float_cast(f32, (@float_from_int(f64, ia) + 1.0) - xa);
    run[ia + 1] += w;
    run[ib] -= w;
    if ib < n {
        cov[ib] += w * @float_cast(f32, xb - @float_from_int(f64, ib));
    }
}

/// Calls visit(ctx, x, y, cov) for each row of the polygons' coverage:
/// cov[i] is the coverage of pixel (x + i, y).
fn rasterize[C](
    p: *Poly, w: usize, h: usize, rule: FillRule, ctx: *mut C, visit: fn(*mut C, usize, usize, []f32) void) !void {
    let b = bounds_of(p, w, h);
    if (b.x1 <= b.x0) or (b.y1 <= b.y0) {
        return;
    }
    let bw = b.x1 - b.x0;
    let ox = @float_from_int(f64, b.x0);
    let cov = mem.alloc.[f32](bw + 1)!;
    defer mem.free(cov);
    let run = mem.alloc.[f32](bw + 2)!;
    defer mem.free(run);
    @mem_set(cov, 0.0);  // each row clears what it used
    @mem_set(run, 0.0);
    let mut edges = vec.Vec.[Edge].init();
    defer edges.deinit();
    let mut row_edges = vec.Vec.[Edge].init();
    defer row_edges.deinit();
    let mut xs = vec.Vec.[Crossing].init();
    defer xs.deinit();
    let wsub = 1.0 / @float_from_int(f32, SUB);
    // every edge (every polygon closed for filling), top first
    let mut by_top = vec.Vec.[Edge].init();
    defer by_top.deinit();
    for pi in @as(usize, 0)..p.starts.len {
        let r = p.range(pi);
        if (r[1] - r[0]) < 2 {
            continue;
        }
        let pts = p.pts.items();
        for i in r[0]..r[1] {
            let mut j = i + 1;
            if j == r[1] {
                j = r[0];
            }
            let ax = pts[2 * i];
            let ay = pts[(2 * i) + 1];
            let bx = pts[2 * j];
            let by = pts[(2 * j) + 1];
            if ay < by {
                by_top.push(Edge.{ x0: ax - ox, y0: ay, y1: by, dxdy: (bx - ax) / (by - ay), dir: 1 })!;
            } else if by < ay {
                by_top.push(Edge.{ x0: bx - ox, y0: by, y1: ay, dxdy: (ax - bx) / (ay - by), dir: -1 })!;
            }
        }
    }
    vec.sort(by_top.items(), edge_less);
    let mut next: usize = 0;
    let mut y0 = b.y0;
    while y0 < b.y1 {
        let bh = @min(BAND, b.y1 - y0);
        let top = @float_from_int(f64, y0);
        let bot = top + @float_from_int(f64, bh);
        // the edges this band's sample lines may cross: drop those that
        // ended above it, take on those that start in it
        let mut kept: usize = 0;
        for k in @as(usize, 0)..edges.len {
            let e = edges.get(k);
            if e.y1 > top {
                edges.set(kept, e);
                kept += 1;
            }
        }
        edges.truncate(kept);
        while (next < by_top.len) and (by_top.get(next).y0 < bot) {
            let e = by_top.get(next);
            if e.y1 > top {
                edges.push(e)!;
            }
            next += 1;
        }
        for row in @as(usize, 0)..bh {
            let yrow = top + @float_from_int(f64, row);
            // this row's edges, so each sample line looks at few
            row_edges.clear();
            for e in edges.items() {
                if (e.y1 > yrow) and (e.y0 < (yrow + 1.0)) {
                    row_edges.push(e)!;
                }
            }
            let es = row_edges.items();
            if es.len < 2 {
                continue;
            }
            let mut any = false;
            let mut minx = FAR;  // the columns the spans touch
            let mut maxx = -FAR;
            // one crossing per edge, kept in x order from line to line
            // (it changes little, so insertion sort is quick); edges a
            // line misses sort last with no direction
            xs.clear();
            for i in @as(usize, 0)..es.len {
                xs.push(Crossing.{ x: FAR, dir: 0, e: i })!;
            }
            let c = xs.items();
            let mut sorted = false;
            for sub in @as(usize, 0)..SUB {
                let ys = yrow + ((@float_from_int(f64, sub) + 0.5) / @float_from_int(f64, SUB));
                let mut active: usize = 0;
                for k in @as(usize, 0)..c.len {
                    let e = es[c[k].e];
                    if (ys >= e.y0) and (ys < e.y1) {
                        c[k].x = e.x0 + ((ys - e.y0) * e.dxdy);
                        c[k].dir = e.dir;
                        active += 1;
                    } else {
                        c[k].x = FAR;
                        c[k].dir = 0;
                    }
                }
                if active < 2 {
                    continue;
                }
                if (not sorted) and (c.len > 32) {
                    vec.sort(c, crossing_less);
                } else {
                    for i in @as(usize, 1)..c.len {
                        let v = c[i];
                        let mut k = i;
                        while (k > 0) and (c[k - 1].x > v.x) {
                            c[k] = c[k - 1];
                            k -= 1;
                        }
                        c[k] = v;
                    }
                }
                sorted = true;
                // one span for each stretch inside (overlapping pieces
                // of a stroke cross inside it often)
                let mut wind: i32 = 0;
                let mut start: f64 = 0.0;
                for i in @as(usize, 0)..c.len {
                    if c[i].x >= FAR {
                        break;
                    }
                    let was = (rule == FillRule.EVEN_ODD and ((wind & 1) != 0)) or
                              ((rule != FillRule.EVEN_ODD) and (wind != 0));
                    wind += c[i].dir;
                    let now = (rule == FillRule.EVEN_ODD and ((wind & 1) != 0)) or
                              ((rule != FillRule.EVEN_ODD) and (wind != 0));
                    if now and not was {
                        start = c[i].x;
                    } else if was and not now and (c[i].x > start) {
                        add_span(cov, run, bw, start, c[i].x, wsub);
                        minx = @min(minx, start);
                        maxx = @max(maxx, c[i].x);
                        any = true;
                    }
                }
            }
            if not any {
                continue;
            }
            let fw = @float_from_int(f64, bw);
            let lo = @int_from_float(usize, @floor(@min(@max(minx, 0.0), fw)));
            let hi = @min(@int_from_float(usize, @floor(@min(@max(maxx, 0.0), fw))) + 1, bw);
            let mut acc: f32 = 0.0;
            for x in lo..hi {
                acc += run[x];
                cov[x] = @min(cov[x] + acc, 1.0);
            }
            if lo < hi {
                visit(ctx, b.x0 + lo, y0 + row, cov[lo..hi]);
            }
            @mem_set(cov[lo..hi + 1], 0.0);
            @mem_set(run[lo..hi + 2], 0.0);
        }
        y0 += bh;
    }
}

// -----------------------------------------------------------------------------
// Strokes
// -----------------------------------------------------------------------------

/// How a stroke's segments meet.
pub enum Join: u8 {
    MITER,
    ROUND,
    BEVEL,
}

/// How a stroke's open ends are drawn.
pub enum Cap: u8 {
    BUTT,
    ROUND,
    SQUARE,
}

pub struct Stroke {
    width: f64,
    join: Join,
    cap: Cap,
    miter_limit: f64,  // longest miter, in widths (as SVG's)
    dash: []f64,       // on, off, on, ... lengths (empty: solid)
    dash_offset: f64,

    pub fn width(w: f64) Stroke {
        return Stroke.{
            width: w,
            join: Join.MITER,
            cap: Cap.BUTT,
            miter_limit: 4.0,
            dash: @slice_from_ptr(@ptr_from_int(*f64, 8), 0),
            dash_offset: 0.0,
        };
    }
}

/// The stroke as polygons (all counterclockwise in user space... their
/// orientation is fixed after transforming), unioned by the nonzero rule.
struct Stroker {
    out: *mut Poly,
    hw: f64,
    style: *Stroke,
    t: Affine,
    tol: f64,  // in user space

    fn piece_begin(self: *mut Stroker) !void {
        self.out.begin()!;
    }

    fn pt(self: *mut Stroker, x: f64, y: f64) !void {
        let q = self.t.apply(x, y);
        self.out.add(q.x, q.y)!;
    }

    fn quad(self: *mut Stroker, ax: f64, ay: f64, bx: f64, by: f64, cx: f64, cy: f64, dx: f64, dy: f64) !void {
        self.piece_begin()!;
        self.pt(ax, ay)!;
        self.pt(bx, by)!;
        self.pt(cx, cy)!;
        self.pt(dx, dy)!;
    }

    fn triangle(self: *mut Stroker, ax: f64, ay: f64, bx: f64, by: f64, cx: f64, cy: f64) !void {
        self.piece_begin()!;
        self.pt(ax, ay)!;
        self.pt(bx, by)!;
        self.pt(cx, cy)!;
    }

    fn disc(self: *mut Stroker, cx: f64, cy: f64) !void {
        let r = self.hw;
        let mut n: usize = 8;
        if r > self.tol {
            let step = 2.0 * math.acos(1.0 - (self.tol / r));
            n = @max(@int_from_float(usize, @ceil(math.TAU / step)), 8);
            n = @min(n, 1024);
        }
        self.piece_begin()!;
        for i in @as(usize, 0)..n {
            let sc = math.sincos((math.TAU * @float_from_int(f64, i)) / @float_from_int(f64, n));
            self.pt(cx + (r * sc.cos), cy + (r * sc.sin))!;
        }
    }

    /// One polyline (user space points).
    fn polyline(self: *mut Stroker, xs: []f64, closed: bool) !void {
        let n = xs.len / 2;
        if n == 0 {
            return;
        }
        let hw = self.hw;
        if n == 1 {
            // a dot: only caps show
            if self.style.cap == Cap.ROUND {
                self.disc(xs[0], xs[1])!;
            } else if self.style.cap == Cap.SQUARE {
                self.quad(xs[0] - hw, xs[1] - hw, xs[0] + hw, xs[1] - hw, xs[0] + hw, xs[1] + hw, xs[0] - hw,
                          xs[1] + hw)!;
            }
            return;
        }
        let mut nseg = n - 1;
        if closed {
            nseg = n;
        }
        for i in @as(usize, 0)..nseg {
            let j = (i + 1) % n;
            let ax = xs[2 * i];
            let ay = xs[(2 * i) + 1];
            let bx = xs[2 * j];
            let by = xs[(2 * j) + 1];
            let dx = bx - ax;
            let dy = by - ay;
            let len = @sqrt((dx * dx) + (dy * dy));
            let nx = (-dy / len) * hw;
            let ny = (dx / len) * hw;
            self.quad(ax + nx, ay + ny, bx + nx, by + ny, bx - nx, by - ny, ax - nx, ay - ny)!;
        }
        // joins
        let mut first_join: usize = 1;
        let mut last_join = n - 1;
        if closed {
            first_join = 0;
            last_join = n;
        }
        for v in first_join..last_join {
            let prev = (v + n - 1) % n;
            let next = (v + 1) % n;
            self.join(xs[2 * prev], xs[(2 * prev) + 1], xs[2 * v], xs[(2 * v) + 1], xs[2 * next], xs[(2 * next) + 1])!;
        }
        // caps
        if not closed {
            self.cap(xs[2], xs[3], xs[0], xs[1])!;
            self.cap(xs[2 * (n - 2)], xs[(2 * (n - 2)) + 1], xs[2 * (n - 1)], xs[(2 * (n - 1)) + 1])!;
        }
    }

    /// The join at (bx, by) between segments from a and to c.
    fn join(self: *mut Stroker, ax: f64, ay: f64, bx: f64, by: f64, cx: f64, cy: f64) !void {
        let hw = self.hw;
        let d1x = bx - ax;
        let d1y = by - ay;
        let d2x = cx - bx;
        let d2y = cy - by;
        let l1 = @sqrt((d1x * d1x) + (d1y * d1y));
        let l2 = @sqrt((d2x * d2x) + (d2y * d2y));
        let u1x = d1x / l1;
        let u1y = d1y / l1;
        let u2x = d2x / l2;
        let u2y = d2y / l2;
        let cross = (u1x * u2y) - (u1y * u2x);
        let dot = (u1x * u2x) + (u1y * u2y);
        if (@abs(cross) < 1.0e-9) and (dot > 0.0) {
            return;  // straight on
        }
        if self.style.join == Join.ROUND {
            return self.disc(bx, by);
        }
        // the outer side: right of a left turn, left of a right one
        let mut s: f64 = 1.0;
        if cross > 0.0 {
            s = -1.0;
        }
        let n1x = (-u1y * hw) * s;
        let n1y = (u1x * hw) * s;
        let n2x = (-u2y * hw) * s;
        let n2y = (u2x * hw) * s;
        if self.style.join == Join.MITER {
            let ratio = @sqrt(2.0 / @max(1.0 + dot, 1.0e-12));
            if ratio <= self.style.miter_limit {
                // the miter tip: along the bisector of the normals
                let mx = bx + (((n1x + n2x) / (1.0 + dot)));
                let my = by + (((n1y + n2y) / (1.0 + dot)));
                return self.quad(bx, by, bx + n1x, by + n1y, mx, my, bx + n2x, by + n2y);
            }
        }
        self.triangle(bx, by, bx + n1x, by + n1y, bx + n2x, by + n2y)!;
    }

    /// The cap at end (ex, ey) of a segment coming from (ax, ay).
    fn cap(self: *mut Stroker, ax: f64, ay: f64, ex: f64, ey: f64) !void {
        let c = self.style.cap;
        if c == Cap.ROUND {
            return self.disc(ex, ey);
        }
        if c == Cap.SQUARE {
            let hw = self.hw;
            let dx = ex - ax;
            let dy = ey - ay;
            let len = @sqrt((dx * dx) + (dy * dy));
            let ux = dx / len;
            let uy = dy / len;
            let nx = -uy * hw;
            let ny = ux * hw;
            self.quad(ex + nx, ey + ny, (ex + nx) + (ux * hw), (ey + ny) + (uy * hw), (ex - nx) + (ux * hw),
                      (ey - ny) + (uy * hw), ex - nx, ey - ny)!;
        }
    }
}

/// Copies a polyline without repeated points (which have no direction).
fn dedupe(xs: []f64, out: *mut vec.Vec.[f64]) !void {
    out.clear();
    let mut i: usize = 0;
    while (i + 1) < xs.len {
        let n = out.len;
        if (n < 2) or (out.get(n - 2) != xs[i]) or (out.get(n - 1) != xs[i + 1]) {
            out.push(xs[i])!;
            out.push(xs[i + 1])!;
        }
        i += 2;
    }
}

fn edge_less(a: Edge, b: Edge) bool {
    return a.y0 < b.y0;
}

fn crossing_less(a: Crossing, b: Crossing) bool {
    return a.x < b.x;
}

/// Splits a polyline into its dashes, stroking each.
fn dashed(sk: *mut Stroker, xs: []f64, closed: bool, buf: *mut vec.Vec.[f64]) !void {
    let dash = sk.style.dash;
    let mut total: f64 = 0.0;
    let mut valid = true;
    for d in dash {
        // (a negative, infinite or NaN length is not a pattern: solid)
        if not (d >= 0.0) or not (d < 1.0e300) {
            valid = false;
        }
        total += @max(d, 0.0);
    }
    if not valid or not (total > 0.0) {
        return sk.polyline(xs, closed);
    }
    let n = xs.len / 2;
    let mut nseg = n - 1;
    if closed {
        nseg = n;
    }
    // a pattern too fine for the path (millions of dashes, or lengths that
    // do not move the position in floating point) is drawn solid
    let mut path_len: f64 = 0.0;
    for i in @as(usize, 0)..nseg {
        let j = (i + 1) % n;
        let dx = xs[2 * j] - xs[2 * i];
        let dy = xs[(2 * j) + 1] - xs[(2 * i) + 1];
        path_len += @sqrt((dx * dx) + (dy * dy));
    }
    if not ((path_len / total) * @float_from_int(f64, dash.len) < 1.0e6) or (total < (path_len * 1.0e-12)) {
        return sk.polyline(xs, closed);
    }
    // where in the pattern the line starts
    let mut phase = @rem(sk.style.dash_offset, total);
    if phase < 0.0 {
        phase += total;
    }
    let mut di: usize = 0;
    while phase >= dash[di] {
        phase -= dash[di];
        di = (di + 1) % dash.len;
    }
    let mut left = dash[di] - phase;  // of the current dash or gap
    buf.clear();
    if (di % 2) == 0 {
        buf.push(xs[0])!;
        buf.push(xs[1])!;
    }
    for i in @as(usize, 0)..nseg {
        let j = (i + 1) % n;
        let ax = xs[2 * i];
        let ay = xs[(2 * i) + 1];
        let bx = xs[2 * j];
        let by = xs[(2 * j) + 1];
        let len = @sqrt(((bx - ax) * (bx - ax)) + ((by - ay) * (by - ay)));
        let mut pos: f64 = 0.0;
        while (len - pos) > left {
            pos += left;
            let t = pos / len;
            let px = ax + ((bx - ax) * t);
            let py = ay + ((by - ay) * t);
            if (di % 2) == 0 {
                // a dash ends
                buf.push(px)!;
                buf.push(py)!;
                sk.polyline(buf.items(), false)!;
                buf.clear();
            } else {
                // a dash starts
                buf.push(px)!;
                buf.push(py)!;
            }
            di = (di + 1) % dash.len;
            left = dash[di];
        }
        left -= len - pos;
        if (di % 2) == 0 {
            buf.push(bx)!;
            buf.push(by)!;
        }
    }
    if ((di % 2) == 0) and (buf.len >= 4) {
        sk.polyline(buf.items(), false)!;
    }
}

// -----------------------------------------------------------------------------
// Paint
// -----------------------------------------------------------------------------

/// What a Paint is.
pub enum PaintKind: u8 {
    SOLID,
    LINEAR,
    RADIAL,
    IMAGE,
}

/// How a gradient continues beyond its ends.
pub enum Spread: u8 {
    PAD,
    REPEAT,
    REFLECT,
}

/// A gradient colour stop: where (0 to 1) and which colour.
pub struct Stop {
    offset: f64,
    color: color.Color,
}

pub struct Paint {
    kind: PaintKind,
    color: color.Color,  // solid
    stops: []Stop,       // gradients, by increasing offset
    x0: f64,             // linear: from (x0, y0) to (x1, y1); radial: centre (x0, y0), radius r
    y0: f64,
    x1: f64,
    y1: f64,
    r: f64,
    spread: Spread,
    image: *image.Image,  // PaintKind.IMAGE
    transform: Affine,    // paint space to user space

    pub fn solid(c: color.Color) Paint {
        let mut p = base_paint(PaintKind.SOLID);
        p.color = c;
        return p;
    }

    pub fn linear(x0: f64, y0: f64, x1: f64, y1: f64, stops: []Stop) Paint {
        let mut p = base_paint(PaintKind.LINEAR);
        p.x0 = x0;
        p.y0 = y0;
        p.x1 = x1;
        p.y1 = y1;
        p.stops = stops;
        return p;
    }

    pub fn radial(cx: f64, cy: f64, r: f64, stops: []Stop) Paint {
        let mut p = base_paint(PaintKind.RADIAL);
        p.x0 = cx;
        p.y0 = cy;
        p.r = r;
        p.stops = stops;
        return p;
    }

    /// An image, its pixel (0, 0) at user-space (0, 0) (then transform),
    /// filtered bilinearly, transparent outside.
    pub fn pattern(img: *image.Image, transform: Affine) Paint {
        let mut p = base_paint(PaintKind.IMAGE);
        p.image = img;
        p.transform = transform;
        return p;
    }

    pub fn with_spread(self: Paint, spread: Spread) Paint {
        let mut p = self;
        p.spread = spread;
        return p;
    }

    pub fn with_transform(self: Paint, t: Affine) Paint {
        let mut p = self;
        p.transform = t;
        return p;
    }
}

fn base_paint(kind: PaintKind) Paint {
    return Paint.{
        kind: kind,
        color: color.Color.rgb(0.0, 0.0, 0.0),
        stops: @slice_from_ptr(@ptr_from_int(*Stop, 8), 0),
        x0: 0.0,
        y0: 0.0,
        x1: 0.0,
        y1: 0.0,
        r: 0.0,
        spread: Spread.PAD,
        image: @ptr_from_int(*image.Image, 8),
        transform: Affine.identity(),
    };
}

/// Premultiplied linear RGBA.
struct Premul {
    r: f32,
    g: f32,
    b: f32,
    a: f32,
}

fn premul(c: color.Color) Premul {
    return Premul.{ r: c.r * c.a, g: c.g * c.a, b: c.b * c.a, a: c.a };
}

const LUT_SIZE: usize = 256;

/// What a paint needs while filling: device pixel to paint space, and a
/// gradient's colours tabulated.
struct Shader {
    paint: *Paint,
    inv: Affine,  // device to paint space
    solid: Premul,
    lut: [256]Premul,
}

fn make_shader(p: *Paint, ctm: Affine, sh: *mut Shader) void {
    sh.paint = p;
    sh.inv = p.transform.then(ctm).invert();
    sh.solid = premul(p.color);
    if (p.kind == PaintKind.LINEAR) or (p.kind == PaintKind.RADIAL) {
        for i in @as(usize, 0)..LUT_SIZE {
            let t = @float_from_int(f64, i) / @float_from_int(f64, LUT_SIZE - 1);
            sh.lut[i] = gradient_at(p.stops, t);
        }
    }
}

/// The stops' colour at t (premultiplied, interpolated in linear light).
fn gradient_at(stops: []Stop, t: f64) Premul {
    if stops.len == 0 {
        return Premul.{ r: 0.0, g: 0.0, b: 0.0, a: 0.0 };
    }
    if t <= stops[0].offset {
        return premul(stops[0].color);
    }
    for i in @as(usize, 1)..stops.len {
        if t <= stops[i].offset {
            let a = premul(stops[i - 1].color);
            let b = premul(stops[i].color);
            let span = stops[i].offset - stops[i - 1].offset;
            let mut f: f32 = 0.0;
            if span > 0.0 {
                f = @float_cast(f32, (t - stops[i - 1].offset) / span);
            }
            return Premul.{
                r: a.r + ((b.r - a.r) * f),
                g: a.g + ((b.g - a.g) * f),
                b: a.b + ((b.b - a.b) * f),
                a: a.a + ((b.a - a.a) * f),
            };
        }
    }
    return premul(stops[stops.len - 1].color);
}

fn spread_t(t: f64, spread: Spread) f64 {
    if spread == Spread.REPEAT {
        return t - @floor(t);
    }
    if spread == Spread.REFLECT {
        let m = @rem(@abs(t), 2.0);
        if m > 1.0 {
            return 2.0 - m;
        }
        return m;
    }
    return @min(@max(t, 0.0), 1.0);
}

/// The paint's colour at the centre of device pixel (x, y).
fn shade(sh: *Shader, x: usize, y: usize) Premul {
    let p = sh.paint;
    if p.kind == PaintKind.SOLID {
        return sh.solid;
    }
    let q = sh.inv.apply(@float_from_int(f64, x) + 0.5, @float_from_int(f64, y) + 0.5);
    if p.kind == PaintKind.IMAGE {
        return sample(p.image, q.x, q.y);
    }
    let mut t: f64 = 0.0;
    if p.kind == PaintKind.LINEAR {
        let dx = p.x1 - p.x0;
        let dy = p.y1 - p.y0;
        let dd = (dx * dx) + (dy * dy);
        if dd > 0.0 {
            t = (((q.x - p.x0) * dx) + ((q.y - p.y0) * dy)) / dd;
        }
    } else if p.r > 0.0 {
        t = @sqrt(((q.x - p.x0) * (q.x - p.x0)) + ((q.y - p.y0) * (q.y - p.y0))) / p.r;
    }
    let s = spread_t(t, p.spread);
    let fi = s * @float_from_int(f64, LUT_SIZE - 1);
    let i0 = @int_from_float(usize, @floor(fi));
    let i1 = @min(i0 + 1, LUT_SIZE - 1);
    let f = @float_cast(f32, fi - @floor(fi));
    let a = sh.lut[i0];
    let b = sh.lut[i1];
    return Premul.{ r: a.r + ((b.r - a.r) * f), g: a.g + ((b.g - a.g) * f), b: a.b + ((b.b - a.b) * f),
                    a: a.a + ((b.a - a.a) * f) };
}

/// Bilinear sample of an image at (x, y) in its pixel space (pixel
/// centres at half-integers); transparent outside.
fn sample(img: *image.Image, x: f64, y: f64) Premul {
    let fx = x - 0.5;
    let fy = y - 0.5;
    let x0 = @floor(fx);
    let y0 = @floor(fy);
    let tx = @float_cast(f32, fx - x0);
    let ty = @float_cast(f32, fy - y0);
    let mut acc = Premul.{ r: 0.0, g: 0.0, b: 0.0, a: 0.0 };
    let wf = @float_from_int(f64, img.width);
    let hf = @float_from_int(f64, img.height);
    for k in @as(usize, 0)..4 {
        let px = x0 + @float_from_int(f64, k % 2);
        let py = y0 + @float_from_int(f64, k / 2);
        let mut wx = 1.0 - tx;
        if (k % 2) == 1 {
            wx = tx;
        }
        let mut wy = 1.0 - ty;
        if (k / 2) == 1 {
            wy = ty;
        }
        let w = wx * wy;
        if (w > 0.0) and (px >= 0.0) and (py >= 0.0) and (px < wf) and (py < hf) {
            let c = premul(img.get(@int_from_float(usize, px), @int_from_float(usize, py)));
            acc.r += c.r * w;
            acc.g += c.g * w;
            acc.b += c.b * w;
            acc.a += c.a * w;
        }
    }
    return acc;
}

// -----------------------------------------------------------------------------
// Compositing
// -----------------------------------------------------------------------------

/// How colours combine: a Porter-Duff operator or a blend mode.
pub enum Op: u8 {
    // Porter-Duff operators
    CLEAR,
    SRC,
    OVER,  // the default
    DST_OVER,
    SRC_IN,
    DST_IN,
    SRC_OUT,
    DST_OUT,
    SRC_ATOP,
    DST_ATOP,
    XOR,
    PLUS,
    // blend modes (source-over, mixing the colours)
    MULTIPLY,
    SCREEN,
    OVERLAY,
    DARKEN,
    LIGHTEN,
    DIFFERENCE,

    /// Whether this is a blend mode (MULTIPLY to DIFFERENCE).
    pub fn is_blend(self: Op) bool {
        return @int_from_enum(self) >= @int_from_enum(Op.MULTIPLY);
    }
}

fn blend_channel(mode: Op, cb: f32, cs: f32) f32 {
    if mode == Op.MULTIPLY {
        return cb * cs;
    }
    if mode == Op.SCREEN {
        return (cb + cs) - (cb * cs);
    }
    if mode == Op.OVERLAY {
        if cb <= 0.5 {
            return (2.0 * cb) * cs;
        }
        let s2 = (2.0 * cs) - 1.0;
        return (cb + s2) - (cb * s2);
    }
    if mode == Op.DARKEN {
        return @min(cb, cs);
    }
    if mode == Op.LIGHTEN {
        return @max(cb, cs);
    }
    return @abs(cb - cs);
}

/// s composited onto d (both premultiplied) by the operator.
fn composite(op: Op, s: Premul, d: Premul) Premul {
    let sa = s.a;
    let da = d.a;
    let mut fs: f32 = 0.0;  // how much of the source and destination remain
    let mut fd: f32 = 0.0;
    if op == Op.CLEAR {
        return Premul.{ r: 0.0, g: 0.0, b: 0.0, a: 0.0 };
    } else if op == Op.SRC {
        fs = 1.0;
    } else if (op == Op.OVER) or op.is_blend() {
        fs = 1.0;
        fd = 1.0 - sa;
    } else if op == Op.DST_OVER {
        fs = 1.0 - da;
        fd = 1.0;
    } else if op == Op.SRC_IN {
        fs = da;
    } else if op == Op.DST_IN {
        fd = sa;
    } else if op == Op.SRC_OUT {
        fs = 1.0 - da;
    } else if op == Op.DST_OUT {
        fd = 1.0 - sa;
    } else if op == Op.SRC_ATOP {
        fs = da;
        fd = 1.0 - sa;
    } else if op == Op.DST_ATOP {
        fs = 1.0 - da;
        fd = sa;
    } else if op == Op.XOR {
        fs = 1.0 - da;
        fd = 1.0 - sa;
    } else {
        fs = 1.0;
        fd = 1.0;
    }
    let mut r = Premul.{
        r: (s.r * fs) + (d.r * fd),
        g: (s.g * fs) + (d.g * fd),
        b: (s.b * fs) + (d.b * fd),
        a: @min((sa * fs) + (da * fd), 1.0),
    };
    if op.is_blend() and (sa > 0.0) and (da > 0.0) {
        // Cr = (1 - da) Cs + (1 - sa) Cb + sa da B(Cb, Cs), premultiplied
        let k = sa * da;
        r.r = ((s.r * (1.0 - da)) + (d.r * (1.0 - sa))) + (k * blend_channel(op, d.r / da, s.r / sa));
        r.g = ((s.g * (1.0 - da)) + (d.g * (1.0 - sa))) + (k * blend_channel(op, d.g / da, s.g / sa));
        r.b = ((s.b * (1.0 - da)) + (d.b * (1.0 - sa))) + (k * blend_channel(op, d.b / da, s.b / sa));
    }
    return r;
}

let mut U16_LINEAR: [65536]f32 = [0.0; 65536];
let mut U16_ONCE: sync.Once = sync.Once.{ state: 0 };

fn build_u16(_u: u8) void {
    for i in @as(usize, 0)..65536 {
        U16_LINEAR[i] = color.srgb_decode(@float_from_int(f32, i) / 65535.0);
    }
}

fn u16_linear(v: u16) f32 {
    U16_ONCE.call(build_u16, @as(u8, 0));
    return U16_LINEAR[@as(usize, v)];
}

/// Reads pixel (x, y) as premultiplied linear colour.
fn read_px(img: *image.Image, x: usize, y: usize) Premul {
    let ch = img.channels;
    let i = ((y * img.width) + x) * ch;
    let mut v: [4]f32 = [0.0, 0.0, 0.0, 1.0];
    if img.depth == image.Depth.U8 {
        let s = img.data;
        for c in @as(usize, 0)..ch {
            if (c == (ch - 1)) and ((ch == 2) or (ch == 4)) {
                v[3] = @float_from_int(f32, s[i + c]) / 255.0;
            } else {
                v[c] = color.linear8(s[i + c]);
            }
        }
    } else if img.depth == image.Depth.U16 {
        let s = img.samples16();
        for c in @as(usize, 0)..ch {
            if (c == (ch - 1)) and ((ch == 2) or (ch == 4)) {
                v[3] = @float_from_int(f32, s[i + c]) / 65535.0;
            } else {
                v[c] = u16_linear(s[i + c]);
            }
        }
    } else {
        let s = img.samples32();
        for c in @as(usize, 0)..ch {
            if (c == (ch - 1)) and ((ch == 2) or (ch == 4)) {
                v[3] = s[i + c];
            } else {
                v[c] = s[i + c];
            }
        }
    }
    if ch <= 2 {
        v[1] = v[0];
        v[2] = v[0];
    }
    return Premul.{ r: v[0] * v[3], g: v[1] * v[3], b: v[2] * v[3], a: v[3] };
}

fn write_px(img: *mut image.Image, x: usize, y: usize, p: Premul) void {
    let mut c = color.Color.{ r: 0.0, g: 0.0, b: 0.0, a: p.a };
    if p.a > 0.0 {
        c = color.Color.{ r: p.r / p.a, g: p.g / p.a, b: p.b / p.a, a: p.a };
    }
    if not img.has_alpha() and (p.a < 1.0) {
        // no alpha channel: over black (what premultiplied means)
        c = color.Color.{ r: p.r, g: p.g, b: p.b, a: 1.0 };
    }
    img.set(x, y, c);
}

// -----------------------------------------------------------------------------
// The canvas
// -----------------------------------------------------------------------------

struct State {
    transform: Affine,
    op: Op,
    alpha: f32,
    clip: []mut f32,  // a copy of the clip mask (empty: none)
}

pub struct Canvas {
    img: *mut image.Image,
    transform: Affine,
    op: Op,      // how colours combine: OVER, or another operator or blend mode
    alpha: f32,  // global alpha
    antialias: bool,
    tolerance: f64,   // how far flattened curves may stray, in pixels
    clip: []mut f32,  // coverage of the clip, a value per pixel (empty: no clip)
    stack: vec.Vec.[State],
    poly: Poly,
    tmp: vec.Vec.[f64],
    tmp2: vec.Vec.[f64],

    pub fn init(img: *mut image.Image) Canvas {
        return Canvas.{
            img: img,
            transform: Affine.identity(),
            op: Op.OVER,
            alpha: 1.0,
            antialias: true,
            tolerance: 0.05,
            clip: @slice_from_ptr(@ptr_from_int(*mut f32, 4), 0),
            stack: vec.Vec.[State].init(),
            poly: Poly.init(),
            tmp: vec.Vec.[f64].init(),
            tmp2: vec.Vec.[f64].init(),
        };
    }

    pub fn deinit(self: *mut Canvas) void {
        while self.stack.len > 0 {
            let s = self.stack.pop() catch break;
            if s.clip.len > 0 {
                mem.free(s.clip);
            }
        }
        self.stack.deinit();
        if self.clip.len > 0 {
            mem.free(self.clip);
        }
        self.poly.deinit();
        self.tmp.deinit();
        self.tmp2.deinit();
    }

    /// Saves the transform, operator, alpha and clip; restore brings them back.
    pub fn save(self: *mut Canvas) !void {
        let mut c = @slice_from_ptr(@ptr_from_int(*mut f32, 4), 0);
        if self.clip.len > 0 {
            c = mem.dup(self.clip[..])!;
        }
        self.stack.push(State.{ transform: self.transform, op: self.op, alpha: self.alpha, clip: c }) catch |e| {
            if c.len > 0 {
                mem.free(c);
            }
            return e;
        };
    }

    pub fn restore(self: *mut Canvas) void {
        let s = self.stack.pop() catch return;
        self.transform = s.transform;
        self.op = s.op;
        self.alpha = s.alpha;
        if self.clip.len > 0 {
            mem.free(self.clip);
        }
        self.clip = s.clip;
    }

    pub fn translate(self: *mut Canvas, tx: f64, ty: f64) void {
        self.transform = Affine.translation(tx, ty).then(self.transform);
    }

    pub fn scale(self: *mut Canvas, sx: f64, sy: f64) void {
        self.transform = Affine.scaling(sx, sy).then(self.transform);
    }

    pub fn rotate(self: *mut Canvas, radians: f64) void {
        self.transform = Affine.rotation(radians).then(self.transform);
    }

    /// Applies t before the current transform (as canvas's transform()).
    pub fn concat(self: *mut Canvas, t: Affine) void {
        self.transform = t.then(self.transform);
    }

    /// Fills every pixel with a colour (ignoring the clip and operator).
    pub fn clear(self: *mut Canvas, c: color.Color) void {
        self.img.fill(c);
    }

    pub fn fill(self: *mut Canvas, path: *Path, paint: Paint, rule: FillRule) !void {
        self.poly.clear();
        flatten(path, self.transform, self.tolerance, &mut self.poly)!;
        self.paint_poly(paint, rule)!;
    }

    pub fn stroke(self: *mut Canvas, path: *Path, paint: Paint, style: Stroke) !void {
        if not (style.width > 0.0) {
            return;
        }
        // flatten in user space (the stroke is built there, so that its
        // width scales with the transform), finely enough for the device
        let scale = self.transform.scale_factor();
        let mut user = Poly.init();
        defer user.deinit();
        flatten(path, Affine.identity(), self.tolerance / scale, &mut user)!;
        self.poly.clear();
        let mut sk = Stroker.{
            out: &mut self.poly,
            hw: style.width / 2.0,
            style: &style,
            t: self.transform,
            tol: self.tolerance / scale,
        };
        for i in @as(usize, 0)..user.starts.len {
            let r = user.range(i);
            dedupe(user.pts.items()[2 * r[0]..2 * r[1]], &mut self.tmp)!;
            let mut closed = user.closed.get(i);
            let xs = self.tmp.items();
            let n = xs.len / 2;
            if closed and (n > 1) and (xs[0] == xs[(2 * n) - 2]) and (xs[1] == xs[(2 * n) - 1]) {
                self.tmp.truncate(xs.len - 2);  // the closing point repeats the first
            }
            if self.tmp.len < 6 {
                closed = false;
            }
            if style.dash.len > 0 {
                dashed(&mut sk, self.tmp.items(), closed, &mut self.tmp2)!;
            } else {
                sk.polyline(self.tmp.items(), closed)!;
            }
        }
        orient_positive(&mut self.poly);
        self.paint_poly(paint, FillRule.NONZERO)!;
    }

    /// Fills a rectangle.
    pub fn fill_rect(self: *mut Canvas, x: f64, y: f64, w: f64, h: f64, paint: Paint) !void {
        let mut p = Path.init();
        defer p.deinit();
        p.rect(x, y, w, h)!;
        self.fill(&p, paint, FillRule.NONZERO)!;
    }

    /// Strokes a line from (x0, y0) to (x1, y1).
    pub fn line(self: *mut Canvas, x0: f64, y0: f64, x1: f64, y1: f64, paint: Paint, style: Stroke) !void {
        let mut p = Path.init();
        defer p.deinit();
        p.move_to(x0, y0)!;
        p.line_to(x1, y1)!;
        self.stroke(&p, paint, style)!;
    }

    /// Draws an image into the rectangle (x, y, w, h) (user space), scaled
    /// with bilinear filtering.
    pub fn draw_image(self: *mut Canvas, img: *image.Image, x: f64, y: f64, w: f64, h: f64) !void {
        let sx = w / @float_from_int(f64, @max(img.width, 1));
        let sy = h / @float_from_int(f64, @max(img.height, 1));
        let t = Affine.scaling(sx, sy).then(Affine.translation(x, y));
        self.fill_rect(x, y, w, h, Paint.pattern(img, t))!;
    }

    /// Intersects the clip with a path.
    pub fn clip_to(self: *mut Canvas, path: *Path, rule: FillRule) !void {
        let w = self.img.width;
        let h = self.img.height;
        let mask = mem.alloc_zeroed.[f32](w * h)!;
        self.poly.clear();
        flatten(path, self.transform, self.tolerance, &mut self.poly) catch |e| {
            mem.free(mask);
            return e;
        };
        let mut mc = MaskCtx.{ mask: mask, w: w, aa: self.antialias };
        rasterize(&self.poly, w, h, rule, &mut mc, mask_row) catch |e| {
            mem.free(mask);
            return e;
        };
        if self.clip.len > 0 {
            for i in @as(usize, 0)..mask.len {
                mask[i] = mask[i] * self.clip[i];
            }
            mem.free(self.clip);
        }
        self.clip = mask;
    }

    /// Removes the clip.
    pub fn reset_clip(self: *mut Canvas) void {
        if self.clip.len > 0 {
            mem.free(self.clip);
        }
        self.clip = @slice_from_ptr(@ptr_from_int(*mut f32, 4), 0);
    }

    fn paint_poly(self: *mut Canvas, paint: Paint, rule: FillRule) !void {
        let sh = mem.create(Shader.{
            paint: &paint,
            inv: Affine.identity(),
            solid: Premul.{ r: 0.0, g: 0.0, b: 0.0, a: 0.0 },
            lut: [Premul.{ r: 0.0, g: 0.0, b: 0.0, a: 0.0 }; 256],
        })!;
        defer mem.destroy(sh);
        make_shader(&paint, self.transform, sh);
        let mut fc = FillCtx.{ cv: self, sh: sh };
        rasterize(&self.poly, self.img.width, self.img.height, rule, &mut fc, fill_row)!;
    }
}

struct MaskCtx {
    mask: []mut f32,
    w: usize,
    aa: bool,
}

fn mask_row(m: *mut MaskCtx, x0: usize, y: usize, cov: []f32) void {
    for i in @as(usize, 0)..cov.len {
        let mut c = cov[i];
        if not m.aa {
            c = threshold(c);
        }
        m.mask[(y * m.w) + x0 + i] = c;
    }
}

fn threshold(c: f32) f32 {
    if c >= 0.5 {
        return 1.0;
    }
    return 0.0;
}

struct FillCtx {
    cv: *mut Canvas,
    sh: *mut Shader,
}

fn fill_row(fc: *mut FillCtx, x0: usize, y: usize, cov: []f32) void {
    let cv = fc.cv;
    let img = cv.img;
    let has_clip = cv.clip.len > 0;
    // an opaque solid colour drawn over or in place of 8-bit pixels: a
    // fully covered pixel just takes its bytes (written once the slow way)
    let fast = (fc.sh.paint.kind == PaintKind.SOLID) and (fc.sh.solid.a >= 1.0) and
               ((cv.op == Op.OVER) or (cv.op == Op.SRC)) and (img.depth == image.Depth.U8) and (cv.alpha >= 1.0);
    let ch = img.channels;
    let mut tpl: [4]u8 = [0; 4];
    let mut have_tpl = false;
    for i in @as(usize, 0)..cov.len {
        let mut c = cov[i];
        if fast and (c >= 1.0) and (not has_clip or (cv.clip[(y * img.width) + x0 + i] >= 1.0)) {
            let o = ((y * img.width) + x0 + i) * ch;
            if have_tpl {
                @mem_copy(img.data[o..o + ch], tpl[..ch]);
            } else {
                write_px(img, x0 + i, y, fc.sh.solid);
                @mem_copy(tpl[..ch], img.data[o..o + ch]);
                have_tpl = true;
            }
            continue;
        }
        if not cv.antialias {
            c = threshold(c);
        }
        let x = x0 + i;
        if has_clip {
            c = c * cv.clip[(y * img.width) + x];
        }
        c = c * cv.alpha;
        if c <= 0.0 {
            continue;
        }
        let s = shade(fc.sh, x, y);
        let d = read_px(img, x, y);
        let r = composite(cv.op, s, d);
        // coverage mixes the result with what was there
        let out = Premul.{
            r: d.r + ((r.r - d.r) * c),
            g: d.g + ((r.g - d.g) * c),
            b: d.b + ((r.b - d.b) * c),
            a: d.a + ((r.a - d.a) * c),
        };
        write_px(img, x, y, out);
    }
}

/// Makes every polygon's signed area positive (so that pieces of a stroke
/// add up under the nonzero rule).
fn orient_positive(p: *mut Poly) void {
    for i in @as(usize, 0)..p.starts.len {
        let r = p.range(i);
        let mut area: f64 = 0.0;
        for k in r[0]..r[1] {
            let mut j = k + 1;
            if j == r[1] {
                j = r[0];
            }
            area += (p.x(k) * p.y(j)) - (p.x(j) * p.y(k));
        }
        if area < 0.0 {
            // reverse the points
            let mut a = r[0];
            let mut b = r[1];
            while (a + 1) < b {
                b -= 1;
                let tx = p.x(a);
                let ty = p.y(a);
                p.pts.set(2 * a, p.x(b));
                p.pts.set((2 * a) + 1, p.y(b));
                p.pts.set(2 * b, tx);
                p.pts.set((2 * b) + 1, ty);
                a += 1;
            }
        }
    }
}

// -----------------------------------------------------------------------------
// Tests
// -----------------------------------------------------------------------------

fn coverage_sum(img: *image.Image) f64 {
    let mut s: f64 = 0.0;
    for y in @as(usize, 0)..img.height {
        for x in @as(usize, 0)..img.width {
            s += @as(f64, img.get(x, y).a);
        }
    }
    return s;
}

test "exact coverage" {
    let mut img = image.Image.init(40, 30, 4, image.Depth.F32)!;
    defer img.deinit();
    let mut cv = Canvas.init(&mut img);
    defer cv.deinit();
    let white = Paint.solid(color.Color.rgb(1.0, 1.0, 1.0));
    // a rectangle with fractional edges: its area, and partial pixels
    cv.fill_rect(2.25, 3.5, 10.5, 4.0, white)!;
    @expect(@abs(coverage_sum(&img) - 42.0) < 1.0e-3);
    @expect(@abs(img.get(2, 5).a - 0.75) < 1.0e-5);
    @expect(@abs(img.get(12, 3).a - 0.375) < 1.0e-5);
    @expect_eq(img.get(5, 5).a, 1.0);
    // a circle: area pi r^2
    cv.clear(color.Color.rgba(0.0, 0.0, 0.0, 0.0));
    let mut p = Path.init();
    defer p.deinit();
    p.circle(20.0, 15.0, 10.0)!;
    cv.fill(&p, white, FillRule.NONZERO)!;
    @expect(@abs(coverage_sum(&img) - (math.PI * 100.0)) < 2.2);  // chords: 2/3 tolerance x circumference
    // even-odd: a ring
    p.circle(20.0, 15.0, 5.0)!;
    cv.clear(color.Color.rgba(0.0, 0.0, 0.0, 0.0));
    cv.fill(&p, white, FillRule.EVEN_ODD)!;
    @expect(@abs(coverage_sum(&img) - (math.PI * 75.0)) < 3.3);
    @expect_eq(img.get(20, 15).a, 0.0);
    // nonzero with the inner circle the same way round: a disc
    cv.clear(color.Color.rgba(0.0, 0.0, 0.0, 0.0));
    cv.fill(&p, white, FillRule.NONZERO)!;
    @expect(@abs(coverage_sum(&img) - (math.PI * 100.0)) < 2.2);
}

test "strokes" {
    let mut img = image.Image.init(60, 40, 4, image.Depth.F32)!;
    defer img.deinit();
    let mut cv = Canvas.init(&mut img);
    defer cv.deinit();
    let white = Paint.solid(color.Color.rgb(1.0, 1.0, 1.0));
    // a horizontal line 4 wide and 30 long, butt caps: area 120
    cv.line(10.0, 20.0, 40.0, 20.0, white, Stroke.width(4.0))!;
    @expect(@abs(coverage_sum(&img) - 120.0) < 1.0e-3);
    // square caps add 2 at each end
    cv.clear(color.Color.rgba(0.0, 0.0, 0.0, 0.0));
    let mut st = Stroke.width(4.0);
    st.cap = Cap.SQUARE;
    cv.line(10.0, 20.0, 40.0, 20.0, white, st)!;
    @expect(@abs(coverage_sum(&img) - 136.0) < 1.0e-3);
    // a closed square outline, miter joins: 20x20 outer ring of width 2
    cv.clear(color.Color.rgba(0.0, 0.0, 0.0, 0.0));
    let mut p = Path.init();
    defer p.deinit();
    p.rect(10.0, 10.0, 18.0, 18.0)!;
    cv.stroke(&p, white, Stroke.width(2.0))!;
    @expect(@abs(coverage_sum(&img) - ((20.0 * 20.0) - (16.0 * 16.0))) < 1.0e-3);
    @expect_eq(img.get(9, 9).a, 1.0);  // the mitred corner
    // dashes: 30 long, 5 on 5 off: 15 of line, width 2
    cv.clear(color.Color.rgba(0.0, 0.0, 0.0, 0.0));
    let dashes: [2]f64 = [5.0, 5.0];
    let mut ds = Stroke.width(2.0);
    ds.dash = dashes[..];
    cv.line(10.0, 5.0, 40.0, 5.0, white, ds)!;
    @expect(@abs(coverage_sum(&img) - 30.0) < 1.0e-3);
    @expect_eq(img.get(16, 5).a, 0.0);
}

test "paint and compositing" {
    let mut img = image.Image.init(20, 10, 3, image.Depth.U8)!;
    defer img.deinit();
    let mut cv = Canvas.init(&mut img);
    defer cv.deinit();
    cv.clear(color.Color.rgb(0.0, 0.0, 1.0));
    // half-transparent red over blue: linear-light mix
    cv.fill_rect(0.0, 0.0, 10.0, 10.0, Paint.solid(color.Color.rgba(1.0, 0.0, 0.0, 0.5)))!;
    let c = img.get(5, 5);
    @expect(@abs(c.r - 0.5) < 0.01);
    @expect(@abs(c.b - 0.5) < 0.01);
    // a gradient from black to white across 20 pixels, in linear light
    let stops: [2]Stop = [
        Stop.{ offset: 0.0, color: color.Color.rgb(0.0, 0.0, 0.0) },
        Stop.{ offset: 1.0, color: color.Color.rgb(1.0, 1.0, 1.0) },
    ];
    cv.fill_rect(0.0, 0.0, 20.0, 10.0, Paint.linear(0.0, 0.0, 20.0, 0.0, stops[..]))!;
    @expect(@abs(img.get(9, 0).r - 0.475) < 0.01);
    // multiply by grey 0.5 halves it
    cv.op = Op.MULTIPLY;
    cv.fill_rect(0.0, 0.0, 20.0, 10.0, Paint.solid(color.Color.rgb(0.5, 0.5, 0.5)))!;
    @expect(@abs(img.get(19, 0).r - 0.4875) < 0.01);
    // a clip limits drawing
    cv.op = Op.OVER;
    let mut p = Path.init();
    defer p.deinit();
    p.rect(0.0, 0.0, 5.0, 10.0)!;
    cv.save()!;
    cv.clip_to(&p, FillRule.NONZERO)!;
    cv.fill_rect(0.0, 0.0, 20.0, 10.0, Paint.solid(color.Color.rgb(0.0, 1.0, 0.0)))!;
    cv.restore();
    @expect_eq(img.get(2, 2).to_rgba8().to_u32(), 0x00ff00ff);
    @expect(img.get(12, 2).g < 0.9);
    // transforms: a unit square scaled by 4 and moved
    cv.clear(color.Color.rgb(0.0, 0.0, 0.0));
    cv.translate(10.0, 2.0);
    cv.scale(4.0, 4.0);
    cv.fill_rect(0.0, 0.0, 1.0, 1.0, Paint.solid(color.Color.rgb(1.0, 1.0, 1.0)))!;
    @expect_eq(img.get(11, 3).to_rgba8().to_u32(), 0xffffffff);
    @expect_eq(img.get(14, 6).to_rgba8().to_u32(), 0x000000ff);
}
