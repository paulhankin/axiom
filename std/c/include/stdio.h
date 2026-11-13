/* stdio.h - input/output (C89 4.9, C99 7.19, C11 7.21, C17 7.21, C23 7.23),
   for the Axiom compiler's hosted C library (std/c/lib/stdio/). Names that
   a standard does not have are not declared in its mode: snprintf, vscanf
   and the other C99 functions from C99, gets before C11 only. */
/* (Annex K: whether its names are asked for, K.3.1.1) */
#include <axc/ext1.h>
#ifndef __AXIOM_STDIO_H
#define __AXIOM_STDIO_H

#if __STDC_VERSION__ >= 202311L
#define __STDC_VERSION_STDIO_H__ 202311L
#endif

/* size_t (defined once with the other headers: axc/types.h) */
#define __AXIOM_WANT_SIZE_T
#include <axc/types.h>

/* the format attribute, in every mode (the compiler's own spelling, a
   reserved identifier): the calls' format strings are checked against
   their arguments (--c-warn-format) */
#ifndef __AXC_FORMAT
#define __AXC_FORMAT(kind, f, a) __axiom_attribute__((__format__(kind, f, a)))
#endif

#ifndef NULL
#define NULL ((void *)0)
#endif

/* A stream. The members are the library's (std/c/lib/stdio/impl.h). */
struct __axc_file {
    unsigned char *__rpos, *__rend; /* reading: the next byte, the end of those to take */
    unsigned char *__dend;          /* reading: the end of those read */
    unsigned char *__wpos, *__wend; /* writing: the next free byte, the end of the room */
    unsigned char *__buf;
    size_t __size;
    unsigned int __flags;
    int __fd;
    unsigned __axiom_int64 __lock; /* (the low 32 bits: a lock word) */
    int __orient;
    unsigned char *__save_rpos, *__save_rend; /* the buffer's, while pushed back bytes are read */
    unsigned char __back[8];                  /* pushed back bytes, read from the end */
    unsigned __axiom_int64 __mbs[4];          /* a wide stream's conversion state (an mbstate_t) */
    __axiom_int64 __base;                     /* the file position of __buf[0] (text-mode reading) */
    struct __axc_file *__next, *__prev;
    char *__tmpname;
    unsigned char __one[4]; /* the buffer of an unbuffered stream */
};
typedef struct __axc_file FILE;

/* A file position: the offset and a wide stream's conversion state */
typedef struct {
    __axiom_int64 __pos;
    unsigned __axiom_int64 __mbs[4];
} fpos_t;

#define _IOFBF 0
#define _IOLBF 1
#define _IONBF 2
#define BUFSIZ 4096
#define EOF (-1)
#define FOPEN_MAX 20
#define FILENAME_MAX 4096
#define L_tmpnam 260
#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2
#define TMP_MAX 2147483647
#if __STDC_VERSION__ >= 202311L
/* (a NaN is printed nan or -nan, with no n-char-sequence) */
#define _PRINTF_NAN_LEN_MAX 0
#endif

extern FILE __axc_stdin, __axc_stdout, __axc_stderr;
#define stdin (&__axc_stdin)
#define stdout (&__axc_stdout)
#define stderr (&__axc_stderr)

/* 7.21.4 operations on files */
int remove(const char *);
int rename(const char *, const char *);
FILE *tmpfile(void);
char *tmpnam(char *);

/* 7.21.5 file access */
int fclose(FILE *);
int fflush(FILE *);
FILE *fopen(const char *__AXC_RESTRICT, const char *__AXC_RESTRICT);
FILE *freopen(const char *__AXC_RESTRICT, const char *__AXC_RESTRICT, FILE *__AXC_RESTRICT);
void setbuf(FILE *__AXC_RESTRICT, char *__AXC_RESTRICT);
int setvbuf(FILE *__AXC_RESTRICT, char *__AXC_RESTRICT, int, size_t);

