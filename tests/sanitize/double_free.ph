// status: trap
// stderr: double free of 8 bytes
// stderr: first freed:
import "../../std/mem.ph" as mem;

export fn main() i32 {
    let p = mem.create(@as(u64, 1)) catch return 1;
    mem.destroy(p);
    mem.destroy(p);
    return 0;
}
