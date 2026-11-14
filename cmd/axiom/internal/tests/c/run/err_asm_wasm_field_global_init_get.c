/* flags: --c-std=gnu17 */
/* wasm-only */
/* expect-error: C3097 */
/* Module fields of file-scope asm (docs/c.md "Inline assembly"): A global's
  initial value of global.get names an immutable imported global (here a
  mutable one). */
__asm__("(import \"env\" \"h\" (global $h (mut i32)))\n"
        "(global $g i32 (global.get $h))");
int main(void) { return 0; }
