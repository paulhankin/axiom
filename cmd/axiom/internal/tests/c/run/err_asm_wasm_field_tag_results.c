/* flags: --c-std=gnu17 --features=+exception-handling */
/* wasm-only */
/* expect-error: C3097 */
/* Module fields of file-scope asm (docs/c.md "Inline assembly"): A tag's type
  has no results. */
__asm__("(tag $e (param i32) (result i32))");
int main(void) { return 0; }
