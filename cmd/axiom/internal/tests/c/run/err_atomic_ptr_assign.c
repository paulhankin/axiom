/* flags: --c-std=c11 */
/* expect-error: C3022 */
/* 6.2.5p27: an atomic type is not a qualified version of its non-atomic type, so
   pointers to them are incompatible. */
_Atomic int ai;
int *p = &ai;
int main(void) { return 0; }
