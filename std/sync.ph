// =============================================================================
// std/sync.ph - synchronization primitives (spec §12.8).
//
// Everything here is built from the atomic builtins and @atomic_wait /
// @atomic_notify, so it works the same on WebAssembly (shared memory) and
// native targets. A primitive must stay at one address while it is in use:
// share it between threads by pointer.
// =============================================================================

import "mem.ph" as mem;

error { Closed, Empty, OutOfMemory }

const WAIT_FOREVER: i64 = -1;
const NOTIFY_ALL: u32 = 0xFFFF_FFFF;

/// An integer that is only accessed atomically.
pub struct Atomic[T: Int] {
    value: T,

    pub fn init(v: T) Self {
        return Self.{ value: v };
    }
    pub fn load(self: *Self) T {
        return @atomic_load(&self.value);
    }
    pub fn store(self: *mut Self, v: T) void {
        @atomic_store(&mut self.value, v);
    }
    /// Adds (wrapping) and returns the previous value.
    pub fn add(self: *mut Self, v: T) T {
        return @atomic_add(&mut self.value, v);
    }
    /// Subtracts (wrapping) and returns the previous value.
    pub fn sub(self: *mut Self, v: T) T {
        return @atomic_sub(&mut self.value, v);
    }
    pub fn swap(self: *mut Self, v: T) T {
        return @atomic_xchg(&mut self.value, v);
    }
    /// Stores `desired` if the value is `expected`; returns the previous value
    /// (the exchange happened iff it equals `expected`).
    pub fn compare_exchange(self: *mut Self, expected: T, desired: T) T {
        return @atomic_cas(&mut self.value, expected, desired);
    }
}

/// A mutual-exclusion lock (futex-based: no system call when uncontended).
pub struct Mutex {
    state: u32,  // 0 unlocked, 1 locked, 2 locked with waiters

    pub fn init() Mutex {
        return Mutex.{ state: 0 };
    }

    pub fn lock(self: *mut Mutex) void {
        let mut s = @atomic_cas(&mut self.state, 0, 1);
        if s == 0 {
            return;
        }
        if s != 2 {
            s = @atomic_xchg(&mut self.state, 2);
        }
        while s != 0 {
            _ = @atomic_wait(&self.state, 2, WAIT_FOREVER);
            s = @atomic_xchg(&mut self.state, 2);
        }
    }

    pub fn try_lock(self: *mut Mutex) bool {
        return @atomic_cas(&mut self.state, 0, 1) == 0;
    }

    pub fn unlock(self: *mut Mutex) void {
        if @atomic_sub(&mut self.state, 1) != 1 {
            @atomic_store(&mut self.state, 0);
            _ = @atomic_notify(&self.state, 1);
        }
    }
}

/// Waits for a number of tasks to finish.
pub struct WaitGroup {
    count: u32,

    pub fn init() WaitGroup {
        return WaitGroup.{ count: 0 };
    }

    pub fn add(self: *mut WaitGroup, n: u32) void {
        _ = @atomic_add(&mut self.count, n);
    }

    pub fn done(self: *mut WaitGroup) void {
        if @atomic_sub(&mut self.count, 1) == 1 {
            _ = @atomic_notify(&self.count, NOTIFY_ALL);
        }
    }

    /// Blocks until the count reaches zero.
    pub fn wait(self: *mut WaitGroup) void {
        loop {
            let c = @atomic_load(&self.count);
            if c == 0 {
                return;
            }
            _ = @atomic_wait(&self.count, c, WAIT_FOREVER);
        }
    }
}

/// A bounded, multi-producer multi-consumer FIFO channel.
pub struct Channel[T] {
    alloc: mem.Allocator,
    buf: []mut T,
    head: usize,  // next slot to receive from
    len: usize,
    closed: bool,
    lock: Mutex,
    not_empty: u32,  // bumped (and notified) whenever an item arrives or on close
    not_full: u32,   // bumped (and notified) whenever a slot frees or on close

    pub fn init(alloc: mem.Allocator, capacity: usize) !Self {
        let buf = alloc.alloc.[T](@max(capacity, 1))!;
        return Self.{ alloc: alloc, buf: buf, head: 0, len: 0, closed: false, lock: Mutex.init(), not_empty: 0,
                      not_full: 0 };
    }

    /// Frees the buffer. No thread may be using the channel.
    pub fn deinit(self: *mut Self) void {
        self.alloc.free(self.buf);
    }

