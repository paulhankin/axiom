// =============================================================================
// runtime/test_main.ph - runs the tests of a native Phalange test binary.
// Each test runs in a forked child (isolation from traps, a timeout, and its
// own captured output). Output protocol: see runtime/test_runner.mjs.
// =============================================================================
import "trap_format.ph" as tf;
import "native.ph" as rt;

extern "C" {
    fn phalange_test_count() u32;
    fn phalange_test_name(i: u32) *u8;
    fn phalange_test_expect(i: u32) i32;
    fn phalange_test_run(i: u32) u32;
    fn phalange_error_name(code: u32) *u8;
    fn pipe(fds: *mut i32) i32;
    fn fork() i32;
    fn dup2(a: i32, b: i32) i32;
    fn _exit(code: i32) void;
    fn setitimer(which: i32, nv: *ITimerVal, old: *mut ITimerVal) i32;
    fn poll(fds: *mut PollFd, n: u64, timeout: i32) i32;
    fn waitpid(pid: i32, status: *mut i32, options: i32) i32;
    fn clock_gettime(clock: i32, ts: *mut rt.Timespec) i32;
    fn realloc(p: *mut void, n: usize) *mut void;
    fn atol(s: *u8) i64;
}

struct ITimerVal {
    interval_sec: i64,
    interval_usec: i64,
    value_sec: i64,
    value_usec: i64,
}

struct PollFd {
    fd: i32,
    events: i16,
    revents: i16,
}

const SIGBUS: i32 = 7;
const SIGSEGV: i32 = 11;
const SIGALRM: i32 = 14;
const POLLIN: i16 = 1;
const POLLERR: i16 = 8;
const POLLHUP: i16 = 16;

let mut result_fd: i32 = -1;
let mut rec: [32768]u8 = [0; 32768];
let mut altstack: [65536]u8 = [0; 65536];

// Stack frames in the PHT protocol: frames separated by \x1e, fields by \x1f:
// kind (T trap site, C call, E error return, R repeat, O omitted), file id,
// start, end, function.
fn put_frame(o: *mut tf.Fmt, kind: u8, s: *tf.Site) void {
    o.c(kind);
    o.c(0x1f);
    o.u(@as(u64, s.file_id));
    o.c(0x1f);
    o.u(@as(u64, s.start));
    o.c(0x1f);
    o.u(@as(u64, s.end));
    o.c(0x1f);
    o.s(tf.cstr(s.fn_));
    o.c(0x1e);
}

fn put_count(o: *mut tf.Fmt, kind: u8, count: i64) void {
    o.c(kind);
    o.c(0x1f);
    o.i(count);
    o.c(0x1f);
    o.c('0');
    o.c(0x1f);
    o.c('0');
    o.c(0x1f);
    o.c(0x1e);
}

fn encode_stack(o: *mut tf.Fmt, top: *tf.Site, ss: *tf.ShadowStack) void {
    let start = o.n;
    let has_top = not tf.is_null(@ptr_cast(*void, top));
    if has_top {
        put_frame(o, 'T', top);
    }
    if tf.is_null(@ptr_cast(*void, ss)) {
        return;
    }
    let depth = @as(i64, ss.depth);
    let cap = @as(i64, tf.SS_CAP);
    let mut shown: u32 = 0;
    let mut i = depth - 1;
    while (i >= 0) and (i >= (depth - cap)) {
        let c = tf.call_at(ss, @truncate(u32, @bit_cast(u64, i)));
        if tf.is_null(@ptr_cast(*void, c)) {
            i -= 1;
            continue;
        }
        if not has_top and (o.n == start) {
            put_frame(o, 'U', c);  // innermost function, location unknown
        }
        if shown >= 40 {
            put_count(o, 'O', i + 1);
            return;
        }
        let mut j = i;
        while ((j - 1) >= 0) and ((j - 1) >= (depth - cap)) and
              (tf.call_at(ss, @truncate(u32, @bit_cast(u64, j - 1))) == c) {
            j -= 1;
        }
        put_frame(o, 'C', c);
        shown += 1;
        if j < i {
            put_count(o, 'R', i - j);
            i = j;
        }
        i -= 1;
    }
    if depth > cap {
        put_count(o, 'O', depth - cap);
    }
}

