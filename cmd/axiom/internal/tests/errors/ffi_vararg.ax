// expect-error: E0824
extern "C" {
    fn printf(fmt: *u8, ...) i32;
}
fn f(x: f32) void {
    _ = printf("%f".ptr, x);
}
