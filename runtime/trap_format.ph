// =============================================================================
// runtime/trap_format.ph - trap records, shadow stacks and their formatting,
// shared by the native runtime modules (native.ph, test_main.ph,
// profile.ph). The records are written by the code generator (codegen:
// site_record, trap_info, the shadow stack).
// =============================================================================

pub extern "C" {
    fn snprintf(buf: *mut u8, n: usize, fmt: *u8, ...) i32;
    fn strlen(s: *u8) usize;
}

// A site record: a trap site, a call site (code 255) or an error return
// (code 254). `fn_` is the enclosing function, `callee` the called one.
pub struct Site {
    code: u32,
    file_id: u32,
    start: u32,
    end: u32,
    line: u32,
    col: u32,
    file: *u8,
    text: *u8,
    fn_: *u8,
    callee: *u8,
}

pub const SS_CAP: u32 = 256;
pub const ET_CAP: u32 = 32;

// A thread's shadow stack (spec §16.4).
pub struct ShadowStack {
    depth: u32,
    errors: u32,
    calls: [256]*Site,
    error_returns: [32]*Site,
}

pub struct TrapInfo {
    site: u64,
    kind: u64,
    a: u64,
    b: u64,
    alen: u64,
    blen: u64,
}

pub fn is_null(p: *void) bool {
    return @int_from_ptr(p) == 0;
}

pub fn cstr(p: *u8) []u8 {
    if @int_from_ptr(p) == 0 {
        return "";
    }
    return @slice_from_ptr(p, strlen(p));
}

pub fn call_at(ss: *ShadowStack, i: u32) *Site {
    return ss.calls[@as(usize, i & (SS_CAP - 1))];
}

pub fn trap_name(code: u32) []u8 {
    let names = ["trap", "index out of bounds", "integer overflow", "division by zero", "int_cast out of range",
                 "memory access out of bounds", "length precondition failed", "invalid union tag", "stack exhausted",
                 "invalid function pointer", "misaligned atomic access", "expectation failed"];
    if code < @int_cast(u32, names.len) {
        return names[@as(usize, code)];
    }
    return "trap";
}

// A fixed-capacity text buffer (no allocation: usable in signal handlers).
pub struct Fmt {
    p: *mut u8,
    cap: usize,
    n: usize,

    pub fn s(self: *mut Fmt, x: []u8) void {
        let d = @slice_from_ptr(self.p, self.cap);
        for c in x {
            if (self.n + 1) >= self.cap {
                return;
            }
            d[self.n] = c;
            self.n += 1;
        }
        d[self.n] = 0;
    }

    pub fn c(self: *mut Fmt, x: u8) void {
        let one: [1]u8 = [x];
        self.s(one[..]);
    }

    pub fn u(self: *mut Fmt, x: u64) void {
        let mut tmp: [24]u8 = [0; 24];
        let mut k: usize = 24;
        let mut v = x;
        loop {
            k -= 1;
            tmp[k] = @truncate(u8, v % 10) + '0';
            v = v / 10;
            if v == 0 {
                break;
            }
        }
        self.s(tmp[k..]);
    }

    pub fn i(self: *mut Fmt, x: i64) void {
        if x < 0 {
            self.c('-');
            self.u(0 -% @bit_cast(u64, x));
            return;
        }
        self.u(@bit_cast(u64, x));
    }

    pub fn hex(self: *mut Fmt, x: u64) void {
        let digits = "0123456789abcdef";
        let mut tmp: [16]u8 = [0; 16];
        let mut k: usize = 16;
        let mut v = x;
        loop {
            k -= 1;
            tmp[k] = digits[@truncate(usize, v & 15)];
            v = v >> 4;
            if v == 0 {
                break;
            }
        }
        self.s(tmp[k..]);
    }

    pub fn g17(self: *mut Fmt, x: f64) void {
        let mut tmp: [40]u8 = [0; 40];
        let k = snprintf(&mut tmp[0], 40, "%.17g".ptr, x);
        if k > 0 {
            self.s(tmp[0..@int_cast(usize, k)]);
        }
    }

    // "T0003"
    pub fn code(self: *mut Fmt, c: u32) void {
        self.c('T');
        if c < 1000 {
            self.c('0');
        }
        if c < 100 {
            self.c('0');
        }
        if c < 10 {
            self.c('0');
        }
        self.u(@as(u64, c));
    }

    pub fn text(self: *Fmt) []u8 {
        return @slice_from_ptr(self.p, self.n);
    }
}

pub fn fmt(buf: []mut u8) Fmt {
    if buf.len > 0 {
        buf[0] = 0;
    }
    return Fmt.{ p: buf.ptr, cap: buf.len, n: 0 };
}

