// =============================================================================
// std/time/tz.ph - time zones: the tz database, POSIX TZ rules and local time.
//
//     let mut z = tz.Zone.load("Europe/Paris")!;       // or Zone.local()
//     defer z.deinit();
//     let dt = z.to_local(time.now());                 // year, month, ... offset
//     let info = z.lookup(t.unix());                   // offset, is_dst, abbrev ("CEST")
//
//     // a local time to an instant: fold 0 or 1 chooses, as in Python
//     // (PEP 495), in a repeated hour and a skipped one
//     let t = z.from_local(time.DateTime.{ year: 2026, month: 10, day: 25, hour: 2, ... }, 0)!;
//
// Zones come from TZif files (RFC 8536, versions 1 to 4), read from the
// system's database (/usr/share/zoneinfo and the like, or the directory
// in $ZONEINFO) by load, or from bytes by parse_tzif; times beyond the
// file's last transition follow its POSIX TZ rule, as do zones made from a
// TZ string by parse_posix ("EST5EDT,M3.2.0,M11.1.0"). local follows $TZ
// (a zone name, a file or a rule) and then /etc/localtime, like the C
// library. Leap seconds are ignored, as they are by time.Time.
//
// Zone names are checked before they are used as paths ("../x" and
// absolute names are refused), and TZif files are validated, so neither
// can make load read outside the database or misbehave.
// =============================================================================

import "../io.ph" as io;
import "../mem.ph" as mem;
import "../os.ph" as os;
import "../os/fs.ph" as fs;
import "../str.ph" as strs;
import "../time.ph" as time;

error { UnknownZone, InvalidZone, NonExistent, Ambiguous, OutOfMemory }

/// What a zone says of an instant.
pub struct Info {
    offset: i32,   // seconds east of UTC
    is_dst: bool,  // daylight saving time
    abbrev: []u8,  // such as "EST" or "+0530" (owned by the Zone)
}

struct Kind {
    offset: i32,
    dst: bool,
    name: []u8,
}

// A day of the year in a POSIX rule: Jn, n or Mm.w.d, and a time of day.
struct When {
    kind: u8,  // 'J', 'n' or 'M'
    n: u32,    // J: 1-365; n: 0-365
    m: u32,
    w: u32,
    d: u32,
    time: i32,  // seconds after midnight, local time (-167 to 167 hours)
}

struct Rule {
    std: Kind,
    dst: Kind,
    has_dst: bool,
    start: When,  // when daylight saving time starts (in standard time)
    end: When,    // and ends (in daylight saving time)
}

pub struct Zone {
    trans: []mut i64,  // transition times, increasing
    idx: []mut u8,     // the kind of each transition
    kinds: []mut Kind,
    names: []mut u8,  // the bytes Kind.name and the rule's names view
    rule: Rule,
    has_rule: bool,

    /// UTC.
    pub fn utc() Zone {
        return Zone.fixed_static(0, "UTC");
    }

    // a fixed zone whose name is not owned
    fn fixed_static(offset: i32, name: []u8) Zone {
        let k = Kind.{ offset: offset, dst: false, name: name };
        let w = When.{ kind: 'n', n: 0, m: 0, w: 0, d: 0, time: 0 };
        return Zone.{
            trans: empty_i64(), idx: empty_u8(), kinds: empty_kinds(), names: empty_u8(), rule: Rule.{
                std: k, dst: k, has_dst: false, start: w, end: w }, has_rule: true };
    }

    /// A zone at a fixed offset (seconds east of UTC), named `name`.
    pub fn fixed(offset: i32, name: []u8) !Zone {
        let owned = mem.alloc.[u8](name.len)!;
        @mem_copy(owned, name);
        let mut z = Zone.fixed_static(offset, owned);
        z.names = owned;
        return z;
    }

    pub fn deinit(self: *mut Zone) void {
        mem.free(self.trans);
        mem.free(self.idx);
        mem.free(self.kinds);
        mem.free(self.names);
    }

