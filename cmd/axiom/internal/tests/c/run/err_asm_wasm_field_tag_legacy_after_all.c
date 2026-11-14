/* flags: --c-std=gnu17 --features=+exception-handling */
/* wasm-only */
/* expect-error: C3097 */
/* Module fields of file-scope asm (docs/c.md "Inline assembly"): The legacy
  form's catch of a tag cannot follow its catch_all. */
__asm__("(tag $e)\n"
        "(func $f try nop catch_all catch $e end)");
int main(void) { return 0; }
