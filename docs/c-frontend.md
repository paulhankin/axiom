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
  `<stdarg.h>`, `<iso646.h>`, `<stdbool.h>`, `<stdalign.h>`, `<stdnoreturn.h>`, and
  C23's `<stdbit.h>` and `<stdckdint.h>`), in `std/c/include/`.
- **Calling convention**: each target's C ABI (x86-64 System V, Windows x64,
  AAPCS64 and its macOS variant, the WebAssembly C ABI), for structures and
  unions by value and variadic functions too (`cgen_abi.ax`, `cgen_va.ax`), so
  that C compiled here calls and is called by the platform's C libraries.
- **No extensions**: no GNU or Microsoft extensions, and `#pragma` lines are ignored
  (as are `_Pragma` operators). Identifiers the compiler needs for its own headers are
  in the reserved namespace (`__builtin_va_list`, `__axiom_int64`, ...).
- **Not supported, and said so by the feature macros**: `_Complex`
  (`__STDC_NO_COMPLEX__`), `_Atomic` (`__STDC_NO_ATOMICS__`), threads
  (`__STDC_NO_THREADS__`), decimal floating types, `long double` wider than `double`
  (it is `double`).
- **Implementation-defined choices** a build can make: `--c-char=signed|unsigned`,
  `--c-int=16|32|64`, `--c-long=32|64`. `short` is 16 bits, `long long` 64, pointers
  and `size_t` the target's. The defaults are the target's C ABI:

  | target | plain `char` | `long` | `wchar_t` |
  |---|---|---|---|
  | `x86_64` (Linux; x86-64 System V) | signed | 64 | `int` |
  | `aarch64` (Linux; AAPCS64) | unsigned | 64 | `unsigned int` |
  | `aarch64-macos` (Apple's arm64 ABI) | signed | 64 | `int` |
  | `x86_64-windows` (Windows x64) | signed | 32 | `unsigned short` |
  | `wasm32`, `wasm64` (the WebAssembly C ABI) | signed | 32, 64 | `int` |

  (`c/run/target_*.c` check them.) Where the platforms' headers choose between
  types of the same width, this implementation's may choose the other (`size_t` is
  `unsigned int` on wasm32, `int64_t` is `long` on macOS): the representation and
  calling convention are the same.
- **Fixed choices**: `BITINT_MAXWIDTH` is 64 (a `_BitInt(N)` is stored in the
  smallest of 8, 16, 32 and 64 bits that holds it); an inline definition (6.7.4p7)
  is compiled with internal linkage for the unit's own calls, so the external
  function is another unit's.
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
  `csema.ax`'s header is the contract: how conversions, lvalues, pointer
  arithmetic, calls and initializers appear in the typed tree.
- `cgen` writes the typed tree into an `ir.Module`, which the existing
  `backend.emit_module` compiles. A program's root is the start-up module
  `std/internal/runtime/c_start.ax`, built as an Axiom program; the C unit's
  IR joins its module (`codegen.ax` calls `cc.gen_ir`) before the optimizer
  runs. C's `main` is `axiom.c.main`, called through `__axiom_c_main`.
- Diagnostics go through `diag.ax` with spans into the source files (`util.load_file`).
  Codes: `C1xxx` preprocessing and lexing, `C2xxx` syntax, `C3xxx` constraints and
  semantics, `C4xxx` warnings. A constraint violation is an error; the messages name
  the construct in C's terms.

## Command line

```
axiom build file.c... [-o out] [--target=...] [-O0..-O3]
axiom check file.c...           the units are analyzed and linked, nothing is written
    --c-std=c89|c90|c95|c99|c11|c17|c18|c23      (default c17)
    --c-char=signed|unsigned  --c-int=16|32|64  --c-long=32|64
    -DNAME[=value]  -UNAME  -Idir   (also -D NAME, -U NAME, -I dir; in order)
    --emit=c-pp     the preprocessed text           (one source; standard output
    --emit=c-tokens the tokens, one to a line        without -o)
    --emit=c-ast    the syntax tree, as text
    --emit=obj|ir   an object, or the IR
    --format=agent-json   the diagnostics as JSON (warnings do not fail a build)
```

The other options of `axiom build` that apply: `--target`, `--cpu`, `--features`,
`-O0`..`-O3`, `--stack-traces` (C functions' calls have call-site records, so
native stack traces show C frames with their lines), `-g` (accepted: this
compiler writes line tables for no code), `-l`, `-L`, `--link=` (objects,
libraries, and C sources for the system's C compiler), `--subsystem`, `--icon`,
`--pdb`. Those of Axiom code (`--overflow`, `--sanitize`, `--coverage`,
`--units`, `--emit-header`, `--link=file.ax`) are errors with C sources, as are the
C options without one, Axiom and C sources together, and `axiom test`, `profile`,
`ide` and `fmt` of a C source. A missing source or include directory and an
output that is a directory are errors before anything is compiled.

Several sources are the translation units of one program. Each is preprocessed,
parsed and analyzed on its own (its macros, types and internal names are its own:
internal names are numbered apart); then the units are linked (`cc.ax`:
`link_check`): an external name is defined in one unit at most (C3064: a second
definition, a tentative one included, is an error with a note at the first), and
an executable's units define `main` once (C3063). Their code is one IR module, so
a reference in one unit to another's definition is to that definition. An object
or IR (`--emit=obj|ir`) of units without `main` holds their code alone (its root
module is `std/internal/runtime/c_object.ax`: no start-up, no runtime), to be
linked into another program; with `main` it is the whole program's, as for Axiom.

## Programs

A C program's `main` is called by a start-up module of the runtime, which the build
adds; functions the program declares and does not define are imports (WebAssembly:
from the module `env`) or undefined symbols for the linker: Linux's C library (the
program is then linked with the system's C compiler), macOS's libSystem, and on
Windows `msvcrt.dll`, unless their names are those of the system DLLs' functions
(`plink.ax`). On Windows the start-up calls the C library's `exit` with `main`'s
value (which flushes its streams and runs the `atexit` functions); elsewhere the
system's start-up does. A program may define the C library's functions itself
(`memcpy`, `memset`, ... in a freestanding program): the runtime's own copies
(AArch64 Linux) are weak.

## Tests

`cmd/axiom/internal/tests/c/`: each `.c` file is a test. Directives are in ordinary
comments, so that they are valid in every standard:

```c
/* flags: --c-std=c89 */
/* expect: 42 */            one line of expected output, in order
/* expect-error: C3012 */   the build must fail with this code
/* expect-warning: C4040 */ the build must warn with this code
/* check-only */            only checked (axiom check, with the flags' --target)
/* linux-only */            natively, only where host_native.c is linked
```

Programs print through functions the test host provides (`print_i64`, `print_u64`,
`print_f64`, `print_str`), declared in `cmd/axiom/internal/tests/c/host.h`
(`#include "host.h"`; natively they are `c/host_native.c`'s, built by the system's
C compiler, or for Windows and macOS `c/host_c.c`'s, a second translation unit
over the platform's C library). The programs of `c/run/` are built for wasm32
(`AXIOM_C_TARGET=wasm64`: wasm64), or with `--native` for the machine
(`AXIOM_NATIVE_TARGET` and `AXIOM_NATIVE_EXEC` for another), at the suite's
optimization level; they may say `/* native-only */` or `/* wasm-only */`. The
`C programs` section tests the driver: its options, several units, its errors.
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
