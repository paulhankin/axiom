/* file.c - streams (C17 7.21.2, 7.21.3, 7.21.5): the standard streams, the
   buffers, opening and closing, reading and writing through the buffer,
   positions. The design is in impl.h. */
#include "impl.h"

static unsigned char stdin_buf[BUFSIZ];
static unsigned char stdout_buf[BUFSIZ];

/* The standard streams (7.21.3p7): stdin and stdout are fully buffered
   unless they refer to an interactive device, then line buffered (decided
   at their first use: F_UNSET); stderr is unbuffered. They are text
   streams. */
FILE __axc_stdin = {
    NULL, NULL, NULL, NULL, NULL, stdin_buf, BUFSIZ, F_READ | F_STATIC | F_UNSET, 0, 0, O_NONE, NULL, NULL,
    {0}, {0, 0, 0, 0}, 0, &__axc_stdout, NULL, NULL, {0}};
FILE __axc_stdout = {
    NULL, NULL, NULL, NULL, NULL, stdout_buf, BUFSIZ, F_WRITE | F_STATIC | F_UNSET, 1, 0, O_NONE, NULL, NULL,
    {0}, {0, 0, 0, 0}, 0, &__axc_stderr, &__axc_stdin, NULL, {0}};
FILE __axc_stderr = {
    NULL, NULL, NULL, NULL, NULL, NULL, 0, F_WRITE | F_STATIC | F_UNSET | F_NBF, 2, 0, O_NONE, NULL, NULL,
    {0}, {0, 0, 0, 0}, 0, NULL, &__axc_stdout, NULL, {0}};

/* Every open stream, the standard ones first; fopen adds at the end of
   the list, and fclose takes out. */
FILE *__axc_files = &__axc_stdin;
unsigned __axiom_int64 __axc_files_lock;

/* The errno value of a host ABI code (std/c/rt/stdio.ax). */
int __axc_errno_of(int code) {
    switch (code) {
#ifdef ENOENT
    case -2:
        return ENOENT;
#endif
#ifdef EACCES
    case -13:
        return EACCES;
#endif
#ifdef EEXIST
    case -17:
        return EEXIST;
#endif
#ifdef ENOTDIR
    case -20:
        return ENOTDIR;
#endif
#ifdef EISDIR
    case -21:
        return EISDIR;
#endif
#ifdef ENOTEMPTY
    case -39:
        return ENOTEMPTY;
#endif
    default:
#ifdef EIO
        return EIO;
#else
        return EDOM;
#endif
    }
}

/* (atomic: threads may ask first at once, and all store the same value) */
static _Atomic int windows = -1;

/* Is this Windows (where text streams translate line ends)? */
int __axc_is_windows(void) {
    int w = windows;
    if (w < 0)
        windows = w = __axc_sys_os() == 3;
    return w;
}

/* The first use of a standard stream: its buffering (unless setvbuf chose
   it) and, on Windows, the translation of line ends. */
void __axc_setup(FILE *f) {
    unsigned int fl = f->__flags;
    if (!(fl & F_UNSET))
        return;
    fl &= ~F_UNSET;
    if (__axc_is_windows())
        fl |= F_TEXT;
    if (!(fl & (F_SETBUF | F_NBF)) && __axc_sys_isatty(f->__fd))
        fl |= F_LBF;
    if ((fl & F_NBF) && f->__buf == NULL) {
        f->__buf = f->__one;
        f->__size = 1;
    }
    f->__flags = fl;
}

/* The stream's buffer, allocated at its first use (or the stream becomes
   unbuffered if there is no memory). */
static void need_buffer(FILE *f) {
    if (f->__buf != NULL)
        return;
    if (!(f->__flags & F_NBF)) {
        if (f->__size == 0)
            f->__size = BUFSIZ;
        f->__buf = __axc_sys_alloc(f->__size);
        if (f->__buf != NULL) {
            f->__flags |= F_OWNBUF;
            return;
        }
        f->__flags |= F_NBF;
    }
    f->__buf = f->__one;
    f->__size = 1;
}

