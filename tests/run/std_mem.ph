// The standard allocators.
import "../lib/io.ph" as io;
import "std/mem" as mem;

let mut BUF: [256]u8 = [0; 256];

export fn run() void {
    let mut fb = mem.FixedBuffer.init(BUF[..]);
    let a = fb.allocator();
    let xs = a.alloc.[u64](8) catch return;
    for i in 0..xs.len {
        xs[i] = @as(u64, i) * 3;
    }
    io.uint(xs[7]);  // expect: 21
    let too_big = a.alloc.[u64](100) catch |e| {
        io.str(@error_name(e));  // expect: OutOfMemory
        return;
    };
    _ = too_big;
}
