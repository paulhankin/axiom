// A program that overflows its stack: run by tests/run.mjs, which checks the
// crash report on stderr.
fn forever(n: u64) u64 {
    return forever(n + 1) + 1;
}

fn start() u64 {
    return forever(0);
}

export fn main() i32 {
    _ = start();
    return 0;
}
