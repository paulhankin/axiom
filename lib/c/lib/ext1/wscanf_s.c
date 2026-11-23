/* fwscanf_s, wscanf_s, swscanf_s, vfwscanf_s, vwscanf_s, vswscanf_s (C11
   K.3.9.1.2, .14, .6, .7, .12, .11): fwscanf's input, but c, s and [ take
   the receiving array's number of elements (an rsize_t) after its
   pointer, and an item that does not fit (with its null character) is a
   matching failure (the array then holds the empty string). Runtime-
   constraints: stream (s) and format not null, no null pointer to store
   through; after a violation the input stops and the result is EOF. The
   __axc_..._c23 forms are C23 mode's (<wchar.h> maps the names): %i takes
   the 0b prefix. */
#define __STDC_WANT_LIB_EXT1__ 1
#include <axc/ext1_impl.h>
#include <stdio.h>
#include <wchar.h>
#include <stdarg.h>

int __axc_vfwscanf_x(FILE *f, const wchar_t *fmt, va_list ap, int c23, const char **viol);
int __axc_vswscanf_x(const wchar_t *s, const wchar_t *fmt, va_list ap, int c23, const char **viol);

static int vf(const char *fn, FILE *stream, const wchar_t *format, va_list arg, int c23) {
    const char *viol = NULL;
    int r = EOF;
    if (stream == NULL)
        viol = "stream is a null pointer";
    else if (format == NULL)
        viol = "format is a null pointer";
    else
        r = __axc_vfwscanf_x(stream, format, arg, c23, &viol);
    if (viol == NULL)
        return r;
    __axc_ext1_violation(fn, viol, EINVAL);
    return EOF;
}

static int vs(const char *fn, const wchar_t *s, const wchar_t *format, va_list arg, int c23) {
    const char *viol = NULL;
    int r = EOF;
    if (s == NULL)
        viol = "s is a null pointer";
    else if (format == NULL)
        viol = "format is a null pointer";
    else
        r = __axc_vswscanf_x(s, format, arg, c23, &viol);
    if (viol == NULL)
        return r;
    __axc_ext1_violation(fn, viol, EINVAL);
    return EOF;
}

int vfwscanf_s(FILE *__AXC_RESTRICT stream, const wchar_t *__AXC_RESTRICT format, va_list arg) {
    return vf("vfwscanf_s", stream, format, arg, 0);
}

int vwscanf_s(const wchar_t *__AXC_RESTRICT format, va_list arg) {
    return vf("vwscanf_s", stdin, format, arg, 0);
}

int vswscanf_s(const wchar_t *__AXC_RESTRICT s, const wchar_t *__AXC_RESTRICT format, va_list arg) {
    return vs("vswscanf_s", s, format, arg, 0);
}

int fwscanf_s(FILE *__AXC_RESTRICT stream, const wchar_t *__AXC_RESTRICT format, ...) {
    va_list ap;
    int r;
    va_start(ap, format);
    r = vf("fwscanf_s", stream, format, ap, 0);
    va_end(ap);
    return r;
}

int wscanf_s(const wchar_t *__AXC_RESTRICT format, ...) {
    va_list ap;
    int r;
    va_start(ap, format);
    r = vf("wscanf_s", stdin, format, ap, 0);
    va_end(ap);
    return r;
}

int swscanf_s(const wchar_t *__AXC_RESTRICT s, const wchar_t *__AXC_RESTRICT format, ...) {
    va_list ap;
    int r;
    va_start(ap, format);
    r = vs("swscanf_s", s, format, ap, 0);
    va_end(ap);
    return r;
}

int __axc_vfwscanf_s_c23(FILE *__AXC_RESTRICT stream, const wchar_t *__AXC_RESTRICT format, va_list arg) {
    return vf("vfwscanf_s", stream, format, arg, 1);
}

int __axc_vwscanf_s_c23(const wchar_t *__AXC_RESTRICT format, va_list arg) {
    return vf("vwscanf_s", stdin, format, arg, 1);
}

int __axc_vswscanf_s_c23(const wchar_t *__AXC_RESTRICT s, const wchar_t *__AXC_RESTRICT format, va_list arg) {
    return vs("vswscanf_s", s, format, arg, 1);
}

int __axc_fwscanf_s_c23(FILE *__AXC_RESTRICT stream, const wchar_t *__AXC_RESTRICT format, ...) {
    va_list ap;
    int r;
    va_start(ap, format);
    r = vf("fwscanf_s", stream, format, ap, 1);
    va_end(ap);
    return r;
}

int __axc_wscanf_s_c23(const wchar_t *__AXC_RESTRICT format, ...) {
    va_list ap;
    int r;
    va_start(ap, format);
    r = vf("wscanf_s", stdin, format, ap, 1);
    va_end(ap);
    return r;
}

int __axc_swscanf_s_c23(const wchar_t *__AXC_RESTRICT s, const wchar_t *__AXC_RESTRICT format, ...) {
    va_list ap;
    int r;
    va_start(ap, format);
    r = vs("swscanf_s", s, format, ap, 1);
    va_end(ap);
    return r;
}
