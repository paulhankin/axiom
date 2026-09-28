// =============================================================================
// runtime/linux.ph - the Linux system calls the native runtime uses, made
// directly (through the linker's ph_syscall stub, selfhost/src/lstub.ph):
// native programs need no C library. Results are the kernel's: a negative
// value is -errno.
// =============================================================================

pub extern "C" {
    fn ph_syscall(n: i64, a: usize, b: usize, c: usize, d: usize, e: usize, f: usize) isize;
    fn ph_thread_create(f: fn(*mut void) void, arg: *mut void, stack_top: usize, tls: usize, ctid: *mut u32) isize;
    fn ph_sigreturn() void;
    fn ph_start_sp() usize;
    fn ph_tls_info() *TlsInfo;
    fn ph_libc_linked() i32;
    fn phalange_target_arch() i32;  // 1 x86-64, 2 AArch64
}

pub struct TlsInfo {
    template: usize,  // address of the initialized part
    filesz: usize,
    memsz: usize,
    align: usize,
}

pub struct Timespec {
    sec: i64,
    nsec: i64,
}

pub const EINTR: isize = 4;
pub const ENOENT: isize = 2;
pub const EACCES: isize = 13;
pub const EAGAIN: isize = 11;
pub const ETIMEDOUT: isize = 110;
pub const ECONNREFUSED: isize = 111;
pub const EADDRINUSE: isize = 98;

// System call numbers: [x86-64, AArch64]
let SYS_READ: [2]i64 = [0, 63];
let SYS_WRITE: [2]i64 = [1, 64];
let SYS_OPENAT: [2]i64 = [257, 56];
let SYS_CLOSE: [2]i64 = [3, 57];
let SYS_MMAP: [2]i64 = [9, 222];
let SYS_MPROTECT: [2]i64 = [10, 226];
let SYS_MUNMAP: [2]i64 = [11, 215];
let SYS_MADVISE: [2]i64 = [28, 233];
let SYS_RT_SIGACTION: [2]i64 = [13, 134];
let SYS_SIGALTSTACK: [2]i64 = [131, 132];
let SYS_GETPID: [2]i64 = [39, 172];
let SYS_GETTID: [2]i64 = [186, 178];
let SYS_TGKILL: [2]i64 = [234, 131];
let SYS_EXIT_GROUP: [2]i64 = [231, 94];
let SYS_SCHED_YIELD: [2]i64 = [24, 124];
let SYS_NANOSLEEP: [2]i64 = [35, 101];
let SYS_FUTEX: [2]i64 = [202, 98];
let SYS_SOCKET: [2]i64 = [41, 198];
let SYS_CONNECT: [2]i64 = [42, 203];
let SYS_ACCEPT4: [2]i64 = [288, 242];
let SYS_SENDTO: [2]i64 = [44, 206];
let SYS_RECVFROM: [2]i64 = [45, 207];
let SYS_BIND: [2]i64 = [49, 200];
let SYS_LISTEN: [2]i64 = [50, 201];
let SYS_GETSOCKNAME: [2]i64 = [51, 204];
let SYS_SETSOCKOPT: [2]i64 = [54, 208];
let SYS_CLONE: [2]i64 = [56, 220];
let SYS_WAIT4: [2]i64 = [61, 260];
let SYS_PIPE2: [2]i64 = [293, 59];
let SYS_DUP3: [2]i64 = [292, 24];
let SYS_SETITIMER: [2]i64 = [38, 103];
let SYS_PPOLL: [2]i64 = [271, 73];
let SYS_CLOCK_GETTIME: [2]i64 = [228, 113];
let SYS_SCHED_GETAFFINITY: [2]i64 = [204, 123];
let SYS_TIMER_CREATE: [2]i64 = [222, 107];
let SYS_TIMER_SETTIME: [2]i64 = [223, 110];
let SYS_TIMER_DELETE: [2]i64 = [226, 111];

fn nr(t: [2]i64) i64 {
    if phalange_target_arch() == 2 {
        return t[1];
    }
    return t[0];
}

