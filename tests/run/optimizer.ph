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

// Loops that only fill memory (they become memset).
fn fill_bytes(xs: []mut u8, lo: usize, hi: usize, v: u8) u64 {
    let mut i = lo;
    while i < hi {
        xs[i] = v;
        i += 1;
    }
    let mut s: u64 = 0;
    for k in 0..xs.len {
        s = (s * 3) + @as(u64, xs[k]);
    }
    return s;
}

fn fill_words(xs: []mut u32, n: usize) u64 {
    for i in 0..n {
        xs[i] = 0xFFFF_FFFF;
    }
    let mut s: u64 = 0;
    for k in 0..xs.len {
        s = s + @as(u64, xs[k]);
    }
    return s;
}

// Loops with bounds checks the loop condition does not imply (run in two
// parts: without the checks up to the smallest bound, then with them).
fn copy_add(dst: []mut u32, src: []u32, lo: usize, hi: usize) u64 {
    let mut s: u64 = 0;
    let mut i = lo;
    while i < hi {
        dst[i] = src[i] + 1;
        s += @as(u64, dst[i]);
        i += 1;
    }
    return (s * 1000) + @as(u64, i);
}

// Stores in both arms of a choice (one store of a selected value).
fn store_either(xs: []mut i64, c: bool, v: i64) i64 {
    if xs.len < 3 {
        return -1;
    }
    if c {
        xs[1] = v;
    } else {
        xs[1] = v * 2;
    }
    if c {
        xs[0] = 7;
    } else {
        xs[2] = 7;
    }
    return (xs[0] * 100) + (xs[1] * 10) + xs[2];
}

// A choice whose result indexes the next load (kept as a branch).
fn heap_path(xs: []u32, n: usize) u64 {
    let mut root: usize = 0;
    let mut s: u64 = 0;
    loop {
        let child = (2 * root) + 1;
        if child >= n {
            return s + @as(u64, root);
        }
        let mut top = root;
        if xs[top] < xs[child] {
            top = child;
        }
        if ((child + 1) < n) and (xs[top] < xs[child + 1]) {
            top = child + 1;
        }
        if top == root {
            return s + @as(u64, root);
        }
        s = (s * 10) + @as(u64, xs[top]);
        root = top;
    }
}

export fn run() void {
    let src: [6]u32 = [1, 2, 3, 4, 5, 6];
    let mut dst: [4]u32 = [0; 4];
    io.uint(copy_add(dst[..], src[..], 1, 4));  // expect: 12004
    io.uint(copy_add(dst[..], src[..], 5, 2));  // expect: 5
    io.uint(copy_add(dst[..], src[..], 0, 0));  // expect: 0
    io.uint(@as(u64, dst[0]) + @as(u64, dst[3]));  // expect: 5
    let mut es: [3]i64 = [0; 3];
    io.int(store_either(es[..], true, 4));     // expect: 740
    io.int(store_either(es[..], false, 4));    // expect: 787
    let hp: [7]u32 = [1, 5, 9, 2, 3, 8, 4];
    io.uint(heap_path(hp[..], 7));             // expect: 11
    io.uint(heap_path(hp[..], 2));             // expect: 6

    io.uint(digits(343073));                    // expect: 18000343073
    io.uint(digits(0));                         // expect: 23000000000
    io.uint(leading_spaces("   ab "));          // expect: 3
    io.uint(leading_spaces("    "));            // expect: 4
    io.uint(leading_spaces(""));                // expect: 0
    io.int(use_checked(5));                     // expect: 11
    io.int(use_checked(-3));                    // expect: 0
    let mut bs: [8]u8 = [9; 8];
    io.uint(fill_bytes(bs[..], 2, 6, 1));       // expect: 26640
    io.uint(fill_bytes(bs[..], 5, 3, 7));       // expect: 26640
    io.uint(fill_bytes(bs[..], 0, 8, 0));       // expect: 0
    let mut ws: [4]u32 = [1, 2, 3, 4];
    io.uint(fill_words(ws[..], 2));             // expect: 8589934597
    io.uint(fill_words(ws[..], 0));             // expect: 8589934597
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
