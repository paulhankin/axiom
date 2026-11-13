/* flags: --c-std=gnu17 --target=aarch64 */
/* native-only */
/* expect-error: C3096 */
/* The constraint 'k' is the stack pointer: an input whose operand is the
   register variable declared asm("sp") (whose value the stack pointer is);
   another operand, an output, or that variable under another constraint are
   errors. */
int main(void) {
    register void *spv asm("sp");
    long x = 0, y;
    asm("mov %0, %1" : "=r"(y) : "k"(x));
    asm("mov %0, sp" : "=k"(spv));
    asm("mov %0, %1" : "=r"(y) : "r"(spv));
    return (int)y;
}
