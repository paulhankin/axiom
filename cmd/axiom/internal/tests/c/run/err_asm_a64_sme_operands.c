/* flags: --c-std=gnu17 --target=aarch64 --features=+sme2 */
/* native-only */
/* expect-error: C3097 */
/* SME2 operands an encoding does not have: a list of four registers must
   start at a multiple of four. */
void f(void) { __asm__ volatile("fmla za.s[w8, 0, vgx4], {z1.s-z4.s}, z0.s[0]" ::: "za"); }
int main(void) { f(); return 0; }
