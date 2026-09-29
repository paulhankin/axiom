// =============================================================================
// std/deque.ph - double-ended queues: a growable ring buffer.
//
//     let mut q = deque.Deque.[u32].init();
//     defer q.deinit();
//     q.push_back(1)!;
//     q.push_front(0)!;
//     let first = q.pop_front()!;
//
// Pushing and popping at either end take constant (amortized) time, and
// get(i) indexes from the front.
// =============================================================================

import "mem.ph" as mem;

error { Empty, OutOfMemory }

pub struct Deque[T] {
    buf: []mut T,  // capacity: 0 or a power of two
    head: usize,   // the index in buf of the front element
    len: usize,

    pub fn init() Self {
        return Self.{ buf: @slice_from_ptr(@ptr_from_int(*mut T, @align_of(T)), 0), head: 0, len: 0 };
    }

    pub fn deinit(self: *mut Self) void {
        if self.buf.len > 0 {
            mem.free(self.buf);
        }
        self.buf = @slice_from_ptr(@ptr_from_int(*mut T, @align_of(T)), 0);
        self.head = 0;
        self.len = 0;
    }

    fn slot(self: *Self, i: usize) usize {
        return (self.head +% i) & (self.buf.len - 1);
    }

    /// Makes room for extra more elements.
    pub fn reserve(self: *mut Self, extra: usize) !void {
        let need = @add_checked(self.len, extra) catch return error.OutOfMemory;
        if need <= self.buf.len {
            return;
        }
        let mut cap = @max(self.buf.len, 8);
        while cap < need {
            cap = @mul_checked(cap, 2) catch return error.OutOfMemory;
        }
        let nb = mem.alloc.[T](cap)!;
        // unwrap the ring into the start of the new buffer
        for i in @as(usize, 0)..self.len {
            nb[i] = self.buf[self.slot(i)];
        }
        if self.buf.len > 0 {
            mem.free(self.buf);
        }
        self.buf = nb;
        self.head = 0;
    }

    pub fn push_back(self: *mut Self, x: T) !void {
        self.reserve(1)!;
        self.buf[self.slot(self.len)] = x;
        self.len += 1;
    }

    pub fn push_front(self: *mut Self, x: T) !void {
        self.reserve(1)!;
        self.head = (self.head +% (self.buf.len - 1)) & (self.buf.len - 1);
        self.buf[self.head] = x;
        self.len += 1;
    }

    pub fn pop_front(self: *mut Self) !T {
        if self.len == 0 {
            return error.Empty;
        }
        let x = self.buf[self.head];
        self.head = self.slot(1);
        self.len -= 1;
        return x;
    }

    pub fn pop_back(self: *mut Self) !T {
        if self.len == 0 {
            return error.Empty;
        }
        self.len -= 1;
        return self.buf[self.slot(self.len)];
    }

    pub fn front(self: *Self) !T {
        if self.len == 0 {
            return error.Empty;
        }
        return self.buf[self.head];
    }

    pub fn back(self: *Self) !T {
        if self.len == 0 {
            return error.Empty;
        }
        return self.buf[self.slot(self.len - 1)];
    }

    /// Element i from the front (i < len, or the program traps).
    pub fn get(self: *Self, i: usize) T {
        if i >= self.len {
            @trap();
        }
        return self.buf[self.slot(i)];
    }

    pub fn set(self: *mut Self, i: usize, x: T) void {
        if i >= self.len {
            @trap();
        }
        self.buf[self.slot(i)] = x;
    }

    pub fn clear(self: *mut Self) void {
        self.head = 0;
        self.len = 0;
    }

    /// The elements as at most two slices, front first: out[0] then out[1].
    pub fn as_slices(self: *Self) [2][]T {
        if self.len == 0 {
            return [self.buf[..0], self.buf[..0]];
        }
        let first = @min(self.len, self.buf.len - self.head);
        return [self.buf[self.head..self.head + first], self.buf[..self.len - first]];
    }
}

// -----------------------------------------------------------------------------
// Tests
// -----------------------------------------------------------------------------

test "deques" {
    let mut q = Deque.[u32].init();
    defer q.deinit();
    @expect(failed(q.pop_front()));
    for i in @as(u32, 0)..100 {
        q.push_back(i)!;
        q.push_front(1000 + i)!;
    }
    @expect_eq(q.len, 200);
    @expect_eq(q.front()!, 1099);
    @expect_eq(q.back()!, 99);
    @expect_eq(q.get(100), 0);
    for i in @as(u32, 0)..100 {
        @expect_eq(q.pop_front()!, 1099 - i);
    }
    // wrap around the ring many times
    for i in @as(u32, 0)..1000 {
        q.push_back(i)!;
        _ = q.pop_front()!;
    }
    @expect_eq(q.len, 100);
    let s = q.as_slices();
    @expect_eq(s[0].len + s[1].len, 100);
    let mut expect: u32 = 900;
    for part in s {
        for v in part {
            @expect_eq(v, expect);
            expect += 1;
        }
    }
    @expect_eq(q.pop_back()!, 999);
    q.set(0, 7);
    @expect_eq(q.front()!, 7);
    q.clear();
    @expect(failed(q.back()));
}

fn failed(r: !u32) bool {
    _ = r catch return true;
    return false;
}