    /// The zone of that name in the system's database ("America/New_York",
    /// "UTC"). UnknownZone if there is none; InvalidZone if its file is bad.
    pub fn load(name: []u8) !Zone {
        if not valid_name(name) {
            return error.UnknownZone;
        }
        // $ZONEINFO first, then the usual places
        let env = os.getenv("ZONEINFO") catch mem.alloc.[u8](0)!;
        defer mem.free(env);
        let dirs: [5][]u8 = [env, "/usr/share/zoneinfo", "/usr/lib/zoneinfo", "/usr/share/lib/zoneinfo",
                             "/etc/zoneinfo"];
        for d in dirs {
            if d.len == 0 {
                continue;
            }
            let mut p = strs.String.init();
            defer p.deinit();
            p.push_str(d)!;
            p.push('/')!;
            p.push_str(name)!;
            let data = fs.read_file(p.bytes()) catch continue;
            defer mem.free(data);
            return Zone.parse_tzif(data)!;
        }
        if strs.eq(name, "UTC") or strs.eq(name, "Etc/UTC") {
            return Zone.utc();
        }
        return error.UnknownZone;
    }

    /// The local time zone: from $TZ (":name", "name", "/path" or a POSIX
    /// rule), else /etc/localtime, else UTC.
    pub fn local() !Zone {
        let tzv = os.getenv("TZ") catch {
            let data = fs.read_file("/etc/localtime") catch return Zone.utc();
            defer mem.free(data);
            return Zone.parse_tzif(data) catch Zone.utc();
        };
        defer mem.free(tzv);
        let mut s: []u8 = tzv;
        if (s.len > 0) and (s[0] == ':') {
            s = s[1..];
        }
        if s.len == 0 {
            return Zone.utc();
        }
        if s[0] == '/' {
            let data = fs.read_file(s) catch return Zone.utc();
            defer mem.free(data);
            return Zone.parse_tzif(data) catch Zone.utc();
        }
        let z = Zone.load(s) catch {
            return Zone.parse_posix(s) catch Zone.utc();
        };
        return z;
    }

    /// A zone from a POSIX TZ string, such as "EST5EDT,M3.2.0,M11.1.0" or
    /// "<+0530>-5:30" (offsets are hours west of UTC, as POSIX has them).
    pub fn parse_posix(s: []u8) !Zone {
        let names = mem.alloc.[u8](s.len)!;
        let mut z = Zone.fixed_static(0, "");
        z.names = names;
        let mut used: usize = 0;
        parse_rule(s, names, &mut used, &mut z.rule) catch |e| {
            z.deinit();
            return e;
        };
        return z;
    }

    /// A zone from the bytes of a TZif file.
    pub fn parse_tzif(data: []u8) !Zone {
        let mut z = Zone.fixed_static(0, "");
        z.has_rule = false;
        read_tzif(data, &mut z) catch |e| {
            z.deinit();
            return e;
        };
        return z;
    }

    /// The offset, daylight saving flag and abbreviation at an instant
    /// (seconds since the Unix epoch).
    pub fn lookup(self: *Zone, t: i64) Info {
        let n = self.trans.len;
        if (n == 0) or (t < self.trans[0]) {
            if (self.kinds.len == 0) or ((n == 0) and self.has_rule) {
                return rule_info(&self.rule, t);
            }
            return info(self.kinds[0]);
        }
        if (t >= self.trans[n - 1]) and self.has_rule {
            return rule_info(&self.rule, t);
        }
        // the last transition at or before t
        let mut lo: usize = 0;
        let mut hi = n;
        while (hi - lo) > 1 {
            let mid = lo + ((hi - lo) / 2);
            if self.trans[mid] <= t {
                lo = mid;
            } else {
                hi = mid;
            }
        }
        return info(self.kinds[@as(usize, self.idx[lo])]);
    }

    /// The calendar fields of t in this zone.
    pub fn to_local(self: *Zone, t: time.Time) time.DateTime {
        return t.at_offset(self.lookup(t.unix()).offset);
    }

    /// The instant a local time names (dt's offset, weekday and yearday
    /// are ignored). A time that occurs twice (when clocks go back) is the
    /// first occurrence for fold 0 and the second for fold 1; a time that
    /// does not occur (clocks go forward) is read with the offset from
    /// before the change for fold 0 and after it for fold 1, so fold 0
    /// lands after the gap and fold 1 before it. This is Python's rule
    /// (PEP 495).
    pub fn from_local(self: *Zone, dt: time.DateTime, fold: u32) !time.Time {
        let l = local_seconds(dt)!;
        let mut c = Candidates.{ t: [0; 3], n: 0, before: 0, after: 0 };
        self.candidates(l, &mut c);
        let mut t: i64 = 0;
        if c.n == 0 {
            // a gap
            if fold == 0 {
                t = l - @as(i64, c.before);
            } else {
                t = l - @as(i64, c.after);
            }
        } else if (c.n == 1) or (fold == 0) {
            t = c.t[0];
        } else {
            t = c.t[c.n - 1];
        }
        return time.Time.from_unix(t, dt.nanosecond);
    }

