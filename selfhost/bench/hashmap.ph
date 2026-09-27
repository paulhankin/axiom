// std/map: inserts, lookups and removals of integer keys (hashing, probing,
// function pointers for hash and equality).
import "std/mem" as mem;
import "std/map" as map;
import "std/hash" as hash;
import "common.ph" as c;

export fn main() i32 {
    let mut arena = mem.Arena.init(mem.page_allocator);
    let al = arena.allocator();
    let mut m = map.Map.[u64, u64].init(al, hash.hash_u64, hash.eq_u64);
    let n: u64 = 1_000_000;
    for i in @as(u64, 0)..n {
        m.put(i *% 2654435761, i) catch return 1;
    }
    let mut sum: u64 = 0;
    for _r in 0..@as(u64, 4) {
        for i in @as(u64, 0)..n {
            sum +%= m.get(i *% 2654435761) catch 0;
        }
    }
    for i in @as(u64, 0)..n {
        if (i % 3) == 0 {
            _ = m.remove(i *% 2654435761);
        }
    }
    c.result("map", sum +% @as(u64, m.len()));
    return 0;
}
