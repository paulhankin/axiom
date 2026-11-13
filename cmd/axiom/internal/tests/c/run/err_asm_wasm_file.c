/* flags: --c-std=gnu17 */
/* wasm-only */
/* expect-error: C3097 */
/* File-scope asm on WebAssembly is module fields, (func ...) and (import ...), each validated: a call of an unknown function, a result missing. */
__asm__("(func $bad (result i32) call $nowhere)");
__asm__("(memory 1)");
int main(void) { return 0; }
