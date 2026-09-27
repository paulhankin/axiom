// Inline WebAssembly (spec §14).
import "../lib/io.ph" as io;

fn clz(val: u32) u32 {
    return wasm -> u32 {
        local.get $val
        i32.clz
    };
}

fn mul_add(a: i64, b: i64, c: i64) i64 {
    return wasm -> i64 {
        local.get $a
        local.get $b
        i64.mul
        local.get $c
        i64.add
    };
}

fn shl_wasm(x: u32, k: u32) u32 {
    // raw wasm shifts keep WebAssembly's modulo semantics
    return wasm -> u32 {
        local.get $x
        local.get $k
        i32.shl
    };
}

export fn run() void {
    io.int(@as(i64, clz(1)));                  // expect: 31
    io.int(mul_add(6, 7, -2));                 // expect: 40
    io.int(@as(i64, shl_wasm(1, 33)));         // expect: 2
    let mut acc: f64 = 0.0;
    let x: f64 = 2.25;
    wasm {
        local.get $x
        f64.sqrt
        f64.const 1.0
        f64.add
        local.set $acc
    }
    io.float(acc);                             // expect: 2.5
    let mut buf: [4]u32 = [0, 0, 0, 0];
    let p = &mut buf;
    wasm {
        local.get $p
        i32.const 77
        i32.store offset=8
    }
    io.int(@as(i64, buf[2]));                  // expect: 77
}
