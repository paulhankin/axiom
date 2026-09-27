// expect-error: E0907
fn f(x: u32) u32 {
    return wasm -> u32 {
        local.get $x
        local.get $x
    };
}
