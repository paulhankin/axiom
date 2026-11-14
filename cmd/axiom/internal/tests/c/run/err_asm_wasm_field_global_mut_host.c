/* flags: --c-std=gnu17 --features=-mutable-globals */
/* wasm-only */
/* expect-error: C3097 */
/* Module fields of file-scope asm (docs/c.md "Inline assembly"): A mutable
  global imported from the host needs the mutable-globals extension. */
__asm__("(import \"host\" \"h\" (global $h (mut i32)))");
int main(void) { return 0; }
