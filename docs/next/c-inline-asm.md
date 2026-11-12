## C

- Inline assembly in the GNU dialects (`--c-std=gnu17` ...) on x86-64 and AArch64:
  `asm` statements with operands, constraints, clobbers and modifiers, `asm goto`,
  explicit register variables (`register long x asm("x8")`) as operands, file-scope
  `asm("...")` and the attribute `naked`. The compiler assembles the templates itself
  (the AT&T syntax on x86-64), with no other tools: the general-purpose instructions,
  BMI, SSE through SSE4.1 on x86-64; the base, atomic, system, floating-point and
  common vector instructions on AArch64. New diagnostics C3096 and C3097; on
  WebAssembly inline assembly stays C3901. See `docs/c.md`, "Inline assembly".
