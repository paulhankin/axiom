// =============================================================================
// runtime/native.ph - the Phalange host ABI (import module "phalange") for
// native targets on Linux, and crash reporting (trap messages and stack
// traces, spec §16.4). The self-hosted compiler builds it into every native
// program; ph_rt_init runs before `main` and ph_rt_fini after it.
//
// Everything goes through Linux system calls (linux.ph): no C library.
// Threads are made with clone() on memory the runtime maps (stack, guard
// page, thread-local storage, signal stack). Host names are resolved here
// too: dotted quads, localhost, /etc/hosts, then DNS (A records) over UDP to
// the name servers of /etc/resolv.conf. If the program is linked against the
// C library (it calls C), threads come from pthread_create instead, so that
// C code on them has the C library's thread state.
// =============================================================================
import "linux.ph" as sys;
import "trap_format.ph" as tf;

pub extern "C" {
    fn phalange_shadow_stack() *tf.ShadowStack;
    // only called when the C library is linked (see selfhost/src/lstub.ph)
    fn pthread_create(t: *mut u64, attr: *void, f: fn(*mut void) *mut void, arg: *mut void) i32;
    fn pthread_attr_init(a: *mut void) i32;
    fn pthread_attr_setdetachstate(a: *mut void, s: i32) i32;
    fn pthread_attr_destroy(a: *mut void) i32;
}

// Error codes shared with runtime/host.mjs and std/net.ph.
const PH_ERR: i32 = -1;
const PH_REFUSED: i32 = -2;
const PH_ADDR_IN_USE: i32 = -3;
const PH_BAD_ADDRESS: i32 = -4;
const PH_TIMED_OUT: i32 = -5;

pub const SIGBUS: i32 = 7;
pub const SIGFPE: i32 = 8;
pub const SIGSEGV: i32 = 11;

pub fn nil_ptr[T]() *mut T {
    return @ptr_from_int(*mut T, 0);
}

fn retry(r: isize) bool {
    return r == -sys.EINTR;
}

