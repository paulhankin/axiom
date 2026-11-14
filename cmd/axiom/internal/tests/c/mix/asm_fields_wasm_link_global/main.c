/* A global of an assembly source declared by another unit with another
   type or mutability than its definition's is a link error (docs/c.md
   "Inline assembly"). */
/* wasm-only */
/* flags: --c-std=gnu17 */
/* expect-error: E9004 */
/* expect-stderr: global 'count' is declared with another type or mutability than its definition's */
__asm__("(import \"env\" \"count\" (global $count i32))\n"
        "(func $read_count (result i32) global.get $count)");
int read_count(void);
int main(void) { return read_count(); }
