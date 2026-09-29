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
    buf: []mut T,
    head: usize,  // next slot to receive from
    len: usize,
    closed: bool,
    lock: Mutex,
    not_empty: u32,  // bumped (and notified) whenever an item arrives or on close
    not_full: u32,   // bumped (and notified) whenever a slot frees or on close

    pub fn init(capacity: usize) !Self {
        let buf = mem.alloc.[T](@max(capacity, 1))!;
        return Self.{ buf: buf, head: 0, len: 0, closed: false, lock: Mutex.init(), not_empty: 0, not_full: 0 };
    }

    /// Frees the buffer. No thread may be using the channel.
    pub fn deinit(self: *mut Self) void {
        mem.free(self.buf);
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

/// A condition variable, used with a Mutex: wait releases the mutex and
/// sleeps until signal or broadcast (or spuriously, so wait in a loop that
/// checks the condition).
pub struct Condvar {
    seq: u32,

    pub fn init() Condvar {
        return Condvar.{ seq: 0 };
    }

    /// Atomically unlocks m and waits; locks m again before returning.
    pub fn wait(self: *mut Condvar, m: *mut Mutex) void {
        let s = @atomic_load(&self.seq);
        m.unlock();
        _ = @atomic_wait(&self.seq, s, WAIT_FOREVER);
        m.lock();
    }

    /// wait, for at most timeout_ns nanoseconds; false if it timed out.
    pub fn wait_timeout(self: *mut Condvar, m: *mut Mutex, timeout_ns: i64) bool {
        let s = @atomic_load(&self.seq);
        m.unlock();
        let r = @atomic_wait(&self.seq, s, @max(timeout_ns, 0));
        m.lock();
        return r != 2;
    }

    /// Wakes one waiting thread.
    pub fn signal(self: *mut Condvar) void {
        _ = @atomic_add(&mut self.seq, 1);
        _ = @atomic_notify(&self.seq, 1);
    }

    /// Wakes every waiting thread.
    pub fn broadcast(self: *mut Condvar) void {
        _ = @atomic_add(&mut self.seq, 1);
        _ = @atomic_notify(&self.seq, NOTIFY_ALL);
    }
}

/// A readers-writer lock: any number of readers, or one writer. Writers are
/// preferred: once one is waiting, new readers wait too, so a stream of
/// readers cannot starve it.
pub struct RwLock {
    lock: Mutex,
    readers: u32,
    writer: bool,
    waiting: u32,  // writers waiting
    seq: u32,      // bumped (and notified) when the lock is released

    pub fn init() RwLock {
        return RwLock.{ lock: Mutex.init(), readers: 0, writer: false, waiting: 0, seq: 0 };
    }

    pub fn read_lock(self: *mut RwLock) void {
        self.lock.lock();
        while self.writer or (self.waiting > 0) {
            self.sleep();
        }
        self.readers += 1;
        self.lock.unlock();
    }

    pub fn try_read_lock(self: *mut RwLock) bool {
        self.lock.lock();
        let ok = not self.writer and (self.waiting == 0);
        if ok {
            self.readers += 1;
        }
        self.lock.unlock();
        return ok;
    }

    pub fn read_unlock(self: *mut RwLock) void {
        self.lock.lock();
        self.readers -= 1;
        let wake = self.readers == 0;
        if wake {
            _ = @atomic_add(&mut self.seq, 1);
        }
        self.lock.unlock();
        if wake {
            _ = @atomic_notify(&self.seq, NOTIFY_ALL);
        }
    }

    pub fn write_lock(self: *mut RwLock) void {
        self.lock.lock();
        self.waiting += 1;
        while self.writer or (self.readers > 0) {
            self.sleep();
        }
        self.waiting -= 1;
        self.writer = true;
        self.lock.unlock();
    }

    pub fn try_write_lock(self: *mut RwLock) bool {
        self.lock.lock();
        let ok = not self.writer and (self.readers == 0);
        if ok {
            self.writer = true;
        }
        self.lock.unlock();
        return ok;
    }

    pub fn write_unlock(self: *mut RwLock) void {
        self.lock.lock();
        self.writer = false;
        _ = @atomic_add(&mut self.seq, 1);
        self.lock.unlock();
        _ = @atomic_notify(&self.seq, NOTIFY_ALL);
    }

    // (called with the lock held; returns with it held)
    fn sleep(self: *mut RwLock) void {
        let s = @atomic_load(&self.seq);
        self.lock.unlock();
        _ = @atomic_wait(&self.seq, s, WAIT_FOREVER);
        self.lock.lock();
    }
}

/// A counting semaphore: acquire takes a permit, waiting for one if there
/// are none; release returns permits.
pub struct Semaphore {
    permits: u32,

    pub fn init(permits: u32) Semaphore {
        return Semaphore.{ permits: permits };
    }

    pub fn acquire(self: *mut Semaphore) void {
        loop {
            let p = @atomic_load(&self.permits);
            if p == 0 {
                _ = @atomic_wait(&self.permits, 0, WAIT_FOREVER);
                continue;
            }
            if @atomic_cas(&mut self.permits, p, p - 1) == p {
                return;
            }
        }
    }

    /// Takes a permit if one is free, without waiting.
    pub fn try_acquire(self: *mut Semaphore) bool {
        loop {
            let p = @atomic_load(&self.permits);
            if p == 0 {
                return false;
            }
            if @atomic_cas(&mut self.permits, p, p - 1) == p {
                return true;
            }
        }
    }

    pub fn release(self: *mut Semaphore, n: u32) void {
        _ = @atomic_add(&mut self.permits, n);
        _ = @atomic_notify(&self.permits, n);
    }

    /// The permits free now (which may change at once).
    pub fn available(self: *Semaphore) u32 {
        return @atomic_load(&self.permits);
    }
}

/// Runs a function exactly once, however many threads call it; the others
/// wait until it has finished.
pub struct Once {
    state: u32,  // 0 not run, 1 running, 2 done

    pub fn init() Once {
        return Once.{ state: 0 };
    }

    pub fn call[C](self: *mut Once, f: fn(C) void, arg: C) void {
        if @atomic_load(&self.state) == 2 {
            return;
        }
        if @atomic_cas(&mut self.state, 0, 1) == 0 {
            f(arg);
            @atomic_store(&mut self.state, 2);
            _ = @atomic_notify(&self.state, NOTIFY_ALL);
            return;
        }
        while @atomic_load(&self.state) != 2 {
            _ = @atomic_wait(&self.state, 1, WAIT_FOREVER);
        }
    }

    pub fn done(self: *Once) bool {
        return @atomic_load(&self.state) == 2;
    }
}

/// Makes a fixed number of threads wait until all of them have arrived;
/// then it resets, for the next phase.
pub struct Barrier {
    lock: Mutex,
    n: u32,
    arrived: u32,
    generation: u32,

    pub fn init(n: u32) Barrier {
        return Barrier.{ lock: Mutex.init(), n: @max(n, 1), arrived: 0, generation: 0 };
    }

    /// Waits for the other threads; true in exactly one of them (the last
    /// to arrive), which can do work between phases.
    pub fn wait(self: *mut Barrier) bool {
        self.lock.lock();
        let g = self.generation;
        self.arrived += 1;
        if self.arrived == self.n {
            self.arrived = 0;
            _ = @atomic_add(&mut self.generation, 1);
            self.lock.unlock();
            _ = @atomic_notify(&self.generation, NOTIFY_ALL);
            return true;
        }
        self.lock.unlock();
        while @atomic_load(&self.generation) == g {
            _ = @atomic_wait(&self.generation, g, WAIT_FOREVER);
        }
        return false;
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
    let mut c = Counter.{ lock: Mutex.init(), n: 0 };
    let t1 = thread.spawn(bump, &mut c)!;
    let t2 = thread.spawn(bump, &mut c)!;
    bump(&mut c);
    t1.join();
    t2.join();
    @expect_eq(c.n, 15000);
}

test "channel delivers in order, then Closed" {
    let mut ch = Channel.[u32].init(4)!;
    defer ch.deinit();
    let t = thread.spawn(feed, &mut ch)!;
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

struct Shared {
    rw: RwLock,
    a: u64,
    b: u64,  // always equal to a, as readers check
    bad: u32,
    sem: Semaphore,
    inside: u32,
    most: u32,
    once: Once,
    runs: u32,
    m: Mutex,
    cv: Condvar,
    ready: u32,
    bar: Barrier,
    phase_sum: [3]u32,
}

fn rw_worker(s: *mut Shared) void {
    for i in @as(u32, 0)..2000 {
        if (i % 4) == 0 {
            s.rw.write_lock();
            s.a += 1;
            s.b += 1;
            s.rw.write_unlock();
        } else {
            s.rw.read_lock();
            if s.a != s.b {
                _ = @atomic_add(&mut s.bad, 1);
            }
            s.rw.read_unlock();
        }
    }
}

fn sem_worker(s: *mut Shared) void {
    for _i in @as(u32, 0)..200 {
        s.sem.acquire();
        let now = @atomic_add(&mut s.inside, 1) + 1;
        _ = @atomic_max(&mut s.most, now);
        _ = @atomic_sub(&mut s.inside, 1);
        s.sem.release(1);
    }
}

fn init_once(s: *mut Shared) void {
    _ = @atomic_add(&mut s.runs, 1);
}

fn once_worker(s: *mut Shared) void {
    s.once.call(init_once, s);
    // the others wait for it to finish
    if @atomic_load(&s.runs) != 1 {
        _ = @atomic_add(&mut s.bad, 1);
    }
}

fn cv_worker(s: *mut Shared) void {
    s.m.lock();
    s.ready += 1;
    s.cv.broadcast();
    s.m.unlock();
}

fn bar_worker(s: *mut Shared) void {
    for p in @as(usize, 0)..3 {
        _ = @atomic_add(&mut s.phase_sum[p], 1);
        _ = s.bar.wait();
        // everyone has finished phase p before anyone goes on
        if @atomic_load(&s.phase_sum[p]) != 4 {
            _ = @atomic_add(&mut s.bad, 1);
        }
        _ = s.bar.wait();
    }
}

fn shared() Shared {
    return Shared.{ rw: RwLock.init(), a: 0, b: 0, bad: 0, sem: Semaphore.init(2), inside: 0, most: 0,
                    once: Once.init(), runs: 0, m: Mutex.init(), cv: Condvar.init(), ready: 0, bar: Barrier.init(4),
                    phase_sum: [0; 3] };
}

test "RwLock, Semaphore, Once, Condvar and Barrier" {
    let mut s = shared();
    let workers: [5]fn(*mut Shared) void = [rw_worker, sem_worker, once_worker, cv_worker, bar_worker];
    for k in @as(usize, 0)..5 {
        let w = workers[k];
        let t1 = thread.spawn(w, &mut s)!;
        let t2 = thread.spawn(w, &mut s)!;
        let t3 = thread.spawn(w, &mut s)!;
        if k == 3 {
            // wait on the condition until all three have arrived
            s.m.lock();
            while s.ready < 3 {
                s.cv.wait(&mut s.m);
            }
            s.m.unlock();
        } else {
            w(&mut s);
        }
        t1.join();
        t2.join();
        t3.join();
    }
    @expect_eq(s.bad, 0);
    @expect_eq(s.a, 2000);
    @expect(s.most <= 2);
    @expect_eq(s.runs, 1);
    @expect(s.once.done());
    // try-locks
    @expect(s.rw.try_write_lock());
    @expect(not s.rw.try_read_lock());
    s.rw.write_unlock();
    @expect(s.rw.try_read_lock());
    @expect(not s.rw.try_write_lock());
    s.rw.read_unlock();
    // a timed wait with nobody to signal times out
    s.m.lock();
    @expect(not s.cv.wait_timeout(&mut s.m, 1000000));
    s.m.unlock();
    @expect(s.sem.try_acquire() and s.sem.try_acquire() and not s.sem.try_acquire());
}