    /// from_local, failing with NonExistent for a skipped time and
    /// Ambiguous for a repeated one.
    pub fn from_local_strict(self: *Zone, dt: time.DateTime) !time.Time {
        let l = local_seconds(dt)!;
        let mut c = Candidates.{ t: [0; 3], n: 0, before: 0, after: 0 };
        self.candidates(l, &mut c);
        if c.n == 0 {
            return error.NonExistent;
        }
        if c.n > 1 {
            return error.Ambiguous;
        }
        return time.Time.from_unix(c.t[0], dt.nanosecond);
    }

    // The instants whose local time is l, in increasing order, from the
    // offsets in force within a day of it; and the offsets a day before and
    // a day after.
    fn candidates(self: *Zone, l: i64, c: *mut Candidates) void {
        let before = self.lookup(l - 86400).offset;
        let after = self.lookup(l + 86400).offset;
        let mid = self.lookup(l).offset;
        c.before = before;
        c.after = after;
        let offs: [3]i32 = [before, mid, after];
        for o in offs {
            let t = l - @as(i64, o);
            if self.lookup(t).offset != o {
                continue;
            }
            let mut dup = false;
            for k in @as(usize, 0)..c.n {
                if c.t[k] == t {
                    dup = true;
                }
            }
            if dup {
                continue;
            }
            // insert in order
            let mut k = c.n;
            while (k > 0) and (c.t[k - 1] > t) {
                c.t[k] = c.t[k - 1];
                k -= 1;
            }
            c.t[k] = t;
            c.n += 1;
        }
    }
}

struct Candidates {
    t: [3]i64,
    n: usize,
    before: i32,
    after: i32,
}

fn empty_i64() []mut i64 {
    return mem.alloc.[i64](0) catch @trap();
}

fn empty_u8() []mut u8 {
    return mem.alloc.[u8](0) catch @trap();
}

fn empty_kinds() []mut Kind {
    return mem.alloc.[Kind](0) catch @trap();
}

fn info(k: Kind) Info {
    return Info.{ offset: k.offset, is_dst: k.dst, abbrev: k.name };
}

// Local time as seconds since the epoch, as if it were UTC.
fn local_seconds(dt: time.DateTime) !i64 {
    let u = time.DateTime.{ year: dt.year, month: dt.month, day: dt.day, hour: dt.hour, minute: dt.minute,
                            second: dt.second, nanosecond: 0, weekday: time.Weekday.SUNDAY, yearday: 0, offset: 0 };
    let t = u.to_time() catch return error.NonExistent;
    return t.unix();
}

fn valid_name(name: []u8) bool {
    if (name.len == 0) or (name.len > 255) or (name[0] == '/') {
        return false;
    }
    for c in name {
        if not ((((strs.is_alnum(c) or (c == '/')) or ((c == '_') or (c == '-'))) or (c == '+')) or (c == '.')) {
            return false;
        }
    }
    // no "." or ".." components
    let mut parts = strs.split(name, "/");
    while true {
        let p = parts.next() catch break;
        if (p.len == 0) or strs.eq(p, ".") or strs.eq(p, "..") {
            return false;
        }
    }
    return true;
}

// -----------------------------------------------------------------------------
// TZif
// -----------------------------------------------------------------------------

fn be32(d: []u8, at: usize) u32 {
    return (((@as(u32, d[at]) << 24) | (@as(u32, d[at + 1]) << 16)) | (@as(u32, d[at + 2]) << 8)) | @as(u32, d[at + 3]);
}

fn be64(d: []u8, at: usize) i64 {
    let hi = @as(u64, be32(d, at));
    let lo = @as(u64, be32(d, at + 4));
    return @bit_cast(i64, (hi << 32) | lo);
}

struct Header {
    version: u8,
    isutcnt: usize,
    isstdcnt: usize,
    leapcnt: usize,
    timecnt: usize,
    typecnt: usize,
    charcnt: usize,
}

