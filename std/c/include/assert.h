/* assert.h - diagnostics (C89 4.2, C99 7.2, C11 7.2, C23 7.2), for the
   Axiom compiler's C library. Included again, it defines assert again by
   the NDEBUG then in force (7.2p1): there is no guard. */
#if __STDC_VERSION__ >= 202311L
#define __STDC_VERSION_ASSERT_H__ 202311L
#endif

#undef assert

/* writes the message of a failed assertion to the standard error stream
   and calls abort (std/c/lib/core/__axc_assert_fail.c) */
#ifndef __AXIOM_ASSERT_FAIL_DECLARED
#define __AXIOM_ASSERT_FAIL_DECLARED
#if __STDC_VERSION__ >= 202311L
[[__noreturn__]]
#elif __STDC_VERSION__ >= 201112L
_Noreturn
#endif
void __axc_assert_fail(const char *__expr, const char *__file, long __line, const char *__func);
#endif

#if __STDC_VERSION__ >= 199901L
#define __AXIOM_ASSERT_FUNC __func__
#else
#define __AXIOM_ASSERT_FUNC ((const char *)0)
#endif

#ifdef NDEBUG
#if __STDC_VERSION__ >= 202311L
#define assert(...) ((void)0)
#else
#define assert(ignore) ((void)0)
#endif
#elif __STDC_VERSION__ >= 202311L
/* (C23 7.2.1.1: a variadic macro, so that a compound literal with commas
   is one argument) */
#define assert(...) ((__VA_ARGS__) ? (void)0 : __axc_assert_fail(#__VA_ARGS__, __FILE__, __LINE__, __AXIOM_ASSERT_FUNC))
#else
#define assert(expression) \
    ((expression) ? (void)0 : __axc_assert_fail(#expression, __FILE__, __LINE__, __AXIOM_ASSERT_FUNC))
#endif

/* C11 and C17 (7.2p3); a keyword in C23 */
#if __STDC_VERSION__ >= 201112L && __STDC_VERSION__ < 202311L
#define static_assert _Static_assert
#endif
