/* A tag that a unit imports from "env" and no unit defines is a link
   error (docs/c.md "Inline assembly"). */
/* wasm-only */
/* flags: --c-std=gnu17 --features=+exception-handling */
/* expect-error: E9004 */
/* expect-stderr: undefined tag 'nowhere' */
__asm__("(import \"env\" \"nowhere\" (tag $e (param i32)))\n"
        "(func $raise_e (param i32) local.get 0 throw $e)");
void raise_e(int);
int main(void) {
    raise_e(1);
    return 0;
}