/* 7.21.6 formatted input/output */
int fprintf(FILE *__AXC_RESTRICT, const char *__AXC_RESTRICT, ...) __AXC_FORMAT(printf, 2, 3);
int fscanf(FILE *__AXC_RESTRICT, const char *__AXC_RESTRICT, ...) __AXC_FORMAT(scanf, 2, 3);
int printf(const char *__AXC_RESTRICT, ...) __AXC_FORMAT(printf, 1, 2);
int scanf(const char *__AXC_RESTRICT, ...) __AXC_FORMAT(scanf, 1, 2);
int sprintf(char *__AXC_RESTRICT, const char *__AXC_RESTRICT, ...) __AXC_FORMAT(printf, 2, 3);
int sscanf(const char *__AXC_RESTRICT, const char *__AXC_RESTRICT, ...) __AXC_FORMAT(scanf, 2, 3);
int vfprintf(FILE *__AXC_RESTRICT, const char *__AXC_RESTRICT, __builtin_va_list) __AXC_FORMAT(printf, 2, 0);
int vprintf(const char *__AXC_RESTRICT, __builtin_va_list) __AXC_FORMAT(printf, 1, 0);
int vsprintf(char *__AXC_RESTRICT, const char *__AXC_RESTRICT, __builtin_va_list) __AXC_FORMAT(printf, 2, 0);
#if __STDC_VERSION__ >= 199901L
int snprintf(char *__AXC_RESTRICT, size_t, const char *__AXC_RESTRICT, ...) __AXC_FORMAT(printf, 3, 4);
int vfscanf(FILE *__AXC_RESTRICT, const char *__AXC_RESTRICT, __builtin_va_list) __AXC_FORMAT(scanf, 2, 0);
int vscanf(const char *__AXC_RESTRICT, __builtin_va_list) __AXC_FORMAT(scanf, 1, 0);
int vsnprintf(char *__AXC_RESTRICT, size_t, const char *__AXC_RESTRICT, __builtin_va_list) __AXC_FORMAT(printf, 3, 0);
int vsscanf(const char *__AXC_RESTRICT, const char *__AXC_RESTRICT, __builtin_va_list) __AXC_FORMAT(scanf, 2, 0);
#endif

#if __STDC_VERSION__ >= 202311L
/* C23 mode's formatted input: %i takes the 0b prefix (7.23.6.2p12) */
int __axc_fscanf_c23(FILE *restrict, const char *restrict, ...) __AXC_FORMAT(scanf, 2, 3);
int __axc_scanf_c23(const char *restrict, ...) __AXC_FORMAT(scanf, 1, 2);
int __axc_sscanf_c23(const char *restrict, const char *restrict, ...) __AXC_FORMAT(scanf, 2, 3);
int __axc_vfscanf_c23(FILE *restrict, const char *restrict, __builtin_va_list) __AXC_FORMAT(scanf, 2, 0);
int __axc_vscanf_c23(const char *restrict, __builtin_va_list) __AXC_FORMAT(scanf, 1, 0);
int __axc_vsscanf_c23(const char *restrict, const char *restrict, __builtin_va_list) __AXC_FORMAT(scanf, 2, 0);
#define fscanf(...) __axc_fscanf_c23(__VA_ARGS__)
#define scanf(...) __axc_scanf_c23(__VA_ARGS__)
#define sscanf(...) __axc_sscanf_c23(__VA_ARGS__)
#define vfscanf(...) __axc_vfscanf_c23(__VA_ARGS__)
#define vscanf(...) __axc_vscanf_c23(__VA_ARGS__)
#define vsscanf(...) __axc_vsscanf_c23(__VA_ARGS__)
#endif

/* 7.21.7 character input/output */
int fgetc(FILE *);
char *fgets(char *__AXC_RESTRICT, int, FILE *__AXC_RESTRICT);
int fputc(int, FILE *);
int fputs(const char *__AXC_RESTRICT, FILE *__AXC_RESTRICT);
int getc(FILE *);
int getchar(void);
#if !defined(__STDC_VERSION__) || __STDC_VERSION__ < 201112L
/* (removed by C11) */
char *gets(char *);
#endif
int putc(int, FILE *);
int putchar(int);
int puts(const char *);
int ungetc(int, FILE *);

/* getc, putc, getchar and putchar's fast paths (7.21.7.5p2, 7.21.7.8p2:
   getc and putc may be macros): a byte of a byte-oriented stream's buffer
   is taken or stored in line while the program has one thread (no thread
   was ever started: nothing else can use the stream); else, and when the
   buffer is empty or full (or a new-line ends a line-buffered stream's
   line), the function. Each macro evaluates its arguments once. */
__axiom_int64 __axc_sys_threaded(void);
static int __axc_getc_fast(FILE *__f) {
    return __f->__rpos < __f->__rend && __f->__orient < 0 && !__axc_sys_threaded() ? (int)*__f->__rpos++ : (getc)(__f);
}
static int __axc_putc_fast(int __c, FILE *__f) {
    return __f->__wpos < __f->__wend && __f->__orient < 0 && ((unsigned char)__c != '\n' || !(__f->__flags & 0x20u)) &&
                   !__axc_sys_threaded()
               ? (int)(*__f->__wpos++ = (unsigned char)__c)
               : (putc)(__c, __f);
}
#define getc(stream) __axc_getc_fast(stream)
#define putc(c, stream) __axc_putc_fast((c), (stream))
#define getchar() __axc_getc_fast(stdin)
#define putchar(c) __axc_putc_fast((c), stdout)

