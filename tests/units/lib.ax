// A library module for the separate-compilation test (tests/run.mjs, "units").
error { Empty, TooBig }

pub struct Pair[T] {
    a: T,
    b: T,

    pub fn second(self: Self) T {
        return self.b;
    }
}

pub fn largest(xs: []u32) !u32 {
    if xs.len == 0 {
        return error.Empty;
    }
    let mut m = xs[0];
    for x in xs {
        if x > m {
            m = x;
        }
    }
    if m > 1000 {
        return error.TooBig;
    }
    return m;
}

pub fn describe(e: error) []u8 {
    match e {
        error.Empty  => { return "empty"; }
        error.TooBig => { return "too big"; }
        else         => { return @error_name(e); }
    }
}
