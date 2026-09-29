// status: trap
// stderr: free of memory that was not allocated
import "../../std/mem.ph" as mem;

let mut table: [8]u32 = [0; 8];

export fn main() i32 {
    mem.free(table[..]);
    return 0;
}
