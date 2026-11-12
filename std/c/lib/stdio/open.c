/* open.c - operations on files and file access (C17 7.21.4, 7.21.5):
   remove, rename, tmpfile, tmpnam, fopen, freopen, fclose, fflush,
   setvbuf, setbuf. */
#include "impl.h"

/* Opens path with the mode's flags into f (a new stream, or freopen's);
   0, or -1 with errno set. */
static int open_into(FILE *f, const char *path, const char *mode) {
    unsigned int flags, oflags;
    int fd;
    if (__axc_parse_mode(mode, &flags, &oflags) != 0) {
#ifdef EINVAL
        SET_ERRNO(EINVAL);
#endif
        return -1;
    }
    fd = __axc_sys_open(path, oflags);
    if (fd < 0) {
        SET_ERRNO(__axc_errno_of(fd));
        return -1;
    }
    f->__fd = fd;
    f->__flags = (f->__flags & F_STATIC) | flags;
    f->__buf = NULL;
    f->__size = 0;
    f->__orient = O_NONE;
    f->__mbs[0] = f->__mbs[1] = f->__mbs[2] = f->__mbs[3] = 0;
    /* (7.21.5.3p8: fully buffered unless it is an interactive device) */
    if (__axc_sys_isatty(fd))
        f->__flags |= F_LBF;
    return 0;
}

FILE *fopen(const char *__AXC_RESTRICT filename, const char *__AXC_RESTRICT mode) {
    FILE *f = __axc_new_file();
    if (f == NULL)
        return NULL;
    if (open_into(f, filename, mode) != 0) {
        __axc_free_file(f);
        return NULL;
    }
    __axc_link(f);
    return f;
}

/* 7.21.5.4: the stream's file is closed (a failure ignored), and the file
   opened into it. With a null filename the mode of the same file changes:
   a mode that needs what the file was not opened for fails; text and
   binary may change. */
FILE *freopen(const char *__AXC_RESTRICT filename, const char *__AXC_RESTRICT mode, FILE *__AXC_RESTRICT stream) {
    int r;
    LOCK(stream);
    if (filename == NULL) {
        unsigned int flags, oflags, have = stream->__flags & (F_READ | F_WRITE);
        if (stream->__flags & F_UNSET)
            __axc_setup(stream);
        if (__axc_parse_mode(mode, &flags, &oflags) != 0 || (oflags & 8) ||
            (flags & (F_READ | F_WRITE) & ~have) != 0 || (stream->__flags & F_CLOSED)) {
            __axc_close_u(stream, 0);
            UNLOCK(stream);
#ifdef EINVAL
            SET_ERRNO(EINVAL);
#endif
            return NULL;
        }
        __axc_flush_u(stream);
        stream->__flags = (stream->__flags & ~(F_READ | F_WRITE | F_APPEND | F_TEXT | F_EOF | F_ERR)) | flags;
        stream->__orient = O_NONE;
        UNLOCK(stream);
        return stream;
    }
    __axc_close_u(stream, 0);
    r = open_into(stream, filename, mode);
    UNLOCK(stream);
    if (r != 0)
        return NULL;
    /* (a standard stream that fclose closed is back in the list) */
    if (stream->__prev == NULL && __axc_files != stream)
        __axc_link(stream);
    return stream;
}

int fclose(FILE *stream) {
    int r;
    LOCK(stream);
    r = __axc_close_u(stream, 0);
    UNLOCK(stream);
    /* (out of the list after the lock: a standard stream stays an object) */
    __axc_unlink(stream);
    if (!(stream->__flags & F_STATIC))
        __axc_free_file(stream);
    return r;
}

int fflush(FILE *stream) {
    int r = 0;
    if (stream == NULL) {
        /* every stream (7.21.5.2p3: those whose last operation was output) */
        FILE *f;
        __axc_sys_lock(&__axc_files_lock);
        for (f = __axc_files; f != NULL; f = f->__next) {
            if (!(f->__flags & F_WR))
                continue;
            LOCK(f);
            if (__axc_flush_u(f) != 0)
                r = EOF;
            UNLOCK(f);
        }
        __axc_sys_unlock(&__axc_files_lock);
        return r;
    }
    LOCK(stream);
    /* (an input stream gives its buffer back to the file: impl.h) */
    if (__axc_flush_u(stream) != 0)
        r = EOF;
    UNLOCK(stream);
    return r;
}

/* 7.21.5.6: before any other operation on the stream. A null buf with a
   buffered mode has the library allocate size bytes (BUFSIZ for 0). */
