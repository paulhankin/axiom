// =============================================================================
// std/collections/treemap.ph - ordered maps: AVL trees.
//
//     let mut m = treemap.TreeMap.[u64, u32].init(less_u64);
//     defer m.deinit();
//     m.put(5, 50)!;
//     let v = m.get(5)!;
//     let k = m.floor(7)!;                  // the greatest key <= 7
//     let mut it = m.iter();                // in key order
//     while it.next(&mut k, &mut v) { ... }
//     let mut r = m.iter_from(3);           // keys >= 3, in order
//
// Keys are ordered by the `less` function given to init; two keys are the
// same when neither is less than the other. Lookups, insertions and
// removals take O(log n) time.
//
// The nodes live in arrays and link by index, so the tree needs no
// pointers that could dangle; removed nodes are reused.
// =============================================================================

import "../mem.ph" as mem;

error { NotFound, OutOfMemory }

const NIL: u32 = 0xffffffff;
const MAX_HEIGHT: usize = 64;  // an AVL tree of 2^32 nodes is < 48 high

pub struct TreeMap[K, V] {
    less: fn(K, K) bool,
    keys: []mut K,
    vals: []mut V,
    left: []mut u32,
    right: []mut u32,
    height: []mut u8,
    root: u32,
    free: u32,  // removed nodes, linked through left
    used: u32,  // nodes ever handed out: [0, used)
    count: usize,

    pub fn init(less: fn(K, K) bool) Self {
        return Self.{
            less: less,
            keys: @slice_from_ptr(@ptr_from_int(*mut K, @align_of(K)), 0),
            vals: @slice_from_ptr(@ptr_from_int(*mut V, @align_of(V)), 0),
            left: @slice_from_ptr(@ptr_from_int(*mut u32, 4), 0),
            right: @slice_from_ptr(@ptr_from_int(*mut u32, 4), 0),
            height: @slice_from_ptr(@ptr_from_int(*mut u8, 1), 0),
            root: NIL,
            free: NIL,
            used: 0,
            count: 0,
        };
    }

    pub fn deinit(self: *mut Self) void {
        if self.left.len > 0 {
            mem.free(self.keys);
            mem.free(self.vals);
            mem.free(self.left);
            mem.free(self.right);
            mem.free(self.height);
        }
        let less = self.less;
        self.* = Self.init(less);
    }

    pub fn len(self: *Self) usize {
        return self.count;
    }

    /// Removes every entry (keeping the memory).
    pub fn clear(self: *mut Self) void {
        self.root = NIL;
        self.free = NIL;
        self.used = 0;
        self.count = 0;
    }

    // --- nodes ----------------------------------------------------------------

    fn new_node(self: *mut Self, k: K, v: V) !u32 {
        let mut n = self.free;
        if n != NIL {
            self.free = self.left[@as(usize, n)];
        } else {
            if @as(usize, self.used) == self.left.len {
                self.grow()!;
            }
            n = self.used;
            self.used += 1;
        }
        let i = @as(usize, n);
        self.keys[i] = k;
        self.vals[i] = v;
        self.left[i] = NIL;
        self.right[i] = NIL;
        self.height[i] = 1;
        return n;
    }

    fn grow(self: *mut Self) !void {
        let old = self.left.len;
        if old >= 0x80000000 {
            return error.OutOfMemory;
        }
        let cap = @max(old * 2, 16);
        // (each array is replaced only once all are allocated)
        let k = mem.alloc.[K](cap)!;
        let v = mem.alloc.[V](cap) catch |e| {
            mem.free(k);
            return e;
        };
        let l = mem.alloc.[u32](cap) catch |e| {
            mem.free(k);
            mem.free(v);
            return e;
        };
        let r = mem.alloc.[u32](cap) catch |e| {
            mem.free(k);
            mem.free(v);
            mem.free(l);
            return e;
        };
        let h = mem.alloc.[u8](cap) catch |e| {
            mem.free(k);
            mem.free(v);
            mem.free(l);
            mem.free(r);
            return e;
        };
        if old > 0 {
            @mem_copy(k[..old], self.keys);
            @mem_copy(v[..old], self.vals);
            @mem_copy(l[..old], self.left);
            @mem_copy(r[..old], self.right);
            @mem_copy(h[..old], self.height);
            mem.free(self.keys);
            mem.free(self.vals);
            mem.free(self.left);
            mem.free(self.right);
            mem.free(self.height);
        }
        self.keys = k;
        self.vals = v;
        self.left = l;
        self.right = r;
        self.height = h;
    }

