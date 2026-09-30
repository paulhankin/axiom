// =============================================================================
// std/os/process.ph - running other programs.
//
//     let argv: [3][]u8 = ["git", "log", "-1"];
//     let mut out = process.run(argv[..])!;          // waits; captures output
//     defer out.deinit();
//     if out.status.success() { ... out.stdout ... }
//
//     let mut cmd = process.Command.init("sort");
//     defer cmd.deinit();
//     cmd.arg("-r")!;
//     cmd.set_env("LC_ALL", "C")!;
//     cmd.stdin(process.Stdio.PIPE);
//     cmd.stdout(process.Stdio.PIPE);
//     let mut child = cmd.spawn()!;
//     child.stdin.write_all("b\na\n")!;
//     child.stdin.close();                          // (wait closes it too)
//     let sorted = child.stdout.read_all()!;        // mem.free
//     let status = child.wait()!;
//
// A program named without a "/" is looked for in the directories of the
// child's PATH. The child inherits this process's environment (with the
// changes made by set_env and remove_env, or none of it after clear_env),
// working directory (or the one given to dir) and standard streams, unless
// told otherwise: each stream can be inherited, a pipe, or /dev/null.
//
// Arguments are passed as they are, with no shell in between, so they need
// no quoting. A program that cannot be started gives NotFound or
// PermissionDenied from spawn.
// =============================================================================

import "../collections/vec.ph" as vec;
import "../io.ph" as io;
import "../mem.ph" as mem;
import "../os.ph" as os;
import "../str.ph" as strs;
import "../thread.ph" as thread;

error { NotFound, PermissionDenied, InvalidArgument, SpawnFailed, Closed, Failed, OutOfMemory }

import host "phalange" {
    fn ph_proc_spawn(spec: *u8, len: usize, stdio: *mut i32) i32;
    fn ph_proc_wait(h: i32, block: u32, status: *mut i32) i32;
    fn ph_proc_kill(h: i32, sig: i32) i32;
    fn ph_pipe_read(h: i32, p: *mut u8, n: usize) isize;
    fn ph_pipe_write(h: i32, p: *u8, n: usize) isize;
    fn ph_pipe_close(h: i32) void;
}

/// What to connect a child's stream to.
pub enum Stdio: u8 {
    INHERIT = 0,  // this process's
    PIPE = 1,     // a pipe to this process (Child.stdin, stdout, stderr)
    NULL = 2,     // /dev/null
}

/// How a child ended.
pub struct Status {
    code: i32,    // the exit status, if it exited
    signal: i32,  // the signal that killed it, or 0

    /// Whether it exited with status 0.
    pub fn success(self: Status) bool {
        return (self.signal == 0) and (self.code == 0);
    }
}

/// One end of a pipe to a child.
pub struct Pipe {
    h: i32,  // -1: none

    pub fn is_open(self: Pipe) bool {
        return self.h >= 0;
    }

    /// Reads some bytes; 0 at the end.
    pub fn read(self: Pipe, buf: []mut u8) !usize {
        if self.h < 0 {
            return error.Closed;
        }
        let n = ph_pipe_read(self.h, buf.ptr, buf.len);
        if n < 0 {
            return error.Failed;
        }
        return @bit_cast(usize, n);
    }

    /// Writes some of the bytes; Closed if the child no longer reads.
    pub fn write(self: Pipe, bytes: []u8) !usize {
        if self.h < 0 {
            return error.Closed;
        }
        let n = ph_pipe_write(self.h, bytes.ptr, bytes.len);
        if n < 0 {
            return error.Closed;
        }
        return @bit_cast(usize, n);
    }

    pub fn write_all(self: Pipe, bytes: []u8) !void {
        let mut rest = bytes;
        while rest.len > 0 {
            let n = self.write(rest)!;
            rest = rest[n..];
        }
    }

    /// Everything until the end (free it with mem.free).
    pub fn read_all(self: Pipe) ![]mut u8 {
        let mut s = strs.String.init();
        let mut buf: [16384]u8 = [0; 16384];
        loop {
            let n = self.read(buf[..]) catch |e| {
                s.deinit();
                return e;
            };
            if n == 0 {
                break;
            }
            s.push_str(buf[..n]) catch |e| {
                s.deinit();
                return e;
            };
        }
        return s.take();
    }

    pub fn close(self: *mut Pipe) void {
        if self.h >= 0 {
            ph_pipe_close(self.h);
            self.h = -1;
        }
    }

    /// An io.Reader of the pipe (it carries the handle, so the Pipe may move).
    pub fn reader(self: Pipe) io.Reader {
        return io.Reader.{ ctx: handle_ctx(self.h), read_fn: pipe_read };
    }