/* Writes n bytes to the file, translating '\n' to "\r\n" in a text stream
   on Windows; 0, or EOF after a failure (the error indicator set). */
static int write_out(FILE *f, const unsigned char *p, size_t n) {
    if ((f->__flags & (F_APPEND | F_FDAPPEND)) == F_APPEND && n > 0)
        __axc_sys_seek(f->__fd, 0, SEEK_END); /* (freopen(NULL, "a", ...)) */
    if (f->__flags & F_TEXT) {
        static const unsigned char crlf[2] = {'\r', '\n'};
        size_t i = 0, start = 0;
        for (; i < n; i++) {
            if (p[i] != '\n')
                continue;
            if (__axc_sys_write(f->__fd, p + start, i - start) != i - start ||
                __axc_sys_write(f->__fd, crlf, 2) != 2)
                goto fail;
            start = i + 1;
        }
        p += start;
        n -= start;
    }
    if (n == 0 || __axc_sys_write(f->__fd, p, n) == n)
        return 0;
fail:
    f->__flags |= F_ERR;
#ifdef EIO
    SET_ERRNO(EIO);
#endif
    return EOF;
}

/* The bytes read and not taken (pushed back ones included), which a
   position must not count. */
static __axiom_int64 unread(FILE *f) {
    if (f->__flags & F_BACK)
        return (f->__rend - f->__rpos) + (f->__dend - f->__save_rpos);
    return f->__dend - f->__rpos;
}

/* Gives the read buffer back to the file: the file's position becomes the
   stream's (if it can be positioned), the buffer and pushed back bytes
   are dropped. */
static void drop_read(FILE *f) {
    __axiom_int64 n = unread(f);
    if (n != 0 && !(f->__flags & F_NOSEEK)) {
        if (__axc_sys_seek(f->__fd, -n, SEEK_CUR) < 0)
            f->__flags |= F_NOSEEK;
    }
    f->__flags &= ~(F_RD | F_BACK);
    f->__rpos = f->__rend = f->__dend = NULL;
}

/* fflush's work on one stream (its lock held): 0, or EOF after an error. */
int __axc_flush_u(FILE *f) {
    if (f->__flags & F_WR) {
        size_t n = (size_t)(f->__wpos - f->__buf);
        f->__wpos = f->__buf;
        if (n != 0 && write_out(f, f->__buf, n) != 0)
            return EOF;
        return 0;
    }
    if (f->__flags & F_RD)
        drop_read(f);
    return 0;
}

/* Into the writing state (7.21.5.3p7 lets output follow input only after
   a seek; here it may at any time). 0, or EOF if the stream cannot be
   written (the error indicator set). */
int __axc_to_write(FILE *f) {
    if (f->__flags & F_UNSET)
        __axc_setup(f);
    if (!(f->__flags & F_WRITE)) {
        f->__flags |= F_ERR;
#ifdef EBADF
        SET_ERRNO(EBADF);
#endif
        return EOF;
    }
    if (f->__flags & F_RD)
        drop_read(f);
    need_buffer(f);
    __axc_at_exit_flush = __axc_stdio_exit;
    f->__flags |= F_WR;
    f->__wpos = f->__buf;
    /* (unbuffered: every byte takes the slow path, which writes it) */
    f->__wend = (f->__flags & F_NBF) ? f->__buf : f->__buf + f->__size;
    return 0;
}

/* Flushes the line-buffered streams that are writing (7.21.3p3: input
   from an unbuffered or line-buffered stream that needs the host
   environment's characters flushes them). The stream f is locked. */
