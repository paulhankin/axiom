/* flags: --c-std=gnu17 --features=+exception-handling */
/* wasm-only */
/* expect-error: C3097 */
/* Module fields of file-scope asm (docs/c.md "Inline assembly"): A tag the
  text names must be declared (defined or imported) where the asm is. */
void f(void) { __asm__("throw $nothing"); }
int main(void) { return 0; }
