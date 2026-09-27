// =============================================================================
// util.ph - the C library, arena allocation, growable lists, interned
// strings, hash maps, 128-bit integers, formatting, sorting and source files.
//
// Everything the compiler allocates lives in one arena and is never freed.
// Pointers that C would make NULL are 0 here: use nil() and isnil().
// =============================================================================

pub extern "C" {
    fn malloc(n: usize) *mut void;
    fn calloc(n: usize, size: usize) *mut void;
    fn realloc(p: *mut void, n: usize) *mut void;
    fn free(p: *mut void) void;
    fn strlen(s: *u8) usize;
    fn strtod(s: *u8, end: *mut *mut u8) f64;
    fn snprintf(buf: *mut u8, n: usize, fmt: *u8, ...) i32;
    fn write(fd: i32, p: *u8, n: usize) isize;
    fn read(fd: i32, p: *mut u8, n: usize) isize;
    fn fopen(path: *u8, mode: *u8) *mut void;
    fn fclose(f: *mut void) i32;
    fn fread(p: *mut u8, size: usize, n: usize, f: *mut void) usize;
    fn fwrite(p: *u8, size: usize, n: usize, f: *mut void) usize;
    fn fseek(f: *mut void, off: isize, whence: i32) i32;
    fn ftell(f: *mut void) isize;
    fn exit(code: i32) void;
    fn _exit(code: i32) void;
    fn getenv(name: *u8) *u8;
    fn setenv(name: *u8, value: *u8, overwrite: i32) i32;
    fn realpath(path: *u8, resolved: *mut u8) *mut u8;
    fn getcwd(buf: *mut u8, n: usize) *mut u8;
    fn readlink(path: *u8, buf: *mut u8, n: usize) isize;
    fn access(path: *u8, mode: i32) i32;
    fn chmod(path: *u8, mode: u32) i32;
    fn getpid() i32;
    fn unlink(path: *u8) i32;
    fn rename(from: *u8, to: *u8) i32;
    fn mkdir(path: *u8, mode: u32) i32;
    fn opendir(path: *u8) *mut void;
    fn readdir(d: *mut void) *mut u8;
    fn closedir(d: *mut void) i32;
    fn fork() i32;
    fn execvp(file: *u8, argv: *mut *mut u8) i32;
    fn waitpid(pid: i32, status: *mut i32, options: i32) i32;
    fn wait(status: *mut i32) i32;
    fn pipe(fds: *mut i32) i32;
    fn dup2(a: i32, b: i32) i32;
    fn close(fd: i32) i32;
    fn open(path: *u8, flags: i32, ...) i32;
    fn sysconf(name: i32) isize;
    fn clock_gettime(clock: i32, ts: *mut Timespec) i32;
}

pub struct Timespec {
    sec: i64,
    nsec: i64,
}

// -----------------------------------------------------------------------------
// Pointers
// -----------------------------------------------------------------------------
pub fn nil[T]() *mut T {
    return @ptr_from_int(*mut T, 0);
}

pub fn isnil[T](p: *T) bool {
    return @int_from_ptr(p) == 0;
}

pub fn notnil[T](p: *T) bool {
    return @int_from_ptr(p) != 0;
}

pub fn vnil() *mut void {
    return @ptr_from_int(*mut void, 0);
}

pub fn isvnil(p: *void) bool {
    return @int_from_ptr(p) == 0;
}

pub fn die(msg: []u8) void {
    eprint("phalange: ");
    eprint(msg);
    eprint("\n");
    quit(2);
}

// -----------------------------------------------------------------------------
// Arena: zeroed, 16-byte aligned, never freed.
// -----------------------------------------------------------------------------
let mut arena_cur: usize = 0;
let mut arena_end: usize = 0;

pub fn alloc(size0: usize) *mut u8 {
    let size = (size0 + 15) & ~15;
    if (arena_end - arena_cur) < size {
        let mut bs: usize = 1048576;
        if size > bs {
            bs = size;
        }
        let p = calloc(1, bs);
        if isvnil(p) {
            die("out of memory");
        }
        arena_cur = @int_from_ptr(p);
        arena_end = arena_cur + bs;
    }
    let r = arena_cur;
    arena_cur += size;
    return @ptr_from_int(*mut u8, r);
}

pub fn new[T]() *mut T {
    return @ptr_cast(*mut T, alloc(@size_of(T)));
}

pub fn newn[T](n: usize) []mut T {
    let p = alloc(n * @size_of(T) + 1);
    return @slice_from_ptr(@ptr_cast(*mut T, p), n);
}

// A copy of `s` in the arena, followed by a NUL (so `.ptr` is a C string).
pub fn dup(s: []u8) []u8 {
    let p = alloc(s.len + 1);
    let d = @slice_from_ptr(p, s.len);
    @mem_copy(d, s);
    return d;
}

pub fn nosym() []u8 {
    return @slice_from_ptr(@ptr_from_int(*u8, 0), 0);
}

pub fn isnos(s: []u8) bool {
    return @int_from_ptr(s.ptr) == 0;
}

// A C string for `s`: a NUL-terminated arena copy.
pub fn cz(s: []u8) *u8 {
    return dup(s).ptr;
}

pub fn from_c(p: *u8) []u8 {
    return @slice_from_ptr(p, strlen(p));
}

// -----------------------------------------------------------------------------
// Growable lists (heap-backed; a zeroed List is empty and valid).
// -----------------------------------------------------------------------------
pub struct List[T] {
    data: *mut T,
    len: usize,
    cap: usize,