    fn h(self: *Self, n: u32) u8 {
        if n == NIL {
            return 0;
        }
        return self.height[@as(usize, n)];
    }

    fn fix(self: *mut Self, n: u32) void {
        let i = @as(usize, n);
        self.height[i] = @max(self.h(self.left[i]), self.h(self.right[i])) + 1;
    }

    fn rot_right(self: *mut Self, n: u32) u32 {
        let i = @as(usize, n);
        let l = self.left[i];
        let li = @as(usize, l);
        self.left[i] = self.right[li];
        self.right[li] = n;
        self.fix(n);
        self.fix(l);
        return l;
    }

    fn rot_left(self: *mut Self, n: u32) u32 {
        let i = @as(usize, n);
        let r = self.right[i];
        let ri = @as(usize, r);
        self.right[i] = self.left[ri];
        self.left[ri] = n;
        self.fix(n);
        self.fix(r);
        return r;
    }

    // Restores the AVL balance at n (whose subtrees are balanced).
    fn balance(self: *mut Self, n: u32) u32 {
        self.fix(n);
        let i = @as(usize, n);
        let hl = @as(i32, self.h(self.left[i]));
        let hr = @as(i32, self.h(self.right[i]));
        if (hl - hr) > 1 {
            let l = @as(usize, self.left[i]);
            if self.h(self.left[l]) < self.h(self.right[l]) {
                self.left[i] = self.rot_left(self.left[i]);
            }
            return self.rot_right(n);
        }
        if (hr - hl) > 1 {
            let r = @as(usize, self.right[i]);
            if self.h(self.right[r]) < self.h(self.left[r]) {
                self.right[i] = self.rot_right(self.right[i]);
            }
            return self.rot_left(n);
        }
        return n;
    }

    // --- lookups -----------------------------------------------------------------

    fn find(self: *Self, k: K) u32 {
        let mut n = self.root;
        while n != NIL {
            let i = @as(usize, n);
            if self.less(k, self.keys[i]) {
                n = self.left[i];
            } else if self.less(self.keys[i], k) {
                n = self.right[i];
            } else {
                return n;
            }
        }
        return NIL;
    }

    pub fn get(self: *Self, k: K) !V {
        let n = self.find(k);
        if n == NIL {
            return error.NotFound;
        }
        return self.vals[@as(usize, n)];
    }

    /// The value for k, to change in place; valid until the map changes.
    pub fn get_ptr(self: *mut Self, k: K) !*mut V {
        let n = self.find(k);
        if n == NIL {
            return error.NotFound;
        }
        return &mut self.vals[@as(usize, n)];
    }

    pub fn contains(self: *Self, k: K) bool {
        return self.find(k) != NIL;
    }

    /// The least key.
    pub fn min(self: *Self) !K {
        let mut n = self.root;
        if n == NIL {
            return error.NotFound;
        }
        while self.left[@as(usize, n)] != NIL {
            n = self.left[@as(usize, n)];
        }
        return self.keys[@as(usize, n)];
    }

    /// The greatest key.
    pub fn max(self: *Self) !K {
        let mut n = self.root;
        if n == NIL {
            return error.NotFound;
        }
        while self.right[@as(usize, n)] != NIL {
            n = self.right[@as(usize, n)];
        }
        return self.keys[@as(usize, n)];
    }

    /// The greatest key <= k.
    pub fn floor(self: *Self, k: K) !K {
        let mut n = self.root;
        let mut best = NIL;
        while n != NIL {
            let i = @as(usize, n);
            if self.less(k, self.keys[i]) {
                n = self.left[i];
            } else {
                best = n;
                n = self.right[i];
            }
        }
        if best == NIL {
            return error.NotFound;
        }
        return self.keys[@as(usize, best)];
    }

    /// The least key >= k.
    pub fn ceil(self: *Self, k: K) !K {
        let mut n = self.root;
        let mut best = NIL;
        while n != NIL {
            let i = @as(usize, n);
            if self.less(self.keys[i], k) {
                n = self.right[i];
            } else {
                best = n;
                n = self.left[i];
            }
        }
        if best == NIL {
            return error.NotFound;
        }
        return self.keys[@as(usize, best)];
    }

