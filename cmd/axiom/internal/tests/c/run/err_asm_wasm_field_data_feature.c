/* flags: --c-std=gnu17 --features=-bulk-memory --target=wasm32 */
/* wasm-only */
/* expect-error: C3097 */
/* Module fields of file-scope asm (docs/c.md "Inline assembly"): memory.init
  of a passive segment needs the bulk-memory extension. */
__asm__("(data $d \"xy\")\n"
        "(func $f i32.const 0 i32.const 0 i32.const 2 memory.init $d)");
int main(void) { return 0; }
