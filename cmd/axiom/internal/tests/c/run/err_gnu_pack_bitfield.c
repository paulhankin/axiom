/* flags: --c-std=gnu11 */
/* expect-error: C3901 */
/* A packed bit-field that no unit within its structure holds (System V and
   AAPCS64, which place it at the next bit; Windows x64 keeps it in a unit
   of its type, which the structure has: gnu_pack_bitfield_windows.c). */
/* not-windows */
struct __attribute__((packed)) S { int b : 20; char c : 4; };
int main(void) { struct S s; s.b = 1; return s.b; }
