// =============================================================================
// std/crypto/random.ph - cryptographically secure random bytes from the
// operating system (getrandom on Linux, crypto.getRandomValues under node).
// =============================================================================

error { RandomFailed }

import host "phalange" {
    fn ph_random(p: *mut u8, n: usize) i32;
}

/// Fills buf with random bytes.
pub fn fill(buf: []mut u8) !void {
    if buf.len == 0 {
        return;
    }
    if ph_random(buf.ptr, buf.len) != 0 {
        return error.RandomFailed;
    }
}

/// A random u64.
pub fn uint64() !u64 {
    let mut b: [8]u8 = [0; 8];
    fill(b[..])!;
    let mut v: u64 = 0;
    for x in b {
        v = (v << 8) | @as(u64, x);
    }
    return v;
}

/// A uniformly random integer in [0, n) (n > 0).
pub fn below(n: u64) !u64 {
    // reject the top partial range to avoid bias
    let limit = 0 -% ((0 -% n) % n);
    loop {
        let v = uint64()!;
        if (limit == 0) or (v < limit) {
            return v % n;
        }
    }
}

test "random bytes differ" {
    let mut a: [32]u8 = [0; 32];
    let mut b: [32]u8 = [0; 32];
    fill(a[..])!;
    fill(b[..])!;
    @expect(not @mem_eq(a[..], b[..]));
    for i in @as(u32, 0)..100 {
        @expect(below(7)! < 7);
    }
}
