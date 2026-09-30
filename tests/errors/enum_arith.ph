// expect-error: E0513
enum Align: u8 {
    START,
    END,
}

fn f(a: Align) Align {
    return a + Align.END;
}
