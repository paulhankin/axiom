/* snprintf_s, vsnprintf_s (C11 K.3.5.3.5, .9): snprintf into s, an array
   of n: at most n - 1 characters and a null character; the result is the
   length of the whole output (it was all written if that is less than n).
   Runtime-constraints: s and format not null, n not 0 and at most
   RSIZE_MAX, fprintf_s's on the format and its arguments; after a
   violation s[0] is the null character (s not null, n 1 to RSIZE_MAX) and
   the result negative. */
#define __STDC_WANT_LIB_EXT1__ 1
#include <axc/ext1_impl.h>
#include <stdio.h>
#include <stdarg.h>

int __axc_vsnprintf_x(char *s, size_t n, const char *fmt, va_list ap, const char **viol);

static int vsn(const char *fn, char *s, rsize_t n, const char *format, va_list arg) {
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
        r = __axc_vsnprintf_x(s, n, format, arg, &viol);
    if (viol == NULL)
        return r;
    if (s != NULL && n > 0 && n <= RSIZE_MAX)
        s[0] = 0;
    __axc_ext1_violation(fn, viol, e);
    return -1;
}

int vsnprintf_s(char *__AXC_RESTRICT s, rsize_t n, const char *__AXC_RESTRICT format, va_list arg) {
    return vsn("vsnprintf_s", s, n, format, arg);
}

int snprintf_s(char *__AXC_RESTRICT s, rsize_t n, const char *__AXC_RESTRICT format, ...) {
    va_list ap;
    int r;
    va_start(ap, format);
    r = vsn("snprintf_s", s, n, format, ap);
    va_end(ap);
    return r;
}
