/* flags: --c-std=gnu17 */
/* wasm-only */
/* expect-error: C3097 */
/* A template must leave the stack as it found it (empty), and end its blocks. */
int main(void) { __asm__("i32.const 1"); __asm__("block nop"); return 0; }
