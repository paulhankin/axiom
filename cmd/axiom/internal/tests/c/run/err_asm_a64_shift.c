/* flags: --c-std=gnu17 --target=aarch64 */
/* native-only */
/* expect-error: C3097 */
/* A shift amount beyond the operation's width (0-31 for a 32-bit one) is an
   error, not an encoding of another amount. */
int main(void) {
    asm("eor w0, w1, w2, asr #44");
    return 0;
}
