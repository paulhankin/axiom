/* strftime (C17 7.27.3.5): the conversions are __axc_strftime_one's; a %
   not followed by a conversion specifier (undefined behavior) is copied. */
#include <axc/text.h>
#include <time.h>

size_t strftime(char *restrict s, size_t maxsize, const char *restrict format, const struct tm *restrict timeptr) {
    size_t k = 0;
    char buf[128];
    for (; *format; format++) {
        const char *piece = format;
        int len = 1;
        if (*format == '%') {
            int m = format[1] == 'E' || format[1] == 'O' ? format[1] : 0;
            int c = m ? format[2] : format[1];
            int r = c ? __axc_strftime_one(buf, sizeof buf, c, m, timeptr) : -1;
            if (r >= 0) {
                piece = buf;
                len = r;
                format += m ? 2 : 1;
            }
        }
        /* (with room for the null character) */
        if ((size_t)len >= maxsize - k)
            return 0;
        while (len-- > 0)
            s[k++] = *piece++;
    }
    if (k >= maxsize)
        return 0;
    s[k] = 0;
    return k;
}
