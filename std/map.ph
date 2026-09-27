// =============================================================================
// std/map.ph - hash maps (open addressing, linear probing).
//
// Phalange has no traits, so a map is given its hash and equality functions
// when it is created:
//
//     let mut m = map.Map.[u64, u32].init(alloc, hash.hash_u64, hash.eq_u64);
//
// Keys and values are stored by value. For []u8 keys only the slice is
// stored, not the bytes it points to: keep the bytes alive (e.g. in an Arena).
// =============================================================================

import "mem.ph" as mem;

error { NotFound, OutOfMemory }

const EMPTY: u8 = 0;
const FULL: u8 = 1;
const DELETED: u8 = 2;

pub struct Map[K, V] {
    alloc: mem.Allocator,
    hash_fn: fn(K) u64,
    eq_fn: fn(K, K) bool,
    keys: []mut K,
    vals: []mut V,
    states: []mut u8,  // capacity == states.len, always 0 or a power of two
    count: usize,      // FULL slots
    used: usize,       // FULL + DELETED slots

    pub fn init(alloc: mem.Allocator, hash_fn: fn(K) u64, eq_fn: fn(K, K) bool) Self {
        return Self.{
            alloc: alloc,
            hash_fn: hash_fn,
            eq_fn: eq_fn,
            keys: @slice_from_ptr(@ptr_from_int(*mut K, @align_of(K)), 0),
            vals: @slice_from_ptr(@ptr_from_int(*mut V, @align_of(V)), 0),
            states: @slice_from_ptr(@ptr_from_int(*mut u8, 1), 0),
            count: 0,
            used: 0,
        };
    }

    pub fn deinit(self: *mut Self) void {
        if self.states.len > 0 {
            self.alloc.free(self.keys);
            self.alloc.free(self.vals);
            self.alloc.free(self.states);
        }
        self.keys = @slice_from_ptr(@ptr_from_int(*mut K, @align_of(K)), 0);
        self.vals = @slice_from_ptr(@ptr_from_int(*mut V, @align_of(V)), 0);
        self.states = @slice_from_ptr(@ptr_from_int(*mut u8, 1), 0);
        self.count = 0;
        self.used = 0;
    }

    pub fn len(self: *Self) usize {
        return self.count;
    }

    pub fn capacity(self: *Self) usize {
        return self.states.len;
    }

    fn start(self: *Self, k: K) usize {
        return @truncate(usize, self.hash_fn(k)) & (self.states.len - 1);
    }

    fn find(self: *Self, k: K) !usize {
        if self.count == 0 {
            return error.NotFound;
        }
        let mask = self.states.len - 1;
        let mut i = self.start(k);
        loop {
            let st = self.states[i];
            if st == EMPTY {
                return error.NotFound;
            }
            if (st == FULL) and self.eq_fn(self.keys[i], k) {
                return i;
            }
            i = (i + 1) & mask;
        }
    }

    // Rehash into a table of `cap` slots (a power of two > count).
    fn rehash(self: *mut Self, cap: usize) !void {
        let nk = self.alloc.alloc.[K](cap)!;
        let nv = self.alloc.alloc.[V](cap) catch |e| {
            self.alloc.free(nk);
            return e;
        };
        let ns = self.alloc.alloc.[u8](cap) catch |e| {
            self.alloc.free(nk);
            self.alloc.free(nv);
            return e;
        };
        @mem_set(ns, EMPTY);
        let mask = cap - 1;
        for i in 0..self.states.len {
            if self.states[i] == FULL {
                let mut j = @truncate(usize, self.hash_fn(self.keys[i])) & mask;
                while ns[j] != EMPTY {
                    j = (j + 1) & mask;
                }
                nk[j] = self.keys[i];
                nv[j] = self.vals[i];
                ns[j] = FULL;
            }
        }
        if self.states.len > 0 {
            self.alloc.free(self.keys);
            self.alloc.free(self.vals);
            self.alloc.free(self.states);
        }
        self.keys = nk;
        self.vals = nv;
        self.states = ns;
        self.used = self.count;
    }

    /// Inserts or replaces the value for `k`.
    pub fn put(self: *mut Self, k: K, v: V) !void {
        if ((self.used + 1) * 4) > (self.states.len * 3) {
            let mut cap = @max(self.states.len, 8);
            if ((self.count + 1) * 2) > cap {
                cap = @mul_checked(cap, 2) catch return error.OutOfMemory;
            }
            self.rehash(cap)!;
        }
        let mask = self.states.len - 1;
        let mut i = self.start(k);
        let mut tomb: usize = 0;
        let mut have_tomb = false;
        loop {
            let st = self.states[i];
            if st == EMPTY {
                break;
            }
            if st == DELETED {
                if not have_tomb {
                    tomb = i;
                    have_tomb = true;
                }
            } else if self.eq_fn(self.keys[i], k) {
                self.vals[i] = v;
                return;
            }
            i = (i + 1) & mask;
        }
        if have_tomb {
            i = tomb;
        } else {
            self.used += 1;
        }
        self.keys[i] = k;
        self.vals[i] = v;
        self.states[i] = FULL;
        self.count += 1;
    }

    pub fn get(self: *Self, k: K) !V {
        let i = self.find(k)!;
        return self.vals[i];
    }

    /// Pointer to the value for `k`; valid until the map is next modified.
    pub fn get_ptr(self: *mut Self, k: K) !*mut V {
        let i = self.find(k)!;
        return &mut self.vals[i];
    }

    pub fn contains(self: *Self, k: K) bool {
        _ = self.find(k) catch return false;
        return true;
    }

    /// Removes `k`; returns whether it was present.
    pub fn remove(self: *mut Self, k: K) bool {
        let i = self.find(k) catch return false;
        self.states[i] = DELETED;
        self.count -= 1;
        return true;
    }

    pub fn clear(self: *mut Self) void {
        @mem_set(self.states, EMPTY);
        self.count = 0;
        self.used = 0;
    }

    /// Iterates over entries in unspecified order. Do not modify the map
    /// while iterating.
    pub fn iter(self: *Self) Iter.[K, V] {
        return Iter.[K, V].{ map: self, i: 0 };
    }
}

pub struct Iter[K, V] {
    map: *Map.[K, V],
    i: usize,

    /// Stores the next entry through `key` and `val` and returns true, or
    /// returns false when there are no more entries.
    pub fn next(self: *mut Self, key: *mut K, val: *mut V) bool {
        while self.i < self.map.states.len {
            let j = self.i;
            self.i += 1;
            if self.map.states[j] == FULL {
                key.* = self.map.keys[j];
                val.* = self.map.vals[j];
                return true;
            }
        }
        return false;
    }
}
