/* __axc_asctime - asctime (C17 7.27.3.1), which ctime calls too (asctime
   is deprecated in C23: axc/text.h): the algorithm's format, "%.3s %.3s%3d
   %.2d:%.2d:%.2d %d\n", "Sun Sep 16 01:03:52 1973\n". Its result is a
   static buffer large enough for any int values of the fields (where the
   standard's 26 bytes overflow, undefined behavior): a name out of range
   is "???". __axc_asctime_to writes it into a buffer of the caller's
   (Annex K's asctime_s). */
#include <axc/text.h>
#include <time.h>

static char *put_name(char *p, const char *names, int i, int count) {
    int k;
    for (k = 0; k < 3; k++)
        *p++ = i >= 0 && i < count ? names[3 * i + k] : '?';
    return p;
}

/* v in decimal: as %*d (width, pad ' ') or %.*d (digits, pad '0') */
static char *put_number(char *p, int v, int width, char pad) {
    char d[12];
    int n = 0;
    unsigned int u = v < 0 ? 0u - (unsigned int)v : (unsigned int)v;
    do {
        d[n++] = (char)('0' + u % 10);
        u /= 10;
    } while (u);
    if (v < 0 && pad == ' ')
        d[n++] = '-';
    if (v < 0 && pad == '0')
        *p++ = '-'; /* (%.2d: two digits after the sign) */
    while (n < width--)
        *p++ = pad;
    while (n > 0)
        *p++ = d[--n];
    return p;
}

/* Writes the conversion into result (64 bytes are enough); returns result */
char *__axc_asctime_to(char *result, const struct tm *timeptr) {
    char *p = result;
    p = put_name(p, "SunMonTueWedThuFriSat", timeptr->tm_wday, 7);
    *p++ = ' ';
    p = put_name(p, "JanFebMarAprMayJunJulAugSepOctNovDec", timeptr->tm_mon, 12);
    p = put_number(p, timeptr->tm_mday, 3, ' ');
    *p++ = ' ';
    p = put_number(p, timeptr->tm_hour, 2, '0');
    *p++ = ':';
    p = put_number(p, timeptr->tm_min, 2, '0');
    *p++ = ':';
    p = put_number(p, timeptr->tm_sec, 2, '0');
    *p++ = ' ';
    /* (1900 + tm_year, in the width of long long: it may not fit an int) */
    {
        long long y = 1900LL + timeptr->tm_year;
        char d[24];
        int n = 0;
        unsigned long long u = y < 0 ? 0 - (unsigned long long)y : (unsigned long long)y;
        do {
            d[n++] = (char)('0' + u % 10);
            u /= 10;
        } while (u);
        if (y < 0)
            *p++ = '-';
        while (n > 0)
            *p++ = d[--n];
    }
    *p++ = '\n';
    *p = 0;
    return result;
}

char *__axc_asctime(const struct tm *timeptr) {
    static char result[64];
    return __axc_asctime_to(result, timeptr);
}
