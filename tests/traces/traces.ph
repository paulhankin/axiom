// Stack traces and error return traces (spec §17.4), checked by tests/run.mjs:
//   // stack: <test> | <function>@<line> ...     innermost first; ~N = a repeat marker
//   // errors: <test> | <function>@<line> ...    oldest first
error { Bad }

fn get(xs: []i32, i: usize) i32 {
    return xs[i];
}

fn check(x: i32) !i32 {
    if x > 3 {
        return error.Bad;
    }
    return x;
}

fn outer(x: i32) !i32 {
    let y = check(x)!;
    return y;
}

fn handled(x: i32) !i32 {
    // the error from check(5) is handled, so it leaves no trace
    let a = check(5) catch 0;
    if x > 0 {
        return error.Bad;
    }
    return a;
}

fn deep(n: u32) u32 {
    if n == 0 {
        let xs: [1]i32 = [0];
        return @bit_cast(u32, get(xs[..], 3));
    }
    return deep(n - 1) + 1;
}

fn forever(n: u64) u64 {
    return forever(n + 1) + 1;
}

fn call_through(f: fn([]i32, usize) i32) i32 {
    let xs: [1]i32 = [0];
    return f(xs[..], 1);
}

// stack: bounds | get@7 test "bounds"@51
test "bounds" {
    let xs: [2]i32 = [1, 2];
    _ = get(xs[..], 5);
}

// errors: error | check@12 outer@18 test "error"@56
test "error" {
    _ = outer(9)!;
}

// errors: catch clears the trace | handled@26 test "catch clears the trace"@61
test "catch clears the trace" {
    _ = handled(1)!;
}

// stack: recursion | get@7 deep@34 deep@36 ~99 test "recursion"@66
test "recursion" {
    _ = deep(100);
}

// stack: indirect call | get@7 call_through@45 test "indirect call"@71
test "indirect call" {
    _ = call_through(get);
}

// stack: stack overflow | forever forever@40 ~255
test "stack overflow" {
    _ = forever(0);
}

fn first(xs: []i32) i32 {
    return xs[0];
}

fn mid(xs: []i32, i: usize) i32 {
    let a = first(xs);
    return a + get(xs, i);
}

// stack: nested inlining | get@7 mid@85 test "nested inlining"@91
test "nested inlining" {
    let xs: [2]i32 = [1, 2];
    _ = mid(xs[..], 4);
}
