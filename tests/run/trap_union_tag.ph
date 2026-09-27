// An out-of-range union tag traps at match (spec §5.10).
// expect-trap
// expect: A
import "../lib/io.ph" as io;

union U { A, B }

fn name(u: U) []u8 {
    match u {
        .A => { return "A"; }
        .B => { return "B"; }
    }
}

export fn run() void {
    let mut u = U.A;
    io.str(name(u));
    let p = @ptr_cast(*mut u8, &mut u);
    p.* = 7;
    io.str(name(u));
}
