# The C front end: design

`axiom build file.c` compiles C translation units with the compiler's own optimizer,
backends and linkers. This file is the design of the front end, for those who work on
it; [`docs/c.md`](c.md) is for users and [`docs/c-diagnostics.md`](c-diagnostics.md)
lists the diagnostics.

## Scope

- **Standards**: C89/C90, C95 (Amendment 1), C99, C11, C17 and C23, chosen with
  `--c-std=`. Each mode accepts its standard's language and rejects what a later one
  added or its own removed (C99 has no implicit `int` or implicit function
  declarations, C23 no K&R definitions).
- **Freestanding**: the compiler provides the headers a freestanding implementation
  must (`<stddef.h>`, `<stdint.h>`, `<limits.h>`, `<float.h>`, `<stdarg.h>`,
  `<iso646.h>`, `<stdbool.h>`, `<stdalign.h>`, `<stdatomic.h>`, `<stdnoreturn.h>`, and C23's `<stdbit.h>`
  and `<stdckdint.h>`), in `std/c/include/`, and of the library what C23 (4p7) asks of a
  freestanding implementation in `<string.h>` (all but `strdup`, `strndup`, `strcoll`,
  `strxfrm` and `strerror`): `std/c/include/string.h`, with one C source per function in
  `std/c/lib/string/`, units of the C library (main.ax `c_library`; kept with
  `--c-freestanding`). C23's `<stdlib.h>` part (and `<fenv.h>`, `<math.h>` when it claims
  Annex F) is not provided. `<float.h>` has no `FLT_SNAN`, `DBL_SNAN`, `LDBL_SNAN`
  (signaling NaNs are not guaranteed to stay signaling).
- **Calling convention**: each target's C ABI (x86-64 System V, Windows x64,
  AAPCS64 and its macOS variant, the WebAssembly C ABI), for structures and
  unions by value and variadic functions too (`cgen_abi.ax`, `cgen_va.ax`), so
  that C compiled here calls and is called by the platform's C libraries, arguments
  aligned to more than 16 bytes included (an x86-64 System V call that puts one on the
  stack aligns its stack pointer for it: `IN_STACK_SAVE`, `IN_STACK_ALLOC` and
  `IN_STACK_RESTORE` around the call). Bit-fields are laid out by the target's ABI
  (`csema_bits.ax`: System V, AAPCS64, Windows x64). `tools/gen_c_abi.py` writes random
  tests of all of it, whose units can be built by different compilers.
- **Extensions only in the GNU dialects** (`--c-std=gnu89` ... `gnu23`, COptions.gnu:
  docs/c.md lists them): the keywords of `ctok.keyword_id_in`, the GNU attributes
  (`csema_gnu.ax`; `cfeat.ax` names them for `__has_attribute`), the builtins
  (`csema_gbuiltin.ax`, `cgen_gbuiltin.ax`, the atomic ones `cgen_gatomic.ax`), statement
  expressions, labels as values, case ranges and the rest (each where its construct is).
  No Microsoft extensions. Pragmas are read in every mode (`cpp.ax`; what the later phases
  need is in `cpragma.ax`). Identifiers the compiler needs for its own headers are in the
  reserved namespace (`__builtin_va_list`, `__axiom_int64`, ...).
- **Atomics and thread storage**: `_Atomic` types (csema_atomic.ax) are
  accessed atomically (cgen_atomic.ax: lock-free for 1, 2, 4 and 8 bytes, a
  table of spinlocks otherwise; every order sequentially consistent);
  `<stdatomic.h>`'s generic functions are builtins. Thread-local objects are the
  IR's thread-local globals natively, and per-thread blocks on WebAssembly with
  threads (cgen_tls.ax). The runtime's threads are reached through
  `__axiom_c_thread_start` and `__axiom_c_thread_join` (the start-up
  `std/internal/runtime/c_threads.ax`), until there is a `<threads.h>`.
