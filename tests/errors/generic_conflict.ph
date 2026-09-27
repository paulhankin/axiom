// expect-error: E0464
fn max[T: Ord](a: T, b: T) T {
    if a > b {
        return a;
    }
    return b;
}
fn f(a: u32, b: u64) u64 {
    return max(a, b);
}
