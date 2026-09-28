// Arrays, slices, for-in, @mem_copy, @cast_slice, strings.
import "../lib/io.ph" as io;

fn total(xs: []u32) u32 {
    let mut t: u32 = 0;
    for x in xs {
        t += x;
    }
    return t;
}

fn fill(xs: []mut u32, v: u32) void {
    for i in 0..xs.len {
        xs[i] = v + @int_cast(u32, i);
    }
}

export fn run() void {
    let mut a: [8]u32 = [0; 8];
    fill(a[2..6], 10);
    io.int(@as(i64, total(a[..])));  // expect: 46
    io.int(@as(i64, a[3]));          // expect: 11
    let s = a[2..];
    io.uint(@as(u64, s.len));  // expect: 6
    let mut b: [4]u32 = [9, 9, 9, 9];
    @mem_copy(b[..], a[2..6]);
    io.int(@as(i64, b[0] + b[3]));  // expect: 23
    let bytes = @cast_slice(u8, b[0..1]);
    io.uint(@as(u64, bytes.len));  // expect: 4
    io.int(@as(i64, bytes[0]));    // expect: 10
    let msg = "hello, world";
    io.str(msg[7..]);  // expect: world
    io.boolean(@mem_eq(msg[0..5], "hello"));  // expect: true
    io.boolean(@mem_eq(msg[0..5], "help!"));  // expect: false
    let mut grid: [2][3]u8 = [[1, 2, 3], [4, 5, 6]];
    grid[1][2] = 60;
    io.int(@as(i64, grid[1][2] + grid[0][0]));  // expect: 61
    let p = &mut grid;
    p[0][1] = 20;
    io.int(@as(i64, grid[0][1]));  // expect: 20
    let mut buf: [5]u8 = [0; 5];
    @mem_set(buf[1..4], 7);
    io.int(@as(i64, buf[0] + buf[1] + buf[3] + buf[4]));  // expect: 14
}
