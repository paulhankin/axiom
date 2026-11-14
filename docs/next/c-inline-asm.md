## Assembler

- Inline assembly in the GNU dialects (`--c-std=gnu17` ...) on x86-64, AArch64 and
  WebAssembly: `asm` statements with operands, constraints, clobbers and modifiers, `asm
  goto`, explicit register variables (`register long x asm("x8")`) as operands,
  file-scope `asm("...")` and the attribute `naked`. The compiler assembles the
  templates itself, with no other tool (docs/c.md, "Inline assembly"). New diagnostics
  C3096 (a rule of asm statements broken: an asm constraint that is not one of the
  target's, an operand wider than 64 bits in a register, an explicit register variable
  on WebAssembly) and C3097 (what the assembler rejects).
- Assembly sources are translation units: `axiom build main.c x.s y.S` (a `.S` is
  preprocessed with `__ASSEMBLER__`), also beside an Axiom program and alone with
  `--emit=obj`, with line information under `-g` (`.file` and `.loc` too, on every object
  format, in the PDB's line tables on Windows).
- Inline assembly's semantics follow the dialect's rules: the cheapest alternative of
  constraints like `"r,m"` (with `?` and `!`), outputs in registers given to the register
  allocator, an asm without `volatile` removed when unused, merged and moved like an
  operation, a `volatile` one without `"memory"` no barrier to memory, and an asm with
  memory operands and no `"memory"` clobber reading and writing exactly the objects of
  its operands; sibling calls from functions whose asm does not use the stack.
- Inline assembly's labels are symbols of the object (`.globl` ones global), its
  `.pushsection` ... `.popsection` put bytes in other sections, and templates and
  file-scope asm can name static functions and objects. A global symbol that a C unit's
  file-scope asm defines keeps the C library's definition of that name out, as an
  assembly source's does; an asm statement sees only its own unit's file-scope asm names.
- The assembler's own language: `.macro` with arguments, defaults, `:req`, `:vararg` and
  `\@`; `.rept`, `.irp` and `.irpc`; `.if`/`.ifdef`/`.ifc`/`.else`/`.elseif`/`.endif`;
  `.equiv`, `.set` and `name = value`; the conventional assembler's full expression
  precedence, symbol differences (also of labels defined further on in the section);
  `.comm`, `.lcomm`, `.incbin` (bounded to the include directories), `.octa`,
  `.uleb128`, `.sleb128` and the `0f`/`0d` floating forms; `.org`; `.section` in each
  object format's syntax (ELF's flags `G`, `o`, `R` and `,unique,N`, Mach-O's
  `segment,section,type,attributes`, COFF's selections) and `.subsection`; `.hidden`,
  `.internal` and `.protected` (ELF's `STV_HIDDEN` and `STV_PROTECTED`, Mach-O's private
  extern); COFF's `.rva` and `.secrel32`; and in 4-byte data `sym@GOTPCREL`, `sym@PLT -
  .` and `sym@IMGREL`. `.balign`/`.p2align` in code pad with the multi-byte NOP
  sequences; `.balignw`, `.p2alignw`, `.balignl` and `.p2alignl` fill with 2- and 4-byte
  units; `.align n` is `2^n` bytes in Mach-O on x86-64 and `n` bytes in COFF on ARM64,
  as the platforms' assemblers have it. Unknown section flags and types, writable code,
  other relocation modifiers in data, and padding that is not whole units are errors
  (some were dropped silently before, giving wrong output).
- Call frame information of assembly: the `.cfi_*` procedures of file-scope asm and
  assembly sources are FDEs in `.eh_frame` on Linux (and under `-g` in `.debug_frame`),
  as `.cfi_sections` chooses; on Linux `.cfi_personality` and `.cfi_lsda` (absolute,
  pc-relative and indirect, 4 and 8 bytes) let assembly take part in exception
  handling; `.cfi_return_column`, `.cfi_val_offset`, `.cfi_label`, and on AArch64
  `.cfi_b_key_frame`, `.cfi_mte_tagged_frame` and `.cfi_negate_ra_state_with_pc` are
  new. On macOS and Windows a personality routine, an LSDA and the CIE's other
  augmentations are errors. The `.seh_*` procedures (x86-64 and ARM64) are unwind
  records in `.xdata` with their `.pdata` entries on Windows. A function whose inline
  asm pushes, pops, calls or uses the stack pointer gets a frame, so its frame
  description stays exact; a template's `.cfi_rel_offset`/`.cfi_restore` say where it
  saved a register.
- Fixed: `.fill` without operands crashed the compiler, and `.fill`, `.space` and the
  alignment directives with an empty or a superfluous operand are errors.
