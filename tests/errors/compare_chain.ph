// expect-error: E0150
fn f(a: u32, b: u32, c: u32) bool {
    return a < b < c;
}