    pub fn writer(self: Pipe) io.Writer {
        return io.Writer.{ ctx: handle_ctx(self.h), write_fn: pipe_write };
    }
}

fn handle_ctx(h: i32) *mut void {
    return @ptr_from_int(*mut void, @as(usize, @bit_cast(u32, h + 1)));
}

fn ctx_pipe(ctx: *mut void) Pipe {
    return Pipe.{ h: @bit_cast(i32, @truncate(u32, @int_from_ptr(ctx))) - 1 };
}

fn pipe_read(ctx: *mut void, buf: []mut u8) !usize {
    return ctx_pipe(ctx).read(buf)!;
}

fn pipe_write(ctx: *mut void, bytes: []u8) !usize {
    return ctx_pipe(ctx).write(bytes)!;
}

/// A running child.
pub struct Child {
    h: i32,
    stdin: Pipe,
    stdout: Pipe,
    stderr: Pipe,
    done: bool,
    status: Status,

    /// Closes stdin (so the child sees its end) and waits for the child to
    /// finish. Then closes the other pipes: read them first.
    pub fn wait(self: *mut Child) !Status {
        self.stdin.close();
        if not self.done {
            let mut st: i32 = 0;
            let r = ph_proc_wait(self.h, 1, &mut st);
            if r <= 0 {
                return error.Failed;
            }
            self.record(r, st);
        }
        self.stdout.close();
        self.stderr.close();
        return self.status;
    }

    /// Whether the child has finished (then its status is in `status`),
    /// without waiting.
    pub fn try_wait(self: *mut Child) !bool {
        if self.done {
            return true;
        }
        let mut st: i32 = 0;
        let r = ph_proc_wait(self.h, 0, &mut st);
        if r < 0 {
            return error.Failed;
        }
        if r == 0 {
            return false;
        }
        self.record(r, st);
        return true;
    }

    fn record(self: *mut Child, r: i32, st: i32) void {
        self.done = true;
        if r == 1 {
            self.status = Status.{ code: st, signal: 0 };
        } else {
            self.status = Status.{ code: -1, signal: st };
        }
    }

    /// Sends a signal (os.SIGTERM, os.SIGKILL, ...); nothing if it has
    /// already finished.
    pub fn kill(self: *mut Child, sig: i32) !void {
        if self.done {
            return;
        }
        if ph_proc_kill(self.h, sig) != 0 {
            return error.Failed;
        }
    }
}

/// A child's status and what it wrote.
pub struct Output {
    status: Status,
    stdout: []mut u8,
    stderr: []mut u8,

    pub fn deinit(self: *mut Output) void {
        mem.free(self.stdout);
        mem.free(self.stderr);
    }
}

/// What to run, and how.
pub struct Command {
    argv: vec.Vec.[[]u8],  // borrowed: they must outlive spawn
    sets: vec.Vec.[[]u8],  // name, value, name, value, ...
    removes: vec.Vec.[[]u8],
    clear: bool,
    cwd: []u8,
    modes: [3]Stdio,

    /// A command running `program` (argument 0).
    pub fn init(program: []u8) Command {
        let mut c = Command.{ argv: vec.Vec.[[]u8].init(), sets: vec.Vec.[[]u8].init(), removes: vec.Vec.[[]u8].init(),
                              clear: false, cwd: "", modes: [Stdio.INHERIT; 3] };
        c.argv.push(program) catch {};
        return c;
    }

    pub fn deinit(self: *mut Command) void {
        self.argv.deinit();
        self.sets.deinit();
        self.removes.deinit();
    }

    pub fn arg(self: *mut Command, a: []u8) !void {
        self.argv.push(a)!;
    }

    pub fn args(self: *mut Command, xs: [][]u8) !void {
        for a in xs {
            self.argv.push(a)!;
        }
    }

    /// Sets an environment variable for the child.
    pub fn set_env(self: *mut Command, name: []u8, value: []u8) !void {
        self.sets.push(name)!;
        self.sets.push(value)!;
    }

    /// Removes an environment variable for the child.
    pub fn remove_env(self: *mut Command, name: []u8) !void {
        self.removes.push(name)!;
    }

    /// Starts the child with no environment but what set_env gives it.
    pub fn clear_env(self: *mut Command) void {
        self.clear = true;
    }

    /// Runs the child in another directory.
    pub fn dir(self: *mut Command, path: []u8) void {
        self.cwd = path;
    }

    pub fn stdin(self: *mut Command, mode: Stdio) void {
        self.modes[0] = mode;
    }

    pub fn stdout(self: *mut Command, mode: Stdio) void {
        self.modes[1] = mode;
    }

