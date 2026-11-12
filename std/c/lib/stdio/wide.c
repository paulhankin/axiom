/* wide.c - wide character input/output (C17 7.29.3): fgetwc, fgetws,
   fputwc, fputws, fwide, getwc, getwchar, putwc, putwchar, ungetwc. A
   wide-oriented stream holds multibyte characters, converted as mbrtowc
   and wcrtomb do with the stream's conversion state (7.21.2p6, __mbs); a
   wide function on a stream without orientation makes it wide-oriented,
   on a byte-oriented one it fails (7.21.2p4). */
#include "impl.h"

#define WIDE_OK(f) ((f)->__orient == O_WIDE || __axc_orient((f), O_WIDE) == O_WIDE)
#define STATE(f) ((mbstate_t *)(void *)(f)->__mbs)

/* (the stream's state must hold an mbstate_t) */
typedef char state_fits[sizeof(mbstate_t) <= sizeof(((FILE *)0)->__mbs) ? 1 : -1];

/* The next wide character, or WEOF (the lock held, wide-oriented) */
wint_t __axc_getwc_u(FILE *f) {
    wchar_t wc;
    for (;;) {
        int c = GETC_U(f);
        char b;
        size_t r;
        if (c == EOF) {
            if (!mbsinit(STATE(f))) {
                /* (the file ends inside a multibyte character) */
                f->__flags |= F_ERR;
                SET_ERRNO(EILSEQ);
            }
            return WEOF;
        }
        b = (char)c;
        r = mbrtowc(&wc, &b, 1, STATE(f));
        if (r == (size_t)-2)
            continue;
        if (r == (size_t)-1) {
            f->__flags |= F_ERR;
            SET_ERRNO(EILSEQ);
            return WEOF;
        }
        return (wint_t)wc;
    }
}

/* Writes the wide character; c, or WEOF (the lock held, wide-oriented) */
wint_t __axc_putwc_u(wchar_t c, FILE *f) {
    char mb[16];
    size_t n = wcrtomb(mb, c, STATE(f)), k;
    if (n == (size_t)-1) {
        f->__flags |= F_ERR;
        return WEOF;
    }
    for (k = 0; k < n; k++) {
        if (PUTC_U((unsigned char)mb[k], f) == EOF)
            return WEOF;
    }
    return (wint_t)c;
}

wint_t fgetwc(FILE *stream) {
    wint_t c;
    LOCK(stream);
    c = WIDE_OK(stream) ? __axc_getwc_u(stream) : WEOF;
    UNLOCK(stream);
    return c;
}

wint_t getwc(FILE *stream) {
    return fgetwc(stream);
}

wint_t getwchar(void) {
    return fgetwc(stdin);
}

wint_t fputwc(wchar_t c, FILE *stream) {
    wint_t r;
    LOCK(stream);
    r = WIDE_OK(stream) ? __axc_putwc_u(c, stream) : WEOF;
    UNLOCK(stream);
    return r;
}

wint_t putwc(wchar_t c, FILE *stream) {
    return fputwc(c, stream);
}

wint_t putwchar(wchar_t c) {
    return fputwc(c, stdout);
}

/* 7.29.3.2: at most n - 1 wide characters, up to and including a new-line */
wchar_t *fgetws(wchar_t *__AXC_RESTRICT s, int n, FILE *__AXC_RESTRICT stream) {
    wchar_t *p = s;
    wint_t c = 0;
    int err;
    if (n <= 0)
        return NULL;
    LOCK(stream);
    if (!WIDE_OK(stream)) {
        UNLOCK(stream);
        return NULL;
    }
    err = (stream->__flags & F_ERR) != 0;
    while (--n > 0) {
        c = __axc_getwc_u(stream);
        if (c == WEOF)
            break;
        *p++ = (wchar_t)c;
        if (c == L'\n')
            break;
    }
    if (c == WEOF && (p == s || (!err && (stream->__flags & F_ERR)))) {
        UNLOCK(stream);
        return NULL;
    }
    UNLOCK(stream);
    *p = 0;
    return s;
}

int fputws(const wchar_t *__AXC_RESTRICT s, FILE *__AXC_RESTRICT stream) {
    int r = 0;
    LOCK(stream);
    if (!WIDE_OK(stream)) {
        r = EOF;
    } else {
        for (; *s != 0; s++) {
            if (__axc_putwc_u(*s, stream) == WEOF) {
                r = EOF;
                break;
            }
        }
    }
    UNLOCK(stream);
    return r;
}

/* 7.29.3.5: mode > 0 tries to make the stream wide-oriented, < 0 byte;
   the orientation it has: > 0 wide, < 0 byte, 0 none */
int fwide(FILE *stream, int mode) {
    int o;
    LOCK(stream);
    if (mode > 0)
        o = __axc_orient(stream, O_WIDE);
    else if (mode < 0)
        o = __axc_orient(stream, O_BYTE);
    else
        o = stream->__orient;
    UNLOCK(stream);
    return o;
}

/* ungetwc's work, the lock held (7.29.3.10): the wide character goes
   back as its multibyte bytes (converted with a state of its own: the
   stream's describes the bytes read), so that at least one can always be
   pushed back. */
wint_t __axc_ungetwc_u(wint_t c, FILE *f) {
    char mb[16];
    mbstate_t st;
    size_t n, k;
    unsigned char *z = (unsigned char *)&st;
    if (c == WEOF)
        return WEOF;
    for (k = 0; k < sizeof st; k++)
        z[k] = 0;
    n = wcrtomb(mb, (wchar_t)c, &st);
    if (n == (size_t)-1 || n == 0)
        return WEOF;
    if (!(f->__flags & F_RD) && __axc_to_read(f) != 0)
        return WEOF;
    if (!(f->__flags & F_BACK)) {
        f->__save_rpos = f->__rpos;
        f->__save_rend = f->__rend;
        f->__rpos = f->__rend = f->__back + BACK_MAX;
        f->__flags |= F_BACK;
    }
    if ((size_t)(f->__rpos - f->__back) < n)
        return WEOF;
    while (n > 0)
        *--f->__rpos = (unsigned char)mb[--n];
    f->__flags &= ~F_EOF;
    return c;
}

wint_t ungetwc(wint_t c, FILE *stream) {
    wint_t r;
    LOCK(stream);
    r = WIDE_OK(stream) ? __axc_ungetwc_u(c, stream) : WEOF;
    UNLOCK(stream);
    return r;
}
