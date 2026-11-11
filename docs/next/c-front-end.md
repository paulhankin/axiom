## C

- `axiom build file.c` and `axiom check file.c` compile C with the compiler's own front
  end, optimizer, code generators and linkers, for every target (WebAssembly, x86-64 and
  AArch64 Linux, x86-64 Windows, AArch64 macOS). Several `.c` files are the translation units
  of one program. [`docs/c.md`](../c.md) describes it; the diagnostics (`C1xxx` to
  `C4xxx`) are in [`docs/c-diagnostics.md`](../c-diagnostics.md).
- The standards are C89, C95, C99, C11, C17 (the default) and C23, chosen with
  `--c-std=`. Each mode accepts its standard's language and rejects what a later one
  added or its own removed, naming the standard.
- Other options: `--c-char=signed|unsigned`, `--c-int=16|32|64`, `--c-long=32|64`,
  `-DNAME[=value]`, `-UNAME`, `-Idir`, and `--emit=c-pp|c-tokens|c-ast` for the
  preprocessed text, the tokens and the syntax tree.
- It is a freestanding implementation: the compiler provides `<float.h>`, `<limits.h>`,
  `<stdarg.h>`, `<stddef.h>`, `<iso646.h>`, `<stdbool.h>`, `<stdint.h>`, `<stdalign.h>`,
  `<stdnoreturn.h>`, `<stdbit.h>` and `<stdckdint.h>` (each from the standard that has
  it), and no library. Functions a program declares and does not define come from the
  platform's C library (Linux, linked by the system's `cc`; macOS; Windows) or, on
  WebAssembly, are imports from the module `env`.
- Functions use each target's C ABI, structures by value and variadic calls included, so
  C compiled here calls and is called by the platform's libraries. Two cases of
  over-aligned variadic arguments are not matched (`docs/c.md`).
- Not supported: `_Complex`, `_Atomic`, decimal floating types, threads, `long double`
  wider than `double`, Annex F, and extensions of any kind; `#pragma` is ignored.
  `_BitInt` is up to 64 bits.
- Native stack traces show C functions and the lines of their calls
  (`--stack-traces`); on WebAssembly they stop at the C entry point.
