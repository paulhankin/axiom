/* flags: --c-std=gnu17 */
/* wasm-only */
/* expect-error: C3097 */
/* Module fields of file-scope asm (docs/c.md "Inline assembly"): A start
  function takes and returns nothing. */
__asm__("(func $s (param i32))\n"
        "(start $s)");
int main(void) { return 0; }