fn encode_errors(o: *mut tf.Fmt, ss: *tf.ShadowStack) void {
    if tf.is_null(@ptr_cast(*void, ss)) {
        return;
    }
    let mut first: u32 = 0;
    if ss.errors > tf.ET_CAP {
        first = ss.errors - tf.ET_CAP;
    }
    for i in first..ss.errors {
        let e = ss.error_returns[@as(usize, i & (tf.ET_CAP - 1))];
        if not tf.is_null(@ptr_cast(*void, e)) {
            put_frame(o, 'E', e);
        }
    }
}

fn emit(o: *tf.Fmt) void {
    if result_fd >= 0 {
        let t = o.text();
        _ = rt.write(result_fd, t.ptr, t.len);
    }
}

// Called by the trapping test (in the child) before it dies.
fn on_trap(site: *tf.Site, info: *tf.TrapInfo, ss: *tf.ShadowStack) void {
    let mut o = tf.fmt(rec[..]);
    o.s("TRAP\t");
    o.u(@as(u64, site.code));
    o.c('\t');
    o.u(@as(u64, site.file_id));
    o.c('\t');
    o.u(@as(u64, site.start));
    o.c('\t');
    o.u(@as(u64, site.end));
    o.c('\t');
    let mut msg: [1024]u8 = [0; 1024];
    let mut m = tf.fmt(msg[..]);
    tf.format_trap(&mut m, site, info);
    o.s(m.text());
    o.c('\t');
    encode_stack(&mut o, site, ss);
    emit(&o);
}

// Crashes other than traps (stack overflow, faults in C code): report the
// shadow stack, then die from the signal.
fn on_crash(sig: i32, si: *mut void, uc: *mut void) void {
    let addr = @ptr_cast(*usize, @ptr_from_int(*u8, @int_from_ptr(si) + 16)).*;
    let mut code: u64 = 5;  // memory access out of bounds, unless the fault is at the stack's end
    if rt.at_stack_limit(addr) {
        code = 8;
    }
    let mut o = tf.fmt(rec[..]);
    o.s("CRASH\t");
    o.u(code);
    o.c('\t');
    encode_stack(&mut o, @ptr_from_int(*tf.Site, 0), rt.phalange_shadow_stack());
    emit(&o);
    _ = rt.signal(sig, 0);
    _ = rt.raise(sig);
}

// A growable byte buffer for the drained pipes.
struct Buf {
    p: *mut u8,
    n: usize,
}

fn buf_add(b: *mut Buf, x: []u8) void {
    b.p = @ptr_cast(*mut u8, realloc(@ptr_cast(*mut void, b.p), b.n + x.len + 1));
    let d = @slice_from_ptr(b.p, b.n + x.len + 1);
    for k in 0..x.len {
        d[b.n + k] = x[k];
    }
    b.n += x.len;
    d[b.n] = 0;
}

fn buf_text(b: *Buf) []u8 {
    if @int_from_ptr(b.p) == 0 {
        return "";
    }
    return @slice_from_ptr(b.p, b.n);
}

fn put_escaped(o: *mut Buf, s: []u8) void {
    for c in s {
        if c == '\\' {
            buf_add(o, "\\\\");
        } else if c == '\t' {
            buf_add(o, "\\t");
        } else if c == '\n' {
            buf_add(o, "\\n");
        } else if c == '\r' {
            buf_add(o, "\\r");
        } else {
            let one: [1]u8 = [c];
            buf_add(o, one[..]);
        }
    }
}

fn now_ns() i64 {
    let mut ts = rt.Timespec.{ sec: 0, nsec: 0 };
    _ = clock_gettime(1, &mut ts);  // CLOCK_MONOTONIC
    return (ts.sec * 1_000_000_000) + ts.nsec;
}

fn starts(s: []u8, p: []u8) bool {
    if s.len < p.len {
        return false;
    }
    for i in 0..p.len {
        if s[i] != p[i] {
            return false;
        }
    }
    return true;
}

fn contains(s: []u8, p: []u8) bool {
    if p.len > s.len {
        return false;
    }
    for i in 0..(s.len - p.len) + 1 {
        if starts(s[i..], p) {
            return true;
        }
    }
    return false;
}

// Parses a decimal number at s[*at..], advancing *at.
fn num(s: []u8, at: *mut usize) u64 {
    let mut v: u64 = 0;
    while (at.* < s.len) and (s[at.*] >= '0') and (s[at.*] <= '9') {
        v = (v *% 10) +% @as(u64, s[at.*] - '0');
        at.* += 1;
    }
    return v;
}

// The text after the k-th tab of s.
fn after_tab(s: []u8, k: u32) []u8 {
    let mut seen: u32 = 0;
    for i in 0..s.len {
        if s[i] == '\t' {
            seen += 1;
            if seen == k {
                return s[i + 1..];
            }
        }
    }
    return "";
}

