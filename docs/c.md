# Compiling C

`axiom build file.c` compiles C. The C front end reads C translation units and hands them
to the same optimizer, code generators and linkers that Axiom programs use, so a C program
is built for every target of the compiler (WebAssembly, x86-64 and AArch64 Linux, x86-64
Windows, AArch64 macOS) with no C compiler and no external linker, except to link to the
platform's C library on Linux (see "What a C program is here").

It is a freestanding implementation, in C's terms: it has the freestanding headers and no
library. It reads C89, C95, C99, C11, C17 and C23 (`--c-std=`; C17 by default), each
mode accepting its standard's language and diagnosing what a later standard added or its
own removed. It has no extensions of any kind.

```
axiom build prog.c -o prog                       # wasm32: prog.wasm
axiom build prog.c --target=native -O2 -o prog   # this machine
axiom build a.c b.c --c-std=c99 -DDEBUG=1 -Iinclude -o prog
axiom check prog.c                               # diagnostics only
```

[`docs/c-diagnostics.md`](c-diagnostics.md) lists the diagnostics. How the front end is
built is in [`docs/c-frontend.md`](c-frontend.md). Axiom programs that link C code
compiled by the system's C compiler (`--link=file.c`) are a different feature, in
[`docs/cli.md`](cli.md).

## What a C program is here

- **No library.** There is no `<stdio.h>`, `<stdlib.h>`, `<string.h>` or any other
  library header, and no library code. `#include <stdio.h>` is the error `C1081`.
- **Undefined functions are the platform's.** A function a program declares and does not
  define is looked up outside the program:
  - On Linux, by the linker, in the system's C library: such a program is linked by the
    system's C compiler (`cc`), which must be installed. A program that defines every
    function it calls is linked by the compiler's own linker, and is a static executable
    that needs nothing.
  - On macOS, in `libSystem`. On Windows, in `msvcrt.dll`, unless the name looks like a
    Windows API function: a capitalized name is `kernel32.dll`'s (`WSA...` and the
    socket functions are `ws2_32.dll`'s, `Rtl...` and `Nt...` are `ntdll.dll`'s,
    `BCrypt...` are `bcrypt.dll`'s).
  - On WebAssembly, it is an import of the module `env`, with the name of the function.
    A host supplies it. The module also imports the runtime's host functions from the
    module `axiom` (`ax_write`, ...), which `std/internal/runtime/host.mjs` provides:
    `node std/internal/runtime/host.mjs prog.wasm` runs a program, with no `env`
    functions, so calling one fails with "host import env.name is not available".
- **A program may define the library's functions.** `memcpy`, `memmove` and `memset`
  among them: the runtime's own copies (on AArch64 Linux, which needs them) are weak.
- **Calls cross the boundary correctly.** Arguments and results follow the target's C
  ABI, so C compiled here calls the platform's library and is called from it ("Calling
  conventions", below).

## The command line

```
axiom build file.c... [options] [-o out]
axiom check file.c...  [options]
```

An input is a C source if its name ends in `.c`. Options take the forms `--name=value`
and `-DNAME`; they may come before or after the inputs. `-D NAME`, `-U NAME` and
`-I dir` (with a space) are accepted too. A build is of C sources or of an Axiom program,
never of both.

### C options