fn header(d: []u8, at: usize, h: *mut Header) !void {
    if ((at + 44) > d.len) or not strs.eq(d[at..at + 4], "TZif") {
        return error.InvalidZone;
    }
    h.version = d[at + 4];
    if (h.version != 0) and ((h.version < '2') or (h.version > '4')) {
        return error.InvalidZone;
    }
    let counts: [6]u32 = [be32(d, at + 20), be32(d, at + 24), be32(d, at + 28), be32(d, at + 32), be32(d, at + 36),
                          be32(d, at + 40)];
    for c in counts {
        if c > 1000000 {
            return error.InvalidZone;
        }
    }
    h.isutcnt = @as(usize, counts[0]);
    h.isstdcnt = @as(usize, counts[1]);
    h.leapcnt = @as(usize, counts[2]);
    h.timecnt = @as(usize, counts[3]);
    h.typecnt = @as(usize, counts[4]);
    h.charcnt = @as(usize, counts[5]);
    if ((h.typecnt == 0) or (h.typecnt > 256)) or (h.charcnt == 0) {
        return error.InvalidZone;
    }
    if ((h.isutcnt != 0) and (h.isutcnt != h.typecnt)) or ((h.isstdcnt != 0) and (h.isstdcnt != h.typecnt)) {
        return error.InvalidZone;
    }
}

fn block_size(h: *Header, tsize: usize) usize {
    return ((((((h.timecnt * tsize) + h.timecnt) + (h.typecnt * 6)) + h.charcnt) + (h.leapcnt * (tsize + 4))) +
            h.isstdcnt) + h.isutcnt;
}

fn read_tzif(d: []u8, z: *mut Zone) !void {
    let mut h = Header.{ version: 0, isutcnt: 0, isstdcnt: 0, leapcnt: 0, timecnt: 0, typecnt: 0, charcnt: 0 };
    header(d, 0, &mut h)!;
    let mut at: usize = 44;
    let mut tsize: usize = 4;
    if h.version != 0 {
        // skip the version 1 data for the 64-bit version
        at += block_size(&h, 4);
        header(d, at, &mut h)!;
        at += 44;
        tsize = 8;
    }
    if (at + block_size(&h, tsize)) > d.len {
        return error.InvalidZone;
    }
    mem.free(z.trans);
    z.trans = mem.alloc.[i64](h.timecnt)!;
    mem.free(z.idx);
    z.idx = mem.alloc.[u8](h.timecnt)!;
    mem.free(z.kinds);
    z.kinds = mem.alloc.[Kind](h.typecnt)!;
    for k in @as(usize, 0)..h.timecnt {
        if tsize == 8 {
            z.trans[k] = be64(d, at + (k * 8));
        } else {
            z.trans[k] = @as(i64, @bit_cast(i32, be32(d, at + (k * 4))));
        }
        if (k > 0) and (z.trans[k] <= z.trans[k - 1]) {
            return error.InvalidZone;
        }
    }
    at += h.timecnt * tsize;
    for k in @as(usize, 0)..h.timecnt {
        if @as(usize, d[at + k]) >= h.typecnt {
            return error.InvalidZone;
        }
        z.idx[k] = d[at + k];
    }
    at += h.timecnt;
    let types_at = at;
    at += h.typecnt * 6;
    // the abbreviations, extended by the rule's names later
    let chars = d[at..at + h.charcnt];
    // (room for the footer's names too: it is shorter than the file)
    mem.free(z.names);
    z.names = mem.alloc.[u8](h.charcnt + d.len)!;
    @mem_copy(z.names[..h.charcnt], chars);
    for k in @as(usize, 0)..h.typecnt {
        let p = types_at + (k * 6);
        let off = @bit_cast(i32, be32(d, p));
        if (off < -89999) or (off > 93599) {
            return error.InvalidZone;
        }
        let di = @as(usize, d[p + 5]);
        if di >= h.charcnt {
            return error.InvalidZone;
        }
        let mut e = di;
        while (e < h.charcnt) and (chars[e] != 0) {
            e += 1;
        }
        z.kinds[k] = Kind.{ offset: off, dst: d[p + 4] != 0, name: z.names[di..e] };
    }
    at = types_at + (block_size(&h, tsize) - (((h.timecnt * tsize) + h.timecnt)));
    // (leap seconds and the standard/UT indicators are not needed)
    z.has_rule = false;
    if (h.version != 0) and (at < d.len) {
        // the footer: "\n" rule "\n"
        if d[at] != '\n' {
            return error.InvalidZone;
        }
        let rest = d[at + 1..];
        let nl = strs.index_of_byte(rest, '\n') catch return error.InvalidZone;
        let s = rest[..nl];
        if s.len > 0 {
            let mut used = h.charcnt;
            parse_rule(s, z.names, &mut used, &mut z.rule)!;
            z.has_rule = true;
        }
    }
}

