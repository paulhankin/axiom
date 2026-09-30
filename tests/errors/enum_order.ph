// expect-error: E0513
enum Align: u8 {
    START,
    END,
}

fn f(a: Align) bool {
    return a < Align.END;
}
