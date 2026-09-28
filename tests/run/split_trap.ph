// A loop whose bounds check fails part way through: the iterations before
// the failing index all happen, in order, then it traps (the optimizer runs
// such loops in two parts: up to the smallest bound without checks, then the
// rest with them).
// expect-trap
import "../lib/io.ph" as io;

let mut g_n: usize = 2;

fn show(xs: []u64, n: usize) u64 {
    let mut t: u64 = 0;
    for i in 0..n {
        t += xs[i];
        io.uint(t);
    }
    return t;
}

export fn run() void {
    let xs: [3]u64 = [1, 2, 3];
    io.uint(show(xs[..], g_n) * 100);   // expect: 1
                                        // expect: 3
                                        // expect: 300
    g_n = 5;
    _ = show(xs[..], g_n);              // expect: 1
                                        // expect: 3
                                        // expect: 6
}
