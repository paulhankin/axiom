// =============================================================================
// std/time/cron.ph - cron schedules: when a crontab line next fires.
//
//     let s = cron.parse("*/15 9-17 * * MON-FRI")!;     // or "@daily", ...
//     let t = s.next(time.now())!;                      // the next firing, UTC
//     let p = s.prev(time.now())!;                      // the last one
//     let mut z = tz.Zone.load("Europe/Paris")!;
//     let local = s.next_in(&z, time.now())!;           // in Paris time
//
// The syntax is Vixie cron's (crontab(5)): five fields, minute (0-59), hour
// (0-23), day of month (1-31), month (1-12 or JAN-DEC) and day of week (0-7
// or SUN-SAT, 0 and 7 both Sunday), or six with a seconds field (0-59)
// first. A field is a comma-separated list of items: `*`, a value `5`, a
// range `1-5`, each optionally with a step (`*/15`, `1-30/2`, and `5/15`
// meaning `5-59/15`). Names are case-insensitive, `?` is read as `*`, and
// the macros @yearly (@annually), @monthly, @weekly, @daily (@midnight) and
// @hourly stand for their five-field forms. Two extensions: `L` in the day
// of month is its last day, and `5L` in the day of week the last Friday of
// the month.
//
// Day of month and day of week combine as in Vixie cron: if either field
// starts with `*` (or `?`), a day must match both (so `* * * * MON` is
// every Monday, and `0 0 */2 * MON` odd-numbered Mondays); otherwise a day
// matching either one will do (`0 0 13 * FRI` is every 13th and every
// Friday).
//
// next and prev work in UTC; next_in and prev_in in a time zone
// (std/time/tz), where daylight saving time changes need rules (these are
// Vixie cron's, roughly):
//
//   - a local time skipped when clocks go forward fires once, at the
//     instant of the change (a job at 02:30 runs at 03:00 on the day the
//     clocks go from 02:00 to 03:00; jobs every minute from 02:00 to 02:59
//     run together then);
//   - a local time that occurs twice when clocks go back fires only the
//     first time, unless the schedule runs every hour (its hour field covers
//     all 24 hours), when it fires both times: a job at 01:30 runs once,
//     a job every 15 minutes keeps running every 15 minutes.
//
// Searches look up to 400 years ahead (or back); a schedule that never
// fires within that ("0 0 30 2 *") fails with NoMatch.
// =============================================================================

import "../time.ph" as time;
import "tz.ph" as tz;

error { InvalidCron, NoMatch }

const SEARCH_YEARS: i32 = 401;

/// A parsed cron expression.
pub struct Schedule {
    seconds: u64,        // bit s for each second that matches (only bit 0 without a seconds field)
    minutes: u64,        // bits 0 to 59
    hours: u32,          // bits 0 to 23
    days: u32,           // bits 1 to 31 (day of month)
    months: u32,         // bits 1 to 12
    weekdays: u32,       // bits 0 to 6 (Sunday is 0)
    last_weekdays: u32,  // `5L`: bit 5 for the last Friday of the month
    last_day: bool,      // `L` in the day of month
    days_star: bool,     // the day of month field starts with '*'
    weekdays_star: bool,

    /// Whether the calendar fields of `dt` (seconds included) match.
    pub fn matches_fields(self: *Schedule, dt: time.DateTime) bool {
        return bit(self.months, @int_cast(i32, dt.month)) and
               self.day_ok(dt.year, @int_cast(i32, dt.month), @int_cast(i32, dt.day)) and
               bit(self.hours, @int_cast(i32, dt.hour)) and bit64(self.minutes, @int_cast(i32, dt.minute)) and
               bit64(self.seconds, @int_cast(i32, dt.second));
    }

    /// Whether the schedule fires at t (UTC; fractions of a second ignored).
    pub fn matches(self: *Schedule, t: time.Time) bool {
        return self.matches_fields(t.utc());
    }

    fn day_ok(self: *Schedule, year: i64, month: i32, day: i32) bool {
        let dim = @int_cast(i32, time.days_in_month(year, @int_cast(u32, month)));
        let days = time.days_from_civil(year, @int_cast(u32, month), @int_cast(u32, day));
        let wd = @int_cast(i32, (days + 4) - (floor_div(days + 4, 7) * 7));
        let dom = bit(self.days, day) or (self.last_day and (day == dim));
        let dow = bit(self.weekdays, wd) or (bit(self.last_weekdays, wd) and ((day + 7) > dim));
        if self.days_star or self.weekdays_star {
            return dom and dow;
        }
        return dom or dow;
    }

    /// The first time after `after` that the schedule fires, in UTC.
    pub fn next(self: *Schedule, after: time.Time) !time.Time {
        return time.Time.from_unix(self.next_wall(after.unix())!, 0);
    }

    /// The last time before `before` that the schedule fires, in UTC.
    pub fn prev(self: *Schedule, before: time.Time) !time.Time {
        return time.Time.from_unix(self.prev_wall(ceil_sec(before))!, 0);
    }

    // Whether every hour matches (the rule for repeated local times).
    fn hourly(self: *Schedule) bool {
        return self.hours == 0xFFFFFF;
    }

