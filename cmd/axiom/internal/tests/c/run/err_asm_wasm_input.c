/* flags: --c-std=gnu17 */
/* wasm-only */
/* expect-error: C3097 */
/* A template cannot write an input operand's local (only outputs, = or +). */
int f(int x) { __asm__("i32.const 1 local.set %0" : : "r"(x)); return x; }
int main(void) { return f(1); }
