// expect-error: E0706
fn f(p: *u32) void {
    _ = @atomic_add(p, 1);
}