    // The least matching time, in seconds of calendar time (the epoch's
    // seconds read as UTC), greater than w.
    fn next_wall(self: *Schedule, w: i64) !i64 {
        let dt = time.Time.from_unix(w + 1, 0).utc();
        let mut year = dt.year;
        let mut month = @int_cast(i32, dt.month);
        let mut day = @int_cast(i32, dt.day);
        let mut hour = @int_cast(i32, dt.hour);
        let mut minute = @int_cast(i32, dt.minute);
        let mut second = @int_cast(i32, dt.second);
        let limit = year + @as(i64, SEARCH_YEARS);
        loop {
            if year > limit {
                return error.NoMatch;
            }
            let m = next_bit(self.months, month, 12);
            if m < 0 {
                year += 1;
                month = 1;
                day = 1;
                hour = 0;
                minute = 0;
                second = 0;
                continue;
            }
            if m != month {
                month = m;
                day = 1;
                hour = 0;
                minute = 0;
                second = 0;
            }
            if day > @int_cast(i32, time.days_in_month(year, @int_cast(u32, month))) {
                month += 1;
                day = 1;
                hour = 0;
                minute = 0;
                second = 0;
                continue;
            }
            if not self.day_ok(year, month, day) {
                day += 1;
                hour = 0;
                minute = 0;
                second = 0;
                continue;
            }
            let h = next_bit(self.hours, hour, 23);
            if h < 0 {
                day += 1;
                hour = 0;
                minute = 0;
                second = 0;
                continue;
            }
            if h != hour {
                hour = h;
                minute = 0;
                second = 0;
            }
            let mi = next_bit64(self.minutes, minute, 59);
            if mi < 0 {
                hour += 1;
                minute = 0;
                second = 0;
                continue;
            }
            if mi != minute {
                minute = mi;
                second = 0;
            }
            let s = next_bit64(self.seconds, second, 59);
            if s < 0 {
                minute += 1;
                second = 0;
                continue;
            }
            return wall(year, month, day, hour, minute, s);
        }
    }

    // The greatest matching time (as for next_wall) less than w.
    fn prev_wall(self: *Schedule, w: i64) !i64 {
        let dt = time.Time.from_unix(w - 1, 0).utc();
        let mut year = dt.year;
        let mut month = @int_cast(i32, dt.month);
        let mut day = @int_cast(i32, dt.day);
        let mut hour = @int_cast(i32, dt.hour);
        let mut minute = @int_cast(i32, dt.minute);
        let mut second = @int_cast(i32, dt.second);
        let limit = year - @as(i64, SEARCH_YEARS);
        loop {
            if year < limit {
                return error.NoMatch;
            }
            let m = prev_bit(self.months, month);
            if m < 0 {
                year -= 1;
                month = 12;
                day = 31;
                hour = 23;
                minute = 59;
                second = 59;
                continue;
            }
            if m != month {
                month = m;
                day = 31;
                hour = 23;
                minute = 59;
                second = 59;
            }
            if day < 1 {
                month -= 1;
                day = 31;
                hour = 23;
                minute = 59;
                second = 59;
                continue;
            }
            day = @min(day, @int_cast(i32, time.days_in_month(year, @int_cast(u32, month))));
            if not self.day_ok(year, month, day) {
                day -= 1;
                hour = 23;
                minute = 59;
                second = 59;
                continue;
            }
            let h = prev_bit(self.hours, hour);
            if h < 0 {
                day -= 1;
                hour = 23;
                minute = 59;
                second = 59;
                continue;
            }
            if h != hour {
                hour = h;
                minute = 59;
                second = 59;
            }
            let mi = prev_bit64(self.minutes, minute);
            if mi < 0 {
                hour -= 1;
                minute = 59;
                second = 59;
                continue;
            }
            if mi != minute {
                minute = mi;
                second = 59;
            }
            let s = prev_bit64(self.seconds, second);
            if s < 0 {
                minute -= 1;
                second = 59;
                continue;
            }
            return wall(year, month, day, hour, minute, s);
        }
    }

    /// The first time after `after` that the schedule fires in the zone's
    /// local time (see the module comment for daylight saving time).
    pub fn next_in(self: *Schedule, z: *tz.Zone, after: time.Time) !time.Time {
        let a = after.unix();
        let w = a + @as(i64, z.lookup(a).offset);
        let r = resolve(z, w);
        if self.hourly() and (r.kind == AMBIGUOUS) and (r.first == a) {
            // in the first pass through a repeated hour: its second pass
            // comes before the times after it
            let c = self.next_wall(w)!;
            if c < r.end {
                return time.Time.from_unix(c - @as(i64, r.off_before), 0);
            }
            let c2 = self.next_wall(r.start - 1)!;
            if c2 < r.end {
                return time.Time.from_unix(c2 - @as(i64, r.off_after), 0);
            }
        }
        let mut lt = w;
        loop {
            let c = self.next_wall(lt)!;
            let rc = resolve(z, c);
            if rc.kind == AMBIGUOUS {
                if rc.first > a {
                    return time.Time.from_unix(rc.first, 0);
                }
                if self.hourly() and (rc.second > a) {
                    return time.Time.from_unix(rc.second, 0);
                }
            } else if rc.first > a {
                return time.Time.from_unix(rc.first, 0);
            }
            lt = c;
        }
    }