fn sys0(t: [2]i64) isize {
    return ph_syscall(nr(t), 0, 0, 0, 0, 0, 0);
}

fn sys1(t: [2]i64, a: usize) isize {
    return ph_syscall(nr(t), a, 0, 0, 0, 0, 0);
}

fn sys2(t: [2]i64, a: usize, b: usize) isize {
    return ph_syscall(nr(t), a, b, 0, 0, 0, 0);
}

fn sys3(t: [2]i64, a: usize, b: usize, c: usize) isize {
    return ph_syscall(nr(t), a, b, c, 0, 0, 0);
}

fn sys4(t: [2]i64, a: usize, b: usize, c: usize, d: usize) isize {
    return ph_syscall(nr(t), a, b, c, d, 0, 0);
}

fn sys6(t: [2]i64, a: usize, b: usize, c: usize, d: usize, e: usize, f: usize) isize {
    return ph_syscall(nr(t), a, b, c, d, e, f);
}

fn ip(p: *void) usize {
    return @int_from_ptr(p);
}

fn si(x: i32) usize {
    return @bit_cast(usize, @as(isize, x));
}

pub fn read(fd: i32, p: *mut u8, n: usize) isize {
    return sys3(SYS_READ, si(fd), @int_from_ptr(p), n);
}

pub fn write(fd: i32, p: *u8, n: usize) isize {
    return sys3(SYS_WRITE, si(fd), @int_from_ptr(p), n);
}

// Writes all of s (retrying after interruptions and short writes).
pub fn write_all(fd: i32, s: []u8) void {
    let mut off: usize = 0;
    while off < s.len {
        let r = write(fd, &s[off], s.len - off);
        if r == -EINTR {
            continue;
        }
        if r <= 0 {
            return;
        }
        off += @bit_cast(usize, r);
    }
}

// path: NUL-terminated
pub fn open(path: *u8, flags: i32, mode: i32) isize {
    return sys4(SYS_OPENAT, si(-100), @int_from_ptr(path), si(flags), si(mode));  // AT_FDCWD
}

pub fn close(fd: i32) isize {
    return sys1(SYS_CLOSE, si(fd));
}

// Anonymous read-write memory, or 0.
pub fn mmap_anon(len: usize) usize {
    // PROT_READ | PROT_WRITE; MAP_PRIVATE | MAP_ANONYMOUS | MAP_NORESERVE
    let r = sys6(SYS_MMAP, 0, len, 3, 0x4022, si(-1), 0);
    if (r < 0) and (r > -4096) {
        return 0;
    }
    return @bit_cast(usize, r);
}

// No access (a guard page).
pub fn protect_none(addr: usize, len: usize) isize {
    return sys3(SYS_MPROTECT, addr, len, 0);
}

pub fn munmap(addr: usize, len: usize) isize {
    return sys2(SYS_MUNMAP, addr, len);
}

pub fn madvise(addr: usize, len: usize, advice: i32) isize {
    return sys3(SYS_MADVISE, addr, len, si(advice));
}

// The kernel's struct sigaction (not the C library's): handler, flags,
// restorer, mask. On AArch64 the kernel supplies the return trampoline.
pub struct KSigAction {
    handler: fn(i32, *mut void, *mut void) void,
    flags: u64,
    restorer: fn() void,
    mask: u64,
}

struct KSigDefault {
    handler: usize,  // 0: SIG_DFL
    flags: u64,
    restorer: usize,
    mask: u64,
}

pub fn sigaction(sig: i32, handler: fn(i32, *mut void, *mut void) void, flags: u64) isize {
    let mut sa = KSigAction.{ handler: handler, flags: flags, restorer: ph_sigreturn, mask: 0 };
    if phalange_target_arch() != 2 {
        sa.flags |= 0x0400_0000;  // SA_RESTORER
    }
    return sys4(SYS_RT_SIGACTION, si(sig), @int_from_ptr(&sa), 0, 8);
}

