pub fn sum(xs: []i64) i64 {
    let mut t: i64 = 0;
    for x in xs {
        t += x;
    }
    return t;
}

test "sum" {
    let xs: [3]i64 = [1, 2, 3];
    @expect_eq(sum(xs[..]), 6);
}
