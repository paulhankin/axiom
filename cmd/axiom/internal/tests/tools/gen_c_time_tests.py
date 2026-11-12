#!/usr/bin/env python3
"""Writes the C library's tests of <time.h>'s calendar (gmtime, timegm,
strftime, asctime) over centuries of dates:

    python3 cmd/axiom/internal/tests/tools/gen_c_time_tests.py [--check]

writes cmd/axiom/internal/tests/c/lib/text/time_calendar.c, a program with
a table of instants and what each must give: the broken-down time, every
strftime conversion of the "C" locale (but %z and %Z, which are the local
zone's) and asctime's line; and of broken-down times with fields out of
their ranges and the instant timegm normalizes them to. The years are
those around the leap-year rules' edges (every 4, not 100, every 400),
negative years, year 0, the ISO 8601 week-based years' first and last days
for 1990-2040 and a few far ones, and the dates at the limits of what an
int tm_year holds.

The expected values are computed here from the calendar's rules (the
proleptic Gregorian calendar, C17 7.27.3.5's definitions of the
conversions), independently of the library's algorithms. --check also
compares them, for the years 1 to 9999, with Python's datetime.
"""
import os
import sys

OUT = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', 'c', 'lib', 'text', 'time_calendar.c')

DAYS = ['Sunday', 'Monday', 'Tuesday', 'Wednesday', 'Thursday', 'Friday', 'Saturday']
MONTHS = ['January', 'February', 'March', 'April', 'May', 'June', 'July', 'August', 'September', 'October',
          'November', 'December']
INT_MIN, INT_MAX = -2 ** 31, 2 ** 31 - 1


def leap(y):
    return y % 4 == 0 and (y % 100 != 0 or y % 400 == 0)


def month_days(y, m):
    return [31, 29 if leap(y) else 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31][m - 1]


def days_before_year(y):
    """Days from 0001-01-01 to y-01-01 (negative before), counting the leap
    days of the years 1 .. y-1 (floor division makes it hold before 1)."""
    p = y - 1
    return 365 * p + p // 4 - p // 100 + p // 400


EPOCH = days_before_year(1970)


def day_number(y, m, d):
    """Days since 1970-01-01."""
    return days_before_year(y) - EPOCH + sum(month_days(y, k) for k in range(1, m)) + d - 1


def civil(n):
    """The date of day n (since 1970-01-01): the year by a search on
    days_before_year, then the month by its lengths."""
    target = n + EPOCH
    y = 1970 + n // 366
    while days_before_year(y + 1) <= target:
        y += 1
    while days_before_year(y) > target:
        y -= 1
    left = target - days_before_year(y)
    m = 1
    while left >= month_days(y, m):
        left -= month_days(y, m)
        m += 1
    return y, m, left + 1


def broken_down(t):
    n, s = divmod(t, 86400)
    y, m, d = civil(n)
    return {
        'year': y, 'mon': m - 1, 'mday': d, 'hour': s // 3600, 'min': s // 60 % 60, 'sec': s % 60,
        'wday': (n + 4) % 7,  # 1970-01-01 was a Thursday
        'yday': n - day_number(y, 1, 1),
    }


def iso_week(tm):
    """ISO 8601: the week (Monday to Sunday) belongs to the year of its
    Thursday, and is numbered from the one with that year's first
    Thursday."""
    n = day_number(tm['year'], tm['mon'] + 1, tm['mday'])
    monday_based = (tm['wday'] + 6) % 7
    thursday = n - monday_based + 3
    ty, _, _ = civil(thursday)
    first_thursday = day_number(ty, 1, 1)
    while (first_thursday + 4) % 7 != 4:
        first_thursday += 1
    return ty, (thursday - first_thursday) // 7 + 1


def first_weekday(tm, wd):
    """%U (wd 0, Sunday) and %W (wd 1, Monday): weeks numbered from the
    year's first such day, the days before it in week 0."""
    jan1 = (tm['wday'] - tm['yday']) % 7
    first = (wd - jan1) % 7  # yday of the first such day
    return 0 if tm['yday'] < first else (tm['yday'] - first) // 7 + 1


def trunc_div(a, b):
    q = abs(a) // abs(b)
    return q if (a >= 0) == (b > 0) else -q


def num(v, width, pad='0'):
    s = str(abs(v))
    if v < 0:
        if pad == '0':
            return '-' + s.rjust(width - 1, '0')
        return ('-' + s).rjust(width, pad)
    return s.rjust(width, pad)