// Back to the default action.
pub fn sigdefault(sig: i32) isize {
    let sa = KSigDefault.{ handler: 0, flags: 0, restorer: 0, mask: 0 };
    return sys4(SYS_RT_SIGACTION, si(sig), @int_from_ptr(&sa), 0, 8);
}

// Ignore the signal.
pub fn sigignore(sig: i32) isize {
    let sa = KSigDefault.{ handler: 1, flags: 0, restorer: 0, mask: 0 };
    return sys4(SYS_RT_SIGACTION, si(sig), @int_from_ptr(&sa), 0, 8);
}

pub struct StackT {
    sp: usize,
    flags: i32,
    size: usize,
}

pub fn sigaltstack(sp: usize, size: usize) isize {
    let st = StackT.{ sp: sp, flags: 0, size: size };
    return sys2(SYS_SIGALTSTACK, @int_from_ptr(&st), 0);
}

pub fn getpid() i32 {
    return @truncate(i32, sys0(SYS_GETPID));
}

pub fn gettid() i32 {
    return @truncate(i32, sys0(SYS_GETTID));
}

// Sends sig to the calling thread.
pub fn raise(sig: i32) isize {
    return sys3(SYS_TGKILL, si(getpid()), si(gettid()), si(sig));
}

pub fn exit(code: i32) void {
    loop {
        _ = sys1(SYS_EXIT_GROUP, si(code));
    }
}

pub fn sched_yield() isize {
    return sys0(SYS_SCHED_YIELD);
}

pub fn nanosleep(ns: u64) void {
    let mut ts = Timespec.{ sec: @bit_cast(i64, ns / 1_000_000_000), nsec: @bit_cast(i64, ns % 1_000_000_000) };
    loop {
        if sys2(SYS_NANOSLEEP, @int_from_ptr(&ts), @int_from_ptr(&ts)) != -EINTR {
            return;
        }
    }
}

pub fn futex(p: *u32, op: i32, val: u32, timeout: usize) isize {
    return sys6(SYS_FUTEX, @int_from_ptr(p), si(op), @as(usize, val), timeout, 0, 0);
}

pub fn socket(domain: i32, ty: i32, protocol: i32) isize {
    return sys3(SYS_SOCKET, si(domain), si(ty), si(protocol));
}

pub fn connect(fd: i32, addr: *void, len: u32) isize {
    return sys3(SYS_CONNECT, si(fd), ip(addr), @as(usize, len));
}

pub fn bind(fd: i32, addr: *void, len: u32) isize {
    return sys3(SYS_BIND, si(fd), ip(addr), @as(usize, len));
}

pub fn listen(fd: i32, backlog: i32) isize {
    return sys2(SYS_LISTEN, si(fd), si(backlog));
}

pub fn accept4(fd: i32, flags: i32) isize {
    return sys4(SYS_ACCEPT4, si(fd), 0, 0, si(flags));
}

pub fn setsockopt(fd: i32, level: i32, name: i32, value: *void, len: u32) isize {
    return ph_syscall(nr(SYS_SETSOCKOPT), si(fd), si(level), si(name), ip(value), @as(usize, len), 0);
}

pub fn getsockname(fd: i32, addr: *mut void, len: *mut u32) isize {
    return sys3(SYS_GETSOCKNAME, si(fd), @int_from_ptr(addr), @int_from_ptr(len));
}

pub fn sendto(fd: i32, p: *u8, n: usize, flags: i32, addr: usize, alen: u32) isize {
    return ph_syscall(nr(SYS_SENDTO), si(fd), @int_from_ptr(p), n, si(flags), addr, @as(usize, alen));
}

pub fn recvfrom(fd: i32, p: *mut u8, n: usize, flags: i32) isize {
    return ph_syscall(nr(SYS_RECVFROM), si(fd), @int_from_ptr(p), n, si(flags), 0, 0);
}

// fork(): clone with SIGCHLD and nothing shared. 0 in the child.
pub fn fork() isize {
    return sys6(SYS_CLONE, 17, 0, 0, 0, 0, 0);
}

