// =============================================================================
// File: generics.ph
// Monomorphized generics: constrained functions, a generic container,
// const parameters, and function-pointer parameters for behavior.
// =============================================================================

import "std/mem" as mem;

error { OutOfMemory }

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

pub struct List[T] {
    items: []mut T,
    len: usize,

    pub fn init(capacity: usize) !Self {
        let items = mem.alloc.[T](capacity)!;
        return Self.{ items: items, len: 0 };
    }

    pub fn deinit(self: *mut Self) void {
        mem.free(self.items);
    }

    pub fn push(self: *mut Self, x: T) !void {
        if self.len == self.items.len {
            return error.OutOfMemory;
        }
        self.items[self.len] = x;
        self.len += 1;
    }

    pub fn get(self: *Self, i: usize) T {
        return self.items[..self.len][i];
    }

    pub fn as_slice(self: *Self) []T {
        return self.items[..self.len];
    }
}

// Behavior is passed explicitly: there are no interfaces or vtables.
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

fn u32_less(a: u32, b: u32) bool {
    return a < b;
}

export fn demo() u32 {
    let mut xs = List.[u32].init(8) catch return 0;
    defer xs.deinit();
    xs.push(30) catch return 0;
    xs.push(10) catch return 0;
    xs.push(20) catch return 0;

    insertion_sort(xs.items[..xs.len], u32_less);  // T deduced as u32

    let lanes: [4]u32 = [1, 2, 3, 4];
    let total = sum(lanes);  // T = u32, N = 4

    return max(xs.get(0), total);  // 10 vs 10 -> 10
}
