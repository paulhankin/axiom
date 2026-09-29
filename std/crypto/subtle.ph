// =============================================================================
// std/crypto/subtle.ph - comparisons that take the same time whatever the
// data, for secrets such as MACs.
// =============================================================================

/// Whether a and b are equal, in time depending only on their lengths.
pub fn equal(a: []u8, b: []u8) bool {
    if a.len != b.len {
        return false;
    }
    let mut d: u8 = 0;
    for i in 0..a.len {
        d = d | (a[i] ^ b[i]);
    }
    return d == 0;
}

/// 1 if x == 0, else 0, without branching.
pub fn is_zero32(x: u32) u32 {
    return @truncate(u32, (@as(u64, x) -% 1) >> 63);
}

/// Overwrites a secret with zeros (before its memory is freed or reused).
pub fn zero(b: []mut u8) void {
    @mem_set(b, 0);
}

test "equal" {
    @expect(equal("abc", "abc"));
    @expect(not equal("abc", "abd"));
    @expect(not equal("abc", "ab"));
    @expect_eq(is_zero32(0), 1);
    @expect_eq(is_zero32(5), 0);
}
