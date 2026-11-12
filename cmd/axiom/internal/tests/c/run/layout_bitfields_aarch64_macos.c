/* flags: --target=aarch64-macos */
/* check-only */
/* The layout of bit-fields on --target=aarch64-macos (Apple's arm64 ABI): sizes, alignments and
   the offsets of the members after them (docs/c.md). */
#include <stddef.h>

struct A { _Bool a : 1; unsigned : 3; _Bool b : 1; _Bool c; };
struct B { unsigned : 3; _Bool b : 1; char m; };
struct C { char a; int : 0; char b; };
struct D { char a; unsigned : 9; char b; };
struct E { char a; long long : 0; char b; };
struct F { char a : 3; int b : 5; char c; };
struct G { int a : 3; unsigned b : 30; char c; };
struct H { char a; int b : 3; char c; };
struct I { int : 0; char a; };
struct J { char x; char a : 2; int : 0; char b; };
struct K { long long a : 3; int b : 2; short c : 4; char d; };
struct L { char a; _Bool b : 1; int c : 31; char d; };
struct M { short a : 3; unsigned short b : 13; char c : 2; char d; };
struct N { char a; int b : 3; char c; int d : 4; char e; };
struct O { int a : 3; char : 0; int b : 3; char c; };
struct P { int a : 3; long long : 0; char d; };
struct Q { unsigned a : 31; unsigned b : 2; char c; };
struct R { char a; long long b : 60; char c; };
union U1 { int a : 3; char b; };
union U2 { char b; int : 3; };
union U3 { unsigned short a : 6; unsigned : 0; signed char b : 3; };
union U4 { long long a : 3; char b; };

_Static_assert(sizeof(struct A) == 2 && _Alignof(struct A) == 1 && offsetof(struct A, c) == 1, "A");
_Static_assert(sizeof(struct B) == 2 && _Alignof(struct B) == 1 && offsetof(struct B, m) == 1, "B");
_Static_assert(sizeof(struct C) == 5 && _Alignof(struct C) == 1 && offsetof(struct C, b) == 4, "C");
_Static_assert(sizeof(struct D) == 4 && _Alignof(struct D) == 1 && offsetof(struct D, b) == 3, "D");
_Static_assert(sizeof(struct E) == 9 && _Alignof(struct E) == 1 && offsetof(struct E, b) == 8, "E");
_Static_assert(sizeof(struct F) == 4 && _Alignof(struct F) == 4 && offsetof(struct F, c) == 1, "F");
_Static_assert(sizeof(struct G) == 12 && _Alignof(struct G) == 4 && offsetof(struct G, c) == 8, "G");
_Static_assert(sizeof(struct H) == 4 && _Alignof(struct H) == 4 && offsetof(struct H, c) == 2, "H");
_Static_assert(sizeof(struct I) == 1 && _Alignof(struct I) == 1 && offsetof(struct I, a) == 0, "I");
_Static_assert(sizeof(struct J) == 5 && _Alignof(struct J) == 1 && offsetof(struct J, b) == 4, "J");
_Static_assert(sizeof(struct K) == 8 && _Alignof(struct K) == 8 && offsetof(struct K, d) == 2, "K");
_Static_assert(sizeof(struct L) == 12 && _Alignof(struct L) == 4 && offsetof(struct L, d) == 8, "L");
_Static_assert(sizeof(struct M) == 4 && _Alignof(struct M) == 2 && offsetof(struct M, d) == 3, "M");
_Static_assert(sizeof(struct N) == 8 && _Alignof(struct N) == 4 && offsetof(struct N, e) == 4, "N");
_Static_assert(sizeof(struct O) == 4 && _Alignof(struct O) == 4 && offsetof(struct O, c) == 2, "O");
_Static_assert(sizeof(struct P) == 12 && _Alignof(struct P) == 4 && offsetof(struct P, d) == 8, "P");
_Static_assert(sizeof(struct Q) == 8 && _Alignof(struct Q) == 4 && offsetof(struct Q, c) == 5, "Q");
_Static_assert(sizeof(struct R) == 24 && _Alignof(struct R) == 8 && offsetof(struct R, c) == 16, "R");
_Static_assert(sizeof(union U1) == 4 && _Alignof(union U1) == 4, "U1");
_Static_assert(sizeof(union U2) == 1 && _Alignof(union U2) == 1, "U2");
_Static_assert(sizeof(union U3) == 2 && _Alignof(union U3) == 2, "U3");
_Static_assert(sizeof(union U4) == 8 && _Alignof(union U4) == 8, "U4");

int main(void) { return 0; }
