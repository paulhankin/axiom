// =============================================================================
// std/fs.ph - the file system: whole files, directories, metadata, walking
// trees and expanding glob patterns.
//
//     let text = fs.read_file("notes.txt")!;           // mem.free it
//     fs.write_file("out/copy.txt", text)!;
//     fs.mkdir_all("out/a/b")!;
//     let info = fs.stat("out")!;                      // info.kind == fs.DIR
//     let mut d = fs.read_dir("out")!;                 // sorted entries
//     defer d.deinit();
//     let mut g = fs.glob("src/**/*.ph")!;             // matching paths
//     defer g.deinit();
//
// Paths are as in std/path. Errors say what went wrong: NotFound,
// AccessDenied, Exists, NotADirectory, IsDirectory, NotEmpty, or IoError.
// =============================================================================

import "crypto/random.ph" as random;
import "io.ph" as io;
import "mem.ph" as mem;
import "os.ph" as os;
import "path.ph" as path;
import "str.ph" as strs;
import "vec.ph" as vec;

error { NotFound, AccessDenied, Exists, NotADirectory, IsDirectory, NotEmpty, IoError, OutOfMemory, BadPattern }

import host "phalange" {
    fn ph_stat(path: *u8, len: usize, follow: u32, out: *mut i64) i32;
    fn ph_mkdir(path: *u8, len: usize, mode: u32) i32;
    fn ph_remove(path: *u8, len: usize, dir: u32) i32;
    fn ph_rename(from: *u8, flen: usize, to: *u8, tlen: usize) i32;
    fn ph_readdir(path: *u8, len: usize, buf: *mut u8, cap: usize) isize;
    fn ph_getcwd(buf: *mut u8, cap: usize) isize;
}

/// Kinds of file.
pub const FILE: u8 = 1;
pub const DIR: u8 = 2;
pub const SYMLINK: u8 = 3;
pub const OTHER: u8 = 4;  // devices, pipes, sockets

fn fail(code: i64) error { if code == -2 {
    return error.NotFound;
}
if code == -13 {
    return error.AccessDenied;
}
if code == -17 {
    return error.Exists;
}
if code == -20 {
    return error.NotADirectory;
}
if code == -21 {
    return error.IsDirectory;
}
if code == -39 {
    return error.NotEmpty;
}
return error.IoError; }

/// What stat says about a file.
pub struct Info {
    kind: u8,
    size: u64,
    mtime_ns: i64,  // last modified, in nanoseconds since 1970
    mode: u32,      // permission bits (0o755 and so on)

    pub fn is_dir(self: Info) bool {
        return self.kind == DIR;
    }

    pub fn is_file(self: Info) bool {
        return self.kind == FILE;
    }
}

fn stat_with(p: []u8, follow: u32) !Info {
    let mut out: [4]i64 = [0; 4];
    let r = ph_stat(p.ptr, p.len, follow, &mut out[0]);
    if r < 0 {
        return fail(@as(i64, r));
    }
    return Info.{
        kind: @truncate(u8, @bit_cast(u64, out[0])),
        size: @bit_cast(u64, out[1]),
        mtime_ns: out[2],
        mode: @truncate(u32, @bit_cast(u64, out[3])),
    };
}

/// Information about the file at p (following symbolic links).
pub fn stat(p: []u8) !Info {
    return stat_with(p, 1);
}

/// Information about p itself (a symbolic link is not followed).
pub fn lstat(p: []u8) !Info {
    return stat_with(p, 0);
}

pub fn exists(p: []u8) bool {
    _ = lstat(p) catch return false;
    return true;
}

pub fn is_dir(p: []u8) bool {
    let i = stat(p) catch return false;
    return i.kind == DIR;
}

pub fn is_file(p: []u8) bool {
    let i = stat(p) catch return false;
    return i.kind == FILE;
}

// -----------------------------------------------------------------------------
// Whole files
// -----------------------------------------------------------------------------

/// The contents of a file (free them with mem.free).
pub fn read_file(p: []u8) ![]mut u8 {
    let f = io.open(p) catch |e| return map_io(e);
    defer f.close();
    return f.reader().read_all() catch |e| return map_io(e);
}

fn map_io(e: error) error { if e == error.NotFound {
    return error.NotFound;
}
if e == error.AccessDenied {
    return error.AccessDenied;
}
if e == error.IsDirectory {
    return error.IsDirectory;
}
if e == error.OutOfMemory {
    return error.OutOfMemory;
}
return error.IoError; }

/// Creates (or truncates) a file holding data.
pub fn write_file(p: []u8, data: []u8) !void {
    let f = io.create(p) catch |e| return map_io(e);
    defer f.close();
    f.writer().write_all(data) catch |e| return map_io(e);
}