    pub fn push(self: *mut Self, x: T) void {
        if self.len == self.cap {
            let mut ncap: usize = 8;
            if self.cap != 0 {
                ncap = self.cap * 2;
            }
            let p = realloc(@ptr_cast(*mut void, self.data), ncap * @size_of(T));
            if isvnil(p) {
                die("out of memory");
            }
            self.data = @ptr_cast(*mut T, p);
            self.cap = ncap;
        }
        let s = @slice_from_ptr(self.data, self.len + 1);
        s[self.len] = x;
        self.len += 1;
    }

    pub fn items(self: *Self) []mut T {
        return @slice_from_ptr(self.data, self.len);
    }

    pub fn at(self: *Self, i: usize) *mut T {
        let s = @slice_from_ptr(self.data, self.len);
        return &mut s[i];
    }

    pub fn get(self: *Self, i: usize) T {
        let s = @slice_from_ptr(self.data, self.len);
        return s[i];
    }

    pub fn set(self: *Self, i: usize, x: T) void {
        let s = @slice_from_ptr(self.data, self.len);
        s[i] = x;
    }

    pub fn last(self: *Self) T {
        return self.get(self.len - 1);
    }

    pub fn pop(self: *mut Self) T {
        self.len -= 1;
        let s = @slice_from_ptr(self.data, self.len + 1);
        return s[self.len];
    }

    pub fn clear(self: *mut Self) void {
        self.len = 0;
    }
}

// -----------------------------------------------------------------------------
// String building
// -----------------------------------------------------------------------------
pub struct Buf {
    b: List.[u8],

    pub fn init() Buf {
        return Buf.{ b: List.[u8].{ data: nil.[u8](), len: 0, cap: 0 } };
    }

    pub fn s(self: *mut Buf, x: []u8) void {
        for c in x {
            self.b.push(c);
        }
    }

    pub fn c(self: *mut Buf, x: u8) void {
        self.b.push(x);
    }

    pub fn len(self: *Buf) usize {
        return self.b.len;
    }

    pub fn view(self: *Buf) []u8 {
        return self.b.items();
    }

    // An arena copy (NUL-terminated); the buffer is released.
    pub fn done(self: *mut Buf) []u8 {
        let r = dup(self.b.items());
        free(@ptr_cast(*mut void, self.b.data));
        self.b.data = nil.[u8]();
        self.b.len = 0;
        self.b.cap = 0;
        return r;
    }
}

pub fn cat2(a: []u8, b: []u8) []u8 {
    let p = alloc(a.len + b.len + 1);
    let d = @slice_from_ptr(p, a.len + b.len);
    @mem_copy(d[0..a.len], a);
    @mem_copy(d[a.len..], b);
    return d;
}

pub fn cat3(a: []u8, b: []u8, c: []u8) []u8 {
    return cat2(cat2(a, b), c);
}

pub fn cat4(a: []u8, b: []u8, c: []u8, d: []u8) []u8 {
    return cat2(cat2(a, b), cat2(c, d));
}

// Formatting: every `%s` takes the next argument, `%%` is a percent sign.
pub fn fmtv(f: []u8, args: [][]u8) []u8 {
    let mut b = Buf.init();
    let mut k: usize = 0;
    let mut i: usize = 0;
    while i < f.len {
        let ch = f[i];
        if (ch == '%') and ((i + 1) < f.len) {
            let n = f[i + 1];
            if n == 's' {
                b.s(args[k]);
                k += 1;
                i += 2;
                continue;
            }
            if n == '%' {
                b.c('%');
                i += 2;
                continue;
            }
        }
        b.c(ch);
        i += 1;
    }
    return b.done();
}

pub fn f1(f: []u8, a: []u8) []u8 {
    let v = [a];
    return fmtv(f, v[..]);
}

pub fn f2(f: []u8, a: []u8, b: []u8) []u8 {
    let v = [a, b];
    return fmtv(f, v[..]);
}

pub fn f3(f: []u8, a: []u8, b: []u8, c: []u8) []u8 {
    let v = [a, b, c];
    return fmtv(f, v[..]);
}

pub fn f4(f: []u8, a: []u8, b: []u8, c: []u8, d: []u8) []u8 {
    let v = [a, b, c, d];
    return fmtv(f, v[..]);
}

pub fn f5(f: []u8, a: []u8, b: []u8, c: []u8, d: []u8, e: []u8) []u8 {
    let v = [a, b, c, d, e];
    return fmtv(f, v[..]);
}

pub fn f6(f: []u8, a: []u8, b: []u8, c: []u8, d: []u8, e: []u8, g: []u8) []u8 {
    let v = [a, b, c, d, e, g];
    return fmtv(f, v[..]);
}

pub fn udec(v0: u64) []u8 {
    let mut buf: [24]u8 = [0; 24];
    let mut n: usize = 24;
    let mut v = v0;
    loop {
        n -= 1;
        buf[n] = '0' + @truncate(u8, v % 10);
        v = v / 10;
        if v == 0 {
            break;
        }
    }
    return dup(buf[n..]);
}

pub fn dec(v: i64) []u8 {
    if v < 0 {
        return cat2("-", udec(0 -% @bit_cast(u64, v)));
    }
    return udec(@bit_cast(u64, v));
}

pub fn idec(v: i32) []u8 {
    return dec(@as(i64, v));
}

