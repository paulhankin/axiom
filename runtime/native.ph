// =============================================================================
// runtime/native.ph - the Phalange host ABI (import module "phalange") for
// native targets on Linux, and crash reporting (trap messages and stack
// traces, spec §16.4). The self-hosted compiler builds it into every native
// program; ph_rt_init runs before `main`.
// =============================================================================
import "trap_format.ph" as tf;

pub extern "C" {
    fn write(fd: i32, p: *u8, n: usize) isize;
    fn read(fd: i32, p: *mut u8, n: usize) isize;
    fn open(path: *u8, flags: i32, ...) i32;
    fn close(fd: i32) i32;
    fn __errno_location() *mut i32;
    fn getaddrinfo(node: *u8, service: *u8, hints: *void, res: *mut *mut AddrInfo) i32;
    fn freeaddrinfo(res: *mut AddrInfo) void;
    fn socket(domain: i32, ty: i32, protocol: i32) i32;
    fn connect(fd: i32, addr: *void, len: u32) i32;
    fn bind(fd: i32, addr: *void, len: u32) i32;
    fn listen(fd: i32, backlog: i32) i32;
    fn accept4(fd: i32, addr: *mut void, len: *mut u32, flags: i32) i32;
    fn setsockopt(fd: i32, level: i32, name: i32, value: *void, len: u32) i32;
    fn getsockname(fd: i32, addr: *mut void, len: *mut u32) i32;
    fn recv(fd: i32, p: *mut u8, n: usize, flags: i32) isize;
    fn send(fd: i32, p: *u8, n: usize, flags: i32) isize;
    fn malloc(n: usize) *mut void;
    fn free(p: *mut void) void;
    fn pthread_create(t: *mut u64, attr: *void, f: fn(*mut void) *mut void, arg: *mut void) i32;
    fn pthread_attr_init(a: *mut void) i32;
    fn pthread_attr_setdetachstate(a: *mut void, s: i32) i32;
    fn pthread_attr_destroy(a: *mut void) i32;
    fn pthread_self() u64;
    fn pthread_getattr_np(t: u64, a: *mut void) i32;
    fn pthread_attr_getstack(a: *void, stk: *mut usize, size: *mut usize) i32;
    fn sysconf(name: i32) isize;
    fn nanosleep(req: *Timespec, rem: *mut Timespec) i32;
    fn syscall(n: i64, ...) i64;
    fn mmap(addr: usize, len: usize, prot: i32, flags: i32, fd: i32, off: i64) usize;
    fn madvise(addr: usize, len: usize, advice: i32) i32;
    fn sched_yield() i32;
    fn sigaction(sig: i32, act: *SigAction, old: *mut SigAction) i32;
    fn sigaltstack(st: *StackT, old: *mut StackT) i32;
    fn signal(sig: i32, handler: usize) usize;
    fn raise(sig: i32) i32;
    fn phalange_shadow_stack() *tf.ShadowStack;
    fn phalange_target_arch() i32;
}

pub struct Timespec {
    sec: i64,
    nsec: i64,
}

pub struct AddrInfo {
    flags: i32,
    family: i32,
    socktype: i32,
    protocol: i32,
    addrlen: u32,
    addr: *mut void,
    canonname: *u8,
    next: *mut AddrInfo,
}

// glibc's struct sigaction (the same on x86-64 and AArch64)
pub struct SigAction {
    handler: fn(i32, *mut void, *mut void) void,
    mask: [16]u64,
    flags: i32,
    restorer: usize,
}

pub struct StackT {
    sp: *mut void,
    flags: i32,
    size: usize,
}

// Error codes shared with runtime/host.mjs and std/net.ph.
const PH_ERR: i32 = -1;
const PH_REFUSED: i32 = -2;
const PH_ADDR_IN_USE: i32 = -3;
const PH_BAD_ADDRESS: i32 = -4;

const EINTR: i32 = 4;
const ENOENT: i32 = 2;
const EACCES: i32 = 13;
const EAGAIN: i32 = 11;
const ETIMEDOUT: i32 = 110;
const ECONNREFUSED: i32 = 111;
const EADDRINUSE: i32 = 98;
const SIGBUS: i32 = 7;
const SIGFPE: i32 = 8;
const SIGSEGV: i32 = 11;

pub fn errno() i32 {
    return __errno_location().*;
}

