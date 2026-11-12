/* flags: --target=wasm32 */
/* check-only */
/* The implementation-defined choices of --target=wasm32 by default: those of
   the WebAssembly C ABI (32-bit pointers, long). Checked when compiled (the
   test is not run). */
#include <float.h>
#include <limits.h>
#include <stddef.h>
#include <stdint.h>

_Static_assert(CHAR_BIT == 8 && sizeof(short) == 2 && sizeof(int) == 4 && sizeof(long long) == 8, "the widths");
_Static_assert((CHAR_MIN < 0) == 1 && ((char)-1 < 0) == 1, "plain char's signedness");
_Static_assert(sizeof(long) == 4 && LONG_MAX == 2147483647L, "long");
_Static_assert(sizeof(void *) == 4 && sizeof(size_t) == 4 && sizeof(ptrdiff_t) == 4, "pointers, size_t");
_Static_assert(sizeof(intptr_t) == 4 && sizeof(uintptr_t) == 4, "intptr_t");
_Static_assert(sizeof(wchar_t) == 4 && sizeof(L'a') == 4, "wchar_t's width");
_Static_assert(((wchar_t)-1 < 0) == 1 && (WCHAR_MIN < 0) == 1, "wchar_t's signedness");
_Static_assert(_Alignof(long long) == 8 && _Alignof(double) == 8, "alignment");
_Static_assert(sizeof(long double) == 16 && _Alignof(long double) == 16 && LDBL_MANT_DIG == 113, "long double: binary128");
_Static_assert(sizeof(int64_t) == 8 && INT64_MAX == 9223372036854775807LL, "int64_t");

int main(void) { return 0; }
