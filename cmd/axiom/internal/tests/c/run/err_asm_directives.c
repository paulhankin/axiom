/* flags: --c-std=gnu17 */
/* native-only */
/* expect-error: C3097 */
/* Directives with operands missing, too many or of the wrong kind (each
   an error, none a crash): .fill without a count, with an empty size or
   four operands; .space with three; .org backward, or in a function's
   template; a word fill that does not divide the padding; section flags
   and types the object format has not, a writable section of code, an
   initialization array not named for one, a unique section; relocation modifiers in data
   that the formats do not take there. */
__asm__(".data\n.fill\n");
__asm__(".data\n.fill 1,,2\n");
__asm__(".data\n.fill 1,2,3,4\n");
__asm__(".data\n.space 1,2,3\n");
__asm__(".data\n.byte 1,2\n.org 1\n");
__asm__(".data\n.byte 1\n.balignw 4, 0x0101\n");
__asm__(".section .df1,\"q\"\n");
__asm__(".section .df2,\"a\",@bogus\n");
__asm__(".section .df3,\"awx\",@progbits\n");
__asm__(".section .df4,\"a\",@init_array\n");
__asm__(".section .df5,\"aG\",@progbits\n");
__asm__(".section .df6,\"a\",@progbits,unique,1\n");
__asm__(".section __DATA,__df7,mod_init_funcs\n");
__asm__(".data\n.long main@PLT\n");
__asm__(".data\n.quad main@GOTPCREL\n");
__asm__(".data\n.org\n");
void f(void) {
    asm(".org 4");
}
int main(void) { return 0; }
