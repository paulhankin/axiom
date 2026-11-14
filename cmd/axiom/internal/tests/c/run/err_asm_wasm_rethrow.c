/* flags: --c-std=gnu17 --features=+exception-handling */
/* wasm-only */
/* expect-error: C3097 */
/* rethrow names a try's catch_all (here a block: an error, so that the
   module stays valid). */
int main(void) {
    __asm__("block $b rethrow $b end");
    return 0;
}