// -----------------------------------------------------------------------------
// POSIX TZ rules
// -----------------------------------------------------------------------------

struct Cursor {
    s: []u8,
    i: usize,

    fn peek(self: *Cursor) u8 {
        if self.i < self.s.len {
            return self.s[self.i];
        }
        return 0;
    }

    fn eat(self: *mut Cursor, c: u8) bool {
        if self.peek() == c {
            self.i += 1;
            return true;
        }
        return false;
    }

    fn num(self: *mut Cursor, max: u32) !u32 {
        let j = self.i;
        let mut v: u32 = 0;
        while strs.is_digit(self.peek()) {
            v = (v * 10) + @as(u32, self.peek() - '0');
            if v > max {
                return error.InvalidZone;
            }
            self.i += 1;
        }
        if self.i == j {
            return error.InvalidZone;
        }
        return v;
    }

    // [+-]hh[:mm[:ss]] as seconds
    fn hms(self: *mut Cursor, max_hours: u32) !i32 {
        let mut neg = false;
        if self.eat('-') {
            neg = true;
        } else {
            _ = self.eat('+');
        }
        let h = self.num(max_hours)!;
        let mut s = @as(i32, @int_cast(i32, h)) * 3600;
        if self.eat(':') {
            s += @int_cast(i32, self.num(59)!) * 60;
            if self.eat(':') {
                s += @int_cast(i32, self.num(59)!);
            }
        }
        if neg {
            return -s;
        }
        return s;
    }

    // a zone name: <...> or letters, copied into names
    fn name(self: *mut Cursor, names: []mut u8, used: *mut usize) ![]u8 {
        let mut a = self.i;
        let mut b = self.i;
        if self.eat('<') {
            a = self.i;
            while (self.peek() != '>') and (self.peek() != 0) {
                let c = self.peek();
                if not ((strs.is_alnum(c) or (c == '+')) or (c == '-')) {
                    return error.InvalidZone;
                }
                self.i += 1;
            }
            b = self.i;
            if not self.eat('>') {
                return error.InvalidZone;
            }
        } else {
            while strs.is_alpha(self.peek()) {
                self.i += 1;
            }
            b = self.i;
        }
        if (b - a) < 3 {
            return error.InvalidZone;
        }
        if ((used.* + b) - a) > names.len {
            return error.InvalidZone;
        }
        let at = used.*;
        @mem_copy(names[at..(at + b) - a], self.s[a..b]);
        used.* += b - a;
        return names[at..used.*];
    }

    fn when(self: *mut Cursor) !When {
        let mut w = When.{ kind: 'n', n: 0, m: 0, w: 0, d: 0, time: 7200 };
        if self.eat('J') {
            w.kind = 'J';
            w.n = self.num(365)!;
            if w.n == 0 {
                return error.InvalidZone;
            }
        } else if self.eat('M') {
            w.kind = 'M';
            w.m = self.num(12)!;
            if (w.m == 0) or not self.eat('.') {
                return error.InvalidZone;
            }
            w.w = self.num(5)!;
            if (w.w == 0) or not self.eat('.') {
                return error.InvalidZone;
            }
            w.d = self.num(6)!;
        } else {
            w.n = self.num(365)!;
        }
        if self.eat('/') {
            w.time = self.hms(167)!;
        }
        return w;
    }
}

