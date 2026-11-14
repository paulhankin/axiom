## Assembler

- Relocation modifiers in data were dropped silently, giving the symbol's plain address:
  in 4-byte data `sym@GOTPCREL` (ELF, Mach-O, x64 COFF), `sym@PLT - .` (x86-64 Linux)
  and `sym@IMGREL` (COFF) now give their relocations, and other modifiers in data are
  errors. New: COFF's `.rva` and `.secrel32`, and `.org` in assembly sources and
  file-scope asm.
- `.section` reads each object format's syntax. ELF's flags `G` (COMDAT groups), `o`,
  `R`, the `w` of `"awx"` and `,unique,N` were dropped; Mach-O's
  `segment,section,type,attributes` was taken for a section named after the segment;
  COFF's `discard` selection was dropped and a read-only section with a name of its own
  was put in `.rdata`. Now they are honoured or, as unknown flags and types, writable
  code, `,unique,N` and an initialization array not named for one, errors. ELF names
  such as `.rodata` stay accepted on macOS and Windows.
- `.balignw`, `.p2alignw`, `.balignl` and `.p2alignl` fill with 2- and 4-byte units (they
  filled with the low byte), and padding that is not whole units is an error; `.align n`
  is `2^n` bytes in Mach-O on x86-64 too and `n` bytes in COFF on ARM64, as the
  platforms' assemblers have it.