// The field between the k-th and (k+1)-th tab.
fn field(s: []u8, k: u32) []u8 {
    let rest = after_tab(s, k);
    for i in 0..rest.len {
        if rest[i] == '\t' {
            return rest[0..i];
        }
    }
    return rest;
}

fn drain(outfd: i32, resfd: i32, out: *mut Buf, res: *mut Buf) void {
    let mut pf: [2]PollFd = [PollFd.{ fd: outfd, events: POLLIN, revents: 0 }, PollFd.{ fd: resfd, events: POLLIN, revents: 0 }];
    let mut open_fds: i32 = 2;
    let mut chunk: [4096]u8 = [0; 4096];
    while open_fds > 0 {
        if (poll(&mut pf[0], 2, -1) < 0) and (rt.errno() != 4) {
            break;
        }
        for k in @as(usize, 0)..2 {
            if (pf[k].fd < 0) or ((pf[k].revents & ((POLLIN | POLLHUP) | POLLERR)) == 0) {
                continue;
            }
            let r = rt.read(pf[k].fd, &mut chunk[0], 4096);
            if r <= 0 {
                _ = rt.close(pf[k].fd);
                pf[k].fd = -1;
                open_fds -= 1;
                continue;
            }
            if k == 0 {
                buf_add(out, chunk[0..@int_cast(usize, r)]);
            } else {
                buf_add(res, chunk[0..@int_cast(usize, r)]);
            }
        }
    }
}

fn run_child(i: u32, resw: i32, outw: i32, timeout_ms: i64) void {
    _ = dup2(outw, 1);
    _ = dup2(outw, 2);
    result_fd = resw;
    rt.trap_hook = on_trap;
    rt.has_trap_hook = true;
    let it = ITimerVal.{ interval_sec: 0, interval_usec: 0, value_sec: timeout_ms / 1000, value_usec: (timeout_ms % 1000) * 1000 };
    _ = setitimer(0, &it, @ptr_from_int(*mut ITimerVal, 0));  // ITIMER_REAL
    let st = rt.StackT.{ sp: @ptr_cast(*mut void, &mut altstack[0]), flags: 0, size: 65536 };
    _ = rt.sigaltstack(&st, @ptr_from_int(*mut rt.StackT, 0));
    let flags: i32 = 4 | 0x0800_0000;  // SA_SIGINFO | SA_ONSTACK
    rt.set_handler(SIGSEGV, on_crash, flags);
    rt.set_handler(SIGBUS, on_crash, flags);
    let code = phalange_test_run(i);
    let mut o = tf.fmt(rec[..]);
    o.s("DONE\t");
    o.u(@as(u64, code));
    o.c('\t');
    if code != 0 {
        encode_errors(&mut o, rt.phalange_shadow_stack());
    }
    emit(&o);
    _exit(0);
}

