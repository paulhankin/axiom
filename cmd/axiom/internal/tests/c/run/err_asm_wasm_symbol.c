/* flags: --c-std=gnu17 */
/* wasm-only */
/* expect-error: C3097 */
/* call needs a function: an object symbol, or an operand in a local, is not one. */
int g;
int f(int (*p)(void)) { int r; __asm__("call %1 local.set %0" : "=r"(r) : "r"(p)); __asm__("call $g"); return r; }
int main(void) { return 0; }
