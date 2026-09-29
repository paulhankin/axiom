// Memory never freed is reported when main returns, which then exits with
// status 23. Memory allocated while leaks are ignored is not reported.
// status: 23
// stderr: memory leaks
// stderr: leak of 24 bytes
// stderr: at keep (tests/sanitize/leak.ph:17:16)
// stderr: 24 bytes leaked in 1 allocations
import "../../std/mem.ph" as mem;

let mut kept: usize = 0;

fn keep() !void {
    mem.leaks_ignored(true);
    let cache = mem.alloc.[u8](100)!;
    mem.leaks_ignored(false);
    kept = @int_from_ptr(cache.ptr);
    let lost = mem.alloc.[u64](3)!;
    lost[0] = 1;
}

export fn main() i32 {
    keep() catch return 1;
    let fine = mem.alloc.[u8](50) catch return 1;
    mem.free(fine);
    return 0;
}