    /// The last time before `before` that the schedule fires in the zone's
    /// local time.
    pub fn prev_in(self: *Schedule, z: *tz.Zone, before: time.Time) !time.Time {
        let b = ceil_sec(before);  // fire times t < before are those with t < b
        let w = b + @as(i64, z.lookup(b).offset);
        let r = resolve(z, w);
        if (r.kind == AMBIGUOUS) and (r.second == b) {
            // in the second pass through a repeated hour: the first pass
            // (all of it) comes after the times before it
            if self.hourly() {
                let c = self.prev_wall(w)!;
                if c >= r.start {
                    return time.Time.from_unix(c - @as(i64, r.off_after), 0);
                }
            }
            let c2 = self.prev_wall(r.end)!;
            if c2 >= r.start {
                return time.Time.from_unix(c2 - @as(i64, r.off_before), 0);
            }
        }
        let mut lt = w;
        loop {
            let c = self.prev_wall(lt)!;
            let rc = resolve(z, c);
            if rc.kind == AMBIGUOUS {
                if self.hourly() and (rc.second < b) {
                    return time.Time.from_unix(rc.second, 0);
                }
                if rc.first < b {
                    return time.Time.from_unix(rc.first, 0);
                }
            } else if rc.first < b {
                return time.Time.from_unix(rc.first, 0);
            }
            lt = c;
        }
    }
}

fn bit(set: u32, i: i32) bool {
    return (i >= 0) and (i < 32) and (((set >> @int_cast(u32, i)) & 1) != 0);
}

fn bit64(set: u64, i: i32) bool {
    return (i >= 0) and (i < 64) and (((set >> @int_cast(u32, i)) & 1) != 0);
}

// The least i >= from (and <= max) in set, or -1.
fn next_bit(set: u32, from: i32, max: i32) i32 {
    return next_bit64(@as(u64, set), from, max);
}

fn next_bit64(set: u64, from: i32, max: i32) i32 {
    if from > max {
        return -1;
    }
    let rest = set >> @int_cast(u32, from);
    if rest == 0 {
        return -1;
    }
    let i = from + @int_cast(i32, @ctz(rest));
    if i > max {
        return -1;
    }
    return i;
}

// The greatest i <= from in set, or -1.
fn prev_bit(set: u32, from: i32) i32 {
    return prev_bit64(@as(u64, set), from);
}

fn prev_bit64(set: u64, from: i32) i32 {
    if from < 0 {
        return -1;
    }
    let below = set & (@max_value(u64) >> @int_cast(u32, 63 - @min(from, 63)));
    if below == 0 {
        return -1;
    }
    return 63 - @int_cast(i32, @clz(below));
}

fn floor_div(a: i64, b: i64) i64 {
    let q = a / b;
    if ((a % b) != 0) and ((a < 0) != (b < 0)) {
        return q - 1;
    }
    return q;
}

fn wall(year: i64, month: i32, day: i32, hour: i32, minute: i32, second: i32) i64 {
    let days = time.days_from_civil(year, @int_cast(u32, month), @int_cast(u32, day));
    return (((days * 86400) + (@as(i64, hour) * 3600)) + (@as(i64, minute) * 60)) + @as(i64, second);
}

// The whole second at or after t.
fn ceil_sec(t: time.Time) i64 {
    if t.nsec > 0 {
        return t.unix() + 1;
    }
    return t.unix();
}

// -----------------------------------------------------------------------------
// Local times in a zone
// -----------------------------------------------------------------------------

const UNIQUE: u8 = 0;
const SKIPPED: u8 = 1;
const AMBIGUOUS: u8 = 2;

// The instants a local time names.
struct Local {
    kind: u8,
    first: i64,   // UNIQUE: the instant; SKIPPED: the instant of the change; AMBIGUOUS: the first occurrence
    second: i64,  // AMBIGUOUS: the second occurrence
    // AMBIGUOUS: the repeated local times [start, end), and the offsets
    // before and after the change
    start: i64,
    end: i64,
    off_before: i32,
    off_after: i32,
}

fn resolve(z: *tz.Zone, w: i64) Local {
    let mut r = Local.{ kind: UNIQUE, first: 0, second: 0, start: 0, end: 0, off_before: 0, off_after: 0 };
    let dt = time.Time.from_unix(w, 0).utc();
    let f0 = (z.from_local(dt, 0) catch @trap()).unix();  // (dt is valid)
    let f1 = (z.from_local(dt, 1) catch @trap()).unix();
    if f0 == f1 {
        r.first = f0;
        return r;
    }
    if f0 < f1 {
        r.kind = AMBIGUOUS;
        r.first = f0;
        r.second = f1;
        let t = change(z, f0, f1);
        r.off_before = z.lookup(f0).offset;
        r.off_after = z.lookup(t).offset;
        r.start = t + @as(i64, r.off_after);
        r.end = t + @as(i64, r.off_before);
        return r;
    }
    // skipped: fold 1 is before the change and fold 0 after it
    r.kind = SKIPPED;
    r.first = change(z, f1, f0);
    return r;
}

// The first instant in (a, b] whose offset differs from a's (there is one).
fn change(z: *tz.Zone, a: i64, b: i64) i64 {
    let off = z.lookup(a).offset;
    let mut lo = a;
    let mut hi = b;
    while (hi - lo) > 1 {
        let mid = lo + ((hi - lo) / 2);
        if z.lookup(mid).offset == off {
            lo = mid;
        } else {
            hi = mid;
        }
    }
    return hi;
}

