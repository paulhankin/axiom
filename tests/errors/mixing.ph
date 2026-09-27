// expect-error: E0150
fn f(a: u32, b: u32) u32 {
    return a + b & 1;
}
