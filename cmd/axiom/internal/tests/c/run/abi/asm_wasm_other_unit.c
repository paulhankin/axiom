/* (err_asm_wasm_unit_names.c's second unit: its asm statement names a
   function of the first unit's file-scope asm, which it does not see) */
int call_helper(void) {
    int r;
    __asm__("call $helper local.set %0" : "=r"(r));
    return r;
}
