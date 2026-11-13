/* axc/ext1_impl.h - what the C library's Annex K functions (std/c/lib/ext1/,
   C11 K.3) share: the runtime-constraint handler and the checks. Private to
   the library; its units define __STDC_WANT_LIB_EXT1__ as 1 before they
   include a header.

   A runtime-constraint violation (K.3.1.4) calls the handler registered by
   set_constraint_handler_s (abort_handler_s until one is) with a message
   "function: what was wrong", a null pointer and the function's result
   (an errno_t: EINVAL for a null pointer or a zero size, ERANGE for a size
   out of range or too small, EINVAL for overlapping objects), or EINVAL for
   a function that does not return an errno_t; when the handler returns,
   the function does what K.3 says it does after a violation. */
#ifndef __AXIOM_AXC_EXT1_IMPL_H
#define __AXIOM_AXC_EXT1_IMPL_H

#include <stdlib.h>
#include <stdint.h>
#include <errno.h>
#include <wchar.h>

/* Calls the current runtime-constraint handler for a violation of the
   function fn's runtime-constraints (what: which), with error */
void __axc_ext1_violation(const char *__fn, const char *__what, errno_t __error);

/* The handler set_constraint_handler_s registered, or null for the
   default (__axc_ext1_handler.c) */
constraint_handler_t __axc_ext1_get_handler(void);
constraint_handler_t __axc_ext1_set_handler(constraint_handler_t __h);

/* The conversions of mbstowcs_s and mbsrtowcs_s, and of wcstombs_s and
   wcsrtombs_s, after their pointers are checked (fn: the caller's name) */
errno_t __axc_ext1_mbs(const char *__fn, size_t *__retval, wchar_t *__dst, rsize_t __dstmax, const char **__src, rsize_t __len,
                       mbstate_t *__ps);
errno_t __axc_ext1_wcs(const char *__fn, size_t *__retval, char *__dst, rsize_t __dstmax, const wchar_t **__src, rsize_t __len,
                       mbstate_t *__ps);

/* Whether [a, a + na) and [b, b + nb) share a byte */
static inline int __axc_ext1_overlap(const void *a, size_t na, const void *b, size_t nb) {
    uintptr_t x = (uintptr_t)a, y = (uintptr_t)b;
    return na > 0 && nb > 0 && x < y + nb && y < x + na;
}

#endif
