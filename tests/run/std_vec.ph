// std/vec
import "../lib/io.ph" as io;
import "std/mem" as mem;
import "std/vec" as vec;

fn less(a: i32, b: i32) bool {
    return a < b;
}

export fn run() void {
    let mut arena = mem.Arena.init(mem.page_allocator);
    defer arena.deinit();
    let mut v = vec.Vec.[i32].init(arena.allocator());
    defer v.deinit();
    for i in @as(i32, 0)..100 {
        v.push((i * 37) % 101) catch return;
    }
    io.uint(@as(u64, v.len));         // expect: 100
    io.boolean(v.capacity() >= 100);  // expect: true
    v.sort(less);
    io.int(@as(i64, v.get(0)));   // expect: 0
    io.int(@as(i64, v.get(99)));  // expect: 100
    let mut sorted = true;
    for i in 1..v.len {
        if v.get(i - 1) > v.get(i) {
            sorted = false;
        }
    }
    io.boolean(sorted);  // expect: true
    io.uint(@as(u64, vec.binary_search(v.items(), 50, less) catch 999));  // expect: 50
    v.insert(0, -5) catch return;
    io.int(@as(i64, v.get(0)));          // expect: -5
    io.int(@as(i64, v.remove(0)));       // expect: -5
    io.int(@as(i64, v.swap_remove(0)));  // expect: 0
    io.int(@as(i64, v.get(0)));          // expect: 100
    io.int(@as(i64, v.pop() catch -1));  // expect: 99
    let extra: [3]i32 = [7, 8, 9];
    v.extend(extra[..]) catch return;
    io.int(@as(i64, v.last() catch -1));  // expect: 9
    v.get_ptr(0).* = 42;
    io.int(@as(i64, v.get(0)));  // expect: 42
    v.truncate(2);
    vec.reverse(v.items());
    io.int(@as(i64, v.get(0)));  // expect: 1
    io.uint(@as(u64, vec.index_of(v.items(), 42) catch 999));  // expect: 1
    v.clear();
    _ = v.pop() catch |e| {
        io.str(@error_name(e));  // expect: Empty
        return;
    };
}
