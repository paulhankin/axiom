/* flags: --c-std=gnu17 */
/* wasm-only */
/* expect-error: C3097 */
/* Module fields of file-scope asm (docs/c.md "Inline assembly"): A global's
  initial value is a constant of its type (here of another type). */
__asm__("(global $g i64 (i32.const 1))");
int main(void) { return 0; }