pub fn zdec(v: usize) []u8 {
    return udec(@as(u64, v));
}

pub fn hex16(v: u64) []u8 {
    let digits = "0123456789abcdef";
    let mut buf: [16]u8 = [0; 16];
    for i in @as(usize, 0)..16 {
        buf[15 - i] = digits[@truncate(usize, (v >> @truncate(u32, i * 4)) & 15)];
    }
    return dup(buf[..]);
}

// Formats one float with a printf conversion such as "%.1f".
pub fn ffmt(f: []u8, v: f64) []u8 {
    let mut buf: [64]u8 = [0; 64];
    let n = snprintf(&mut buf[0], 64, cz(f), v);
    return dup(buf[0..@int_cast(usize, n)]);
}

pub fn pick(c: bool, a: []u8, b: []u8) []u8 {
    if c {
        return a;
    }
    return b;
}

pub fn str_eq(a: []u8, b: []u8) bool {
    return @mem_eq(a, b);
}

// strcmp on the bytes (as unsigned), treating the slice end as NUL.
pub fn str_cmp(a: []u8, b: []u8) i32 {
    let mut i: usize = 0;
    loop {
        let mut ca: u8 = 0;
        let mut cb: u8 = 0;
        if i < a.len {
            ca = a[i];
        }
        if i < b.len {
            cb = b[i];
        }
        if ca != cb {
            return @as(i32, ca) - @as(i32, cb);
        }
        if ca == 0 {
            return 0;
        }
        i += 1;
    }
}

pub fn starts(s: []u8, p: []u8) bool {
    return (s.len >= p.len) and @mem_eq(s[0..p.len], p);
}

pub fn ends(s: []u8, p: []u8) bool {
    return (s.len >= p.len) and @mem_eq(s[s.len - p.len..], p);
}

// Index of the last `c` in `s`, or -1.
pub fn rfind(s: []u8, c: u8) i64 {
    let mut i = s.len;
    while i > 0 {
        i -= 1;
        if s[i] == c {
            return @int_cast(i64, i);
        }
    }
    return -1;
}

pub fn find(s: []u8, c: u8) i64 {
    for i in 0..s.len {
        if s[i] == c {
            return @int_cast(i64, i);
        }
    }
    return -1;
}

pub fn is_digit(c: u8) bool {
    return (c >= '0') and (c <= '9');
}

pub fn is_alnum(c: u8) bool {
    return ((c >= 'a') and (c <= 'z')) or ((c >= 'A') and (c <= 'Z')) or is_digit(c);
}

// atoi / atol: optional sign, decimal digits; stops at the first non-digit.
pub fn atol(s: []u8) i64 {
    let mut i: usize = 0;
    let mut neg = false;
    if (i < s.len) and ((s[i] == '-') or (s[i] == '+')) {
        neg = s[i] == '-';
        i += 1;
    }
    let mut v: i64 = 0;
    while (i < s.len) and is_digit(s[i]) {
        v = (v *% 10) +% @as(i64, s[i] - '0');
        i += 1;
    }
    if neg {
        return 0 -% v;
    }
    return v;
}

// C's strtoll / strtoull with base 0 (0x hex, leading-0 octal, decimal).
pub struct CInt {
    value: u64,     // magnitude, clamped to u64 max
    neg: bool,
    end: usize,     // bytes consumed (0: no conversion)
    big: bool,      // magnitude exceeded u64
}

fn digit_val(c: u8) u32 {
    if is_digit(c) {
        return @as(u32, c - '0');
    }
    if (c >= 'a') and (c <= 'f') {
        return @as(u32, c - 'a') + 10;
    }
    if (c >= 'A') and (c <= 'F') {
        return @as(u32, c - 'A') + 10;
    }
    return 99;
}

pub fn c_parse_int(s: []u8) CInt {
    let mut r = CInt.{ value: 0, neg: false, end: 0, big: false };
    let mut i: usize = 0;
    while (i < s.len) and ((s[i] == ' ') or (s[i] == '\t') or (s[i] == '\n')) {
        i += 1;
    }
    if (i < s.len) and ((s[i] == '-') or (s[i] == '+')) {
        r.neg = s[i] == '-';
        i += 1;
    }
    let mut base: u32 = 10;
    if (i < s.len) and (s[i] == '0') {
        if ((i + 2) < s.len) and ((s[i + 1] == 'x') or (s[i + 1] == 'X')) and (digit_val(s[i + 2]) < 16) {
            base = 16;
            i += 2;
        } else {
            base = 8;
        }
    }
    let start = i;
    while (i < s.len) and (digit_val(s[i]) < base) {
        let d = @as(u64, digit_val(s[i]));
        if r.value > ((0xFFFF_FFFF_FFFF_FFFF - d) / @as(u64, base)) {
            r.big = true;
        } else {
            r.value = (r.value * @as(u64, base)) + d;
        }
        i += 1;
    }
    if r.big {
        r.value = 0xFFFF_FFFF_FFFF_FFFF;
    }
    if i == start {
        r.end = 0;
        r.value = 0;
        r.neg = false;
    } else {
        r.end = i;
    }
    return r;
}

