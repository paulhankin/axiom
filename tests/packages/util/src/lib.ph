pub fn clamp(x: i64, lo: i64, hi: i64) i64 {
    if x < lo {
        return lo;
    }
    if x > hi {
        return hi;
    }
    return x;
}

test "clamp" {
    @expect_eq(clamp(5, 0, 3), 3);
    @expect_eq(clamp(-5, 0, 3), 0);
}
