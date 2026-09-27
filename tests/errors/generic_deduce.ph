// expect-error: E0463
fn id[T: Int](a: T) T {
    return a;
}
fn f() u32 {
    return id(5);
}
