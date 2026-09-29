// =============================================================================
// std/os.ph - the process: its arguments, environment, directory, signals
// and exit.
//
//     for i in @as(usize, 1)..os.arg_count() {
//         let a = os.arg(i)!;          // mem.free it
//         ...
//     }
//     let home = os.getenv("HOME") catch |e| ...;   // error.NotFound if unset
//     os.setenv("LANG", "C")!;
//     os.chdir("/tmp")!;
//     os.signal_catch(os.SIGINT)!;                  // then poll or wait:
//     let got = os.signal_wait(-1)!;                // bits: 1 << os.SIGINT
//     os.exit(2);
//
// Argument 0 is the program's name as it was run. Child processes are in
// std/process.
//
// On the node host (WebAssembly) each thread has its own copy of the
// environment, only the main thread can change directory, and signals are
// not supported (Unsupported): node delivers them only to its event loop.
// =============================================================================

import "mem.ph" as mem;
import "vec.ph" as vec;

error { NotFound, OutOfMemory, InvalidArgument, PermissionDenied, Unsupported, Failed }

import host "phalange" {
    fn ph_argc() i32;
    fn ph_arg(i: i32, buf: *mut u8, cap: usize) isize;
    fn ph_getenv(name: *u8, len: usize, buf: *mut u8, cap: usize) isize;
    fn ph_setenv(k: *u8, kn: usize, v: *u8, vn: usize) i32;
    fn ph_unsetenv(k: *u8, kn: usize) i32;
    fn ph_environ(buf: *mut u8, cap: usize) isize;
    fn ph_getpid() i32;
    fn ph_chdir(path: *u8, len: usize) i32;
    fn ph_getcwd(buf: *mut u8, cap: usize) isize;
    fn ph_signal(sig: i32, action: u32) i32;
    fn ph_signal_take(timeout_ns: i64) i64;
    fn ph_exit(code: i32) void;
}

/// How many arguments the program has (including its name).
pub fn arg_count() usize {
    let n = ph_argc();
    if n < 0 {
        return 0;
    }
    return @as(usize, @bit_cast(u32, n));
}

/// Argument i (free it with mem.free); error.NotFound past the last.
pub fn arg(i: usize) ![]mut u8 {
    if i >= arg_count() {
        return error.NotFound;
    }
    let idx = @truncate(i32, @bit_cast(isize, i));
    let mut cap: usize = 64;
    loop {
        let buf = mem.alloc.[u8](cap)!;
        let n = ph_arg(idx, buf.ptr, buf.len);
        if n < 0 {
            mem.free(buf);
            return error.NotFound;
        }
        let need = @bit_cast(usize, n);
        if need <= cap {
            return mem.realloc(buf, need) catch |e| {
                mem.free(buf);
                return e;
            };
        }
        mem.free(buf);
        cap = need;
    }
}

/// All the arguments.
pub struct Args {
    items: vec.Vec.[[]mut u8],

    pub fn deinit(self: *mut Args) void {
        for a in self.items.items() {
            mem.free(a);
        }
        self.items.deinit();
    }
}

pub fn args() !Args {
    let mut out = Args.{ items: vec.Vec.[[]mut u8].init() };
    for i in @as(usize, 0)..arg_count() {
        let a = arg(i) catch |e| {
            out.deinit();
            return e;
        };
        out.items.push(a) catch |e| {
            mem.free(a);
            out.deinit();
            return e;
        };
    }
    return out;
}

/// The value of an environment variable (free it with mem.free);
/// error.NotFound if it is not set.
pub fn getenv(name: []u8) ![]mut u8 {
    let mut cap: usize = 256;
    loop {
        let buf = mem.alloc.[u8](cap)!;
        let n = ph_getenv(name.ptr, name.len, buf.ptr, buf.len);
        if n < 0 {
            mem.free(buf);
            return error.NotFound;
        }
        let need = @bit_cast(usize, n);
        if need <= cap {
            return mem.realloc(buf, need) catch |e| {
                mem.free(buf);
                return e;
            };
        }
        mem.free(buf);
        cap = need;
    }
}

/// Whether an environment variable is set.
pub fn has_env(name: []u8) bool {
    let mut b: [1]u8 = [0];
    return ph_getenv(name.ptr, name.len, &mut b[0], 0) >= 0;
}

