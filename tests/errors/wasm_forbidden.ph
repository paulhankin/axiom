// expect-error: E0901
fn f() void {
    wasm {
        call 0
    }
}
