/* flags: --target=wasm32 --c-std=gnu23 */
/* check-only */
/* The layout of the integers wider than 64 bits on --target=wasm32 (the WebAssembly C ABI): __int128
   is 16 bytes aligned to 16; _BitInt(N) for N > 64 is in 64-bit limbs, aligned to 8. */
#include <limits.h>
#include <stddef.h>

_Static_assert(sizeof(__int128) == 16 && _Alignof(__int128) == 16, "__int128");
_Static_assert(sizeof(unsigned __int128) == 16 && _Alignof(unsigned __int128) == 16, "unsigned __int128");
_Static_assert(sizeof(_BitInt(65)) == 16 && _Alignof(_BitInt(65)) == 8, "_BitInt(65)");
_Static_assert(sizeof(_BitInt(100)) == 16 && _Alignof(_BitInt(100)) == 8, "_BitInt(100)");
_Static_assert(sizeof(_BitInt(128)) == 16 && _Alignof(_BitInt(128)) == 8, "_BitInt(128)");
_Static_assert(sizeof(_BitInt(129)) == 24 && _Alignof(_BitInt(129)) == 8, "_BitInt(129)");
_Static_assert(sizeof(_BitInt(192)) == 24 && _Alignof(_BitInt(192)) == 8, "_BitInt(192)");
_Static_assert(sizeof(_BitInt(193)) == 32 && _Alignof(_BitInt(193)) == 8, "_BitInt(193)");
_Static_assert(sizeof(_BitInt(256)) == 32 && _Alignof(_BitInt(256)) == 8, "_BitInt(256)");
_Static_assert(sizeof(_BitInt(521)) == 72 && _Alignof(_BitInt(521)) == 8, "_BitInt(521)");
_Static_assert(sizeof(_BitInt(65535)) == 8192 && _Alignof(_BitInt(65535)) == 8, "_BitInt(65535)");
_Static_assert(sizeof(unsigned _BitInt(100)) == 16, "unsigned _BitInt(100)");
struct s1 { char c; __int128 x; };
struct s2 { char c; _BitInt(100) x; };
_Static_assert(offsetof(struct s1, x) == 16 && sizeof(struct s1) == 32, "a structure with __int128");
_Static_assert(offsetof(struct s2, x) == 8 && sizeof(struct s2) == 24, "a structure with _BitInt(100)");
_Static_assert(_Alignof(_Atomic __int128) == 16 && _Alignof(_Atomic _BitInt(128)) == 16, "atomic");
_Static_assert(BITINT_MAXWIDTH == 65535, "BITINT_MAXWIDTH");
_Static_assert(_Alignof(max_align_t) >= 8, "max_align_t");
int main(void) { return 0; }
