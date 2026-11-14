/* flags: --c-std=gnu17 */
/* wasm-only */
/* expect-error: C3097 */
/* Module fields of file-scope asm (docs/c.md "Inline assembly"): An element
  segment of file-scope asm is passive or declarative: the function table's
  layout is the program's. */
__asm__("(func $g)\n"
        "(elem (i32.const 1) func $g)");
int main(void) { return 0; }
