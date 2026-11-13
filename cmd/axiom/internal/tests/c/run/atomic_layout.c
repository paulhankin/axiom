/* flags: --c-std=c11 */
/* The layout of atomic types (6.2.5p27), what every target has alike: an
   atomic type is at least as large as its non-atomic version, the same size
   for the scalar types of 1, 2, 4 and 8 bytes and for structures of those
   sizes, which are aligned to their size. What differs (padding up to a
   power of two, the alignment of 16 bytes) is in atomic_layout_<target>.c,
   checked against the platforms' compilers. */
#include "host.h"
struct s1 { char a; };
struct s2 { char a[2]; };
struct s3 { char a[3]; };
struct s5 { char a[5]; };
struct s8 { char a[8]; };
struct s12 { int a[3]; };
struct s24 { long long a[3]; };
struct s32 { char a[32]; };
#define L(T) _Static_assert(sizeof(_Atomic T) == sizeof(T), #T)
#define G(T) _Static_assert(sizeof(_Atomic T) >= sizeof(T) && _Alignof(_Atomic T) >= _Alignof(T), #T)
L(char); L(short); L(int); L(long); L(long long); L(float); L(double); L(void *); L(_Bool);
L(struct s1); L(struct s2); L(struct s8); L(struct s24); L(struct s32);
G(struct s3); G(struct s5); G(struct s12); G(long double); G(double _Complex); G(float _Complex);
#define A(T, n) _Static_assert(_Alignof(_Atomic T) == (n), #T)
A(char, 1); A(short, 2); A(int, 4); A(long long, 8); A(float, 4); A(double, 8); A(_Bool, 1);
A(struct s1, 1); A(struct s2, 2); A(struct s8, 8); A(struct s24, _Alignof(long long)); A(struct s32, 1);
A(float _Complex, 8);
_Static_assert(_Alignof(_Atomic(void *)) == sizeof(void *), "pointer");
/* members of atomic type are placed by their alignment */
struct m { char c; _Atomic struct s2 p; char d; _Atomic struct s8 q; char e; };
_Static_assert(__builtin_offsetof(struct m, p) == 2 && __builtin_offsetof(struct m, q) == 8, "m");
_Static_assert(sizeof(struct m) == 24 && _Alignof(struct m) == 8, "m");
/* qualifiers with _Atomic, and the specifier, are one type */
_Static_assert(_Generic((const _Atomic int *)0, _Atomic const int *: 1, default: 0), "q");
_Static_assert(_Generic((_Atomic(int) *)0, _Atomic int *: 1, default: 0), "s");
_Atomic struct s8 g8[3];
_Atomic struct s3 g3[2];
int main(void) {
    _Atomic struct s8 l8;
    print_i64((unsigned long long)&g8[1] % 8 + (unsigned long long)&l8 % 8);
    print_i64((char *)&g3[1] - (char *)&g3[0] == sizeof(_Atomic struct s3));
    print_i64(sizeof(struct m));
    return 0;
}
/* expect: 0 */
/* expect: 1 */
/* expect: 24 */