static void flush_line_buffered(FILE *f) {
    FILE *g;
    __axc_sys_lock(&__axc_files_lock);
    for (g = __axc_files; g != NULL; g = g->__next) {
        if (g == f || !(g->__flags & F_LBF) || !(g->__flags & F_WR))
            continue;
        LOCK(g);
        __axc_flush_u(g);
        UNLOCK(g);
    }
    __axc_sys_unlock(&__axc_files_lock);
}

/* Into the reading state; 0, or EOF if the stream cannot be read. */
int __axc_to_read(FILE *f) {
    if (f->__flags & F_UNSET)
        __axc_setup(f);
    if (!(f->__flags & F_READ)) {
        f->__flags |= F_ERR;
#ifdef EBADF
        SET_ERRNO(EBADF);
#endif
        return EOF;
    }
    if (f->__flags & F_WR) {
        if (__axc_flush_u(f) != 0)
            return EOF;
        f->__flags &= ~F_WR;
        f->__wpos = f->__wend = NULL;
    }
    need_buffer(f);
    if ((f->__flags & F_TEXT) && f->__size < 2) {
        /* (a '\r' and the byte after it must fit: impl.h) */
        if (f->__flags & F_OWNBUF)
            __axc_sys_free(f->__buf, f->__size);
        f->__flags &= ~F_OWNBUF;
        f->__buf = f->__one;
        f->__size = 2;
    }
    f->__flags |= F_RD;
    f->__rpos = f->__rend = f->__dend = f->__buf;
    return 0;
}

/* The end of the bytes the fast path may take from p on: before the next
   '\r' of a text stream (impl.h). */
static unsigned char *take_end(FILE *f, unsigned char *p) {
    unsigned char *e = f->__dend;
    if (f->__flags & F_TEXT) {
        while (p < e && *p != '\r')
            p++;
        return p;
    }
    return e;
}

/* More bytes for a reading stream whose __rpos has reached __rend: 0 if
   there are (__rpos < __rend), else EOF with the end-of-file or error
   indicator set. */
int __axc_refill(FILE *f) {
    ptrdiff_t n;
    size_t keep = 0;
    if (f->__flags & F_BACK) {
        /* the pushed back bytes are taken: back to the buffer */
        f->__flags &= ~F_BACK;
        f->__rpos = f->__save_rpos;
        f->__rend = take_end(f, f->__rpos);
        if (f->__rpos < f->__rend)
            return 0;
    }
    if (f->__rpos < f->__dend) {
        /* a text stream at a '\r': "\r\n" is read as '\n' */
        if (f->__rpos + 1 < f->__dend) {
            if (f->__rpos[1] == '\n')
                f->__rpos++;
            f->__rend = take_end(f, f->__rpos + 1);
            return 0;
        }
        /* the '\r' ends the buffer: keep it, and read what follows */
        f->__buf[0] = '\r';
        keep = 1;
    }
    if (f->__flags & F_EOF)
        return EOF;
    if (f->__flags & (F_LBF | F_NBF))
        flush_line_buffered(f);
    n = __axc_sys_read(f->__fd, f->__buf + keep, f->__size - keep);
    if (n <= 0) {
        if (keep) {
            /* a '\r' at the end of the file is itself */
            f->__rpos = f->__buf;
            f->__dend = f->__rend = f->__buf + 1;
            return 0;
        }
        f->__rpos = f->__rend = f->__dend = f->__buf;
        if (n == 0) {
            f->__flags |= F_EOF;
        } else {
            f->__flags |= F_ERR;
#ifdef EIO
            SET_ERRNO(EIO);
#endif
        }
        return EOF;
    }
    f->__rpos = f->__buf;
    f->__dend = f->__buf + keep + n;
    if (keep) {
        if (f->__buf[1] == '\n')
            f->__rpos++;
        f->__rend = take_end(f, f->__rpos + 1);
    } else {
        f->__rend = take_end(f, f->__rpos);
    }
    return 0;
}