pub fn format_value(o: *mut Fmt, kind: u64, v: u64, len: u64) void {
    if kind == 1 {
        o.i(@bit_cast(i64, v));
    } else if kind == 2 {
        o.u(v);
    } else if kind == 3 {
        o.g17(@bit_cast(f64, v));
    } else if kind == 4 {
        if v != 0 {
            o.s("true");
        } else {
            o.s("false");
        }
    } else if kind == 5 {
        let mut n = len;
        if n > 200 {
            n = 200;
        }
        o.c('"');
        if v != 0 {
            o.s(@slice_from_ptr(@ptr_from_int(*u8, @truncate(usize, v)), @truncate(usize, n)));
        }
        o.c('"');
    } else if kind == 6 {
        o.s("error #");
        o.u(v);
    } else {
        o.s("0x");
        o.hex(v);
    }
}

// "trap T0001 (index out of bounds): <text>" or
// "expected equal values, left: 3, right: 4: <text>"
pub fn format_trap(o: *mut Fmt, s: *Site, info: *TrapInfo) void {
    if not is_null(@ptr_cast(*void, info)) and (info.kind != 0) {
        o.s("expected equal values, left: ");
        format_value(o, info.kind, info.a, info.alen);
        o.s(", right: ");
        format_value(o, info.kind, info.b, info.blen);
        o.s(": ");
        o.s(cstr(s.text));
    } else if s.code == 11 {
        o.s("expectation failed: ");
        o.s(cstr(s.text));
    } else {
        o.s("trap ");
        o.code(s.code);
        o.s(" (");
        o.s(trap_name(s.code));
        o.s("): ");
        o.s(cstr(s.text));
    }
}

fn frame_line(o: *mut Fmt, c: *Site) void {
    o.s("  at ");
    o.s(cstr(c.fn_));
    o.s(" (");
    o.s(cstr(c.file));
    o.c(':');
    o.u(@as(u64, c.line));
    o.c(':');
    o.u(@as(u64, c.col));
    o.s(")\n");
}

// "stack trace" lines for a trap at `top` (0: location unknown).
pub fn format_stack(o: *mut Fmt, top: *Site, ss: *ShadowStack) void {
    let has_ss = not is_null(@ptr_cast(*void, ss));
    let mut depth: u32 = 0;
    if has_ss {
        depth = ss.depth;
    }
    let has_top = not is_null(@ptr_cast(*void, top));
    if not has_top and (depth == 0) {
        return;
    }
    o.s("stack trace (innermost first):\n");
    if has_top {
        frame_line(o, top);
    } else {
        let c = call_at(ss, depth - 1);
        o.s("  at ");
        if not is_null(@ptr_cast(*void, c)) and (cstr(c.callee).len > 0) {
            o.s(cstr(c.callee));
        } else {
            o.s("<indirect call>");
        }
        o.s("\n");
    }
    let mut shown: u32 = 0;
    let mut i = @as(i64, depth) - 1;
    while (i >= 0) and (i >= (@as(i64, depth) - @as(i64, SS_CAP))) {
        let c = call_at(ss, @truncate(u32, @bit_cast(u64, i)));
        if is_null(@ptr_cast(*void, c)) {
            i -= 1;
            continue;
        }
        if shown >= 40 {
            o.s("  ... ");
            o.u(@bit_cast(u64, i + 1));
            o.s(" more frames\n");
            return;
        }
        let mut j = i;
        while ((j - 1) >= 0) and ((j - 1) >= (@as(i64, depth) - @as(i64, SS_CAP))) and
              (call_at(ss, @truncate(u32, @bit_cast(u64, j - 1))) == c) {
            j -= 1;
        }
        frame_line(o, c);
        shown += 1;
        if j < i {
            o.s("  ... the frame above repeats ");
            o.u(@bit_cast(u64, i - j));
            if (i - j) == 1 {
                o.s(" more time\n");
            } else {
                o.s(" more times\n");
            }
            i = j;
        }
        i -= 1;
    }
    if depth > SS_CAP {
        o.s("  ... ");
        o.u(@as(u64, depth - SS_CAP));
        o.s(" more frames\n");
    }
}

pub fn format_error_trace(o: *mut Fmt, ss: *ShadowStack) void {
    if is_null(@ptr_cast(*void, ss)) or (ss.errors == 0) {
        return;
    }
    o.s("error return trace (oldest first):\n");
    let mut first: u32 = 0;
    if ss.errors > ET_CAP {
        first = ss.errors - ET_CAP;
    }
    for i in first..ss.errors {
        let e = ss.error_returns[@as(usize, i & (ET_CAP - 1))];
        if is_null(@ptr_cast(*void, e)) {
            continue;
        }
        o.s("  ");
        o.s(cstr(e.file));
        o.c(':');
        o.u(@as(u64, e.line));
        o.c(':');
        o.u(@as(u64, e.col));
        o.s(" in ");
        o.s(cstr(e.fn_));
        o.s(": ");
        o.s(cstr(e.text));
        o.s("\n");
    }
}
