// expect-error: E0462
fn add[T: Num](a: T, b: T) T {
    return a + b;
}
fn f() bool {
    return add(true, false);
}
