/* flags: --c-std=gnu17 --target=aarch64 */
/* native-only */
/* expect-error: C3097 */
/* A 32-bit offset register is written with its extend ([x0, w1, uxtw]). */
int main(void) {
    asm("ldr x0, [x1, w2]");
    return 0;
}
