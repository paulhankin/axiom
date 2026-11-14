/* flags: --c-std=gnu17 --features=+exception-handling */
/* wasm-only */
/* expect-error: C3097 */
/* Module fields of file-scope asm (docs/c.md "Inline assembly"): try_table's
  catch gives its label the tag's parameters: the label's block must take
  them. */
__asm__("(tag $e (param i32))\n"
        "(func $f (block $h (try_table (catch $e $h) nop)))");
int main(void) { return 0; }
