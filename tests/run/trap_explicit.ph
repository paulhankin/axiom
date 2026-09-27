// @trap() and catch-block divergence.
// expect-trap
// expect: start
import "../lib/io.ph" as io;

error { Nope }

fn fails() !u32 {
    return error.Nope;
}

export fn run() void {
    io.str("start");
    let x = fails() catch {
        @trap();
    };
    io.int(@as(i64, x));
}
