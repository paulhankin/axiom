/* flags: --c-std=gnu17 --target=aarch64 */
/* native-only */
/* expect-error: C3097 */
/* A load with writeback to the register it loads is CONSTRAINED
   UNPREDICTABLE: an error. */
int main(void) {
    asm("ldr x1, [x1], #8");
    return 0;
}
