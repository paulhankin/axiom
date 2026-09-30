// expect-error: E0630
enum Align: u8 {
    START,
    END,
    LEFT,
}

fn f(a: Align) u8 {
    match a {
        .START => { return 1; }
        .END   => { return 2; }
    }
}