// strtoll(s, &end, 0): the value (clamped); `ovf` reports ERANGE.
pub fn c_strtoll(s: []u8) CStrtoll {
    let p = c_parse_int(s);
    let mut r = CStrtoll.{ v: 0, end: p.end, ovf: false };
    if p.neg {
        if p.big or (p.value > 0x8000_0000_0000_0000) {
            r.v = -9223372036854775807 - 1;
            r.ovf = true;
        } else {
            r.v = 0 -% @bit_cast(i64, p.value);
        }
    } else {
        if p.big or (p.value > 0x7FFF_FFFF_FFFF_FFFF) {
            r.v = 9223372036854775807;
            r.ovf = true;
        } else {
            r.v = @bit_cast(i64, p.value);
        }
    }
    return r;
}

pub struct CStrtoll {
    v: i64,
    end: usize,
    ovf: bool,
}

// strtoull(s, &end, 0) as a signed 64-bit pattern.
pub fn c_strtoull(s: []u8) CStrtoll {
    let p = c_parse_int(s);
    let mut r = CStrtoll.{ v: 0, end: p.end, ovf: p.big };
    let mut u = p.value;
    if p.neg and not p.big {
        u = 0 -% u;
    }
    r.v = @bit_cast(i64, u);
    return r;
}

// strtod over a slice (which must be followed by a NUL, as arena strings are).
pub fn c_strtod(s: []u8) CFloat {
    let z = cz(s);
    let mut end: *mut u8 = nil.[u8]();
    let v = strtod(z, &mut end);
    return CFloat.{ v: v, end: @int_from_ptr(end) - @int_from_ptr(z) };
}

pub struct CFloat {
    v: f64,
    end: usize,
}

// -----------------------------------------------------------------------------
// Interned strings: equal contents => the same pointer. A Sym is a []u8 whose
// bytes are followed by a NUL; the absent Sym (C's NULL) has a 0 pointer.
// -----------------------------------------------------------------------------
struct InternEntry {
    s: []u8,
    hash: u64,
}

let mut itab: *mut InternEntry = @ptr_from_int(*mut InternEntry, 0);
let mut itab_cap: usize = 0;
let mut itab_len: usize = 0;

pub fn hash_fnv(s: []u8) u64 {
    let mut h: u64 = 1469598103934665603;
    for c in s {
        h = (h ^ @as(u64, c)) *% 1099511628211;
    }
    return h;
}

pub fn intern(s: []u8) []u8 {
    if (itab_len * 2) >= itab_cap {
        let mut ncap: usize = 4096;
        if itab_cap != 0 {
            ncap = itab_cap * 2;
        }
        let nt = @slice_from_ptr(@ptr_cast(*mut InternEntry, calloc(ncap, @size_of(InternEntry))), ncap);
        let old = @slice_from_ptr(itab, itab_cap);
        for i in 0..itab_cap {
            if isnos(old[i].s) {
                continue;
            }
            let mut j = @truncate(usize, old[i].hash) & (ncap - 1);
            while not isnos(nt[j].s) {
                j = (j + 1) & (ncap - 1);
            }
            nt[j] = old[i];
        }
        free(@ptr_cast(*mut void, itab));
        itab = nt.ptr;
        itab_cap = ncap;
    }
    let t = @slice_from_ptr(itab, itab_cap);
    let h = hash_fnv(s);
    let mut j = @truncate(usize, h) & (itab_cap - 1);
    while not isnos(t[j].s) {
        if (t[j].hash == h) and @mem_eq(t[j].s, s) {
            return t[j].s;
        }
        j = (j + 1) & (itab_cap - 1);
    }
    t[j] = InternEntry.{ s: dup(s), hash: h };
    itab_len += 1;
    return t[j].s;
}

// Interned strings compare by pointer.
pub fn same(a: []u8, b: []u8) bool {
    return a.ptr == b.ptr;
}

// -----------------------------------------------------------------------------
// SymMap: interned-string -> V hash map. A missing key reads as a zeroed V.
// -----------------------------------------------------------------------------
fn ptr_hash(p: usize) usize {
    let mut x = @as(u64, p);
    x = x ^ (x >> 33);
    x = x *% 0xff51afd7ed558ccd;
    x = x ^ (x >> 33);
    return @truncate(usize, x);
}

pub struct SymMap[V] {
    keys: *mut usize,
    vals: *mut V,
    cap: usize,
    len: usize,

    fn grow(self: *mut Self) void {
        let mut ncap: usize = 16;
        if self.cap != 0 {
            ncap = self.cap * 2;
        }
        let nk = @slice_from_ptr(@ptr_cast(*mut usize, calloc(ncap, @size_of(usize))), ncap);
        let nv = @slice_from_ptr(@ptr_cast(*mut V, calloc(ncap, @size_of(V))), ncap);
        let ok = @slice_from_ptr(self.keys, self.cap);
        let ov = @slice_from_ptr(self.vals, self.cap);
        for i in 0..self.cap {
            if ok[i] == 0 {
                continue;
            }
            let mut j = ptr_hash(ok[i]) & (ncap - 1);
            while nk[j] != 0 {
                j = (j + 1) & (ncap - 1);
            }
            nk[j] = ok[i];
            nv[j] = ov[i];
        }
        free(@ptr_cast(*mut void, self.keys));
        free(@ptr_cast(*mut void, self.vals));
        self.keys = nk.ptr;
        self.vals = nv.ptr;
        self.cap = ncap;
    }

