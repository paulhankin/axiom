// expect-error: E0822
enum E: u8 {
    A,
}

fn f(e: E) u32 {
    return @int_cast(u32, e);
}