/* 7.21.8 direct input/output */
size_t fread(void *__AXC_RESTRICT, size_t, size_t, FILE *__AXC_RESTRICT);
size_t fwrite(const void *__AXC_RESTRICT, size_t, size_t, FILE *__AXC_RESTRICT);

/* 7.21.9 file positioning */
int fgetpos(FILE *__AXC_RESTRICT, fpos_t *__AXC_RESTRICT);
int fseek(FILE *, long int, int);
int fsetpos(FILE *, const fpos_t *);
long int ftell(FILE *);
void rewind(FILE *);

/* 7.21.10 error handling */
void clearerr(FILE *);
int feof(FILE *);
int ferror(FILE *);
void perror(const char *);

#if __AXC_EXT1
/* Annex K (K.3.5): the bounds-checking interfaces (std/c/lib/ext1/) */
#define __AXIOM_WANT_ERRNO_T
#define __AXIOM_WANT_SIZE_T
#define __AXIOM_WANT_RSIZE_T
#include <axc/types.h>
/* (tmpnam_s's names are tmpnam's) */
#define L_tmpnam_s L_tmpnam
#define TMP_MAX_S TMP_MAX
errno_t tmpfile_s(FILE *__AXC_RESTRICT *__AXC_RESTRICT __streamptr);
errno_t tmpnam_s(char *__s, rsize_t __maxsize);
errno_t fopen_s(FILE *__AXC_RESTRICT *__AXC_RESTRICT __streamptr, const char *__AXC_RESTRICT __filename,
                const char *__AXC_RESTRICT __mode);
errno_t freopen_s(FILE *__AXC_RESTRICT *__AXC_RESTRICT __newstreamptr, const char *__AXC_RESTRICT __filename,
                  const char *__AXC_RESTRICT __mode, FILE *__AXC_RESTRICT __stream);
int fprintf_s(FILE *__AXC_RESTRICT __stream, const char *__AXC_RESTRICT __format, ...);
int fscanf_s(FILE *__AXC_RESTRICT __stream, const char *__AXC_RESTRICT __format, ...);
int printf_s(const char *__AXC_RESTRICT __format, ...);
int scanf_s(const char *__AXC_RESTRICT __format, ...);
int snprintf_s(char *__AXC_RESTRICT __s, rsize_t __n, const char *__AXC_RESTRICT __format, ...);
int sprintf_s(char *__AXC_RESTRICT __s, rsize_t __n, const char *__AXC_RESTRICT __format, ...);
int sscanf_s(const char *__AXC_RESTRICT __s, const char *__AXC_RESTRICT __format, ...);
int vfprintf_s(FILE *__AXC_RESTRICT __stream, const char *__AXC_RESTRICT __format, __builtin_va_list __arg);
int vfscanf_s(FILE *__AXC_RESTRICT __stream, const char *__AXC_RESTRICT __format, __builtin_va_list __arg);
int vprintf_s(const char *__AXC_RESTRICT __format, __builtin_va_list __arg);
int vscanf_s(const char *__AXC_RESTRICT __format, __builtin_va_list __arg);
int vsnprintf_s(char *__AXC_RESTRICT __s, rsize_t __n, const char *__AXC_RESTRICT __format, __builtin_va_list __arg);
int vsprintf_s(char *__AXC_RESTRICT __s, rsize_t __n, const char *__AXC_RESTRICT __format, __builtin_va_list __arg);
int vsscanf_s(const char *__AXC_RESTRICT __s, const char *__AXC_RESTRICT __format, __builtin_va_list __arg);
char *gets_s(char *__s, rsize_t __n);
#if __STDC_VERSION__ >= 202311L
/* (C23 mode: %i takes the 0b prefix, as fscanf's does) */
int __axc_fscanf_s_c23(FILE *restrict, const char *restrict, ...);
int __axc_scanf_s_c23(const char *restrict, ...);
int __axc_sscanf_s_c23(const char *restrict, const char *restrict, ...);
int __axc_vfscanf_s_c23(FILE *restrict, const char *restrict, __builtin_va_list);
int __axc_vscanf_s_c23(const char *restrict, __builtin_va_list);
int __axc_vsscanf_s_c23(const char *restrict, const char *restrict, __builtin_va_list);
#define fscanf_s(...) __axc_fscanf_s_c23(__VA_ARGS__)
#define scanf_s(...) __axc_scanf_s_c23(__VA_ARGS__)
#define sscanf_s(...) __axc_sscanf_s_c23(__VA_ARGS__)
#define vfscanf_s(...) __axc_vfscanf_s_c23(__VA_ARGS__)
#define vscanf_s(...) __axc_vscanf_s_c23(__VA_ARGS__)
#define vsscanf_s(...) __axc_vsscanf_s_c23(__VA_ARGS__)
#endif
#endif

#endif
