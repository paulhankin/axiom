/* flags: --c-std=c11 */
/* The layout of atomic types (6.2.5p27): the size of the non-atomic
   version; a size of 1, 2, 4, 8 or 16 bytes is also the alignment (the
   psABIs' rule for _Atomic, on every target), others keep theirs. Checked
   against the platform compilers on x86-64 and AArch64 Linux. */
#include "host.h"
struct s1 { char a; };
struct s2 { char a[2]; };
struct s3 { char a[3]; };
struct s5 { char a[5]; };
struct s8 { char a[8]; };
struct s12 { int a[3]; };
struct s16 { long long a[2]; };
struct s24 { long long a[3]; };
struct s32 { char a[32]; };
#define L(T) _Static_assert(sizeof(_Atomic T) == sizeof(T), #T)
L(char); L(short); L(int); L(long); L(long long); L(float); L(double); L(void *); L(_Bool);
L(struct s1); L(struct s2); L(struct s3); L(struct s5); L(struct s8); L(struct s12); L(struct s16); L(struct s24);
L(struct s32);
#define A(T, n) _Static_assert(_Alignof(_Atomic T) == (n), #T)
A(char, 1); A(short, 2); A(int, 4); A(long long, 8); A(float, 4); A(double, 8); A(_Bool, 1);
A(struct s1, 1); A(struct s2, 2); A(struct s3, 1); A(struct s5, 1); A(struct s8, 8); A(struct s12, 4);
A(struct s16, 16); A(struct s24, _Alignof(long long)); A(struct s32, 1);
_Static_assert(_Alignof(_Atomic(void *)) == sizeof(void *), "pointer");
/* long double: 16 bytes aligned to 16 (x86-64 and AArch64 Linux,
   WebAssembly), or double's 8 (Windows, macOS); its complex type twice the
   size, aligned as it */
L(long double); L(long double _Complex); L(double _Complex); L(float _Complex);
#if __AXIOM_LDBL_WIDTH__ == 64
A(long double, 8); A(long double _Complex, 16);
#else
A(long double, 16); A(long double _Complex, 16);
#endif
A(float _Complex, 8); A(double _Complex, 16);
/* members of atomic type are placed by their alignment */
struct m { char c; _Atomic struct s2 p; char d; _Atomic struct s8 q; char e; _Atomic struct s16 r; };
_Static_assert(__builtin_offsetof(struct m, p) == 2 && __builtin_offsetof(struct m, q) == 8, "m");
_Static_assert(__builtin_offsetof(struct m, r) == 32 && sizeof(struct m) == 48 && _Alignof(struct m) == 16, "m");
/* qualifiers with _Atomic, and the specifier, are one type */
_Static_assert(_Generic((const _Atomic int *)0, _Atomic const int *: 1, default: 0), "q");
_Static_assert(_Generic((_Atomic(int) *)0, _Atomic int *: 1, default: 0), "s");
_Atomic struct s16 g16;
_Atomic struct s8 g8[3];
int main(void) {
    _Atomic struct s16 l16;
    print_i64((unsigned long long)&g16 % 16 + (unsigned long long)&l16 % 16);
    print_i64((unsigned long long)&g8[1] % 8);
    print_i64(sizeof(struct m));
    return 0;
}
/* expect: 0 */
/* expect: 0 */
/* expect: 48 */