int setvbuf(FILE *__AXC_RESTRICT stream, char *__AXC_RESTRICT buf, int mode, size_t size) {
    if (mode != _IOFBF && mode != _IOLBF && mode != _IONBF)
        return -1;
    LOCK(stream);
    __axc_flush_u(stream);
    if (stream->__flags & F_WR) {
        stream->__flags &= ~F_WR;
        stream->__wpos = stream->__wend = NULL;
    }
    if (stream->__flags & F_OWNBUF)
        __axc_sys_free(stream->__buf, stream->__size);
    stream->__flags &= ~(F_OWNBUF | F_LBF | F_NBF);
    stream->__flags |= F_SETBUF;
    if (mode == _IONBF || (buf != NULL && size == 0)) {
        stream->__flags |= F_NBF;
        stream->__buf = stream->__one;
        stream->__size = 1;
    } else {
        if (mode == _IOLBF)
            stream->__flags |= F_LBF;
        stream->__buf = (unsigned char *)buf;
        stream->__size = size;
    }
    UNLOCK(stream);
    return 0;
}

void setbuf(FILE *__AXC_RESTRICT stream, char *__AXC_RESTRICT buf) {
    if (buf == NULL)
        setvbuf(stream, NULL, _IONBF, 0);
    else
        setvbuf(stream, buf, _IOFBF, BUFSIZ);
}

int remove(const char *filename) {
    int r = __axc_sys_remove(filename);
    if (r < 0) {
        SET_ERRNO(__axc_errno_of(r));
        return -1;
    }
    return 0;
}

int rename(const char *old, const char *new_name) {
    int r = __axc_sys_rename(old, new_name);
    if (r < 0) {
        SET_ERRNO(__axc_errno_of(r));
        return -1;
    }
    return 0;
}

/* A file name in the directory for temporary files, with 12 random
   letters and digits, into buf (L_tmpnam bytes); 0, or -1. */
static int temp_name(char *buf) {
    static const char digits[] = "0123456789abcdefghijklmnopqrstuvwxyz";
    unsigned char rnd[12];
    ptrdiff_t n = __axc_sys_tmpdir(buf, L_tmpnam);
    int k;
    if (n < 0 || n + 17 > L_tmpnam || __axc_sys_random(rnd, sizeof rnd) != 0)
        return -1;
    buf[n++] = 'a';
    buf[n++] = 'x';
    buf[n++] = 'c';
    buf[n++] = '-';
    for (k = 0; k < 12; k++)
        buf[n++] = digits[rnd[k] % 36];
    buf[n] = 0;
    return 0;
}

/* 7.21.4.4: a name that is not the name of an existing file (one that
   cannot be opened for reading, then). */
char *tmpnam(char *s) {
    static char name[L_tmpnam];
    char *buf = s != NULL ? s : name;
    int tries;
    for (tries = 0; tries < 100; tries++) {
        int fd;
        if (temp_name(buf) != 0)
            return NULL;
        fd = __axc_sys_open(buf, 0);
        if (fd < 0)
            return buf;
        __axc_sys_close(fd);
    }
    return NULL;
}

/* 7.21.4.3: a new file, opened "wb+", removed when it is closed or when
   the program ends normally. Elsewhere than on Windows it is removed at
   once (the open file stays); on Windows it is removed when closed (and by
   __axc_stdio_exit). */
FILE *tmpfile(void) {
    char name[L_tmpnam];
    int tries, fd = -1;
    FILE *f;
    for (tries = 0; tries < 100 && fd < 0; tries++) {
        if (temp_name(name) != 0)
            break;
        fd = __axc_sys_open(name, 4 | 8);
        if (fd < 0 && fd != -17) {
            SET_ERRNO(__axc_errno_of(fd));
            return NULL;
        }
    }
    if (fd < 0) {
#ifdef EEXIST
        SET_ERRNO(EEXIST);
#endif
        return NULL;
    }
    f = __axc_new_file();
    if (f == NULL) {
        __axc_sys_close(fd);
        __axc_sys_remove(name);
        return NULL;
    }
    f->__fd = fd;
    f->__flags = F_READ | F_WRITE;
    if (__axc_is_windows()) {
        size_t n, k;
        for (n = 0; name[n] != 0; n++)
            ;
        f->__tmpname = __axc_sys_alloc(n + 1);
        if (f->__tmpname != NULL) {
            for (k = 0; k <= n; k++)
                f->__tmpname[k] = name[k];
        }
        __axc_at_exit_flush = __axc_stdio_exit;
    } else {
        __axc_sys_remove(name);
    }
    __axc_link(f);
    return f;
}
