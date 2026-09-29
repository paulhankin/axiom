// =============================================================================
// std/time.ph - clocks, durations, and calendar dates.
//
//     let start = time.Instant.now();
//     work();
//     let d = start.elapsed();                  // a Duration
//     fmt.print("took {}\n", [fmt.str(d.format(buf[..]))])!;   // "1.25ms"
//
//     let t = time.now();                       // wall-clock Time
//     let dt = t.utc();                         // year, month, day, ...
//     let s = t.format_rfc3339(buf[..]);        // "2026-09-28T12:34:56.789Z"
//
// Instant is monotonic: it never goes backwards, so use it to measure
// intervals. Time is the real (wall-clock) time, which can jump when the
// system clock is set. Dates use the proleptic Gregorian calendar, in UTC or
// at a fixed offset from it; there is no time-zone database.
//
// The clocks use the Phalange host ABI (import module "phalange").
// =============================================================================

import "io.ph" as io;

error { InvalidTime, NoSpaceLeft }

import host "phalange" {
    fn ph_clock_ns(clock: u32) i64;
    fn ph_sleep_ns(ns: u64) void;
}

const CLOCK_REALTIME: u32 = 0;
const CLOCK_MONOTONIC: u32 = 1;

const NS_PER_US: i64 = 1000;
const NS_PER_MS: i64 = 1000000;
const NS_PER_S: i64 = 1000000000;

// -----------------------------------------------------------------------------
// Duration
// -----------------------------------------------------------------------------

/// A signed length of time in nanoseconds (about ±292 years).
pub struct Duration {
    ns: i64,

    pub fn nanoseconds(n: i64) Duration {
        return Duration.{ ns: n };
    }

    pub fn microseconds(n: i64) Duration {
        return Duration.{ ns: n * NS_PER_US };
    }

    pub fn milliseconds(n: i64) Duration {
        return Duration.{ ns: n * NS_PER_MS };
    }

    pub fn seconds(n: i64) Duration {
        return Duration.{ ns: n * NS_PER_S };
    }

    pub fn minutes(n: i64) Duration {
        return Duration.{ ns: n * (60 * NS_PER_S) };
    }

    pub fn hours(n: i64) Duration {
        return Duration.{ ns: n * (3600 * NS_PER_S) };
    }

    /// Whole milliseconds (truncated toward zero).
    pub fn as_millis(self: Duration) i64 {
        return self.ns / NS_PER_MS;
    }

    /// Whole microseconds (truncated toward zero).
    pub fn as_micros(self: Duration) i64 {
        return self.ns / NS_PER_US;
    }

    pub fn as_seconds(self: Duration) f64 {
        return @float_from_int(f64, self.ns) / 1.0e9;
    }

    pub fn add(self: Duration, o: Duration) Duration {
        return Duration.{ ns: self.ns + o.ns };
    }

    pub fn sub(self: Duration, o: Duration) Duration {
        return Duration.{ ns: self.ns - o.ns };
    }

    /// Like Go's Duration.String: "1h2m3.5s", "1.5s", "250ms", "3µs",
    /// "7ns", "0s". 32 bytes suffice.
    pub fn format(self: Duration, buf: []mut u8) []u8 {
        let mut b = Buf.{ buf: buf, n: 0 };
        let neg = self.ns < 0;
        let mut u = @bit_cast(u64, self.ns);
        if neg {
            u = 0 -% u;
            b.put('-');
        }
        if u < 1000000000 {
            // below a second: a fraction of the biggest fitting unit
            if u == 0 {
                b.puts("0s");
            } else if u < 1000 {
                b.uint(u);
                b.puts("ns");
            } else if u < 1000000 {
                b.fraction(u, 1000);
                b.puts("\u{b5}s");
            } else {
                b.fraction(u, 1000000);
                b.puts("ms");
            }
            return b.buf[..b.n];
        }
        let secs = u / 1000000000;
        let h = secs / 3600;
        let m = (secs / 60) % 60;
        if h > 0 {
            b.uint(h);
            b.put('h');
        }
        if (h > 0) or (m > 0) {
            b.uint(m);
            b.put('m');
        }
        b.fraction((u % 60000000000), 1000000000);
        b.put('s');
        return b.buf[..b.n];
    }
}

