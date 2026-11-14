/* flags: --target=x86_64 --c-std=gnu17 */
/* check-only */
/* The attribute ms_struct and #pragma ms_struct (a GNU dialect, x86-64):
   the Microsoft layout of bit-fields (Windows x64's) on a System V target.
   A bit-field joins the open unit only if its type has the unit's size and
   it fits, another member closes the unit, one of width 0 closes it and
   aligns the next member to its type (ignored where no unit is open); in a
   union, bit-fields add their type's size but not its alignment.
   gcc_struct and #pragma ms_struct off keep the System V layout. */
#include <stddef.h>

#define MS __attribute__((ms_struct))

struct MS A { char a : 4; short b : 4; char c; };      /* units of 1 and 2, then c */
struct     A2 { char a : 4; short b : 4; char c; };    /* one unit of 2 */
struct MS B { unsigned a : 12; unsigned : 0; unsigned b : 12; };
struct MS C { char a : 4; short : 0; char b; };
struct MS D { char a : 6; int : 0; };                  /* width 0 closes it, aligned to int */
struct MS E { char a; int : 0; char b; };              /* no unit open: ignored */
struct MS F { double d; unsigned char a : 8; int b : 32; unsigned char c : 5;
              short e : 14; short f : 13; unsigned char : 0; };
struct MS G { unsigned a : 26; unsigned char b : 2; };
struct __attribute__((gcc_struct)) H { unsigned a : 26; unsigned char b : 2; };
union MS U { char a : 4; int b : 20; };
union MS V { unsigned char a : 4; int : 0; };

#pragma pack(push, 2)
struct MS P { char a; long long b : 50; char d; };
#pragma pack(pop)

#pragma ms_struct on
struct Q { char a : 4; short : 0; char b; };
#pragma ms_struct off
struct R { char a : 4; short : 0; char b; };
#pragma ms_struct on
struct __attribute__((gcc_struct)) S { char a : 4; short b : 4; };
#pragma ms_struct reset
struct T { char a : 4; short b : 4; };

_Static_assert(sizeof(struct A) == 6 && _Alignof(struct A) == 2 && offsetof(struct A, c) == 4, "A");
_Static_assert(sizeof(struct A2) == 2 && offsetof(struct A2, c) == 1, "A2");
_Static_assert(sizeof(struct B) == 8, "B");
_Static_assert(sizeof(struct C) == 4 && offsetof(struct C, b) == 2, "C");
_Static_assert(sizeof(struct D) == 4 && _Alignof(struct D) == 4, "D");
_Static_assert(sizeof(struct E) == 2 && offsetof(struct E, b) == 1, "E");
_Static_assert(sizeof(struct F) == 24, "F");
_Static_assert(sizeof(struct G) == 8 && _Alignof(struct G) == 4, "G");
_Static_assert(sizeof(struct H) == 4, "H");
_Static_assert(sizeof(union U) == 4 && _Alignof(union U) == 1, "U");
_Static_assert(sizeof(union V) == 4 && _Alignof(union V) == 1, "V");
_Static_assert(sizeof(struct P) == 12 && _Alignof(struct P) == 2 && offsetof(struct P, d) == 10, "P");
_Static_assert(sizeof(struct Q) == 4 && offsetof(struct Q, b) == 2, "Q");
_Static_assert(sizeof(struct R) == 3 && offsetof(struct R, b) == 2, "R");
_Static_assert(sizeof(struct S) == 2, "S");
_Static_assert(sizeof(struct T) == 2, "T");
#if !__has_attribute(ms_struct) || !__has_attribute(gcc_struct)
#error "__has_attribute"
#endif

int main(void) { return 0; }
