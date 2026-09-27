// Division by zero traps in every mode.
// flags: --release
// expect-trap
import "../lib/io.ph" as io;

fn div(a: u32, b: u32) u32 {
    return a / b;
}

export fn run() void {
    io.int(@as(i64, div(10, 0)));
}
