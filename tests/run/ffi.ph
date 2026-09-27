// C FFI in both directions (spec §15.5); runs on wasm and native.
// link: ffi.c
// header: ffi_generated.h
import "../lib/io.ph" as io;
import "std/c" as c;

pub struct Point {
    x: i32,
    y: f64,
    flag: bool,
    id: u16,
}

extern "C" {
    fn c_add(a: i32, b: i32) i32;
    fn c_scale(x: f64, k: f64) f64;
    fn c_sum_bytes(p: *u8, n: usize) u32;
    fn c_neg8(x: i8) i8;
    fn c_hi16(x: u32) u16;
    fn c_is_even(x: i32) bool;
    fn c_sum_ints(n: i32, ...) i64;
    fn c_avg(n: i32, ...) f64;
    fn c_apply(f: fn(i32) i32, x: i32) i32;
    fn c_fill_point(p: *mut Point) void;
    fn c_calls_phalange(x: i32) i32;
    fn c_greeting() *u8;
    fn c_null() *mut void;
}

// Called from C by name.
export fn ph_triple(x: i32) i32 {
    return x * 3;
}

export fn ph_small(x: i8) i8 {
    return x;
}

export fn ph_point_sum(p: *Point) f64 {
    return @float_from_int(f64, @as(i64, p.x)) + p.y;
}

// Passed to C as a callback; needs no annotation.
fn plus_ten(x: i32) i32 {
    return x + 10;
}

export fn run() void {
    io.int(@as(i64, c_add(40, 2)));                    // expect: 42
    io.float(c_scale(1.5, 4.0));                       // expect: 6
    let bytes = "abc";
    io.uint(@as(u64, c_sum_bytes(bytes.ptr, bytes.len)));  // expect: 294
    io.int(@as(i64, c_neg8(-100)));                    // expect: 100
    io.uint(@as(u64, c_hi16(0xABCD_1234)));            // expect: 43981
    io.boolean(c_is_even(10));                         // expect: true
    io.int(c_sum_ints(3, @as(i64, 1), @as(i64, 2), @as(i64, 3000000000)));  // expect: 3000000003
    io.float(c_avg(2, 1.0, 2.0));                      // expect: 1.5
    io.int(@as(i64, c_apply(plus_ten, 5)));            // expect: 16
    let mut p = Point.{ x: 0, y: 0.0, flag: false, id: 0 };
    c_fill_point(&mut p);
    io.int(@as(i64, p.x));                             // expect: 7
    io.float(p.y);                                     // expect: 2.5
    io.boolean(p.flag);                                // expect: true
    io.uint(@as(u64, p.id));                           // expect: 65535
    io.float(ph_point_sum(&p));                        // expect: 9.5
    io.int(@as(i64, c_calls_phalange(5)));             // expect: 12
    io.str(c.from_cstr(c_greeting()));                 // expect: hello from C
    io.boolean(c.is_null_void(c_null()));              // expect: true
}
