/* flags: --c-std=gnu17 */
/* wasm-only */
/* expect-error: C3096 */
/* WebAssembly has no registers: a machine constraint of another target is not one of its. */
int main(void) { int r; __asm__("" : "=a"(r)); return r; }
