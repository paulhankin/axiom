/* flags: --c-std=gnu17 */
/* wasm-only */
/* __builtin_setjmp is not available on WebAssembly (C3300). */
/* expect-error: C3300 */
static void *jb[5];
int main(void) { return __builtin_setjmp(jb); }
