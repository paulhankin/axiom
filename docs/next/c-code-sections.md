## Objects and linking

- A function's attribute `section` places its code in that section (ELF, Mach-O,
  COFF), with its frame table, debug information and Windows unwind information
  following it; cold functions (and the cold paths the optimizer moves out of
  functions) go in `.text.unlikely`, hot ones in `.text.hot`.
- New options `--function-sections` and `--data-sections` put each function and datum
  in a section of its own, and `--gc-sections` makes the compiler's linkers leave out
  the sections of the linked objects and archives that nothing refers to.
- Objects for other links (`--emit=obj`, `lib`, `shared`) put each link-once
  definition in a COMDAT group (ELF) or COMDAT section (COFF), so that the system's
  linkers keep one copy too; the compiler's linkers honour COFF's COMDAT selections
  ("no duplicates", "same size", "exact match", "largest") instead of taking every one
  as "any".