export fn ph_write(fd: i32, p: *u8, n: usize) isize {
    loop {
        let r = sys.write(fd, p, n);
        if retry(r) {
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
        let r = sys.read(fd, p, n);
        if retry(r) {
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
    let fd = sys.open(&buf[0], f | 0x80000, 0o644);  // O_CLOEXEC
    if fd >= 0 {
        return @truncate(i32, fd);
    }
    if fd == -sys.ENOENT {
        return -2;
    }
    if fd == -sys.EACCES {
        return -13;
    }
    return PH_ERR;
}

export fn ph_close(fd: i32) void {
    _ = sys.close(fd);
}

// -----------------------------------------------------------------------------
// Files, directories and the process
// -----------------------------------------------------------------------------

// The host ABI's code for a failed call's -errno.
fn fs_err(r: isize) i32 {
    let e = -r;
    if e == sys.ENOENT {
        return -2;
    }
    if (e == sys.EACCES) or (e == sys.EPERM) {
        return -13;
    }
    if e == sys.EEXIST {
        return -17;
    }
    if e == sys.ENOTDIR {
        return -20;
    }
    if e == sys.EISDIR {
        return -21;
    }
    if e == sys.ENOTEMPTY {
        return -39;
    }
    return PH_ERR;
}

fn rd16(b: []u8, i: usize) u32 {
    return @as(u32, b[i]) | (@as(u32, b[i + 1]) << 8);
}

fn rd32(b: []u8, i: usize) u32 {
    return rd16(b, i) | (rd16(b, i + 2) << 16);
}

fn rd64(b: []u8, i: usize) u64 {
    return @as(u64, rd32(b, i)) | (@as(u64, rd32(b, i + 4)) << 32);
}

// out: kind (1 file, 2 directory, 3 symbolic link, 4 other), size,
// modification time (ns since 1970), permission bits
export fn ph_stat(path: *u8, len: usize, follow: u32, out: *mut i64) i32 {
    let mut buf: [4096]u8 = [0; 4096];
    if not copy_cstr(buf[..], path, len) {
        return PH_ERR;
    }
    let mut st: [256]u8 = [0; 256];
    let mut flags: i32 = 0x100;  // AT_SYMLINK_NOFOLLOW
    if follow != 0 {
        flags = 0;
    }
    let r = sys.statx(&buf[0], flags, &mut st[0]);
    if r < 0 {
        return fs_err(r);
    }
    let mode = rd16(st[..], 28);
    let fmt = mode & 0o170000;
    let mut kind: i64 = 4;
    if fmt == 0o100000 {
        kind = 1;
    } else if fmt == 0o040000 {
        kind = 2;
    } else if fmt == 0o120000 {
        kind = 3;
    }
    let o = @slice_from_ptr(out, 4);
    o[0] = kind;
    o[1] = @bit_cast(i64, rd64(st[..], 40));
    o[2] = (@bit_cast(i64, rd64(st[..], 112)) * 1000000000) + @as(i64, rd32(st[..], 120));
    o[3] = @as(i64, mode & 0o7777);
    return 0;
}

export fn ph_mkdir(path: *u8, len: usize) i32 {
    let mut buf: [4096]u8 = [0; 4096];
    if not copy_cstr(buf[..], path, len) {
        return PH_ERR;
    }
    let r = sys.mkdir(&buf[0], 0o777);
    if r < 0 {
        return fs_err(r);
    }
    return 0;
}

export fn ph_remove(path: *u8, len: usize, dir: u32) i32 {
    let mut buf: [4096]u8 = [0; 4096];
    if not copy_cstr(buf[..], path, len) {
        return PH_ERR;
    }
    let mut flags: i32 = 0;
    if dir != 0 {
        flags = 0x200;  // AT_REMOVEDIR
    }
    let r = sys.unlink(&buf[0], flags);
    if r < 0 {
        return fs_err(r);
    }
    return 0;
}

export fn ph_rename(from: *u8, flen: usize, to: *u8, tlen: usize) i32 {
    let mut a: [4096]u8 = [0; 4096];
    let mut b: [4096]u8 = [0; 4096];
    if not copy_cstr(a[..], from, flen) or not copy_cstr(b[..], to, tlen) {
        return PH_ERR;
    }
    let r = sys.rename(&a[0], &b[0]);
    if r < 0 {
        return fs_err(r);
    }
    return 0;
}

// Writes the directory's entries (not . and ..) to buf as a kind byte
// (as ph_stat's, 0 if unknown), the name and a 0 byte each; returns the
// bytes all of them need (more than cap: call again with more room).
export fn ph_readdir(path: *u8, len: usize, buf: *mut u8, cap: usize) isize {
    let mut name: [4096]u8 = [0; 4096];
    if not copy_cstr(name[..], path, len) {
        return @as(isize, PH_ERR);
    }
    let fd = sys.open(&name[0], 0x80000, 0);  // O_RDONLY | O_CLOEXEC
    if fd < 0 {
        return @as(isize, fs_err(fd));
    }
    let out = @slice_from_ptr(buf, cap);
    let mut need: usize = 0;
    let mut d: [8192]u8 = [0; 8192];
    loop {
        let n = sys.getdents64(@truncate(i32, fd), &mut d[0], d.len);
        if n < 0 {
            _ = sys.close(@truncate(i32, fd));
            return @as(isize, fs_err(n));
        }
        if n == 0 {
            break;
        }
        let mut p: usize = 0;
        while p < @bit_cast(usize, n) {
            let reclen = @as(usize, rd16(d[..], p + 16));
            let ty = d[p + 18];
            let mut e = p + 19;
            while (e < (p + reclen)) and (d[e] != 0) {
                e += 1;
            }
            let nm = d[p + 19..e];
            let dots = ((nm.len == 1) and (nm[0] == '.')) or ((nm.len == 2) and (nm[0] == '.') and (nm[1] == '.'));
            if not dots {
                let mut kind: u8 = 0;
                if ty == 8 {
                    kind = 1;  // DT_REG
                } else if ty == 4 {
                    kind = 2;  // DT_DIR
                } else if ty == 10 {
                    kind = 3;  // DT_LNK
                } else if ty != 0 {
                    kind = 4;
                }
                if (need + nm.len + 2) <= cap {
                    out[need] = kind;
                    @mem_copy(out[need + 1..need + 1 + nm.len], nm);
                    out[need + 1 + nm.len] = 0;
                }
                need += nm.len + 2;
            }
            p += reclen;
        }
    }
    _ = sys.close(@truncate(i32, fd));
    return @bit_cast(isize, need);
}

export fn ph_getcwd(buf: *mut u8, cap: usize) isize {
    let mut tmp: [4096]u8 = [0; 4096];
    let r = sys.getcwd(&mut tmp[0], tmp.len);
    if r < 0 {
        return @as(isize, fs_err(r));
    }
    let mut n: usize = 0;
    while (n < tmp.len) and (tmp[n] != 0) {
        n += 1;
    }
    if n <= cap {
        @mem_copy(@slice_from_ptr(buf, n), tmp[..n]);
    }
    return @bit_cast(isize, n);
}

// whence: 0 from the start, 1 from the current position, 2 from the end
export fn ph_seek(fd: i32, off: i64, whence: u32) i64 {
    let r = sys.lseek(fd, off, @bit_cast(i32, whence));
    if r < 0 {
        return @as(i64, fs_err(r));
    }
    return @as(i64, r);
}

export fn ph_argc() i32 {
    return @truncate(i32, @bit_cast(isize, sys.arg_count()));
}

// Copies argument i to buf; returns its length (-1 if there is none).
export fn ph_arg(i: i32, buf: *mut u8, cap: usize) isize {
    if i < 0 {
        return @as(isize, PH_ERR);
    }
    let a = sys.arg(@as(usize, @bit_cast(u32, i)));
    if (a.len == 0) and (@int_from_ptr(a.ptr) == 0) {
        return @as(isize, PH_ERR);
    }
    if a.len <= cap {
        @mem_copy(@slice_from_ptr(buf, a.len), a);
    }
    return @bit_cast(isize, a.len);
}

// Copies environment variable name's value to buf; returns its length (-2
// if it is not set).
export fn ph_getenv(name: *u8, len: usize, buf: *mut u8, cap: usize) isize {
    let v = sys.getenv(@slice_from_ptr(name, len));
    if (v.len == 0) and (@int_from_ptr(v.ptr) == 0) {
        return -2;
    }
    if v.len <= cap {
        @mem_copy(@slice_from_ptr(buf, v.len), v);
    }
    return @bit_cast(isize, v.len);
}

export fn ph_exit(code: i32) void {
    ph_rt_fini();
    sys.exit(code);
}

// -----------------------------------------------------------------------------
// Host names (IPv4)
// -----------------------------------------------------------------------------
// An IPv4 address, as its four bytes in network order (a in the low byte).
fn parse_ipv4(s: []u8) i64 {
    let mut v: u32 = 0;
    let mut part: u32 = 0;
    let mut digits: u32 = 0;
    let mut dots: u32 = 0;
    for i in 0..s.len + 1 {
        if (i == s.len) or (s[i] == '.') {
            if (digits == 0) or (part > 255) {
                return -1;
            }
            v |= part << (dots * 8);
            if i < s.len {
                dots += 1;
                if dots > 3 {
                    return -1;
                }
            }
            part = 0;
            digits = 0;
        } else if (s[i] >= '0') and (s[i] <= '9') {
            part = (part * 10) + @as(u32, s[i] - '0');
            digits += 1;
            if digits > 3 {
                return -1;
            }
        } else {
            return -1;
        }
    }
    if dots != 3 {
        return -1;
    }
    return @as(i64, v);
}

fn lower(c: u8) u8 {
    if (c >= 'A') and (c <= 'Z') {
        return c + 32;
    }
    return c;
}

fn same_name(a: []u8, b: []u8) bool {
    if a.len != b.len {
        return false;
    }
    for i in 0..a.len {
        if lower(a[i]) != lower(b[i]) {
            return false;
        }
    }
    return true;
}

fn is_space(c: u8) bool {
    return (c == ' ') or (c == '\t') or (c == '\r');
}

// Reads a small file (up to buf.len bytes); its bytes, or none.
fn read_small(path: []u8, buf: []mut u8) []u8 {
    let mut name: [256]u8 = [0; 256];
    if not copy_cstr(name[..], path.ptr, path.len) {
        return buf[0..0];
    }
    let fd = sys.open(&name[0], 0x80000, 0);
    if fd < 0 {
        return buf[0..0];
    }
    let mut n: usize = 0;
    while n < buf.len {
        let r = sys.read(@truncate(i32, fd), &mut buf[n], buf.len - n);
        if retry(r) {
            continue;
        }
        if r <= 0 {
            break;
        }
        n += @bit_cast(usize, r);
    }
    _ = sys.close(@truncate(i32, fd));
    return buf[0..n];
}

// The words of each line of `text` (comments removed), one line at a time.
struct Lines {
    text: []u8,
    pos: usize,
    words: [8][]u8,
    n: usize,

    fn next(self: *mut Self) bool {
        if self.pos >= self.text.len {
            return false;
        }
        let mut end = self.pos;
        while (end < self.text.len) and (self.text[end] != '\n') {
            end += 1;
        }
        let mut line = self.text[self.pos..end];
        self.pos = end + 1;
        for i in 0..line.len {
            if line[i] == '#' {
                line = line[0..i];
                break;
            }
        }
        self.n = 0;
        let mut i: usize = 0;
        while (i < line.len) and (self.n < 8) {
            while (i < line.len) and is_space(line[i]) {
                i += 1;
            }
            let s = i;
            while (i < line.len) and not is_space(line[i]) {
                i += 1;
            }
            if i > s {
                self.words[self.n] = line[s..i];
                self.n += 1;
            }
        }
        return true;
    }
}

fn lines(text: []u8) Lines {
    return Lines.{ text: text, pos: 0, words: [text[0..0]; 8], n: 0 };
}

let mut file_buf: [65536]u8 = [0; 65536];

fn from_hosts(name: []u8, out: []mut u32) usize {
    let mut ls = lines(read_small("/etc/hosts", file_buf[..]));
    let mut n: usize = 0;
    while ls.next() {
        if ls.n < 2 {
            continue;
        }
        let a = parse_ipv4(ls.words[0]);
        if a < 0 {
            continue;
        }
        for k in 1..ls.n {
            if same_name(ls.words[k], name) and (n < out.len) {
                out[n] = @truncate(u32, @bit_cast(u64, a));
                n += 1;
            }
        }
    }
    return n;
}

struct SockAddrIn {
    family: u16,
    port: u16,  // network byte order
    addr: u32,  // network byte order
    zero: u64,
}

fn sockaddr(a: u32, port: u32) SockAddrIn {
    let p = @truncate(u16, ((port & 0xff) << 8) | ((port >> 8) & 0xff));
    return SockAddrIn.{ family: 2, port: p, addr: a, zero: 0 };
}

// One DNS query for the A records of `name`, to name server `ns`.
fn dns_query(name: []u8, ns: u32, out: []mut u32) usize {
    let mut q: [512]u8 = [0; 512];
    let mut ts = sys.Timespec.{ sec: 0, nsec: 0 };
    _ = sys.clock_gettime(1, &mut ts);
    let id = @truncate(u16, @bit_cast(u64, ts.nsec) ^ @bit_cast(u64, ts.sec));
    q[0] = @truncate(u8, id >> 8);
    q[1] = @truncate(u8, id);
    q[2] = 1;  // recursion desired
    q[5] = 1;  // one question
    let mut n: usize = 12;
    let mut s: usize = 0;
    for i in 0..name.len + 1 {
        if (i == name.len) or (name[i] == '.') {
            let len = i - s;
            if (len == 0) or (len > 63) or ((n + len + 6) > q.len) {
                return 0;
            }
            q[n] = @truncate(u8, len);
            @mem_copy(q[n + 1..n + 1 + len], name[s..i]);
            n += len + 1;
            s = i + 1;
        }
    }
    q[n] = 0;
    q[n + 2] = 1;  // type A
    q[n + 4] = 1;  // class IN
    n += 5;
    let fd = sys.socket(2, 2 | 0x80000, 0);  // AF_INET, SOCK_DGRAM | SOCK_CLOEXEC
    if fd < 0 {
        return 0;
    }
    let sock = @truncate(i32, fd);
    let sa = sockaddr(ns, 53);
    let mut got: usize = 0;
    let mut r: [1024]u8 = [0; 1024];
    for _attempt in @as(u32, 0)..2 {
        if sys.sendto(sock, &q[0], n, 0, @int_from_ptr(&sa), 16) < 0 {
            break;
        }
        let mut pfd: [2]u32 = [@bit_cast(u32, sock), 1];  // POLLIN
        if sys.ppoll(@ptr_cast(*mut void, &mut pfd[0]), 1, 2000) <= 0 {
            continue;
        }
        let len = sys.recvfrom(sock, &mut r[0], r.len, 0);
        if len < 12 {
            continue;
        }
        got = parse_answer(r[0..@bit_cast(usize, len)], id, out);
        break;
    }
    _ = sys.close(sock);
    return got;
}

// Skips a (possibly compressed) name at i; the index after it.
fn skip_name(m: []u8, i0: usize) usize {
    let mut i = i0;
    while i < m.len {
        let c = m[i];
        if c == 0 {
            return i + 1;
        }
        if (c & 0xC0) == 0xC0 {
            return i + 2;
        }
        i += @as(usize, c) + 1;
    }
    return m.len;
}

fn be16(m: []u8, i: usize) usize {
    return (@as(usize, m[i]) << 8) | @as(usize, m[i + 1]);
}

fn parse_answer(m: []u8, id: u16, out: []mut u32) usize {
    if (be16(m, 0) != @as(usize, id)) or ((m[3] & 15) != 0) {
        return 0;
    }
    let qd = be16(m, 4);
    let an = be16(m, 6);
    let mut i: usize = 12;
    for _q in 0..qd {
        i = skip_name(m, i) + 4;
    }
    let mut n: usize = 0;
    for _a in 0..an {
        i = skip_name(m, i);
        if (i + 10) > m.len {
            break;
        }
        let ty = be16(m, i);
        let len = be16(m, i + 8);
        i += 10;
        if (i + len) > m.len {
            break;
        }
        if (ty == 1) and (len == 4) and (n < out.len) {
            out[n] = @as(u32, m[i]) | (@as(u32, m[i + 1]) << 8) | (@as(u32, m[i + 2]) << 16) | (
                @as(u32, m[i + 3]) << 24);
            n += 1;
        }
        i += len;
    }
    return n;
}

fn from_dns(name: []u8, out: []mut u32) usize {
    let mut servers: [4]u32 = [0; 4];
    let mut ns: usize = 0;
    let mut search: [4][]u8 = [name[0..0]; 4];
    let mut nsearch: usize = 0;
    let mut ls = lines(read_small("/etc/resolv.conf", file_buf[..]));
    while ls.next() {
        if (ls.n >= 2) and u_eq(ls.words[0], "nameserver") and (ns < 4) {
            let a = parse_ipv4(ls.words[1]);
            if a >= 0 {
                servers[ns] = @truncate(u32, @bit_cast(u64, a));
                ns += 1;
            }
        } else if (ls.n >= 2) and (u_eq(ls.words[0], "search") or u_eq(ls.words[0], "domain")) {
            for k in 1..ls.n {
                if nsearch < 4 {
                    search[nsearch] = ls.words[k];
                    nsearch += 1;
                }
            }
        }
    }
    if ns == 0 {
        servers[0] = 0x0100007F;  // 127.0.0.1
        ns = 1;
    }
    // the names to try: as given, then with each search domain if it has no dot
    let mut names: [5][]u8 = [name; 5];
    let mut storage: [5][256]u8 = [[0; 256]; 5];
    let mut nn: usize = 1;
    let mut dotted = false;
    for c in name {
        if c == '.' {
            dotted = true;
        }
    }
    if not dotted {
        for k in 0..nsearch {
            let d = search[k];
            if (name.len + 1 + d.len) <= 256 {
                @mem_copy(storage[nn][0..name.len], name);
                storage[nn][name.len] = '.';
                @mem_copy(storage[nn][name.len + 1..name.len + 1 + d.len], d);
                names[nn] = storage[nn][0..name.len + 1 + d.len];
                nn += 1;
            }
        }
    }
    for k in 0..nn {
        for s in 0..ns {
            let n = dns_query(names[k], servers[s], out);
            if n > 0 {
                return n;
            }
        }
    }
    return 0;
}

fn u_eq(a: []u8, b: []u8) bool {
    return (a.len == b.len) and @mem_eq(a, b);
}

// The IPv4 addresses of a host name (at most out.len); 0 if none.
fn resolve(name: []u8, out: []mut u32) usize {
    let a = parse_ipv4(name);
    if a >= 0 {
        out[0] = @truncate(u32, @bit_cast(u64, a));
        return 1;
    }
    if (name.len == 0) or (name.len > 253) {
        return 0;
    }
    if same_name(name, "localhost") {
        out[0] = 0x0100007F;
        return 1;
    }
    let n = from_hosts(name, out);
    if n > 0 {
        return n;
    }
    return from_dns(name, out);
}

export fn ph_tcp_connect(name: *u8, len: usize, port: u32) i32 {
    let mut addrs: [8]u32 = [0; 8];
    let n = resolve(@slice_from_ptr(name, len), addrs[..]);
    if n == 0 {
        return PH_BAD_ADDRESS;
    }
    let mut result = PH_ERR;
    for i in 0..n {
        let fd = sys.socket(2, 1 | 0x80000, 0);  // AF_INET, SOCK_STREAM | SOCK_CLOEXEC
        if fd < 0 {
            continue;
        }
        let sa = sockaddr(addrs[i], port);
        let r = sys.connect(@truncate(i32, fd), @ptr_cast(*void, &sa), 16);
        if r == 0 {
            return @truncate(i32, fd);
        }
        result = PH_ERR;
        if r == -sys.ECONNREFUSED {
            result = PH_REFUSED;
        }
        _ = sys.close(@truncate(i32, fd));
    }
    return result;
}

export fn ph_tcp_listen(name: *u8, len: usize, port: u32) i32 {
    let mut addrs: [8]u32 = [0; 8];
    let mut n: usize = 1;  // (no name: every address)
    if len > 0 {
        n = resolve(@slice_from_ptr(name, len), addrs[..]);
        if n == 0 {
            return PH_BAD_ADDRESS;
        }
    }
    let mut result = PH_ERR;
    for i in 0..n {
        let fd = sys.socket(2, 1 | 0x80000, 0);
        if fd < 0 {
            continue;
        }
        let sock = @truncate(i32, fd);
        let one: i32 = 1;
        _ = sys.setsockopt(sock, 1, 2, @ptr_cast(*void, &one), 4);  // SOL_SOCKET, SO_REUSEADDR
        let sa = sockaddr(addrs[i], port);
        let r = sys.bind(sock, @ptr_cast(*void, &sa), 16);
        if (r == 0) and (sys.listen(sock, 128) == 0) {
            return sock;
        }
        result = PH_ERR;
        if r == -sys.EADDRINUSE {
            result = PH_ADDR_IN_USE;
        }
        _ = sys.close(sock);
    }
    return result;
}

export fn ph_tcp_accept(listener: i32) i32 {
    loop {
        let fd = sys.accept4(listener, 0x80000);
        if retry(fd) {
            continue;
        }
        if fd < 0 {
            return PH_ERR;
        }
        return @truncate(i32, fd);
    }
}

export fn ph_tcp_local_port(h: i32) i32 {
    let mut ss: [128]u8 = [0; 128];
    let mut sl: u32 = 128;
    if sys.getsockname(h, @ptr_cast(*mut void, &mut ss[0]), &mut sl) != 0 {
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
        let r = sys.recvfrom(h, p, n, 0);
        if retry(r) {
            continue;
        }
        if r == -sys.EAGAIN {
            return @as(isize, PH_TIMED_OUT);
        }
        if r < 0 {
            return -1;
        }
        return r;
    }
}

export fn ph_sock_write(h: i32, p: *u8, n: usize) isize {
    loop {
        let r = sys.sendto(h, p, n, 0x4000, 0, 0);  // MSG_NOSIGNAL
        if retry(r) {
            continue;
        }
        if r == -sys.EAGAIN {
            return @as(isize, PH_TIMED_OUT);
        }
        if r < 0 {
            return -1;
        }
        return r;
    }
}

// Reads and writes on h fail with PH_TIMED_OUT after ms milliseconds (0:
// never).
export fn ph_sock_timeout(h: i32, ms: u32) i32 {
    let tv: [2]i64 = [@as(i64, ms / 1000), @as(i64, ms % 1000) * 1000];
    let a = sys.setsockopt(h, 1, 20, @ptr_cast(*void, &tv), 16);  // SOL_SOCKET, SO_RCVTIMEO
    let b = sys.setsockopt(h, 1, 21, @ptr_cast(*void, &tv), 16);  // SO_SNDTIMEO
    if (a != 0) or (b != 0) {
        return PH_ERR;
    }
    return 0;
}

// Shutting the socket down first wakes a thread blocked in accept on it.
export fn ph_sock_close(h: i32) void {
    _ = sys.shutdown(h, 2);  // SHUT_RDWR
    _ = sys.close(h);
}

// -----------------------------------------------------------------------------
// Threads (std/thread) and wait / notify
//
// Each thread gets one mapping: a guard page, its stack, its signal stack,
// its thread-local storage block and thread control block, and the
// ThreadStart record. The kernel clears `ctid` when the thread has exited;
// later spawns unmap such blocks.
// -----------------------------------------------------------------------------
const GUARD: usize = 65536;
const STACK: usize = 8 << 20;
const ALT: usize = 65536;

struct ThreadStart {
    entry: fn(*mut void) void,
    arg: *mut void,
    exited: *mut u32,
    base: usize,  // the mapping
    size: usize,
    alt: usize,   // the signal stack
    ctid: u32,    // cleared by the kernel when the thread is gone
    next: usize,  // earlier threads' records, until reclaimed
}

let mut next_thread_id: i32 = 1;
let mut threads: usize = 0;  // ThreadStart records of threads not yet reclaimed
let mut threads_lock: u32 = 0;

fn thread_run(p: *mut void) void {
    let st = @ptr_cast(*mut ThreadStart, p);
    _ = sys.sigaltstack(st.alt, ALT);
    st.entry(st.arg);
    @atomic_store(st.exited, 1);
    _ = ph_notify32(st.exited, 1);
}

// (pthread_create's calling convention: returns a pointer)
fn thread_main(p: *mut void) *mut void {
    thread_run(p);
    return @ptr_from_int(*mut void, 0);
}

fn align_up(x: usize, a: usize) usize {
    return (x + (a - 1)) & ~(a - 1);
}

// Unmaps the blocks of threads that have exited.
fn reclaim() void {
    while @atomic_xchg(&mut threads_lock, 1) != 0 {
        _ = sys.sched_yield();
    }
    let mut prev: usize = 0;
    let mut t = threads;
    while t != 0 {
        let st = @ptr_from_int(*mut ThreadStart, t);
        let next = st.next;
        if @atomic_load(&st.ctid) == 0 {
            if prev == 0 {
                threads = next;
            } else {
                @ptr_from_int(*mut ThreadStart, prev).next = next;
            }
            _ = sys.munmap(st.base, st.size);
        } else {
            prev = t;
        }
        t = next;
    }
    @atomic_store(&mut threads_lock, 0);
}

// stack_top is ignored: native threads get their stack from the runtime.
export fn ph_thread_spawn(entry: fn(*mut void) void, arg: *mut void, stack_top: usize, exited: *mut u32) i32 {
    reclaim();
    if sys.ph_libc_linked() != 0 {
        return spawn_pthread(entry, arg, exited);
    }
    let ti = sys.ph_tls_info();
    let align = @max(ti.align, 1);
    let tls_align = @max(align, 16);
    let tls_size = align_up(ti.memsz, tls_align) + (2 * tls_align) + 16;
    let size = align_up(GUARD + STACK + ALT + tls_size + @size_of(ThreadStart) + 64, 65536);
    let base = sys.mmap_anon(size);
    if base == 0 {
        return PH_ERR;
    }
    _ = sys.protect_none(base, GUARD);
    let stack_top_addr = base + GUARD + STACK;  // 16-aligned
    let alt = stack_top_addr;
    let tls_area = alt + ALT;
    let mut tp: usize = 0;
    let mut block: usize = 0;
    if sys.phalange_target_arch() == 2 {
        // variant I: TCB (16 bytes) at tp, then the block
        tp = align_up(tls_area, tls_align);
        block = tp + align_up(16, align);
    } else {
        // variant II: the block ends at tp, which points at itself
        block = align_up(tls_area, tls_align);
        tp = block + align_up(ti.memsz, align);
        @ptr_from_int(*mut usize, tp).* = tp;
    }
    if ti.filesz > 0 {
        @mem_copy(
            @slice_from_ptr(@ptr_from_int(*mut u8, block), ti.filesz), @slice_from_ptr(
                @ptr_from_int(*u8, ti.template), ti.filesz));
    }
    let st = @ptr_from_int(*mut ThreadStart, align_up(tls_area + tls_size, 16));
    st.* = ThreadStart.{ entry: entry, arg: arg, exited: exited, base: base, size: size, alt: alt, ctid: 1, next: 0 };
    let r = sys.ph_thread_create(thread_run, @ptr_cast(*mut void, st), stack_top_addr, tp, &mut st.ctid);
    if r < 0 {
        _ = sys.munmap(base, size);
        return PH_ERR;
    }
    while @atomic_xchg(&mut threads_lock, 1) != 0 {
        _ = sys.sched_yield();
    }
    st.next = threads;
    threads = @int_from_ptr(st);
    @atomic_store(&mut threads_lock, 0);
    return @atomic_add(&mut next_thread_id, 1);
}

// With the C library linked: its threads (and its own thread-local storage),
// so that C code runs on them too. The mapping holds the record and signal
// stack; it is kept for the life of the program.
fn spawn_pthread(entry: fn(*mut void) void, arg: *mut void, exited: *mut u32) i32 {
    let size = align_up(ALT + @size_of(ThreadStart) + 64, 65536);
    let base = sys.mmap_anon(size);
    if base == 0 {
        return PH_ERR;
    }
    let st = @ptr_from_int(*mut ThreadStart, base + ALT);
    st.* = ThreadStart.{ entry: entry, arg: arg, exited: exited, base: base, size: size, alt: base, ctid: 1, next: 0 };
    let mut t: u64 = 0;
    let mut attr: [64]u64 = [0; 64];
    _ = pthread_attr_init(@ptr_cast(*mut void, &mut attr[0]));
    _ = pthread_attr_setdetachstate(@ptr_cast(*mut void, &mut attr[0]), 1);  // PTHREAD_CREATE_DETACHED
    let rc = pthread_create(&mut t, @ptr_cast(*void, &attr[0]), thread_main, @ptr_cast(*mut void, st));
    _ = pthread_attr_destroy(@ptr_cast(*mut void, &mut attr[0]));
    if rc != 0 {
        _ = sys.munmap(base, size);
        return PH_ERR;
    }
    return @atomic_add(&mut next_thread_id, 1);
}

// Native threads need no stack from the program.
export fn ph_thread_stack_size() usize {
    return 0;
}

export fn ph_cpu_count() u32 {
    return sys.cpu_count();
}

export fn ph_sleep_ns(ns: u64) void {
    sys.nanosleep(ns);
}

// Fills p[0..n] with cryptographically secure random bytes; 0 on success.
export fn ph_random(p: *mut u8, n: usize) i32 {
    let mut done: usize = 0;
    while done < n {
        let r = sys.getrandom(@ptr_from_int(*mut u8, @int_from_ptr(p) + done), n - done);
        if retry(r) {
            continue;
        }
        if r <= 0 {
            return PH_ERR;
        }
        done += @int_cast(usize, r);
    }
    return 0;
}

// Nanoseconds on clock 0 (real time since 1970) or 1 (monotonic).
export fn ph_clock_ns(clock: u32) i64 {
    let mut ts = sys.Timespec.{ sec: 0, nsec: 0 };
    let mut id: i32 = 0;  // CLOCK_REALTIME
    if clock == 1 {
        id = 1;  // CLOCK_MONOTONIC
    }
    _ = sys.clock_gettime(id, &mut ts);
    return (ts.sec * 1000000000) + ts.nsec;
}

// Same results as WebAssembly memory.atomic.wait32: 0 woken, 1 not equal, 2 timed out.
export fn ph_wait32(p: *mut u32, expected: u32, timeout_ns: i64) i32 {
    let mut ts = sys.Timespec.{ sec: 0, nsec: 0 };
    let mut tsp: usize = 0;
    if timeout_ns >= 0 {
        ts.sec = timeout_ns / 1000000000;
        ts.nsec = timeout_ns % 1000000000;
        tsp = @int_from_ptr(&ts);
    }
    loop {
        let r = sys.futex(p, 128, expected, tsp);  // FUTEX_WAIT_PRIVATE
        if r == 0 {
            return 0;
        }
        if r == -sys.EAGAIN {
            return 1;
        }
        if r == -sys.ETIMEDOUT {
            return 2;
        }
        if r != -sys.EINTR {
            return 0;
        }
    }
}

export fn ph_notify32(p: *mut u32, count: i32) i32 {
    let mut n = count;
    if count < 0 {
        n = 0x7fff_ffff;
    }
    let r = sys.futex(p, 129, @bit_cast(u32, n), 0);  // FUTEX_WAKE_PRIVATE
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
            let p = sys.mmap_anon(size + 65536);
            if p != 0 {
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
        _ = sys.sched_yield();
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
        _ = sys.madvise(a, b - a, 4);  // MADV_DONTNEED
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
    sys.write_all(2, o.text());
    flush_profile();
}

// The stack pointer when a signal arrived, from its ucontext.
pub fn signal_sp(uc: *mut void) usize {
    let mut off: usize = 160;  // x86-64: uc_mcontext.gregs[REG_RSP]
    if sys.phalange_target_arch() == 2 {
        off = 432;  // AArch64: uc_mcontext.sp
    }
    return @ptr_from_int(*usize, @int_from_ptr(uc) + off).*;
}

// Was a fault at addr (with the stack pointer at sp) a stack overflow: an
// access just beyond the stack pointer, into a guard page?
pub fn at_stack_limit(addr: usize, sp: usize) bool {
    return ((addr + 65536) >= sp) and (addr < (sp + 65536));
}

// Crashes that are not Phalange traps (a stack overflow, or a fault in C code
// called through the FFI) are reported with the shadow stack, then the
// signal is re-raised.
fn on_crash(sig: i32, si: *mut void, uc: *mut void) void {
    let addr = @ptr_cast(*usize, @ptr_from_int(*u8, @int_from_ptr(si) + 16)).*;
    let mut code: u32 = 5;
    if sig == SIGFPE {
        code = 3;
    } else if at_stack_limit(addr, signal_sp(uc)) {
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
    sys.write_all(2, o.text());
    flush_profile();
    _ = sys.sigdefault(sig);
    _ = sys.raise(sig);
}

// The main thread's signal stack (threads have theirs in their mapping).
pub fn install_crash_stack() void {
    let p = sys.mmap_anon(ALT);
    if p != 0 {
        _ = sys.sigaltstack(p, ALT);
    }
}

pub fn set_handler(sig: i32, f: fn(i32, *mut void, *mut void) void, flags: u64) void {
    _ = sys.sigaction(sig, f, flags);
}

// main's arguments (the environment follows argv).
export fn ph_rt_args(argc: i32, argv: *mut *mut u8) void {
    sys.main_argc = @as(usize, @bit_cast(u32, argc));
    sys.main_argv = @int_from_ptr(argv);
}

// Runs before main.
export fn ph_rt_init() void {
    install_crash_stack();
    let flags: u64 = 4 | 0x0800_0000;  // SA_SIGINFO | SA_ONSTACK
    set_handler(SIGSEGV, on_crash, flags);
    set_handler(SIGBUS, on_crash, flags);
    set_handler(SIGFPE, on_crash, flags);
}

// Runs after main returns.
export fn ph_rt_fini() void {
    flush_profile();
}
