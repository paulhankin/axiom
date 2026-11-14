/* flags: --target=x86_64-windows --c-std=gnu17 */
/* check-only */
/* On Windows x64, whose bit-fields have the Microsoft layout, the attribute
   gcc_struct (a GNU dialect) gives a structure the System V one, and
   ms_struct the target's own. */
struct __attribute__((gcc_struct)) A { char a : 4; short b : 4; char c; };
struct __attribute__((ms_struct)) B { char a : 4; short b : 4; char c; };
struct C { char a : 4; short b : 4; char c; };
union __attribute__((gcc_struct)) U { char a : 4; int b : 20; };

_Static_assert(sizeof(struct A) == 2, "A");
_Static_assert(sizeof(struct B) == 6 && sizeof(struct C) == 6, "B, C");
_Static_assert(sizeof(union U) == 4 && _Alignof(union U) == 4, "U");

int main(void) { return 0; }
