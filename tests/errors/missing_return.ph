// expect-error: E0601
fn f(x: u32) u32 {
    if x > 1 {
        return 1;
    }
}
