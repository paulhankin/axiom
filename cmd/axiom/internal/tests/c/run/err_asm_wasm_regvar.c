/* flags: --c-std=gnu17 */
/* wasm-only */
/* expect-error: C3096 */
/* WebAssembly has no registers: an explicit register variable names none of them. */
int main(void) { register int r asm("eax") = 1; return r; }
