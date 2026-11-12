/* stdio.h - input/output (C89 4.9, C99 7.19, C11 7.21, C17 7.21, C23 7.23),
   for the Axiom compiler's hosted C library (std/c/lib/stdio/). Names that
   a standard does not have are not declared in its mode: snprintf, vscanf
   and the other C99 functions from C99, gets before C11 only. */
#ifndef __AXIOM_STDIO_H
#define __AXIOM_STDIO_H

#if __STDC_VERSION__ >= 202311L
#define __STDC_VERSION_STDIO_H__ 202311L
#endif

/* size_t (defined once with the other headers: axc/types.h) */
#define __AXIOM_WANT_SIZE_T
#include <axc/types.h>

/* the format attribute in the GNU dialects (its arguments are checked, the
   calls not yet), nothing under an ISO standard */
#ifdef __AXIOM_GNU__
#define __AXC_FORMAT(kind, f, a) __attribute__((__format__(kind, f, a)))
#else
#define __AXC_FORMAT(kind, f, a)
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

#endif
