// expect-error: E0632
enum Align: u8 {
    START,
    END,
}

fn f(a: Align) u8 {
    match a {
        .MIDDLE => { return 1; }
        else    => { return 2; }
    }
}