/// A small formatting cursor over a caller's buffer.
struct Buf {
    buf: []mut u8,
    n: usize,

    fn put(self: *mut Buf, c: u8) void {
        if self.n < self.buf.len {
            self.buf[self.n] = c;
            self.n += 1;
        }
    }

    fn puts(self: *mut Buf, s: []u8) void {
        for c in s {
            self.put(c);
        }
    }

    fn uint(self: *mut Buf, x: u64) void {
        let mut tmp: [20]u8 = [0; 20];
        self.puts(io.fmt_u64(tmp[..], x));
    }

    /// x / unit with the fraction's trailing zeros dropped.
    fn fraction(self: *mut Buf, x: u64, unit: u64) void {
        self.uint(x / unit);
        let mut f = x % unit;
        if f == 0 {
            return;
        }
        self.put('.');
        let mut u = unit / 10;
        while (f > 0) and (u > 0) {
            self.put('0' + @truncate(u8, f / u));
            f = f % u;
            u = u / 10;
        }
    }

    /// x in exactly `width` digits with leading zeros.
    fn padded(self: *mut Buf, x: u64, width: usize) void {
        let mut tmp: [20]u8 = [0; 20];
        let d = io.fmt_u64(tmp[..], x);
        for i in d.len..width {
            self.put('0');
        }
        self.puts(d);
    }
}

// -----------------------------------------------------------------------------
// Instant: the monotonic clock
// -----------------------------------------------------------------------------

/// A point on the monotonic clock, for measuring intervals. Its zero is
/// arbitrary (often the boot time).
pub struct Instant {
    ns: i64,

    pub fn now() Instant {
        return Instant.{ ns: ph_clock_ns(CLOCK_MONOTONIC) };
    }

    /// The time since this instant.
    pub fn elapsed(self: Instant) Duration {
        return Duration.{ ns: ph_clock_ns(CLOCK_MONOTONIC) - self.ns };
    }

    /// self - earlier.
    pub fn since(self: Instant, earlier: Instant) Duration {
        return Duration.{ ns: self.ns - earlier.ns };
    }

    pub fn add(self: Instant, d: Duration) Instant {
        return Instant.{ ns: self.ns + d.ns };
    }
}

/// Blocks the calling thread for at least d (nothing if d <= 0).
pub fn sleep(d: Duration) void {
    if d.ns > 0 {
        ph_sleep_ns(@bit_cast(u64, d.ns));
    }
}

// -----------------------------------------------------------------------------
// Calendar arithmetic (Howard Hinnant's algorithms)
// -----------------------------------------------------------------------------

/// Days since 1970-01-01 of a proleptic Gregorian date.
pub fn days_from_civil(year: i64, month: u32, day: u32) i64 {
    let mut y = year;
    if month <= 2 {
        y -= 1;
    }
    let mut era = y / 400;
    if y < 0 {
        era = (y - 399) / 400;
    }
    let yoe = y - (era * 400);
    let mut mp = @as(i64, month) + 9;
    if month > 2 {
        mp = @as(i64, month) - 3;
    }
    let doy = (((153 * mp) + 2) / 5) + (@as(i64, day) - 1);
    let doe = ((yoe * 365) + (yoe / 4) - (yoe / 100)) + doy;
    return ((era * 146097) + doe) - 719468;
}

struct Civil {
    year: i64,
    month: u32,
    day: u32,
}

fn civil_from_days(days: i64) Civil {
    let z = days + 719468;
    let mut era = z / 146097;
    if z < 0 {
        era = (z - 146096) / 146097;
    }
    let doe = z - (era * 146097);
    let yoe = (((doe - (doe / 1460)) + (doe / 36524)) - (doe / 146096)) / 365;
    let doy = doe - (((365 * yoe) + (yoe / 4)) - (yoe / 100));
    let mp = ((5 * doy) + 2) / 153;
    let d = (doy - (((153 * mp) + 2) / 5)) + 1;
    let mut m = mp + 3;
    if mp >= 10 {
        m = mp - 9;
    }
    let mut y = yoe + (era * 400);
    if m <= 2 {
        y += 1;
    }
    return Civil.{ year: y, month: @int_cast(u32, m), day: @int_cast(u32, d) };
}

