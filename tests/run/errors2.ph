// match on errors; !void; checked arithmetic
import "../lib/io.ph" as io;

error { A, B }

fn f(n: u32) !void {
    if n == 1 {
        return error.A;
    }
    if n == 2 {
        return error.B;
    }
}

fn describe(n: u32) []u8 {
    f(n) catch |e| {
        match e {
            error.A => { return "A"; }
            error.B => { return "B"; }
            else    => { return "?"; }
        }
    };
    return "ok";
}

export fn run() void {
    io.str(describe(0));  // expect: ok
    io.str(describe(1));  // expect: A
    io.str(describe(2));  // expect: B
    let big: u32 = 4000000000;
    let s = @add_checked(big, big) catch |e| {
        io.str(@error_name(e));  // expect: Overflow
        return;
    };
    io.int(@as(i64, s));
}
