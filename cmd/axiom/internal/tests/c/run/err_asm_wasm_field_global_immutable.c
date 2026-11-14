/* flags: --c-std=gnu17 */
/* wasm-only */
/* expect-error: C3097 */
/* Module fields of file-scope asm (docs/c.md "Inline assembly"): global.set
  of an immutable global is an error. */
__asm__("(global $g i32 (i32.const 1))\n"
        "(func $f i32.const 2 global.set $g)");
int main(void) { return 0; }