pub fn is_leap_year(year: i64) bool {
    return ((year % 4) == 0) and (((year % 100) != 0) or ((year % 400) == 0));
}

pub fn days_in_month(year: i64, month: u32) u32 {
    let days: [12]u32 = [31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31];
    if (month == 2) and is_leap_year(year) {
        return 29;
    }
    return days[@as(usize, month - 1)];
}

fn floor_div(a: i64, b: i64) i64 {
    let q = a / b;
    if ((a % b) != 0) and ((a < 0) != (b < 0)) {
        return q - 1;
    }
    return q;
}

// -----------------------------------------------------------------------------
// DateTime: a broken-down time
// -----------------------------------------------------------------------------

/// Calendar fields of a Time at some offset from UTC.
pub struct DateTime {
    year: i64,
    month: u32,       // 1 to 12
    day: u32,         // 1 to 31
    hour: u32,        // 0 to 23
    minute: u32,      // 0 to 59
    second: u32,      // 0 to 59
    nanosecond: u32,  // 0 to 999_999_999
    weekday: u32,     // 0 Sunday to 6 Saturday
    yearday: u32,     // 1 to 366
    offset: i32,      // seconds east of UTC

    /// The Time these fields name. Fails with InvalidTime if a field is out
    /// of range (weekday and yearday are ignored).
    pub fn to_time(self: DateTime) !Time {
        if (self.month < 1) or (self.month > 12) or (self.day < 1) or
           (self.day > days_in_month(self.year, self.month)) {
            return error.InvalidTime;
        }
        if (self.hour > 23) or (self.minute > 59) or (self.second > 59) or (self.nanosecond > 999999999) {
            return error.InvalidTime;
        }
        if (self.year < -1000000) or (self.year > 1000000) {
            return error.InvalidTime;
        }
        let days = days_from_civil(self.year, self.month, self.day);
        let secs = (((days * 86400) + (@as(i64, self.hour) * 3600)) + (@as(i64, self.minute) * 60)) + @as(
            i64, self.second);
        return Time.{ sec: secs - @as(i64, self.offset), nsec: self.nanosecond };
    }
}

/// A DateTime from its date and time fields, at UTC.
pub fn date(year: i64, month: u32, day: u32, hour: u32, minute: u32, second: u32) !Time {
    let dt = DateTime.{
        year: year,
        month: month,
        day: day,
        hour: hour,
        minute: minute,
        second: second,
        nanosecond: 0,
        weekday: 0,
        yearday: 0,
        offset: 0,
    };
    return dt.to_time();
}

// -----------------------------------------------------------------------------
// Time: the wall clock
// -----------------------------------------------------------------------------

/// A moment in real time: seconds and nanoseconds since 1970-01-01 UTC
/// (the Unix epoch), ignoring leap seconds.
pub struct Time {
    sec: i64,
    nsec: u32,  // 0 to 999_999_999

    pub fn from_unix(sec: i64, nsec: u32) Time {
        let t = Time.{ sec: sec, nsec: 0 };
        return t.add(Duration.{ ns: @as(i64, nsec) });
    }

    pub fn from_unix_millis(ms: i64) Time {
        return Time.{ sec: floor_div(ms, 1000), nsec: @int_cast(u32, (ms - (floor_div(ms, 1000) * 1000)) * 1000000) };
    }

    /// Seconds since the epoch.
    pub fn unix(self: Time) i64 {
        return self.sec;
    }

    /// (Saturates beyond what an i64 holds, about 292 million years.)
    pub fn unix_millis(self: Time) i64 {
        return @add_sat(@mul_sat(self.sec, 1000), @as(i64, self.nsec / 1000000));
    }

