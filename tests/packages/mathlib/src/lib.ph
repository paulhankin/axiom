import "extra/stats.ph" as stats;
import "pkg:util" as util;

pub fn square(x: i64) i64 {
    return x * x;
}

pub fn clamped_square(x: i64) i64 {
    return util.clamp(square(x), 0, 100);
}

pub fn mean(xs: []i64) i64 {
    return stats.sum(xs) / @bit_cast(i64, @as(u64, xs.len));
}

test "square" {
    @expect_eq(square(7), 49);
    @expect_eq(clamped_square(20), 100);
}
