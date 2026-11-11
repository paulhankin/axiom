# The C front end: design and plan

Work in progress, on a branch. `axiom build file.c` compiles a C translation unit with
the compiler's own optimizer, backends and linkers. This file is the design the pieces
are written against; `docs/c.md` (to come) is for users.

## Scope

- **Standards**: C89/C90, C95 (Amendment 1), C99, C11, C17 and C23, chosen with
  `--c-std=`. Each mode accepts its standard's language and rejects what a later one
  added or its own removed (C23 has no K&R definitions, no implicit `int`).
- **Freestanding**: no C library. The compiler provides the headers a freestanding
  implementation must (`<stddef.h>`, `<stdint.h>`, `<limits.h>`, `<float.h>`,
  `<stdarg.h>`, `<iso646.h>`, `<stdbool.h>`, `<stdalign.h>`, `<stdnoreturn.h>`), in
  `std/c/include/`.
- **No extensions**: no GNU or Microsoft extensions, and `#pragma` lines are ignored
  (as are `_Pragma` operators). Identifiers the compiler needs for its own headers are
  in the reserved namespace (`__builtin_va_list`, `__axiom_int64`, ...).
- **Not supported, and said so by the feature macros**: `_Complex`
  (`__STDC_NO_COMPLEX__`), `_Atomic` (`__STDC_NO_ATOMICS__`), threads
  (`__STDC_NO_THREADS__`), decimal floating types, `long double` wider than `double`
  (it is `double`).
- **Implementation-defined choices** a build can make: `--c-char=signed|unsigned`,
  `--c-int=16|32|64`, `--c-long=32|64`. `short` is 16 bits, `long long` 64, pointers
  and `size_t` the target's. The defaults are the target's C ABI.
- **Macros and include paths**: `-DNAME`, `-DNAME=value`, `-UNAME`, `-Idir`.

## Rules

- Written from the standards (the ISO texts and their public drafts: N1256 for C99,
  N1570 for C11, N2176 for C17, N3220 for C23; the C89 rationale and ANSI text for C89).
  No code or tests are taken from another compiler or test suite.
- The system's C compiler (`cc -std=... -pedantic-errors -ffreestanding`) may be used
  while developing as an oracle for what a program prints or whether it is valid. Tests
  checked in carry their expected output; the suite does not run another compiler.
- The usual repository rules hold (`CLAUDE.md`): no mention of outside projects in
  code, comments, tests or docs; canonical formatting (`axiom fmt`).
- The C front end exists in the compiler only (`cmd/axiom/src/c/`); the bootstrap
  compiler does not have it and does not need it.

## Layout

```
cmd/axiom/src/c/copts.ax    options: the standard, the implementation's choices
cmd/axiom/src/c/ctok.ax     the tokens the preprocessor hands the parser
cmd/axiom/src/c/cpp.ax      preprocessor and lexer (phases 1-6)            (+ cpp_*.ax)
cmd/axiom/src/c/cast.ax     the syntax tree
cmd/axiom/src/c/cparse.ax   parser                                         (+ cparse_*.ax)
cmd/axiom/src/c/ctypes.ax   C types, layout, conversions
cmd/axiom/src/c/csema*.ax   semantic analysis: the typed tree
cmd/axiom/src/c/cgen*.ax    IR generation (ir.ax)
cmd/axiom/src/c/cc.ax       the driver
std/c/include/              the compiler's headers
cmd/axiom/internal/tests/c/ tests
```

## Interfaces

- `cpp.preprocess(path, opts) -> List[Token]`: the translation unit's tokens, ending in
  `T_EOF`. Phases 1-6 are done: trigraphs (C89-C17), line splicing, comments, macro
  expansion, `#include`, conditionals, string literal concatenation, conversion of
  numbers and character constants. Keywords are those of the standard in force
  (`ctok.keyword_id`).
- `cparse` builds `cast` trees from tokens. It resolves typedef names (it must, to
  parse) but does no type checking.
- `csema` turns the tree into a typed tree: every expression has its type and value
  category, implicit conversions are explicit nodes, constant expressions are
  evaluated, initializers are in their normal form (one entry per scalar subobject),
  objects and types have their layout.
- `cgen` writes the typed tree into an `ir.Module`, which the existing
  `backend.emit_module` compiles.
- Diagnostics go through `diag.ax` with spans into the source files (`util.load_file`).
  Codes: `C1xxx` preprocessing and lexing, `C2xxx` syntax, `C3xxx` constraints and
  semantics, `C4xxx` warnings. A constraint violation is an error; the messages name
  the construct in C's terms.

## Command line

```
axiom build file.c [-o out] [--target=...] [-O0..-O3]
    --c-std=c89|c95|c99|c11|c17|c23      (default c17)
    --c-char=signed|unsigned  --c-int=16|32|64  --c-long=32|64
    -DNAME[=value]  -UNAME  -Idir
    --emit=c-pp     the preprocessed text
    --emit=c-ast    the syntax tree, as text
    --emit=obj|ir   an object, or the IR
```

## Programs

A C program's `main` is called by a start-up module of the runtime, which the build
adds; functions the program declares and does not define are imports (WebAssembly:
from the module `env`) or undefined symbols for the linker.

## Tests

`cmd/axiom/internal/tests/c/`: each `.c` file is a test. Directives are in ordinary
comments, so that they are valid in every standard:

```c
/* flags: --c-std=c89 */
/* expect: 42 */            one line of expected output, in order
/* expect-error: C3012 */   the build must fail with this code
```

Programs print through functions the test host provides (`print_i64`, `print_u64`,
`print_f64`, `print_str`), declared in `cmd/axiom/internal/tests/c/host.h`.
Preprocessor tests compare `--emit=c-pp` output with a `.pp` file next to them; parser
tests compare `--emit=c-ast` with a `.ast` file.

## Phases

1. Preprocessor and lexer; syntax tree and parser. (parallel)
2. The core: types and layout, the typed tree, IR generation for the scalar types,
   pointers, arrays, structures, functions, the statements; program start-up.
3. Completion, by area (parallel): expressions; declarations, initializers and layout;
   control flow, `goto`, variable-length arrays; variadic functions and the headers.
4. Each standard's particulars, diagnostics for constraint violations, documentation,
   larger programs.
