/* flags: --c-std=gnu17 --target=aarch64 */
/* native-only */
/* expect-error: C3097 */
/* Vector arrangements an AArch64 instruction does not have (the
   architecture's lists of each instruction's arrangements): errors, not
   encodings of something else. */
int main(void) {
    asm("smin v0.2d, v1.2d, v2.2d");
    asm("mul v0.2d, v1.2d, v2.2d");
    asm("rev64 v0.2d, v1.2d");
    asm("addv s0, v1.8b");
    asm("dup v0.8b, x1");
    asm("mov v0.2d, v1.2d");
    asm("ins w0, v1.s[1]");
    asm("cmp wsp, xzr");
    asm("prfm #4096, [x0]");
    return 0;
}