    /// (Saturates beyond what an i64 holds: before 1677 and after 2262.)
    pub fn unix_nanos(self: Time) i64 {
        return @add_sat(@mul_sat(self.sec, NS_PER_S), @as(i64, self.nsec));
    }

    pub fn add(self: Time, d: Duration) Time {
        let mut sec = @add_sat(self.sec, d.ns / NS_PER_S);
        let mut ns = @as(i64, self.nsec) + (d.ns % NS_PER_S);
        if ns < 0 {
            ns += NS_PER_S;
            sec -= 1;
        } else if ns >= NS_PER_S {
            ns -= NS_PER_S;
            sec += 1;
        }
        return Time.{ sec: sec, nsec: @int_cast(u32, ns) };
    }

    /// self - other (saturating: a Duration holds about 292 years either way).
    pub fn sub(self: Time, other: Time) Duration {
        let s = @mul_sat(@sub_sat(self.sec, other.sec), NS_PER_S);
        return Duration.{ ns: @add_sat(s, @as(i64, self.nsec) - @as(i64, other.nsec)) };
    }

    /// Negative if self is earlier than other, 0 if equal, positive if later.
    pub fn compare(self: Time, other: Time) i32 {
        if self.sec != other.sec {
            if self.sec < other.sec {
                return -1;
            }
            return 1;
        }
        if self.nsec != other.nsec {
            if self.nsec < other.nsec {
                return -1;
            }
            return 1;
        }
        return 0;
    }

    pub fn before(self: Time, other: Time) bool {
        return self.compare(other) < 0;
    }

    pub fn after(self: Time, other: Time) bool {
        return self.compare(other) > 0;
    }

    /// The calendar fields in UTC.
    pub fn utc(self: Time) DateTime {
        return self.at_offset(0);
    }

    /// The calendar fields at `offset` seconds east of UTC.
    pub fn at_offset(self: Time, offset: i32) DateTime {
        let s = self.sec + @as(i64, offset);
        let days = floor_div(s, 86400);
        let rem = s - (days * 86400);
        let c = civil_from_days(days);
        let jan1 = days_from_civil(c.year, 1, 1);
        return DateTime.{
            year: c.year,
            month: c.month,
            day: c.day,
            hour: @int_cast(u32, rem / 3600),
            minute: @int_cast(u32, (rem / 60) % 60),
            second: @int_cast(u32, rem % 60),
            nanosecond: self.nsec,
            weekday: @int_cast(u32, (days + 4) - (floor_div(days + 4, 7) * 7)),
            yearday: @int_cast(u32, (days - jan1) + 1),
            offset: offset,
        };
    }

    /// RFC 3339 in UTC, with a fraction only when there is one:
    /// "2026-09-28T12:34:56Z", "2026-09-28T12:34:56.5Z". 40 bytes suffice.
    pub fn format_rfc3339(self: Time, buf: []mut u8) []u8 {
        return self.format_rfc3339_offset(buf, 0);
    }

    /// RFC 3339 at `offset` seconds east of UTC: "2026-09-28T14:34:56+02:00".
    pub fn format_rfc3339_offset(self: Time, buf: []mut u8, offset: i32) []u8 {
        let dt = self.at_offset(offset);
        let mut b = Buf.{ buf: buf, n: 0 };
        write_year(&mut b, dt.year);
        b.put('-');
        b.padded(@as(u64, dt.month), 2);
        b.put('-');
        b.padded(@as(u64, dt.day), 2);
        b.put('T');
        b.padded(@as(u64, dt.hour), 2);
        b.put(':');
        b.padded(@as(u64, dt.minute), 2);
        b.put(':');
        b.padded(@as(u64, dt.second), 2);
        if dt.nanosecond != 0 {
            let mut f = dt.nanosecond;
            let mut digits: usize = 9;
            while (f % 10) == 0 {
                f = f / 10;
                digits -= 1;
            }
            b.put('.');
            b.padded(@as(u64, f), digits);
        }
        if offset == 0 {
            b.put('Z');
        } else {
            let mut o = offset;
            if o < 0 {
                b.put('-');
                o = -o;
            } else {
                b.put('+');
            }
            b.padded(@as(u64, @int_cast(u32, o / 3600)), 2);
            b.put(':');
            b.padded(@as(u64, @int_cast(u32, (o / 60) % 60)), 2);
        }
        return b.buf[..b.n];
    }

