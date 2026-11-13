/* swprintf_s, vswprintf_s (C11 K.3.9.1.4, .10): swprintf into s, an array
   of n wide characters, whose output (with its null wide character) must
   fit: the result is its length. Runtime-constraints: s and format not null, n not 0 and at most
   RSIZE_MAX, the output fitting in n characters, fwprintf_s's on the
   format and its arguments; after a violation s[0] is the null character
   (s not null, n 1 to RSIZE_MAX) and the result 0, but negative for an
   output that does not fit (K.3.9.1.4p4) and for an encoding error. */
#define __STDC_WANT_LIB_EXT1__ 1
#include <axc/ext1_impl.h>
#include <stdio.h>
#include <wchar.h>
#include <stdarg.h>

int __axc_vswprintf_x(wchar_t *s, size_t n, const wchar_t *fmt, va_list ap, const char **viol);

static int vs(const char *fn, wchar_t *s, rsize_t n, const wchar_t *format, va_list arg) {
    const char *viol = NULL;
    errno_t e = EINVAL;
    int r = 0;
    if (s == NULL)
        viol = "s is a null pointer";
    else if (format == NULL)
        viol = "format is a null pointer";
    else if (n > RSIZE_MAX)
        viol = "n is greater than RSIZE_MAX", e = ERANGE;
    else if (n == 0)
        viol = "n is zero", e = ERANGE;
    else {
        r = __axc_vswprintf_x(s, n, format, arg, &viol);
        if (viol != NULL)
            r = 0;
        if (viol == NULL && r >= 0 && (size_t)r >= n)
            viol = "the output does not fit in n wide characters", e = ERANGE, r = -1;
    }
    if (viol == NULL)
        return r;
    if (s != NULL && n > 0 && n <= RSIZE_MAX)
        s[0] = 0;
    __axc_ext1_violation(fn, viol, e);
    return r < 0 ? -1 : 0;
}

int vswprintf_s(wchar_t *__AXC_RESTRICT s, rsize_t n, const wchar_t *__AXC_RESTRICT format, va_list arg) {
    return vs("vswprintf_s", s, n, format, arg);
}

int swprintf_s(wchar_t *__AXC_RESTRICT s, rsize_t n, const wchar_t *__AXC_RESTRICT format, ...) {
    va_list ap;
    int r;
    va_start(ap, format);
    r = vs("swprintf_s", s, n, format, ap);
    va_end(ap);
    return r;
}