// -----------------------------------------------------------------------------
// Parsing
// -----------------------------------------------------------------------------

let MONTH_NAMES: [12][]u8 = ["jan", "feb", "mar", "apr", "may", "jun", "jul", "aug", "sep", "oct", "nov", "dec"];
let DAY_NAMES: [7][]u8 = ["sun", "mon", "tue", "wed", "thu", "fri", "sat"];

const F_SECOND: u32 = 0;
const F_MINUTE: u32 = 1;
const F_HOUR: u32 = 2;
const F_DAY: u32 = 3;
const F_MONTH: u32 = 4;
const F_WEEKDAY: u32 = 5;

// One field's matches.
struct Field {
    set: u64,
    last: u64,  // `L` (day of month: bit 0) or `nL` (day of week: bit n)
    star: bool,
}

fn is_space(c: u8) bool {
    return (c == ' ') or (c == '\t');
}

fn lower(c: u8) u8 {
    if (c >= 'A') and (c <= 'Z') {
        return c + 32;
    }
    return c;
}

// A number or (for months and weekdays) a name at s[i.*..].
fn value(s: []u8, i: *mut usize, kind: u32) !i32 {
    let start = i.*;
    if (start < s.len) and (s[start] >= '0') and (s[start] <= '9') {
        let mut v: i32 = 0;
        while (i.* < s.len) and (s[i.*] >= '0') and (s[i.*] <= '9') {
            v = (v * 10) + @as(i32, s[i.*] - '0');
            if v > 1000 {
                return error.InvalidCron;
            }
            i.* += 1;
        }
        return v;
    }
    if ((kind != F_MONTH) and (kind != F_WEEKDAY)) or ((start + 3) > s.len) {
        return error.InvalidCron;
    }
    let w: [3]u8 = [lower(s[start]), lower(s[start + 1]), lower(s[start + 2])];
    i.* += 3;
    if kind == F_MONTH {
        for k in @as(usize, 0)..12 {
            if @mem_eq(w[..], MONTH_NAMES[k]) {
                return @int_cast(i32, k) + 1;
            }
        }
    } else {
        for k in @as(usize, 0)..7 {
            if @mem_eq(w[..], DAY_NAMES[k]) {
                return @int_cast(i32, k);
            }
        }
    }
    return error.InvalidCron;
}

fn parse_field(s: []u8, kind: u32) !Field {
    let mut lo: i32 = 0;
    let mut hi: i32 = 59;
    if kind == F_HOUR {
        hi = 23;
    } else if kind == F_DAY {
        lo = 1;
        hi = 31;
    } else if kind == F_MONTH {
        lo = 1;
        hi = 12;
    } else if kind == F_WEEKDAY {
        hi = 7;
    }
    let mut f = Field.{ set: 0, last: 0, star: (s.len > 0) and ((s[0] == '*') or (s[0] == '?')) };
    let mut i: usize = 0;
    loop {
        // one item, up to ',' or the end
        if i >= s.len {
            return error.InvalidCron;
        }
        let mut a = lo;
        let mut b = hi;
        let mut step: i32 = 1;
        if (s[i] == '*') or (s[i] == '?') {
            i += 1;
        } else if (kind == F_DAY) and (s[i] == 'L') {
            i += 1;
            f.last |= 1;
            if (i < s.len) and (s[i] != ',') {
                return error.InvalidCron;
            }
            if i == s.len {
                break;
            }
            i += 1;
            continue;
        } else {
            a = value(s, &mut i, kind)!;
            b = a;
            if (i < s.len) and (s[i] == '-') {
                i += 1;
                b = value(s, &mut i, kind)!;
            } else if (i < s.len) and (s[i] == '/') {
                b = hi;  // `a/n` is `a-max/n`
            } else if (kind == F_WEEKDAY) and (i < s.len) and (s[i] == 'L') {
                i += 1;
                if (a < 0) or (a > 7) {
                    return error.InvalidCron;
                }
                f.last |= @as(u64, 1) << @int_cast(u32, a % 7);
                if (i < s.len) and (s[i] != ',') {
                    return error.InvalidCron;
                }
                if i == s.len {
                    break;
                }
                i += 1;
                continue;
            }
        }
        if (i < s.len) and (s[i] == '/') {
            i += 1;
            if (i >= s.len) or (s[i] < '0') or (s[i] > '9') {
                return error.InvalidCron;
            }
            step = value(s, &mut i, F_SECOND)!;
        }
        if (a < lo) or (b > hi) or (a > b) or (step < 1) {
            return error.InvalidCron;
        }
        let mut v = a;
        while v <= b {
            f.set |= @as(u64, 1) << @int_cast(u32, v % 64);
            v += step;
        }
        if i == s.len {
            break;
        }
        if s[i] != ',' {
            return error.InvalidCron;
        }
        i += 1;
    }
    if kind == F_WEEKDAY {
        // 7 is Sunday too
        if (f.set & 0x80) != 0 {
            f.set = (f.set & 0x7f) | 1;
        }
    }
    return f;
}