| Option | |
|---|---|
| `--c-std=<std>` | The standard (see "Standards"; default `c17`). |
| `--c-char=signed\|unsigned` | Whether plain `char` is signed (default: the target's C ABI). |
| `--c-int=16\|32\|64` | The width of `int` (default 32). |
| `--c-long=32\|64` | The width of `long` (default: the target's C ABI; at least `--c-int`). |
| `-DNAME`, `-DNAME=value` | Define a macro (`NAME` is `NAME=1`). `-D'F(x)=x+1'` defines a function-like macro. |
| `-UNAME` | Undefine a macro. The predefined macros of the standard cannot be undefined (`C1053`). |
| `-Idir` | Add a directory to the include path. |

`-D`, `-U` and `-I` apply in the order given. The C options are errors for an Axiom
source. The unknown ones are errors too: there are no `-W...`, `-std=`, `-c` or `-f...`
options, and no way to silence a warning.

### Options of `axiom build` that apply

`--target`, `--cpu`, `--features`, `-O0`..`-O3` (see "Optimization"), `--stack-traces`
(see "Stack traces"), `--format=agent-json`, `--std=<dir>` (the standard library's
directory, where the compiler finds the C headers in `c/include/`), `-o`, `--emit`,
`--link`, `-l`, `-L`, `--subsystem`, `--icon`, `--pdb`, and `-g` (accepted, and
changing nothing: the compiler's own code generators write no line tables). The default
target is `wasm32`. `--link=file.c` links a further C source, compiled by the system's C
compiler, as for Axiom programs; `-l` and `-L` pass libraries and directories to that
linker.

These are errors with C sources, since they mean something only for Axiom code:
`--overflow`, `--sanitize`, `--coverage`, `--units`, `--emit-header`, `--link=file.ax`.
So are `axiom test`, `axiom profile`, `axiom ide` and `axiom fmt` on a C source, and C
sources together with an Axiom source.

A missing source, a missing include directory (`-I`) and an output that is a directory are
errors before anything is compiled.

### `--emit`

| Value | Output |
|---|---|
| `exe` | The default: an executable (a `.wasm` module for WebAssembly targets). |
| `obj` | An object file. |
| `ir` | The compiler's IR as text. |
| `c-pp` | The preprocessed text: the tokens of the translation unit, on their lines. |
| `c-tokens` | The tokens, one to a line, with their kinds and values. |
| `c-ast` | The syntax tree, as text. No semantic analysis is done. |

`c-pp`, `c-tokens` and `c-ast` take one source, and print to standard output without
`-o`. `--emit=asm` is not supported. Without `-o`, the output is named after the first
source with `.c` removed: `prog`, `prog.wasm`, `prog.o` or `prog.ir`.

An object or IR of translation units that do not define `main` holds their code alone:
there is no start-up code and no runtime. The system's C compiler links such an object
into a C program (`cc main.c lib.o`). It cannot be linked into an Axiom program with
`--link=lib.o`: the link fails on the runtime symbols that the object defines (observed
with `--target=x86_64`).

### `axiom check`

Analyzes every unit and checks the link (below) and writes nothing. It reports what
`build` reports before generating code. A few diagnostics come from code generation and
only `build` reports them (`C3062`, `C4053`), and `check` does not require a `main` or
that undefined functions exist anywhere.

### Diagnostics

Errors and warnings go to standard error in text (`file:line:col: error[C3001]: ...`,
with the source line), or with `--format=agent-json` to standard output as the JSON of
spec §16.2. A warning does not fail the build; the exit status is 1 if there is an error.
A `#line` directive changes the file and line shown by the text format; the JSON gives
the physical position and `presumed_file` and `presumed_line` beside it.

## Several source files

```
axiom build main.c util.c -o prog
```

Each source is a translation unit: it is preprocessed, parsed and analyzed on its own,
with its own macros, types and internal names. Then the units are linked:

- An external name is defined in at most one unit. A second definition, a tentative one
  included, is `C3064`, with a note at the first.
- An executable's units define `main` once (`C3063` if not).
- A call or reference in one unit to another's definition is to that definition. Types
  are not compared between units: declare a function the same way in both, as a header
  does.

All the units are one program for the optimizer, so calls are inlined across them at
`-O2` and `-O3`.

## Standards

| `--c-std=` | Standard |
|---|---|
| `c89`, `c90`, `iso9899:1990` | ISO/IEC 9899:1990 |
| `c95`, `iso9899:199409` | C90 with Amendment 1 |
| `c99`, `iso9899:1999` | ISO/IEC 9899:1999 |
| `c11`, `iso9899:2011` | ISO/IEC 9899:2011 |
| `c17`, `c18`, `iso9899:2017`, `iso9899:2018` | ISO/IEC 9899:2018 (the default) |
| `c23`, `iso9899:2024` | ISO/IEC 9899:2024 |

Other names, and upper case ones, are errors. `__STDC_VERSION__` is not
defined for C89, and is `199409L`, `199901L`, `201112L`, `201710L` and `202311L` for the
others.

A mode accepts the language of its standard. A construct of a later standard, or one
that its own standard removed, is an error (one per construct) naming the standard that
has it: `'long long' is a C99 feature`. The main differences:

| From | Features |
|---|---|
| C89 | The base language. Trigraphs. Implicit `int` and implicit function declarations (warning `C4001`). K&R function definitions. |
| C95 | Digraphs, `<iso646.h>`, `__STDC_VERSION__`. |
| C99 | `//` comments, `long long`, `_Bool`, `inline`, `restrict`, variable length arrays, flexible array members, designated initializers, compound literals, declarations after statements and in `for`, variadic macros and empty macro arguments, `_Pragma`, hexadecimal floating constants, universal character names, `__func__`, `<stdint.h>`, `<stdbool.h>`. No more implicit `int` or implicit function declarations (`C3060`). |
| C11 | `_Generic`, `_Static_assert`, `_Noreturn`, `_Alignas`, `_Alignof`, `_Thread_local`, anonymous structures and unions, `u""`, `U""` and `u8""` literals and `u''` and `U''` constants, `<stdalign.h>`, `<stdnoreturn.h>`, the `__STDC_NO_...__` macros. |
| C17 | The same language as C11. |
| C23 | Attributes (`[[deprecated]]` and the other standard ones), `constexpr`, `auto` type inference, `typeof` and `typeof_unqual`, `nullptr`, `bool`, `true`, `false`, `alignas`, `alignof`, `static_assert` and `thread_local` as keywords, `_BitInt(N)` and the `wb` suffix, binary constants, digit separators, `#embed`, `__has_include`, `__has_embed`, `__has_c_attribute`, `#elifdef`, `#elifndef`, `#warning`, `__VA_OPT__`, `{}` initializers, enumerations with a fixed underlying type, labels before declarations, unnamed parameters in definitions, `u8''` constants, `static_assert` without a message, `<stdbit.h>`, `<stdckdint.h>`, `unreachable` and `nullptr_t` in `<stddef.h>`. No more trigraphs, K&R function definitions or `()` as an unprototyped declarator. |

Attributes are checked where the standard says (a `[[nodiscard]]` on an object is an
error). The standard ones, `deprecated`, `fallthrough`, `maybe_unused`, `nodiscard`,
`noreturn`, `unsequenced` and `reproducible`, are accepted; `deprecated`, `nodiscard`
and `noreturn` also give warnings (`C4050`, `C4051`, `C4053`). An attribute with a prefix
(`[[vendor::x]]`) is ignored; one without a prefix that is not a standard attribute is
ignored with the warning `C4052`.

`#pragma` lines are ignored in every mode, the standard `STDC` ones included, and so are
`_Pragma` operators.

### Predefined macros

`__STDC__` (1), `__STDC_VERSION__` (above), `__DATE__`, `__TIME__` (the build's time, or
the `SOURCE_DATE_EPOCH` environment variable's, in UTC), `__FILE__`, `__LINE__`. From C99,
`__STDC_HOSTED__` is 0. From C11, `__STDC_UTF_16__`, `__STDC_UTF_32__`,
`__STDC_NO_ATOMICS__` and `__STDC_NO_THREADS__` are 1 (`__STDC_NO_COMPLEX__` is not defined:
complex types are supported). In C23,
`__STDC_EMBED_NOT_FOUND__`, `__STDC_EMBED_FOUND__` and `__STDC_EMBED_EMPTY__` are 0, 1
and 2. `__STDC_IEC_559__`, `__STDC_IEC_60559_BFP__` and `__STDC_ISO_10646__` are not
defined, nor is `__STDC_IEC_559_COMPLEX__` (see "Known deviations").

The compiler's own macros say what the build chose:

| Macro | Value |
|---|---|
| `__AXIOM__` | 1 |
| `__AXIOM_INT_WIDTH__`, `__AXIOM_LONG_WIDTH__` | The widths of `int` and `long` in bits. |
| `__AXIOM_PTR_WIDTH__` | The width of a pointer: 32 or 64. |
| `__AXIOM_WCHAR_WIDTH__` | The width of `wchar_t`: 16 or 32. |
| `__AXIOM_WCHAR_SIGNED__` | 1 if `wchar_t` is signed, else 0. |
| `__CHAR_UNSIGNED__` | 1, defined only if plain `char` is unsigned. |

No macro names the target (`__linux__`, `_WIN32`, `__x86_64__` and the like are not
defined): pass `-D` for what a program needs to know.

### Include path

`#include "file"` looks next to the including file, then in the `-I` directories in the
order given, then in the compiler's own headers. `#include <file>` looks in the same
places, without the first. An absolute path is used as it is. `#include` nests to 200
files.

## Implementation-defined behavior

### Per target

The defaults, which `--c-char`, `--c-int` and `--c-long` change. Types of the same width
may be spelled differently from the platform's headers (`size_t` is `unsigned int` on
wasm32, `int64_t` is `long` on macOS): representation and calling convention are the
same.

| Target | `char` | `int` | `long` | `long long` | pointer | `size_t` | `ptrdiff_t` | `wchar_t` | `int64_t` |
|---|---|---|---|---|---|---|---|---|---|
| `x86_64` (Linux) | signed | 32 | 64 | 64 | 64 | `unsigned long` | `long` | `int` | `long` |
| `aarch64` (Linux) | unsigned | 32 | 64 | 64 | 64 | `unsigned long` | `long` | `unsigned int` | `long` |
| `aarch64-macos` | signed | 32 | 64 | 64 | 64 | `unsigned long` | `long` | `int` | `long` |
| `x86_64-windows` | signed | 32 | 32 | 64 | 64 | `unsigned long long` | `long long` | `unsigned short` | `long long` |
| `wasm32` | signed | 32 | 32 | 64 | 32 | `unsigned int` | `int` | `int` | `long long` |
| `wasm64` | signed | 32 | 64 | 64 | 64 | `unsigned long` | `long` | `int` | `long` |

`short` is 16 bits on every target. `intmax_t` is `int64_t`, and `intptr_t` is the signed
type of `size_t`'s width. With `--c-int=16` or `--c-int=64` the headers choose types by width:
`size_t` is the first of `int`, `long` and `long long` as wide as a pointer, and there is
no `int32_t` if neither `int` nor `long` is 32 bits wide. Changing `--c-int` or
`--c-long` changes the sizes in the program and in calls, but the platform's library keeps
its own: a program built with other widths cannot pass such values to it.

### On every target

| | |
|---|---|
| Integers | Two's complement, 8-bit bytes, little-endian. `short` 16 bits, `long long` 64. |
| Conversion to a signed type of a value out of range | Wraps modulo 2^N. |
| Signed overflow | Undefined. The code generated wraps, at every optimization level tried. |
| Right shift of a negative value | Arithmetic (sign-extending): `-8 >> 1` is `-4`. |
| Division | Truncates toward zero; `%` has the sign of the dividend. |
| Complex types | `float _Complex`, `double _Complex` and `long double _Complex` (C99 and later) are laid out as an array of two elements of the real type, the real part first: the size is twice the real type's, the alignment the real type's. There are no imaginary types. |
| Floating types | `float` and `double` are IEEE 754 binary32 and binary64. `long double` is `double`: the same size, alignment and `LDBL_*` values. `FLT_EVAL_METHOD` is 0, `FLT_ROUNDS` is 1 (to nearest); the rounding mode cannot be changed. |
| Sizes and alignment | A scalar's alignment is its size. A structure is laid out naturally: each member at the next multiple of its alignment, the structure aligned as its strictest member, its size rounded up to that. `#pragma pack` is ignored. |
| `max_align_t` | A structure of a `long long` and a `long double`: size 16, alignment 8. |
| Alignment specifiers | `_Alignas` takes any power of two up to 4096 (more is `C3071`). |
| Bit-fields | Allocated from the least significant bit of a storage unit of the declared type; one never straddles two units. A plain `int` bit-field is signed. A `char` bit-field has `char`'s signedness. An unnamed bit-field takes space and adds nothing to the alignment of the structure; one of width 0 moves the next member to the next unit. |
| Enumerations | The type is `unsigned int` if no enumerator is negative, else `int`, when all fit; the enumeration constants are `int`. In C23, if some enumerator does not fit `int`, the first of `long`, `long long` (`unsigned long`, `unsigned long long` for an unsigned type) that holds all of them, and then the constants have the enumerated type. Before C23 a value outside `int` is `C3013`. With a fixed underlying type (C23), that type. |
| `_BitInt(N)` | `BITINT_MAXWIDTH` is 64. An object is stored in the smallest of 8, 16, 32 or 64 bits that holds N, extended to it. |
| Null pointer | All bits zero. |
| Source and execution character sets | UTF-8. Narrow string literals are UTF-8 bytes (invalid UTF-8 in a literal is `C1024`). `L"..."` is UTF-16 on Windows (`wchar_t` is 16 bits) and UTF-32 elsewhere; `u"..."` is UTF-16 and `U"..."` UTF-32. |
| Multi-character constants | `'ab'` is `'a' * 256 + 'b'` as an `int` of the build's width (the first character is the most significant); a warning, `C4102`. A wide one has the value of its last character. More characters than the type holds is `C1011`. |
| `main` | `int main(void)` and `int main(int, char **)` only (`C3061`). Reaching the end of `main` returns 0, in every mode. |
| Inline functions | An inline definition (6.7.4p7) is compiled with internal linkage, for the unit's own calls: the external function is another unit's. |
| `volatile` | Accesses are marked volatile for the optimizer. |
| `restrict` | Accepted; the code generator does not use it. |

### Translation limits

| | |
|---|---|
| Nesting | 1000 levels of expressions, declarators and statements together (`C2002`). An expression chain such as `1+1+...+1` counts one level per operator; a block counts for more than one level. |
| `#include` | 200 files deep (`C1082`). |
| Macros | 400 levels of invocations nested in arguments (`C1061`); a single macro's replacement of at most 2,000,000 tokens (`C1062`). |
| `#if` | An expression nested too deeply is `C1037`. Arithmetic is on `intmax_t` and `uintmax_t` (64 bits). |
| Arrays and objects | At most 2^47 bytes (`C3009`). |
| Alignment | At most 4096. |
| Integer constants | Up to `unsigned long long`; a larger one is `C1016`. |

## Headers

The compiler provides the headers a freestanding implementation must have, in
`std/c/include/`. Each is available from the standard that introduced it; including it
in an earlier mode is an `#error`.

| Header | Modes | Notes |
|---|---|---|
| `<float.h>` | all | `FLT_`, `DBL_` and `LDBL_` macros. No `FLT_SNAN`, `DBL_SNAN`, `LDBL_SNAN`. |
| `<limits.h>` | all | `LLONG_` from C99; `*_WIDTH`, `BOOL_MAX` and `BITINT_MAXWIDTH` (64) in C23. |
| `<stdarg.h>` | all | `va_list`, `va_start`, `va_arg`, `va_copy`, `va_end`. In C23 `va_start` needs no second argument. |
| `<stddef.h>` | all | `size_t`, `ptrdiff_t`, `wchar_t`, `NULL`, `offsetof`; `max_align_t` from C11; `nullptr_t` and `unreachable` in C23. |
| `<iso646.h>` | C95 and later | |
| `<stdbool.h>` | C99 and later | In C23, `bool`, `true` and `false` are keywords and the header only defines `__bool_true_false_are_defined`. |
| `<stdint.h>` | C99 and later | No `int32_t` family when no type is 32 bits wide. |
| `<complex.h>` | C99 and later | `complex`, `I` and `_Complex_I` (a constant of type `float _Complex`), `CMPLX`, `CMPLXF` and `CMPLXL` (C11; constant expressions when the operands are), and the declarations of every function. `creal`, `cimag` and `conj` (and their `f` and `l` forms) are also macros the compiler expands in line; the others are a library's, which is not provided. No `imaginary` or `_Imaginary_I`. |
| `<stdalign.h>` | C11 and later | |
| `<stdnoreturn.h>` | C11 and later | |
| `<stdbit.h>` | C23 | The `stdc_` functions as `static inline` functions and type-generic macros. |
| `<stdckdint.h>` | C23 | `ckd_add`, `ckd_sub`, `ckd_mul`. |

Every other header, the C library's included, is `C1081`. C23 (4p7) also asks a
freestanding implementation for most of `<string.h>` and some functions of `<stdlib.h>`
(and `<fenv.h>` and `<math.h>` if it claims Annex F). These are a library's, and the
compiler does not provide them. The C23 headers also define `__STDC_VERSION_<NAME>_H__`
as the standard says.

## Not supported

- `_Imaginary` (the imaginary types, which are optional), `_Atomic` (`__STDC_NO_ATOMICS__`),
  decimal floating types and constants: each is `C3901` (a decimal constant is `C1019`).
- Threads (`__STDC_NO_THREADS__`; there is no `<threads.h>`). `thread_local` and
  `_Thread_local` are accepted and checked, and such an object has static storage
  duration, as it would in a program with one thread.
- `long double` wider than `double`.
- The library, and Annex F, `<fenv.h>` and `<math.h>`: exception flags and rounding modes.
- `#embed` parameters other than `limit`, `prefix`, `suffix` and `if_empty` (`C1084`).
- Extensions of any kind. These are all errors: `__attribute__`, `asm` and `__asm__`,
  `__declspec`, `__int128`, `__typeof__` (and `typeof` before C23), statement expressions,
  `?:` with an omitted middle operand, case ranges, computed `goto` and labels as values,
  `__builtin_*` functions other than those below, `#include_next`, `#ident`,
  `#import`, `#assert`.

## Reserved identifiers

The compiler's headers are written in C with a few names in the reserved namespace.
Programs should not use them.

| Name | |
|---|---|
| `__builtin_va_list`, `__builtin_va_start`, `__builtin_va_arg`, `__builtin_va_copy`, `__builtin_va_end` | What `<stdarg.h>` is made of. |
| `__builtin_offsetof` | `offsetof`. |
| `__builtin_unreachable` | `unreachable` (C23). |
| `__builtin_stdc_...` | The operations of `<stdbit.h>`. |
| `__builtin_ckd_add`, `__builtin_ckd_sub`, `__builtin_ckd_mul` | `<stdckdint.h>`. |
| `__builtin_inff`, `__builtin_nanf` | `<float.h>`. |
| `__builtin_complex`, `__builtin_creal`, `__builtin_cimag`, `__builtin_conj` | `<complex.h>`: `__builtin_complex(x, y)` is `x + iy` for `x` and `y` of one real floating type. |
| `__axiom_cmul_f32`, `__axiom_cdiv_f64`, ... | The compiler's own functions for complex multiplication and division (internal to the program). |
| `__axiom_int64` | A keyword for a 64-bit integer type (`unsigned __axiom_int64` too) in every mode, for `size_t` in headers before C99 and for programs that need one in C89. |
| `__AXIOM_...`, `__axiom_...` | Macros and members of the headers. |

## Programs

A C program's `main` is called by a start-up module of the runtime, which the build adds.
It gives `main` its arguments (`argv[argc]` is null) and uses its result as the exit
status. On Windows the start-up calls the C library's `exit` with `main`'s value, which
flushes the library's streams and runs its `atexit` functions; elsewhere the system's
own start-up ends the process (on WebAssembly, the host).

A program that defines `memcpy`, `memset` and `memmove` itself builds for every target.
Names that the runtime uses for its own functions (such as `ax_write`) cannot be
defined: `C3062` (reported by `build`).

## Calling conventions and interoperability

C functions use the target's C ABI, for structures and unions passed and returned by
value and for variadic functions as well, so that C compiled here calls the platform's C
library and is called by it:

| Target | ABI |
|---|---|
| `x86_64` | x86-64 System V. |
| `x86_64-windows` | Windows x64. |
| `aarch64` | AAPCS64. |
| `aarch64-macos` | Apple's variant of AAPCS64. |
| `wasm32`, `wasm64` | The WebAssembly C ABI. |

A complex value is passed and returned as the structure of its two parts would be, as
each ABI says (`float _Complex` in one SSE register and `double _Complex` in two on
x86-64 System V, a homogeneous aggregate in two floating-point registers on AArch64, by
value in an integer register or by reference on Windows, by reference on WebAssembly);
checked against the system's C compiler on `x86_64` and `aarch64` Linux.

`va_list` is the ABI's: an array of one structure on x86-64 System V, the AAPCS64
structure on AArch64 Linux, and `char *` elsewhere, so it can be passed to the library's
`v...` functions.

Known limits:

- On x86-64 System V, a variadic argument whose type is aligned to more than 16 bytes and
  that goes on the stack is not placed at an address aligned as its type is: that would
  need the caller's frame realigned, and the stack keeps 16 bytes' alignment.
- On macOS, a variadic argument of a type aligned to 16 bytes gets no padding before it,
  where the platform aligns its stack slot to 16.

Verified with a program that calls the C library's `ldiv` (a structure returned by
value), `snprintf` and `printf` (variadic, `double` and `long long`) on `x86_64` Linux;
the other targets are tested by the suite, not run by hand.

## Stack traces

With `--stack-traces=on` (the default) a trap or crash prints a stack trace. On native
targets the trace shows the C functions that were running, each caller with the line and
column of its call:

```
axiom: crash: segmentation fault (address 0x0)
stack trace (innermost first):
  at inner
  at middle (crash.c:3:28)
  at main (crash.c:4:25)
  at main (std/internal/runtime/c_start.ax:38:12)
```

`--stack-traces=off` prints the crash line only. At `-O2` and `-O3` a trace can be
shorter, or absent, when the optimizer has inlined the functions or moved the faulting
access, or when a function's last act was a call (`return f(x);`), which then leaves
its frame first: the trace lacks the caller; at `-O0` and `-O1` it is complete in the programs tried. C code has none of
Axiom's run-time checks: no bounds or overflow checks, so a bad pointer is a crash if it
is one at all. On WebAssembly C functions leave no call records: a trap (such as a
division by zero) reports `__axiom_c_main` and the start-up, and no C frames.

## Optimization

`-O0` (the default) to `-O3` are the compiler's: C goes through the same passes as Axiom
code (README of `cmd/axiom/`, "Optimization"). All the units of a program are one module,
so `-O2` and `-O3` inline across them. The levels do not change the behavior of a
program that has no undefined behavior; they differ in speed and size, and in how
complete a stack trace is after a crash.

## Known deviations

These are the places where the implementation does not follow the standard it claims, or
claims less than the standard allows. None is hidden: each is said above or here.

- The implementation is freestanding (`__STDC_HOSTED__` is 0), and misses the parts of
  the library that C23 4p7 asks a freestanding one for: most of `<string.h>`, some of
  `<stdlib.h>`.
- Annex F is not claimed. The floating types have IEC 60559 formats (`FLT_IS_IEC_60559`
  in `<float.h>`), but there are no exception flags or rounding modes; signaling NaNs are
  not guaranteed to stay signaling, so `<float.h>` has no `FLT_SNAN`.
- `long double` is `double`.
- Annex G is not claimed (`__STDC_IEC_559_COMPLEX__` is not defined): it needs Annex F,
  and before C23 the imaginary types, which are absent. Complex arithmetic follows it
  all the same where it can: a real operand of `+ - * /` with a complex one is not made
  complex (G.5: `2.0 * z` is `2.0 * creal(z) + 2.0 * cimag(z) * I`, and signed zeros of
  the other part are kept), and multiplication and division recover the infinities and
  zeros G.5.1 describes when the formulas give NaN + iNaN. Division uses Smith's method
  (no overflow forming the divisor's squared magnitude); it may differ in the last bit
  from other implementations' choices. `CX_LIMITED_RANGE` has no effect (the full
  algorithms are always used).
- `_Atomic`, decimal floating types and threads are absent, as the feature
  macros say.
- `thread_local` objects have static storage duration.
- `restrict` has no effect on the generated code; `#pragma` (the `STDC` ones included)
  is ignored, so `FP_CONTRACT`, `FENV_ACCESS` and `CX_LIMITED_RANGE` have no effect.
- The variadic-argument placement limits on x86-64 System V and macOS above.
- Signed overflow, which is undefined, wraps in the code the compiler generates today;
  a program must not rely on it.
- An object from `--emit=obj` of units with no `main` does not link into an Axiom
  program with `--link=` (above).