def conversions(tm):
    y = tm['year']
    gy, gw = iso_week(tm)
    hour12 = tm['hour'] % 12 or 12
    c = {
        'a': DAYS[tm['wday']][:3], 'A': DAYS[tm['wday']], 'b': MONTHS[tm['mon']][:3], 'B': MONTHS[tm['mon']],
        'C': num(trunc_div(y, 100), 2), 'd': num(tm['mday'], 2), 'e': num(tm['mday'], 2, ' '),
        'g': num(abs(gy) % 100, 2), 'G': str(gy), 'H': num(tm['hour'], 2), 'I': num(hour12, 2),
        'j': num(tm['yday'] + 1, 3), 'm': num(tm['mon'] + 1, 2), 'M': num(tm['min'], 2), 'n': '\n',
        'p': 'AM' if tm['hour'] < 12 else 'PM', 'S': num(tm['sec'], 2), 't': '\t',
        'u': str(tm['wday'] or 7), 'U': num(first_weekday(tm, 0), 2), 'V': num(gw, 2), 'w': str(tm['wday']),
        'W': num(first_weekday(tm, 1), 2), 'y': num(abs(y) % 100, 2), 'Y': str(y), '%': '%',
    }
    c['h'] = c['b']
    c['D'] = '%s/%s/%s' % (c['m'], c['d'], c['y'])
    c['F'] = '%s-%s-%s' % (c['Y'], c['m'], c['d'])
    c['T'] = '%s:%s:%s' % (c['H'], c['M'], c['S'])
    c['R'] = '%s:%s' % (c['H'], c['M'])
    c['r'] = '%s:%s:%s %s' % (c['I'], c['M'], c['S'], c['p'])
    c['c'] = '%s %s %s %s %s' % (c['a'], c['b'], c['e'], c['T'], c['Y'])
    c['x'] = c['D']
    c['X'] = c['T']
    # the modifiers: the "C" locale has no alternative forms
    for k in 'cCxXyY':
        c['E' + k] = c[k]
    for k in 'bBdeHImMSuUVwWy':
        c['O' + k] = c[k]
    return c


# the conversions in the order of the test's format (all but %z and %Z)
SPECS = ['a', 'A', 'b', 'B', 'c', 'C', 'd', 'D', 'e', 'F', 'g', 'G', 'h', 'H', 'I', 'j', 'm', 'M', 'n', 'p', 'r', 'R',
         'S', 't', 'T', 'u', 'U', 'V', 'w', 'W', 'x', 'X', 'y', 'Y', '%', 'Ec', 'EC', 'Ex', 'EX', 'Ey', 'EY', 'Od',
         'Oe', 'OH', 'OI', 'Om', 'OM', 'OS', 'Ou', 'OU', 'OV', 'Ow', 'OW', 'Oy', 'Ob', 'OB']
FORMAT = '|'.join('%' + s for s in SPECS)


def asctime(tm):
    return '%s %s%3d %02d:%02d:%02d %d\n' % (DAYS[tm['wday']][:3], MONTHS[tm['mon']][:3], tm['mday'], tm['hour'],
                                             tm['min'], tm['sec'], tm['year'])


def instants():
    ts = set()

    def add(y, m, d, s=0):
        ts.add(day_number(y, m, d) * 86400 + s)

    edges = [-2001, -2000, -1601, -1600, -401, -400, -399, -101, -100, -99, -5, -4, -1, 0, 1, 2, 3, 4, 99, 100, 101,
             399, 400, 401, 1582, 1599, 1600, 1601, 1699, 1700, 1701, 1799, 1800, 1801, 1899, 1900, 1901, 1969, 1970,
             1971, 1972, 1999, 2000, 2001, 2037, 2038, 2099, 2100, 2101, 2399, 2400, 2401, 9999, 10000, 12345, 100000,
             -100000, 1000000]
    for i, y in enumerate(edges):
        for (m, d) in [(1, 1), (2, 28), (3, 1), (12, 31)]:
            add(y, m, d, (i * 7919 + m * 613 + d * 37) % 86400)
        if leap(y):
            add(y, 2, 29, 43200)
    # the ISO week-based years' edges, 1990 to 2040, and some far ones
    for y in list(range(1990, 2041)) + [-1, 0, 1, 1600, 2400, 10000]:
        for d in range(26, 32):
            add(y, 12, d, (y * 31 + d * 3600) % 86400)
        for d in range(1, 5):
            add(y + 1, 1, d, (y * 17 + d * 7200 + 59) % 86400)
    # times of day: midnight, noon, the hours around them
    for s in [0, 1, 3599, 3600, 43199, 43200, 46800, 86399]:
        add(2024, 7, 4, s)
    # the first and last instants whose year tm_year holds
    lo = day_number(INT_MIN + 1900, 1, 1) * 86400
    hi = day_number(INT_MAX + 1900, 12, 31) * 86400 + 86399
    ts.update([lo, lo + 1, hi - 1, hi])
    return sorted(ts), lo, hi


