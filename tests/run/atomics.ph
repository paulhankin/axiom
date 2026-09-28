// Atomic builtins (single thread).
import "../lib/io.ph" as io;

export fn run() void {
    let mut x: u32 = 5;
    io.int(@as(i64, @atomic_add(&mut x, 3)));   // expect: 5
    io.int(@as(i64, @atomic_load(&x)));         // expect: 8
    io.int(@as(i64, @atomic_sub(&mut x, 10)));  // expect: 8
    io.uint(@as(u64, x));                       // expect: 4294967294
    @atomic_store(&mut x, 12);
    io.int(@as(i64, @atomic_and(&mut x, 10)));      // expect: 12
    io.int(@as(i64, @atomic_or(&mut x, 1)));        // expect: 8
    io.int(@as(i64, @atomic_xor(&mut x, 15)));      // expect: 9
    io.int(@as(i64, @atomic_xchg(&mut x, 100)));    // expect: 6
    io.int(@as(i64, @atomic_cas(&mut x, 7, 1)));    // expect: 100
    io.int(@as(i64, @atomic_cas(&mut x, 100, 1)));  // expect: 100
    io.int(@as(i64, x));                            // expect: 1
    let mut s: i64 = -5;
    _ = @atomic_min(&mut s, -9);
    _ = @atomic_max(&mut s, 3);
    io.int(s);  // expect: 3
    let mut b: u8 = 250;
    _ = @atomic_add(&mut b, 10);
    io.int(@as(i64, b));  // expect: 4
    @fence();
}