pub fn nil_ptr[T]() *mut T {
    return @ptr_from_int(*mut T, 0);
}

export fn ph_write(fd: i32, p: *u8, n: usize) isize {
    loop {
        let r = write(fd, p, n);
        if (r < 0) and (errno() == EINTR) {
            continue;
        }
        if r < 0 {
            return -1;
        }
        return r;
    }
}

export fn ph_read(fd: i32, p: *mut u8, n: usize) isize {
    loop {
        let r = read(fd, p, n);
        if (r < 0) and (errno() == EINTR) {
            continue;
        }
        if r < 0 {
            return -1;
        }
        return r;
    }
}

// A NUL-terminated copy of s in buf (false if it does not fit).
fn copy_cstr(buf: []mut u8, s: *u8, n: usize) bool {
    if (n + 1) > buf.len {
        return false;
    }
    if n > 0 {
        @mem_copy(buf[0..n], @slice_from_ptr(s, n));
    }
    buf[n] = 0;
    return true;
}

export fn ph_open(path: *u8, len: usize, flags: u32) i32 {
    let mut buf: [4096]u8 = [0; 4096];
    if not copy_cstr(buf[..], path, len) {
        return PH_ERR;
    }
    let mut f: i32 = 0;  // O_RDONLY
    if flags == 1 {
        f = (1 | 0x40) | 0x200;  // O_WRONLY | O_CREAT | O_TRUNC
    } else if flags == 2 {
        f = (1 | 0x40) | 0x400;  // O_WRONLY | O_CREAT | O_APPEND
    }
    let fd = open(&buf[0], f | 0x80000, @as(i32, 0o644));
    if fd >= 0 {
        return fd;
    }
    let e = errno();
    if e == ENOENT {
        return -2;
    }
    if e == EACCES {
        return -13;
    }
    return PH_ERR;
}

export fn ph_close(fd: i32) void {
    _ = close(fd);
}

fn resolve(name: *u8, len: usize, port: u32, passive: bool, res: *mut *mut AddrInfo) i32 {
    let mut hostname: [256]u8 = [0; 256];
    if not copy_cstr(hostname[..], name, len) {
        return PH_BAD_ADDRESS;
    }
    let mut serv: [16]u8 = [0; 16];
    _ = tf.snprintf(&mut serv[0], 16, "%u".ptr, port);
    let mut hints = AddrInfo.{
        flags: 0, family: 0, socktype: 1, protocol: 0, addrlen: 0, addr: @ptr_from_int(*mut void, 0),
        canonname: @ptr_from_int(*u8, 0), next: nil_ptr.[AddrInfo]() };
    if passive {
        hints.flags = 1;  // AI_PASSIVE
    }
    let mut node = @ptr_from_int(*u8, 0);
    if len > 0 {
        node = &hostname[0];
    }
    if getaddrinfo(node, &serv[0], @ptr_cast(*void, &hints), res) != 0 {
        return PH_BAD_ADDRESS;
    }
    return 0;
}

export fn ph_tcp_connect(name: *u8, len: usize, port: u32) i32 {
    let mut res = nil_ptr.[AddrInfo]();
    let rc = resolve(name, len, port, false, &mut res);
    if rc != 0 {
        return rc;
    }
    let mut result = PH_ERR;
    let mut a = res;
    while @int_from_ptr(a) != 0 {
        let fd = socket(a.family, a.socktype | 0x80000, a.protocol);
        if fd >= 0 {
            if connect(fd, @ptr_cast(*void, a.addr), a.addrlen) == 0 {
                result = fd;
                break;
            }
            result = PH_ERR;
            if errno() == ECONNREFUSED {
                result = PH_REFUSED;
            }
            _ = close(fd);
        }
        a = a.next;
    }
    freeaddrinfo(res);
    return result;
}

export fn ph_tcp_listen(name: *u8, len: usize, port: u32) i32 {
    let mut res = nil_ptr.[AddrInfo]();
    let rc = resolve(name, len, port, true, &mut res);
    if rc != 0 {
        return rc;
    }
    let mut result = PH_ERR;
    let mut a = res;
    while @int_from_ptr(a) != 0 {
        let fd = socket(a.family, a.socktype | 0x80000, a.protocol);
        if fd >= 0 {
            let one: i32 = 1;
            _ = setsockopt(fd, 1, 2, @ptr_cast(*void, &one), 4);  // SOL_SOCKET, SO_REUSEADDR
            if (bind(fd, @ptr_cast(*void, a.addr), a.addrlen) == 0) and (listen(fd, 128) == 0) {
                result = fd;
                break;
            }
            result = PH_ERR;
            if errno() == EADDRINUSE {
                result = PH_ADDR_IN_USE;
            }
            _ = close(fd);
        }
        a = a.next;
    }
    freeaddrinfo(res);
    return result;
}

