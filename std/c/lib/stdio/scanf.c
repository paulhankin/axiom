/* scanf.c - the formatted input functions (C17 7.21.6.2, .4, .7, .9,
   .11, .14): the engine (scanf_core.h) over a stream or a string. */
#include "impl.h"

#define CH char
#define SF_WIDE 0
#define SF_NAME __axc_vscanf_core
#include "scanf_core.h"

/* Input from a stream (its lock held) */
static int file_get(struct __axc_in *i) {
    int c = GETC_U(i->f);
    if (c == EOF) {
        i->eof = 1;
        return -1;
    }
    return c;
}

static void file_unget(struct __axc_in *i, int c) {
    __axc_ungetc_u(c, i->f);
}

static int vfscanf_with(FILE *stream, const char *format, va_list arg, int c23, const char **viol) {
    struct __axc_in in;
    int r;
    in.get = file_get;
    in.unget = file_unget;
    in.count = 0;
    in.f = stream;
    in.s = NULL;
    in.ws = NULL;
    in.eof = in.error = 0;
    in.c23 = c23;
    in.viol = viol;
    LOCK(stream);
    if (stream->__orient != O_BYTE && __axc_orient(stream, O_BYTE) != O_BYTE) {
        UNLOCK(stream);
        return EOF;
    }
    r = __axc_vscanf_core(&in, format, arg);
    UNLOCK(stream);
    return r;
}

int fscanf(FILE *__AXC_RESTRICT stream, const char *__AXC_RESTRICT format, ...) {
    va_list ap;
    int r;
    va_start(ap, format);
    r = vfscanf(stream, format, ap);
    va_end(ap);
    return r;
}

int vscanf(const char *__AXC_RESTRICT format, va_list arg) {
    return vfscanf(stdin, format, arg);
}

int scanf(const char *__AXC_RESTRICT format, ...) {
    va_list ap;
    int r;
    va_start(ap, format);
    r = vfscanf(stdin, format, ap);
    va_end(ap);
    return r;
}

/* Input from a string: its end is the end of the input */
static int str_get(struct __axc_in *i) {
    if (*i->s == 0) {
        i->eof = 1;
        return -1;
    }
    return *i->s++;
}

static void str_unget(struct __axc_in *i, int c) {
    (void)c;
    i->s--;
}

static int vsscanf_with(const char *s, const char *format, va_list arg, int c23, const char **viol) {
    struct __axc_in in;
    in.get = str_get;
    in.unget = str_unget;
    in.count = 0;
    in.f = NULL;
    in.s = (const unsigned char *)s;
    in.ws = NULL;
    in.eof = in.error = 0;
    in.c23 = c23;
    in.viol = viol;
    return __axc_vscanf_core(&in, format, arg);
}

int sscanf(const char *__AXC_RESTRICT s, const char *__AXC_RESTRICT format, ...) {
    va_list ap;
    int r;
    va_start(ap, format);
    r = vsscanf(s, format, ap);
    va_end(ap);
    return r;
}

int vfscanf(FILE *__AXC_RESTRICT stream, const char *__AXC_RESTRICT format, va_list arg) {
    return vfscanf_with(stream, format, arg, 0, NULL);
}

int vsscanf(const char *__AXC_RESTRICT s, const char *__AXC_RESTRICT format, va_list arg) {
    return vsscanf_with(s, format, arg, 0, NULL);
}

/* C23 mode's (<stdio.h> maps the names here): %i takes the 0b prefix */
int __axc_vfscanf_c23(FILE *__AXC_RESTRICT stream, const char *__AXC_RESTRICT format, va_list arg) {
    return vfscanf_with(stream, format, arg, 1, NULL);
}

int __axc_vsscanf_c23(const char *__AXC_RESTRICT s, const char *__AXC_RESTRICT format, va_list arg) {
    return vsscanf_with(s, format, arg, 1, NULL);
}

int __axc_vscanf_c23(const char *__AXC_RESTRICT format, va_list arg) {
    return vfscanf_with(stdin, format, arg, 1, NULL);
}

int __axc_fscanf_c23(FILE *__AXC_RESTRICT stream, const char *__AXC_RESTRICT format, ...) {
    va_list ap;
    int r;
    va_start(ap, format);
    r = vfscanf_with(stream, format, ap, 1, NULL);
    va_end(ap);
    return r;
}

int __axc_scanf_c23(const char *__AXC_RESTRICT format, ...) {
    va_list ap;
    int r;
    va_start(ap, format);
    r = vfscanf_with(stdin, format, ap, 1, NULL);
    va_end(ap);
    return r;
}

int __axc_sscanf_c23(const char *__AXC_RESTRICT s, const char *__AXC_RESTRICT format, ...) {
    va_list ap;
    int r;
    va_start(ap, format);
    r = vsscanf_with(s, format, ap, 1, NULL);
    va_end(ap);
    return r;
}

/* Annex K's fscanf_s and its kin (std/c/lib/ext1/): the engine with its
   runtime-constraints (viol) */
int __axc_vfscanf_x(FILE *f, const char *fmt, va_list ap, int c23, const char **viol) {
    return vfscanf_with(f, fmt, ap, c23, viol);
}

int __axc_vsscanf_x(const char *s, const char *fmt, va_list ap, int c23, const char **viol) {
    return vsscanf_with(s, fmt, ap, c23, viol);
}