fn parse_rule(s: []u8, names: []mut u8, used: *mut usize, r: *mut Rule) !void {
    let mut c = Cursor.{ s: s, i: 0 };
    let std_name = c.name(names, used)!;
    // POSIX offsets are west of UTC
    let std_off = -c.hms(24)!;
    r.std = Kind.{ offset: std_off, dst: false, name: std_name };
    r.dst = r.std;
    r.has_dst = false;
    if c.i == s.len {
        return;
    }
    let dst_name = c.name(names, used)!;
    let mut dst_off = std_off + 3600;
    if (c.peek() != ',') and (c.peek() != 0) {
        dst_off = -c.hms(24)!;
    }
    r.dst = Kind.{ offset: dst_off, dst: true, name: dst_name };
    r.has_dst = true;
    if c.eat(',') {
        r.start = c.when()!;
        if not c.eat(',') {
            return error.InvalidZone;
        }
        r.end = c.when()!;
    } else {
        // the default: the United States' rule
        r.start = When.{ kind: 'M', n: 0, m: 3, w: 2, d: 0, time: 7200 };
        r.end = When.{ kind: 'M', n: 0, m: 11, w: 1, d: 0, time: 7200 };
    }
    if c.i != s.len {
        return error.InvalidZone;
    }
}

// The day (days since the epoch) a rule names in a year.
fn rule_day(w: When, year: i64) i64 {
    let jan1 = time.days_from_civil(year, 1, 1);
    if w.kind == 'J' {
        // 1-365, never counting February 29
        let mut d = @as(i64, w.n) - 1;
        if time.is_leap_year(year) and (w.n >= 60) {
            d += 1;
        }
        return jan1 + d;
    }
    if w.kind == 'n' {
        return jan1 + @as(i64, w.n);
    }
    // day d of week w (5: the last) of month m
    let first = time.days_from_civil(year, w.m, 1);
    let wd = (((first + 4) % 7) + 7) % 7;  // 1970-01-01 was a Thursday
    let mut day = ((@as(i64, w.d) - wd) + 7) % 7;
    day += @as(i64, w.w - 1) * 7;
    let len = @as(i64, time.days_in_month(year, w.m));
    while day >= len {
        day -= 7;
    }
    return first + day;
}

fn rule_info(r: *Rule, t: i64) Info {
    if not r.has_dst {
        return info(r.std);
    }
    // the year, in local standard time
    let y = time.Time.from_unix(t + @as(i64, r.std.offset), 0).utc().year;
    let start = ((rule_day(r.start, y) * 86400) + @as(i64, r.start.time)) - @as(i64, r.std.offset);
    let end = ((rule_day(r.end, y) * 86400) + @as(i64, r.end.time)) - @as(i64, r.dst.offset);
    let mut dst = false;
    if start < end {
        dst = (t >= start) and (t < end);
    } else {
        dst = not ((t >= end) and (t < start));
    }
    if dst {
        return info(r.dst);
    }
    return info(r.std);
}

// -----------------------------------------------------------------------------
// Tests
// -----------------------------------------------------------------------------

test "Python's zoneinfo" {
    let data = fs.read_file("std/time/tz/testdata/cases.txt") catch return;
    defer mem.free(data);
    let mut lines = strs.lines(data);
    let mut current = strs.String.init();
    defer current.deinit();
    let mut z = Zone.utc();
    defer z.deinit();
    let mut n: usize = 0;
    while true {
        let line = lines.next() catch break;
        let mut f = strs.fields(line);
        let op = f.next()!;
        let file = f.next()!;
        if not strs.eq(file, current.bytes()) {
            let mut p = strs.String.init();
            defer p.deinit();
            p.push_str("std/time/tz/testdata/")!;
            p.push_str(file)!;
            let bytes = fs.read_file(p.bytes())!;
            defer mem.free(bytes);
            z.deinit();
            z = Zone.parse_tzif(bytes)!;
            current.clear();
            current.push_str(file)!;
        }
        if strs.eq(op, "lookup") {
            let t = fmt_i64(f.next()!);
            let off = fmt_i64(f.next()!);
            let dst = fmt_i64(f.next()!);
            let abbr = f.next()!;
            let got = z.lookup(t);
            if ((@as(i64, got.offset) != off) or (got.is_dst != (dst == 1))) or not strs.eq(got.abbrev, abbr) {
                @expect_eq(line, "");
            }
        } else {
            let y = fmt_i64(f.next()!);
            let mut v: [5]u32 = [0; 5];
            for k in @as(usize, 0)..5 {
                v[k] = @int_cast(u32, fmt_i64(f.next()!));
            }
            let fold = @int_cast(u32, fmt_i64(f.next()!));
            let want = fmt_i64(f.next()!);
            let dt = time.DateTime.{ year: y, month: v[0], day: v[1], hour: v[2], minute: v[3], second: v[4],
                                     nanosecond: 0, weekday: time.Weekday.SUNDAY, yearday: 0, offset: 0 };
            let got = z.from_local(dt, fold)!;
            if got.unix() != want {
                @expect_eq(line, "");
            }
        }
        n += 1;
    }
    @expect(n == 6270);
}

