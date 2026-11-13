/* flags: --c-std=gnu17 */
/* wasm-only */
/* expect-error: C3097 */
/* A WebAssembly template is validated: an operand of the wrong type on the stack. */
int f(long long x) { int r; __asm__("local.get %1 local.set %0" : "=r"(r) : "r"(x)); return r; }
int main(void) { return f(1); }
