// expect-error: E0620
fn g() u32 {
    return 1;
}
fn f() void {
    g();
}