    /// Blocks while the channel is full. Fails with Closed after close().
    pub fn send(self: *mut Self, x: T) !void {
        self.lock.lock();
        loop {
            if self.closed {
                self.lock.unlock();
                return error.Closed;
            }
            if self.len < self.buf.len {
                break;
            }
            let seq = @atomic_load(&self.not_full);
            self.lock.unlock();
            _ = @atomic_wait(&self.not_full, seq, WAIT_FOREVER);
            self.lock.lock();
        }
        self.buf[(self.head + self.len) % self.buf.len] = x;
        self.len += 1;
        _ = @atomic_add(&mut self.not_empty, 1);
        self.lock.unlock();
        _ = @atomic_notify(&self.not_empty, 1);
    }

    /// Blocks while the channel is empty. Fails with Closed once the channel
    /// is closed and drained.
    pub fn recv(self: *mut Self) !T {
        self.lock.lock();
        loop {
            if self.len > 0 {
                break;
            }
            if self.closed {
                self.lock.unlock();
                return error.Closed;
            }
            let seq = @atomic_load(&self.not_empty);
            self.lock.unlock();
            _ = @atomic_wait(&self.not_empty, seq, WAIT_FOREVER);
            self.lock.lock();
        }
        let x = self.buf[self.head];
        self.head = (self.head + 1) % self.buf.len;
        self.len -= 1;
        _ = @atomic_add(&mut self.not_full, 1);
        self.lock.unlock();
        _ = @atomic_notify(&self.not_full, 1);
        return x;
    }

    /// Like recv, but fails with Empty instead of blocking.
    pub fn try_recv(self: *mut Self) !T {
        self.lock.lock();
        if self.len == 0 {
            let closed = self.closed;
            self.lock.unlock();
            if closed {
                return error.Closed;
            }
            return error.Empty;
        }
        let x = self.buf[self.head];
        self.head = (self.head + 1) % self.buf.len;
        self.len -= 1;
        _ = @atomic_add(&mut self.not_full, 1);
        self.lock.unlock();
        _ = @atomic_notify(&self.not_full, 1);
        return x;
    }

    /// Wakes all blocked senders and receivers. Further sends fail; receives
    /// drain what is left, then fail with Closed.
    pub fn close(self: *mut Self) void {
        self.lock.lock();
        self.closed = true;
        _ = @atomic_add(&mut self.not_empty, 1);
        _ = @atomic_add(&mut self.not_full, 1);
        self.lock.unlock();
        _ = @atomic_notify(&self.not_empty, NOTIFY_ALL);
        _ = @atomic_notify(&self.not_full, NOTIFY_ALL);
    }
}

// -----------------------------------------------------------------------------
// Tests (phalange test std/sync.ph)
// -----------------------------------------------------------------------------

import "thread.ph" as thread;

struct Counter {
    lock: Mutex,
    n: u64,
}

fn bump(c: *mut Counter) void {
    for i in @as(u32, 0)..5000 {
        c.lock.lock();
        c.n += 1;
        c.lock.unlock();
    }
}

fn feed(ch: *mut Channel.[u32]) void {
    for i in @as(u32, 0)..100 {
        ch.send(i) catch return;
    }
    ch.close();
}

test "mutex protects a counter across threads" {
    let al = mem.page_allocator;
    let mut c = Counter.{ lock: Mutex.init(), n: 0 };
    let t1 = thread.spawn(al, bump, &mut c)!;
    let t2 = thread.spawn(al, bump, &mut c)!;
    bump(&mut c);
    t1.join();
    t2.join();
    @expect_eq(c.n, 15000);
}

test "channel delivers in order, then Closed" {
    let al = mem.page_allocator;
    let mut ch = Channel.[u32].init(al, 4)!;
    defer ch.deinit();
    let t = thread.spawn(al, feed, &mut ch)!;
    let mut expect: u32 = 0;
    loop {
        let v = ch.recv() catch |e| {
            @expect_eq(e, error.Closed);
            break;
        };
        @expect_eq(v, expect);
        expect += 1;
    }
    t.join();
    @expect_eq(expect, 100);
}

test "Atomic" {
    let mut a = Atomic.[i32].init(5);
    @expect_eq(a.add(2), 5);
    @expect_eq(a.compare_exchange(7, 1), 7);
    @expect_eq(a.load(), 1);
}
