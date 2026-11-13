/* sprintf_s, vsprintf_s (C11 K.3.5.3.6, .11): sprintf into s, an array of
   n, whose output (with its null character) must fit: the result is its
   length. Runtime-constraints: s and format not null, n not 0 and at most
   RSIZE_MAX, the output fitting in n characters, fprintf_s's on the
   format and its arguments; after a violation s[0] is the null character
   (s not null, n 1 to RSIZE_MAX) and the result 0; an encoding error
   (%lc, %ls) gives a negative result. */
#define __STDC_WANT_LIB_EXT1__ 1
#include <axc/ext1_impl.h>
#include <stdio.h>
#include <stdarg.h>

int __axc_vsnprintf_x(char *s, size_t n, const char *fmt, va_list ap, const char **viol);

static int vs(const char *fn, char *s, rsize_t n, const char *format, va_list arg) {
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
    else {
        r = __axc_vsnprintf_x(s, n, format, arg, &viol);
        if (viol == NULL && r >= 0 && (size_t)r >= n)
            viol = "the output does not fit in n characters", e = ERANGE;
    }
    if (viol == NULL)
        return r;
    if (s != NULL && n > 0 && n <= RSIZE_MAX)
        s[0] = 0;
    __axc_ext1_violation(fn, viol, e);
    return 0;
}

int vsprintf_s(char *__AXC_RESTRICT s, rsize_t n, const char *__AXC_RESTRICT format, va_list arg) {
    return vs("vsprintf_s", s, n, format, arg);
}

int sprintf_s(char *__AXC_RESTRICT s, rsize_t n, const char *__AXC_RESTRICT format, ...) {
    va_list ap;
    int r;
    va_start(ap, format);
    r = vs("sprintf_s", s, n, format, ap);
    va_end(ap);
    return r;
}
