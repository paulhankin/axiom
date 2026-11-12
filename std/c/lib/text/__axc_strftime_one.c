/* __axc_strftime_one - one conversion of strftime and wcsftime (C17
   7.27.3.5, C23 7.29.3.5), in the "C" locale, which every locale's LC_TIME
   is (7.27.3.5p7: its names, "%a %b %e %T %Y" for %c, "%m/%d/%y" for %x,
   "%T" for %X, "AM" and "PM" for %p, "%I:%M:%S %p" for %r). The E and O
   modifiers ask for the locale's alternative forms, which "C" does not
   have: the conversion is the plain one; a modifier on another specifier
   is not a conversion. A field out of its range (undefined behavior) is
   converted all the same, a name as "?", and nothing is read outside the
   tables.

   Years: %Y is the year in full, with a '-' before a year before 1 (year
   0 is 1 BCE); %C is the year divided by 100, truncated (so 0 from -99 to
   99), with at least 2 digits; %y is the last two digits of the year
   (of its absolute value); %G and %g likewise for the ISO 8601 week-based
   year (__axc_iso_week). The time zone of %z and %Z is the local zone's
   at the time tm names read as a local time with its tm_isdst; with
   tm_isdst negative there is none, and they give nothing. */
#include <axc/text.h>
#include <time.h>

static const char *const days[7] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
static const char *const months[12] = {"January", "February", "March",     "April",   "May",      "June",
                                       "July",    "August",   "September", "October", "November", "December"};

/* the decimal digits of v, at least width of them (zeros or spaces
   before), into s: their number */
static int number(char *s, long long v, int width, char pad) {
    char d[24];
    int n = 0, k = 0;
    unsigned long long u = v < 0 ? 0 - (unsigned long long)v : (unsigned long long)v;
    do {
        d[n++] = (char)('0' + u % 10);
        u /= 10;
    } while (u);
    if (v < 0) {
        if (pad == '0')
            s[k++] = '-';
        else
            d[n++] = '-';
        width--;
    }
    while (n < width--)
        s[k++] = pad;
    while (n > 0)
        s[k++] = d[--n];
    return k;
}

static int text(char *s, const char *t, int len) {
    int k = 0;
    while (t[k] && k != len) {
        s[k] = t[k];
        k++;
    }
    return k;
}

static int name(char *s, const char *const *names, int count, int i, int len) {
    if (i < 0 || i >= count)
        return text(s, "?", -1);
    return text(s, names[i], len);
}

/* the conversion of a format of plain specifiers (%c, %D, ...) */
static int composite(char *s, size_t n, const char *f, const struct tm *tm) {
    int k = 0;
    for (; *f; f++) {
        if (*f == '%') {
            f++;
            k += __axc_strftime_one(s + k, n - (size_t)k, *f, 0, tm);
        } else {
            s[k++] = *f;
        }
    }
    return k;
}

/* the last two digits of a year */
static long long last2(long long y) {
    return (y < 0 ? -y : y) % 100;
}

/* the local zone's offset and abbreviation at the time tm names: 0 if
   there is none (tm_isdst < 0) */
static int zone(const struct tm *tm, long long *off, char *abbr, size_t cap) {
    long long dst;
    if (tm->tm_isdst < 0)
        return 0;
    __axc_tz_info(__axc_tz_from_local(__axc_tm_to_seconds(tm), tm->tm_isdst), off, &dst, abbr, cap);
    return 1;
}

int __axc_strftime_one(char *s, size_t n, int c, int m, const struct tm *tm) {
    int w;
    long long y = (long long)tm->tm_year + 1900;
    if (m == 'E' && c != 'c' && c != 'C' && c != 'x' && c != 'X' && c != 'y' && c != 'Y')
        return -1;
    if (m == 'O' && c != 'b' && c != 'B' && c != 'd' && c != 'e' && c != 'H' && c != 'I' && c != 'm' && c != 'M' &&
        c != 'S' && c != 'u' && c != 'U' && c != 'V' && c != 'w' && c != 'W' && c != 'y')
        return -1;
    switch (c) {
    case 'a':
        return name(s, days, 7, tm->tm_wday, 3);
    case 'A':
        return name(s, days, 7, tm->tm_wday, -1);
    case 'b':
    case 'h':
        return name(s, months, 12, tm->tm_mon, 3);
    case 'B':
        return name(s, months, 12, tm->tm_mon, -1);
    case 'c':
        return composite(s, n, "%a %b %e %H:%M:%S %Y", tm);
    case 'C':
        return number(s, y / 100, 2, '0');
    case 'd':
        return number(s, tm->tm_mday, 2, '0');
    case 'D':
        return composite(s, n, "%m/%d/%y", tm);
    case 'e':
        return number(s, tm->tm_mday, 2, ' ');
    case 'F':
        return composite(s, n, "%Y-%m-%d", tm);
    case 'g':
        return number(s, last2(__axc_iso_week(tm, &w)), 2, '0');
    case 'G':
        return number(s, __axc_iso_week(tm, &w), 1, '0');
    case 'H':
        return number(s, tm->tm_hour, 2, '0');
    case 'I':
        return number(s, tm->tm_hour % 12 == 0 ? 12 : tm->tm_hour % 12, 2, '0');
    case 'j':
        return number(s, (long long)tm->tm_yday + 1, 3, '0');
    case 'm':
        return number(s, (long long)tm->tm_mon + 1, 2, '0');
    case 'M':
        return number(s, tm->tm_min, 2, '0');
    case 'n':
        return text(s, "\n", -1);
    case 'p':
        return text(s, tm->tm_hour < 12 ? "AM" : "PM", -1);
    case 'r':
        return composite(s, n, "%I:%M:%S %p", tm);
    case 'R':
        return composite(s, n, "%H:%M", tm);
    case 'S':
        return number(s, tm->tm_sec, 2, '0');
    case 't':
        return text(s, "\t", -1);
    case 'T':
    case 'X':
        return composite(s, n, "%H:%M:%S", tm);
    case 'u':
        return number(s, tm->tm_wday == 0 ? 7 : tm->tm_wday, 1, '0');
    case 'U':
        /* (weeks from the first Sunday: week 1 begins on it) */
        return number(s, ((long long)tm->tm_yday + 7 - tm->tm_wday) / 7, 2, '0');
    case 'V':
        __axc_iso_week(tm, &w);
        return number(s, w, 2, '0');
    case 'w':
        return number(s, tm->tm_wday, 1, '0');
    case 'W':
        /* (weeks from the first Monday) */
        return number(s, ((long long)tm->tm_yday + 7 - (tm->tm_wday + 6) % 7) / 7, 2, '0');
    case 'x':
        return composite(s, n, "%m/%d/%y", tm);
    case 'y':
        return number(s, last2(y), 2, '0');
    case 'Y':
        return number(s, y, 1, '0');
    case 'z': {
        long long off;
        int k;
        char abbr[1];
        if (!zone(tm, &off, abbr, 0))
            return 0;
        s[0] = off < 0 ? '-' : '+';
        if (off < 0)
            off = -off;
        k = 1 + number(s + 1, off / 3600, 2, '0');
        return k + number(s + k, off / 60 % 60, 2, '0');
    }
    case 'Z': {
        long long off;
        char abbr[32];
        if (!zone(tm, &off, abbr, sizeof abbr))
            return 0;
        return text(s, abbr, -1);
    }
    case '%':
        return text(s, "%", -1);
    default:
        return -1;
    }
}
