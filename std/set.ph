// =============================================================================
// std/set.ph - hash sets, on std/map.ph's Swiss tables.
//
//     let mut s = set.Set.[u64].init(hash.hash_u64, hash.eq_u64);
//     defer s.deinit();
//     _ = s.add(7)!;              // true: it was new
//     if s.contains(7) { ... }
//     let mut it = s.iter();
//     let mut k: u64 = 0;
//     while it.next(&mut k) { ... }
//
// As with maps, []u8 elements are stored as slices: keep the bytes alive.
// =============================================================================

import "hash.ph" as hash;
import "map.ph" as maps;

error { OutOfMemory }

pub struct Set[K] {
    m: maps.Map.[K, u8],

    pub fn init(hash_fn: fn(K) u64, eq_fn: fn(K, K) bool) Self {
        return Self.{ m: maps.Map.[K, u8].init(hash_fn, eq_fn) };
    }

    pub fn deinit(self: *mut Self) void {
        self.m.deinit();
    }

    pub fn len(self: *Self) usize {
        return self.m.len();
    }

    /// Adds k; returns whether it was not there before.
    pub fn add(self: *mut Self, k: K) !bool {
        if self.m.contains(k) {
            return false;
        }
        self.m.put(k, 0)!;
        return true;
    }

    pub fn contains(self: *Self, k: K) bool {
        return self.m.contains(k);
    }

    /// Removes k; returns whether it was there.
    pub fn remove(self: *mut Self, k: K) bool {
        return self.m.remove(k);
    }

    pub fn clear(self: *mut Self) void {
        self.m.clear();
    }

    /// The elements in unspecified order. Do not change the set meanwhile.
    pub fn iter(self: *Self) Iter.[K] {
        return Iter.[K].{ it: self.m.iter() };
    }

    /// Adds every element of other.
    pub fn add_all(self: *mut Self, other: *Self) !void {
        for i in @as(usize, 0)..other.m.ctrl.len {
            if other.m.ctrl[i] < 0x80 {
                _ = self.add(other.m.keys[i])!;
            }
        }
    }

    /// Keeps only the elements that are also in other.
    pub fn keep_common(self: *mut Self, other: *Self) void {
        self.keep_where(other, true);
    }

    /// Removes the elements that are in other.
    pub fn remove_all(self: *mut Self, other: *Self) void {
        self.keep_where(other, false);
    }

    fn keep_where(self: *mut Self, other: *Self, inside: bool) void {
        // (removing while iterating a Swiss table is safe: slots only
        // become EMPTY or DELETED, and the scan goes by slot)
        for i in @as(usize, 0)..self.m.ctrl.len {
            if self.m.ctrl[i] < 0x80 {
                let k = self.m.keys[i];
                if other.contains(k) != inside {
                    _ = self.m.remove(k);
                }
            }
        }
    }

    /// Whether every element is also in other.
    pub fn is_subset(self: *Self, other: *Self) bool {
        if self.len() > other.len() {
            return false;
        }
        for i in @as(usize, 0)..self.m.ctrl.len {
            if (self.m.ctrl[i] < 0x80) and not other.contains(self.m.keys[i]) {
                return false;
            }
        }
        return true;
    }
}

pub struct Iter[K] {
    it: maps.Iter.[K, u8],

    /// Stores the next element through k and returns true, or returns false
    /// at the end.
    pub fn next(self: *mut Self, k: *mut K) bool {
        let mut v: u8 = 0;
        return self.it.next(k, &mut v);
    }
}

// -----------------------------------------------------------------------------
// Tests
// -----------------------------------------------------------------------------

test "sets" {
    let mut s = Set.[u64].init(hash.hash_u64, hash.eq_u64);
    defer s.deinit();
    for i in @as(u64, 0)..1000 {
        @expect(s.add(i * 3)!);
    }
    @expect(not s.add(3)!);
    @expect_eq(s.len(), 1000);
    @expect(s.contains(2997));
    @expect(not s.contains(2998));
    @expect(s.remove(3));
    @expect(not s.remove(3));
    let mut sum: u64 = 0;
    let mut it = s.iter();
    let mut k: u64 = 0;
    while it.next(&mut k) {
        sum += k;
    }
    @expect_eq(sum, (3 * ((999 * 1000) / 2)) - 3);
    let mut t = Set.[u64].init(hash.hash_u64, hash.eq_u64);
    defer t.deinit();
    for i in @as(u64, 0)..10 {
        _ = t.add(i)!;
    }
    let mut u = Set.[u64].init(hash.hash_u64, hash.eq_u64);
    defer u.deinit();
    let empty = Set.[u64].init(hash.hash_u64, hash.eq_u64);
    u.add_all(&empty)!;
    u.add_all(&t)!;
    u.keep_common(&s);  // 0, 6, 9
    @expect_eq(u.len(), 3);
    @expect(u.is_subset(&t));
    @expect(not t.is_subset(&u));
    t.remove_all(&u);
    @expect_eq(t.len(), 7);
    @expect(not t.contains(6));
}

test "sets of strings" {
    let mut s = Set.[[]u8].init(hash.hash_bytes, hash.eq_bytes);
    defer s.deinit();
    _ = s.add("apple")!;
    _ = s.add("pear")!;
    @expect(not s.add("apple")!);
    @expect(s.contains("pear"));
    @expect_eq(s.len(), 2);
}
