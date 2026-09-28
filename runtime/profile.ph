// =============================================================================
// runtime/profile.ph - sampling profiler for native Phalange programs (spec
// §16.6). `phalange profile` builds it in and runs the program with
// PHALANGE_PROFILE=<raw output file> and PHALANGE_PROFILE_HZ=<rate>.
//
// A SIGPROF timer on the process CPU clock interrupts whichever thread is
// running; the handler copies that thread's shadow stack (§16.4) into a
// preallocated buffer. At exit the samples are aggregated and written in the
// raw format read by `phalange profile`:
//
//   # phalange-profile-raw 1
//   hz <rate>
//   clock cpu
//   S <count> <depth> <leaf> <site>...        (tab-separated, sites innermost first)
//
// where each <site> is <file id>\x1f<start>\x1f<end>\x1f<function>.
// =============================================================================
import "native.ph" as rt;
import "trap_format.ph" as tf;

extern "C" {
    fn getenv(name: *u8) *u8;
    fn atoi(s: *u8) i32;
    fn __cxa_atexit(f: fn(*mut void) void, arg: *mut void, dso: *mut void) i32;  // atexit is not in libc.so
    fn mmap(addr: *mut void, len: usize, prot: i32, flags: i32, fd: i32, off: i64) *mut void;
    fn calloc(n: usize, size: usize) *mut void;
    fn gettid() i32;
    fn getpid() i32;
    fn timer_create(clock: i32, sev: *SigEvent, timer: *mut usize) i32;
    fn timer_settime(timer: usize, flags: i32, nv: *ITimerSpec, old: *mut ITimerSpec) i32;
    fn timer_delete(timer: usize) i32;
}

struct SigEvent {
    value: u64,
    signo: i32,
    notify: i32,
    pad: [12]u64,
}

struct ITimerSpec {
    interval_sec: i64,
    interval_nsec: i64,
    value_sec: i64,
    value_nsec: i64,
}

struct Entry {
    at: usize,  // offset of the sample in buf
    count: u64,
}

const SIGPROF: i32 = 27;
const BUF_WORDS: usize = 32 << 20;  // 256 MiB of address space, touched only as used
const THREAD_BIT: u64 = 1 << 63;

let mut buf: *mut u64 = @ptr_from_int(*mut u64, 0);
let mut buf_pos: usize = 0;
let mut dropped: u64 = 0;
let mut hz: i32 = 999;
let mut out_path: *u8 = @ptr_from_int(*u8, 0);
let mut timer: usize = 0;
let mut timer_ok: bool = false;
let mut written: bool = false;

fn words() []mut u64 {
    return @slice_from_ptr(buf, BUF_WORDS);
}

// Each sample: [depth (top bit: not the main thread), weight, frames...].
// The kernel may check CPU timers only at scheduler ticks; expirations it
// merged are reported as the overrun, so a sample stands for 1 + overrun
// periods.
fn on_prof(sig: i32, si: *mut void, uc: *mut void) void {
    let ss = rt.phalange_shadow_stack();
    if tf.is_null(@ptr_cast(*void, ss)) {
        return;
    }
    let depth = @atomic_load(&ss.depth);
    let mut n = depth;
    if n > tf.SS_CAP {
        n = tf.SS_CAP;
    }
    let at = @atomic_add(&mut buf_pos, @as(usize, n) + 2);
    if (at + @as(usize, n) + 2) > BUF_WORDS {
        _ = @atomic_add(&mut dropped, 1);
        return;
    }
    let w = words();
    w[at] = @as(u64, depth);
    if gettid() != getpid() {
        w[at] = w[at] | THREAD_BIT;
    }
    // siginfo: si_code at 8; for timers si_overrun at 20
    let base = @int_from_ptr(si);
    let si_code = @ptr_from_int(*i32, base + 8).*;
    let overrun = @ptr_from_int(*i32, base + 20).*;
    w[at + 1] = 1;
    if (si_code == -2) and (overrun > 0) {  // SI_TIMER
        w[at + 1] = 1 + @as(u64, @bit_cast(u32, overrun));
    }
    for i in 0..n {
        w[at + 2 + @as(usize, i)] = @as(u64, @int_from_ptr(tf.call_at(ss, (depth - 1) - i)));
    }
}

fn hash_words(s: []u64) u64 {
    let mut h: u64 = 1469598103934665603;
    for x in s {
        h = (h ^ x) *% 1099511628211;
    }
    return h;
}

fn sample_len(at: usize) usize {
    let d = words()[at] & ~THREAD_BIT;
    if d < @as(u64, tf.SS_CAP) {
        return 2 + @truncate(usize, d);
    }
    return 2 + @as(usize, tf.SS_CAP);
}

// Buffered output to a file descriptor.
struct Out {
    fd: i32,
    n: usize,
    b: [8192]u8,
}

fn out_flush(o: *mut Out) void {
    let mut off: usize = 0;
    while off < o.n {
        let w = rt.write(o.fd, &o.b[off], o.n - off);
        if w <= 0 {
            break;
        }
        off += @int_cast(usize, w);
    }
    o.n = 0;
}

fn out_s(o: *mut Out, s: []u8) void {
    for c in s {
        if o.n == 8192 {
            out_flush(o);
        }
        o.b[o.n] = c;
        o.n += 1;
    }
}

fn out_u(o: *mut Out, v: u64) void {
    let mut tmp: [24]u8 = [0; 24];
    let mut f = tf.fmt(tmp[..]);
    f.u(v);
    out_s(o, f.text());
}

