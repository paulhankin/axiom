// expect-error: E0702
struct C {
    n: u32,
    pub fn inc(self: *mut C) void {
        self.n += 1;
    }
}
fn f(c: *C) void {
    c.inc();
}