    // --- changes -----------------------------------------------------------------

    /// Inserts or replaces the value for k.
    pub fn put(self: *mut Self, k: K, v: V) !void {
        let n = self.find(k);
        if n != NIL {
            self.vals[@as(usize, n)] = v;
            return;
        }
        let fresh = self.new_node(k, v)!;
        self.root = self.insert(self.root, fresh);
        self.count += 1;
    }

    // Inserts node x (a key not in the tree) under n; returns the new root.
    fn insert(self: *mut Self, n: u32, x: u32) u32 {
        if n == NIL {
            return x;
        }
        let i = @as(usize, n);
        if self.less(self.keys[@as(usize, x)], self.keys[i]) {
            self.left[i] = self.insert(self.left[i], x);
        } else {
            self.right[i] = self.insert(self.right[i], x);
        }
        return self.balance(n);
    }

    /// Removes k; returns whether it was there.
    pub fn remove(self: *mut Self, k: K) bool {
        let n = self.find(k);
        if n == NIL {
            return false;
        }
        self.root = self.delete(self.root, k);
        self.left[@as(usize, n)] = self.free;
        self.free = n;
        self.count -= 1;
        return true;
    }

    /// Removes and returns the value for k.
    pub fn take(self: *mut Self, k: K) !V {
        let v = self.get(k)!;
        _ = self.remove(k);
        return v;
    }

    // Unlinks the node with key k from under n; returns the new root.
    fn delete(self: *mut Self, n: u32, k: K) u32 {
        let i = @as(usize, n);
        if self.less(k, self.keys[i]) {
            self.left[i] = self.delete(self.left[i], k);
            return self.balance(n);
        }
        if self.less(self.keys[i], k) {
            self.right[i] = self.delete(self.right[i], k);
            return self.balance(n);
        }
        let l = self.left[i];
        let r = self.right[i];
        if l == NIL {
            return r;
        }
        if r == NIL {
            return l;
        }
        // replace n by the least node of its right subtree
        let mut m = r;
        while self.left[@as(usize, m)] != NIL {
            m = self.left[@as(usize, m)];
        }
        let mi = @as(usize, m);
        self.right[mi] = self.unlink_min(r);
        self.left[mi] = l;
        return self.balance(m);
    }

    // Unlinks the least node under n; returns the new root.
    fn unlink_min(self: *mut Self, n: u32) u32 {
        let i = @as(usize, n);
        if self.left[i] == NIL {
            return self.right[i];
        }
        self.left[i] = self.unlink_min(self.left[i]);
        return self.balance(n);
    }

    // --- iteration -----------------------------------------------------------------

    /// Every entry in key order. Do not change the map while iterating.
    pub fn iter(self: *Self) Iter.[K, V] {
        let mut it = Iter.[K, V].{ map: self, stack: [NIL; MAX_HEIGHT], sp: 0 };
        it.push_left(self.root);
        return it;
    }

    /// The entries with keys >= lo, in key order.
    pub fn iter_from(self: *Self, lo: K) Iter.[K, V] {
        let mut it = Iter.[K, V].{ map: self, stack: [NIL; MAX_HEIGHT], sp: 0 };
        let mut n = self.root;
        while n != NIL {
            let i = @as(usize, n);
            if self.less(self.keys[i], lo) {
                n = self.right[i];
            } else {
                it.stack[it.sp] = n;
                it.sp += 1;
                n = self.left[i];
            }
        }
        return it;
    }
}

pub struct Iter[K, V] {
    map: *TreeMap.[K, V],
    stack: [MAX_HEIGHT]u32,  // nodes whose left subtree is done
    sp: usize,

    fn push_left(self: *mut Self, n0: u32) void {
        let mut n = n0;
        while n != NIL {
            self.stack[self.sp] = n;
            self.sp += 1;
            n = self.map.left[@as(usize, n)];
        }
    }

    /// Stores the next entry through key and val and returns true, or
    /// returns false at the end.
    pub fn next(self: *mut Self, key: *mut K, val: *mut V) bool {
        if self.sp == 0 {
            return false;
        }
        self.sp -= 1;
        let n = self.stack[self.sp];
        let i = @as(usize, n);
        key.* = self.map.keys[i];
        val.* = self.map.vals[i];
        self.push_left(self.map.right[i]);
        return true;
    }
}