export fn ph_tcp_accept(listener: i32) i32 {
    loop {
        let fd = accept4(listener, @ptr_from_int(*mut void, 0), nil_ptr.[u32](), 0x80000);
        if (fd < 0) and (errno() == EINTR) {
            continue;
        }
        if fd < 0 {
            return PH_ERR;
        }
        return fd;
    }
}

export fn ph_tcp_local_port(h: i32) i32 {
    let mut ss: [128]u8 = [0; 128];
    let mut sl: u32 = 128;
    if getsockname(h, @ptr_cast(*mut void, &mut ss[0]), &mut sl) != 0 {
        return PH_ERR;
    }
    let family = @as(u32, ss[0]) | (@as(u32, ss[1]) << 8);
    if (family == 2) or (family == 10) {
        return (@as(i32, ss[2]) << 8) | @as(i32, ss[3]);  // network byte order
    }
    return PH_ERR;
}

export fn ph_sock_read(h: i32, p: *mut u8, n: usize) isize {
    loop {
        let r = recv(h, p, n, 0);
        if (r < 0) and (errno() == EINTR) {
            continue;
        }
        if r < 0 {
            return -1;
        }
        return r;
    }
}

export fn ph_sock_write(h: i32, p: *u8, n: usize) isize {
    loop {
        let r = send(h, p, n, 0x4000);  // MSG_NOSIGNAL
        if (r < 0) and (errno() == EINTR) {
            continue;
        }
        if r < 0 {
            return -1;
        }
        return r;
    }
}

export fn ph_sock_close(h: i32) void {
    _ = close(h);
}

// -----------------------------------------------------------------------------
// Threads (std/thread) and wait / notify
// -----------------------------------------------------------------------------
struct ThreadStart {
    entry: fn(*mut void) void,
    arg: *mut void,
    exited: *mut u32,
}

let mut next_thread_id: i32 = 1;

fn thread_main(p: *mut void) *mut void {
    let st = @ptr_cast(*mut ThreadStart, p);
    let entry = st.entry;
    let arg = st.arg;
    let exited = st.exited;
    free(p);
    install_crash_stack();
    entry(arg);
    @atomic_store(exited, 1);
    _ = ph_notify32(exited, 1);
    return @ptr_from_int(*mut void, 0);
}

// stack_top is ignored: native threads get their stack from the OS.
export fn ph_thread_spawn(entry: fn(*mut void) void, arg: *mut void, stack_top: usize, exited: *mut u32) i32 {
    let mem = malloc(@size_of(ThreadStart));
    if @int_from_ptr(mem) == 0 {
        return PH_ERR;
    }
    let st = @ptr_cast(*mut ThreadStart, mem);
    st.entry = entry;
    st.arg = arg;
    st.exited = exited;
    let mut t: u64 = 0;
    let mut attr: [64]u64 = [0; 64];
    _ = pthread_attr_init(@ptr_cast(*mut void, &mut attr[0]));
    _ = pthread_attr_setdetachstate(@ptr_cast(*mut void, &mut attr[0]), 1);  // PTHREAD_CREATE_DETACHED
    let rc = pthread_create(&mut t, @ptr_cast(*void, &attr[0]), thread_main, mem);
    _ = pthread_attr_destroy(@ptr_cast(*mut void, &mut attr[0]));
    if rc != 0 {
        free(mem);
        return PH_ERR;
    }
    return @atomic_add(&mut next_thread_id, 1);
}

// Native threads need no stack from the program.
export fn ph_thread_stack_size() usize {
    return 0;
}

export fn ph_cpu_count() u32 {
    let n = sysconf(84);  // _SC_NPROCESSORS_ONLN
    if n > 0 {
        return @int_cast(u32, n);
    }
    return 1;
}

export fn ph_sleep_ns(ns: u64) void {
    let mut ts = Timespec.{ sec: @bit_cast(i64, ns / 1000000000), nsec: @bit_cast(i64, ns % 1000000000) };
    while (nanosleep(&ts, &mut ts) != 0) and (errno() == EINTR) {}
}