    fn slot(self: *mut Self, k: []u8) usize {
        if self.cap == 0 {
            self.grow();
        }
        let key = @int_from_ptr(k.ptr);
        let ks = @slice_from_ptr(self.keys, self.cap);
        let mut j = ptr_hash(key) & (self.cap - 1);
        while (ks[j] != 0) and (ks[j] != key) {
            j = (j + 1) & (self.cap - 1);
        }
        return j;
    }

    pub fn get(self: *mut Self, k: []u8) V {
        let j = self.slot(k);
        let vs = @slice_from_ptr(self.vals, self.cap);
        return vs[j];
    }

    pub fn has(self: *mut Self, k: []u8) bool {
        let j = self.slot(k);
        let ks = @slice_from_ptr(self.keys, self.cap);
        return ks[j] != 0;
    }

    pub fn put(self: *mut Self, k: []u8, v: V) void {
        if ((self.len + 1) * 2) >= self.cap {
            self.grow();
        }
        let j = self.slot(k);
        let ks = @slice_from_ptr(self.keys, self.cap);
        let vs = @slice_from_ptr(self.vals, self.cap);
        if ks[j] == 0 {
            ks[j] = @int_from_ptr(k.ptr);
            self.len += 1;
        }
        vs[j] = v;
    }
}

// A map key for an object's identity.
pub fn addr_key(p: usize) []u8 {
    return intern(udec(@as(u64, p)));
}

// -----------------------------------------------------------------------------
// Sorting: a stable merge sort. `cmp` returns <0, 0 or >0 like qsort's.
// -----------------------------------------------------------------------------
pub fn sort[T](xs: []mut T, cmp: fn(T, T) i32) void {
    if xs.len < 2 {
        return;
    }
    let tmp = @slice_from_ptr(@ptr_cast(*mut T, malloc(xs.len * @size_of(T))), xs.len);
    merge_sort(xs, tmp, cmp);
    free(@ptr_cast(*mut void, tmp.ptr));
}

fn merge_sort[T](xs: []mut T, tmp: []mut T, cmp: fn(T, T) i32) void {
    let n = xs.len;
    if n < 2 {
        return;
    }
    let h = n / 2;
    merge_sort(xs[0..h], tmp[0..h], cmp);
    merge_sort(xs[h..n], tmp[h..n], cmp);
    let mut i: usize = 0;
    let mut j = h;
    let mut k: usize = 0;
    while (i < h) and (j < n) {
        if cmp(xs[j], xs[i]) < 0 {
            tmp[k] = xs[j];
            j += 1;
        } else {
            tmp[k] = xs[i];
            i += 1;
        }
        k += 1;
    }
    while i < h {
        tmp[k] = xs[i];
        i += 1;
        k += 1;
    }
    while j < n {
        tmp[k] = xs[j];
        j += 1;
        k += 1;
    }
    @mem_copy(xs, tmp[0..n]);
}

// -----------------------------------------------------------------------------
// 128-bit two's complement integers (constant folding needs them).
// -----------------------------------------------------------------------------
pub struct I128 {
    lo: u64,
    hi: u64,

    pub fn of(v: i64) I128 {
        let mut hi: u64 = 0;
        if v < 0 {
            hi = 0xFFFF_FFFF_FFFF_FFFF;
        }
        return I128.{ lo: @bit_cast(u64, v), hi: hi };
    }

    pub fn ofu(v: u64) I128 {
        return I128.{ lo: v, hi: 0 };
    }

    pub fn zero() I128 {
        return I128.{ lo: 0, hi: 0 };
    }

    // 2^k (k < 127)
    pub fn pow2(k: u32) I128 {
        return I128.ofu(1).shl(k);
    }

    pub fn is_neg(self: I128) bool {
        return (self.hi >> 63) != 0;
    }

    pub fn is_zero(self: I128) bool {
        return (self.lo == 0) and (self.hi == 0);
    }

    pub fn add(self: I128, b: I128) I128 {
        let lo = self.lo +% b.lo;
        let mut carry: u64 = 0;
        if lo < self.lo {
            carry = 1;
        }
        return I128.{ lo: lo, hi: (self.hi +% b.hi) +% carry };
    }

    pub fn not_(self: I128) I128 {
        return I128.{ lo: ~self.lo, hi: ~self.hi };
    }

    pub fn neg(self: I128) I128 {
        return self.not_().add(I128.ofu(1));
    }

    pub fn sub(self: I128, b: I128) I128 {
        return self.add(b.neg());
    }

    pub fn and_(self: I128, b: I128) I128 {
        return I128.{ lo: self.lo & b.lo, hi: self.hi & b.hi };
    }

    pub fn or_(self: I128, b: I128) I128 {
        return I128.{ lo: self.lo | b.lo, hi: self.hi | b.hi };
    }

    pub fn xor(self: I128, b: I128) I128 {
        return I128.{ lo: self.lo ^ b.lo, hi: self.hi ^ b.hi };
    }

    pub fn mul(self: I128, b: I128) I128 {
        let p = mul_full(self.lo, b.lo);
        let hi = (p.hi +% (self.lo *% b.hi)) +% (self.hi *% b.lo);
        return I128.{ lo: p.lo, hi: hi };
    }

    pub fn shl(self: I128, k: u32) I128 {
        if k == 0 {
            return self;
        }
        if k >= 128 {
            return I128.zero();
        }
        if k >= 64 {
            return I128.{ lo: 0, hi: self.lo << (k - 64) };
        }
        return I128.{ lo: self.lo << k, hi: (self.hi << k) | (self.lo >> (64 - k)) };
    }

