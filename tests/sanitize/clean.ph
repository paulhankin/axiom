// A correct program that uses much of std, from several threads: nothing
// is reported.
// status: 0
import "../../std/collections/map.ph" as map;
import "../../std/collections/vec.ph" as vec;
import "../../std/encoding/json.ph" as json;
import "../../std/fmt.ph" as fmt;
import "../../std/hash.ph" as hash;
import "../../std/str.ph" as strs;
import "../../std/thread.ph" as thread;

struct Job {
    seed: u64,
    out: u64,
}

fn work(j: *mut Job) void {
    j.out = run(j.seed);
}

fn run(seed: u64) u64 {
    let mut m = map.Map.[u64, u64].init(hash.hash_u64, hash.eq_u64);
    defer m.deinit();
    let mut v = vec.Vec.[u64].init();
    defer v.deinit();
    let mut total: u64 = 0;
    for i in @as(u64, 0)..2000 {
        m.put(i *% seed, i) catch return 0;
        v.push(i) catch return 0;
        if (i % 3) == 0 {
            _ = m.remove(i *% seed);
        }
    }
    for x in v.items() {
        total +%= m.get(x *% seed) catch 0;
    }
    let mut s = strs.String.init();
    defer s.deinit();
    fmt.write(s.writer(), "{} and {}", [fmt.int(total), fmt.str("text")]) catch return 0;
    let mut doc = json.parse("{\"a\": [1, 2, {\"b\": \"c\"}], \"d\": 1.5}") catch return 0;
    defer doc.deinit();
    return total +% @as(u64, s.len);
}

export fn main() i32 {
    let mut jobs: [4]Job = [Job.{ seed: 3, out: 0 }; 4];
    let mut ts: [4]thread.Thread.[*mut Job] = [thread.spawn(work, &mut jobs[0]) catch return 1; 4];
    for i in @as(usize, 1)..4 {
        jobs[i].seed = @as(u64, i) + 5;
        ts[i] = thread.spawn(work, &mut jobs[i]) catch return 1;
    }
    let mut sum: u64 = 0;
    for i in @as(usize, 0)..4 {
        ts[i].join();
        sum +%= jobs[i].out;
    }
    if sum == 0 {
        return 2;
    }
    return 0;
}
