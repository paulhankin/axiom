// Globals, constants, function pointers in globals.
import "../lib/io.ph" as io;

const N: usize = 4;
const HALF: f64 = 0.5;
let TABLE: [N]i32 = [10, 20, 30, 40];
let mut counter: u32 = 0;
let GREETING: []u8 = "hi";

struct Op {
    name: []u8,
    f: fn(i32, i32) i32,
}

fn add(a: i32, b: i32) i32 {
    return a + b;
}
fn mul(a: i32, b: i32) i32 {
    return a * b;
}

let OPS: [2]Op = [Op.{ name: "add", f: add }, Op.{ name: "mul", f: mul }];

fn bump() u32 {
    counter += 1;
    return counter;
}

export fn run() void {
    io.int(@as(i64, TABLE[N - 1]));  // expect: 40
    io.float(HALF * 3.0);            // expect: 1.5
    _ = bump();
    _ = bump();
    io.int(@as(i64, bump()));  // expect: 3
    io.str(GREETING);          // expect: hi
    for op in OPS {
        io.str(op.name);
        io.int(@as(i64, op.f(6, 7)));
    }
    // expect: add
    // expect: 13
    // expect: mul
    // expect: 42
    let g = &mut counter;
    g.* = 100;
    io.int(@as(i64, counter));  // expect: 100
}