fn put_site(o: *mut Out, s: *tf.Site) void {
    out_s(o, "\t");
    out_u(o, @as(u64, s.file_id));
    out_s(o, "\x1f");
    out_u(o, @as(u64, s.start));
    out_s(o, "\x1f");
    out_u(o, @as(u64, s.end));
    out_s(o, "\x1f");
    out_s(o, tf.cstr(s.fn_));
}

fn same_sample(a: usize, b: usize, n: usize) bool {
    let w = words();
    if (sample_len(a) != n) or (w[a] != w[b]) {
        return false;
    }
    for k in 2..n {
        if w[a + k] != w[b + k] {
            return false;
        }
    }
    return true;
}

fn write_profile() void {
    if written or (@int_from_ptr(buf) == 0) {
        return;
    }
    written = true;
    if timer_ok {
        _ = timer_delete(timer);
    }
    _ = rt.signal(SIGPROF, 1);  // SIG_IGN
    let mut end = @atomic_load(&buf_pos);
    if end > BUF_WORDS {
        end = BUF_WORDS;
    }
    let fd = rt.open(out_path, 0x241, @as(u32, 0o644));  // O_WRONLY | O_CREAT | O_TRUNC
    if fd < 0 {
        return;
    }
    let o = @ptr_cast(*mut Out, rt.malloc(@size_of(Out)));
    o.fd = fd;
    o.n = 0;
    out_s(o, "# phalange-profile-raw 1\nhz ");
    out_u(o, @as(u64, @bit_cast(u32, hz)));
    out_s(o, "\nclock cpu\n");
    if dropped != 0 {
        out_s(o, "dropped ");
        out_u(o, dropped);
        out_s(o, "\n");
    }
    let mut cap: usize = 1024;
    while cap < (end / 4) {
        cap = cap << 1;
    }
    let tp = @ptr_cast(*mut Entry, calloc(cap, @size_of(Entry)));
    if @int_from_ptr(tp) == 0 {
        out_flush(o);
        _ = rt.close(fd);
        return;
    }
    let tab = @slice_from_ptr(tp, cap);
    let w = words();
    let mut at: usize = 0;
    while at < end {
        let n = sample_len(at);
        if (at + n) > end {
            break;
        }
        let wt = w[at + 1];
        let mut h = @truncate(usize, hash_words(w[at..at + 1]) ^ hash_words(w[at + 2..at + n])) & (cap - 1);
        loop {
            if tab[h].count == 0 {
                tab[h].at = at;
                tab[h].count = wt;
                break;
            }
            if same_sample(tab[h].at, at, n) {
                tab[h].count += wt;
                break;
            }
            h = (h + 1) & (cap - 1);
        }
        at += n;
    }
    for i in 0..cap {
        if tab[i].count == 0 {
            continue;
        }
        let s = tab[i].at;
        let n = sample_len(s) - 2;
        let mut leaf: []u8 = "main";
        if n > 0 {
            let inner = @ptr_from_int(*tf.Site, @truncate(usize, w[s + 2]));
            leaf = tf.cstr(inner.callee);
            if leaf.len == 0 {
                leaf = "<indirect call>";
            }
        } else if (w[s] & THREAD_BIT) != 0 {
            leaf = "<thread entry>";
        }
        out_s(o, "S\t");
        out_u(o, tab[i].count);
        out_s(o, "\t");
        out_u(o, w[s] & ~THREAD_BIT);
        out_s(o, "\t");
        out_s(o, leaf);
        for k in 0..n {
            put_site(o, @ptr_from_int(*tf.Site, @truncate(usize, w[s + 2 + k])));
        }
        out_s(o, "\n");
    }
    out_flush(o);
    _ = rt.close(fd);
    rt.free(@ptr_cast(*mut void, tp));
    rt.free(@ptr_cast(*mut void, o));
}

fn at_exit(p: *mut void) void {
    write_profile();
}

export fn ph_profile_start() void {
    out_path = getenv("PHALANGE_PROFILE".ptr);
    if (@int_from_ptr(out_path) == 0) or (out_path.* == 0) {
        return;
    }
    let r = getenv("PHALANGE_PROFILE_HZ".ptr);
    if (@int_from_ptr(r) != 0) and (atoi(r) > 0) {
        hz = atoi(r);
    }
    // PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_NORESERVE
    let p = mmap(@ptr_from_int(*mut void, 0), BUF_WORDS * 8, 3, 0x4022, -1, 0);
    if @int_from_ptr(p) == @bit_cast(usize, @as(isize, -1)) {
        return;
    }
    buf = @ptr_cast(*mut u64, p);
    _ = __cxa_atexit(at_exit, @ptr_from_int(*mut void, 0), @ptr_from_int(*mut void, 0));
    rt.profile_flush = write_profile;
    rt.has_profile = true;
    rt.set_handler(SIGPROF, on_prof, 0x1000_0004);  // SA_RESTART | SA_SIGINFO
    // A timer on the process's CPU clock: unlike setitimer(ITIMER_PROF) it
    // is not limited to the scheduler tick.
    let sev = SigEvent.{ value: 0, signo: SIGPROF, notify: 0, pad: [0; 12] };  // SIGEV_SIGNAL
    if timer_create(2, &sev, &mut timer) != 0 {  // CLOCK_PROCESS_CPUTIME_ID
        return;
    }
    timer_ok = true;
    let ns = @as(i64, 1_000_000_000 / hz);
    let its = ITimerSpec.{ interval_sec: ns / 1_000_000_000, interval_nsec: ns % 1_000_000_000,
                           value_sec: ns / 1_000_000_000, value_nsec: ns % 1_000_000_000 };
    _ = timer_settime(timer, 0, &its, @ptr_from_int(*mut ITimerSpec, 0));
}
