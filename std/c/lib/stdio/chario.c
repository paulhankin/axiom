/* chario.c - character and direct input/output (C17 7.21.7, 7.21.8):
   fgetc, getc, getchar, fgets, gets, fputc, putc, putchar, fputs, puts,
   ungetc, fread, fwrite. A byte function on a stream without orientation
   makes it byte-oriented (7.21.2p4); on a wide-oriented stream it fails. */
#include "impl.h"

#define BYTE_OK(f) ((f)->__orient == O_BYTE || __axc_orient((f), O_BYTE) == O_BYTE)

int fgetc(FILE *stream) {
    int c;
    LOCK(stream);
    c = BYTE_OK(stream) ? GETC_U(stream) : EOF;
    UNLOCK(stream);
    return c;
}

int getc(FILE *stream) {
    int c;
    LOCK(stream);
    c = BYTE_OK(stream) ? GETC_U(stream) : EOF;
    UNLOCK(stream);
    return c;
}

int getchar(void) {
    return getc(stdin);
}

/* 7.21.7.2: at most n - 1 characters, up to and including a new-line */
char *fgets(char *__AXC_RESTRICT s, int n, FILE *__AXC_RESTRICT stream) {
    char *p = s;
    int c = 0, err;
    size_t left;
    if (n <= 0)
        return NULL;
    left = (size_t)n - 1;
    LOCK(stream);
    if (!BYTE_OK(stream)) {
        UNLOCK(stream);
        return NULL;
    }
    err = (stream->__flags & F_ERR) != 0;
    while (left > 0) {
        /* (a run of the buffer at a time) */
        unsigned char *r = stream->__rpos;
        if (r < stream->__rend) {
            size_t k = (size_t)(stream->__rend - r);
            uint64_t got;
            if (k > left)
                k = left;
            got = __axc_sys_line((unsigned char *)p, r, k);
            k = (size_t)(got & ~((uint64_t)1 << 63));
            p += k;
            stream->__rpos = r + k;
            if (got >> 63)
                goto done;
            left -= k;
            continue;
        }
        c = __axc_uflow(stream);
        if (c == EOF)
            break;
        *p++ = (char)c;
        left--;
        if (c == '\n')
            break;
    }
done:
    /* (7.21.7.2p3: nothing read at the end of the file, or a read error) */
    if (c == EOF && (p == s || (!err && (stream->__flags & F_ERR)))) {
        UNLOCK(stream);
        return NULL;
    }
    UNLOCK(stream);
    *p = 0;
    return s;
}

/* (C89, C99; removed by C11, and not declared in its modes) */
char *gets(char *s) {
    char *p = s;
    int c = 0;
    LOCK(stdin);
    if (!BYTE_OK(stdin)) {
        UNLOCK(stdin);
        return NULL;
    }
    while ((c = GETC_U(stdin)) != EOF && c != '\n')
        *p++ = (char)c;
    UNLOCK(stdin);
    if (c == EOF && (p == s || ((stdin->__flags & F_ERR) && !(stdin->__flags & F_EOF))))
        return NULL;
    *p = 0;
    return s;
}

int fputc(int c, FILE *stream) {
    int r;
    LOCK(stream);
    r = BYTE_OK(stream) ? PUTC_U(c, stream) : EOF;
    UNLOCK(stream);
    return r;
}

int putc(int c, FILE *stream) {
    int r;
    LOCK(stream);
    r = BYTE_OK(stream) ? PUTC_U(c, stream) : EOF;
    UNLOCK(stream);
    return r;
}

int putchar(int c) {
    return putc(c, stdout);
}

int fputs(const char *__AXC_RESTRICT s, FILE *__AXC_RESTRICT stream) {
    size_t n = 0, w;
    while (s[n] != 0)
        n++;
    LOCK(stream);
    w = BYTE_OK(stream) ? __axc_write_u(stream, (const unsigned char *)s, n) : (size_t)-1;
    UNLOCK(stream);
    return w == n ? 1 : EOF;
}

