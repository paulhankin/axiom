/* flags: --c-std=gnu17 --target=aarch64 */
/* native-only */
/* expect-error: C3097 */
/* An SME2 instruction for a target without the extension: the assembler
   names what --features needs (here +sme2). */
void f(void) { __asm__ volatile("fmla za.s[w8, 0, vgx2], {z0.s-z1.s}, z2.s" ::: "za"); }
int main(void) { f(); return 0; }