    /// The HTTP date format (RFC 9110 IMF-fixdate):
    /// "Sun, 06 Nov 1994 08:49:37 GMT". 29 bytes.
    pub fn format_http(self: Time, buf: []mut u8) []u8 {
        let dt = self.utc();
        let mut b = Buf.{ buf: buf, n: 0 };
        let wd = dt.weekday * 3;
        b.puts(DAY_NAMES[@as(usize, wd)..@as(usize, wd + 3)]);
        b.puts(", ");
        b.padded(@as(u64, dt.day), 2);
        b.put(' ');
        let mo = (dt.month - 1) * 3;
        b.puts(MONTH_NAMES[@as(usize, mo)..@as(usize, mo + 3)]);
        b.put(' ');
        write_year(&mut b, dt.year);
        b.put(' ');
        b.padded(@as(u64, dt.hour), 2);
        b.put(':');
        b.padded(@as(u64, dt.minute), 2);
        b.put(':');
        b.padded(@as(u64, dt.second), 2);
        b.puts(" GMT");
        return b.buf[..b.n];
    }
}

let DAY_NAMES: []u8 = "SunMonTueWedThuFriSat";
let MONTH_NAMES: []u8 = "JanFebMarAprMayJunJulAugSepOctNovDec";

fn write_year(b: *mut Buf, y: i64) void {
    if y < 0 {
        b.put('-');
        b.padded(@bit_cast(u64, -y), 4);
    } else {
        b.padded(@bit_cast(u64, y), 4);
    }
}

/// The current wall-clock time.
pub fn now() Time {
    let ns = ph_clock_ns(CLOCK_REALTIME);
    return Time.{ sec: floor_div(ns, NS_PER_S), nsec: @int_cast(u32, ns - (floor_div(ns, NS_PER_S) * NS_PER_S)) };
}

// -----------------------------------------------------------------------------
// Parsing
// -----------------------------------------------------------------------------

/// A cursor over the text being parsed.
struct Scan {
    s: []u8,
    i: usize,

    fn digits(self: *mut Scan, n: usize) !u32 {
        if (self.i + n) > self.s.len {
            return error.InvalidTime;
        }
        let mut v: u32 = 0;
        for k in 0..n {
            let c = self.s[self.i + k];
            if (c < '0') or (c > '9') {
                return error.InvalidTime;
            }
            v = (v * 10) + @as(u32, c - '0');
        }
        self.i += n;
        return v;
    }

    fn expect(self: *mut Scan, c: u8) !void {
        if (self.i >= self.s.len) or (self.s[self.i] != c) {
            return error.InvalidTime;
        }
        self.i += 1;
    }

    fn peek(self: *Scan) u8 {
        if self.i >= self.s.len {
            return 0;
        }
        return self.s[self.i];
    }
}

