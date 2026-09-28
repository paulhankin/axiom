// Loops, if/else chains, short-circuit logic, while, loop.
import "../lib/io.ph" as io;

fn collatz(n0: u64) u32 {
    let mut n = n0;
    let mut steps: u32 = 0;
    loop {
        if n == 1 {
            break;
        }
        if n % 2 == 0 {
            n = n / 2;
        } else {
            n = (3 * n) + 1;
        }
        steps += 1;
    }
    return steps;
}

fn sign(x: i32) []u8 {
    if x < 0 {
        return "neg";
    } else if x == 0 {
        return "zero";
    } else {
        return "pos";
    }
}

fn side(tag: []u8, v: bool) bool {
    io.str(tag);
    return v;
}

fn fib(n: u32) u64 {
    if n < 2 {
        return @as(u64, n);
    }
    return fib(n - 1) + fib(n - 2);
}

export fn run() void {
    io.int(@as(i64, collatz(27)));  // expect: 111
    io.str(sign(-5));               // expect: neg
    io.str(sign(0));                // expect: zero
    io.str(sign(9));                // expect: pos
    let r = side("a", false) and side("b", true);
    io.boolean(r);
    // expect: a
    // expect: false
    let r2 = side("c", true) or side("d", true);
    io.boolean(r2);
    // expect: c
    // expect: true
    io.uint(fib(30));  // expect: 832040
    let mut i: i32 = 10;
    while i > 0 {
        i -= 3;
    }
    io.int(@as(i64, i));  // expect: -2
}