pub fn wait4(pid: i32, status: *mut i32, options: i32) isize {
    return sys4(SYS_WAIT4, si(pid), @int_from_ptr(status), si(options), 0);
}

pub fn pipe2(fds: *mut i32, flags: i32) isize {
    return sys2(SYS_PIPE2, @int_from_ptr(fds), si(flags));
}

pub fn dup2(a: i32, b: i32) isize {
    return sys3(SYS_DUP3, si(a), si(b), 0);
}

pub fn setitimer(which: i32, value: *void) isize {
    return sys3(SYS_SETITIMER, si(which), ip(value), 0);
}

pub fn ppoll(fds: *mut void, n: u64, timeout_ms: i32) isize {
    let mut ts = Timespec.{ sec: @as(i64, timeout_ms) / 1000, nsec: (@as(i64, timeout_ms) % 1000) * 1000000 };
    let mut tsp: usize = 0;
    if timeout_ms >= 0 {
        tsp = @int_from_ptr(&ts);
    }
    return ph_syscall(nr(SYS_PPOLL), @int_from_ptr(fds), @truncate(usize, n), tsp, 0, 8, 0);
}

pub fn clock_gettime(clock: i32, ts: *mut Timespec) isize {
    return sys2(SYS_CLOCK_GETTIME, si(clock), @int_from_ptr(ts));
}

// The number of processors the program may run on.
pub fn cpu_count() u32 {
    let mut set: [128]u64 = [0; 128];
    let r = sys3(SYS_SCHED_GETAFFINITY, 0, 1024, @int_from_ptr(&mut set[0]));
    if r <= 0 {
        return 1;
    }
    let mut n: u64 = 0;
    for i in 0..@bit_cast(usize, r) / 8 {
        n += @popcount(set[i]);
    }
    if n == 0 {
        return 1;
    }
    return @truncate(u32, n);
}

pub fn timer_create(clock: i32, sev: *void, timer: *mut i32) isize {
    return sys3(SYS_TIMER_CREATE, si(clock), ip(sev), @int_from_ptr(timer));
}

pub fn timer_settime(timer: i32, value: *void) isize {
    return sys4(SYS_TIMER_SETTIME, si(timer), 0, ip(value), 0);
}

pub fn timer_delete(timer: i32) isize {
    return sys1(SYS_TIMER_DELETE, si(timer));
}

// -----------------------------------------------------------------------------
// The process's environment, from the stack at entry: argc, argv..., 0,
// envp..., 0.
// -----------------------------------------------------------------------------
fn cstr_len(p: usize) usize {
    let mut n: usize = 0;
    while @ptr_from_int(*u8, p + n).* != 0 {
        n += 1;
    }
    return n;
}

// main's arguments (runtime/native.ph: ph_rt_args), for when the C
// library's _start ran instead of ours (a program linked by the C toolchain)
pub let mut main_argc: usize = 0;
pub let mut main_argv: usize = 0;

// The value of environment variable `name`, or nosym (a 0 pointer) if unset.
pub fn getenv(name: []u8) []u8 {
    let none = @slice_from_ptr(@ptr_from_int(*u8, 0), 0);
    let sp = ph_start_sp();
    let mut e: usize = 0;
    if sp != 0 {
        let argc = @ptr_from_int(*usize, sp).*;
        e = sp + ((argc + 2) * @size_of(usize));
    } else if main_argv != 0 {
        e = main_argv + ((main_argc + 1) * @size_of(usize));
    } else {
        return none;
    }
    loop {
        let p = @ptr_from_int(*usize, e).*;
        if p == 0 {
            return none;
        }
        let s = @slice_from_ptr(@ptr_from_int(*u8, p), cstr_len(p));
        if (s.len > name.len) and (s[name.len] == '=') and @mem_eq(s[0..name.len], name) {
            return s[name.len + 1..];
        }
        e += @size_of(usize);
    }
}
