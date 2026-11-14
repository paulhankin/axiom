/* flags: --c-std=gnu17 */
/* native-only */
/* expect-error: C3097 */
/* Directives with operands missing or too many are errors, never a
   crash: .fill without a count, with an empty size or four operands;
   .space with three; .balign with four. */
__asm__(".data\n.fill\n");
__asm__(".data\n.fill 1,,2\n");
__asm__(".data\n.fill 1,2,3,4\n");
__asm__(".data\n.space 1,2,3\n");
__asm__(".data\n.balign 4,0,1,2\n");
int main(void) { return 0; }