/* getc's slow path (the lock held): the next byte, or EOF. */
int __axc_uflow(FILE *f) {
    if (!(f->__flags & F_RD) && __axc_to_read(f) != 0)
        return EOF;
    /* (7.21.7.1p3: with the end-of-file indicator set, EOF at once:
       __axc_refill) */
    while (f->__rpos >= f->__rend) {
        if (__axc_refill(f) != 0)
            return EOF;
    }
    return *f->__rpos++;
}

/* putc's slow path (the lock held): c written, or EOF. */
int __axc_oflow(FILE *f, int c) {
    unsigned char b = (unsigned char)c;
    if (!(f->__flags & F_WR) && __axc_to_write(f) != 0)
        return EOF;
    if (f->__flags & F_NBF)
        return write_out(f, &b, 1) == 0 ? b : EOF;
    if (f->__wpos >= f->__wend && __axc_flush_u(f) != 0)
        return EOF;
    *f->__wpos++ = b;
    if (b == '\n' && (f->__flags & F_LBF) && __axc_flush_u(f) != 0)
        return EOF;
    return b;
}

/* Copies n bytes into the buffer, flushing it when full (a whole buffer
   or more goes to the file at once); the bytes taken. */
static size_t put_buffered(FILE *f, const unsigned char *p, size_t n) {
    size_t room, done = 0;
    while (done < n) {
        room = (size_t)(f->__wend - f->__wpos);
        if (room == 0) {
            if (__axc_flush_u(f) != 0)
                return done;
            room = (size_t)(f->__wend - f->__wpos);
        }
        if (f->__wpos == f->__buf && n - done >= f->__size) {
            /* a whole buffer or more: no copy */
            if (write_out(f, p + done, n - done) != 0)
                return done;
            return n;
        }
        if (room > n - done)
            room = n - done;
        __axc_sys_copy(f->__wpos, p + done, room);
        f->__wpos += room;
        done += room;
    }
    return done;
}

/* fwrite's work (the lock held): the bytes written. A line-buffered
   stream writes up to the last new-line and keeps the rest. */
size_t __axc_write_u(FILE *f, const unsigned char *p, size_t n) {
    size_t line = 0, done;
    if (!(f->__flags & F_WR) && __axc_to_write(f) != 0)
        return 0;
    if (f->__flags & F_NBF)
        return write_out(f, p, n) == 0 ? n : 0;
    if (f->__flags & F_LBF) {
        size_t k = n;
        while (k > 0 && p[k - 1] != '\n')
            k--;
        line = k;
    }
    if (line > 0) {
        done = put_buffered(f, p, line);
        if (done != line || __axc_flush_u(f) != 0)
            return done;
        return line + put_buffered(f, p + line, n - line);
    }
    return put_buffered(f, p, n);
}

/* fread's work (the lock held): the bytes read. */
size_t __axc_read_u(FILE *f, unsigned char *p, size_t n) {
    size_t done = 0;
    if (!(f->__flags & F_RD) && __axc_to_read(f) != 0)
        return 0;
    while (done < n) {
        size_t avail = (size_t)(f->__rend - f->__rpos);
        if (avail > 0) {
            if (avail > n - done)
                avail = n - done;
            __axc_sys_copy(p + done, f->__rpos, avail);
            f->__rpos += avail;
            done += avail;
            continue;
        }
        if (!(f->__flags & (F_BACK | F_TEXT | F_EOF)) && f->__rpos >= f->__dend && n - done >= f->__size) {
            /* a whole buffer or more: straight into p */
            ptrdiff_t r = __axc_sys_read(f->__fd, p + done, n - done);
            if (r > 0) {
                done += (size_t)r;
                continue;
            }
            if (r == 0) {
                f->__flags |= F_EOF;
            } else {
                f->__flags |= F_ERR;
#ifdef EIO
                SET_ERRNO(EIO);
#endif
            }
            break;
        }
        {
            int c = __axc_uflow(f);
            if (c == EOF)
                break;
            p[done++] = (unsigned char)c;
        }
    }
    return done;
}

