/* flags: --c-std=gnu17 */
/* wasm-only */
/* expect-error: C3097 */
/* return (and return_call) in an asm statement would skip the function's epilogue. */
int main(void) { __asm__("return"); return 0; }
