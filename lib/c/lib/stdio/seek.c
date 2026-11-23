/* seek.c - file positioning and error handling (C17 7.21.9, 7.21.10):
   fseek, ftell, rewind, fgetpos, fsetpos, clearerr, feof, ferror,
   perror. Positions are 64-bit; ftell fails (EOVERFLOW) for one that
   long cannot hold (on Windows, where it is 32 bits). */
#include "impl.h"

char *strerror(int errnum); /* (<string.h>'s, the core part's) */

int fseek(FILE *stream, long int offset, int whence) {
    int r;
    LOCK(stream);
    r = __axc_seek_u(stream, offset, whence);
    UNLOCK(stream);
    return r;
}

long int ftell(FILE *stream) {
    __axiom_int64 pos;
    LOCK(stream);
    pos = __axc_tell_u(stream);
    UNLOCK(stream);
    if (pos > LONG_MAX) {
#ifdef EOVERFLOW
        SET_ERRNO(EOVERFLOW);
#endif
        return -1L;
    }
    return (long)pos;
}

void rewind(FILE *stream) {
    LOCK(stream);
    __axc_seek_u(stream, 0, SEEK_SET);
    stream->__flags &= ~F_ERR;
    UNLOCK(stream);
}

/* (the position, and a wide stream's conversion state: 7.21.9.1p2) */
int fgetpos(FILE *__AXC_RESTRICT stream, fpos_t *__AXC_RESTRICT pos) {
    __axiom_int64 p;
    LOCK(stream);
    p = __axc_tell_u(stream);
    {
        int k;
        for (k = 0; k < 4; k++)
            pos->__mbs[k] = stream->__mbs[k];
    }
    UNLOCK(stream);
    if (p < 0)
        return -1;
    pos->__pos = p;
    return 0;
}

int fsetpos(FILE *stream, const fpos_t *pos) {
    int r;
    LOCK(stream);
    r = __axc_seek_u(stream, pos->__pos, SEEK_SET);
    if (r == 0) {
        int k;
        for (k = 0; k < 4; k++)
            stream->__mbs[k] = pos->__mbs[k];
    }
    UNLOCK(stream);
    return r;
}

void clearerr(FILE *stream) {
    LOCK(stream);
    stream->__flags &= ~(F_EOF | F_ERR);
    UNLOCK(stream);
}

int feof(FILE *stream) {
    int r;
    LOCK(stream);
    r = (stream->__flags & F_EOF) != 0;
    UNLOCK(stream);
    return r;
}

int ferror(FILE *stream) {
    int r;
    LOCK(stream);
    r = (stream->__flags & F_ERR) != 0;
    UNLOCK(stream);
    return r;
}

/* 7.21.10.4: "s: message\n" (or the message alone) on stderr, the message
   strerror's. */
void perror(const char *s) {
    const char *m = strerror(errno);
    size_t n = 0;
    FILE *f = stderr;
    LOCK(f);
    if (f->__orient != O_WIDE) {
        __axc_orient(f, O_BYTE);
        if (s != NULL && *s != 0) {
            while (s[n] != 0)
                n++;
            __axc_write_u(f, (const unsigned char *)s, n);
            __axc_write_u(f, (const unsigned char *)": ", 2);
        }
        for (n = 0; m[n] != 0; n++)
            ;
        __axc_write_u(f, (const unsigned char *)m, n);
        __axc_write_u(f, (const unsigned char *)"\n", 1);
    }
    UNLOCK(f);
}
