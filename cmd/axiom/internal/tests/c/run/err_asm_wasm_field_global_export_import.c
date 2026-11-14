/* flags: --c-std=gnu17 */
/* wasm-only */
/* expect-error: C3097 */
/* Module fields of file-scope asm (docs/c.md "Inline assembly"): Only the
  program's own globals are exported. */
__asm__("(import \"env\" \"h\" (global $h i32))\n"
        "(export \"h\" (global $h))");
int main(void) { return 0; }
