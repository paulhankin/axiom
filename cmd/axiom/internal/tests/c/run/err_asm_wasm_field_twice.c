/* flags: --c-std=gnu17 */
/* wasm-only */
/* expect-error: C3097 */
/* Module fields of file-scope asm (docs/c.md "Inline assembly"): A name is
  declared once in each namespace. */
__asm__("(global $g i32 (i32.const 1))\n"
        "(global $g i32 (i32.const 2))");
int main(void) { return 0; }