/// Adds data to the end of a file, creating it if needed.
pub fn append_file(p: []u8, data: []u8) !void {
    let f = io.append(p) catch |e| return map_io(e);
    defer f.close();
    f.writer().write_all(data) catch |e| return map_io(e);
}

/// Copies a file's contents to another (created or truncated).
pub fn copy_file(from: []u8, to: []u8) !void {
    let src = io.open(from) catch |e| return map_io(e);
    defer src.close();
    let dst = io.create(to) catch |e| return map_io(e);
    defer dst.close();
    let buf = mem.alloc.[u8](1 << 16)!;
    defer mem.free(buf);
    loop {
        let n = src.reader().read(buf) catch |e| return map_io(e);
        if n == 0 {
            return;
        }
        dst.writer().write_all(buf[..n]) catch |e| return map_io(e);
    }
}

// -----------------------------------------------------------------------------
// Directories
// -----------------------------------------------------------------------------

/// Creates a directory (error.Exists if there is something at p).
pub fn mkdir(p: []u8) !void {
    mkdir_mode(p, 0o777)!;
}

/// Creates a directory with the given permissions (less the umask).
pub fn mkdir_mode(p: []u8, mode: u32) !void {
    let r = ph_mkdir(p.ptr, p.len, mode);
    if r < 0 {
        return fail(@as(i64, r));
    }
}

/// Creates a directory and any missing parents; fine if it exists.
pub fn mkdir_all(p: []u8) !void {
    if is_dir(p) {
        return;
    }
    let parent = path.dir(p);
    if (parent.len < p.len) and not ((parent.len == 1) and ((parent[0] == '.') or (parent[0] == '/'))) {
        mkdir_all(parent)!;
    }
    mkdir(p) catch |e| {
        if (e == error.Exists) and is_dir(p) {
            return;  // made meanwhile
        }
        return e;
    };
}

/// Removes a file, a symbolic link or an empty directory.
pub fn remove(p: []u8) !void {
    let i = lstat(p)!;
    let mut dir: u32 = 0;
    if i.kind == DIR {
        dir = 1;
    }
    let r = ph_remove(p.ptr, p.len, dir);
    if r < 0 {
        return fail(@as(i64, r));
    }
}

/// Removes p and, if it is a directory, everything in it (symbolic links
/// are removed, not followed). Fine if p does not exist.
pub fn remove_all(p: []u8) !void {
    let i = lstat(p) catch |e| {
        if e == error.NotFound {
            return;
        }
        return e;
    };
    if i.kind == DIR {
        let mut d = read_dir(p)!;
        defer d.deinit();
        for e in d.entries.items() {
            let child = path.join2(p, e.name)!;
            defer mem.free(child);
            remove_all(child)!;
        }
    }
    remove(p)!;
}

/// Moves (renames) a file or directory, replacing a file at `to`.
pub fn rename(from: []u8, to: []u8) !void {
    let r = ph_rename(from.ptr, from.len, to.ptr, to.len);
    if r < 0 {
        return fail(@as(i64, r));
    }
}

pub struct Entry {
    name: []u8,
    kind: u8,  // FILE, DIR, SYMLINK, OTHER, or 0 if the system did not say
}

/// A directory's entries, sorted by name (not "." and "..").
pub struct Dir {
    entries: vec.Vec.[Entry],
    buf: []mut u8,  // the names

    pub fn deinit(self: *mut Dir) void {
        self.entries.deinit();
        mem.free(self.buf);
    }
}

fn entry_less(a: Entry, b: Entry) bool {
    return strs.compare(a.name, b.name) < 0;
}

/// The entries of the directory at p.
pub fn read_dir(p: []u8) !Dir {
    let mut cap: usize = 4096;
    loop {
        let buf = mem.alloc.[u8](cap)!;
        let n = ph_readdir(p.ptr, p.len, buf.ptr, buf.len);
        if n < 0 {
            mem.free(buf);
            return fail(@as(i64, n));
        }
        let need = @bit_cast(usize, n);
        if need > cap {
            mem.free(buf);
            cap = need + 1024;  // it may have grown meanwhile
            continue;
        }
        let mut d = Dir.{ entries: vec.Vec.[Entry].init(), buf: buf };
        let mut i: usize = 0;
        while i < need {
            let kind = buf[i];
            let mut e = i + 1;
            while (e < need) and (buf[e] != 0) {
                e += 1;
            }
            d.entries.push(Entry.{ name: buf[i + 1..e], kind: kind }) catch |err| {
                d.deinit();
                return err;
            };
            i = e + 1;
        }
        d.entries.sort(entry_less);
        return d;
    }
}

