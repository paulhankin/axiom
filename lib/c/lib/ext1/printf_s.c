/* fprintf_s, printf_s, vfprintf_s, vprintf_s (C11 K.3.5.3.1, .3, .8,
   .10): fprintf's output, with the runtime-constraints: stream and format
   not null, no %n in the format, no null pointer for %s (or %ls). After a
   violation the output stops (what it had written stays written) and the
   result is negative. */
#define __STDC_WANT_LIB_EXT1__ 1
#include <axc/ext1_impl.h>
#include <stdio.h>
#include <stdarg.h>

int __axc_vfprintf_x(FILE *f, const char *fmt, va_list ap, const char **viol);

static int vf(const char *fn, FILE *stream, const char *format, va_list arg) {
    const char *viol = NULL;
    int r = -1;
    if (stream == NULL)
        viol = "stream is a null pointer";
    else if (format == NULL)
        viol = "format is a null pointer";
    else
        r = __axc_vfprintf_x(stream, format, arg, &viol);
    if (viol == NULL)
        return r;
    __axc_ext1_violation(fn, viol, EINVAL);
    return -1;
}

int vfprintf_s(FILE *__AXC_RESTRICT stream, const char *__AXC_RESTRICT format, va_list arg) {
    return vf("vfprintf_s", stream, format, arg);
}

int vprintf_s(const char *__AXC_RESTRICT format, va_list arg) {
    return vf("vprintf_s", stdout, format, arg);
}

int fprintf_s(FILE *__AXC_RESTRICT stream, const char *__AXC_RESTRICT format, ...) {
    va_list ap;
    int r;
    va_start(ap, format);
    r = vf("fprintf_s", stream, format, ap);
    va_end(ap);
    return r;
}

int printf_s(const char *__AXC_RESTRICT format, ...) {
    va_list ap;
    int r;
    va_start(ap, format);
    r = vf("printf_s", stdout, format, ap);
    va_end(ap);
    return r;
}
