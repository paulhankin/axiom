// expect-error: E0823
fn f(p: *u64) u32 {
    return @atomic_wait(p, 0, -1);
}
