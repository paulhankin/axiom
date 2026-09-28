// Float semantics and conversions.
import "../lib/io.ph" as io;

export fn run() void {
    let zero: f64 = 0.0;
    let nan = zero / zero;
    io.float(nan);           // expect: NaN
    io.boolean(nan == nan);  // expect: false
    io.boolean(nan != nan);  // expect: true
    io.float(1.0 / zero);    // expect: Infinity
    io.int(@as(i64, @int_from_float(i32, nan)));     // expect: 0
    io.int(@as(i64, @int_from_float(i32, 1.0e20)));  // expect: 2147483647
    io.int(@as(i64, @int_from_float(u8, -5.0)));     // expect: 0
    io.int(@int_from_float(i64, -2.9));              // expect: -2
    io.float(@min(1.0, nan));                        // expect: NaN
    io.float(@float_from_int(f64, @as(i64, -3)));    // expect: -3
    let f: f32 = 0.1;
    io.float(@as(f64, f));                      // expect: 0.10000000149011612
    io.float(@as(f64, @float_cast(f32, 0.1)));  // expect: 0.10000000149011612
    io.float(@nearest(2.5));                    // expect: 2
    io.float(@floor(-1.5));                     // expect: -2
    io.float(@rem(7.5, 2.0));                   // expect: 1.5
    io.float(@abs(-0.25));                      // expect: 0.25
    io.float(@fma(2.0, 3.0, 1.0));              // expect: 7
    let bits = @bit_cast(u64, @as(f64, 1.0));
    io.uint(bits);  // expect: 4607182418800017408
}
