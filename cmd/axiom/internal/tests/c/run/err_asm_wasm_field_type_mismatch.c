/* flags: --c-std=gnu17 */
/* wasm-only */
/* expect-error: C3097 */
/* Module fields of file-scope asm (docs/c.md "Inline assembly"): A (type $t)
  use with (param ...) (result ...) must repeat the type's. */
__asm__("(type $t (func (param i32)))\n"
        "(func $f (type $t) (param i64))");
int main(void) { return 0; }
