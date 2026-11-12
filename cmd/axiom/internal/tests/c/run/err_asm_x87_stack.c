/* flags: --c-std=gnu17 --target=x86_64 */
/* native-only */
/* expect-error: C3096 */
/* The x87 stack's rules for asm operands: an operand in st(1) ('u') needs
   one in st(0) ('t'), and the inputs a template pops (its clobbers) are the
   topmost. */
int main(void) {
    double r, a = 1.0;
    __asm__("fld1" : "=u"(r) : "f"(a));
    __asm__("fstp %%st(1)" : : "t"(a), "u"(a) : "st(1)");
    return 0;
}
