/* flags: --c-std=gnu17 */
/* wasm-only */
/* expect-error: C3097 */
/* Module fields of file-scope asm (docs/c.md "Inline assembly"): A (table
  ...) field only declares the program's function table again (here of another
  element type). */
__asm__("(table 1 externref)");
int main(void) { return 0; }
