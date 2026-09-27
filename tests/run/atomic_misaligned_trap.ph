// Misaligned atomic accesses trap on every target.
// expect-trap
// expect: ok
import "../lib/io.ph" as io;

export fn run() void {
    let mut words: [2]u64 = [0, 0];
    let base = @int_from_ptr(&mut words);
    let p = @ptr_from_int(*mut u32, base + 4);
    _ = @atomic_add(p, 1);
    io.str("ok");
    let q = @ptr_from_int(*mut u32, base + 2);
    _ = @atomic_add(q, 1);
}
