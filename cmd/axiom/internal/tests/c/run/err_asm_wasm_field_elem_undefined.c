/* flags: --c-std=gnu17 */
/* wasm-only */
/* expect-error: C3097 */
/* Module fields of file-scope asm (docs/c.md "Inline assembly"): An element
  segment's functions must be declared. */
__asm__("(elem $e func $nothing)");
int main(void) { return 0; }
