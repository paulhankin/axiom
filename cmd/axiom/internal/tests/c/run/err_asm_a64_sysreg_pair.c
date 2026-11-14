/* flags: --c-std=gnu17 --target=aarch64 */
/* native-only */
/* expect-error: C3097 */
/* mrrs and msrr name only the 128-bit system registers. */
int main(void) {
    asm volatile("mrrs x0, x1, tpidr_el0");
    return 0;
}