/// Parses a cron expression: five fields (minute hour day-of-month month
/// day-of-week), six with seconds first, or a macro such as "@daily".
/// InvalidCron if it is not one.
pub fn parse(expr: []u8) !Schedule {
    let mut s = expr;
    while (s.len > 0) and is_space(s[0]) {
        s = s[1..];
    }
    while (s.len > 0) and is_space(s[s.len - 1]) {
        s = s[..s.len - 1];
    }
    if (s.len > 0) and (s[0] == '@') {
        let names: [7][]u8 = ["@yearly", "@annually", "@monthly", "@weekly", "@daily", "@midnight", "@hourly"];
        let exprs: [7][]u8 = ["0 0 1 1 *", "0 0 1 1 *", "0 0 1 * *", "0 0 * * 0", "0 0 * * *", "0 0 * * *",
                              "0 * * * *"];
        for k in @as(usize, 0)..7 {
            if s.len == names[k].len {
                let mut same = true;
                for j in @as(usize, 0)..s.len {
                    if lower(s[j]) != names[k][j] {
                        same = false;
                    }
                }
                if same {
                    return parse(exprs[k]);
                }
            }
        }
        return error.InvalidCron;
    }
    let mut fields: [6][]u8 = [""; 6];
    let mut n: usize = 0;
    let mut i: usize = 0;
    while i < s.len {
        if is_space(s[i]) {
            i += 1;
            continue;
        }
        let start = i;
        while (i < s.len) and not is_space(s[i]) {
            i += 1;
        }
        if n == 6 {
            return error.InvalidCron;
        }
        fields[n] = s[start..i];
        n += 1;
    }
    if (n != 5) and (n != 6) {
        return error.InvalidCron;
    }
    let mut sec = Field.{ set: 1, last: 0, star: false };
    let mut k: usize = 0;
    if n == 6 {
        sec = parse_field(fields[0], F_SECOND)!;
        k = 1;
    }
    let minute = parse_field(fields[k], F_MINUTE)!;
    let hour = parse_field(fields[k + 1], F_HOUR)!;
    let day = parse_field(fields[k + 2], F_DAY)!;
    let month = parse_field(fields[k + 3], F_MONTH)!;
    let weekday = parse_field(fields[k + 4], F_WEEKDAY)!;
    return Schedule.{
        seconds: sec.set,
        minutes: minute.set,
        hours: @truncate(u32, hour.set),
        days: @truncate(u32, day.set),
        months: @truncate(u32, month.set),
        weekdays: @truncate(u32, weekday.set),
        last_weekdays: @truncate(u32, weekday.last),
        last_day: day.last != 0,
        days_star: day.star,
        weekdays_star: weekday.star,
    };
}

// -----------------------------------------------------------------------------
// Tests (phalange test std/time/cron.ph)
// -----------------------------------------------------------------------------

import "../mem.ph" as mem;
import "../os/fs.ph" as fs;

fn at(s: []u8) time.Time {
    return time.parse_rfc3339(s) catch @trap();
}

// The next n firings after `from`, as RFC 3339 UTC, separated by spaces.
fn run(expr: []u8, from: []u8, n: u32, buf: []mut u8) []u8 {
    let s = parse(expr) catch @trap();
    let mut t = at(from);
    let mut len: usize = 0;
    let mut tb: [40]u8 = [0; 40];
    for k in @as(u32, 0)..n {
        t = s.next(t) catch @trap();
        let f = t.format_rfc3339(tb[..]);
        if k > 0 {
            buf[len] = ' ';
            len += 1;
        }
        @mem_copy(buf[len..len + f.len], f);
        len += f.len;
    }
    return buf[..len];
}

// The same backwards with prev.
fn run_back(expr: []u8, from: []u8, n: u32, buf: []mut u8) []u8 {
    let s = parse(expr) catch @trap();
    let mut t = at(from);
    let mut len: usize = 0;
    let mut tb: [40]u8 = [0; 40];
    for k in @as(u32, 0)..n {
        t = s.prev(t) catch @trap();
        let f = t.format_rfc3339(tb[..]);
        if k > 0 {
            buf[len] = ' ';
            len += 1;
        }
        @mem_copy(buf[len..len + f.len], f);
        len += f.len;
    }
    return buf[..len];
}

fn invalid(r: !Schedule) bool {
    _ = r catch |e| return e == error.InvalidCron;
    return false;
}