    pub fn stderr(self: *mut Command, mode: Stdio) void {
        self.modes[2] = mode;
    }

    /// Starts the child.
    pub fn spawn(self: *Command) !Child {
        let mut spec = strs.String.init();
        defer spec.deinit();
        self.build(&mut spec)!;
        let mut sio: [3]i32 = [
            @as(i32, @int_from_enum(self.modes[0])),
            @as(i32, @int_from_enum(self.modes[1])),
            @as(i32, @int_from_enum(self.modes[2])),
        ];
        let bytes = spec.bytes();
        let h = ph_proc_spawn(bytes.ptr, bytes.len, &mut sio[0]);
        if h < 0 {
            if h == -2 {
                return error.NotFound;
            }
            if h == -13 {
                return error.PermissionDenied;
            }
            return error.SpawnFailed;
        }
        return Child.{ h: h, stdin: Pipe.{ h: sio[0] }, stdout: Pipe.{ h: sio[1] }, stderr: Pipe.{ h: sio[2] },
                       done: false, status: Status.{ code: -1, signal: 0 } };
    }

    /// Runs the child to the end, capturing its standard output and error
    /// (whatever those modes were set to; stdin stays as set).
    pub fn output(self: *mut Command) !Output {
        let saved = self.modes;
        self.modes[1] = Stdio.PIPE;
        self.modes[2] = Stdio.PIPE;
        let r = self.spawn();
        self.modes = saved;
        let mut child = r!;
        // read stderr on another thread, so that neither pipe can fill up
        // while the other is read
        let mut side = Drain.{ pipe: child.stderr, data: mem.alloc.[u8](0)!, failed: false };
        let t = thread.spawn(drain, &mut side) catch |e| {
            child.kill(os.SIGKILL) catch {};
            _ = child.wait() catch Status.{ code: -1, signal: 0 };
            mem.free(side.data);
            return e;
        };
        let out = child.stdout.read_all();
        t.join();
        let st = child.wait();
        let o = out catch |e| {
            mem.free(side.data);
            return e;
        };
        if side.failed {
            mem.free(o);
            mem.free(side.data);
            return error.Failed;
        }
        let status = st catch |e| {
            mem.free(o);
            mem.free(side.data);
            return e;
        };
        return Output.{ status: status, stdout: o, stderr: side.data };
    }

    /// Runs the child to the end, with its streams as set; its status.
    pub fn status(self: *mut Command) !Status {
        let mut child = self.spawn()!;
        return child.wait()!;
    }

    // argv\0...\0 env\0...\0 cwd
    fn build(self: *Command, spec: *mut strs.String) !void {
        let argv = self.argv.items();
        if argv[0].len == 0 {
            return error.InvalidArgument;
        }
        for a in argv {
            if contains_nul(a) {
                return error.InvalidArgument;
            }
            spec.push_str(a)!;
            spec.push(0)!;
        }
        spec.push(0)!;
        let sets = self.sets.items();
        let mut k: usize = 0;
        while k < sets.len {
            if not valid_env_name(sets[k]) or contains_nul(sets[k + 1]) {
                return error.InvalidArgument;
            }
            k += 2;
        }
        if not self.clear {
            let mut env = os.environ()!;
            defer env.deinit();
            for i in @as(usize, 0)..env.len() {
                let name = env.name(i);
                if self.overridden(name) {
                    continue;
                }
                spec.push_str(env.entry(i))!;
                spec.push(0)!;
            }
        }
        k = 0;
        while k < sets.len {
            // (a later setting of the same name wins)
            let mut later = false;
            let mut j = k + 2;
            while j < sets.len {
                if strs.eq(sets[j], sets[k]) {
                    later = true;
                }
                j += 2;
            }
            if not later and not self.removed(sets[k]) {
                spec.push_str(sets[k])!;
                spec.push('=')!;
                spec.push_str(sets[k + 1])!;
                spec.push(0)!;
            }
            k += 2;
        }
        spec.push(0)!;
        if contains_nul(self.cwd) {
            return error.InvalidArgument;
        }
        spec.push_str(self.cwd)!;
    }

    fn overridden(self: *Command, name: []u8) bool {
        for r in self.removes.items() {
            if strs.eq(r, name) {
                return true;
            }
        }
        let sets = self.sets.items();
        let mut k: usize = 0;
        while k < sets.len {
            if strs.eq(sets[k], name) {
                return true;
            }
            k += 2;
        }
        return false;
    }

    // (a removal wins over a setting, whatever their order)
    fn removed(self: *Command, name: []u8) bool {
        for r in self.removes.items() {
            if strs.eq(r, name) {
                return true;
            }
        }
        return false;
    }
}

