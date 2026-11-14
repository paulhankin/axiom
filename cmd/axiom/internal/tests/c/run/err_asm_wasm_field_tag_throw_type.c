/* flags: --c-std=gnu17 --features=+exception-handling */
/* wasm-only */
/* expect-error: C3097 */
/* Module fields of file-scope asm (docs/c.md "Inline assembly"): throw pops
  the tag's parameters (here an i64 for an i32). */
__asm__("(tag $e (param i32))\n"
        "(func $f i64.const 1 throw $e)");
int main(void) { return 0; }
