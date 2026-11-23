/* fwprintf_s, wprintf_s, vfwprintf_s, vwprintf_s (C11 K.3.9.1.1, .13,
   .5, .9): fwprintf's output, with the runtime-constraints: stream and format
   not null, no %n in the format, no null pointer for %s (or %ls). After a
   violation the output stops (what it had written stays written) and the
   result is negative. */
#define __STDC_WANT_LIB_EXT1__ 1
#include <axc/ext1_impl.h>
#include <stdio.h>
#include <wchar.h>
#include <stdarg.h>

int __axc_vfwprintf_x(FILE *f, const wchar_t *fmt, va_list ap, const char **viol);

static int vf(const char *fn, FILE *stream, const wchar_t *format, va_list arg) {
    const char *viol = NULL;
    int r = -1;
    if (stream == NULL)
        viol = "stream is a null pointer";
    else if (format == NULL)
        viol = "format is a null pointer";
    else
        r = __axc_vfwprintf_x(stream, format, arg, &viol);
    if (viol == NULL)
        return r;
    __axc_ext1_violation(fn, viol, EINVAL);
    return -1;
}

int vfwprintf_s(FILE *__AXC_RESTRICT stream, const wchar_t *__AXC_RESTRICT format, va_list arg) {
    return vf("vfwprintf_s", stream, format, arg);
}

int vwprintf_s(const wchar_t *__AXC_RESTRICT format, va_list arg) {
    return vf("vwprintf_s", stdout, format, arg);
}

int fwprintf_s(FILE *__AXC_RESTRICT stream, const wchar_t *__AXC_RESTRICT format, ...) {
    va_list ap;
    int r;
    va_start(ap, format);
    r = vf("fwprintf_s", stream, format, ap);
    va_end(ap);
    return r;
}

int wprintf_s(const wchar_t *__AXC_RESTRICT format, ...) {
    va_list ap;
    int r;
    va_start(ap, format);
    r = vf("wprintf_s", stdout, format, ap);
    va_end(ap);
    return r;
}
