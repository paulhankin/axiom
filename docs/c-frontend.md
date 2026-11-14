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
- **Hosted, with the C library**: `__STDC_HOSTED__` is 1 and the C library
  (`std/c/include/`, `std/c/lib/`, `std/c/rt/`: docs/c.md, "The C library") is linked;
  `--c-freestanding` (`COptions.hosted` false) makes `__STDC_HOSTED__` 0 and finds only
  the freestanding headers (`<stddef.h>`, `<stdint.h>`, `<limits.h>`, `<float.h>`,
  `<stdarg.h>`, `<iso646.h>`, `<stdbool.h>`, `<stdalign.h>`, `<stdatomic.h>`,
  `<stdnoreturn.h>`, C23's `<stdbit.h>` and `<stdckdint.h>`, `<complex.h>` and
  `<string.h>`: `cpp.ax`, `freestanding_header`), and of the library what C23 (4p7) asks
  of a freestanding implementation in `<string.h>` (`std/c/lib/string/`, kept with
  `--c-freestanding`). `<float.h>` has no `FLT_SNAN`, `DBL_SNAN`, `LDBL_SNAN`
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
- **Inline assembly** (GNU dialects; x86-64 and AArch64): an asm statement is the IR's
  opaque intrinsic `IN_ASM`, whose first operand indexes its description
  (`asm/inline.ax`: the template's pieces, each operand's kind, class, register, width
  and tie, the clobbers); outputs in registers go through temporaries. The backends
  (`x64.ax`, `a64.ax`: `gen_asm`) make one machine instruction of it whose register
  operands the allocator places (pinned ones copied into and out of their registers,
  outputs and clobbers written early), then render the operands into the template and
  assemble it (`asm/x64.ax`, `asm/a64.ax`) into the function's code at its place.
  Semantic analysis assembles each template with stand-in operands for its
  diagnostics. File-scope asm and naked functions are assembled into the object's
  sections (`asm/obj.ax`). `c/run/asm_table_*.c` check thousands of instruction
  encodings.
- **Atomics and thread storage**: `_Atomic` types (csema_atomic.ax) are
  accessed atomically (cgen_atomic.ax: lock-free for 1, 2, 4 and 8 bytes, a
  table of spinlocks otherwise; every order sequentially consistent);
  `<stdatomic.h>`'s generic functions are builtins. Thread-local objects are the
  IR's thread-local globals natively, and per-thread blocks on WebAssembly with
  threads (cgen_tls.ax). Threads are the C library's `<threads.h>`
  (`std/c/lib/threads/`, `std/c/rt/threads*.ax`), on the runtime's threads.