test "fields, ranges, steps and names" {
    let mut b: [400]u8 = [0; 400];
    // 2026-10-01 is a Thursday
    @expect_eq(run("*/15 9-10 * * *", "2026-10-01T10:20:00Z", 4, b[..]),
               "2026-10-01T10:30:00Z 2026-10-01T10:45:00Z 2026-10-02T09:00:00Z 2026-10-02T09:15:00Z");
    @expect_eq(run("0 12 * * MON-FRI", "2026-10-02T12:00:00Z", 2, b[..]), "2026-10-05T12:00:00Z 2026-10-06T12:00:00Z");
    @expect_eq(run("30 4 1,15 * *", "2026-10-01T05:00:00Z", 3, b[..]),
               "2026-10-15T04:30:00Z 2026-11-01T04:30:00Z 2026-11-15T04:30:00Z");
    @expect_eq(run("0 0 1 jan,Jul *", "2026-10-01T00:00:00Z", 2, b[..]), "2027-01-01T00:00:00Z 2027-07-01T00:00:00Z");
    @expect_eq(run("5/20 * * * *", "2026-10-01T00:00:00Z", 4, b[..]),
               "2026-10-01T00:05:00Z 2026-10-01T00:25:00Z 2026-10-01T00:45:00Z 2026-10-01T01:05:00Z");
    @expect_eq(run("0 0 * * 7", "2026-10-01T00:00:00Z", 1, b[..]), "2026-10-04T00:00:00Z");  // Sunday
    @expect_eq(run("0 0 * * 5-7", "2026-10-03T00:00:00Z", 3, b[..]),
               "2026-10-04T00:00:00Z 2026-10-09T00:00:00Z 2026-10-10T00:00:00Z");
    @expect_eq(run("0 1-10/3 * * *", "2026-10-01T00:00:00Z", 4, b[..]),
               "2026-10-01T01:00:00Z 2026-10-01T04:00:00Z 2026-10-01T07:00:00Z 2026-10-01T10:00:00Z");
    @expect_eq(run("0 1-10/3 * * *", "2026-10-01T10:00:00Z", 1, b[..]), "2026-10-02T01:00:00Z");
    // seconds; fractions of a second
    @expect_eq(run("*/20 * * * * *", "2026-10-01T23:59:30.5Z", 3, b[..]),
               "2026-10-01T23:59:40Z 2026-10-02T00:00:00Z 2026-10-02T00:00:20Z");
    @expect_eq(run("0 * * * *", "2026-10-01T23:59:59.999Z", 1, b[..]), "2026-10-02T00:00:00Z");
    // the year rolls over
    @expect_eq(run("59 23 31 12 *", "2026-12-31T23:59:00Z", 1, b[..]), "2027-12-31T23:59:00Z");
    // tabs and runs of spaces, ? for *
    @expect_eq(run(" 0\t0  ? * ? ", "2026-10-01T00:00:00Z", 1, b[..]), "2026-10-02T00:00:00Z");
}

test "day of month and day of week" {
    let mut b: [400]u8 = [0; 400];
    // both restricted: either will do (13th, or a Friday); Friday 2026-11-13 once
    @expect_eq(run("0 0 13 * FRI", "2026-10-01T00:00:00Z", 3, b[..]),
               "2026-10-02T00:00:00Z 2026-10-09T00:00:00Z 2026-10-13T00:00:00Z");
    @expect_eq(run("0 0 13 * FRI", "2026-10-13T00:00:00Z", 3, b[..]),
               "2026-10-16T00:00:00Z 2026-10-23T00:00:00Z 2026-10-30T00:00:00Z");
    @expect_eq(run("0 0 13 11 FRI", "2026-11-01T00:00:00Z", 3, b[..]),
               "2026-11-06T00:00:00Z 2026-11-13T00:00:00Z 2026-11-20T00:00:00Z");
    // a field starting with * means both must match: odd days that are Mondays
    @expect_eq(run("0 0 */2 * MON", "2026-10-01T00:00:00Z", 3, b[..]),
               "2026-10-05T00:00:00Z 2026-10-19T00:00:00Z 2026-11-09T00:00:00Z");
    @expect_eq(run("0 0 1-7 * */7", "2026-10-01T00:00:00Z", 1, b[..]), "2026-10-04T00:00:00Z");
    // L: the last day of the month (leap years), and the last Friday
    @expect_eq(run("0 0 L * *", "2027-12-31T00:00:00Z", 4, b[..]),
               "2028-01-31T00:00:00Z 2028-02-29T00:00:00Z 2028-03-31T00:00:00Z 2028-04-30T00:00:00Z");
    @expect_eq(run("0 0 L 2 *", "2100-01-01T00:00:00Z", 1, b[..]), "2100-02-28T00:00:00Z");
    @expect_eq(run("0 18 * * 5L", "2026-10-01T00:00:00Z", 3, b[..]),
               "2026-10-30T18:00:00Z 2026-11-27T18:00:00Z 2026-12-25T18:00:00Z");
}

test "leap years and macros" {
    let mut b: [400]u8 = [0; 400];
    @expect_eq(run("0 0 29 2 *", "2025-01-01T00:00:00Z", 3, b[..]),
               "2028-02-29T00:00:00Z 2032-02-29T00:00:00Z 2036-02-29T00:00:00Z");
    // 2100 is not a leap year, 2400 is
    @expect_eq(run("0 0 29 2 *", "2096-03-01T00:00:00Z", 1, b[..]), "2104-02-29T00:00:00Z");
    @expect_eq(run_back("0 0 29 2 *", "2104-01-01T00:00:00Z", 2, b[..]), "2096-02-29T00:00:00Z 2092-02-29T00:00:00Z");
    @expect_eq(run("@yearly", "2026-10-01T00:00:00Z", 1, b[..]), "2027-01-01T00:00:00Z");
    @expect_eq(run("@ANNUALLY", "2026-10-01T00:00:00Z", 1, b[..]), "2027-01-01T00:00:00Z");
    @expect_eq(run("@monthly", "2026-10-01T00:00:00Z", 1, b[..]), "2026-11-01T00:00:00Z");
    @expect_eq(run("@weekly", "2026-10-01T00:00:00Z", 1, b[..]), "2026-10-04T00:00:00Z");
    @expect_eq(run("@daily", "2026-10-01T00:00:00Z", 1, b[..]), "2026-10-02T00:00:00Z");
    @expect_eq(run("@midnight", "2026-10-01T00:00:00Z", 1, b[..]), "2026-10-02T00:00:00Z");
    @expect_eq(run("@hourly", "2026-10-01T00:59:59Z", 2, b[..]), "2026-10-01T01:00:00Z 2026-10-01T02:00:00Z");
    // never
    let feb30 = parse("0 0 30 2 *")!;
    _ = feb30.next(at("2026-01-01T00:00:00Z")) catch |e| {
        @expect_eq(e, error.NoMatch);
        _ = feb30.prev(at("2026-01-01T00:00:00Z")) catch |e2| {
            @expect_eq(e2, error.NoMatch);
            return;
        };
        @trap();
    };
    @trap();
}

