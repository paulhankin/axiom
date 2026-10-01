// Stack exhaustion traps.
// wasm-only
// expect-trap
fn deep(n: u64) u64 {
    let mut buf: [64]u64 = [0; 64];
    buf[@int_cast(usize, n % 64)] = n;
    return deep(n + 1) + buf[0];
}

export fn run() void {
    _ = deep(0);
}
