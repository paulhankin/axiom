// @enum_from_int traps on a value that is not a member (spec §5.14).
// expect-trap
// expect: B
import "../lib/io.ph" as io;

enum E: u8 {
    A = 1,
    B = 2,
}

fn name(e: E) []u8 {
    match e {
        .A => { return "A"; }
        .B => { return "B"; }
    }
}

export fn run() void {
    let mut n: u8 = 2;
    io.str(name(@enum_from_int(E, n)));
    n = 3;
    io.str(name(@enum_from_int(E, n)));
}