    // arithmetic shift right (floor)
    pub fn sar(self: I128, k: u32) I128 {
        if k == 0 {
            return self;
        }
        let shi = @bit_cast(i64, self.hi);
        if k >= 128 {
            return I128.of(shi >> 63);
        }
        if k >= 64 {
            return I128.{ lo: @bit_cast(u64, shi >> (k - 64)), hi: @bit_cast(u64, shi >> 63) };
        }
        return I128.{ lo: (self.lo >> k) | (self.hi << (64 - k)), hi: @bit_cast(u64, shi >> k) };
    }

    pub fn eq(self: I128, b: I128) bool {
        return (self.lo == b.lo) and (self.hi == b.hi);
    }

    pub fn lt(self: I128, b: I128) bool {
        let a1 = @bit_cast(i64, self.hi);
        let b1 = @bit_cast(i64, b.hi);
        if a1 != b1 {
            return a1 < b1;
        }
        return self.lo < b.lo;
    }

    pub fn le(self: I128, b: I128) bool {
        return not b.lt(self);
    }

    pub fn gt(self: I128, b: I128) bool {
        return b.lt(self);
    }

    pub fn ge(self: I128, b: I128) bool {
        return not self.lt(b);
    }

    pub fn lti(self: I128, b: i64) bool {
        return self.lt(I128.of(b));
    }

    pub fn gti(self: I128, b: i64) bool {
        return self.gt(I128.of(b));
    }

    pub fn eqi(self: I128, b: i64) bool {
        return self.eq(I128.of(b));
    }

    // The low 64 bits, as C's (int64_t) / (uint64_t) casts.
    pub fn i64v(self: I128) i64 {
        return @bit_cast(i64, self.lo);
    }

    pub fn u64v(self: I128) u64 {
        return self.lo;
    }

    pub fn abs_u(self: I128) I128 {
        if self.is_neg() {
            return self.neg();
        }
        return self;
    }

    // Truncating signed division and remainder (C semantics).
    pub fn div(self: I128, b: I128) I128 {
        let q = udivmod(self.abs_u(), b.abs_u()).q;
        if self.is_neg() != b.is_neg() {
            return q.neg();
        }
        return q;
    }

    pub fn rem(self: I128, b: I128) I128 {
        let r = udivmod(self.abs_u(), b.abs_u()).r;
        if self.is_neg() {
            return r.neg();
        }
        return r;
    }

    // (double)v, rounded to nearest, ties to even.
    pub fn to_f64(self: I128) f64 {
        let neg = self.is_neg();
        let m = self.abs_u();
        let mut r: f64 = 0.0;
        if m.hi == 0 {
            r = @float_from_int(f64, m.lo);
        } else {
            // top bit position p >= 64: keep 53 bits, round to nearest even.
            let p = 127 - @truncate(u32, @clz(m.hi));
            let s = p - 52;
            let mant = m.sar(s).lo;
            let rem_mask = I128.ofu(1).shl(s).sub(I128.ofu(1));
            let rest = m.and_(rem_mask);
            let half = I128.ofu(1).shl(s - 1);
            let mut mm = mant;
            if rest.gt(half) or (rest.eq(half) and ((mant & 1) == 1)) {
                mm += 1;
            }
            r = @float_from_int(f64, mm);
            let mut k: u32 = 0;
            while k < s {
                r = r * 2.0;
                k += 1;
            }
        }
        if neg {
            return -r;
        }
        return r;
    }

    pub fn str(self: I128) []u8 {
        let neg = self.is_neg();
        let mut m = self.abs_u();
        let mut buf: [48]u8 = [0; 48];
        let mut n: usize = 48;
        let ten = I128.ofu(10);
        loop {
            let d = udivmod(m, ten);
            n -= 1;
            buf[n] = '0' + @truncate(u8, d.r.lo);
            m = d.q;
            if m.is_zero() {
                break;
            }
        }
        if neg {
            n -= 1;
            buf[n] = '-';
        }
        return dup(buf[n..]);
    }
}

struct U128Pair {
    lo: u64,
    hi: u64,
}

fn mul_full(a: u64, b: u64) U128Pair {
    let m32: u64 = 0xFFFF_FFFF;
    let a0 = a & m32;
    let a1 = a >> 32;
    let b0 = b & m32;
    let b1 = b >> 32;
    let p00 = a0 * b0;
    let p01 = a0 * b1;
    let p10 = a1 * b0;
    let p11 = a1 * b1;
    let mid = ((p00 >> 32) + (p01 & m32)) + (p10 & m32);
    let hi = ((p11 + (p01 >> 32)) + (p10 >> 32)) + (mid >> 32);
    let lo = (p00 & m32) | (mid << 32);
    return U128Pair.{ lo: lo, hi: hi };
}

struct DivMod {
    q: I128,
    r: I128,
}

fn ult(a: I128, b: I128) bool {
    if a.hi != b.hi {
        return a.hi < b.hi;
    }
    return a.lo < b.lo;
}

// Unsigned 128-bit division (b != 0).
fn udivmod(a: I128, b: I128) DivMod {
    let mut q = I128.zero();
    let mut r = I128.zero();
    let mut i: u32 = 128;
    while i > 0 {
        i -= 1;
        r = r.shl(1);
        let mut w: u64 = 0;
        if i >= 64 {
            w = a.hi >> (i - 64);
        } else {
            w = a.lo >> i;
        }
        r.lo = r.lo | (w & 1);
        if not ult(r, b) {
            r = r.sub(b);
            if i >= 64 {
                q.hi = q.hi | (@as(u64, 1) << (i - 64));
            } else {
                q.lo = q.lo | (@as(u64, 1) << i);
            }
        }
    }
    return DivMod.{ q: q, r: r };
}