fn sys_futex() i64 {
    if phalange_target_arch() == 2 {
        return 98;  // AArch64
    }
    return 202;  // x86-64
}

// Same results as WebAssembly memory.atomic.wait32: 0 woken, 1 not equal, 2 timed out.
export fn ph_wait32(p: *mut u32, expected: u32, timeout_ns: i64) i32 {
    let mut ts = Timespec.{ sec: 0, nsec: 0 };
    let mut tsp: usize = 0;
    if timeout_ns >= 0 {
        ts.sec = timeout_ns / 1000000000;
        ts.nsec = timeout_ns % 1000000000;
        tsp = @int_from_ptr(&ts);
    }
    loop {
        let r = syscall(sys_futex(), p, @as(i32, 128), expected, tsp, @as(usize, 0), @as(i32, 0));  // FUTEX_WAIT_PRIVATE
        if r == 0 {
            return 0;
        }
        let e = errno();
        if e == EAGAIN {
            return 1;
        }
        if e == ETIMEDOUT {
            return 2;
        }
        if e != EINTR {
            return 0;
        }
    }
}

export fn ph_notify32(p: *mut u32, count: i32) i32 {
    let mut n = count;
    if count < 0 {
        n = 0x7fff_ffff;
    }
    let r = syscall(sys_futex(), p, @as(i32, 129), n, @as(usize, 0), @as(usize, 0), @as(i32, 0));  // FUTEX_WAKE_PRIVATE
    if r < 0 {
        return 0;
    }
    return @truncate(i32, r);
}

// -----------------------------------------------------------------------------
// Linear memory (@memory_size, @memory_grow): 64 KiB pages committed from one
// region reserved at the first use. The region is as large as the system
// allows (up to 1 TiB of address space); the kernel supplies physical pages
// only when they are touched.
// -----------------------------------------------------------------------------
let mut mem_base: usize = 0;   // 64 KiB-aligned start of the region
let mut mem_limit: usize = 0;  // its size in pages
let mut mem_pages: usize = 0;  // pages handed out
let mut mem_state: u32 = 0;    // 0: not reserved, 1: reserving, 2: ready, 3: failed

fn mem_reserve() void {
    if @atomic_cas(&mut mem_state, 0, 1) == 0 {
        let mut size: usize = 1 << 40;
        while size >= (1 << 28) {
            // PROT_READ | PROT_WRITE; MAP_PRIVATE | MAP_ANONYMOUS | MAP_NORESERVE
            let p = mmap(0, size + 65536, 3, 0x4022, -1, 0);
            if p != @max_value(usize) {
                mem_base = (p + 65535) & ~@as(usize, 65535);
                mem_limit = size / 65536;
                @atomic_store(&mut mem_state, 2);
                return;
            }
            size /= 2;
        }
        @atomic_store(&mut mem_state, 3);
        return;
    }
    while @atomic_load(&mem_state) == 1 {
        _ = sched_yield();
    }
}

export fn ph_memory_size() isize {
    if @atomic_load(&mem_state) != 2 {
        mem_reserve();
    }
    return @bit_cast(isize, (mem_base / 65536) + @atomic_load(&mem_pages));
}

// The previous size in pages (as ph_memory_size counts them), or -1.
export fn ph_memory_grow(n: isize) isize {
    if @atomic_load(&mem_state) != 2 {
        mem_reserve();
    }
    if (@atomic_load(&mem_state) != 2) or (n < 0) {
        return -1;
    }
    let k = @bit_cast(usize, n);
    loop {
        let old = @atomic_load(&mem_pages);
        if k > (mem_limit - old) {
            return -1;
        }
        if @atomic_cas(&mut mem_pages, old, old + k) == old {
            return @bit_cast(isize, (mem_base / 65536) + old);
        }
    }
}

// @memory_discard: the whole 64 KiB pages in the range go back to the system
// (they stay mapped, and read as zero when next touched).
export fn ph_memory_discard(addr: usize, len: usize) void {
    let a = (addr + 65535) & ~@as(usize, 65535);
    let b = (addr + len) & ~@as(usize, 65535);
    if a < b {
        _ = madvise(a, b - a, 4);  // MADV_DONTNEED
    }
}

