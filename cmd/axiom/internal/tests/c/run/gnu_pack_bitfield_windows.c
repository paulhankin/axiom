/* flags: --c-std=gnu11 --target=x86_64-windows */
/* check-only */
/* Windows x64 keeps a packed structure's bit-fields in units of their type
   (unaligned), so the bit-field that err_gnu_pack_bitfield.c rejects on the
   other targets has a unit here: b in 4 bytes at 0, c in a byte at 4. */
#include <stddef.h>
struct __attribute__((packed)) S { int b : 20; char c : 4; char d; };
_Static_assert(sizeof(struct S) == 6 && _Alignof(struct S) == 1 && offsetof(struct S, d) == 5, "S");
int main(void) { struct S s; s.b = 1; return s.b; }