// -----------------------------------------------------------------------------
// Output. stdout is buffered (flushed by flush_out and quit); stderr is not.
// -----------------------------------------------------------------------------
let mut out_buf: List.[u8] = List.[u8].{ data: @ptr_from_int(*mut u8, 0), len: 0, cap: 0 };

pub fn print(s: []u8) void {
    for c in s {
        out_buf.push(c);
    }
    if out_buf.len > 65536 {
        flush_out();
    }
}

pub fn flush_out() void {
    write_all(1, out_buf.items());
    out_buf.len = 0;
}

pub fn eprint(s: []u8) void {
    write_all(2, s);
}

pub fn write_all(fd: i32, s: []u8) void {
    let mut off: usize = 0;
    while off < s.len {
        let n = write(fd, &s[off], s.len - off);
        if n <= 0 {
            return;
        }
        off += @int_cast(usize, n);
    }
}

pub fn quit(code: i32) void {
    flush_out();
    exit(code);
}

// -----------------------------------------------------------------------------
// Files
// -----------------------------------------------------------------------------
// The file's bytes (followed by a NUL), or a nil slice if it can't be read.
pub fn read_file(path: []u8) []u8 {
    let f = fopen(cz(path), "rb".ptr);
    if isvnil(f) {
        return nosym();
    }
    _ = fseek(f, 0, 2);
    let n = ftell(f);
    _ = fseek(f, 0, 0);
    if n < 0 {
        _ = fclose(f);
        return nosym();
    }
    let len = @int_cast(usize, n);
    let buf = alloc(len + 1);
    if len > 0 {
        if fread(buf, 1, len, f) != len {
            _ = fclose(f);
            return nosym();
        }
    }
    _ = fclose(f);
    return @slice_from_ptr(buf, len);
}

pub fn write_file(path: []u8, data: []u8) bool {
    let f = fopen(cz(path), "w".ptr);
    if isvnil(f) {
        return false;
    }
    if data.len > 0 {
        _ = fwrite(data.ptr, 1, data.len, f);
    }
    _ = fclose(f);
    return true;
}

pub fn file_exists(p: []u8) bool {
    return access(cz(p), 4) == 0;
}

pub fn is_dir(p: []u8) bool {
    let d = opendir(cz(p));
    if isvnil(d) {
        return false;
    }
    _ = closedir(d);
    return true;
}

pub fn is_file(p: []u8) bool {
    return (access(cz(p), 0) == 0) and not is_dir(p);
}

pub fn file_size(p: []u8) i64 {
    let f = fopen(cz(p), "rb".ptr);
    if isvnil(f) {
        return -1;
    }
    _ = fseek(f, 0, 2);
    let n = ftell(f);
    _ = fclose(f);
    return @as(i64, n);
}

// realpath(3); a nil slice on failure.
pub fn real_path(p: []u8) []u8 {
    let r = realpath(cz(p), nil.[u8]());
    if isnil(r) {
        return nosym();
    }
    let s = dup(from_c(r));
    free(@ptr_cast(*mut void, r));
    return s;
}

pub fn cwd() []u8 {
    let mut buf: [4096]u8 = [0; 4096];
    if isnil(getcwd(&mut buf[0], 4096)) {
        return "";
    }
    return dup(from_c(&buf[0]));
}

pub fn env(name: []u8) []u8 {
    let v = getenv(cz(name));
    if isnil(v) {
        return nosym();
    }
    return from_c(v);
}

pub fn mkdirs(path: []u8) void {
    let buf = dup(path);
    for i in 1..buf.len {
        if buf[i] == '/' {
            _ = mkdir(cz(buf[0..i]), 493);
        }
    }
    _ = mkdir(cz(buf), 493);
}

// Directory entries (names), excluding none; order as readdir returns them.
pub fn list_dir(path: []u8, out: *mut List.[[]u8]) bool {
    let d = opendir(cz(path));
    if isvnil(d) {
        return false;
    }
    loop {
        let e = readdir(d);
        if isnil(e) {
            break;
        }
        // struct dirent: d_name is at offset 19 on 64-bit Linux (glibc).
        out.push(dup(from_c(@ptr_from_int(*u8, @int_from_ptr(e) + 19))));
    }
    _ = closedir(d);
    return true;
}

// -----------------------------------------------------------------------------
// Processes
// -----------------------------------------------------------------------------
pub fn wifexited(s: i32) bool {
    return (s & 127) == 0;
}

pub fn wexitstatus(s: i32) i32 {
    return (s >> 8) & 255;
}

pub fn wtermsig(s: i32) i32 {
    return s & 127;
}

// A NULL-terminated argv array for exec.
pub fn c_argv(args: [][]u8) *mut *mut u8 {
    let v = newn.[*mut u8](args.len + 1);
    for i in 0..args.len {
        v[i] = @ptr_from_int(*mut u8, @int_from_ptr(cz(args[i])));
    }
    v[args.len] = nil.[u8]();
    return v.ptr;
}