export fn ph_test_main(argc: i32, argv: *mut *mut u8) i32 {
    let mut filter: []u8 = "";
    let mut timeout_ms: i64 = 10000;
    let args = @slice_from_ptr(argv, @int_cast(usize, argc));
    for k in 1..args.len {
        let a = tf.cstr(args[k]);
        if starts(a, "--filter=") {
            filter = a[9..];
        } else if starts(a, "--timeout=") {
            timeout_ms = atol(&a[10]);
        }
    }
    let n = phalange_test_count();
    for i in 0..n {
        if (filter.len > 0) and not contains(tf.cstr(phalange_test_name(i)), filter) {
            continue;
        }
        let expect = phalange_test_expect(i);
        let mut res: [2]i32 = [0, 0];
        let mut outp: [2]i32 = [0, 0];
        if (pipe(&mut res[0]) != 0) or (pipe(&mut outp[0]) != 0) {
            return 2;
        }
        let t0 = now_ns();
        let pid = fork();
        if pid == 0 {
            _ = rt.close(res[0]);
            _ = rt.close(outp[0]);
            run_child(i, res[1], outp[1], timeout_ms);
        }
        _ = rt.close(res[1]);
        _ = rt.close(outp[1]);
        let mut outb = Buf.{ p: @ptr_from_int(*mut u8, 0), n: 0 };
        let mut resb = Buf.{ p: @ptr_from_int(*mut u8, 0), n: 0 };
        drain(outp[0], res[0], &mut outb, &mut resb);
        let mut status: i32 = 0;
        _ = waitpid(pid, &mut status, 0);
        let ms = @float_from_int(f64, now_ns() - t0) / 1e6;

        let r = buf_text(&resb);
        let mut st: []u8 = "pass";
        let mut code: []u8 = "";
        let mut stack: []u8 = "";
        let mut msgbuf: [1500]u8 = [0; 1500];
        let mut msg = tf.fmt(msgbuf[..]);
        let mut codebuf: [8]u8 = [0; 8];
        let mut cb = tf.fmt(codebuf[..]);
        let mut file_id: i64 = -1;
        let mut start: u64 = 0;
        let mut end: u64 = 0;
        let signaled = ((status & 0x7f) != 0) and ((status & 0x7f) != 0x7f);
        let sig = status & 0x7f;
        if starts(r, "DONE\t") {
            let mut at: usize = 5;
            let ec = num(r, &mut at);
            if ec != 0 {
                st = "fail";
                code = "error";
                stack = after_tab(r, 2);
                msg.s("test returned error.");
                msg.s(tf.cstr(phalange_error_name(@truncate(u32, ec))));
            } else if expect != -1 {
                st = "fail";
                code = "no_trap";
                if expect == -2 {
                    msg.s("expected a trap, but the test completed");
                } else {
                    msg.s("expected trap ");
                    msg.code(@bit_cast(u32, expect));
                    msg.s(", but the test completed");
                }
            }
        } else if starts(r, "TRAP\t") {
            let mut at: usize = 5;
            let tc = num(r, &mut at);
            at += 1;
            file_id = @bit_cast(i64, num(r, &mut at));
            at += 1;
            start = num(r, &mut at);
            at += 1;
            end = num(r, &mut at);
            let text = field(r, 5);
            stack = after_tab(r, 6);
            if (expect == -2) or (@as(i64, expect) == @bit_cast(i64, tc)) {
                file_id = -1;
            } else {
                cb.code(@truncate(u32, tc));
                st = "fail";
                code = cb.text();
                if expect >= 0 {
                    msg.s("expected trap ");
                    msg.code(@bit_cast(u32, expect));
                    msg.s(", got ");
                }
                msg.s(text);
            }
        } else if signaled {
            let mut tc: u32 = 0;
            if (sig == SIGSEGV) or (sig == SIGBUS) {
                tc = 5;
            }
            if starts(r, "CRASH\t") {
                let mut at: usize = 6;
                tc = @truncate(u32, num(r, &mut at));
                stack = after_tab(r, 2);
            }
            if sig == SIGALRM {
                st = "fail";
                code = "timeout";
                msg.s("test did not finish within ");
                msg.i(timeout_ms);
                msg.s(" ms");
            } else if (expect == -2) or (expect == @bit_cast(i32, tc)) {
                // pass
            } else {
                cb.code(tc);
                st = "fail";
                code = cb.text();
                msg.s("trap ");
                msg.code(tc);
                msg.s(" (");
                msg.s(tf.trap_name(tc));
                msg.s(", signal ");
                msg.i(@as(i64, sig));
                msg.s(")");
            }
        } else {
            st = "fail";
            code = "T0000";
            msg.s("test process exited unexpectedly (status ");
            msg.i(@as(i64, status));
            msg.s(")");
        }
        let mut line = Buf.{ p: @ptr_from_int(*mut u8, 0), n: 0 };
        let mut hb: [256]u8 = [0; 256];
        let mut h = tf.fmt(hb[..]);
        h.s("PHT\t");
        h.u(@as(u64, i));
        h.c('\t');
        h.s(st);
        h.c('\t');
        h.s(code);
        h.c('\t');
        h.i(file_id);
        h.c('\t');
        h.u(start);
        h.c('\t');
        h.u(end);
        h.c('\t');
        h.u(@int_from_float(u64, ms + 0.5));
        h.c('\t');
        buf_add(&mut line, h.text());
        put_escaped(&mut line, msg.text());
        buf_add(&mut line, "\t");
        put_escaped(&mut line, buf_text(&outb));
        buf_add(&mut line, "\t");
        if not starts(st, "pass") {
            put_escaped(&mut line, stack);
        }
        buf_add(&mut line, "\n");
        let t = buf_text(&line);
        let mut off: usize = 0;
        while off < t.len {
            let w = rt.write(1, &t[off], t.len - off);
            if w <= 0 {
                break;
            }
            off += @int_cast(usize, w);
        }
        rt.free(@ptr_cast(*mut void, line.p));
        rt.free(@ptr_cast(*mut void, outb.p));
        rt.free(@ptr_cast(*mut void, resb.p));
    }
    return 0;
}
