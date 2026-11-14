## C

- `axiom build file.c` and `axiom check file.c` compile C with the compiler's own front
  end, optimizer, code generators, assemblers and linkers, for every target
  (WebAssembly, x86-64 and AArch64 Linux, x86-64 Windows, AArch64 macOS, and the new
  `x86_64-macos` and `aarch64-windows`, whose output has not been run). Several `.c`
  files (and `.s`/`.S` assembly sources) are the translation units of one program.
  [`docs/c.md`](../c.md) describes it, with its remaining gaps in one list ("Known
  gaps"); the diagnostics (`C1xxx` to `C4xxx`) are in
  [`docs/c-diagnostics.md`](../c-diagnostics.md), the clause-by-clause account in
  [`docs/c-conformance.md`](../c-conformance.md).
- The standards are C89, C95, C99, C11, C17 (the default) and C23, and their GNU
  dialects, chosen with `--c-std=`. Each mode accepts its standard's language and rejects
  what a later one added or its own removed, naming the standard.
- It is a hosted implementation with the compiler's own C library (see the C library's
  notes); `--c-freestanding` makes it a freestanding one, and `--c-lib=system` gives a
  program the system's C library instead.
- Options: `--c-char`, `--c-int`, `--c-long`, `--c-long-double`, `-D`, `-U`, `-A`, `-I`,
  `--c-asm-dialect`, `--c-fenv-access`, `--c-keep-null-checks`, `--c-warn-format`,
  `--c-warn-unknown-pragmas`, `--c-system-include`, and `--emit=c-pp|c-tokens|c-ast` for
  the preprocessed text, the tokens and the syntax tree ([`docs/cli.md`](../cli.md)).
- Functions use each target's C ABI, structures, unions, vectors, complex values, wide
  integers, `long double` and variadic calls included, so C compiled here calls and is
  called by the platform's libraries (checked against the system's C compiler on x86-64
  and AArch64 Linux).
- Stack traces show C functions and the lines of their calls, natively and on
  WebAssembly, at every optimization level.
