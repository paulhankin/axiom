// =============================================================================
// std/os.ph - the process: its arguments, environment and exit.
//
//     for i in @as(usize, 1)..os.arg_count() {
//         let a = os.arg(i)!;          // mem.free it
//         ...
//     }
//     let home = os.getenv("HOME") catch |e| ...;   // error.NotFound if unset
//     os.exit(2);
//
// Argument 0 is the program's name as it was run.
// =============================================================================

import "mem.ph" as mem;
import "vec.ph" as vec;

error { NotFound, OutOfMemory }

import host "phalange" {
    fn ph_argc() i32;
    fn ph_arg(i: i32, buf: *mut u8, cap: usize) isize;
    fn ph_getenv(name: *u8, len: usize, buf: *mut u8, cap: usize) isize;
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