fn valid_name(name: []u8) bool {
    if name.len == 0 {
        return false;
    }
    for c in name {
        if (c == '=') or (c == 0) {
            return false;
        }
    }
    return true;
}

/// Sets an environment variable (for this process and the children it
/// starts). InvalidArgument for an empty name, or one with "=", or a NUL
/// in either.
pub fn setenv(name: []u8, value: []u8) !void {
    if not valid_name(name) {
        return error.InvalidArgument;
    }
    for c in value {
        if c == 0 {
            return error.InvalidArgument;
        }
    }
    if ph_setenv(name.ptr, name.len, value.ptr, value.len) != 0 {
        return error.Failed;
    }
}

/// Removes an environment variable (nothing if it is not set).
pub fn unsetenv(name: []u8) !void {
    if not valid_name(name) {
        return error.InvalidArgument;
    }
    if ph_unsetenv(name.ptr, name.len) != 0 {
        return error.Failed;
    }
}

/// The whole environment.
pub struct Env {
    data: []mut u8,  // "NAME=value\0" entries
    entries: vec.Vec.[[]u8],

    pub fn deinit(self: *mut Env) void {
        mem.free(self.data);
        self.entries.deinit();
    }

    pub fn len(self: *Env) usize {
        return self.entries.len;
    }

    /// Entry i as "NAME=value".
    pub fn entry(self: *Env, i: usize) []u8 {
        return self.entries.get(i);
    }

    pub fn name(self: *Env, i: usize) []u8 {
        let e = self.entries.get(i);
        let mut k: usize = 0;
        while (k < e.len) and (e[k] != '=') {
            k += 1;
        }
        return e[..k];
    }

    pub fn value(self: *Env, i: usize) []u8 {
        let e = self.entries.get(i);
        let n = self.name(i).len;
        if n >= e.len {
            return e[e.len..];
        }
        return e[n + 1..];
    }
}

pub fn environ() !Env {
    let mut cap: usize = 4096;
    loop {
        let buf = mem.alloc.[u8](cap)!;
        let n = ph_environ(buf.ptr, buf.len);
        if n < 0 {
            mem.free(buf);
            return error.Failed;
        }
        let need = @bit_cast(usize, n);
        if need > cap {
            mem.free(buf);
            cap = need + 256;
            continue;
        }
        let mut env = Env.{ data: buf, entries: vec.Vec.[[]u8].init() };
        let mut i: usize = 0;
        while i < need {
            let mut j = i;
            while (j < need) and (buf[j] != 0) {
                j += 1;
            }
            if j > i {
                env.entries.push(buf[i..j]) catch |e| {
                    env.deinit();
                    return e;
                };
            }
            i = j + 1;
        }
        return env;
    }
}

/// The process's id.
pub fn pid() i32 {
    return ph_getpid();
}

/// The working directory (free it with mem.free).
pub fn cwd() ![]mut u8 {
    let mut cap: usize = 256;
    loop {
        let buf = mem.alloc.[u8](cap)!;
        let n = ph_getcwd(buf.ptr, buf.len);
        if n < 0 {
            mem.free(buf);
            return error.Failed;
        }
        let need = @bit_cast(usize, n);
        if need <= cap {
            return mem.realloc(buf, need) catch |e| {
                mem.free(buf);
                return e;
            };
        }
        mem.free(buf);
        cap = need;
    }
}

/// Changes the working directory.
pub fn chdir(path: []u8) !void {
    let r = ph_chdir(path.ptr, path.len);
    if r == 0 {
        return;
    }
    if r == -2 {
        return error.NotFound;
    }
    if r == -13 {
        return error.PermissionDenied;
    }
    return error.Failed;
}

/// Signal numbers (Linux).
pub const SIGHUP: i32 = 1;
pub const SIGINT: i32 = 2;
pub const SIGQUIT: i32 = 3;
pub const SIGKILL: i32 = 9;
pub const SIGUSR1: i32 = 10;
pub const SIGUSR2: i32 = 12;
pub const SIGPIPE: i32 = 13;
pub const SIGALRM: i32 = 14;
pub const SIGTERM: i32 = 15;
pub const SIGCHLD: i32 = 17;
pub const SIGCONT: i32 = 18;
pub const SIGSTOP: i32 = 19;
pub const SIGTSTP: i32 = 20;
pub const SIGWINCH: i32 = 28;

