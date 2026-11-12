/* flags: --c-std=gnu17 */
/* native-only */
/* expect-error: C3096 */
/* A naked function's body is its asm statements only, without operands. */
__attribute__((naked)) int f(int x) {
    x += 1;
    asm("nop");
}
__attribute__((naked)) int g(int x) {
    asm("nop" :: "r"(x));
}
int main(void) { return 0; }
