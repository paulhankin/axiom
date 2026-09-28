// --release saturates instead of trapping.
// flags: --release
import "../lib/io.ph" as io;

export fn run() void {
    let x: u8 = 250;
    io.int(@as(i64, x + 10));  // expect: 255
    let y: i8 = -100;
    io.int(@as(i64, y - 100));  // expect: -128
    let z: i32 = 100000;
    io.int(@as(i64, z * z));  // expect: 2147483647
    let n: i32 = -100000;
    io.int(@as(i64, n * z));  // expect: -2147483648
    let m: i32 = @min_value(i32);
    io.int(@as(i64, m / -1));  // expect: 2147483647
    io.int(@as(i64, -m));      // expect: 2147483647
}