test "prev" {
    let mut b: [400]u8 = [0; 400];
    @expect_eq(run_back("*/15 9-10 * * *", "2026-10-02T09:00:00Z", 3, b[..]),
               "2026-10-01T10:45:00Z 2026-10-01T10:30:00Z 2026-10-01T10:15:00Z");
    @expect_eq(run_back("0 0 1 1 *", "2026-01-01T00:00:00.001Z", 2, b[..]),
               "2026-01-01T00:00:00Z 2025-01-01T00:00:00Z");
    @expect_eq(run_back("0 0 L * *", "2028-03-01T00:00:00Z", 2, b[..]), "2028-02-29T00:00:00Z 2028-01-31T00:00:00Z");
    @expect_eq(run_back("30 * * * * *", "2026-01-01T00:00:00Z", 2, b[..]), "2025-12-31T23:59:30Z 2025-12-31T23:58:30Z");
    @expect_eq(run_back("0 12 13 * FRI", "2026-10-13T12:00:00Z", 2, b[..]),
               "2026-10-09T12:00:00Z 2026-10-02T12:00:00Z");
    // next and prev agree on a sample of schedules
    let exprs: [6][]u8 = ["*/7 */5 * * *", "0 0 13 * FRI", "15 3 L * *", "0 0 * * 1L", "1-5/2 0 1 1-3 *",
                          "*/13 * * * * *"];
    for e in exprs {
        let s = parse(e)!;
        let mut t = at("2027-12-30T21:03:00Z");
        for k in @as(u32, 0)..30 {
            let n = s.next(t)!;
            @expect(s.matches(n) and n.after(t));
            @expect_eq(s.prev(n)!.compare(t) <= 0, true);
            @expect_eq(s.next(s.prev(n)!)!.unix(), n.unix());
            t = n;
        }
    }
}

test "invalid expressions" {
    let bad: [20][]u8 = [
        "",
        "* * * *",
        "* * * * * * *",
        "60 * * * *",
        "* 24 * * *",
        "* * 0 * *",
        "* * 32 * *",
        "* * * 13 *",
        "* * * * 8",
        "5-1 * * * *",
        "*/0 * * * *",
        "a * * * *",
        "1,,2 * * * *",
        "1, * * * *",
        "* * * JANUARY *",
        "@reboot",
        "@every 5m",
        "1- * * * *",
        "* * * * MON-",
        "* * 5L * *",
    ];
    for s in bad {
        @expect(invalid(parse(s)));
    }
    @expect(not invalid(parse("0 0 * * 5L,1")));
}

fn load_zone(name: []u8) tz.Zone {
    let data = fs.read_file(name) catch @trap();
    defer mem.free(data);
    return tz.Zone.parse_tzif(data) catch @trap();
}

// The next (or, with back, previous) n firings in z after `from`, as RFC 3339 UTC.
fn run_in(z: *tz.Zone, expr: []u8, from: []u8, n: u32, back: bool, buf: []mut u8) []u8 {
    let s = parse(expr) catch @trap();
    let mut t = at(from);
    let mut len: usize = 0;
    let mut tb: [40]u8 = [0; 40];
    for k in @as(u32, 0)..n {
        if back {
            t = s.prev_in(z, t) catch @trap();
        } else {
            t = s.next_in(z, t) catch @trap();
        }
        let f = t.format_rfc3339(tb[..]);
        if k > 0 {
            buf[len] = ' ';
            len += 1;
        }
        @mem_copy(buf[len..len + f.len], f);
        len += f.len;
    }
    return buf[..len];
}

