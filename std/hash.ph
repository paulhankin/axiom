// =============================================================================
// std/hash.ph - hash and equality functions for std/map.
// These are not cryptographic.
// =============================================================================

/// splitmix64 finalizer: a fast, well-mixed hash for integers.
pub fn hash_u64(x: u64) u64 {
    let mut z = x +% 0x9E3779B97F4A7C15;
    z = (z ^ (z >> 30)) *% 0xBF58476D1CE4E5B9;
    z = (z ^ (z >> 27)) *% 0x94D049BB133111EB;
    return z ^ (z >> 31);
}

pub fn hash_u32(x: u32) u64 {
    return hash_u64(@as(u64, x));
}

pub fn hash_i64(x: i64) u64 {
    return hash_u64(@bit_cast(u64, x));
}

pub fn hash_i32(x: i32) u64 {
    return hash_u64(@bit_cast(u64, @as(i64, x)));
}

pub fn hash_usize(x: usize) u64 {
    return hash_u64(@as(u64, x));
}

/// FNV-1a over the bytes, followed by a finalizer.
pub fn hash_bytes(s: []u8) u64 {
    let mut h: u64 = 0xcbf29ce484222325;
    for b in s {
        h = (h ^ @as(u64, b)) *% 0x100000001b3;
    }
    return hash_u64(h);
}

pub fn eq_u64(a: u64, b: u64) bool {
    return a == b;
}

pub fn eq_u32(a: u32, b: u32) bool {
    return a == b;
}

pub fn eq_i64(a: i64, b: i64) bool {
    return a == b;
}

pub fn eq_i32(a: i32, b: i32) bool {
    return a == b;
}

pub fn eq_usize(a: usize, b: usize) bool {
    return a == b;
}

pub fn eq_bytes(a: []u8, b: []u8) bool {
    return @mem_eq(a, b);
}

// -----------------------------------------------------------------------------
// Tests (phalange test std/hash.ph)
// -----------------------------------------------------------------------------

test "hashes are deterministic and spread" {
    @expect_eq(hash_u64(42), hash_u64(42));
    @expect(hash_u64(1) != hash_u64(2));
    @expect_eq(hash_bytes("abc"), hash_bytes("abc"));
    @expect(hash_bytes("abc") != hash_bytes("abd"));
    @expect(eq_bytes("xy", "xy"));
    @expect(not eq_bytes("xy", "x"));
}
