// Dense f64 matrix multiplication and a small n-body style struct update:
// floating point, nested loops, bounds checks on computed indices.
import "std/mem" as mem;
import "common.ph" as c;

struct Body {
    x: f64,
    y: f64,
    vx: f64,
    vy: f64,
    m: f64,
}

fn step(bs: []mut Body, dt: f64) void {
    for i in 0..bs.len {
        for j in (i + 1)..bs.len {
            let dx = bs[i].x - bs[j].x;
            let dy = bs[i].y - bs[j].y;
            let d2 = ((dx * dx) + (dy * dy)) + 0.01;
            let inv = 1.0 / (d2 * @sqrt(d2));
            bs[i].vx -= (dx * bs[j].m) * inv * dt;
            bs[i].vy -= (dy * bs[j].m) * inv * dt;
            bs[j].vx += (dx * bs[i].m) * inv * dt;
            bs[j].vy += (dy * bs[i].m) * inv * dt;
        }
    }
    for i in 0..bs.len {
        bs[i].x += bs[i].vx * dt;
        bs[i].y += bs[i].vy * dt;
    }
}

export fn main() i32 {
    let n: usize = 300;
    let a = mem.page_allocator.alloc.[f64](n * n) catch return 1;
    let b = mem.page_allocator.alloc.[f64](n * n) catch return 1;
    let r = mem.page_allocator.alloc.[f64](n * n) catch return 1;
    for i in 0..(n * n) {
        a[i] = @float_from_int(f64, i % 17) * 0.5;
        b[i] = @float_from_int(f64, i % 13) - 3.0;
    }
    for i in 0..n {
        for j in 0..n {
            let mut s: f64 = 0.0;
            for k in 0..n {
                s += a[(i * n) + k] * b[(k * n) + j];
            }
            r[(i * n) + j] = s;
        }
    }
    let mut sum: f64 = 0.0;
    for i in 0..(n * n) {
        sum += r[i];
    }
    let bs = mem.page_allocator.alloc.[Body](400) catch return 1;
    for i in 0..bs.len {
        let f = @float_from_int(f64, i);
        bs[i] = Body.{ x: f * 0.37, y: f * 0.11, vx: 0.0, vy: 0.0, m: 1.0 + (f * 0.001) };
    }
    for _s in 0..@as(usize, 40) {
        step(bs, 0.001);
    }
    for i in 0..bs.len {
        sum += bs[i].x + bs[i].y;
    }
    c.result("matmul", @int_from_float(u64, @abs(sum)));
    return 0;
}
