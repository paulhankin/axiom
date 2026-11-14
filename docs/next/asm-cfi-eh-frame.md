## Assembler

- On Linux the `.cfi_*` procedures of file-scope asm and assembly sources are in
  `.eh_frame` (and under `-g` in `.debug_frame`), as `.cfi_sections` chooses.
  `.cfi_personality` and `.cfi_lsda` take the routine's and the LSDA's symbols in the
  absolute, pc-relative and indirect 4- and 8-byte encodings, so assembly can take part
  in exception handling; new are `.cfi_return_column`, `.cfi_val_offset`, `.cfi_label`,
  and on AArch64 `.cfi_b_key_frame`, `.cfi_mte_tagged_frame` and
  `.cfi_negate_ra_state_with_pc`. On macOS and Windows a personality routine, an LSDA
  and the CIE's other augmentations are errors.
