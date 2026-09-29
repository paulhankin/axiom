// =============================================================================
// std/unicode/linebreak.ph - where text may be broken into lines: the Unicode
// Line Breaking Algorithm (UAX #14), Unicode 15.1.
//
//     let mut b = linebreak.Breaks.init(text)!;
//     defer b.deinit();
//     loop {
//         let at = b.next() catch break;   // a break opportunity
//         // text[..at.pos] may end a line; at.mandatory: it must (a newline)
//     }
//
// The last opportunity is always the end of the text. Opportunities come
// in order; the text is not copied.
// =============================================================================

import "../mem.ph" as mem;
import "../str.ph" as strs;
import "../unicode.ph" as uni;
import "../vec.ph" as vec;
import "tables.ph" as t;

error { OutOfMemory, Done }

pub struct Break {
    pos: usize,       // byte offset: a line may end here
    mandatory: bool,  // it must (after a line or paragraph separator)
}

/// The break opportunities of a text.
pub struct Breaks {
    s: []u8,
    starts: vec.Vec.[usize],  // each code point's byte offset (and the end)
    raw: vec.Vec.[u8],        // classes after LB1
    eff: vec.Vec.[u8],        // after LB9 and LB10 (combining marks take their base's)
    base: vec.Vec.[usize],    // each character's base (itself unless it is a combining mark)
    cps: vec.Vec.[u32],
    k: usize,  // the next boundary to test (between k - 1 and k)

    pub fn init(s: []u8) !Breaks {
        let mut b = Breaks.{
            s: s,
            starts: vec.Vec.[usize].init(),
            raw: vec.Vec.[u8].init(),
            eff: vec.Vec.[u8].init(),
            base: vec.Vec.[usize].init(),
            cps: vec.Vec.[u32].init(),
            k: 1,
        };
        b.classify() catch |e| {
            b.deinit();
            return e;
        };
        return b;
    }

    pub fn deinit(self: *mut Breaks) void {
        self.starts.deinit();
        self.raw.deinit();
        self.eff.deinit();
        self.base.deinit();
        self.cps.deinit();
    }

    fn classify(self: *mut Breaks) !void {
        let s = self.s;
        let mut i: usize = 0;
        while i < s.len {
            let r = strs.decode_rune(s[i..]);
            self.starts.push(i)!;
            self.cps.push(r.value)!;
            let mut c = uni.line_break_class(r.value);
            // LB1
            if (c == t.LB_AI) or (c == t.LB_SG) or (c == t.LB_XX) {
                c = t.LB_AL;
            } else if c == t.LB_SA {
                let gc = uni.category(r.value);
                if (gc == uni.GC_MN) or (gc == uni.GC_MC) {
                    c = t.LB_CM;
                } else {
                    c = t.LB_AL;
                }
            } else if c == t.LB_CJ {
                c = t.LB_NS;
            }
            self.raw.push(@truncate(u8, c))!;
            i += r.len;
        }
        self.starts.push(s.len)!;
        // LB9 / LB10
        let n = self.raw.len;
        for k in @as(usize, 0)..n {
            let c = @as(u32, self.raw.get(k));
            if ((c == t.LB_CM) or (c == t.LB_ZWJ)) and (k > 0) {
                let pb = self.base.get(k - 1);
                let p = @as(u32, self.raw.get(k - 1));
                let pbase_raw = @as(u32, self.raw.get(pb));
                if not ((p == t.LB_BK) or (p == t.LB_CR) or (p == t.LB_LF) or (p == t.LB_NL) or (p == t.LB_SP) or
                        (p == t.LB_ZW)) and
                   not ((pbase_raw == t.LB_BK) or (pbase_raw == t.LB_CR) or (pbase_raw == t.LB_LF) or
                        (pbase_raw == t.LB_NL) or (pbase_raw == t.LB_SP) or (pbase_raw == t.LB_ZW)) {
                    self.base.push(pb)!;
                    self.eff.push(self.eff.get(pb))!;
                    continue;
                }
            }
            self.base.push(k)!;
            if (c == t.LB_CM) or (c == t.LB_ZWJ) {
                self.eff.push(@truncate(u8, t.LB_AL))!;  // LB10
            } else {
                self.eff.push(@truncate(u8, c))!;
            }
        }
    }

