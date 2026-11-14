/* flags: --c-std=gnu17 */
/* native-only */
/* expect-error: C3097 */
/* .eh_frame pointer encodings this assembler does not write: data- and
   text-relative ones (0x30, 0x20), 2-byte ones (0x1a), and the directives
   left out (.cfi_val_encoded_addr, .cfi_inline_lsda). */
__asm__(".text\nf1:\n.cfi_startproc\n.cfi_personality 0x30, g\n  ret\n.cfi_endproc\n");
__asm__(".text\nf2:\n.cfi_startproc\n.cfi_lsda 0x1a, g\n  ret\n.cfi_endproc\n");
__asm__(".text\nf3:\n.cfi_startproc\n.cfi_val_encoded_addr 3, 0x1b, g\n  ret\n.cfi_endproc\n");
int main(void) { return 0; }
