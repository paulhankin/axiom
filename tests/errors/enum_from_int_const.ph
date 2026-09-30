// expect-error: E0826
enum E: u8 {
    A,
    B = 5,
}

fn f() E {
    return @enum_from_int(E, 3);
}
