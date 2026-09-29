// =============================================================================
// std/heap.ph - priority queues: binary heaps.
//
//     let mut h = heap.Heap.[u32].init(less_u32);   // a min-heap
//     defer h.deinit();
//     h.push(5)!;
//     h.push(1)!;
//     let smallest = h.pop()!;                      // 1
//
// The heap keeps its least element (by `less`) on top; give a "greater"
// function for a max-heap.
// =============================================================================

import "vec.ph" as vec;

error { Empty, OutOfMemory }

pub struct Heap[T] {
    items: vec.Vec.[T],
    less: fn(T, T) bool,

    pub fn init(less: fn(T, T) bool) Self {
        return Self.{ items: vec.Vec.[T].init(), less: less };
    }

    /// A heap of the elements of xs, built in linear time.
    pub fn from_slice(xs: []T, less: fn(T, T) bool) !Self {
        let mut h = Self.init(less);
        h.items.extend(xs)!;
        let n = h.items.len;
        let mut i = n / 2;
        while i > 0 {
            i -= 1;
            h.down(i);
        }
        return h;
    }

    pub fn deinit(self: *mut Self) void {
        self.items.deinit();
    }

    pub fn len(self: *Self) usize {
        return self.items.len;
    }

    pub fn clear(self: *mut Self) void {
        self.items.clear();
    }

    pub fn push(self: *mut Self, x: T) !void {
        self.items.push(x)!;
        self.up(self.items.len - 1);
    }

    /// The least element, without removing it.
    pub fn peek(self: *Self) !T {
        if self.items.len == 0 {
            return error.Empty;
        }
        return self.items.get(0);
    }

    /// Removes and returns the least element.
    pub fn pop(self: *mut Self) !T {
        if self.items.len == 0 {
            return error.Empty;
        }
        let a = self.items.items();
        let top = a[0];
        let last = self.items.pop()!;
        if self.items.len > 0 {
            self.items.set(0, last);
            self.down(0);
        }
        return top;
    }

    /// Pops the least element and pushes x in one step (x may come back).
    pub fn replace(self: *mut Self, x: T) !T {
        if self.items.len == 0 {
            return error.Empty;
        }
        let top = self.items.get(0);
        self.items.set(0, x);
        self.down(0);
        return top;
    }

    fn up(self: *mut Self, i0: usize) void {
        let a = self.items.items();
        let x = a[i0];
        let mut i = i0;
        while i > 0 {
            let p = (i - 1) / 2;
            if not self.less(x, a[p]) {
                break;
            }
            a[i] = a[p];
            i = p;
        }
        a[i] = x;
    }

    fn down(self: *mut Self, i0: usize) void {
        let a = self.items.items();
        let n = a.len;
        let x = a[i0];
        let mut i = i0;
        loop {
            let l = (2 * i) + 1;
            if l >= n {
                break;
            }
            let mut c = l;
            if ((l + 1) < n) and self.less(a[l + 1], a[l]) {
                c = l + 1;
            }
            if not self.less(a[c], x) {
                break;
            }
            a[i] = a[c];
            i = c;
        }
        a[i] = x;
    }
}

// -----------------------------------------------------------------------------
// Tests
// -----------------------------------------------------------------------------

fn lt(a: u64, b: u64) bool {
    return a < b;
}

fn gt(a: u64, b: u64) bool {
    return a > b;
}

fn is_empty_error(r: !u64) bool {
    _ = r catch |e| {
        return e == error.Empty;
    };
    return false;
}

test "heaps" {
    let mut h = Heap.[u64].init(lt);
    defer h.deinit();
    let mut x: u64 = 12345;
    for _i in @as(usize, 0)..1000 {
        x = (x *% 6364136223846793005) +% 1442695040888963407;
        h.push(x >> 40)!;
    }
    @expect_eq(h.len(), 1000);
    let mut prev: u64 = 0;
    for _i in @as(usize, 0)..1000 {
        let v = h.pop()!;
        @expect(v >= prev);
        prev = v;
    }
    @expect(is_empty_error(h.pop()));
    let xs: [7]u64 = [5, 1, 9, 3, 7, 2, 8];
    let mut m = Heap.[u64].from_slice(xs[..], gt)!;
    defer m.deinit();
    @expect_eq(m.peek()!, 9);
    @expect_eq(m.replace(4)!, 9);
    @expect_eq(m.pop()!, 8);
    @expect_eq(m.pop()!, 7);
    @expect_eq(m.pop()!, 5);
    @expect_eq(m.pop()!, 4);
}
