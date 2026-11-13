## C front end

- x86-64 inline and file-scope assembly can be written in the Intel syntax:
  `.intel_syntax noprefix` switches to it (`.att_syntax` back), and
  `--c-asm-dialect=intel` makes it the syntax of every template and file-scope `asm`,
  selects the second alternative of `{att|intel}` in templates and writes operands as
  that syntax has them (`DWORD PTR [rbp-8]`, `OFFSET FLAT:sym`, registers without
  `%`). The comparison pseudo-ops (`vcmpltps`, `vpcmpnleud` ...) are accepted in both
  syntaxes, and AVX-512 compares into a mask register take a write mask.
- The constraint `A` puts an `__int128` operand in the rdx:rax pair (a value of up to
  64 bits in rax).
- Fixed: `mov $imm, %ah` (and `%bh`, `%ch`, `%dh`) was encoded with the wrong register.
- Call frame information of assembly: file-scope asm's `.cfi_*` procedures become FDEs
  in `.debug_frame` under `-g`, and its `.seh_*` procedures (x86-64 and ARM64) unwind
  records in `.xdata` with their `.pdata` entries on Windows. A function whose inline
  asm pushes, pops, calls or uses the stack pointer gets a frame, so its frame
  description stays exact; a template's `.cfi_rel_offset`/`.cfi_restore` say where it
  saved a register.
- Inline assembly's labels are symbols of the object (`.globl` ones global), its
  `.pushsection` ... `.popsection` put bytes in other sections, and templates and
  file-scope asm can name static functions and objects.
- Assembly sources are translation units: `axiom build main.c x.s y.S` (a `.S` is
  preprocessed with `__ASSEMBLER__`), also beside an Axiom program and alone with
  `--emit=obj`, with line information under `-g`.
- Inline assembly's semantics follow the dialect's rules: the cheapest alternative of
  constraints like `"r,m"` (with `?` and `!`), outputs in registers given to the register
  allocator, an asm without `volatile` and memory operands removed when unused, merged
  and moved like an operation, a `volatile` one without `"memory"` no barrier to memory,
  sibling calls from functions whose asm does not use the stack.
