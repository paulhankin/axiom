/* flags: --c-std=gnu17 --target=aarch64 */
/* native-only */
/* expect-error: C3097 */
/* An encoding the architecture makes CONSTRAINED UNPREDICTABLE (here a pair
   loaded into one register) is an error, not an instruction. */
int main(void) {
    asm("ldp x0, x0, [x1]");
    return 0;
}