test "daylight saving time in New York" {
    if not fs.exists("std/time/tz/testdata/America-New_York.tzif") {
        return;
    }
    let mut z = load_zone("std/time/tz/testdata/America-New_York.tzif");
    defer z.deinit();
    let mut b: [400]u8 = [0; 400];
    // 2026-03-08: 02:00 EST becomes 03:00 EDT (07:00Z). 02:30 is skipped and
    // runs at the change; the day before and after at 02:30 local
    @expect_eq(run_in(&z, "30 2 * * *", "2026-03-07T00:00:00Z", 3, false, b[..]),
               "2026-03-07T07:30:00Z 2026-03-08T07:00:00Z 2026-03-09T06:30:00Z");
    @expect_eq(run_in(&z, "30 2 * * *", "2026-03-09T06:30:00Z", 3, true, b[..]),
               "2026-03-08T07:00:00Z 2026-03-07T07:30:00Z 2026-03-06T07:30:00Z");
    // every 20 minutes: the skipped hour's times all run at the change
    @expect_eq(run_in(&z, "*/20 * * * *", "2026-03-08T06:30:00Z", 4, false, b[..]),
               "2026-03-08T06:40:00Z 2026-03-08T07:00:00Z 2026-03-08T07:20:00Z 2026-03-08T07:40:00Z");
    @expect_eq(run_in(&z, "*/20 * * * *", "2026-03-08T07:20:00Z", 3, true, b[..]),
               "2026-03-08T07:00:00Z 2026-03-08T06:40:00Z 2026-03-08T06:20:00Z");
    // 2026-11-01: 02:00 EDT becomes 01:00 EST (06:00Z). 01:30 runs once, at
    // its first occurrence (EDT)
    @expect_eq(run_in(&z, "30 1 * * *", "2026-10-31T12:00:00Z", 3, false, b[..]),
               "2026-11-01T05:30:00Z 2026-11-02T06:30:00Z 2026-11-03T06:30:00Z");
    @expect_eq(run_in(&z, "30 1 * * *", "2026-11-02T06:30:00Z", 2, true, b[..]),
               "2026-11-01T05:30:00Z 2026-10-31T05:30:00Z");
    // ... and from inside the repeated hour, in either pass
    @expect_eq(run_in(&z, "30 1 * * *", "2026-11-01T06:10:00Z", 1, false, b[..]), "2026-11-02T06:30:00Z");
    @expect_eq(run_in(&z, "30 1 * * *", "2026-11-01T06:10:00Z", 1, true, b[..]), "2026-11-01T05:30:00Z");
    // an hourly schedule runs in both passes
    @expect_eq(run_in(&z, "*/30 * * * *", "2026-11-01T04:45:00Z", 3, false, b[..]),
               "2026-11-01T05:00:00Z 2026-11-01T05:30:00Z 2026-11-01T06:00:00Z");
    @expect_eq(run_in(&z, "*/30 * * * *", "2026-11-01T06:00:00Z", 2, false, b[..]),
               "2026-11-01T06:30:00Z 2026-11-01T07:00:00Z");
    @expect_eq(run_in(&z, "*/30 * * * *", "2026-11-01T07:00:00Z", 3, true, b[..]),
               "2026-11-01T06:30:00Z 2026-11-01T06:00:00Z 2026-11-01T05:30:00Z");
    @expect_eq(run_in(&z, "*/30 * * * *", "2026-11-01T05:30:00Z", 2, true, b[..]),
               "2026-11-01T05:00:00Z 2026-11-01T04:30:00Z");
    @expect_eq(run_in(&z, "*/30 * * * *", "2026-11-01T05:10:00Z", 2, false, b[..]),
               "2026-11-01T05:30:00Z 2026-11-01T06:00:00Z");
    @expect_eq(run_in(&z, "*/30 * * * *", "2026-11-01T06:10:00Z", 2, true, b[..]),
               "2026-11-01T06:00:00Z 2026-11-01T05:30:00Z");
    // with an hour field, it does not
    @expect_eq(run_in(&z, "*/30 1 * * *", "2026-11-01T04:45:00Z", 3, false, b[..]),
               "2026-11-01T05:00:00Z 2026-11-01T05:30:00Z 2026-11-02T06:00:00Z");
    // daily at midnight, across both changes
    @expect_eq(run_in(&z, "@daily", "2026-03-07T12:00:00Z", 2, false, b[..]),
               "2026-03-08T05:00:00Z 2026-03-09T04:00:00Z");
}

test "other zones" {
    if not fs.exists("std/time/tz/testdata/Europe-London.tzif") {
        return;
    }
    let mut lon = load_zone("std/time/tz/testdata/Europe-London.tzif");
    defer lon.deinit();
    let mut b: [400]u8 = [0; 400];
    // 2026-03-29 01:00 GMT -> 02:00 BST; 2026-10-25 02:00 BST -> 01:00 GMT
    @expect_eq(run_in(&lon, "15 1 * * SUN", "2026-03-22T12:00:00Z", 2, false, b[..]),
               "2026-03-29T01:00:00Z 2026-04-05T00:15:00Z");
    @expect_eq(run_in(&lon, "15 1 * * SUN", "2026-10-24T12:00:00Z", 2, false, b[..]),
               "2026-10-25T00:15:00Z 2026-11-01T01:15:00Z");
    // Lord Howe Island moves its clocks by half an hour
    let mut lh = load_zone("std/time/tz/testdata/Australia-Lord_Howe.tzif");
    defer lh.deinit();
    // 2026-10-04 02:00 +1030 -> 02:30 +11 (2026-10-03T15:30Z): 02:00 and 02:20
    // are skipped and run at the change
    @expect_eq(run_in(&lh, "*/20 2 * * *", "2026-10-03T15:00:00Z", 3, false, b[..]),
               "2026-10-03T15:30:00Z 2026-10-03T15:40:00Z 2026-10-04T15:00:00Z");
    // UTC as a zone is the same as next and prev
    let mut u = tz.Zone.utc();
    defer u.deinit();
    let s = parse("*/7 3-5 * * *")!;
    let mut t = at("2026-10-01T00:00:00Z");
    for k in @as(u32, 0)..50 {
        let n = s.next_in(&u, t)!;
        @expect_eq(n.unix(), s.next(t)!.unix());
        @expect_eq(s.prev_in(&u, n)!.unix(), s.prev(n)!.unix());
        t = n;
    }
}