fn fmt_i64(s: []u8) i64 {
    return io.parse_i64(s) catch @trap();
}

test "rules, names and the system database" {
    let mut z = Zone.parse_posix("EST5EDT,M3.2.0,M11.1.0")!;
    defer z.deinit();
    // 2026-03-08 06:59:59 UTC is 01:59:59 EST; a second later, 03:00 EDT
    let t = time.date(2026, 3, 8, 6, 59, 59)!.unix();
    @expect(strs.eq(z.lookup(t).abbrev, "EST") and (z.lookup(t).offset == -18000));
    @expect(strs.eq(z.lookup(t + 1).abbrev, "EDT") and z.lookup(t + 1).is_dst);
    let gap = time.DateTime.{ year: 2026, month: 3, day: 8, hour: 2, minute: 30, second: 0, nanosecond: 0,
                              weekday: time.Weekday.SUNDAY, yearday: 0, offset: 0 };
    // fold 0: read with EST, so 07:30 UTC (03:30 EDT); fold 1: with EDT
    @expect(z.from_local(gap, 0)!.unix() == (t + 1801));
    @expect(z.from_local(gap, 1)!.unix() == ((t + 1801) - 3600));
    @expect(nonexistent(z.from_local_strict(gap)));
    let back = time.DateTime.{ year: 2026, month: 11, day: 1, hour: 1, minute: 30, second: 0, nanosecond: 0,
                               weekday: time.Weekday.SUNDAY, yearday: 0, offset: 0 };
    let first = z.from_local(back, 0)!.unix();
    let second = z.from_local(back, 1)!.unix();
    @expect((second - first) == 3600);
    // zero-based days: 59 is March 1, or February 29 in a leap year
    let mut n = Zone.parse_posix("AAA0BBB,59/0,300/0")!;
    defer n.deinit();
    let mar1 = time.date(2027, 3, 1, 0, 0, 0)!.unix();
    @expect(not n.lookup(mar1 - 1).is_dst and n.lookup(mar1).is_dst);
    let feb29 = time.date(2028, 2, 29, 0, 0, 0)!.unix();
    @expect(not n.lookup(feb29 - 1).is_dst and n.lookup(feb29).is_dst);
    // day 300 of 2027 is October 28 (ending at midnight BBB, 23:00 AAA)
    let oct28 = time.date(2027, 10, 28, 0, 0, 0)!.unix() - 3600;
    @expect(n.lookup(oct28 - 1).is_dst and not n.lookup(oct28).is_dst);
    let bad: [7][]u8 = ["", "E5", "EST", "EST5EDT,M3.2.0", "EST5EDT,M13.1.0,M11.1.0", "<+05", "EST5EDT,J0,J365"];
    for s in bad {
        @expect(invalid(Zone.parse_posix(s)));
    }
    let names: [6][]u8 = ["../etc/passwd", "/etc/passwd", "a//b", "a/./b", "", "Europe/Paris\n"];
    for s in names {
        @expect(not valid_name(s));
    }
    @expect(valid_name("America/Argentina/Buenos_Aires") and valid_name("Etc/GMT+5"));
    // the local zone (UTC when there is none)
    let mut lz = Zone.local()!;
    let now = lz.lookup(time.now().unix());
    @expect((now.offset > -86400) and (now.offset < 86400));
    lz.deinit();
    // the system's database, where there is one
    let mut ny = Zone.load("America/New_York") catch return;
    defer ny.deinit();
    let summer = time.date(2026, 7, 1, 12, 0, 0)!;
    let dt = ny.to_local(summer);
    @expect((dt.hour == 8) and (dt.offset == -14400));
    @expect(unknown(Zone.load("No/Such_Zone")));
}

fn invalid(r: !Zone) bool {
    let mut z = r catch |e| return e == error.InvalidZone;
    z.deinit();
    return false;
}

fn unknown(r: !Zone) bool {
    let mut z = r catch |e| return e == error.UnknownZone;
    z.deinit();
    return false;
}

fn nonexistent(r: !time.Time) bool {
    _ = r catch |e| return e == error.NonExistent;
    return false;
}