- **Complex types** (C99): `float`, `double` and `long double _Complex`, with
  `<complex.h>` (the library's functions only declared), and the imaginary types
  (Annex G: an imaginary value is its real type's coefficient; csema_expr.ax's
  `imag_domain` gives each operator's domain by G.5, cconst.ax folds, cgen_expr.ax's
  `imag_arith` generates). `__STDC_IEC_559_COMPLEX__` is not defined, though
  multiplication and division follow Annex G.5.1 (`ccomplex.ax` folds constants,
  `cgen_complex.ax` generates the same operations, with helper functions of the module
  for the slow paths).
- **Interchange and extended floating types** (Annex H): `_Float16` is held as its
  bits (an `i16`) and computed in `float`, rounded once (cgen_half.ax, ldouble.ax's
  `h_from_*`); `_Float128` and `_Float64x` are wide formats as a wide `long double` is
  (cgen_ld.ax takes the format from the type: `cconst.fmt_t`). Ranks: ctypes.ax
  `float_rank`.
- **GNU vector types**: ctypes.ax `vector_of` (TK_VECTOR), csema_vec.ax (the operand
  rules), cgen_vec.ax (IR vectors in chunks of at most 64 bytes, lane-by-lane code where
  the target has no instruction), cgen_abi.ax (the platforms' classes).
- **Nested functions** (GNU): csema_nest.ax marks the enclosing objects a nested
  function uses; cgen_nest.ax builds the frame record they are reached through (a chain
  argument, `abi.add_chain`) and the trampoline pools, and the non-local gotos to its
  local labels (a `jmp_buf` armed before each call).
- **restrict** beyond parameters: cgen_restrict.ax marks a block's restricted pointer
  objects' values and restrict-qualified members' reads (`ir.IN_RSCOPE`,
  `ir.IN_RESTRICT`), and tags the loads, stores and calls made in the block with its
  scope (`ir.rtag`), which `ir.restrict_apart` and `ir.restrict_tagged` read; its header
  argues why that is sound. inline.ax makes an inlined call of a function with restrict
  parameters a scope and tags the inlined code.
- **Format checks**: csema_format.ax, from the attribute `format` (csema_gnu.ax).
- **Decimal floating types** (C23: `_Decimal32`, `_Decimal64`, `_Decimal128`, `df`
  `dd` `dl` constants; `__STDC_IEC_60559_DFP__`; Annex H's `_Decimal64x`, `d64x`,
  in decimal128's format, `cty.TK_DEC64X`, ranked by `cty.dec_rank`): IEEE 754-2019's decimal formats in
  the BID encoding, computed in software (`std/c/rt/decimal.ax`, below), with the
  decimal rounding direction of `fe_dec_setround` and the exceptions in `<fenv.h>`'s
  flags; the library's decimal functions are `std/c/rt/dfp.ax` and `dfp_math.ax`.
- **Not claimed**: Annex F for the binary types (`__STDC_IEC_559__` and
  `__STDC_IEC_60559_BFP__` are not defined): the floating types have IEC 60559's
  formats (`FLT_IS_IEC_60559`); the library's `<fenv.h>` reads and sets the rounding
  modes and flags (through `__builtin_axiom_fenv_get`/`_set`: `csema_fp.ax`,
  `cgen_fp.ax`, the intrinsics `IN_FPENV_GET`/`SET`), but the optimizer assumes the
  default environment.
- **Implementation-defined choices** a build can make: `--c-char=signed|unsigned`,
  `--c-int=16|32|64`, `--c-long=32|64`, `--c-long-double=64|80|128`. `short` is 16 bits, `long long` 64, pointers
  and `size_t` the target's. The defaults are the target's C ABI:

  | target | plain `char` | `long` | `wchar_t` |
  |---|---|---|---|
  | `x86_64` (Linux; x86-64 System V) | signed | 64 | `int` |
  | `aarch64` (Linux; AAPCS64) | unsigned | 64 | `unsigned int` |
  | `aarch64-macos` (Apple's arm64 ABI) | signed | 64 | `int` |
  | `x86_64-macos` (Apple's x86-64 ABI, System V's) | signed | 64 | `int` |
  | `x86_64-windows` (Windows x64) | signed | 32 | `unsigned short` |
  | `aarch64-windows` (Windows ARM64) | signed | 32 | `unsigned short` |
  | `wasm32`, `wasm64` (the WebAssembly C ABI) | signed | 32, 64 | `int` |

  `long double` is the x87's 80-bit format on `x86_64` Linux and macOS, binary128 on `aarch64`
  Linux and WebAssembly, and binary64 elsewhere (`c/run/hdr_float_ld.c`).
  (`c/run/target_*.c` check them.) The typedefs of `<stddef.h>` and `<stdint.h>` are
  the platform's types (`copts.set_platform`; the headers read them from predefined
  `__AXIOM_*_TYPE__` macros), so that `size_t` is `unsigned long` on wasm32 and
  `int64_t` is `long long` on macOS, as there.
- **Fixed choices**: `BITINT_MAXWIDTH` is 65535 (a `_BitInt(N)` is stored in the
  smallest of 8, 16, 32 and 64 bits that holds it, or wider in 64-bit limbs, on
  AArch64 in pairs of them); `__int128` in the GNU dialects; an inline definition (6.7.4p7)
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
cmd/axiom/src/c/ctarget.ax  the target's conventional predefined macros (__x86_64__, __SIZE_TYPE__ ...)
cmd/axiom/src/c/cast.ax     the syntax tree                          (+ cast_dump.ax)
cmd/axiom/src/c/cparse.ax   parser
cmd/axiom/src/c/ctypes.ax   C types, layout, conversions
cmd/axiom/src/c/cconst.ax   constant expressions                        (+ ccomplex.ax)
cmd/axiom/src/c/cldbl.ax    long double constants wider than double: read exactly, folded
cmd/axiom/src/c/cdec.ax     decimal floating constants: read exactly (with their quanta), folded
cmd/axiom/src/c/cwide.ax    integer constants wider than 64 bits, folded
cmd/axiom/src/c/csema*.ax   semantic analysis: the typed tree (csema_atomic.ax: atomic types, <stdatomic.h>;
                            csema_fp.ax: the builtins of <math.h>, <tgmath.h> and <fenv.h>,
                            generated by cgen_fp.ax)
cmd/axiom/src/c/csema_asm.ax   asm statements (a GNU dialect): constraints, operands, clobbers, references
cmd/axiom/src/c/cgen_asm.ax    their IR (ir.IN_ASM), naked functions
cmd/axiom/src/asm/          inline assembly: inline.ax (the IN_ASM description shared with the backends),
                            core.ax (statements, expressions, labels, sections, directives, passes),
                            x64.ax and a64.ax (the assemblers), check.ax (stand-in operands),
                            obj.ax (file-scope asm into the object)
cmd/axiom/src/c/cgen*.ax    IR generation (ir.ax), the platforms' calling conventions
                            (cgen_complex.ax: complex arithmetic; cgen_ld.ax: long double
                            wider than double; cgen_dec.ax: the decimal floating types;
                            cgen_wint.ax: integers wider than 64 bits;
                            cgen_atomic.ax: atomic operations; cgen_tls.ax: thread storage)
std/c/rt/ldouble.ax         that long double's arithmetic, in software (the compiler's too)
std/c/rt/decimal.ax         the decimal floating arithmetic and conversions, in software (the compiler's too;
                            dec_env.ax: the thread's decimal rounding direction and flags)
std/c/rt/dfp.ax, dfp_math.ax, dfp_env.ax   the library's decimal functions (<math.h>, <fenv.h>)
std/c/rt/wideint.ax         the integers wider than 64 bits: what is not inline (the compiler's too)
cmd/axiom/src/c/cc.ax       the driver, and the link of the units (with Axiom's)
cmd/axiom/src/c/cstrip.ax   leaves out of an executable what the units define and nothing reaches
std/c/include/              the compiler's headers
std/c/lib/                  the library's sources (C), one function to a file (math:
                            std/c/lib/math/)
std/c/rt/                   the library's parts in Axiom (math: std/c/rt/math*.ax)
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
- The decimal floating types (`cty.is_decimal`): a `_Decimal32` or `_Decimal64` value
  is an integer of its width in the IR (its BID encoding: registers, no memory), a
  `_Decimal128` (or `_Decimal64x`) 16 bytes of memory as a wide long double is (`cty.is_dec128`). Their
  operations are calls of `std/c/rt/decimal.ax`'s `dec_` functions (`cgen_dec.ax`,
  through `cg.ld_fn`; `main.ax` loads the module when a unit operates on them:
  `cty.g_used_dec`), which round in the thread's decimal rounding direction and raise
  their exceptions in `dec_env.ax`, which `<fenv.h>` merges into the hardware's flags.
  Constants are their encodings (`Expr.xv`): decimal floating constants read exactly
  (`cdec.ax`), folded with the same functions (rounding to nearest). The usual
  arithmetic conversions do not mix them with the standard floating or complex types
  (`C3230`). The ABIs pass them as the binary types of their sizes (cgen_abi.ax; the
  narrow ones bit-cast to the floating-point piece at the boundary), as types of their
  own in AAPCS64's homogeneous aggregates, as integers on Windows x64.
- Integers wider than 64 bits (`cty.is_wint`: `__int128`, `_BitInt(N)` for N > 64):
  a value is its 64-bit limbs in memory, which cgen handles as it does a structure's (its
  address; scalar replacement makes the two limbs of a 128-bit value two SSA values
  again). `cgen_wint.ax` generates two-limb operations inline (`IN_UMULH` for the
  product's high half on the native targets) and calls `std/c/rt/wideint.ax`'s
  functions (`wi_*`, through `cg.ld_fn`, generated only when called; `main.ax` loads the
  module when a unit uses such a type: `cty.g_used_wint`) for wider ones and for
  division; constants are folded by `cwide.ax` with the same functions (an `Expr`'s
  value: its low limb in `cval`, all of it in a table indexed by `xv.hi`). The ABIs pass
  them as the structures of their limbs (cgen_abi.ax), but on Windows an `__int128`
  result is in `xmm0` and on WebAssembly a 16-byte one is two `i64`s. The backends
  recognize the carry idiom the two-limb code makes, x + (p <u q) and x - (p <u q),
  as adc and sbb (x64.ax `mark_carry`, a64.ax: adc, sbc after the add's or sub's own
  flags); x86-64's two-limb division is a generated function
  (`__axiom_c_udivmod128`) around `ir.IN_UDIV128`, the 128-by-64-bit `div`.
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
(objects, libraries, and C sources compiled to objects first), `--subsystem`, `--icon`,
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
- `exit`: when the program defines it (a unit, the library's, or an Axiom export), the
  start-up calls it with `main`'s value (`cgen.generate_start`, converted to the
  width of the definition's `int`), as it calls the C library's on Windows. A library
  unit's definition gives way to an Axiom module's `export fn` of the same name
  (`cc.boundary_check`), as to a program's unit's (`link_check`).
- `setjmp` (`std/c/include/setjmp.h`) is `__axc_setjmp`, whose direct calls mark the
  calling function `returns_twice` (`cgen_expr.ax`: `ir.Func`): the optimizer leaves it
  alone but for folding and CFG simplification (`passes.ax`: no promotion of its
  objects to registers, no slot sharing, no value numbering), does not inline it or
  into it (`inline.ax`), makes no sibling calls in it, and the x86-64 backend does not
  shrink-wrap it, so that every object lives in a slot of its own across the call and
  nothing is in a register but what `longjmp` restores. The functions are machine code
  of `jstub.ax`, which every native linker adds (weak). On WebAssembly a call is
  `C3300`; its address taken, `C3301`.

The C library's tests are `cmd/axiom/internal/tests/c/lib/<part>/*.c` (`[c-lib]` in
`run.mjs`): hosted programs printing through `c/mix/print.ax`, built and run on each
target (wasm32, or `--native`), with `expect:`, `expect-status:`, `expect-signal:` (the
default action of a signal ends it), `expect-stderr:`, `expect-error:`, `flags:` and
`native-only`, `wasm-only`, `linux-only`, `posix-only`.

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
(`plink.ax`). The start-up calls the C library's `exit` with `main`'s value (the
compiler's library's; with `--c-freestanding` on Windows `msvcrt.dll`'s, and elsewhere
the system's start-up ends the process). A program may define the C library's functions itself
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
/* macos-only */            natively, only for a macOS target (AXIOM_NATIVE_TARGET=...-macos)
/* wasm-flags: --features=+tail-call */  more flags for a WebAssembly build
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
(its `--host` output is `c/run/abi_gen_*.c`, with their callees in `c/run/abi/`;
`--int128` and `--bitint` add the integers wider than 64 bits). `tools/gen_wint_tests.py`
writes `c/run/wint_gen_*.c`: every operation on those integers at run time and folded,
against Python's values. `tools/gen_dec_tests.py` writes `c/lib/math/dec_gen_*.c`: the
decimal floating operations and conversions in every decimal rounding direction, at run
time and folded, with the expected encodings and flags from Python's `decimal` module
and exact arithmetic; `gen_c_abi.py --dec` adds the decimal types to the ABI tests.
The hosted C library's tests are `c/lib/<part>/*.c` (`[c-lib]`: programs built with
the library, which print through it, with directives for standard input, standard
error and their order, `wasm-flags:` for a WebAssembly build's extensions and
`host-flags:` for the node host's files, `--files=memory` or `--root=dir`), and `c/mix/stdio_threads/` (streams written by several
threads). `c/lib/prog/` holds larger hosted programs (a text processor with wide
I/O, a calculator that recovers from errors with `longjmp`, sorting and searching of
200000 records, date reports, a work queue of `<threads.h>` and atomics, numeric
programs in `long double`, complex arithmetic and `<tgmath.h>`, the bit operations of
`_BitInt`, `__int128`, `<stdbit.h>` and `<stdckdint.h>`, every conversion of `printf` and
`scanf` with a logger's `va_list` passed down), whose output was compared with the system's compiler and
library at -O0 and -O2 (the differences, which are implementation-defined, are said in
each); `tools/gen_c_programs.py --hosted` writes random hosted programs for that
comparison, which print with `printf` and pass every value through the library's
conversions. `c/constraints.txt` (`tools/c_constraints.mjs`), `tools/c_limits.mjs`
and `c/headers/names.txt` (`tools/c_header_names.mjs`) are the conformance checks of
[`c-conformance.md`](c-conformance.md).
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