/// Parses an RFC 3339 timestamp: "2026-09-28T12:34:56Z",
/// "2026-09-28t12:34:56.123456+02:00", "2026-09-28 12:34:56-05:30".
pub fn parse_rfc3339(s: []u8) !Time {
    let mut sc = Scan.{ s: s, i: 0 };
    let year = sc.digits(4)!;
    sc.expect('-')!;
    let month = sc.digits(2)!;
    sc.expect('-')!;
    let day = sc.digits(2)!;
    let sep = sc.peek();
    if (sep != 'T') and (sep != 't') and (sep != ' ') {
        return error.InvalidTime;
    }
    sc.i += 1;
    let hour = sc.digits(2)!;
    sc.expect(':')!;
    let minute = sc.digits(2)!;
    sc.expect(':')!;
    let second = sc.digits(2)!;
    let mut nsec: u32 = 0;
    if sc.peek() == '.' {
        sc.i += 1;
        let mut n: usize = 0;
        let mut scale: u32 = 100000000;
        while (sc.peek() >= '0') and (sc.peek() <= '9') {
            if n < 9 {
                nsec += @as(u32, sc.peek() - '0') * scale;
                scale = scale / 10;
            }
            sc.i += 1;
            n += 1;
        }
        if n == 0 {
            return error.InvalidTime;
        }
    }
    let mut offset: i32 = 0;
    let z = sc.peek();
    if (z == 'Z') or (z == 'z') {
        sc.i += 1;
    } else if (z == '+') or (z == '-') {
        sc.i += 1;
        let oh = sc.digits(2)!;
        sc.expect(':')!;
        let om = sc.digits(2)!;
        if (oh > 23) or (om > 59) {
            return error.InvalidTime;
        }
        offset = @int_cast(i32, (oh * 3600) + (om * 60));
        if z == '-' {
            offset = -offset;
        }
    } else {
        return error.InvalidTime;
    }
    if sc.i != s.len {
        return error.InvalidTime;
    }
    // RFC 3339 allows a leap second (60); it is taken as the next second
    let mut leap: i64 = 0;
    let mut sec = second;
    if sec == 60 {
        sec = 59;
        leap = 1;
    }
    let dt = DateTime.{
        year: @as(i64, year),
        month: month,
        day: day,
        hour: hour,
        minute: minute,
        second: sec,
        nanosecond: nsec,
        weekday: 0,
        yearday: 0,
        offset: offset,
    };
    let t = dt.to_time()!;
    return Time.{ sec: t.sec + leap, nsec: t.nsec };
}

fn name_index(names: []u8, s: []u8) !u32 {
    let mut i: usize = 0;
    while (i + 3) <= names.len {
        if @mem_eq(names[i..i + 3], s) {
            return @int_cast(u32, i / 3);
        }
        i += 3;
    }
    return error.InvalidTime;
}

/// Parses an HTTP date in the IMF-fixdate form:
/// "Sun, 06 Nov 1994 08:49:37 GMT".
pub fn parse_http(s: []u8) !Time {
    if s.len != 29 {
        return error.InvalidTime;
    }
    _ = name_index(DAY_NAMES, s[0..3])!;
    let mut sc = Scan.{ s: s, i: 3 };
    sc.expect(',')!;
    sc.expect(' ')!;
    let day = sc.digits(2)!;
    sc.expect(' ')!;
    let month = name_index(MONTH_NAMES, s[sc.i..sc.i + 3])! + 1;
    sc.i += 3;
    sc.expect(' ')!;
    let year = sc.digits(4)!;
    sc.expect(' ')!;
    let hour = sc.digits(2)!;
    sc.expect(':')!;
    let minute = sc.digits(2)!;
    sc.expect(':')!;
    let second = sc.digits(2)!;
    if not @mem_eq(s[sc.i..], " GMT") {
        return error.InvalidTime;
    }
    return date(@as(i64, year), month, day, hour, minute, second);
}

// -----------------------------------------------------------------------------
// Tests (phalange test std/time.ph)
// -----------------------------------------------------------------------------

test "durations" {
    let mut b: [32]u8 = [0; 32];
    @expect_eq(Duration.seconds(0).format(b[..]), "0s");
    @expect_eq(Duration.nanoseconds(7).format(b[..]), "7ns");
    @expect_eq(Duration.microseconds(3).format(b[..]), "3\u{b5}s");
    @expect_eq(Duration.nanoseconds(1500).format(b[..]), "1.5\u{b5}s");
    @expect_eq(Duration.microseconds(1250).format(b[..]), "1.25ms");
    @expect_eq(Duration.milliseconds(1500).format(b[..]), "1.5s");
    @expect_eq(Duration.seconds(3723).add(Duration.milliseconds(500)).format(b[..]), "1h2m3.5s");
    @expect_eq(Duration.minutes(-2).format(b[..]), "-2m0s");
    @expect_eq(Duration.hours(1).as_millis(), 3600000);
    @expect_eq(Duration.milliseconds(250).as_seconds(), 0.25);
}