def c_string(s):
    return '"' + s.replace('\\', '\\\\').replace('"', '\\"').replace('\n', '\\n').replace('\t', '\\t') + '"'


def normalizations():
    """Broken-down times with fields out of range: (fields, instant),
    the instant by carrying each field's excess into the next one (C17
    7.27.2.3p2): months into years, then days, hours, minutes and seconds
    counted from the month's first day."""
    cases = [
        (2000, 0, 1, 0, 0, 0), (2000, 12, 1, 0, 0, 0), (2000, -1, 1, 0, 0, 0), (2000, -13, 1, 0, 0, 0),
        (2000, 1, 30, 0, 0, 0), (1999, 1, 29, 0, 0, 0), (2000, 0, 0, 0, 0, 0), (2000, 0, -365, 0, 0, 0),
        (2000, 0, 1, 24, 0, 0), (2000, 0, 1, -1, 0, 0), (2000, 0, 1, 0, 60, 0), (2000, 0, 1, 0, 0, 60),
        (2000, 0, 1, 0, 0, -1), (2000, 0, 1, 0, 0, 86400 * 366), (1970, 0, 1, 0, 0, 2147483647),
        (1970, 0, 1, 0, 0, -2147483647 - 1), (2001, 1200, 1, 0, 0, 0), (2001, -2400, 1, 0, 0, 0),
        (2000, 2147483647, 1, 0, 0, 0), (2000, 0, 2147483647, 2147483647, 2147483647, 2147483647),
        (2000, 0, -2147483647 - 1, -2147483647 - 1, -2147483647 - 1, -2147483647 - 1), (-1900, 0, 1, 0, 0, 0),
        (1900, 1, 29, 0, 0, 0), (2100, 1, 29, 0, 0, 0), (2400, 1, 29, 12, 0, 0), (1972, 5, 30, 23, 59, 60),
    ]
    out = []
    for (y, mon, mday, hour, mn, sec) in cases:
        yy, mm = y + mon // 12, mon % 12
        t = (day_number(yy, mm + 1, 1) + mday - 1) * 86400 + hour * 3600 + mn * 60 + sec
        out.append(((y - 1900, mon, mday, hour, mn, sec), t))
    return out


def check():
    import datetime
    for y in range(1, 10000):
        for (m, d) in [(1, 1), (2, 28), (3, 1), (6, 15), (12, 28), (12, 31)]:
            n = day_number(y, m, d)
            dt = datetime.date(y, m, d)
            assert n == dt.toordinal() - datetime.date(1970, 1, 1).toordinal(), (y, m, d)
            assert civil(n) == (y, m, d)
            tm = broken_down(n * 86400)
            assert tm['wday'] == dt.isoweekday() % 7
            assert tm['yday'] == dt.timetuple().tm_yday - 1
            iy, iw, _ = dt.isocalendar()
            assert iso_week(tm) == (iy, iw), (y, m, d)
            assert first_weekday(tm, 0) == int(dt.strftime('%U')), (y, m, d)
            assert first_weekday(tm, 1) == int(dt.strftime('%W')), (y, m, d)
    print('the calendar agrees with datetime for the years 1 to 9999')


