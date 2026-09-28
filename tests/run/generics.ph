// Monomorphized generics (spec §10).
import "../lib/io.ph" as io;
import "std/mem" as mem;

fn max[T: Ord](a: T, b: T) T {
    if a > b {
        return a;
    }
    return b;
}

fn sum[T: Num, const N: usize](xs: [N]T) T {
    let mut total: T = 0;
    for x in xs {
        total += x;
    }
    return total;
}

fn first_or[T](xs: []T, fallback: T) T {
    if xs.len == 0 {
        return fallback;
    }
    return xs[0];
}

struct Pair[A, B] {
    a: A,
    b: B,

    pub fn swap(self: Self) Pair.[B, A] {
        return Pair.[B, A].{ a: self.b, b: self.a };
    }
}

pub struct Stack[T] {
    items: []mut T,
    len: usize,

    pub fn init(cap: usize) !Self {
        let items = mem.alloc.[T](cap)!;
        return Self.{ items: items, len: 0 };
    }

    pub fn push(self: *mut Self, x: T) !void {
        if self.len == self.items.len {
            return error.OutOfMemory;
        }
        self.items[self.len] = x;
        self.len += 1;
    }

    pub fn pop(self: *mut Self) T {
        self.len -= 1;
        return self.items[self.len];
    }
}

fn insertion_sort[T](items: []mut T, less: fn(T, T) bool) void {
    for i in 1..items.len {
        let key = items[i];
        let mut j = i;
        while (j > 0) and less(key, items[j - 1]) {
            items[j] = items[j - 1];
            j -= 1;
        }
        items[j] = key;
    }
}

fn desc(a: i32, b: i32) bool {
    return a > b;
}

export fn run() void {
    io.int(@as(i64, max(3, @as(i32, 9))));  // expect: 9
    io.float(max.[f64](1.5, -2.0));         // expect: 1.5
    let arr: [4]u16 = [1, 2, 3, 4];
    io.int(@as(i64, sum(arr)));  // expect: 10
    let fa: [2]f64 = [0.25, 0.5];
    io.float(sum(fa));  // expect: 0.75
    let empty = arr[0..0];
    io.int(@as(i64, first_or(empty, @as(u16, 77))));  // expect: 77
    let p = Pair.[u8, bool].{ a: 5, b: true };
    let q = p.swap();
    io.boolean(q.a);        // expect: true
    io.int(@as(i64, q.b));  // expect: 5

    let mut st = Stack.[i64].init(4) catch return;
    defer mem.free(st.items);
    st.push(10) catch return;
    st.push(20) catch return;
    io.int(st.pop());  // expect: 20
    io.int(st.pop());  // expect: 10

    let mut xs: [5]i32 = [3, -1, 4, 1, 5];
    insertion_sort(xs[..], desc);
    for x in xs {
        io.int(@as(i64, x));
    }
    // expect: 5
    // expect: 4
    // expect: 3
    // expect: 1
    // expect: -1
    repeat_demo();
}

// A const generic parameter as an array repeat count.
fn zeros[const N: usize]() [N]u32 {
    return [0; N];
}

fn sum_zeros_plus[const N: usize](k: u32) u32 {
    let mut a: [N]u32 = [k; N];
    a[0] = 1;
    let mut s: u32 = 0;
    for x in a {
        s += x;
    }
    return s;
}

fn repeat_demo() void {
    let z = zeros.[5]();
    io.int(@int_cast(i64, z.len));            // expect: 5
    io.int(@as(i64, sum_zeros_plus.[4](3)));  // expect: 10
}
