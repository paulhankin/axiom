// Arena over page_allocator: many chunks, reset, deinit.
import "../lib/io.ph" as io;
import "std/mem" as mem;

export fn run() void {
    let mut arena = mem.Arena.init(mem.page_allocator);
    defer arena.deinit();
    let al = arena.allocator();
    let mut sum: u64 = 0;
    for round in @as(u64, 0)..50 {
        let block = al.alloc.[u64](1000) catch return;
        block[999] = round;
        sum += block[999];
    }
    io.uint(sum);  // expect: 1225
    arena.reset();
    let again = al.alloc.[u8](10) catch return;
    again[0] = 42;
    io.int(@as(i64, again[0]));  // expect: 42
}
