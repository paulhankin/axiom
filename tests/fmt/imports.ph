// Imports: sorted, duplicates and unused ones removed, a missing standard
// library import added (mem is used below but was never imported).
import "std/vec" as vec;
import "std/io" as io;
import "std/hash" as hash;
import "std/vec" as vec;

fn make() !vec.Vec.[i32] {
    let v = vec.Vec.[i32].init();
    let scratch = mem.alloc.[u8](16)!;
    mem.free(scratch);
    return v;
}

fn show() void {
    io.stdout().print("hi\n") catch return;
}
