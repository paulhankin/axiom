/* wprintf.c - the formatted wide output functions (C17 7.29.2.1, .3, .5,
   .7, .9, .11): the engine (printf_core.h) for wide characters, over a
   stream (each wide character converted as fputwc does) or a wide string. */
#include "impl.h"

#define CH wchar_t
#define PF_WIDE 1
#define PF_NAME __axc_vwprintf_core
#define PF_TEXT wpf_text
#define PF_PAD wpf_pad
#define PF_FIELD wpf_field
#include "printf_core.h"

/* Output to a wide-oriented stream (writing, its lock held) */
static void wfile_put(struct __axc_out *o, const void *p, size_t n) {
    const wchar_t *w = (const wchar_t *)p;
    size_t k;
    o->count += n;
    for (k = 0; k < n && !o->failed; k++) {
        if (__axc_putwc_u(w[k], o->f) == WEOF)
            o->failed = 1;
    }
}

int vfwprintf(FILE *__AXC_RESTRICT stream, const wchar_t *__AXC_RESTRICT format, va_list arg) {
    FILE *f = stream;
    struct __axc_out o;
    unsigned char tmp[1024];
    int r;
    LOCK(f);
    if ((f->__orient != O_WIDE && __axc_orient(f, O_WIDE) != O_WIDE) ||
        (!(f->__flags & F_WR) && __axc_to_write(f) != 0)) {
        UNLOCK(f);
        return -1;
    }
    o.put = wfile_put;
    o.count = 0;
    o.failed = 0;
    o.f = f;
    o.s = NULL;
    o.room = 0;
    if (f->__flags & F_NBF) {
        /* (a buffer for the call, as vfprintf gives one) */
        unsigned char *buf = f->__buf;
        size_t size = f->__size;
        f->__buf = f->__wpos = tmp;
        f->__wend = tmp + sizeof tmp;
        f->__size = sizeof tmp;
        f->__flags &= ~F_NBF;
        r = __axc_vwprintf_core(&o, format, arg);
        if (__axc_flush_u(f) != 0)
            r = -1;
        f->__flags |= F_NBF;
        f->__buf = f->__wpos = f->__wend = buf;
        f->__size = size;
    } else {
        r = __axc_vwprintf_core(&o, format, arg);
    }
    UNLOCK(f);
    return r;
}

int fwprintf(FILE *__AXC_RESTRICT stream, const wchar_t *__AXC_RESTRICT format, ...) {
    va_list ap;
    int r;
    va_start(ap, format);
    r = vfwprintf(stream, format, ap);
    va_end(ap);
    return r;
}

int vwprintf(const wchar_t *__AXC_RESTRICT format, va_list arg) {
    return vfwprintf(stdout, format, arg);
}

int wprintf(const wchar_t *__AXC_RESTRICT format, ...) {
    va_list ap;
    int r;
    va_start(ap, format);
    r = vfwprintf(stdout, format, ap);
    va_end(ap);
    return r;
}

/* Output to a wide string (room: wide characters that fit before the null) */
static void wstr_put(struct __axc_out *o, const void *p, size_t n) {
    const wchar_t *w = (const wchar_t *)p;
    wchar_t *d = (wchar_t *)(void *)o->s;
    size_t k = n < o->room ? n : o->room, i;
    for (i = 0; i < k; i++)
        d[i] = w[i];
    o->s = (unsigned char *)(void *)(d + k);
    o->room -= k;
    o->count += n;
}

/* 7.29.2.7: at most n - 1 wide characters and a null; a negative value if
   n or more were to be written */
int vswprintf(wchar_t *__AXC_RESTRICT s, size_t n, const wchar_t *__AXC_RESTRICT format, va_list arg) {
    struct __axc_out o;
    int r;
    if (n == 0)
        return -1;
    o.put = wstr_put;
    o.count = 0;
    o.failed = 0;
    o.f = NULL;
    o.s = (unsigned char *)(void *)s;
    o.room = n - 1;
    r = __axc_vwprintf_core(&o, format, arg);
    *(wchar_t *)(void *)o.s = 0;
    if (r >= 0 && (size_t)r >= n)
        return -1;
    return r;
}

int swprintf(wchar_t *__AXC_RESTRICT s, size_t n, const wchar_t *__AXC_RESTRICT format, ...) {
    va_list ap;
    int r;
    va_start(ap, format);
    r = vswprintf(s, n, format, ap);
    va_end(ap);
    return r;
}