fn set_signal(sig: i32, action: u32) !void {
    let r = ph_signal(sig, action);
    if r == -2 {
        return error.Unsupported;
    }
    if r != 0 {
        return error.InvalidArgument;
    }
}

/// Catches sig instead of taking its default action: it is then recorded,
/// for signal_poll and signal_wait. SIGKILL, SIGSTOP and the signals the
/// runtime uses for crashes (SIGSEGV, SIGBUS, SIGFPE) are InvalidArgument.
pub fn signal_catch(sig: i32) !void {
    set_signal(sig, 2)!;
}

/// Ignores sig.
pub fn signal_ignore(sig: i32) !void {
    set_signal(sig, 1)!;
}

/// Restores sig's default action.
pub fn signal_default(sig: i32) !void {
    set_signal(sig, 0)!;
}

/// The caught signals that have arrived since the last poll or wait, as
/// bits (1 << sig), without waiting.
pub fn signal_poll() !u64 {
    return signal_wait(0)!;
}

/// Waits up to timeout_ns nanoseconds (-1: for ever) for a caught signal;
/// returns those that arrived, as bits (0 if it timed out).
pub fn signal_wait(timeout_ns: i64) !u64 {
    let r = ph_signal_take(timeout_ns);
    if r < 0 {
        return error.Unsupported;
    }
    return @bit_cast(u64, r);
}

/// Ends the process with an exit status, without returning (and without
/// running defers or leak checks).
pub fn exit(code: i32) void {
    ph_exit(code);
    @trap();
}

fn failed_with(r: ![]mut u8, want: error) bool {
    let v = r catch |e| return e == want;
    mem.free(v);
    return false;
}

test "arguments and environment" {
    @expect(arg_count() >= 1 or arg_count() == 0);
    @expect(failed_with(arg(1000000), error.NotFound));
    @expect(failed_with(getenv("PHALANGE_SURELY_UNSET_VARIABLE"), error.NotFound));
    @expect(not has_env("PHALANGE_SURELY_UNSET_VARIABLE"));
    if has_env("PATH") {
        let p = getenv("PATH")!;
        defer mem.free(p);
        @expect(p.len > 0);
    }
}

test "changing the environment and directory" {
    setenv("PHALANGE_TEST_VAR", "one two")!;
    let v = getenv("PHALANGE_TEST_VAR")!;
    defer mem.free(v);
    @expect_eq(v, "one two");
    setenv("PHALANGE_TEST_VAR", "")!;
    let e = getenv("PHALANGE_TEST_VAR")!;
    defer mem.free(e);
    @expect(e.len == 0);
    let mut env = environ()!;
    let mut found = false;
    for i in @as(usize, 0)..env.len() {
        if @mem_eq(env.name(i), "PHALANGE_TEST_VAR") {
            found = env.value(i).len == 0;
        }
    }
    env.deinit();
    @expect(found);
    unsetenv("PHALANGE_TEST_VAR")!;
    @expect(not has_env("PHALANGE_TEST_VAR"));
    @expect(bad_arg(setenv("A=B", "x")) and bad_arg(setenv("", "x")) and bad_arg(unsetenv("")));
    @expect(pid() > 0);
    let here = cwd()!;
    defer mem.free(here);
    chdir("/") catch |err| {
        // (the node host: only the main thread may change directory)
        @expect(err == error.Failed);
        return;
    };
    let root = cwd()!;
    defer mem.free(root);
    @expect_eq(root, "/");
    chdir(here)!;
    @expect(not_found(chdir("/surely/not/a/directory")));
}

fn not_found(r: !void) bool {
    r catch |e| return e == error.NotFound;
    return false;
}

fn bad_arg(r: !void) bool {
    r catch |e| return e == error.InvalidArgument;
    return false;
}

test "signals" {
    signal_catch(SIGUSR2) catch |e| {
        @expect(e == error.Unsupported);
        return;
    };
    @expect(signal_poll()! == 0);
    @expect(signal_wait(1000000)! == 0);
    signal_default(SIGUSR2)!;
    @expect(bad_arg(signal_catch(SIGKILL)) and bad_arg(signal_catch(0)) and bad_arg(signal_catch(99)));
}
