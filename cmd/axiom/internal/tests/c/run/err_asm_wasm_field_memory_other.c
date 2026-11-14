/* flags: --c-std=gnu17 --target=wasm32 */
/* wasm-only */
/* expect-error: C3097 */
/* Module fields of file-scope asm (docs/c.md "Inline assembly"): A (memory
  ...) field only declares the program's memory again (here of the other index
  type). */
__asm__("(memory i64 1)");
int main(void) { return 0; }
