// =============================================================================
// File: wordcount.ph
// Counts word frequencies on stdin and prints the most common words.
//
//   phalange build examples/wordcount.ph -O2 -o wc.wasm
//   node runtime/host.mjs wc.wasm < some.txt          (or --target=native)
// =============================================================================

import "std/mem" as mem;
import "std/io" as io;
import "std/vec" as vec;
import "std/map" as map;
import "std/hash" as hash;

struct Entry {
    word: []u8,
    count: u32,
}

fn more_frequent(a: Entry, b: Entry) bool {
    return a.count > b.count;
}

fn is_letter(c: u8) bool {
    return ((c >= 'a') and (c <= 'z')) or ((c >= 'A') and (c <= 'Z'));
}

fn lower(c: u8) u8 {
    if (c >= 'A') and (c <= 'Z') {
        return c + 32;
    }
    return c;
}

fn count_words() !void {
    let mut arena = mem.Arena.init(mem.page_allocator);
    defer arena.deinit();
    let al = arena.allocator();

    let text = io.stdin().read_all(al)!;
    for i in 0..text.len {
        text[i] = lower(text[i]);
    }

    let mut counts = map.Map.[[]u8, u32].init(al, hash.hash_bytes, hash.eq_bytes);
    let mut start: usize = 0;
    for i in 0..text.len + 1 {
        let boundary = (i == text.len) or not is_letter(text[i]);
        if boundary {
            if i > start {
                let w = text[start..i];
                let c = counts.get(w) catch 0;
                counts.put(w, c + 1)!;
            }
            start = i + 1;
        }
    }

    let mut entries = vec.Vec.[Entry].init(al);
    let mut it = counts.iter();
    let mut w: []u8 = "";
    let mut c: u32 = 0;
    while it.next(&mut w, &mut c) {
        entries.push(Entry.{ word: w, count: c })!;
    }
    entries.sort(more_frequent);

    let mut out = io.BufWriter.init(io.stdout());
    let o = out.writer();
    let shown = @min(entries.len, 10);
    for i in 0..shown {
        let e = entries.get(i);
        o.print_u64(@as(u64, e.count))!;
        o.print(" ")!;
        o.println(e.word)!;
    }
    o.print_u64(@as(u64, counts.len()))!;
    o.println(" distinct words")!;
    out.flush()!;
}

export fn main() i32 {
    count_words() catch |e| {
        io.stderr().println(@error_name(e)) catch {};
        return 1;
    };
    return 0;
}
