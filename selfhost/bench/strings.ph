// Byte scanning: tokenizing a generated text into words, hashing them
// (FNV-1a) and counting in an open-addressing table of slices.
import "common.ph" as c;
import "std/mem" as mem;

fn fnv(s: []u8) u64 {
    let mut h: u64 = 0xcbf29ce484222325;
    for b in s {
        h = (h ^ @as(u64, b)) *% 0x100000001b3;
    }
    return h;
}

fn same(a: []u8, b: []u8) bool {
    if a.len != b.len {
        return false;
    }
    for i in 0..a.len {
        if a[i] != b[i] {
            return false;
        }
    }
    return true;
}

export fn main() i32 {
    let n: usize = 30_000_000;
    let text = mem.alloc.[u8](n) catch return 1;
    let mut s: u32 = 12345;
    for i in 0..n {
        s = (s *% 1103515245) +% 12345;
        let r = (s >> 16) % 8;
        if r == 0 {
            text[i] = ' ';
        } else {
            text[i] = 'a' + @truncate(u8, (s >> 8) % 4);
        }
    }
    let cap: usize = 1 << 16;
    let keys = mem.alloc.[[]u8](cap) catch return 1;
    let counts = mem.alloc.[u64](cap) catch return 1;
    for i in 0..cap {
        keys[i] = text[0..0];
        counts[i] = 0;
    }
    let mut i: usize = 0;
    let mut words: u64 = 0;
    while i < n {
        while (i < n) and (text[i] == ' ') {
            i += 1;
        }
        let start = i;
        while (i < n) and (text[i] != ' ') {
            i += 1;
        }
        if i > start {
            let w = text[start..i];
            let mut j = @truncate(usize, fnv(w)) & (cap - 1);
            loop {
                if counts[j] == 0 {
                    keys[j] = w;
                    counts[j] = 1;
                    break;
                }
                if same(keys[j], w) {
                    counts[j] += 1;
                    break;
                }
                j = (j + 1) & (cap - 1);
            }
            words += 1;
        }
    }
    let mut distinct: u64 = 0;
    for k in 0..cap {
        if counts[k] != 0 {
            distinct += 1;
        }
    }
    c.result("words", (words * 1000000) + distinct);
    return 0;
}
