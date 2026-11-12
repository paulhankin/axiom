/* flags: --c-std=gnu17 */
/* wasm-only */
/* expect-error: C3901 */
/* Inline assembly is for x86-64 and AArch64 targets only. */
int main(void) { __asm__ volatile ("nop"); return 0; }
