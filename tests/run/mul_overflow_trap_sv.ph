// A checked 64-bit multiplication that overflows traps.
// expect-trap
import "../lib/io.ph" as io;

let mut us: [6]u64 = [0, 1, 2, 3, 0xFFFF_FFFF, 0x5555_5555_5555_5555];
let mut ss: [6]i64 = [0, 1, -1, 2, -2, 0x3FFF_FFFF_FFFF_FFFF];

fn umul(a: u64, b: u64) u64 {
    return a * b;
}

fn smul(a: i64, b: i64) i64 {
    return a * b;
}

export fn run() void {
    io.int(smul(ss[2], -0x7FFF_FFFF_FFFF_FFFF - 1));
}
