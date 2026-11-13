/* fscanf_s, scanf_s, sscanf_s, vfscanf_s, vscanf_s, vsscanf_s (C11
   K.3.5.3.2, .4, .7, .9, .12, .14): fscanf's input, but c, s and [ take
   the receiving array's number of elements (an rsize_t) after its
   pointer, and an item that does not fit (with its null character) is a
   matching failure (the array then holds the empty string). Runtime-
   constraints: stream (s) and format not null, no null pointer to store
   through; after a violation the input stops and the result is EOF. The
   __axc_..._c23 forms are C23 mode's (<stdio.h> maps the names): %i takes
   the 0b prefix. */
#define __STDC_WANT_LIB_EXT1__ 1
#include <axc/ext1_impl.h>
#include <stdio.h>
#include <stdarg.h>

int __axc_vfscanf_x(FILE *f, const char *fmt, va_list ap, int c23, const char **viol);
int __axc_vsscanf_x(const char *s, const char *fmt, va_list ap, int c23, const char **viol);

static int vf(const char *fn, FILE *stream, const char *format, va_list arg, int c23) {
    const char *viol = NULL;
    int r = EOF;
    if (stream == NULL)
        viol = "stream is a null pointer";
    else if (format == NULL)
        viol = "format is a null pointer";
    else
        r = __axc_vfscanf_x(stream, format, arg, c23, &viol);
    if (viol == NULL)
        return r;
    __axc_ext1_violation(fn, viol, EINVAL);
    return EOF;
}

static int vs(const char *fn, const char *s, const char *format, va_list arg, int c23) {
    const char *viol = NULL;
    int r = EOF;
    if (s == NULL)
        viol = "s is a null pointer";
    else if (format == NULL)
        viol = "format is a null pointer";
    else
        r = __axc_vsscanf_x(s, format, arg, c23, &viol);
    if (viol == NULL)
        return r;
    __axc_ext1_violation(fn, viol, EINVAL);
    return EOF;
}

int vfscanf_s(FILE *__AXC_RESTRICT stream, const char *__AXC_RESTRICT format, va_list arg) {
    return vf("vfscanf_s", stream, format, arg, 0);
}

int vscanf_s(const char *__AXC_RESTRICT format, va_list arg) {
    return vf("vscanf_s", stdin, format, arg, 0);
}

int vsscanf_s(const char *__AXC_RESTRICT s, const char *__AXC_RESTRICT format, va_list arg) {
    return vs("vsscanf_s", s, format, arg, 0);
}

int fscanf_s(FILE *__AXC_RESTRICT stream, const char *__AXC_RESTRICT format, ...) {
    va_list ap;
    int r;
    va_start(ap, format);
    r = vf("fscanf_s", stream, format, ap, 0);
    va_end(ap);
    return r;
}

int scanf_s(const char *__AXC_RESTRICT format, ...) {
    va_list ap;
    int r;
    va_start(ap, format);
    r = vf("scanf_s", stdin, format, ap, 0);
    va_end(ap);
    return r;
}

int sscanf_s(const char *__AXC_RESTRICT s, const char *__AXC_RESTRICT format, ...) {
    va_list ap;
    int r;
    va_start(ap, format);
    r = vs("sscanf_s", s, format, ap, 0);
    va_end(ap);
    return r;
}

int __axc_vfscanf_s_c23(FILE *__AXC_RESTRICT stream, const char *__AXC_RESTRICT format, va_list arg) {
    return vf("vfscanf_s", stream, format, arg, 1);
}

int __axc_vscanf_s_c23(const char *__AXC_RESTRICT format, va_list arg) {
    return vf("vscanf_s", stdin, format, arg, 1);
}

int __axc_vsscanf_s_c23(const char *__AXC_RESTRICT s, const char *__AXC_RESTRICT format, va_list arg) {
    return vs("vsscanf_s", s, format, arg, 1);
}

int __axc_fscanf_s_c23(FILE *__AXC_RESTRICT stream, const char *__AXC_RESTRICT format, ...) {
    va_list ap;
    int r;
    va_start(ap, format);
    r = vf("fscanf_s", stream, format, ap, 1);
    va_end(ap);
    return r;
}

int __axc_scanf_s_c23(const char *__AXC_RESTRICT format, ...) {
    va_list ap;
    int r;
    va_start(ap, format);
    r = vf("scanf_s", stdin, format, ap, 1);
    va_end(ap);
    return r;
}

int __axc_sscanf_s_c23(const char *__AXC_RESTRICT s, const char *__AXC_RESTRICT format, ...) {
    va_list ap;
    int r;
    va_start(ap, format);
    r = vs("sscanf_s", s, format, ap, 1);
    va_end(ap);
    return r;
}
