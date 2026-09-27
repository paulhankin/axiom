// Programs that optimizations must not break: aliasing through pointers,
// stack slots whose address escapes, aggregate copies, facts from branch
// conditions, comparisons at the edges of their range, inlining.
import "../lib/io.ph" as io;

struct Pair {
    a: i64,
    b: i64,
}

struct Rec {
    p: Pair,
    flag: bool,
    tag: u8,
    n: u32,
    s: []u8,
}

let mut g_counter: i64 = 0;
let mut g_ptr: *mut i64 = @ptr_from_int(*mut i64, 0);

fn bump() void {
    g_counter += 1;
    if @int_from_ptr(g_ptr) != 0 {
        g_ptr.* += 100;
    }
}

// Two pointers that may be the same.
fn alias(p: *mut Pair, q: *mut Pair) i64 {
    p.a = 1;
    q.a = 2;
    return p.a;
}

// A load must not be reused across a call that writes the location.
fn across_call() i64 {
    let mut x: i64 = 5;
    g_ptr = &mut x;
    let before = x;
    bump();
    let after = x;
    g_ptr = @ptr_from_int(*mut i64, 0);
    return (before * 1000) + after;
}

fn read_global_twice() i64 {
    let a = g_counter;
    bump();
    return a + g_counter;
}

fn set_via(p: *mut i64, v: i64) void {
    p.* = v;
}

// The address of a local escapes into a call.
fn escaping_local() i64 {
    let mut x: i64 = 1;
    set_via(&mut x, 42);
    return x;
}

fn swap_fields(r: Pair) Pair {
    return Pair.{ a: r.b, b: r.a };
}

// Copies of aggregates are independent of their source.
fn copies() i64 {
    let mut r = Rec.{ p: Pair.{ a: 3, b: 4 }, flag: true, tag: 7, n: 9, s: "hello" };
    let c = r;
    r.p.a = 30;
    r.s = r.s[1..3];
    let d = swap_fields(c.p);
    let mut t: i64 = 0;
    if c.flag {
        t = 1;
    }
    return (c.p.a * 100000) + (d.a * 10000) + (r.p.a * 100) + (@as(i64, c.tag) * 10) + @int_cast(i64, r.s.len) + t;
}

// Bytes of a struct written through a byte pointer.
fn bytes_of() u64 {
    let mut p = Pair.{ a: 0, b: 0 };
    let bp = @ptr_cast(*mut u8, &mut p);
    let s = @slice_from_ptr(bp, 16);
    s[0] = 1;
    s[9] = 2;
    return @bit_cast(u64, p.a) + @bit_cast(u64, p.b);
}

fn classify(x: u32, n: u32) i64 {
    if x < n {
        if x < n {
            if n > x {
                return 1;
            }
            return 99;
        }
        return 98;
    }
    if x >= n {
        if x == 7 {
            if x == 7 {
                return 7;
            }
            return 97;
        }
        return 2;
    }
    return 96;
}

fn edges(x: u8, y: u64) i64 {
    let mut r: i64 = 0;
    if x < 1 {
        r += 1;
    }
    if @as(u32, x) == 255 {
        r += 10;
    }
    if @as(u32, x) == 256 {
        r += 1000;
    }
    if (y +% 1) < y {
        r += 100;
    }
    if (y -% 5) > y {
        r += 10000;
    }
    if (y +% 3) <= y {
        r += 100000;
    }
    if y < (y +% 2) {
        r += 1000000;
    }
    return r;
}

fn fib(n: u32) u64 {
    if n < 2 {
        return @as(u64, n);
    }
    return fib(n - 1) + fib(n - 2);
}

fn loop_store(xs: []mut i64) i64 {
    // the length is read again after stores through the slice
    let mut t: i64 = 0;
    for i in 0..xs.len {
        xs[i] = @int_cast(i64, i) * 2;
        t += xs[i];
    }
    return t;
}

fn sel(c: bool, a: i64, b: i64) i64 {
    let x = c;
    if not x {
        return b;
    }
    return a;
}

fn modify_in_loop(p: *mut Pair) i64 {
    let mut t: i64 = 0;
    let mut i: i64 = 0;
    while i < 4 {
        t += p.a;
        p.a += 1;
        i += 1;
    }
    return t;
}

error { Neg }

// The first test of the loop is known on entry (jump threading peels it);
// values flow around the back edge.
fn digits(x0: u64) u64 {
    let mut buf: [24]u8 = [0; 24];
    let mut i = buf.len;
    let mut v = x0;
    loop {
        i -= 1;
        buf[i] = @truncate(u8, v % 10) + '0';
        v = v / 10;
        if v == 0 {
            break;
        }
    }
    let mut s: u64 = 0;
    for k in i..buf.len {
        s = (s * 10) + @as(u64, buf[k] - '0');
    }
    return s + (@as(u64, i) * 1000000000);
}

fn leading_spaces(s: []u8) u64 {
    let mut i: usize = 0;
    let mut n: u64 = 0;
    while (i < s.len) and (s[i] == ' ') {
        i += 1;
        n += 1;
    }
    return n;
}

fn checked(x: i64) !i64 {
    if x < 0 {
        return error.Neg;
    }
    return x * 2;
}

fn use_checked(x: i64) i64 {
    let a = checked(x) catch -1;
    return a + 1;
}

export fn run() void {
    io.uint(digits(343073));                    // expect: 18000343073
    io.uint(digits(0));                         // expect: 23000000000
    io.uint(leading_spaces("   ab "));          // expect: 3
    io.uint(leading_spaces("    "));            // expect: 4
    io.uint(leading_spaces(""));                // expect: 0
    io.int(use_checked(5));                     // expect: 11
    io.int(use_checked(-3));                    // expect: 0
    let mut pa = Pair.{ a: 0, b: 0 };
    io.int(alias(&mut pa, &mut pa));            // expect: 2
    let mut pb = Pair.{ a: 0, b: 0 };
    io.int(alias(&mut pa, &mut pb));            // expect: 1
    io.int(across_call());                      // expect: 5105
    io.int(read_global_twice());                // expect: 3
    io.int(escaping_local());                   // expect: 42
    io.int(copies());                           // expect: 343073
    io.uint(bytes_of());                        // expect: 513
    io.int(classify(3, 5));                     // expect: 1
    io.int(classify(7, 5));                     // expect: 7
    io.int(classify(8, 5));                     // expect: 2
    io.int(edges(0, 0xFFFF_FFFF_FFFF_FFFF));    // expect: 100101
    io.int(edges(255, 3));                      // expect: 1010010
    io.int(edges(4, 10));                       // expect: 1000000
    io.uint(fib(20));                           // expect: 6765
    let mut arr: [5]i64 = [0; 5];
    io.int(loop_store(arr[..]));                // expect: 20
    io.int(arr[4]);                             // expect: 8
    io.int(sel(true, 1, 2));                    // expect: 1
    io.int(sel(false, 1, 2));                   // expect: 2
    let mut pc = Pair.{ a: 10, b: 0 };
    io.int(modify_in_loop(&mut pc));            // expect: 46
    io.int(pc.a);                               // expect: 14
}