// -----------------------------------------------------------------------------
// Traps and crashes
// -----------------------------------------------------------------------------
// Called by code for `phalange profile` builds to write the samples so far.
pub let mut profile_flush: fn() void = no_flush;
pub let mut has_profile: bool = false;
// Replaces the default trap report (runtime/test_main.ph).
pub let mut trap_hook: fn(*tf.Site, *tf.TrapInfo, *tf.ShadowStack) void = no_hook;
pub let mut has_trap_hook: bool = false;

fn no_flush() void {}

fn no_hook(site: *tf.Site, info: *tf.TrapInfo, ss: *tf.ShadowStack) void {}

let mut report: [16384]u8 = [0; 16384];

fn flush_profile() void {
    if has_profile {
        profile_flush();
    }
}

// Every trap calls this (see codegen: phalange.trap).
export fn ph_on_trap(site: *tf.Site, info: *tf.TrapInfo, ss: *tf.ShadowStack) void {
    if has_trap_hook {
        trap_hook(site, info, ss);
        return;
    }
    let mut o = tf.fmt(report[..]);
    o.s("phalange: ");
    tf.format_trap(&mut o, site, info);
    o.s(" at ");
    o.s(tf.cstr(site.file));
    o.c(':');
    o.u(@as(u64, site.line));
    o.c(':');
    o.u(@as(u64, site.col));
    o.c('\n');
    tf.format_stack(&mut o, site, ss);
    let t = o.text();
    _ = write(2, t.ptr, t.len);
    flush_profile();
}

// Is addr within 64 KiB of the current thread's stack limit?
pub fn at_stack_limit(addr: usize) bool {
    let mut attr: [64]u64 = [0; 64];
    let mut stk: usize = 0;
    let mut size: usize = 0;
    let mut hit = false;
    if pthread_getattr_np(pthread_self(), @ptr_cast(*mut void, &mut attr[0])) == 0 {
        if pthread_attr_getstack(@ptr_cast(*void, &attr[0]), &mut stk, &mut size) == 0 {
            hit = ((addr + 65536) >= stk) and (addr < (stk + 65536));
        }
        _ = pthread_attr_destroy(@ptr_cast(*mut void, &mut attr[0]));
    }
    return hit;
}

// Crashes that are not Phalange traps (a stack overflow, or a fault in C code
// called through the FFI) are reported with the shadow stack, then the
// signal is re-raised.
fn on_crash(sig: i32, si: *mut void, uc: *mut void) void {
    let addr = @ptr_cast(*usize, @ptr_from_int(*u8, @int_from_ptr(si) + 16)).*;
    let mut code: u32 = 5;
    if sig == SIGFPE {
        code = 3;
    } else if at_stack_limit(addr) {
        code = 8;
    }
    let mut o = tf.fmt(report[..]);
    if code == 8 {
        o.s("phalange: trap T0008 (stack exhausted)\n");
    } else {
        o.s("phalange: crash: ");
        if sig == SIGSEGV {
            o.s("segmentation fault");
        } else if sig == SIGBUS {
            o.s("bus error");
        } else {
            o.s("arithmetic fault");
        }
        o.s(" (address 0x");
        o.hex(@as(u64, addr));
        o.s(")\n");
    }
    tf.format_stack(&mut o, @ptr_from_int(*tf.Site, 0), phalange_shadow_stack());
    let t = o.text();
    _ = write(2, t.ptr, t.len);
    flush_profile();
    _ = signal(sig, 0);
    _ = raise(sig);
}

// Each thread needs its own signal stack to report its stack overflow.
pub fn install_crash_stack() void {
    let p = malloc(65536);
    if @int_from_ptr(p) != 0 {
        let st = StackT.{ sp: p, flags: 0, size: 65536 };
        _ = sigaltstack(&st, nil_ptr.[StackT]());
    }
}

pub fn set_handler(sig: i32, f: fn(i32, *mut void, *mut void) void, flags: i32) void {
    let mut sa = SigAction.{ handler: f, mask: [0; 16], flags: flags, restorer: 0 };
    _ = sigaction(sig, &sa, nil_ptr.[SigAction]());
}

// Runs before main.
export fn ph_rt_init() void {
    install_crash_stack();
    let flags: i32 = 4 | 0x0800_0000;  // SA_SIGINFO | SA_ONSTACK
    set_handler(SIGSEGV, on_crash, flags);
    set_handler(SIGBUS, on_crash, flags);
    set_handler(SIGFPE, on_crash, flags);
}