/* Sets the stream's orientation if it has none (7.21.2p4); the
   orientation it then has. */
int __axc_orient(FILE *f, int o) {
    if (f->__orient == O_NONE)
        f->__orient = o;
    return f->__orient;
}

/* fseek's work (the lock held); 0, or -1 with errno set. */
int __axc_seek_u(FILE *f, __axiom_int64 off, int whence) {
    __axiom_int64 r;
    if (f->__flags & F_UNSET)
        __axc_setup(f);
    if (whence != SEEK_SET && whence != SEEK_CUR && whence != SEEK_END) {
#ifdef EINVAL
        SET_ERRNO(EINVAL);
#endif
        return -1;
    }
    if ((f->__flags & F_WR) && __axc_flush_u(f) != 0)
        return -1;
    if (whence == SEEK_CUR) {
        __axiom_int64 here = __axc_tell_u(f);
        if (here < 0)
            return -1;
        off += here;
        whence = SEEK_SET;
    }
    if (f->__flags & F_RD) {
        f->__flags &= ~(F_RD | F_BACK);
        f->__rpos = f->__rend = f->__dend = NULL;
    }
    if (f->__flags & F_WR) {
        f->__flags &= ~F_WR;
        f->__wpos = f->__wend = NULL;
    }
    if (whence == SEEK_SET && off < 0) {
#ifdef EINVAL
        SET_ERRNO(EINVAL);
#endif
        return -1;
    }
    r = __axc_sys_seek(f->__fd, off, whence);
    if (r < 0) {
#ifdef ESPIPE
        SET_ERRNO(ESPIPE);
#endif
        return -1;
    }
    f->__flags &= ~(F_EOF | F_NOSEEK);
    return 0;
}

/* ftell's work (the lock held): the position, or -1 with errno set. */
__axiom_int64 __axc_tell_u(FILE *f) {
    __axiom_int64 pos;
    if (f->__flags & F_UNSET)
        __axc_setup(f);
    if ((f->__flags & (F_WR | F_APPEND)) == (F_WR | F_APPEND)) {
        /* appended: at the end, after what is buffered */
        if (__axc_flush_u(f) != 0)
            return -1;
        pos = __axc_sys_seek(f->__fd, 0, SEEK_END);
    } else {
        pos = __axc_sys_seek(f->__fd, 0, SEEK_CUR);
    }
    if (pos < 0) {
#ifdef ESPIPE
        SET_ERRNO(ESPIPE);
#endif
        return -1;
    }
    if (f->__flags & F_RD) {
        pos -= unread(f);
        if (pos < 0)
            pos = 0;
    } else if (f->__flags & F_WR) {
        const unsigned char *q;
        pos += f->__wpos - f->__buf;
        if (f->__flags & F_TEXT) {
            for (q = f->__buf; q < f->__wpos; q++)
                pos += *q == '\n';
        }
    }
    return pos;
}

/* Parses fopen's mode (7.21.5.3p3): the stream's flags and ax_open's. 0,
   or -1 if the mode is not one of the standard's. A mode is r, w or a,
   then + (update), b (binary) and x (with w: the file must not exist, C11)
   in any order; what follows another character is ignored. */
int __axc_parse_mode(const char *mode, unsigned int *flags, unsigned int *open_flags) {
    int plus = 0, bin = 0, excl = 0;
    char m = *mode++;
    for (;; mode++) {
        if (*mode == '+' && !plus)
            plus = 1;
        else if (*mode == 'b' && !bin)
            bin = 1;
        else if (*mode == 'x' && !excl)
            excl = 1;
        else
            break;
    }
    switch (m) {
    case 'r':
        *flags = plus ? F_READ | F_WRITE : F_READ;
        *open_flags = plus ? 3 : 0;
        break;
    case 'w':
        *flags = plus ? F_READ | F_WRITE : F_WRITE;
        *open_flags = plus ? 4 : 1;
        break;
    case 'a':
        *flags = (plus ? F_READ | F_WRITE : F_WRITE) | F_APPEND;
        *open_flags = plus ? 5 : 2;
        break;
    default:
        return -1;
    }
    if (excl) {
        if (m != 'w')
            return -1;
        *open_flags |= 8;
    }
    if (!bin && __axc_is_windows())
        *flags |= F_TEXT;
    return 0;
}