struct Drain {
    pipe: Pipe,
    data: []mut u8,
    failed: bool,
}

fn drain(d: *mut Drain) void {
    let got = d.pipe.read_all() catch {
        d.failed = true;
        return;
    };
    mem.free(d.data);
    d.data = got;
}

fn contains_nul(s: []u8) bool {
    for c in s {
        if c == 0 {
            return true;
        }
    }
    return false;
}

fn valid_env_name(s: []u8) bool {
    if s.len == 0 {
        return false;
    }
    for c in s {
        if (c == '=') or (c == 0) {
            return false;
        }
    }
    return true;
}

/// Runs argv[0] with the arguments argv[1..], capturing its output.
pub fn run(argv: [][]u8) !Output {
    if argv.len == 0 {
        return error.InvalidArgument;
    }
    let mut c = Command.init(argv[0]);
    defer c.deinit();
    c.args(argv[1..])!;
    return c.output()!;
}

// -----------------------------------------------------------------------------
// Tests
// -----------------------------------------------------------------------------

test "running programs" {
    let argv: [3][]u8 = ["sh", "-c", "echo out; echo err >&2; exit 3"];
    let mut o = run(argv[..])!;
    defer o.deinit();
    @expect_eq(o.stdout, "out\n");
    @expect_eq(o.stderr, "err\n");
    @expect((o.status.code == 3) and not o.status.success());
    // a pipe both ways
    let mut c = Command.init("sort");
    defer c.deinit();
    c.arg("-r")!;
    c.stdin(Stdio.PIPE);
    c.stdout(Stdio.PIPE);
    let mut child = c.spawn()!;
    child.stdin.write_all("b\nc\na\n")!;
    child.stdin.close();
    let sorted = child.stdout.read_all()!;
    defer mem.free(sorted);
    @expect_eq(sorted, "c\nb\na\n");
    @expect(child.wait()!.success());
    // lots of output on both streams at once does not deadlock
    let big: [3][]u8 = ["sh", "-c", "i=0; while [ $i -lt 3000 ]; do echo line $i; echo err $i >&2; i=$((i+1)); done"];
    let mut b = run(big[..])!;
    defer b.deinit();
    @expect(strs.count(b.stdout, "\n") == 3000);
    @expect(strs.count(b.stderr, "\n") == 3000);
}

test "environment, directory and failures" {
    let mut c = Command.init("sh");
    defer c.deinit();
    let script: [2][]u8 = ["-c", "echo \"$PH_A|$PH_B|$HOME|$(pwd)\""];
    c.args(script[..])!;
    c.set_env("PH_A", "1")!;
    c.set_env("PH_B", "x y")!;
    c.set_env("PH_A", "2")!;
    c.remove_env("HOME")!;
    c.dir("/");
    let mut o = c.output()!;
    defer o.deinit();
    @expect_eq(o.stdout, "2|x y||/\n");
    // with no environment at all, PATH is the default
    let mut d = Command.init("sh");
    defer d.deinit();
    let show: [2][]u8 = ["-c", "env | grep -v '^PWD=\\|^SHLVL=\\|^_=' | wc -l"];
    d.args(show[..])!;
    d.clear_env();
    let mut e = d.output()!;
    defer e.deinit();
    @expect_eq(strs.trim(e.stdout), "0");
    let nothing: [1][]u8 = ["/surely/no/such/program"];
    @expect(fails(run(nothing[..]), error.NotFound));
    let unnamed: [1][]u8 = ["surely-no-such-program-anywhere"];
    @expect(fails(run(unnamed[..]), error.NotFound));
    let bad: [2][]u8 = ["sh", "a\x00b"];
    @expect(fails(run(bad[..]), error.InvalidArgument));
    // a killed child
    let mut k = Command.init("sleep");
    defer k.deinit();
    k.arg("10")!;
    let mut child = k.spawn()!;
    @expect(not child.try_wait()!);
    child.kill(os.SIGTERM)!;
    let st = child.wait()!;
    @expect((st.signal == os.SIGTERM) and not st.success());
}

fn fails(r: !Output, want: error) bool {
    let mut o = r catch |e| return e == want;
    o.deinit();
    return false;
}

test "signals from a child" {
    os.signal_catch(os.SIGUSR1) catch return;  // (not on every host)
    defer os.signal_default(os.SIGUSR1) catch {};
    let mut c = Command.init("sh");
    defer c.deinit();
    let script: [2][]u8 = ["-c", "kill -USR1 $PPID"];
    c.args(script[..])!;
    @expect(c.status()!.success());
    let got = os.signal_wait(5000000000)!;
    @expect((got & (@as(u64, 1) << 10)) != 0);
}
