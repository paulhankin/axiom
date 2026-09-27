// 64-bit multiply overflow detection (no 128-bit multiply on wasm32).
import "../lib/io.ph" as io;

fn show(r: !u64) void {
    let v = r catch |e| {
        io.str(@error_name(e));
        return;
    };
    io.uint(v);
}

fn shows(r: !i64) void {
    let v = r catch |e| {
        io.str(@error_name(e));
        return;
    };
    io.int(v);
}

export fn run() void {
    let a: u64 = 4294967296;
    show(@mul_checked(a, a - 1));            // expect: 18446744069414584320
    show(@mul_checked(a, a));                // expect: Overflow
    let m: u64 = @max_value(u64);
    show(@mul_checked(m, 1));                // expect: 18446744073709551615
    show(@mul_checked(m, 2));                // expect: Overflow
    let s: i64 = -3037000499;
    shows(@mul_checked(s, s));               // expect: 9223372030926249001
    shows(@mul_checked(s, 3037000500));      // expect: -9223372033963249500
    shows(@mul_checked(s, -3037000500));     // expect: 9223372033963249500
    shows(@mul_checked(@min_value(i64), -1)); // expect: Overflow
    shows(@mul_checked(-1, @min_value(i64))); // expect: Overflow
    shows(@mul_checked(@min_value(i64), 1));  // expect: -9223372036854775808
    shows(@mul_checked(@as(i64, 0), @min_value(i64))); // expect: 0
    let big: u64 = 123456789012;
    io.uint(big *% big);                      // expect: 4568148269394301328
    io.uint(@mul_sat(big, big));              // expect: 18446744073709551615
}