    /// The next break opportunity, or error.Done.
    pub fn next(self: *mut Breaks) !Break {
        let n = self.raw.len;
        if self.k > n {
            return error.Done;
        }
        while self.k < n {
            let k = self.k;
            self.k += 1;
            let d = self.decide(k);
            if d != NO {
                return Break.{ pos: self.starts.get(k), mandatory: d == MUST };
            }
        }
        self.k = n + 1;
        if n == 0 {
            return error.Done;
        }
        return Break.{ pos: self.s.len, mandatory: true };  // LB3
    }

    fn rc(self: *Breaks, k: usize) u32 {
        return @as(u32, self.raw.get(k));
    }

    fn ec(self: *Breaks, k: usize) u32 {
        return @as(u32, self.eff.get(k));
    }

    /// The base of the character before the one based at index `at`, or
    /// none (-1) at the start.
    fn prev_base(self: *Breaks, at: usize) i64 {
        let b = self.base.get(at);
        if b == 0 {
            return -1;
        }
        return @as(i64, @truncate(u32, self.base.get(b - 1)));
    }

    /// The effective class of the character after index k's combining
    /// sequence (-1 at the end).
    fn next_class(self: *Breaks, k: usize) i64 {
        let mut j = k + 1;
        while (j < self.raw.len) and (self.base.get(j) == self.base.get(k)) {
            j += 1;
        }
        if j >= self.raw.len {
            return -1;
        }
        return @as(i64, self.ec(j));
    }

    fn next_index(self: *Breaks, k: usize) i64 {
        let mut j = k + 1;
        while (j < self.raw.len) and (self.base.get(j) == self.base.get(k)) {
            j += 1;
        }
        if j >= self.raw.len {
            return -1;
        }
        return @as(i64, @truncate(u32, j));
    }

    fn is_ea(self: *Breaks, k: usize) bool {
        let ea = uni.east_asian_width(self.cps.get(k));
        return (ea == uni.EA_F) or (ea == uni.EA_W) or (ea == uni.EA_H);
    }

