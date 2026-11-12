/* flags: --target=x86_64-windows */
/* check-only */
/* The implementation-defined choices of --target=x86_64-windows by default:
   those of the Windows x64 ABI: long is 32 bits, wchar_t is unsigned short.
   Checked when compiled (the test is not run). */
#include <limits.h>
#include <stddef.h>
#include <stdint.h>

_Static_assert(CHAR_BIT == 8 && sizeof(short) == 2 && sizeof(int) == 4 && sizeof(long long) == 8, "the widths");
_Static_assert((CHAR_MIN < 0) == 1 && ((char)-1 < 0) == 1, "plain char's signedness");
_Static_assert(sizeof(long) == 4 && LONG_MAX == 2147483647L, "long");
_Static_assert(sizeof(void *) == 8 && sizeof(size_t) == 8 && sizeof(ptrdiff_t) == 8, "pointers, size_t");
_Static_assert(sizeof(intptr_t) == 8 && sizeof(uintptr_t) == 8, "intptr_t");
_Static_assert(sizeof(wchar_t) == 2 && sizeof(L'a') == 2, "wchar_t's width");
_Static_assert(((wchar_t)-1 < 0) == 0 && (WCHAR_MIN < 0) == 0, "wchar_t's signedness");
_Static_assert(_Alignof(long long) == 8 && _Alignof(double) == 8 && sizeof(long double) == 8, "alignment; long double is double (docs/c-frontend.md)");
_Static_assert(sizeof(int64_t) == 8 && INT64_MAX == 9223372036854775807LL, "int64_t");

/* The exact types of the typedefs, as the platform's C ABI and C library
   have them (a declaration with them is compatible with the platform's) */
#define IS(x, t) _Generic((x), t: 1, default: 0)
_Static_assert(IS((size_t)0, unsigned long long) && IS(sizeof(int), unsigned long long) && IS((ptrdiff_t)0, long long), "size_t, ptrdiff_t");
_Static_assert(IS((char *)0 - (char *)0, long long), "a pointer difference");
_Static_assert(IS((intptr_t)0, long long) && IS((uintptr_t)0, unsigned long long), "intptr_t");
_Static_assert(IS((int64_t)0, long long) && IS((uint64_t)0, unsigned long long) && IS(INT64_C(1), long long), "int64_t");
_Static_assert(IS((intmax_t)0, long long) && IS((uintmax_t)0, unsigned long long) && IS(INTMAX_MAX, long long), "intmax_t");
_Static_assert(IS((int_fast16_t)0, int) && IS((int_fast32_t)0, int) && IS((uint_fast32_t)0, unsigned int), "int_fastN_t");
_Static_assert(IS(SIZE_MAX, unsigned long long) && IS(PTRDIFF_MIN, long long) && IS(UINT64_MAX, unsigned long long), "the limits' types");
_Static_assert(IS(WINT_MAX, int) && IS((wchar_t)0, unsigned short) && IS(L'a', unsigned short), "wint_t's limits, wchar_t");
_Static_assert(_Alignof(max_align_t) == 8, "max_align_t");

int main(void) { return 0; }