- **Complex types** (C99): `float`, `double` and `long double _Complex`, with
  `<complex.h>` (the library's functions only declared). No imaginary types, and
  `__STDC_IEC_559_COMPLEX__` is not defined, though multiplication and division
  follow Annex G.5.1 (`ccomplex.ax` folds constants, `cgen_complex.ax` generates the
  same operations, with helper functions of the module for the slow paths).
- **Not supported, and said so by the feature macros**: `<threads.h>`
  (`__STDC_NO_THREADS__`), decimal floating types. Annex F is not claimed (`__STDC_IEC_559__` and
  `__STDC_IEC_60559_BFP__` are not defined): the floating types have IEC 60559's
  formats (`FLT_IS_IEC_60559`), but without a library there is no `<fenv.h>` or
  `<math.h>`, and no exception flags or rounding modes.
- **Implementation-defined choices** a build can make: `--c-char=signed|unsigned`,
  `--c-int=16|32|64`, `--c-long=32|64`, `--c-long-double=64|80|128`. `short` is 16 bits, `long long` 64, pointers
  and `size_t` the target's. The defaults are the target's C ABI:

  | target | plain `char` | `long` | `wchar_t` |
  |---|---|---|---|
  | `x86_64` (Linux; x86-64 System V) | signed | 64 | `int` |
  | `aarch64` (Linux; AAPCS64) | unsigned | 64 | `unsigned int` |
  | `aarch64-macos` (Apple's arm64 ABI) | signed | 64 | `int` |
  | `x86_64-windows` (Windows x64) | signed | 32 | `unsigned short` |
  | `wasm32`, `wasm64` (the WebAssembly C ABI) | signed | 32, 64 | `int` |

  `long double` is the x87's 80-bit format on `x86_64` Linux, binary128 on `aarch64`
  Linux and WebAssembly, and binary64 elsewhere (`c/run/hdr_float_ld.c`).
  (`c/run/target_*.c` check them.) The typedefs of `<stddef.h>` and `<stdint.h>` are
  the platform's types (`copts.set_platform`; the headers read them from predefined
  `__AXIOM_*_TYPE__` macros), so that `size_t` is `unsigned long` on wasm32 and
  `int64_t` is `long long` on macOS, as there.
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
cmd/axiom/src/c/copts.ax    options: the standard (and GNU dialect), the implementation's choices
cmd/axiom/src/c/cpragma.ax  what pragmas say after preprocessing (pack, weak, GCC diagnostic)
cmd/axiom/src/c/cfeat.ax    the GNU attributes and builtins implemented (__has_attribute, __has_builtin)
cmd/axiom/src/c/ctok.ax     the tokens the preprocessor hands the parser
cmd/axiom/src/c/cpp.ax      preprocessor and lexer (phases 1-6)            (+ cpp_*.ax)
cmd/axiom/src/c/cast.ax     the syntax tree                          (+ cast_dump.ax)
cmd/axiom/src/c/cparse.ax   parser
cmd/axiom/src/c/ctypes.ax   C types, layout, conversions
cmd/axiom/src/c/cconst.ax   constant expressions                        (+ ccomplex.ax)
cmd/axiom/src/c/cldbl.ax    long double constants wider than double: read exactly, folded
cmd/axiom/src/c/csema*.ax   semantic analysis: the typed tree (csema_atomic.ax: atomic types, <stdatomic.h>)
cmd/axiom/src/c/cgen*.ax    IR generation (ir.ax), the platforms' calling conventions
                            (cgen_complex.ax: complex arithmetic; cgen_ld.ax: long double
                            wider than double; cgen_atomic.ax: atomic operations;
                            cgen_tls.ax: thread storage)
std/c/rt/ldouble.ax         that long double's arithmetic, in software (the compiler's too)
cmd/axiom/src/c/cc.ax       the driver, and the link of the units (with Axiom's)
cmd/axiom/src/c/cstrip.ax   leaves out of an executable what the units define and nothing reaches
std/c/include/              the compiler's headers
std/c/lib/                  the library's sources (C), one function to a file
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
- `long double` wider than `double` (`cty.is_wide`): a value is 16 bytes of memory,
  which cgen handles as it does a structure's (its address), its operations are calls
  of `std/c/rt/ldouble.ax`'s functions (binary128 and the x87's format, written from IEEE
  754-2019; codegen.ax generates those a program calls, `cg.ld_fn`, into its module),
  and its constants are values of that format (`Expr.xv`): floating constants read
  exactly (`cldbl.ax`, in arbitrary precision), folded with the same functions. The ABIs
  pass it in memory or as pieces (cgen_abi.ax): a 128-bit vector for a register that
  holds a binary128 (`v`, `xmm`), and, for an x87 result in `st(0)`, the intrinsics
  `IN_X87_LOAD` and `IN_X87_STORE` (`fld`, `fstp`: the x86-64 backend's only x87 code)
  just before the return and just after the call.
- Diagnostics go through `diag.ax` with spans into the source files (`util.load_file`).
  Codes: `C1xxx` preprocessing and lexing, `C2xxx` syntax, `C3xxx` constraints and
  semantics, `C4xxx` warnings. A constraint violation is an error; the messages name
  the construct in C's terms.
  A `#line` directive marks the lines after it in the file (`util.mark_lines`): the
  text format shows the presumed file and line, the `agent-json` format the physical
  span with `presumed_file` and `presumed_line` beside it. A feature of a later
  standard (or one an earlier one removed) gets one diagnostic naming the standard.

## Command line

```
axiom build file.c... [-o out] [--target=...] [-O0..-O3]
axiom check file.c...           the units are analyzed and linked, nothing is written
    --c-std=c89|c90|c95|c99|c11|c17|c18|c23      (default c17)
            gnu89|gnu90|gnu99|gnu11|gnu17|gnu18|gnu23   the GNU dialects
    --c-warn-unknown-pragmas
    --c-char=signed|unsigned  --c-int=16|32|64  --c-long=32|64  --c-long-double=64|80|128
    -DNAME[=value]  -UNAME  -Idir   (also -D NAME, -U NAME, -I dir; in order)
    --emit=c-pp     the preprocessed text           (one source; standard output
    --emit=c-tokens the tokens, one to a line        without -o)
    --emit=c-ast    the syntax tree, as text
    --emit=obj|ir   an object, or the IR
    --format=agent-json   the diagnostics as JSON (warnings do not fail a build)
```

The other options of `axiom build` that apply: `--target`, `--cpu`, `--features`,
`-O0`..`-O3`, `--stack-traces` (on native targets C functions' calls have call-site
records, so stack traces show C frames with their lines; on WebAssembly they have none),
`-g` (debug information: `cgen_debug.ax` describes the types and variables, which
`dwarf.ax` writes; at `-O0` the objects of variables stay in memory, `ir.F_VAR`), `-l`, `-L`, `--link=`
(objects, libraries, and C sources for the system's C compiler), `--subsystem`, `--icon`,
`--pdb`. Those of Axiom code (`--overflow`, `--sanitize`, `--coverage`,
`--units`, `--emit-header`) are errors with C sources (an Axiom program with C units
takes `--overflow` and `--emit-header`), as are the C options without one, and `axiom
test`, `profile`, `ide` and `fmt` of a C source. A missing source or include directory and an
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

## C and Axiom in one program

`axiom build main.ax x.c` checks the Axiom program (sema), then the C units
(`main.ax`: `check_c_units`), links the units (`link_check`, with no `main` required)
and checks the names both define (`cc.boundary_check`: `C3065`, `C3066`); the units'
IR joins the Axiom program's module as it joins a C program's start-up module.
`axiom build main.c rt.ax` is a C program whose start-up module has the Axiom modules
beside it (sema's extra files). In both, every unit and module is one IR module:

- C's references go to the module's functions by name (`cgen.func_of`): a declaration
  of a function that Axiom defines (an `export fn`) is that function; an Axiom
  `extern "C"` declaration that a unit defines is given the definition's signature
  (`ir.retype_decl`: a variadic definition has more parameters) and body.
- The Axiom modules under `<std>/c/rt/` are a library's (`codegen.ax`:
  `lazy_export`): their exports are not roots, and are generated when something
  reaches them (`define_lazy`), unless a C unit defines the name. The C sources under
  `<std>/c/lib/` are a library's units (`cc.check` with `lib`), whose definitions give
  way to the program's (`link_check`). `main.ax`'s `c_library` adds both; nothing is
  there yet.
- An executable that our own linkers link is stripped (`cstrip.ax`) before the
  optimizer: the roots are everything Axiom generated (which it generates only when
  reachable or exported), the start-up's call of `main`, and the functions code
  generation calls by name; everything the units define that the roots do not reach,
  through instructions' operands, call-site records and initializers, is removed.
- `exit`: when the program defines it (a unit, or an Axiom export), the start-up calls
  it with `main`'s value (`cgen.generate_start`), as it calls the C library's on
  Windows.

Tests: each directory of `cmd/axiom/internal/tests/c/mix/` is a program of both
languages (`[c-mix]` in `run.mjs`: its `main.ax` or `main.c` with the directory's
sources, built and run on each target, printing through `c/mix/print.ax`); the `C
programs` section has the command line, a package's `c-sources`, the size of what is
linked, the library directories, the Windows and macOS builds.

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
/* expect-error: C3022 */   the build must fail with this code
/* expect-warning: C4040 */ the build must warn with this code
/* check-only */            only checked (axiom check, with the flags' --target)
/* units: abi/x.c */        more translation units, relative to the test
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
`c/prog/` holds larger programs (300 to 1500 lines each, combining features, in
each standard and with `--c-int`/`--c-long` choices), built and run the same way
(`[c-prog]`); their expected output was checked against another implementation.
`tools/gen_c_programs.py` writes random programs free of undefined behaviour for
differential testing, and `tools/gen_c_abi.py` random tests of the calling conventions
(its `--host` output is `c/run/abi_gen_*.c`, with their callees in `c/run/abi/`).
Preprocessor tests compare `--emit=c-pp` output with a `.pp` file next to them; parser
tests compare `--emit=c-ast` with a `.ast` file.

## Phases

A translation unit goes through these, in order. Errors in the first four stages stop
the build before code is generated; the parser and the analysis go on after an error to
report more.

1. **Preprocessing and lexing** (`cpp*.ax`): translation phases 1 to 6, ending in the
   tokens of `ctok.ax`. `--emit=c-pp` and `--emit=c-tokens` stop here.
2. **Parsing** (`cparse.ax`): the syntax tree, with typedef names resolved and no types.
   `--emit=c-ast` stops here.
3. **Semantic analysis** (`csema*.ax`, `ctypes.ax`, `cconst.ax`): the typed tree and the
   constraint diagnostics. `axiom check` stops here, after step 4.
4. **Link check** (`cc.ax`, `link_check`): once all the units are analyzed, one
   definition of each external name and one `main`.
5. **IR generation** (`cgen*.ax`): the typed tree of each unit into the IR module of the
   start-up (`c_start.ax`) or of the object (`c_object.ax`). The few diagnostics that
   only this stage can give (`C3062`, `C4053`) come here.
6. **The compiler's back end**: optimization, code generation and linking, as for Axiom.