// -----------------------------------------------------------------------------
// Source files and spans
// -----------------------------------------------------------------------------
pub struct SourceFile {
    path: []u8,
    display: []u8,
    text: []u8,
    id: i32,
    line_starts: List.[u32],
    nlines: usize,
}

pub struct Span {
    file: i32,
    start: u32,
    end: u32,
}

pub struct LineCol {
    sl: i32,
    sc: i32,
    el: i32,
    ec: i32,
}

pub let mut g_files: List.[*mut SourceFile] = List.[*mut SourceFile].{
    data: @ptr_from_int(*mut *mut SourceFile, 0),
    len: 0,
    cap: 0,
};

pub fn file_of(id: i32) *mut SourceFile {
    return g_files.get(@int_cast(usize, id));
}

pub fn mkspan(file: i32, a: u32, b: u32) Span {
    return Span.{ file: file, start: a, end: b };
}

pub fn load_file(path: []u8, display: []u8) *mut SourceFile {
    let text = read_file(path);
    if isnos(text) {
        return nil.[SourceFile]();
    }
    let sf = new.[SourceFile]();
    sf.path = dup(path);
    if isnos(display) {
        sf.display = sf.path;
    } else {
        sf.display = display;
    }
    sf.text = text;
    sf.id = @int_cast(i32, g_files.len);
    g_files.push(sf);
    return sf;
}

struct LC {
    line: i32,
    col: i32,
}

fn offset_linecol(f: *mut SourceFile, off0: u32) LC {
    if f.line_starts.len == 0 {
        f.line_starts.push(0);
        for i in 0..f.text.len {
            if f.text[i] == '\n' {
                f.line_starts.push(@int_cast(u32, i) + 1);
            }
        }
        f.nlines = f.line_starts.len;
    }
    let mut off = off0;
    if @as(usize, off) > f.text.len {
        off = @int_cast(u32, f.text.len);
    }
    let ls = f.line_starts.items();
    let mut lo: usize = 0;
    let mut hi = f.nlines - 1;
    while lo < hi {
        let mid = (lo + hi + 1) / 2;
        if ls[mid] <= off {
            lo = mid;
        } else {
            hi = mid - 1;
        }
    }
    let mut c: i32 = 1;
    let mut i = ls[lo];
    while i < off {
        if (f.text[@as(usize, i)] & 0xC0) != 0x80 {
            c += 1;
        }
        i += 1;
    }
    return LC.{ line: @int_cast(i32, lo) + 1, col: c };
}

pub fn span_linecol(s: Span) LineCol {
    let f = file_of(s.file);
    let a = offset_linecol(f, s.start);
    let b = offset_linecol(f, s.end);
    return LineCol.{ sl: a.line, sc: a.col, el: b.line, ec: b.col };
}

pub fn span_join(a: Span, b: Span) Span {
    let mut r = a;
    if b.file == a.file {
        if b.start < r.start {
            r.start = b.start;
        }
        if b.end > r.end {
            r.end = b.end;
        }
    }
    return r;
}

// The source text of a span.
pub fn span_text(sp: Span) []u8 {
    let f = file_of(sp.file);
    return dup(f.text[@as(usize, sp.start)..@as(usize, sp.end)]);
}

// -----------------------------------------------------------------------------
// Tests (phalange test selfhost/src/util.ph --target=native)
// -----------------------------------------------------------------------------
fn neg_cmp(a: i64, b: i64) i32 {
    if a < b {
        return -1;
    }
    if a > b {
        return 1;
    }
    return 0;
}

test "i128" {
    let a = I128.of(-7);
    @expect_eq(a.str(), "-7");
    @expect_eq(a.div(I128.of(2)).str(), "-3");
    @expect_eq(a.rem(I128.of(2)).str(), "-1");
    @expect_eq(a.sar(1).str(), "-4");
    let big = I128.pow2(100);
    @expect_eq(big.str(), "1267650600228229401496703205376");
    @expect_eq(big.mul(I128.of(-3)).div(I128.of(3)).str(), "-1267650600228229401496703205376");
    @expect(big.to_f64() == 1267650600228229401496703205376.0);
    @expect_eq(I128.ofu(0xFFFF_FFFF_FFFF_FFFF).add(I128.ofu(1)).str(), "18446744073709551616");
    @expect(I128.of(-1).lt(I128.of(0)));
    @expect_eq(I128.of(5).sub(I128.of(9)).i64v(), -4);
}

test "format and parse" {
    @expect_eq(f2("a %s b %s%%", "x", dec(-42)), "a x b -42%");
    @expect_eq(hex16(255), "00000000000000ff");
    @expect_eq(c_strtoll("0x1F").v, 31);
    @expect_eq(c_strtoll("010").v, 8);
    @expect_eq(c_strtoll("-5").v, -5);
    @expect(c_strtoll("99999999999999999999").ovf);
    @expect_eq(c_strtoull("18446744073709551615").v, -1);
    @expect_eq(ffmt("%.1f", 2.25), "2.2");
    let mut xs: [5]i64 = [3, 1, 2, 5, 4];
    sort(xs[..], neg_cmp);
    @expect_eq(xs[0], 1);
    @expect_eq(xs[4], 5);
    @expect(same(intern("abc"), intern(dup("abc"))));
    let mut m = SymMap.[i32].{ keys: nil.[usize](), vals: nil.[i32](), cap: 0, len: 0 };
    m.put(intern("k"), 7);
    @expect_eq(m.get(intern("k")), 7);
    @expect_eq(m.get(intern("q")), 0);
}
