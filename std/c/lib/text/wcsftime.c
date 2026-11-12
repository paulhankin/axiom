/* wcsftime (C17 7.29.5.1): strftime with wide characters; the conversions
   are __axc_strftime_one's, whose characters are all ASCII. */
#include <axc/text.h>
#include <time.h>
#include <wchar.h>

size_t wcsftime(wchar_t *restrict s, size_t maxsize, const wchar_t *restrict format,
                const struct tm *restrict timeptr) {
    size_t k = 0;
    char buf[128];
    for (; *format; format++) {
        int len = 1, r = -1, m = 0;
        if (*format == L'%') {
            wchar_t c;
            m = format[1] == L'E' || format[1] == L'O' ? (int)format[1] : 0;
            c = m ? format[2] : format[1];
            if (c > 0 && c < 0x80)
                r = __axc_strftime_one(buf, sizeof buf, (int)c, m, timeptr);
            if (r >= 0)
                len = r;
        }
        if ((size_t)len >= maxsize - k)
            return 0;
        if (r >= 0) {
            int i;
            for (i = 0; i < len; i++)
                s[k++] = (wchar_t)(unsigned char)buf[i];
            format += m ? 2 : 1;
        } else {
            s[k++] = *format;
        }
    }
    if (k >= maxsize)
        return 0;
    s[k] = 0;
    return k;
}
