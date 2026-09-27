// =============================================================================
// A small Phalange library used from C (see main.c).
//
//   phalange build stats.ph --target=native --emit=obj --emit-header=stats.h -o stats.o
//   cc main.c stats.o -o demo && ./demo
// =============================================================================

import "std/vec" as vec;

pub struct Summary {
    count: usize,
    min: f64,
    max: f64,
    mean: f64,
}

fn less(a: f64, b: f64) bool {
    return a < b;
}

/// Fills `out` with statistics of xs[0..n]. Returns false if n == 0.
export fn stats_summarize(xs: *f64, n: usize, out: *mut Summary) bool {
    if n == 0 {
        return false;
    }
    let data = @slice_from_ptr(xs, n);
    let mut lo = data[0];
    let mut hi = data[0];
    let mut sum: f64 = 0.0;
    for x in data {
        lo = @min(lo, x);
        hi = @max(hi, x);
        sum += x;
    }
    out.* = Summary.{ count: n, min: lo, max: hi, mean: sum / @float_from_int(f64, @as(u64, n)) };
    return true;
}

/// Sorts xs[0..n] in place, calling `progress` once at the end with n.
export fn stats_sort(xs: *mut f64, n: usize, progress: fn(usize) void) void {
    vec.sort(@slice_from_ptr(xs, n), less);
    progress(n);
}
