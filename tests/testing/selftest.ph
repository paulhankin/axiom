// Self-test of `phalange test`: each `// test-result:` line gives a test's
// expected status, failure code and failure line.
// test-result: addition works | pass
// test-result: strings compare | pass
// test-result: failing expectation | fail | T0011 | 39
// test-result: failing bool | fail | T0011 | 43
// test-result: string mismatch | fail | T0011 | 47
// test-result: error propagates | fail | error | 50
// test-result: bounds trap | fail | T0001 | 58
// test-result: expected trap | pass
// test-result: expected trap but none | fail | no_trap | 68
// test-result: overflow | pass
// test-result: wrong trap code | fail | T0003 | 79
// test-result: infinite loop | fail | timeout
// flags: --timeout=2000
import "std/io" as io;

error { Bad }

pub fn add(a: i32, b: i32) i32 {
    return a + b;
}

fn fails() !u32 {
    return error.Bad;
}

test "addition works" {
    @expect_eq(add(2, 3), 5);
    @expect(add(-1, 1) == 0);
}

test "strings compare" {
    @expect_eq("abc", "abc");
}

test "failing expectation" {
    io.stdout().println("some output") catch {};
    @expect_eq(add(2, 2), 5);
}

test "failing bool" {
    @expect(add(1, 1) == 3);
}

test "string mismatch" {
    @expect_eq("hello", "help");
}

test "error propagates" {
    _ = fails()!;
}

test "bounds trap" {
    let a: [2]u8 = [1, 2];
    let s = a[..];
    let i: usize = 5;
    _ = s[i];
}

test "expected trap" expect trap(T0001) {
    let a: [2]u8 = [1, 2];
    let s = a[..];
    let i: usize = 5;
    _ = s[i];
}

test "expected trap but none" expect trap {
    @expect(true);
}

test "overflow" expect trap(T0002) {
    let mut x: u8 = 255;
    x += 1;
}

test "wrong trap code" expect trap(T0001) {
    let z: u32 = 0;
    let q = 10 / z;
    _ = q;
}

test "infinite loop" {
    loop {
    }
}
