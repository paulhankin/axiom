/* flags: --c-std=gnu17 */
/* native-only */
/* expect-error: C3097 */
/* Call frame information a template cannot have: adjustments that do not
   add up to 0, a register saved and not restored, a directive that would
   change the frame; and in file-scope asm a procedure not closed. */
void f(void) {
    asm(".cfi_adjust_cfa_offset 8");
    asm(".cfi_rel_offset 3, 0");
    asm(".cfi_def_cfa_offset 16");
    asm(".cfi_startproc");
}
__asm__(".text\nfoo:\n.cfi_startproc\n");
int main(void) { return 0; }