// -----------------------------------------------------------------------------
// Tests
// -----------------------------------------------------------------------------

fn lt(a: u64, b: u64) bool {
    return a < b;
}

fn str_lt(a: []u8, b: []u8) bool {
    let n = @min(a.len, b.len);
    for i in @as(usize, 0)..n {
        if a[i] != b[i] {
            return a[i] < b[i];
        }
    }
    return a.len < b.len;
}

// Checks the AVL invariants and ordering; returns the height.
fn check[K, V](m: *TreeMap.[K, V], n: u32) i32 {
    if n == NIL {
        return 0;
    }
    let i = @as(usize, n);
    let hl = check(m, m.left[i]);
    let hr = check(m, m.right[i]);
    @expect((hl - hr) <= 1);
    @expect((hr - hl) <= 1);
    @expect_eq(@as(i32, m.height[i]), @max(hl, hr) + 1);
    if m.left[i] != NIL {
        @expect(m.less(m.keys[@as(usize, m.left[i])], m.keys[i]));
    }
    if m.right[i] != NIL {
        @expect(m.less(m.keys[i], m.keys[@as(usize, m.right[i])]));
    }
    return @max(hl, hr) + 1;
}

test "ordered maps" {
    let mut m = TreeMap.[u64, u64].init(lt);
    defer m.deinit();
    let mut x: u64 = 1;
    let mut keys: [2000]u64 = [0; 2000];
    for i in @as(usize, 0)..2000 {
        x = (x *% 6364136223846793005) +% 1442695040888963407;
        keys[i] = (x >> 33) % 100000;
        m.put(keys[i], @as(u64, i))!;
    }
    _ = check(&m, m.root);
    // in order, and the values are the last put for each key
    let mut it = m.iter();
    let mut k: u64 = 0;
    let mut v: u64 = 0;
    let mut prev: u64 = 0;
    let mut n: usize = 0;
    while it.next(&mut k, &mut v) {
        if n > 0 {
            @expect(k > prev);
        }
        @expect_eq(keys[@truncate(usize, v)], k);
        prev = k;
        n += 1;
    }
    @expect_eq(n, m.len());
    // remove half, checking balance along the way
    for i in @as(usize, 0)..1000 {
        _ = m.remove(keys[i]);
        if (i % 97) == 0 {
            _ = check(&m, m.root);
        }
    }
    _ = check(&m, m.root);
    for i in @as(usize, 0)..1000 {
        @expect(not m.contains(keys[i]));
    }
    // reuse of removed nodes
    let used = m.used;
    m.put(123456789, 1)!;
    @expect_eq(m.used, used);
    @expect_eq(m.max()!, 123456789);
    @expect_eq(m.take(123456789)!, 1);
    // floor, ceil, iter_from
    let mut s = TreeMap.[u64, u64].init(lt);
    defer s.deinit();
    for i in @as(u64, 1)..11 {
        s.put(i * 10, i)!;
    }
    @expect_eq(s.floor(35)!, 30);
    @expect_eq(s.ceil(35)!, 40);
    @expect_eq(s.floor(30)!, 30);
    @expect_eq(s.min()!, 10);
    @expect(failed(s.floor(5)));
    @expect(failed(s.ceil(101)));
    let mut r = s.iter_from(55);
    let mut sum: u64 = 0;
    while r.next(&mut k, &mut v) {
        sum += k;
    }
    @expect_eq(sum, 60 + 70 + 80 + 90 + 100);
    (s.get_ptr(10)!).* = 99;
    @expect_eq(s.get(10)!, 99);
    s.clear();
    @expect_eq(s.len(), 0);
    @expect(failed(s.min()));
}

test "ordered maps of strings" {
    let mut m = TreeMap.[[]u8, i32].init(str_lt);
    defer m.deinit();
    m.put("pear", 1)!;
    m.put("apple", 2)!;
    m.put("fig", 3)!;
    let mut it = m.iter();
    let mut k: []u8 = "";
    let mut v: i32 = 0;
    @expect(it.next(&mut k, &mut v));
    @expect_eq(k, "apple");
    @expect(it.next(&mut k, &mut v));
    @expect_eq(k, "fig");
}

fn failed(r: !u64) bool {
    _ = r catch return true;
    return false;
}
