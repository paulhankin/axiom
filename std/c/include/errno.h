/* errno.h - errors (C89 4.1.3, C99 7.5, C11 7.5, C23 7.5), for the Axiom
   compiler's C library. The values are the library's own, the same on
   every target (they are not the system's: the library translates), and
   the names past the standard's three are those other parts of the library
   report (names beginning with E and an upper-case letter are reserved for
   this header in every standard, 7.31.3). */
/* (Annex K: whether its names are asked for, K.3.1.1) */
#include <axc/ext1.h>
#ifndef __AXIOM_ERRNO_H
#define __AXIOM_ERRNO_H

/* errno: an lvalue of type int; through one function, the storage's owner
   (std/c/lib/core/__axc_errno.c), so that it becomes per-thread there */
int *__axc_errno(void);
#define errno (*__axc_errno())

#if __AXC_EXT1
/* errno_t (K.3.2) */
#define __AXIOM_WANT_ERRNO_T
#include <axc/types.h>
#endif

#define EDOM 33
#define ERANGE 34
#define EILSEQ 84

#define EPERM 1
#define ENOENT 2
#define ESRCH 3
#define EINTR 4
#define EIO 5
#define ENXIO 6
#define E2BIG 7
#define ENOEXEC 8
#define EBADF 9
#define ECHILD 10
#define EAGAIN 11
#define ENOMEM 12
#define EACCES 13
#define EFAULT 14
#define EBUSY 16
#define EEXIST 17
#define EXDEV 18
#define ENODEV 19
#define ENOTDIR 20
#define EISDIR 21
#define EINVAL 22
#define ENFILE 23
#define EMFILE 24
#define ENOTTY 25
#define ETXTBSY 26
#define EFBIG 27
#define ENOSPC 28
#define ESPIPE 29
#define EROFS 30
#define EMLINK 31
#define EPIPE 32
#define EDEADLK 35
#define ENAMETOOLONG 36
#define ENOLCK 37
#define ENOSYS 38
#define ENOTEMPTY 39
#define ELOOP 40
#define EWOULDBLOCK EAGAIN
#define EOVERFLOW 75
#define ENOTSUP 95
#define ETIMEDOUT 110

#endif
