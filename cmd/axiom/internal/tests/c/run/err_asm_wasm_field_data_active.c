/* flags: --c-std=gnu17 */
/* wasm-only */
/* expect-error: C3097 */
/* Module fields of file-scope asm (docs/c.md "Inline assembly"): A data
  segment of file-scope asm is passive: an active one would write the
  program's memory at a fixed address. */
__asm__("(data $d (i32.const 64) \"xy\")");
int main(void) { return 0; }
