// The global allocator under threads: each thread allocates and frees its
// own objects, and hands some to another thread to free.
import "../lib/io.ph" as io;
import "std/mem" as mem;
import "std/sync" as sync;
import "std/thread" as thread;

struct Work {
    seed: u64,
    ch: *mut sync.Channel.[[]mut u64],
    sum: u64,
}

fn rng(s: *mut u64) u64 {
    s.* ^= s.* << 13;
    s.* ^= s.* >> 7;
    s.* ^= s.* << 17;
    return s.*;
}

fn worker(w: *mut Work) void {
    let mut seed = w.seed;
    let live = mem.alloc.[[]mut u64](256) catch return;
    for i in 0..live.len {
        live[i] = mem.alloc.[u64](1) catch return;
        live[i][0] = 1;
    }
    for r in @as(u64, 0)..20000 {
        let i = @truncate(usize, rng(&mut seed) % 256);
        w.sum += live[i][live[i].len - 1];
        if (r % 16) == 0 {
            w.ch.send(live[i]) catch return;  // freed by the receiver
        } else {
            mem.free(live[i]);
        }
        let n = 1 + @truncate(usize, rng(&mut seed) % 3000);
        live[i] = mem.alloc.[u64](n) catch return;
        live[i][n - 1] = 1;
    }
    for i in 0..live.len {
        mem.free(live[i]);
    }
    mem.free(live);
}

fn drain(ch: *mut sync.Channel.[[]mut u64]) void {
    loop {
        let s = ch.recv() catch return;
        mem.free(s);
    }
}

export fn run() void {
    let base = mem.in_use();
    let mut ch = sync.Channel.[[]mut u64].init(64) catch return;
    let d = thread.spawn(drain, &mut ch) catch return;
    let mut ws: [4]Work = [Work.{ seed: 1, ch: &mut ch, sum: 0 }; 4];
    for i in 0..ws.len {
        ws[i].seed = 0x9E37_79B9 * (@as(u64, i) + 1);
    }
    let t1 = thread.spawn(worker, &mut ws[0]) catch return;
    let t2 = thread.spawn(worker, &mut ws[1]) catch return;
    let t3 = thread.spawn(worker, &mut ws[2]) catch return;
    worker(&mut ws[3]);
    t1.join();
    t2.join();
    t3.join();
    ch.close();
    d.join();
    ch.deinit();
    io.uint(ws[0].sum + ws[1].sum + ws[2].sum + ws[3].sum);  // expect: 80000
    io.boolean(mem.in_use() == base);  // expect: true
}
