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

error { NotFound, AccessDenied, Exists, NotADirectory, IsDirectory, NotEmpty, IoError, OutOfMemory, BadPattern,
        Unsupported }

import host "phalange" {
    fn ph_stat(path: *u8, len: usize, follow: u32, out: *mut i64) i32;
    fn ph_mkdir(path: *u8, len: usize, mode: u32) i32;
    fn ph_remove(path: *u8, len: usize, dir: u32) i32;
    fn ph_rename(from: *u8, flen: usize, to: *u8, tlen: usize) i32;
    fn ph_readdir(path: *u8, len: usize, buf: *mut u8, cap: usize) isize;
    fn ph_getcwd(buf: *mut u8, cap: usize) isize;
    fn ph_dir_open(dir: i32, name: *u8, len: usize) i32;
    fn ph_dir_read(dir: i32, buf: *mut u8, cap: usize) isize;
    fn ph_dir_remove(dir: i32, name: *u8, len: usize, isdir: u32) i32;
    fn ph_dir_close(dir: i32) void;
    fn ph_seek(fd: i32, off: i64, whence: u32) i64;
    fn ph_chmod(path: *u8, len: usize, mode: u32) i32;
    fn ph_symlink(target: *u8, tlen: usize, link: *u8, llen: usize) i32;
    fn ph_link(from: *u8, flen: usize, to: *u8, tlen: usize) i32;
    fn ph_readlink(path: *u8, len: usize, buf: *mut u8, cap: usize) isize;
    fn ph_utimes(path: *u8, len: usize, atime_ns: i64, mtime_ns: i64) i32;
    fn ph_ftruncate(fd: i32, size: i64) i32;
    fn ph_fsync(fd: i32) i32;
    fn ph_flock(fd: i32, op: u32) i32;
    fn ph_mmap(fd: i32, off: i64, len: usize, writable: u32) usize;
    fn ph_munmap(addr: usize, len: usize) i32;
    fn ph_msync(addr: usize, len: usize) i32;
    fn ph_watch_open() i32;
    fn ph_watch_add(h: i32, path: *u8, len: usize) i32;
    fn ph_watch_remove(h: i32, wd: i32) i32;
    fn ph_watch_read(h: i32, buf: *mut u8, cap: usize, timeout_ms: i32) isize;
    fn ph_watch_close(h: i32) void;
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
///
/// Directories are opened relative to their parents without following
/// symbolic links, so replacing one with a link while this runs cannot
/// make it remove files elsewhere (except on the JavaScript host, which
/// has no way to do that).
pub fn remove_all(p: []u8) !void {
    let i = lstat(p) catch |e| {
        if e == error.NotFound {
            return;
        }
        return e;
    };
    if i.kind == DIR {
        let h = ph_dir_open(-1, p.ptr, p.len);
        if h >= 0 {
            defer ph_dir_close(h);
            empty_dir(h)!;
        } else if (h != -20) and (h != -40) {
            return fail(@as(i64, h));
        }
    }
    remove(p) catch |e| {
        if e == error.NotFound {
            return;
        }
        return e;
    };
}

// Removes everything in the open directory h.
fn empty_dir(h: i32) !void {
    let mut d = read_dir_at(h)!;
    defer d.deinit();
    for e in d.entries.items() {
        remove_at(h, e.name, e.kind)!;
    }
}

// Removes the entry `name` of the open directory h, and what is in it.
fn remove_at(h: i32, name: []u8, kind: u8) !void {
    if (kind == DIR) or (kind == 0) {
        let c = ph_dir_open(h, name.ptr, name.len);
        if c >= 0 {
            let r = empty_dir(c);
            ph_dir_close(c);
            r!;
            let q = ph_dir_remove(h, name.ptr, name.len, 1);
            if (q < 0) and (q != -2) {
                return fail(@as(i64, q));
            }
            return;
        }
        if c == -2 {
            return;
        }
        if (c != -20) and (c != -40) {
            return fail(@as(i64, c));
        }
    }
    // a file or link (or a directory that was one a moment ago)
    let r = ph_dir_remove(h, name.ptr, name.len, 0);
    if r == -21 {
        return remove_at(h, name, DIR);
    }
    if (r < 0) and (r != -2) {
        return fail(@as(i64, r));
    }
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
    return entries_of(p, -1);
}

// The entries of the open directory h.
fn read_dir_at(h: i32) !Dir {
    return entries_of("", h);
}

fn entries_of(p: []u8, h: i32) !Dir {
    let mut cap: usize = 4096;
    loop {
        let buf = mem.alloc.[u8](cap)!;
        let mut n: isize = 0;
        if h < 0 {
            n = ph_readdir(p.ptr, p.len, buf.ptr, buf.len);
        } else {
            n = ph_dir_read(h, buf.ptr, buf.len);
        }
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
    // "root/." so that a root that is a link to a directory is followed
    let mut dot = mem.alloc.[u8](root.len + 2)!;
    defer mem.free(dot);
    @mem_copy(dot[..root.len], root);
    dot[root.len] = '/';
    dot[root.len + 1] = '.';
    let h = ph_dir_open(-1, dot.ptr, dot.len);
    if h < 0 {
        return fail(@as(i64, h));
    }
    defer ph_dir_close(h);
    return walk_at(h, root, ctx, visit);
}

// walk, for the open directory h, whose path is `at`.
fn walk_at[C](h: i32, at: []u8, ctx: C, visit: fn(C, []u8, u8) bool) !void {
    let mut d = read_dir_at(h)!;
    defer d.deinit();
    for e in d.entries.items() {
        let child = path.join2(at, e.name)!;
        defer mem.free(child);
        let mut kind = e.kind;
        if kind == 0 {
            let i = lstat(child) catch continue;
            kind = i.kind;
        }
        if visit(ctx, child, kind) and (kind == DIR) {
            let c = ph_dir_open(h, e.name.ptr, e.name.len);
            if c < 0 {
                if (c == -2) or (c == -20) or (c == -40) {
                    continue;  // gone or replaced meanwhile
                }
                return fail(@as(i64, c));
            }
            let r = walk_at(c, child, ctx, visit);
            ph_dir_close(c);
            r!;
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
// Permissions, links, times, locks, mappings and watching
// -----------------------------------------------------------------------------

/// Sets the permission bits (0o644 and so on).
pub fn chmod(p: []u8, mode: u32) !void {
    let r = ph_chmod(p.ptr, p.len, mode);
    if r < 0 {
        return fail(@as(i64, r));
    }
}

/// Makes link a symbolic link to target (which need not exist).
pub fn symlink(target: []u8, link: []u8) !void {
    let r = ph_symlink(target.ptr, target.len, link.ptr, link.len);
    if r < 0 {
        return fail(@as(i64, r));
    }
}

/// The target of a symbolic link (free it with mem.free).
pub fn read_link(p: []u8) ![]mut u8 {
    let mut cap: usize = 256;
    loop {
        let buf = mem.alloc.[u8](cap)!;
        let n = ph_readlink(p.ptr, p.len, buf.ptr, buf.len);
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

/// Makes `link` another name for the existing file (a hard link).
pub fn hard_link(existing: []u8, link: []u8) !void {
    let r = ph_link(existing.ptr, existing.len, link.ptr, link.len);
    if r < 0 {
        return fail(@as(i64, r));
    }
}

/// Sets when a file was last read and modified, in nanoseconds since 1970
/// (the node host keeps microseconds).
pub fn set_times(p: []u8, atime_ns: i64, mtime_ns: i64) !void {
    let r = ph_utimes(p.ptr, p.len, atime_ns, mtime_ns);
    if r < 0 {
        return fail(@as(i64, r));
    }
}

/// Cuts an open file to `size` bytes, or extends it with zeros.
pub fn truncate(f: io.File, size: u64) !void {
    let r = ph_ftruncate(f.fd, @bit_cast(i64, size));
    if r < 0 {
        return fail(@as(i64, r));
    }
}

/// Waits until what was written to the file is on the disk.
pub fn sync(f: io.File) !void {
    if ph_fsync(f.fd) != 0 {
        return error.IoError;
    }
}

/// Lock kinds, for lock and try_lock.
pub const SHARED: u8 = 1;
pub const EXCLUSIVE: u8 = 2;

/// Takes an advisory lock on the whole file (flock), waiting while another
/// process holds a conflicting one. Unsupported on the node host.
pub fn lock(f: io.File, kind: u8) !void {
    let r = ph_flock(f.fd, @as(u32, kind & 3));
    if r == -95 {
        return error.Unsupported;
    }
    if r < 0 {
        return error.IoError;
    }
}

/// lock, without waiting: false if another process holds a conflicting one.
pub fn try_lock(f: io.File, kind: u8) !bool {
    let r = ph_flock(f.fd, @as(u32, kind & 3) | 4);
    if r == -95 {
        return error.Unsupported;
    }
    if r == -11 {
        return false;
    }
    if r < 0 {
        return error.IoError;
    }
    return true;
}

pub fn unlock(f: io.File) void {
    _ = ph_flock(f.fd, 8);
}

/// A file's contents in memory: mapped, so reading data reads the file
/// and (if writable) writing it writes the file; where mapping is not
/// possible (the node host, WebAssembly), a copy that flush and deinit
/// write back. The file's size does not change.
pub struct Mapping {
    data: []mut u8,
    file: io.File,
    mapped: bool,
    writable: bool,

    /// Makes the changes reach the file (and the disk).
    pub fn flush(self: *mut Mapping) !void {
        if not self.writable or (self.data.len == 0) {
            return;
        }
        if self.mapped {
            if ph_msync(@int_from_ptr(self.data.ptr), self.data.len) != 0 {
                return error.IoError;
            }
            return;
        }
        _ = ph_seek(self.file.fd, 0, 0);
        self.file.writer().write_all(self.data) catch return error.IoError;
    }

    /// Flushes (ignoring failures: call flush first to see them), unmaps
    /// and closes the file.
    pub fn deinit(self: *mut Mapping) void {
        self.flush() catch {};
        if self.mapped {
            _ = ph_munmap(@int_from_ptr(self.data.ptr), self.data.len);
        } else {
            mem.free(self.data);
        }
        self.file.close();
    }
}

/// Maps the whole of the file at p (read-only unless writable).
pub fn map(p: []u8, writable: bool) !Mapping {
    let info = stat(p)!;
    if info.kind != FILE {
        return error.IsDirectory;
    }
    let mut f = io.open(p) catch |e| return fail_io(e);
    if writable {
        f.close();
        f = io.open_update(p) catch |e| return fail_io(e);
    }
    let n = @truncate(usize, info.size);
    if (n == 0) or (@as(u64, n) != info.size) {
        if @as(u64, n) != info.size {
            f.close();
            return error.OutOfMemory;
        }
        return Mapping.{ data: mem.alloc.[u8](0)!, file: f, mapped: false, writable: writable };
    }
    let mut w: u32 = 0;
    if writable {
        w = 1;
    }
    let addr = ph_mmap(f.fd, 0, n, w);
    if addr != 0 {
        return Mapping.{ data: @slice_from_ptr(@ptr_from_int(*mut u8, addr), n), file: f, mapped: true,
                         writable: writable };
    }
    // a copy
    let data = mem.alloc.[u8](n) catch |e| {
        f.close();
        return e;
    };
    f.reader().read_exact(data) catch {
        mem.free(data);
        f.close();
        return error.IoError;
    };
    return Mapping.{ data: data, file: f, mapped: false, writable: writable };
}

fn fail_io(e: error) error { if e == error.NotFound {
    return error.NotFound;
}
if e == error.AccessDenied {
    return error.AccessDenied;
}
return error.IoError; }

/// Kinds of change a Watcher reports.
pub const MODIFIED: u8 = 1;
pub const ATTRIBUTES: u8 = 2;  // permissions, times, ...
pub const CREATED: u8 = 3;
pub const REMOVED: u8 = 4;
pub const MOVED_FROM: u8 = 5;    // renamed away (in a watched directory)
pub const MOVED_TO: u8 = 6;      // renamed to here
pub const SELF_REMOVED: u8 = 7;  // the watched file or directory itself
pub const SELF_MOVED: u8 = 8;

/// A change: in watched file or directory `watch` (as add returned it),
/// to its entry `name` (empty for the watched file itself).
pub struct Event {
    watch: i32,
    kind: u8,
    name: []u8,
}

/// Watches files and directories for changes (inotify on Linux; fs.watch
/// on the node host, which reports less exactly).
pub struct Watcher {
    h: i32,
    buf: []mut u8,
    events: vec.Vec.[Event],

    pub fn init() !Watcher {
        let h = ph_watch_open();
        if h < 0 {
            return error.IoError;
        }
        let buf = mem.alloc.[u8](65536) catch |e| {
            ph_watch_close(h);
            return e;
        };
        return Watcher.{ h: h, buf: buf, events: vec.Vec.[Event].init() };
    }

    pub fn deinit(self: *mut Watcher) void {
        ph_watch_close(self.h);
        mem.free(self.buf);
        self.events.deinit();
    }

    /// Starts watching a file, or a directory's entries (not its
    /// subdirectories'); returns the watch's id.
    pub fn add(self: *mut Watcher, p: []u8) !i32 {
        let r = ph_watch_add(self.h, p.ptr, p.len);
        if r < 0 {
            return fail(@as(i64, r));
        }
        return r;
    }

    pub fn remove(self: *mut Watcher, id: i32) void {
        _ = ph_watch_remove(self.h, id);
    }

    /// Waits up to timeout_ms milliseconds (-1: for ever) for changes;
    /// returns those that came (none if it timed out), valid until the
    /// next call.
    pub fn wait(self: *mut Watcher, timeout_ms: i32) ![]Event {
        self.events.clear();
        let n = ph_watch_read(self.h, self.buf.ptr, self.buf.len, timeout_ms);
        if n < 0 {
            return error.IoError;
        }
        let b = self.buf[..@bit_cast(usize, n)];
        let mut i: usize = 0;
        while (i + 16) <= b.len {
            let wd = @bit_cast(i32, le32(b, i));
            let mask = le32(b, i + 4);
            let len = @as(usize, le32(b, i + 12));
            if (i + 16 + len) > b.len {
                break;
            }
            let mut name = b[i + 16..i + 16 + len];
            let mut k: usize = 0;
            while (k < name.len) and (name[k] != 0) {
                k += 1;
            }
            name = name[..k];
            i += 16 + len;
            let kind = event_kind(mask);
            if kind != 0 {
                self.events.push(Event.{ watch: wd, kind: kind, name: name })!;
            }
        }
        return self.events.items();
    }
}

fn le32(b: []u8, i: usize) u32 {
    return ((@as(u32, b[i]) | (@as(u32, b[i + 1]) << 8)) | (@as(u32, b[i + 2]) << 16)) | (@as(u32, b[i + 3]) << 24);
}

// inotify's mask bits to a kind (0: an event not reported)
fn event_kind(mask: u32) u8 {
    if (mask & 0x2) != 0 {
        return MODIFIED;
    }
    if (mask & 0x4) != 0 {
        return ATTRIBUTES;
    }
    if (mask & 0x100) != 0 {
        return CREATED;
    }
    if (mask & 0x200) != 0 {
        return REMOVED;
    }
    if (mask & 0x40) != 0 {
        return MOVED_FROM;
    }
    if (mask & 0x80) != 0 {
        return MOVED_TO;
    }
    if (mask & 0x400) != 0 {
        return SELF_REMOVED;
    }
    if (mask & 0x800) != 0 {
        return SELF_MOVED;
    }
    return 0;
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

test "permissions, links and times" {
    let dir = make_temp_dir("fsx")!;
    defer mem.free(dir);
    defer remove_all(dir) catch {};
    let f = path.join2(dir, "a.txt")!;
    defer mem.free(f);
    write_file(f, "hello")!;
    chmod(f, 0o600)!;
    @expect(stat(f)!.mode == 0o600);
    chmod(f, 0o644)!;
    @expect(stat(f)!.mode == 0o644);
    let l = path.join2(dir, "link")!;
    defer mem.free(l);
    symlink("a.txt", l)!;
    let t = read_link(l)!;
    defer mem.free(t);
    @expect_eq(t, "a.txt");
    @expect((lstat(l)!.kind == SYMLINK) and (stat(l)!.kind == FILE));
    let h = path.join2(dir, "hard")!;
    defer mem.free(h);
    hard_link(f, h)!;
    let hc = read_file(h)!;
    defer mem.free(hc);
    @expect_eq(hc, "hello");
    @expect(exists_err(symlink("x", l)));
    set_times(f, 1000000000000000000, 1500000000123456000)!;
    let m = stat(f)!.mtime_ns;
    @expect((m >= 1500000000123000000) and (m <= 1500000000123456000));
    let mut file = io.open_update(f)!;
    truncate(file, 2)!;
    sync(file)!;
    file.close();
    let cut = read_file(f)!;
    defer mem.free(cut);
    @expect_eq(cut, "he");
    @expect(not_found(read_link("/surely/not/there")));
}

fn exists_err(r: !void) bool {
    r catch |e| return e == error.Exists;
    return false;
}

fn not_found(r: ![]mut u8) bool {
    let x = r catch |e| return e == error.NotFound;
    mem.free(x);
    return false;
}

test "locks and mappings" {
    let dir = make_temp_dir("fsm")!;
    defer mem.free(dir);
    defer remove_all(dir) catch {};
    let f = path.join2(dir, "data")!;
    defer mem.free(f);
    write_file(f, "abcdefgh")!;
    let a = io.open(f)!;
    defer a.close();
    let b = io.open(f)!;
    defer b.close();
    let mut supported = true;
    lock(a, EXCLUSIVE) catch |e| {
        @expect(e == error.Unsupported);
        supported = false;
    };
    if supported {
        // (separate opens of a file have separate locks)
        @expect(not try_lock(b, SHARED)!);
        unlock(a);
        @expect(try_lock(b, SHARED)!);
        @expect(try_lock(a, SHARED)!);
        unlock(a);
        unlock(b);
    }
    let mut m = map(f, true)!;
    @expect_eq(m.data, "abcdefgh");
    m.data[0] = 'X';
    m.flush()!;
    let now = read_file(f)!;
    defer mem.free(now);
    @expect_eq(now, "Xbcdefgh");
    m.data[7] = 'Y';
    m.deinit();
    let after = read_file(f)!;
    defer mem.free(after);
    @expect_eq(after, "XbcdefgY");
    let mut r = map(f, false)!;
    @expect(r.data.len == 8);
    r.deinit();
    let e = path.join2(dir, "empty")!;
    defer mem.free(e);
    write_file(e, "")!;
    let mut z = map(e, false)!;
    @expect(z.data.len == 0);
    z.deinit();
}

test "watching" {
    let dir = make_temp_dir("fsw")!;
    defer mem.free(dir);
    defer remove_all(dir) catch {};
    let mut w = Watcher.init()!;
    defer w.deinit();
    let id = w.add(dir)!;
    let f = path.join2(dir, "new.txt")!;
    defer mem.free(f);
    write_file(f, "x")!;
    // collect for a while: creation (and modification) of new.txt
    let mut created = false;
    for _k in @as(usize, 0)..20 {
        let evs = w.wait(200)!;
        for ev in evs {
            if (ev.watch == id) and strs.eq(ev.name, "new.txt") and (ev.kind == CREATED) {
                created = true;
            }
        }
        if created {
            break;
        }
    }
    @expect(created);
    remove(f)!;
    let mut removed = false;
    for _k in @as(usize, 0)..20 {
        let evs = w.wait(200)!;
        for ev in evs {
            if strs.eq(ev.name, "new.txt") and (ev.kind == REMOVED) {
                removed = true;
            }
        }
        if removed {
            break;
        }
    }
    @expect(removed);
    @expect((w.wait(10)!).len == 0);
    @expect(not_found_i32(w.add("/surely/not/there")));
}

fn not_found_i32(r: !i32) bool {
    _ = r catch |e| return e == error.NotFound;
    return false;
}
