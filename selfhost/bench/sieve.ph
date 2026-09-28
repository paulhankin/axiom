// Sieve of Eratosthenes: byte arrays, bounds-checked indexing, nested loops.
import "common.ph" as c;
import "std/mem" as mem;

export fn main() i32 {
    let n: usize = 20_000_000;
    let flags = mem.alloc.[u8](n + 1) catch return 1;
    let mut total: u64 = 0;
    for _round in 0..@as(usize, 5) {
        for i in 0..(n + 1) {
            flags[i] = 1;
        }
        flags[0] = 0;
        flags[1] = 0;
        let mut p: usize = 2;
        while (p * p) <= n {
            if flags[p] != 0 {
                let mut m = p * p;
                while m <= n {
                    flags[m] = 0;
                    m += p;
                }
            }
            p += 1;
        }
        for i in 0..(n + 1) {
            total += @as(u64, flags[i]);
        }
    }
    c.result("primes", total);
    return 0;
}