int puts(const char *s) {
    size_t n = 0;
    int r = EOF;
    while (s[n] != 0)
        n++;
    LOCK(stdout);
    if (BYTE_OK(stdout) && __axc_write_u(stdout, (const unsigned char *)s, n) == n && PUTC_U('\n', stdout) != EOF)
        r = 1;
    UNLOCK(stdout);
    return r;
}

/* ungetc's work, the lock held: up to BACK_MAX bytes pushed back; the
   file is not changed, and a seek drops them (7.21.7.10). */
int __axc_ungetc_u(int c, FILE *f) {
    if (c == EOF)
        return EOF;
    if (!(f->__flags & F_RD) && __axc_to_read(f) != 0)
        return EOF;
    if (f->__flags & F_BACK) {
        if (f->__rpos <= f->__back)
            return EOF;
        *--f->__rpos = (unsigned char)c;
    } else if (!(f->__flags & F_TEXT) && f->__rpos > f->__buf && f->__rend == f->__dend) {
        /* (into the buffer: a binary stream's position is the buffer's) */
        *--f->__rpos = (unsigned char)c;
    } else {
        f->__save_rpos = f->__rpos;
        f->__save_rend = f->__rend;
        f->__rend = f->__back + BACK_MAX;
        f->__rpos = f->__rend - 1;
        *f->__rpos = (unsigned char)c;
        f->__flags |= F_BACK;
    }
    f->__flags &= ~F_EOF;
    return (unsigned char)c;
}

int ungetc(int c, FILE *stream) {
    int r;
    LOCK(stream);
    r = BYTE_OK(stream) ? __axc_ungetc_u(c, stream) : EOF;
    UNLOCK(stream);
    return r;
}

size_t fread(void *__AXC_RESTRICT ptr, size_t size, size_t nmemb, FILE *__AXC_RESTRICT stream) {
    size_t n, got;
    if (size == 0 || nmemb == 0)
        return 0;
    n = size * nmemb;
    if (n / size != nmemb) {
        stream->__flags |= F_ERR;
        return 0;
    }
    LOCK(stream);
    got = BYTE_OK(stream) ? __axc_read_u(stream, (unsigned char *)ptr, n) : 0;
    UNLOCK(stream);
    return got == n ? nmemb : got / size;
}

size_t fwrite(const void *__AXC_RESTRICT ptr, size_t size, size_t nmemb, FILE *__AXC_RESTRICT stream) {
    size_t n, put;
    if (size == 0 || nmemb == 0)
        return 0;
    n = size * nmemb;
    if (n / size != nmemb) {
        stream->__flags |= F_ERR;
        return 0;
    }
    LOCK(stream);
    put = BYTE_OK(stream) ? __axc_write_u(stream, (const unsigned char *)ptr, n) : 0;
    UNLOCK(stream);
    return put == n ? nmemb : put / size;
}

/* The first c in p[0..n), or NULL: eight bytes at a time where aligned,
   by the test for a zero byte in x ^ (c * 0x01..01) (a byte b is zero iff
   (b - 1) & ~b has its top bit set). */
const unsigned char *__axc_find_byte(const unsigned char *p, int c, size_t n) {
    const unsigned __axiom_int64 ones = 0x0101010101010101ULL, highs = 0x8080808080808080ULL;
    unsigned __axiom_int64 pat = ones * (unsigned char)c;
    while (n > 0 && ((size_t)p & 7) != 0) {
        if (*p == (unsigned char)c)
            return p;
        p++;
        n--;
    }
    while (n >= 8) {
        unsigned __axiom_int64 x = *(const unsigned __axiom_int64 *)(const void *)p ^ pat;
        if (((x - ones) & ~x & highs) != 0)
            break;
        p += 8;
        n -= 8;
    }
    for (; n > 0; p++, n--) {
        if (*p == (unsigned char)c)
            return p;
    }
    return NULL;
}