test "calendar" {
    @expect_eq(days_from_civil(1970, 1, 1), 0);
    @expect_eq(days_from_civil(2000, 3, 1), 11017);
    @expect_eq(days_from_civil(1969, 12, 31), -1);
    let c = civil_from_days(-719468);
    @expect_eq(c.year, 0);
    @expect_eq(c.month, 3);
    @expect(is_leap_year(2000) and is_leap_year(2024) and not is_leap_year(1900));
    @expect_eq(days_in_month(2024, 2), 29);
    // every day for a few centuries round-trips
    let mut d: i64 = -100000;
    while d < 100000 {
        let cv = civil_from_days(d);
        @expect_eq(days_from_civil(cv.year, cv.month, cv.day), d);
        d += 1;
    }
}

test "times and dates" {
    let t = Time.from_unix(1234567890, 0);
    let dt = t.utc();
    @expect_eq(dt.year, 2009);
    @expect_eq(dt.month, 2);
    @expect_eq(dt.day, 13);
    @expect_eq(dt.hour, 23);
    @expect_eq(dt.minute, 31);
    @expect_eq(dt.second, 30);
    @expect_eq(dt.weekday, 5);
    @expect_eq(dt.yearday, 44);
    @expect_eq(dt.to_time()!.unix(), 1234567890);
    let mut b: [40]u8 = [0; 40];
    @expect_eq(t.format_rfc3339(b[..]), "2009-02-13T23:31:30Z");
    @expect_eq(t.add(Duration.milliseconds(120)).format_rfc3339(b[..]), "2009-02-13T23:31:30.12Z");
    @expect_eq(t.format_rfc3339_offset(b[..], -(5 * 3600) - 1800), "2009-02-13T18:01:30-05:30");
    @expect_eq(Time.from_unix(784111777, 0).format_http(b[..]), "Sun, 06 Nov 1994 08:49:37 GMT");
    let before = Time.from_unix(-1, 500000000);
    @expect_eq(before.format_rfc3339(b[..]), "1969-12-31T23:59:59.5Z");
    @expect_eq(before.utc().weekday, 3);
    @expect_eq(Time.from_unix_millis(-1).nsec, 999000000);
    @expect_eq(Time.from_unix_millis(-1).sec, -1);
    @expect_eq(t.sub(before).as_millis(), 1234567890500);
    @expect(before.before(t) and t.after(before));
    _ = date(2023, 2, 29, 0, 0, 0) catch return;
    @trap();
}

test "parsing" {
    @expect_eq(parse_rfc3339("2009-02-13T23:31:30Z")!.unix(), 1234567890);
    let t = parse_rfc3339("2009-02-14t01:31:30.25+02:00")!;
    @expect_eq(t.unix(), 1234567890);
    @expect_eq(t.nsec, 250000000);
    @expect_eq(parse_rfc3339("1994-11-06 08:49:37.123456789123-00:00")!.nsec, 123456789);
    @expect_eq(parse_http("Sun, 06 Nov 1994 08:49:37 GMT")!.unix(), 784111777);
    let bad: [6][]u8 = ["2009-02-13T23:31:30", "2009-13-01T00:00:00Z", "2009-02-13T24:00:00Z", "2009-02-13T23:31:30.Z",
                        "09-02-13T23:31:30Z", "2009-02-13T23:31:30Zx"];
    for s in bad {
        _ = parse_rfc3339(s) catch continue;
        @trap();
    }
}

test "clocks" {
    let a = Instant.now();
    let t = now();
    @expect(t.unix() > 1600000000);
    sleep(Duration.milliseconds(2));
    let d = a.elapsed();
    @expect(d.ns >= 2000000);
    @expect(Instant.now().since(a).ns >= d.ns);
}

test "far times saturate instead of trapping" {
    let far = parse_rfc3339("9999-12-31T23:59:59Z")!;
    let old = parse_rfc3339("0001-01-01T00:00:00Z")!;
    @expect_eq(far.unix_nanos(), @max_value(i64));
    @expect_eq(old.unix_nanos(), @min_value(i64));
    @expect_eq(far.sub(old).ns, @max_value(i64));
    @expect_eq(old.sub(far).ns, @min_value(i64));
}
