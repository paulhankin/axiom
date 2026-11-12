/* printf.c - the formatted output functions (C17 7.21.6.1, .3, .5, .6,
   .8 to .13): the engine (printf_core.h) over a stream or a string. */
#include "impl.h"

#define CH char
#define PF_WIDE 0
#define PF_NAME __axc_vprintf_core
#define PF_TEXT pf_text
#define PF_PAD pf_pad
#define PF_FIELD pf_field
#include "printf_core.h"

/* Output to a stream (writing, its lock held) */
static void file_put(struct __axc_out *o, const void *p, size_t n) {
    FILE *f = o->f;
    o->count += n;
    if ((size_t)(f->__wend - f->__wpos) > n && !(f->__flags & F_LBF)) {
        unsigned char *w = f->__wpos;
        if (n <= 16) {
            /* (short runs: no call) */
            const unsigned char *q = (const unsigned char *)p;
            size_t k;
            for (k = 0; k < n; k++)
                w[k] = q[k];
        } else {
            __axc_sys_copy(w, p, n);
        }
        f->__wpos = w + n;
        return;
    }
    if (__axc_write_u(f, (const unsigned char *)p, n) != n)
        o->failed = 1;
}

/* vfprintf's work, the lock held. An unbuffered stream is given a buffer
   for the call (7.21.3p3: the characters are to appear "as soon as
   possible", which is at the end of the call, and in one write). */
int __axc_vfprintf_u(FILE *f, const char *fmt, va_list ap) {
    struct __axc_out o;
    unsigned char tmp[1024];
    int r;
    if (f->__orient != O_BYTE && __axc_orient(f, O_BYTE) != O_BYTE)
        return -1;
    if (!(f->__flags & F_WR) && __axc_to_write(f) != 0)
        return -1;
    o.put = file_put;
    o.count = 0;
    o.failed = 0;
    o.f = f;
    o.s = NULL;
    o.room = 0;
    if (f->__flags & F_NBF) {
        unsigned char *buf = f->__buf;
        size_t size = f->__size;
        f->__buf = f->__wpos = tmp;
        f->__wend = tmp + sizeof tmp;
        f->__size = sizeof tmp;
        f->__flags &= ~F_NBF;
        r = __axc_vprintf_core(&o, fmt, ap);
        if (__axc_flush_u(f) != 0)
            r = -1;
        f->__flags |= F_NBF;
        f->__buf = f->__wpos = f->__wend = buf;
        f->__size = size;
        return r;
    }
    r = __axc_vprintf_core(&o, fmt, ap);
    return r;
}

int vfprintf(FILE *__AXC_RESTRICT stream, const char *__AXC_RESTRICT format, va_list arg) {
    int r;
    LOCK(stream);
    r = __axc_vfprintf_u(stream, format, arg);
    UNLOCK(stream);
    return r;
}

int fprintf(FILE *__AXC_RESTRICT stream, const char *__AXC_RESTRICT format, ...) {
    va_list ap;
    int r;
    va_start(ap, format);
    r = vfprintf(stream, format, ap);
    va_end(ap);
    return r;
}

int vprintf(const char *__AXC_RESTRICT format, va_list arg) {
    return vfprintf(stdout, format, arg);
}

int printf(const char *__AXC_RESTRICT format, ...) {
    va_list ap;
    int r;
    va_start(ap, format);
    r = vfprintf(stdout, format, ap);
    va_end(ap);
    return r;
}

/* Output to a string: what fits is stored, everything is counted */
static void str_put(struct __axc_out *o, const void *p, size_t n) {
    size_t k = n < o->room ? n : o->room;
    const unsigned char *q = (const unsigned char *)p;
    size_t i;
    for (i = 0; i < k; i++)
        o->s[i] = q[i];
    o->s += k;
    o->room -= k;
    o->count += n;
}

/* 7.21.6.12: at most n - 1 characters and a null (none for n 0); the
   count the whole output would have */
int vsnprintf(char *__AXC_RESTRICT s, size_t n, const char *__AXC_RESTRICT format, va_list arg) {
    struct __axc_out o;
    int r;
    char dummy;
    o.put = str_put;
    o.count = 0;
    o.failed = 0;
    o.f = NULL;
    o.s = (unsigned char *)(n > 0 ? s : &dummy);
    o.room = n > 0 ? n - 1 : 0;
    r = __axc_vprintf_core(&o, format, arg);
    if (n > 0)
        *o.s = 0;
    return r;
}

int snprintf(char *__AXC_RESTRICT s, size_t n, const char *__AXC_RESTRICT format, ...) {
    va_list ap;
    int r;
    va_start(ap, format);
    r = vsnprintf(s, n, format, ap);
    va_end(ap);
    return r;
}

int vsprintf(char *__AXC_RESTRICT s, const char *__AXC_RESTRICT format, va_list arg) {
    return vsnprintf(s, (size_t)-1 / 2, format, arg);
}

int sprintf(char *__AXC_RESTRICT s, const char *__AXC_RESTRICT format, ...) {
    va_list ap;
    int r;
    va_start(ap, format);
    r = vsnprintf(s, (size_t)-1 / 2, format, ap);
    va_end(ap);
    return r;
}
