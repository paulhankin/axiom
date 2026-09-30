// expect-error: E0412
enum Align: u8 {
    START,
    END,
}

fn f(a: Align) bool {
    return a == 0;
}