    fn decide(self: *Breaks, k: usize) u8 {
        let a = self.rc(k - 1);
        let b = self.rc(k);
        // LB4, LB5
        if a == t.LB_BK {
            return MUST;
        }
        if (a == t.LB_CR) and (b == t.LB_LF) {
            return NO;
        }
        if (a == t.LB_CR) or (a == t.LB_LF) or (a == t.LB_NL) {
            return MUST;
        }
        // LB6, LB7
        if (b == t.LB_BK) or (b == t.LB_CR) or (b == t.LB_LF) or (b == t.LB_NL) or (b == t.LB_SP) or (b == t.LB_ZW) {
            return NO;
        }
        // LB8: ZW SP* ÷
        let mut j = k;
        while (j > 0) and (self.rc(j - 1) == t.LB_SP) {
            j -= 1;
        }
        if (j > 0) and (self.rc(j - 1) == t.LB_ZW) {
            return MAY;
        }
        // LB8a
        if a == t.LB_ZWJ {
            return NO;
        }
        // LB9
        if ((b == t.LB_CM) or (b == t.LB_ZWJ)) and (self.base.get(k) != k) {
            return NO;
        }
        let pa = self.base.get(k - 1);  // the character before (its base)
        let ea = self.ec(k - 1);
        let eb = self.ec(k);
        // the class before any spaces (for the SP* rules), and its index
        let mut xs = k;
        while (xs > 0) and (self.rc(xs - 1) == t.LB_SP) {
            xs -= 1;
        }
        let mut x: u32 = 0xff;
        let mut xi: usize = 0;
        if xs > 0 {
            xi = self.base.get(xs - 1);
            x = self.ec(xi);
        }
        // LB11
        if (eb == t.LB_WJ) or (ea == t.LB_WJ) {
            return NO;
        }
        // LB12, LB12a
        if ea == t.LB_GL {
            return NO;
        }
        if (eb == t.LB_GL) and (ea != t.LB_SP) and (ea != t.LB_BA) and (ea != t.LB_HY) {
            return NO;
        }
        // LB13
        if (eb == t.LB_CL) or (eb == t.LB_CP) or (eb == t.LB_EX) or (eb == t.LB_SY) {
            return NO;
        }
        // LB14
        if x == t.LB_OP {
            return NO;
        }
        // LB15a: (sot | BK | CR | LF | NL | OP | QU | GL | SP | ZW) [Pi & QU] SP* ×
        if (x == t.LB_QU) and (uni.category(self.cps.get(xi)) == uni.GC_PI) {
            let before = self.prev_base(xi);
            if before < 0 {
                return NO;
            }
            let bc = self.ec(@truncate(usize, @bit_cast(u64, before)));
            if (bc == t.LB_BK) or (bc == t.LB_CR) or (bc == t.LB_LF) or (bc == t.LB_NL) or (bc == t.LB_OP) or
               (bc == t.LB_QU) or (bc == t.LB_GL) or (bc == t.LB_SP) or (bc == t.LB_ZW) {
                return NO;
            }
        }
        // LB15b: × [Pf & QU] (SP | GL | WJ | CL | QU | CP | EX | IS | SY | BK | CR | LF | NL | ZW | eot)
        if (eb == t.LB_QU) and (uni.category(self.cps.get(k)) == uni.GC_PF) {
            let nc = self.next_class(k);
            if nc < 0 {
                return NO;
            }
            let c = @truncate(u32, @bit_cast(u64, nc));
            if (c == t.LB_SP) or (c == t.LB_GL) or (c == t.LB_WJ) or (c == t.LB_CL) or (c == t.LB_QU) or
               (c == t.LB_CP) or (c == t.LB_EX) or (c == t.LB_IS) or (c == t.LB_SY) or (c == t.LB_BK) or
               (c == t.LB_CR) or (c == t.LB_LF) or (c == t.LB_NL) or (c == t.LB_ZW) {
                return NO;
            }
        }
        // LB13 (continued): × IS
        if eb == t.LB_IS {
            return NO;
        }
        // LB16, LB17
        if ((x == t.LB_CL) or (x == t.LB_CP)) and (eb == t.LB_NS) {
            return NO;
        }
        if (x == t.LB_B2) and (eb == t.LB_B2) {
            return NO;
        }
        // LB18
        if a == t.LB_SP {
            return MAY;
        }
        // LB19
        if (eb == t.LB_QU) and (uni.category(self.cps.get(k)) != uni.GC_PI) {
            return NO;
        }
        if (ea == t.LB_QU) and (uni.category(self.cps.get(pa)) != uni.GC_PF) {
            return NO;
        }
        // LB19a
        if eb == t.LB_QU {
            if not self.is_ea(pa) {
                return NO;
            }
            let ni = self.next_index(k);
            if (ni < 0) or not self.is_ea(@truncate(usize, @bit_cast(u64, ni))) {
                return NO;
            }
        }
        if ea == t.LB_QU {
            if not self.is_ea(k) {
                return NO;
            }
            let before = self.prev_base(pa);
            if (before < 0) or not self.is_ea(@truncate(usize, @bit_cast(u64, before))) {
                return NO;
            }
        }
        // LB20
        if (eb == t.LB_CB) or (ea == t.LB_CB) {
            return MAY;
        }
        // LB21
        if (eb == t.LB_BA) or (eb == t.LB_HY) or (eb == t.LB_NS) or (ea == t.LB_BB) {
            return NO;
        }
        // LB21a: HL (HY | BA) ×
        if (ea == t.LB_HY) or (ea == t.LB_BA) {
            let before = self.prev_base(pa);
            if (before >= 0) and (self.ec(@truncate(usize, @bit_cast(u64, before))) == t.LB_HL) {
                return NO;
            }
        }
        // LB21b
        if (ea == t.LB_SY) and (eb == t.LB_HL) {
            return NO;
        }
        // LB22
        if eb == t.LB_IN {
            return NO;
        }
        // LB23, LB23a
        if (((ea == t.LB_AL) or (ea == t.LB_HL)) and (eb == t.LB_NU)) or
           ((ea == t.LB_NU) and ((eb == t.LB_AL) or (eb == t.LB_HL))) {
            return NO;
        }
        if (ea == t.LB_PR) and ((eb == t.LB_ID) or (eb == t.LB_EB) or (eb == t.LB_EM)) {
            return NO;
        }
        if ((ea == t.LB_ID) or (ea == t.LB_EB) or (ea == t.LB_EM)) and (eb == t.LB_PO) {
            return NO;
        }
        // LB24
        if ((ea == t.LB_PR) or (ea == t.LB_PO)) and ((eb == t.LB_AL) or (eb == t.LB_HL)) {
            return NO;
        }
        if ((ea == t.LB_AL) or (ea == t.LB_HL)) and ((eb == t.LB_PR) or (eb == t.LB_PO)) {
            return NO;
        }
        // LB25
        if (ea == t.LB_PR) or (ea == t.LB_PO) {
            if eb == t.LB_NU {
                return NO;
            }
            if ((eb == t.LB_OP) or (eb == t.LB_HY)) and (self.next_class(k) == @as(i64, t.LB_NU)) {
                return NO;
            }
        }
        if ((ea == t.LB_OP) or (ea == t.LB_HY)) and (eb == t.LB_NU) {
            return NO;
        }
        if (eb == t.LB_NU) or (eb == t.LB_SY) or (eb == t.LB_IS) or (eb == t.LB_CL) or (eb == t.LB_CP) or
           (eb == t.LB_PO) or (eb == t.LB_PR) {
            // NU (NU | SY | IS)* [(CL | CP)] × ...
            let mut p = @as(i64, @truncate(u32, pa));
            let mut closed = false;
            let pc = self.ec(pa);
            if (pc == t.LB_CL) or (pc == t.LB_CP) {
                if (eb == t.LB_PO) or (eb == t.LB_PR) {
                    closed = true;
                    p = self.prev_base(pa);
                } else {
                    p = -1;
                }
            }
            let mut found = false;
            while p >= 0 {
                let c = self.ec(@truncate(usize, @bit_cast(u64, p)));
                if c == t.LB_NU {
                    found = true;
                }
                if (c == t.LB_NU) or (c == t.LB_SY) or (c == t.LB_IS) {
                    // a NU found here ends the search only if nothing but these precede
                    let q = self.prev_base(@truncate(usize, @bit_cast(u64, p)));
                    if c == t.LB_NU {
                        break;
                    }
                    p = q;
                    continue;
                }
                break;
            }
            if found {
                if closed {
                    return NO;
                }
                if (eb == t.LB_NU) or (eb == t.LB_SY) or (eb == t.LB_IS) or (eb == t.LB_CL) or (eb == t.LB_CP) or
                   (eb == t.LB_PO) or (eb == t.LB_PR) {
                    return NO;
                }
            }
        }
        // LB26, LB27
        if (ea == t.LB_JL) and ((eb == t.LB_JL) or (eb == t.LB_JV) or (eb == t.LB_H2) or (eb == t.LB_H3)) {
            return NO;
        }
        if ((ea == t.LB_JV) or (ea == t.LB_H2)) and ((eb == t.LB_JV) or (eb == t.LB_JT)) {
            return NO;
        }
        if ((ea == t.LB_JT) or (ea == t.LB_H3)) and (eb == t.LB_JT) {
            return NO;
        }
        if korean(ea) and (eb == t.LB_PO) {
            return NO;
        }
        if (ea == t.LB_PR) and korean(eb) {
            return NO;
        }
        // LB28
        if ((ea == t.LB_AL) or (ea == t.LB_HL)) and ((eb == t.LB_AL) or (eb == t.LB_HL)) {
            return NO;
        }
        // LB28a (Brahmic orthographic syllables; U+25CC is the dotted circle)
        let a_ak = (ea == t.LB_AK) or (ea == t.LB_AS) or (self.cps.get(pa) == 0x25cc);
        let b_ak = (eb == t.LB_AK) or (eb == t.LB_AS) or (self.cps.get(k) == 0x25cc);
        if (ea == t.LB_AP) and b_ak {
            return NO;
        }
        if a_ak and ((eb == t.LB_VF) or (eb == t.LB_VI)) {
            return NO;
        }
        if (ea == t.LB_VI) and ((eb == t.LB_AK) or (self.cps.get(k) == 0x25cc)) {
            let before = self.prev_base(pa);
            if before >= 0 {
                let bi = @truncate(usize, @bit_cast(u64, before));
                let bc = self.ec(bi);
                if (bc == t.LB_AK) or (bc == t.LB_AS) or (self.cps.get(bi) == 0x25cc) {
                    return NO;
                }
            }
        }
        if a_ak and b_ak and (self.next_class(k) == @as(i64, t.LB_VF)) {
            return NO;
        }
        // LB29
        if (ea == t.LB_IS) and ((eb == t.LB_AL) or (eb == t.LB_HL)) {
            return NO;
        }
        // LB30
        if ((ea == t.LB_AL) or (ea == t.LB_HL) or (ea == t.LB_NU)) and (eb == t.LB_OP) and not self.is_ea(k) {
            return NO;
        }
        if (ea == t.LB_CP) and not self.is_ea(pa) and ((eb == t.LB_AL) or (eb == t.LB_HL) or (eb == t.LB_NU)) {
            return NO;
        }
        // LB30a: pairs of regional indicators
        if (ea == t.LB_RI) and (eb == t.LB_RI) {
            let mut count: usize = 0;
            let mut p = @as(i64, @truncate(u32, pa));
            while (p >= 0) and (self.ec(@truncate(usize, @bit_cast(u64, p))) == t.LB_RI) {
                count += 1;
                p = self.prev_base(@truncate(usize, @bit_cast(u64, p)));
            }
            if (count % 2) == 1 {
                return NO;
            }
        }
        // LB30b
        if eb == t.LB_EM {
            if ea == t.LB_EB {
                return NO;
            }
            let c = self.cps.get(pa);
            if uni.is_extended_pictographic(c) and (uni.category(c) == uni.GC_CN) {
                return NO;
            }
        }
        return MAY;  // LB31
    }
}