/// The working directory (free it with mem.free).
pub fn cwd() ![]mut u8 {
    let mut cap: usize = 256;
    loop {
        let buf = mem.alloc.[u8](cap)!;
        let n = ph_getcwd(buf.ptr, buf.len);
        if n < 0 {
            mem.free(buf);
            return fail(@as(i64, n));
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

/// p as an absolute, cleaned path (relative paths are taken from the
/// working directory; symbolic links are not resolved).
pub fn absolute(p: []u8) ![]mut u8 {
    if path.is_abs(p) {
        return path.clean(p);
    }
    let here = cwd()!;
    defer mem.free(here);
    return path.join2(here, p);
}

/// The directory for temporary files: $TMPDIR, else /tmp (free it).
pub fn temp_dir() ![]mut u8 {
    let t = os.getenv("TMPDIR") catch |e| {
        if e == error.OutOfMemory {
            return e;
        }
        return mem.dup("/tmp");
    };
    if t.len == 0 {
        mem.free(t);
        return mem.dup("/tmp");
    }
    return t;
}

/// Creates a new directory with a unique name in temp_dir(), starting with
/// prefix; returns its path (free it; remove it with remove_all).
pub fn make_temp_dir(prefix: []u8) ![]mut u8 {
    let base = temp_dir()!;
    defer mem.free(base);
    let digits = "0123456789abcdefghijklmnopqrstuvwxyz";
    for _attempt in @as(u32, 0)..100 {
        let mut name = strs.String.init();
        defer name.deinit();
        name.push_str(prefix)!;
        let mut rnd: [10]u8 = [0; 10];
        random.fill(rnd[..]) catch return error.IoError;
        for b in rnd {
            name.push(digits[@as(usize, b) % 36])!;
        }
        let p = path.join2(base, name.bytes())!;
        mkdir_mode(p, 0o700) catch |e| {
            mem.free(p);
            if e == error.Exists {
                continue;
            }
            return e;
        };
        return p;
    }
    return error.Exists;
}

// -----------------------------------------------------------------------------
// Trees and patterns
// -----------------------------------------------------------------------------

/// Visits every entry under root (not root itself), depth first, in name
/// order: visit(ctx, path, kind) gets each path (valid only during the
/// call) and returns whether to go into it, if it is a directory.
/// Symbolic links are reported, not followed.
pub fn walk[C](root: []u8, ctx: C, visit: fn(C, []u8, u8) bool) !void {
    let mut d = read_dir(root)!;
    defer d.deinit();
    for e in d.entries.items() {
        let child = path.join2(root, e.name)!;
        defer mem.free(child);
        let mut kind = e.kind;
        if kind == 0 {
            let i = lstat(child) catch continue;
            kind = i.kind;
        }
        if visit(ctx, child, kind) and (kind == DIR) {
            walk(child, ctx, visit)!;
        }
    }
}

/// A list of paths (see glob).
pub struct Paths {
    items: vec.Vec.[[]mut u8],

    pub fn deinit(self: *mut Paths) void {
        for p in self.items.items() {
            mem.free(p);
        }
        self.items.deinit();
    }
}

struct GlobState {
    pattern: []u8,
    out: *mut Paths,
    failed: bool,
}

fn glob_visit(s: *mut GlobState, p: []u8, kind: u8) bool {
    if s.failed {
        return false;
    }
    let mut rel = p;
    if (rel.len > 2) and (rel[0] == '.') and (rel[1] == '/') and not ((s.pattern.len > 1) and (s.pattern[0] == '.')) {
        rel = rel[2..];
    }
    if path.matches(s.pattern, rel) {
        let copy = mem.dup(rel) catch {
            s.failed = true;
            return false;
        };
        s.out.items.push(copy) catch {
            mem.free(copy);
            s.failed = true;
            return false;
        };
    }
    _ = kind;
    return true;
}

fn path_less(a: []mut u8, b: []mut u8) bool {
    return strs.compare(a, b) < 0;
}

/// The paths matching a glob pattern (std/path's syntax, with "**"),
/// sorted. The walk starts at the pattern's leading elements that have no
/// glob characters.
pub fn glob(pattern: []u8) !Paths {
    let mut out = Paths.{ items: vec.Vec.[[]mut u8].init() };
    glob_into(pattern, &mut out) catch |e| {
        out.deinit();
        return e;
    };
    out.items.sort(path_less);
    return out;
}

fn glob_into(pattern: []u8, out: *mut Paths) !void {
    if not path.has_glob(pattern) {
        if exists(pattern) {
            out.items.push(mem.dup(pattern)!)!;
        }
        return;
    }
    // the fixed part: whole elements before the first glob character
    let mut fixed_end: usize = 0;
    let mut i: usize = 0;
    while i < pattern.len {
        let s = i;
        while (i < pattern.len) and (pattern[i] != '/') {
            i += 1;
        }
        if path.has_glob(pattern[s..i]) {
            break;
        }
        fixed_end = i;
        while (i < pattern.len) and (pattern[i] == '/') {
            i += 1;
        }
    }
    let mut root = pattern[..fixed_end];
    if root.len == 0 {
        if path.is_abs(pattern) {
            root = "/";
        } else {
            root = ".";
        }
    }
    if not is_dir(root) {
        return;
    }
    let mut st = GlobState.{ pattern: pattern, out: out, failed: false };
    walk(root, &mut st, glob_visit)!;
    if st.failed {
        return error.OutOfMemory;
    }
}

// -----------------------------------------------------------------------------
// Tests
// -----------------------------------------------------------------------------

fn count_visit(n: *mut u32, p: []u8, kind: u8) bool {
    n.* += 1;
    _ = p;
    _ = kind;
    return true;
}

fn failed_with(r: ![]mut u8, want: error) bool {
    let v = r catch |e| return e == want;
    mem.free(v);
    return false;
}

test "files and directories" {
    let tmp = make_temp_dir("phalange-fs-")!;
    defer mem.free(tmp);
    defer remove_all(tmp) catch {};
    @expect(is_dir(tmp));
    let a = path.join2(tmp, "a/b/c")!;
    defer mem.free(a);
    mkdir_all(a)!;
    mkdir_all(a)!;  // again: fine
    @expect(is_dir(a));
    let f = path.join2(a, "hello.txt")!;
    defer mem.free(f);
    write_file(f, "hello")!;
    append_file(f, ", world")!;
    let text = read_file(f)!;
    defer mem.free(text);
    @expect_eq(text, "hello, world");
    let info = stat(f)!;
    @expect(info.is_file());
    @expect_eq(info.size, 12);
    @expect(info.mtime_ns > 0);
    // seek and size
    let h = io.open(f)!;
    @expect_eq(h.size()!, 12);
    _ = h.seek(7, io.SEEK_SET)!;
    let mut buf: [5]u8 = [0; 5];
    h.reader().read_exact(buf[..])!;
    @expect_eq(buf[..], "world");
    h.close();
    // copy and rename
    let g = path.join2(tmp, "a/copy.txt")!;
    defer mem.free(g);
    copy_file(f, g)!;
    let moved = path.join2(tmp, "moved.txt")!;
    defer mem.free(moved);
    rename(g, moved)!;
    @expect(not exists(g));
    @expect(is_file(moved));
    // errors
    @expect(failed_with(read_file(g), error.NotFound));
    mkdir(a) catch |e| {
        @expect_eq(e, error.Exists);
    };
    let ab = path.join2(tmp, "a")!;
    defer mem.free(ab);
    remove(ab) catch |e| {
        @expect_eq(e, error.NotEmpty);
    };
    @expect(not failed_with(read_file(ab), error.NotFound));
    // listing
    let mut d = read_dir(tmp)!;
    defer d.deinit();
    @expect_eq(d.entries.len, 2);
    @expect_eq(d.entries.get(0).name, "a");
    @expect_eq(d.entries.get(0).kind, DIR);
    @expect_eq(d.entries.get(1).name, "moved.txt");
    let mut n: u32 = 0;
    walk(tmp, &mut n, count_visit)!;
    @expect_eq(n, 5);  // a, a/b, a/b/c, a/b/c/hello.txt, moved.txt
    // globs
    let pat = path.join2(tmp, "**/*.txt")!;
    defer mem.free(pat);
    let mut gl = glob(pat)!;
    defer gl.deinit();
    @expect_eq(gl.items.len, 2);
    @expect(strs.ends_with(gl.items.get(0), "a/b/c/hello.txt"));
    @expect(strs.ends_with(gl.items.get(1), "moved.txt"));
    let abs = absolute("x/../y")!;
    defer mem.free(abs);
    @expect(path.is_abs(abs));
    @expect(strs.ends_with(abs, "/y"));
}

test "temporary directories are private; names cannot hide a NUL" {
    let d = make_temp_dir("phfs")!;
    defer mem.free(d);
    let info = stat(d)!;
    @expect_eq(info.mode & 0o077, 0);
    let mut p = strs.String.init();
    defer p.deinit();
    p.push_str(d)!;
    p.push_str("/a.txt")!;
    write_file(p.bytes(), "x")!;
    // "a.txt\0.png" must not open a.txt
    p.push_str("\x00.png")!;
    @expect(failed_with(read_file(p.bytes()), error.IoError) or failed_with(read_file(p.bytes()), error.NotFound));
    remove_all(d)!;
}
