/* snwprintf_s, vsnwprintf_s (C11 K.3.9.1.3, .8): snprintf's rules for wide
   output into s, an array of n wide characters: at most n - 1 and a null
   wide character; the result is the
   length of the whole output (it was all written if that is less than n).
   Runtime-constraints: s and format not null, n not 0 and at most
   RSIZE_MAX, fwprintf_s's on the format and its arguments; after a
   violation s[0] is the null character (s not null, n 1 to RSIZE_MAX) and
   the result negative. */
#define __STDC_WANT_LIB_EXT1__ 1
#include <axc/ext1_impl.h>
#include <stdio.h>
#include <wchar.h>
#include <stdarg.h>

int __axc_vswprintf_x(wchar_t *s, size_t n, const wchar_t *fmt, va_list ap, const char **viol);

static int vsn(const char *fn, wchar_t *s, rsize_t n, const wchar_t *format, va_list arg) {
    const char *viol = NULL;
    errno_t e = EINVAL;
    int r = -1;
    if (s == NULL)
        viol = "s is a null pointer";
    else if (format == NULL)
        viol = "format is a null pointer";
    else if (n > RSIZE_MAX)
        viol = "n is greater than RSIZE_MAX", e = ERANGE;
    else if (n == 0)
        viol = "n is zero", e = ERANGE;
    else
        r = __axc_vswprintf_x(s, n, format, arg, &viol);
    if (viol == NULL)
        return r;
    if (s != NULL && n > 0 && n <= RSIZE_MAX)
        s[0] = 0;
    __axc_ext1_violation(fn, viol, e);
    return -1;
}

int vsnwprintf_s(wchar_t *__AXC_RESTRICT s, rsize_t n, const wchar_t *__AXC_RESTRICT format, va_list arg) {
    return vsn("vsnwprintf_s", s, n, format, arg);
}

int snwprintf_s(wchar_t *__AXC_RESTRICT s, rsize_t n, const wchar_t *__AXC_RESTRICT format, ...) {
    va_list ap;
    int r;
    va_start(ap, format);
    r = vsn("snwprintf_s", s, n, format, ap);
    va_end(ap);
    return r;
}