const NO: u8 = 0;
const MAY: u8 = 1;
const MUST: u8 = 2;

fn korean(c: u32) bool {
    return (c == t.LB_JL) or (c == t.LB_JV) or (c == t.LB_JT) or (c == t.LB_H2) or (c == t.LB_H3);
}

// -----------------------------------------------------------------------------
// Tests
// -----------------------------------------------------------------------------

import "../io.ph" as io;

fn parse_hex(s: []u8) u32 {
    let mut v: u32 = 0;
    for c in s {
        let mut d: u32 = 0;
        if (c >= '0') and (c <= '9') {
            d = @as(u32, c - '0');
        } else if (c >= 'A') and (c <= 'F') {
            d = @as(u32, c - 'A') + 10;
        } else if (c >= 'a') and (c <= 'f') {
            d = @as(u32, c - 'a') + 10;
        }
        v = (v * 16) + d;
    }
    return v;
}

/// Runs LineBreakTest.txt: [cases, failures] ([0, 0] if it is missing).
fn run_conformance(file: []u8) ![2]usize {
    let f = io.open(file) catch return [@as(usize, 0), 0];
    let data = f.reader().read_all() catch |e| {
        f.close();
        return e;
    };
    f.close();
    defer mem.free(data);
    let mut cases: usize = 0;
    let mut failures: usize = 0;
    let mut lines = strs.lines(data);
    loop {
        let raw = lines.next() catch break;
        let hash = strs.index_of(raw, "#") catch raw.len;
        let line = strs.trim(raw[..hash]);
        if line.len == 0 {
            continue;
        }
        let mut text = strs.String.init();
        defer text.deinit();
        let mut want = vec.Vec.[usize].init();
        defer want.deinit();
        let mut fs = strs.fields(line);
        let mut first = true;
        loop {
            let tok = fs.next() catch break;
            if @mem_eq(tok, "\xc3\xb7") {
                if not first {
                    want.push(text.len)!;
                }
            } else if not @mem_eq(tok, "\xc3\x97") {
                let mut b: [4]u8 = [0; 4];
                text.push_str(strs.encode_rune(b[..], parse_hex(tok)))!;
            }
            first = false;
        }
        cases += 1;
        let mut br = Breaks.init(text.bytes())!;
        defer br.deinit();
        let mut got = vec.Vec.[usize].init();
        defer got.deinit();
        loop {
            let x = br.next() catch break;
            got.push(x.pos)!;
        }
        let mut same = got.len == want.len;
        if same {
            for k in @as(usize, 0)..got.len {
                if got.get(k) != want.get(k) {
                    same = false;
                }
            }
        }
        if not same {
            failures += 1;
        }
    }
    return [cases, failures];
}

test "conformance (LineBreakTest.txt, when installed)" {
    let r = run_conformance("/usr/share/unicode/auxiliary/LineBreakTest.txt")!;
    @expect_eq(r[1], 0);
}

test "opportunities" {
    let mut b = Breaks.init("Hello, world. A line\nNext 3.14 (x)")!;
    defer b.deinit();
    let want: [7]usize = [7, 14, 16, 21, 26, 31, 34];
    let must: [7]bool = [false, false, false, true, false, false, true];
    for k in @as(usize, 0)..7 {
        let x = b.next()!;
        @expect_eq(x.pos, want[k]);
        @expect_eq(x.mandatory, must[k]);
    }
    _ = b.next() catch |e| {
        @expect_eq(e, error.Done);
        return;
    };
    @trap();
}