def main():
    if '--check' in sys.argv:
        check()
    ts, lo, hi = instants()
    rows = []
    for t in ts:
        tm = broken_down(t)
        c = conversions(tm)
        rows.append('    {%dLL, {%d, %d, %d, %d, %d, %d, %d, %d}, %s, %s},' % (
            t, tm['year'] - 1900, tm['mon'], tm['mday'], tm['hour'], tm['min'], tm['sec'], tm['wday'], tm['yday'],
            c_string('|'.join(c[s] for s in SPECS)), c_string(asctime(tm))))
    norm = ['    {{%d, %d, %d, %d, %d, %d}, %dLL},' % (f + (t,)) for (f, t) in normalizations()]
    text = r'''/* <time.h>'s calendar over centuries of dates: gmtime (and gmtime_r), the
   inverse timegm, strftime's conversions in the "C" locale (all but %%z and
   %%Z, the local zone's) and asctime, for each instant of the table; timegm
   of broken-down times with fields out of their ranges; gmtime's null
   pointer for the instants just beyond what tm_year holds; and, in
   whatever zone the system has, localtime and mktime as each other's
   inverse. Written by cmd/axiom/internal/tests/tools/gen_c_time_tests.py,
   whose calendar the expected values come from. */
/* flags: --c-std=c23 */
/* uses: standin/core.c standin/core.ax */
#include <errno.h>
#include <time.h>
#include "../../mix/print.h"
#include "standin/standin.h"

struct row {
    long long t;
    int f[8]; /* tm_year tm_mon tm_mday tm_hour tm_min tm_sec tm_wday tm_yday */
    const char *conversions;
    const char *asctime;
};

static const struct row rows[] = {
%s
};

struct norm {
    int f[6]; /* tm_year tm_mon tm_mday tm_hour tm_min tm_sec */
    long long t;
};

static const struct norm norms[] = {
%s
};

static long bad;

static void fail(const char *what, long long t) {
    if (++bad <= 10) {
        print_cstr(what);
        print_i64(t);
    }
}

int main(void) {
    char buf[1024];
    size_t i;
    long checked = 0;
    for (i = 0; i < sizeof rows / sizeof rows[0]; i++) {
        const struct row *r = &rows[i];
        time_t t = (time_t)r->t;
        struct tm tm, *p = gmtime(&t), copy;
        if (!p) {
            fail("gmtime", r->t);
            continue;
        }
        if (p->tm_year != r->f[0] || p->tm_mon != r->f[1] || p->tm_mday != r->f[2] || p->tm_hour != r->f[3] ||
            p->tm_min != r->f[4] || p->tm_sec != r->f[5] || p->tm_wday != r->f[6] || p->tm_yday != r->f[7] ||
            p->tm_isdst != 0)
            fail("fields", r->t);
        if (gmtime_r(&t, &tm) != &tm || memcmp(&tm, p, sizeof tm))
            fail("gmtime_r", r->t);
        copy = tm;
        if (timegm(&copy) != t || memcmp(&copy, &tm, sizeof tm))
            fail("timegm", r->t);
        if (strftime(buf, sizeof buf, "%s", &tm) != strlen(r->conversions) || strcmp(buf, r->conversions))
            fail("strftime", r->t);
        if (strcmp(asctime(&tm), r->asctime))
            fail("asctime", r->t);
        checked++;
    }
    print_i64(checked);
    for (i = 0; i < sizeof norms / sizeof norms[0]; i++) {
        const struct norm *n = &norms[i];
        struct tm tm = {0}, *g;
        time_t t;
        tm.tm_year = n->f[0];
        tm.tm_mon = n->f[1];
        tm.tm_mday = n->f[2];
        tm.tm_hour = n->f[3];
        tm.tm_min = n->f[4];
        tm.tm_sec = n->f[5];
        t = timegm(&tm);
        g = gmtime(&t);
        if (t != n->t || !g || memcmp(g, &tm, sizeof tm))
            fail("normalized", (long long)i);
    }
    print_i64((long long)(sizeof norms / sizeof norms[0]));
    {
        time_t below = (time_t)%dLL, above = (time_t)%dLL;
        struct tm tm;
        errno = 0;
        if (gmtime(&below) || errno != EOVERFLOW || gmtime_r(&above, &tm))
            fail("beyond tm_year", 0);
    }
    /* the local zone: localtime and mktime are inverses */
    for (i = 0; i < sizeof rows / sizeof rows[0]; i++) {
        time_t t = (time_t)rows[i].t, back;
        struct tm *p = localtime(&t), tm;
        if (!p)
            continue;
        tm = *p;
        back = mktime(&tm);
        if (back != t || memcmp(&tm, p, sizeof tm))
            fail("mktime", rows[i].t);
    }
    print_i64(bad);
    return 0;
    /* expect: %d */
    /* expect: %d */
    /* expect: 0 */
}
''' % ('\n'.join(rows), '\n'.join(norm), FORMAT, lo - 1, hi + 1, len(rows), len(norm))
    with open(OUT, 'w') as f:
        f.write(text)


main()
