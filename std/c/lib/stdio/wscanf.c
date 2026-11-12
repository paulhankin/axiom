/* wscanf.c - the formatted wide input functions (C17 7.29.2.2, .4, .6,
   .8, .10, .12): the engine (scanf_core.h) for wide characters, over a
   wide-oriented stream or a wide string. */
#include "impl.h"

#define CH wchar_t
#define SF_WIDE 1
#define SF_NAME __axc_vwscanf_core
#include "scanf_core.h"

static int wfile_get(struct __axc_in *i) {
    wint_t c = __axc_getwc_u(i->f);
    if (c == WEOF) {
        i->eof = 1;
        return -1;
    }
    return (int)c;
}

static void wfile_unget(struct __axc_in *i, int c) {
    __axc_ungetwc_u((wint_t)c, i->f);
}

static int vfwscanf_with(FILE *stream, const wchar_t *format, va_list arg, int c23) {
    struct __axc_in in;
    int r;
    in.get = wfile_get;
    in.unget = wfile_unget;
    in.count = 0;
    in.f = stream;
    in.s = NULL;
    in.ws = NULL;
    in.eof = in.error = 0;
    in.c23 = c23;
    LOCK(stream);
    if (stream->__orient != O_WIDE && __axc_orient(stream, O_WIDE) != O_WIDE) {
        UNLOCK(stream);
        return EOF;
    }
    r = __axc_vwscanf_core(&in, format, arg);
    UNLOCK(stream);
    return r;
}

int fwscanf(FILE *__AXC_RESTRICT stream, const wchar_t *__AXC_RESTRICT format, ...) {
    va_list ap;
    int r;
    va_start(ap, format);
    r = vfwscanf(stream, format, ap);
    va_end(ap);
    return r;
}

int vwscanf(const wchar_t *__AXC_RESTRICT format, va_list arg) {
    return vfwscanf(stdin, format, arg);
}

int wscanf(const wchar_t *__AXC_RESTRICT format, ...) {
    va_list ap;
    int r;
    va_start(ap, format);
    r = vfwscanf(stdin, format, ap);
    va_end(ap);
    return r;
}

static int wstr_get(struct __axc_in *i) {
    if (*i->ws == 0) {
        i->eof = 1;
        return -1;
    }
    return (int)*i->ws++;
}

static void wstr_unget(struct __axc_in *i, int c) {
    (void)c;
    i->ws--;
}

static int vswscanf_with(const wchar_t *s, const wchar_t *format, va_list arg, int c23) {
    struct __axc_in in;
    in.get = wstr_get;
    in.unget = wstr_unget;
    in.count = 0;
    in.f = NULL;
    in.s = NULL;
    in.ws = s;
    in.eof = in.error = 0;
    in.c23 = c23;
    return __axc_vwscanf_core(&in, format, arg);
}

int swscanf(const wchar_t *__AXC_RESTRICT s, const wchar_t *__AXC_RESTRICT format, ...) {
    va_list ap;
    int r;
    va_start(ap, format);
    r = vswscanf(s, format, ap);
    va_end(ap);
    return r;
}

int vfwscanf(FILE *__AXC_RESTRICT stream, const wchar_t *__AXC_RESTRICT format, va_list arg) {
    return vfwscanf_with(stream, format, arg, 0);
}

int vswscanf(const wchar_t *__AXC_RESTRICT s, const wchar_t *__AXC_RESTRICT format, va_list arg) {
    return vswscanf_with(s, format, arg, 0);
}

/* C23 mode's (<wchar.h> maps the names here): %i takes the 0b prefix */
int __axc_vfwscanf_c23(FILE *__AXC_RESTRICT stream, const wchar_t *__AXC_RESTRICT format, va_list arg) {
    return vfwscanf_with(stream, format, arg, 1);
}

int __axc_vswscanf_c23(const wchar_t *__AXC_RESTRICT s, const wchar_t *__AXC_RESTRICT format, va_list arg) {
    return vswscanf_with(s, format, arg, 1);
}

int __axc_vwscanf_c23(const wchar_t *__AXC_RESTRICT format, va_list arg) {
    return vfwscanf_with(stdin, format, arg, 1);
}

int __axc_fwscanf_c23(FILE *__AXC_RESTRICT stream, const wchar_t *__AXC_RESTRICT format, ...) {
    va_list ap;
    int r;
    va_start(ap, format);
    r = vfwscanf_with(stream, format, ap, 1);
    va_end(ap);
    return r;
}

int __axc_wscanf_c23(const wchar_t *__AXC_RESTRICT format, ...) {
    va_list ap;
    int r;
    va_start(ap, format);
    r = vfwscanf_with(stdin, format, ap, 1);
    va_end(ap);
    return r;
}

int __axc_swscanf_c23(const wchar_t *__AXC_RESTRICT s, const wchar_t *__AXC_RESTRICT format, ...) {
    va_list ap;
    int r;
    va_start(ap, format);
    r = vswscanf_with(s, format, ap, 1);
    va_end(ap);
    return r;
}
