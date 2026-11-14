## Assembler

- `.hidden`, `.internal` and `.protected` reach the object: ELF's `STV_HIDDEN` (for
  `.internal` too) and `STV_PROTECTED`, Mach-O's private extern for the hidden ones.
  They were parsed and dropped, so a hidden definition in a shared library was exported.
- `.fill` without operands no longer crashes the compiler, and `.fill`, `.space` and
  the alignment directives with an empty or a superfluous operand are errors.
