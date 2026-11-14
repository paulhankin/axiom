/* flags: --c-std=gnu17 --target=aarch64 */
/* native-only */
/* expect-error: C3097 */
/* A system register only read (an ID register) cannot be written. */
int main(void) {
    asm volatile("msr midr_el1, x0");
    return 0;
}
