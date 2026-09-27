// defer ordering: reverse, per block, on return / break / error.
import "../lib/io.ph" as io;

error { Fail }

fn a() void {
    defer io.str("a1");
    defer io.str("a2");
    {
        defer io.str("inner");
        io.str("body");
    }
    io.str("end");
}

fn b(fail: bool) !u32 {
    defer io.str("b-cleanup");
    if fail {
        return error.Fail;
    }
    return 5;
}

fn c() void {
    for i in @as(u32, 0)..3 {
        defer io.int(@as(i64, i));
        if i == 1 {
            continue;
        }
        if i == 2 {
            break;
        }
        io.str("iter");
    }
}

fn d() u32 {
    let mut x: u32 = 1;
    defer x = 100;
    return x;
}

fn e2() !u32 {
    defer io.str("e2-defer");
    let v = b(true)!;
    return v;
}

export fn run() void {
    a();
    // expect: body
    // expect: inner
    // expect: end
    // expect: a2
    // expect: a1
    _ = b(false) catch 0;
    // expect: b-cleanup
    c();
    // expect: iter
    // expect: 0
    // expect: 1
    // expect: 2
    io.int(@as(i64, d()));   // expect: 1
    _ = e2() catch 0;
    // expect: b-cleanup
    // expect: e2-defer
}
