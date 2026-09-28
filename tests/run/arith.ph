// Integer arithmetic semantics (spec §7.6).
import "../lib/io.ph" as io;

export fn run() void {
    let a: i32 = -7;
    let b: i32 = 2;
    io.int(@as(i64, a / b));   // expect: -3
    io.int(@as(i64, a % b));   // expect: -1
    io.int(@as(i64, 7 % -2));  // expect: 1
    let m: i32 = @min_value(i32);
    io.int(@as(i64, m % -1));  // expect: 0
    let w: u8 = 250;
    io.int(@as(i64, w +% 10));   // expect: 4
    io.int(@as(i64, w -% 251));  // expect: 255
    io.int(@as(i64, w *% 2));    // expect: 244
    io.int(@as(i64, -%w));       // expect: 6

    // shifts have their mathematical meaning for every count
    let x: u32 = 0xFFFF_FFFF;
    let k: u32 = 40;
    io.uint(@as(u64, x << k));  // expect: 0
    io.uint(@as(u64, x >> k));  // expect: 0
    let neg: i32 = -8;
    io.int(@as(i64, neg >> k));  // expect: -1
    io.int(@as(i64, neg >> 1));  // expect: -4
    let one: u32 = 1;
    io.uint(@as(u64, one << 31));  // expect: 2147483648
    io.uint(@as(u64, one << 32));  // expect: 0
    let c8: u8 = 3;
    io.uint(@as(u64, x >> c8));  // expect: 536870911

    // precedence: * binds tighter than +, & tighter than ==
    io.int(@as(i64, 1 + 2 * 3));  // expect: 7
    let f: u32 = 6;
    io.boolean(f & 1 == 0);  // expect: true

    // constant folding
    const_test();
}

const K: u64 = 1 << 40;
const NEG: i16 = -32768;

fn const_test() void {
    io.uint(K);                // expect: 1099511627776
    io.int(@as(i64, NEG));     // expect: -32768
    io.uint(@max_value(u64));  // expect: 18446744073709551615
}
