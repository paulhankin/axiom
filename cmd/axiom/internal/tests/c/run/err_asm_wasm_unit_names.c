/* flags: --c-std=gnu17 */
/* wasm-only */
/* units: abi/asm_wasm_other_unit.c */
/* expect-error: C3097 */
/* The names file-scope asm declares are its unit's: another unit's asm
   statement does not see them (it declares the function in C, or imports
   it in its own file-scope asm). */
__asm__("(func $helper (result i32) i32.const 7)");
int call_helper(void);
int main(void) { return call_helper(); }