/* A new stream, not yet in the list. */
FILE *__axc_new_file(void) {
    FILE *f = __axc_sys_alloc(sizeof(FILE));
    unsigned char *p = (unsigned char *)f;
    size_t k;
    if (f == NULL) {
#ifdef ENOMEM
        SET_ERRNO(ENOMEM);
#endif
        return NULL;
    }
    for (k = 0; k < sizeof(FILE); k++)
        p[k] = 0;
    f->__fd = -1;
    return f;
}

void __axc_free_file(FILE *f) {
    __axc_sys_free(f, sizeof(FILE));
}

/* Adds f to the end of the list of open streams. */
void __axc_link(FILE *f) {
    FILE *g;
    __axc_sys_lock(&__axc_files_lock);
    f->__next = NULL;
    f->__prev = NULL;
    if (__axc_files == NULL) {
        __axc_files = f;
    } else {
        for (g = __axc_files; g->__next != NULL; g = g->__next)
            ;
        g->__next = f;
        f->__prev = g;
    }
    __axc_sys_unlock(&__axc_files_lock);
}

void __axc_unlink(FILE *f) {
    __axc_sys_lock(&__axc_files_lock);
    if (f->__prev != NULL)
        f->__prev->__next = f->__next;
    else if (__axc_files == f)
        __axc_files = f->__next;
    if (f->__next != NULL)
        f->__next->__prev = f->__prev;
    f->__next = f->__prev = NULL;
    __axc_sys_unlock(&__axc_files_lock);
}

/* Closes f's file (the lock held): flushes it, closes the descriptor,
   frees the buffer, removes a temporary file (tmpfile on Windows). The
   stream stays in the list unless free_it (then it is freed, unless it is
   a standard stream). 0, or EOF if the flush failed. */
int __axc_close_u(FILE *f, int free_it) {
    int r = __axc_flush_u(f);
    size_t n;
    if (f->__fd >= 0)
        __axc_sys_close(f->__fd);
    f->__fd = -1;
    if (f->__flags & F_OWNBUF)
        __axc_sys_free(f->__buf, f->__size);
    if (f->__tmpname != NULL) {
        __axc_sys_remove(f->__tmpname);
        for (n = 0; f->__tmpname[n] != 0; n++)
            ;
        __axc_sys_free(f->__tmpname, n + 1);
        f->__tmpname = NULL;
    }
    f->__buf = NULL;
    f->__size = 0;
    f->__flags = (f->__flags & F_STATIC) | F_CLOSED;
    f->__rpos = f->__rend = f->__dend = f->__wpos = f->__wend = NULL;
    f->__orient = O_NONE;
    if (free_it) {
        __axc_unlink(f);
        if (!(f->__flags & F_STATIC))
            __axc_free_file(f);
    }
    return r;
}

/* At exit (7.22.4.4p4): every open stream with unwritten data is flushed,
   every open stream closed, files tmpfile created removed. The standard
   streams' files are left open, for the runtime. */
void __axc_stdio_exit(void) {
    FILE *f, *next;
    __axc_sys_lock(&__axc_files_lock);
    for (f = __axc_files; f != NULL; f = next) {
        next = f->__next;
        if (f->__flags & F_WR)
            __axc_flush_u(f);
        if (f->__tmpname != NULL) {
            if (f->__fd >= 0)
                __axc_sys_close(f->__fd);
            f->__fd = -1;
            __axc_sys_remove(f->__tmpname);
        }
    }
    __axc_sys_unlock(&__axc_files_lock);
}
