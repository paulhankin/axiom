// expect-error: E0610
error { Bad }
fn g() !u32 {
    return error.Bad;
}
fn f() u32 {
    return g()!;
}
