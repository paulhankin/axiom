// expect-error: E0150
error { Bad }
fn g() !u32 {
    return 1;
}
fn f(a: u32) u32 {
    return a + g() catch 0;
}
