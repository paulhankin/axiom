// =============================================================================
// std/vec.ph - growable arrays and slice algorithms.
// =============================================================================

import "mem.ph" as mem;

error { Empty, NotFound, OutOfMemory }

/// A growable array. Elements live in memory from `alloc`; `deinit` frees them.
/// Slices returned by `items()` are invalidated by any operation that grows
/// the vector.
pub struct Vec[T] {
    alloc: mem.Allocator,
    buf: []mut T,  // capacity == buf.len
    len: usize,

    pub fn init(alloc: mem.Allocator) Self {
        return Self.{ alloc: alloc, buf: @slice_from_ptr(@ptr_from_int(*mut T, @align_of(T)), 0), len: 0 };
    }

    pub fn with_capacity(alloc: mem.Allocator, cap: usize) !Self {
        let mut v = Self.init(alloc);
        v.reserve(cap)!;
        return v;
    }

    pub fn deinit(self: *mut Self) void {
        if self.buf.len > 0 {
            self.alloc.free(self.buf);
        }
        self.buf = @slice_from_ptr(@ptr_from_int(*mut T, @align_of(T)), 0);
        self.len = 0;
    }

    pub fn capacity(self: *Self) usize {
        return self.buf.len;
    }

    /// Ensures room for `extra` more elements without reallocating.
    pub fn reserve(self: *mut Self, extra: usize) !void {
        let need = @add_checked(self.len, extra) catch return error.OutOfMemory;
        if need <= self.buf.len {
            return;
        }
        let mut cap = @max(self.buf.len, 4);
        while cap < need {
            cap = @mul_checked(cap, 2) catch return error.OutOfMemory;
        }
        let nb = self.alloc.alloc.[T](cap)!;
        @mem_copy(nb[0..self.len], self.buf[0..self.len]);
        if self.buf.len > 0 {
            self.alloc.free(self.buf);
        }
        self.buf = nb;
    }

    pub fn push(self: *mut Self, x: T) !void {
        if self.len == self.buf.len {
            self.reserve(1)!;
        }
        self.buf[self.len] = x;
        self.len += 1;
    }

    pub fn pop(self: *mut Self) !T {
        if self.len == 0 {
            return error.Empty;
        }
        self.len -= 1;
        return self.buf[self.len];
    }

    pub fn last(self: *Self) !T {
        if self.len == 0 {
            return error.Empty;
        }
        return self.buf[self.len - 1];
    }

    /// Element `i`; traps if `i >= len`.
    pub fn get(self: *Self, i: usize) T {
        return self.buf[..self.len][i];
    }

    pub fn set(self: *mut Self, i: usize, x: T) void {
        self.buf[..self.len][i] = x;
    }

    pub fn get_ptr(self: *mut Self, i: usize) *mut T {
        return &mut self.buf[..self.len][i];
    }

    /// The live elements.
    pub fn items(self: *Self) []mut T {
        return self.buf[..self.len];
    }

    pub fn clear(self: *mut Self) void {
        self.len = 0;
    }

    pub fn truncate(self: *mut Self, n: usize) void {
        if n < self.len {
            self.len = n;
        }
    }

    pub fn extend(self: *mut Self, xs: []T) !void {
        self.reserve(xs.len)!;
        @mem_copy(self.buf[self.len..self.len + xs.len], xs);
        self.len += xs.len;
    }

    /// Inserts at `i` (0 <= i <= len), shifting later elements right.
    pub fn insert(self: *mut Self, i: usize, x: T) !void {
        if i > self.len {
            @trap();
        }
        self.reserve(1)!;
        let mut j = self.len;
        while j > i {
            self.buf[j] = self.buf[j - 1];
            j -= 1;
        }
        self.buf[i] = x;
        self.len += 1;
    }

    /// Removes element `i`, shifting later elements left. Traps if `i >= len`.
    pub fn remove(self: *mut Self, i: usize) T {
        let x = self.buf[..self.len][i];
        for j in i..self.len - 1 {
            self.buf[j] = self.buf[j + 1];
        }
        self.len -= 1;
        return x;
    }

    /// Removes element `i` in O(1) by moving the last element into its place.
    pub fn swap_remove(self: *mut Self, i: usize) T {
        let x = self.buf[..self.len][i];
        self.buf[i] = self.buf[self.len - 1];
        self.len -= 1;
        return x;
    }

    pub fn sort(self: *mut Self, less: fn(T, T) bool) void {
        sort(self.buf[..self.len], less);
    }
}

// -----------------------------------------------------------------------------
// Slice algorithms
// -----------------------------------------------------------------------------

fn sift_down[T](xs: []mut T, start: usize, end: usize, less: fn(T, T) bool) void {
    let mut root = start;
    loop {
        let child = (2 * root) + 1;
        if child >= end {
            return;
        }
        let mut top = root;
        if less(xs[top], xs[child]) {
            top = child;
        }
        if ((child + 1) < end) and less(xs[top], xs[child + 1]) {
            top = child + 1;
        }
        if top == root {
            return;
        }
        let tmp = xs[root];
        xs[root] = xs[top];
        xs[top] = tmp;
        root = top;
    }
}

/// Sorts in place in O(n log n) (heapsort; not stable).
pub fn sort[T](xs: []mut T, less: fn(T, T) bool) void {
    let n = xs.len;
    if n < 2 {
        return;
    }
    let mut start = n / 2;
    while start > 0 {
        start -= 1;
        sift_down(xs, start, n, less);
    }
    let mut end = n;
    while end > 1 {
        end -= 1;
        let tmp = xs[0];
        xs[0] = xs[end];
        xs[end] = tmp;
        sift_down(xs, 0, end, less);
    }
}

pub fn reverse[T](xs: []mut T) void {
    if xs.len < 2 {
        return;
    }
    let mut i: usize = 0;
    let mut j = xs.len - 1;
    while i < j {
        let tmp = xs[i];
        xs[i] = xs[j];
        xs[j] = tmp;
        i += 1;
        j -= 1;
    }
}

pub fn index_of[T: Eq](xs: []T, x: T) !usize {
    for i in 0..xs.len {
        if xs[i] == x {
            return i;
        }
    }
    return error.NotFound;
}

/// Binary search in a slice sorted by `less`. Returns the index of an element
/// equal to `x`, or error.NotFound.
pub fn binary_search[T](xs: []T, x: T, less: fn(T, T) bool) !usize {
    let mut lo: usize = 0;
    let mut hi = xs.len;
    while lo < hi {
        let mid = lo + ((hi - lo) / 2);
        if less(xs[mid], x) {
            lo = mid + 1;
        } else if less(x, xs[mid]) {
            hi = mid;
        } else {
            return mid;
        }
    }
    return error.NotFound;
}
