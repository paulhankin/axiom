// Checked 64-bit multiplication at the edges of its range: constant factors
// (compared against a limit) and variable ones (the machine's multiply).
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
    io.uint(us[5] * 3);                          // expect: 18446744073709551615
    io.uint(umul(us[5], us[3]));                 // expect: 18446744073709551615
    io.uint(umul(us[4], us[4] + 2));             // expect: 18446744073709551615
    io.uint(umul(us[0], 0xFFFF_FFFF_FFFF_FFFF)); // expect: 0
    io.uint(us[1] * 0xFFFF_FFFF_FFFF_FFFF);      // expect: 18446744073709551615
    io.int(ss[5] * 2);                           // expect: 9223372036854775806
    io.int(smul(ss[5], ss[3]));                  // expect: 9223372036854775806
    io.int(smul(ss[5] + 1, -ss[3]));             // expect: -9223372036854775808
    io.int(smul(ss[2], 0x7FFF_FFFF_FFFF_FFFF));  // expect: -9223372036854775807
    io.int(ss[3] * -0x4000_0000_0000_0000);      // expect: -9223372036854775808
    io.int(smul(ss[0], -0x7FFF_FFFF_FFFF_FFFF - 1)); // expect: 0
    io.int(ss[1] * (-0x7FFF_FFFF_FFFF_FFFF - 1));  // expect: -9223372036854775808
    io.int(ss[5] * -2);                          // expect: -9223372036854775806
    io.int(smul(ss[5], ss[4]));                  // expect: -9223372036854775806
}
