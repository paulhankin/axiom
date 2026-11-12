/* flags: --target=aarch64 --c-std=gnu17 */
/* check-only */
/* Bit-fields of packed structures and under #pragma pack on --target=aarch64
   (AAPCS64): sizes, alignments and the offsets of the members after them. */
#include <stddef.h>

struct __attribute__((packed)) A { char a; int b : 30; int c : 30; char d; };
struct __attribute__((packed)) B { signed char : 4; unsigned char f : 7; char d; };
struct __attribute__((packed)) C { char a; long long : 0; char d; };
#pragma pack(push, 1)
struct D { char a : 3; int b : 20; char d; };
#pragma pack(pop)
#pragma pack(push, 2)
struct E { char a; long long b : 50; char d; };
#pragma pack(pop)
#pragma pack(push, 4)
struct F { short a : 10; int b : 25; char d; };
#pragma pack(pop)
#pragma pack(push, 2)
struct G { signed char a : 1; signed char b; unsigned long long : 0; char d; };
#pragma pack(pop)
#pragma pack(push, 1)
struct H { int : 0; char a; int b : 5; char d; };
#pragma pack(pop)
#pragma pack(push, 2)
struct I { int a; char b : 3; long long c : 20; char d; };
#pragma pack(pop)
#pragma pack(push, 4)
union J { long long a : 3; char b; };
#pragma pack(pop)

_Static_assert(sizeof(struct A) == 10 && _Alignof(struct A) == 1 && offsetof(struct A, d) == 9, "A");
_Static_assert(sizeof(struct B) == 3 && _Alignof(struct B) == 1 && offsetof(struct B, d) == 2, "B");
_Static_assert(sizeof(struct C) == 16 && _Alignof(struct C) == 8 && offsetof(struct C, d) == 8, "C");
_Static_assert(sizeof(struct D) == 4 && _Alignof(struct D) == 1 && offsetof(struct D, d) == 3, "D");
_Static_assert(sizeof(struct E) == 10 && _Alignof(struct E) == 2 && offsetof(struct E, d) == 8, "E");
_Static_assert(sizeof(struct F) == 8 && _Alignof(struct F) == 4 && offsetof(struct F, d) == 5, "F");
_Static_assert(sizeof(struct G) == 16 && _Alignof(struct G) == 8 && offsetof(struct G, d) == 8, "G");
_Static_assert(sizeof(struct H) == 4 && _Alignof(struct H) == 4 && offsetof(struct H, d) == 2, "H");
_Static_assert(sizeof(struct I) == 8 && _Alignof(struct I) == 2 && offsetof(struct I, d) == 7, "I");
_Static_assert(sizeof(union J) == 4 && _Alignof(union J) == 4, "J");

int main(void) { return 0; }
