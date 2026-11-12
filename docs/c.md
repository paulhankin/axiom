# Compiling C

`axiom build file.c` compiles C. The C front end reads C translation units and hands them
to the same optimizer, code generators and linkers that Axiom programs use, so a C program
is built for every target of the compiler (WebAssembly, x86-64 and AArch64 Linux, x86-64
Windows, AArch64 macOS) with no C compiler and no external linker, except to link to the
platform's C library on Linux (see "What a C program is here").

It is a hosted implementation, in C's terms: a C program links the compiler's own C
library ("The C library"), and `--c-freestanding` makes it a freestanding one, with the
freestanding headers and of the library only `<string.h>`'s functions that C23 asks of
one. It reads C89, C95, C99, C11, C17 and C23 (`--c-std=`; C17 by default), each mode
accepting its standard's language and diagnosing what a later standard added or its own
removed, and their GNU dialects (`--c-std=gnu89` ... `gnu23`), which add the widely used
GNU extensions ("GNU dialects", below). Under an ISO standard there are no extensions,
but the pragmas, which every mode has.

```
axiom build prog.c -o prog                       # wasm32: prog.wasm
axiom build prog.c --target=native -O2 -o prog   # this machine
axiom build a.c b.c --c-std=c99 -DDEBUG=1 -Iinclude -o prog
axiom check prog.c                               # diagnostics only
```

[`docs/c-diagnostics.md`](c-diagnostics.md) lists the diagnostics. How the front end is
built is in [`docs/c-frontend.md`](c-frontend.md). C units and Axiom modules build into
one program ("C and Axiom in one program", below). Axiom programs that link C code
compiled by the system's C compiler (`--link=file.c`) are a different feature, in
[`docs/cli.md`](cli.md).

## What a C program is here

- **The library is the compiler's, as far as it goes.** The headers and functions of
  "The C library" (below) are linked into every program that uses them, on every target.
  There is no `<math.h>`, `<fenv.h>` or `<threads.h>` yet: `#include <math.h>` is the
  error `C1081`.
- **Other undefined functions are the platform's.** A function a program declares and
  that neither it nor the library defines is looked up outside the program:
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
- **A program may define the library's functions.** `memcpy`, `strlen` and the others of
  `<string.h>` among them: a function the program defines is not taken from the library,
  whose functions then call the program's, and the runtime's own copies of `memcpy`,
  `memmove` and `memset` (on AArch64 Linux, which needs them) are weak.
- **Calls cross the boundary correctly.** Arguments and results follow the target's C
  ABI, so C compiled here calls the platform's library and is called from it ("Calling
  conventions", below).

## The command line

```
axiom build file.c... [options] [-o out]
axiom check file.c...  [options]
axiom build main.ax file.c... [options]       an Axiom program and C units
axiom build main.c... file.ax... [options]    a C program and Axiom modules
```

An input is a C source if its name ends in `.c`. Options take the forms `--name=value`
and `-DNAME`; they may come before or after the inputs. `-D NAME`, `-U NAME` and
`-I dir` (with a space) are accepted too. The first input decides what the program is:
a C program if it is a C source, an Axiom program if it is an Axiom source ("C and
Axiom in one program").

### C options

| Option | |
|---|---|
| `--c-std=<std>` | The standard (see "Standards"; default `c17`), or a GNU dialect (`gnu89` ... `gnu23`: "GNU dialects"). |
| `--c-char=signed\|unsigned` | Whether plain `char` is signed (default: the target's C ABI). |
| `--c-int=16\|32\|64` | The width of `int` (default 32). |
| `--c-long=32\|64` | The width of `long` (default: the target's C ABI; at least `--c-int`). |
| `--c-long-double=64\|80\|128` | The format of `long double` (default: the target's C ABI; see "Floating types"). `80` is for `x86_64` (Linux) only; `128` for `x86_64` and `aarch64` (Linux) and WebAssembly. |
| `-DNAME`, `-DNAME=value` | Define a macro (`NAME` is `NAME=1`). `-D'F(x)=x+1'` defines a function-like macro. |
| `-UNAME` | Undefine a macro. The predefined macros of the standard cannot be undefined (`C1053`). |
| `--c-warn-unknown-pragmas` | An unknown `#pragma` is the warning `C4111` (it is ignored either way). |
| `-Idir` | Add a directory to the include path. |
| `--c-freestanding` | A freestanding program: `__STDC_HOSTED__` is 0, only the freestanding headers are found (another is `C1081`), and no C library is linked ("The C library"). |

`-D`, `-U` and `-I` apply in the order given. The C options are errors for an Axiom
source. The unknown ones are errors too: there are no `-W...`, `-std=`, `-c` or `-f...`
options; a warning is silenced only by `#pragma GCC diagnostic` ("Pragmas").

### Options of `axiom build` that apply

`--target`, `--cpu`, `--features`, `-O0`..`-O3` (see "Optimization"), `--stack-traces`
(see "Stack traces"), `--format=agent-json`, `--std=<dir>` (the standard library's
directory, where the compiler finds the C headers in `c/include/`), `-o`, `--emit`,
`--link`, `-l`, `-L`, `--subsystem`, `--icon`, `--pdb`, and `-g` /
`--debug-info=` (see "Debugging"). The default
target is `wasm32`. `--link=file.c` links a further C source, compiled by the system's C
compiler, as for Axiom programs; `-l` and `-L` pass libraries and directories to that
linker.

These are errors with C sources, since they mean something only for Axiom code:
`--overflow`, `--sanitize`, `--coverage`, `--units`, `--emit-header`. An Axiom program
with C units takes `--overflow` and `--emit-header`, which apply to its Axiom code (and
the header is written before the C units are compiled, so they may include it). So are
`axiom test`, `axiom profile`, `axiom ide` and `axiom fmt` on a C source, and `axiom
test` and `axiom profile` of an Axiom program with C units.

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
with `--target=x86_64`). Give the C sources to the build of the Axiom program instead
("C and Axiom in one program").

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

## C and Axiom in one program

C translation units and Axiom modules build into one program, on every target, with no
C compiler or linker of the system's (unless the program calls the system's C library on
Linux, as any C program may: "What a C program is here"):

```
axiom build main.ax util.c parse.c -o prog       # an Axiom program; its main is Axiom's
axiom build main.c rt.ax io.ax -o prog           # a C program; its main is C's
axiom build pkg/                                  # a package with c-sources (below)
```

- **What the program is** is decided by the first input. If it is an Axiom source, it is
  the program's root module, as in any Axiom build (its `export fn main` is the entry),
  and the other inputs must be C sources: the program's C units. A C unit that defines
  `main` then is `C3066`. If the first input is a C source, the program is a C program:
  one of its units defines `main` (`C3063`), and the Axiom sources among the inputs (or
  given with `--link=file.ax`) are modules of the program, beside the C start-up module
  (`std/internal/runtime/c_start.ax`), with their imports. An Axiom module that exports
  `main` then is `E0802`.
- **A package** names a directory of C sources in its manifest: `c-sources = "csrc"`
  under `[package]`. Every `.c` file in it, at any depth and in the order of their paths,
  is a C unit of the program, as if given on the command line; so are those of the
  packages it depends on. `#include "..."` finds headers next to the including file and
  in the `-I` directories.
- **Axiom calls C** through its foreign declarations (spec §14.5.1): a function in an
  `extern "C"` block that a C unit defines is that definition. Its arguments and result
  are passed as the target's C ABI says, so the types at the boundary (spec §14.5.3)
  mean the same on both sides: `i32` is `int`, `*u8` is `const char *`, a `fn(i32) i32`
  is an `int (*)(int)`, and so on. Variadic C functions take Axiom's variadic calls.
- **C calls Axiom**: an `export fn` of any of the program's modules is a C function of
  the same name, which a C declaration (a prototype, `--emit-header`'s or one's own)
  calls. An Axiom function passed to C as a function pointer is called with C's calling
  convention, and a C function pointer passed to Axiom is called with an Axiom call.
- **Structures** cross by pointer: an Axiom `struct` is laid out as C lays out the
  structure with the same members (spec §5.9), so a `*Point` is a `struct point *`.
  Passing one by value between Axiom and C is not supported (spec §14.5.3, `E0801`);
  between C units it is, with the C ABI.
- **Data** is shared through pointers: the Axiom language has no declarations of
  external objects, so C's objects are reached through C functions that return their
  addresses, and Axiom's through exported functions that return theirs.
- **Names**: an external name is one symbol for the whole program, C's and Axiom's. On
  macOS the object files add the leading underscore of Mach-O names to every one of
  them; on WebAssembly a function that the program defines is a function of the module
  and is called directly, and only one that nothing defines is an import (of `env`,
  "What a C program is here"). An Axiom `export fn` is also an export of the WebAssembly
  module, as in any Axiom program.
- **Conflicts**: a name defined by two C units is `C3064`; one defined by a C unit and
  by an Axiom `export fn` is `C3065`; a name of the runtime's own functions defined by C
  is `C3062`. A name that nothing defines is the platform's (above): on Linux, a link
  error if the C library does not have it either (`E9004`, after `link error: undefined
  symbol 'name'`).

Weak definitions, which standard C does not have, behave as a linker treats them: a
weak definition gives way to another unit's definition that is not weak, and of two weak
ones the first unit's is kept. The runtime's own copies of `memcpy`, `memmove` and
`memset` are weak, so that a C unit may define them. Tentative definitions (6.9.2) are
definitions: two units' tentative definitions of one name are `C3064`, as two
definitions are (6.9p5), and there are no "common" objects.

### What is linked

An executable holds what its `main` (and the Axiom program's exports) can reach, and
nothing else the C units define: a function that nothing calls or takes the address of,
an object that nothing refers to, and a string literal only they use are left out, at
every optimization level, before the optimizer runs. In a GNU dialect, what has the
attribute `used`, and an object placed in a section (`section`), are kept too, and so
are the constructors and destructors, which the start-up calls. So a C library given in full (a
unit of 200 functions, say) adds nothing to a program that calls none of its functions,
and the size of one that calls one function is that of the program with that function
alone. An object (`--emit=obj`) and a program that the system's linker links (with
`--link=` inputs or `-l` libraries, whose code may refer to anything) keep every
definition.

### Start-up and exit

The order in which a program starts and ends is the same with or without C:

1. The system starts the runtime's entry point (on WebAssembly, the host calls the
   module's `main` export). It records the arguments, checks the processor when the
   program is built for more than its target's baseline (`--cpu`), and starts the
   runtime (`ax_rt_init`: the crash handlers; the allocator and the threads start on
   their first use). Axiom's global variables have their initial values in the
   executable's data, before any code runs.
2. The program's `main` runs: an Axiom program's, or for a C program the start-up
   module's, which makes C strings of the arguments (with the Axiom allocator) and calls
   C's `main` with them. Everything of the runtime works from C's first instruction on:
   C code may call Axiom functions that allocate, print or start threads.
3. When C's `main` returns, its value is the argument of a call of `exit` (5.1.2.2.3)
   if the program defines `exit` (the C library's, unless `--c-freestanding`; or the
   program's own: a C unit, or an Axiom module's `export fn exit`), and on Windows,
   where `exit` is otherwise the system C library's (`msvcrt.dll`), which runs its
   `atexit` functions and flushes its streams. The library's `exit` runs the `atexit`
   functions, then the GNU dialects' destructors, then the standard I/O's flushing, and ends the
   process through the runtime: `std/os.exit`, which finishes the runtime
   (`ax_rt_fini`) and exits with the status. Its argument has the width of `int`
   (`--c-int`).
4. Otherwise `main`'s value goes back to the runtime's entry point, which finishes the
   runtime and returns it to the system (on Linux with the C library linked, to the C
   library's start-up, which calls its `exit`).

A C call of `exit` anywhere is the same function as in step 3. A freestanding C program
on Linux that calls the system C library's `exit` (it defines none) ends through the C
library, without the runtime's finishing step, which only writes a profile (`axiom
profile` does not take C programs). Under WebAssembly a hosted program ends by calling
the host's `ax_exit` (`std/internal/runtime/host.mjs`: `process.exit`, or the
embedder's `exit` option) rather than by returning from the module's `main`.

## The C library

A hosted C program links the compiler's own C library, which is compiled from the
standard library's sources with the program, unless `--c-freestanding` is given (then
only `<string.h>`'s functions that C23 asks of a freestanding implementation). It is in
parts, each with its sources and its private header:

| Part | Headers | Sources |
|---|---|---|
| core | `<assert.h>`, `<ctype.h>`, `<errno.h>`, `<setjmp.h>`, `<signal.h>`, `<stdlib.h>` (but its multibyte functions), `<string.h>`'s hosted functions | `std/c/lib/core/`, `std/c/rt/core*.ax`, `axc/core_impl.h` |
| strings | `<string.h>` (C23's freestanding functions) | `std/c/lib/string/` |
| standard input/output | `<stdio.h>`, `<wchar.h>`'s input/output | `std/c/lib/stdio/`, `std/c/rt/stdio.ax` |
| text | `<locale.h>`, `<wchar.h>`, `<wctype.h>`, `<uchar.h>`, `<time.h>`, `<inttypes.h>`, `<stdlib.h>`'s multibyte functions | `std/c/lib/text/`, `std/c/rt/text*.ax`, `axc/text.h`, `axc/locale.h` ([`docs/c-text.md`](c-text.md)) |

There is no `<math.h>`, `<fenv.h>` or `<threads.h>` yet (`__STDC_NO_THREADS__`).

### How the library is linked

- The library's C units (`<std>/c/lib/**.c`) are compiled on demand: a unit is checked
  and compiled when it defines a name that the program uses (its C units', its Axiom
  modules' `extern "C"` declarations, the start-up's `exit`, the memory functions the
  code generators call), and so on for the names that unit uses. The index
  `<std>/c/lib/index` says which file defines each external name; it is written by
  `cmd/internal/tools/gen_c_lib_index.mjs` (from `--emit=c-lib-defs`, the names a unit
  defines) and the suite checks that it is current. Each external name `X` is defined in
  a file `X.c` of its own where it can be; the standard I/O's larger files define several.
- The run-time modules (`<std>/c/rt/*.ax`: the system's services, from the Axiom standard
  library, under C names) are modules of the program when one of their exports is used
  (with the C functions they call, which the index lists); their `export fn`s are
  generated only when something reaches them. A program whose `main` is Axiom's has all
  of them. `std/c/rt/ldouble.ax` (the arithmetic of a `long double` wider than `double`)
  is every such program's, freestanding or not.
- A name that one of the program's own units, or one of its Axiom modules (an `export
  fn`), defines is the program's, and the library's definition of it is left out (as a
  linker leaves out an archive's member nothing needs), so a program may define a library
  function itself.
- The library's units are C17 whatever the program's `--c-std` (its headers serve every
  standard), without extensions, with the program's `--c-int`, `--c-long`, `--c-char`
  and `--c-long-double`, and without its `-D`, `-U` and `-I`. Behaviours that C23
  changed are chosen per call: in C23 mode the headers map `strtol` and its kin,
  `wcstol`'s, `strtoimax`'s and `fscanf`'s families to `__axc_..._c23` functions, which
  take the `0b` prefix. Names the parts share begin with `__axc_`; the types several
  headers declare (`size_t`, `wchar_t`, `wint_t`, `mbstate_t`) are defined once, from
  the compiler's `__AXIOM_*_TYPE__` macros, by `axc/types.h`. The functions of
  `<std>/c/rt/` take and return no `int`, whose width the build chooses (64-bit
  integers cross instead).
- What a program does not reach is not in its executable ("What is linked"). A program
  that returns 3 from `main` builds within about 20 ms of a freestanding one (x86-64
  Linux) and is a few KB larger (the exit path).
- `exit` runs the `atexit` functions (last registered first), then the GNU dialects'
  destructors (highest priority first), then flushes and closes the streams and removes
  `tmpfile`'s files; the constructors run before `main` (lowest priority first), also
  in a program whose `main` is Axiom's, where the destructors run when it returns.

Under Linux a program that still calls a function of the system's C library (one of the
headers this library does not have yet: `<math.h>`, such as `sin`) is linked with it, as before,
and the library's functions (`exit`, `qsort`, ...) are the ones the program uses. Its
`malloc` and the others of `<stdlib.h>`'s memory management then use the system
library's heap (`__libc_malloc` and its kin), as they always do on macOS
(`libSystem`'s), so that memory one library allocates the other frees; in a program that
needs nothing of the system's (a static one on Linux; Windows; WebAssembly) they use the
Axiom allocator. A C source linked with `--link=` (compiled by the system's C compiler,
against its headers) uses the system library's streams, and the program's code this
library's, under the same names (`printf`, `fopen`, ...): such a program should be built
with `--c-freestanding` if both write. On
Windows nothing the library defines comes from `msvcrt.dll`; the rest still does by
name (the functions of `<math.h>`). On macOS the runtime reaches
`libSystem`'s `exit`, `getenv`, `raise`, `malloc`, `calloc` and `free` under names of
its own (`__axiom_libc_...`, which the linker binds to those), so that they are never
the program's.

The library's core (`std/c/lib/core/`, `std/c/rt/core*.ax`):

| Header | What is there |
|---|---|
| `<assert.h>` | `assert` (C23: variadic), `static_assert` (C11, C17). A failed assertion writes `file:line: function: assertion failed: expression` to standard error and calls `abort`. |
| `<ctype.h>` | All of it, from one table: the "C" locale's classes in every locale (the bytes past 127 are in no class); every `int` is safe (EOF, `unsigned char` values, and others: no class). |
| `<errno.h>` | `errno`, each thread's own (a macro over the function `__axc_errno`, whose object is `_Thread_local`), `EDOM`, `ERANGE`, `EILSEQ` and the codes the library reports (`ENOMEM`, `EINVAL`, ...): the library's own values, the same on every target. |
| `<setjmp.h>` | `jmp_buf`, `setjmp`, `longjmp` ("setjmp and longjmp", below). |
| `<signal.h>` | `signal`, `raise`, `sig_atomic_t`, the six signals ("Signals", below). |
| `<stdlib.h>` | All of it (the multibyte functions and `MB_CUR_MAX` are the text part's). Its header comment lists the choices: `malloc(0)` and the like give a unique pointer; memory is the Axiom allocator's (`std/mem`, thread-safe) or the system C library's (above), aligned to 16, `aligned_alloc` to any power of two; `calloc` checks its product; `rand` is a 64-bit linear congruential generator (`RAND_MAX` 2147483647); `qsort` is an introspective sort (O(n log n) at worst, O(log n) stack); `strtod`, `strtof` and `strtold` (in `long double`'s format, whichever the target has) are correctly rounded for every input, decimal and hexadecimal, and set `ERANGE` on overflow and on a zero or subnormal result; in C23 mode `strtol` and its kin take `0b`; `atexit` and `at_quick_exit` take any number of functions; `getenv`'s strings stay valid until the variable's value is asked for again after it changed; `system` uses `/bin/sh` (Linux, macOS) or `cmd.exe` (Windows); under WebAssembly there is no command processor; `strfromd`, `strfromf` and `strfroml` (C23) format with `snprintf`, the standard I/O's. |
| `<string.h>` | The hosted functions: `strerror` (the library's messages), `strcoll` and `strxfrm` (the order of the bytes, in every locale), and in C23 `strdup` and `strndup`. |

Measured on x86-64 Linux against the system's C library (both at their best options):
`qsort` of integers and of 24-byte records 1.1 times its time, `strtod` 1.0 times,
`malloc` and `free` of small blocks 0.96 times.

### Wide characters, locales and time

See [`docs/c-text.md`](c-text.md). `__STDC_ISO_10646__` is `202309L` (Unicode 15.1)
where `wchar_t` is 32 bits; with Windows' 16-bit `wchar_t` (UTF-16) it is not defined.

### `long double`

`strtold`, `wcstold`, `strfroml`, and `fprintf`'s and `fscanf`'s `L` conversions are
exact (correctly rounded) in the format `long double` has: binary64, the x87 80-bit
format or binary128 (`LDBL_MANT_DIG` 53, 64 or 113, "Floating types"), read from and
written to the value's bits by the standard I/O's engines (`fmtfloat.c`, `scanfloat.c`).

### Standard input/output: `<stdio.h>`

The library's `<stdio.h>` (`std/c/lib/stdio/`, over `std/c/rt/stdio.ax`) is all of
C17's 7.21 with C23's additions, and the wide-character input/output of `<wchar.h>`
(7.29.2, 7.29.3), on every target. Its implementation-defined choices:

- **Streams**: `stdin` and `stdout` are fully buffered unless they are a terminal, then
  line buffered (decided at their first use); `stderr` is unbuffered; a file `fopen`
  opens is line buffered when it is a terminal. `BUFSIZ` is 4096, `FOPEN_MAX` 20 (no
  limit but memory), `FILENAME_MAX` 4096, `L_tmpnam` 260, `TMP_MAX` 2147483647. A
  line-buffered stream writes up to its last new-line; reading from an unbuffered or
  line-buffered stream first writes the line-buffered output streams (7.21.3p3). A
  formatted output to an unbuffered stream is one write.
- **Files**: every mode of 7.21.5.3, `x` (C11) included; another character ends the
  mode. Text streams translate `'\n'` to and from `"\r\n"` on Windows (a lone `'\r'`
  is itself); elsewhere text and binary streams are the same. Positions are 64-bit
  (`fgetpos`, `fsetpos`, and `fseek`/`ftell` as far as `long` goes: on Windows and
  wasm32, where `long` has 32 bits, `ftell` fails with `EOVERFLOW` past 2 GB).
  `tmpfile` creates a file in the directory for temporary files (`TMPDIR`, else `/tmp`;
  on Windows `TMP`, `TEMP`, ...) and removes it at once (on Windows when it is closed
  or the program exits); `tmpnam` gives names there that no file has. `remove`
  removes an empty directory too. On WebAssembly the files are the node host's.
- **Pushback**: `ungetc` takes up to 8 bytes, `ungetwc` a wide character as its
  multibyte bytes (one at least). Output after input, and input after output, need no
  `fflush` or seek between them (7.21.5.3p7 asks for one); `fflush` of an input stream
  gives its unread bytes back to the file.
- **Orientation**: a byte function on a wide-oriented stream fails (returns `EOF` or a
  negative value, writing and reading nothing), and a wide one on a byte-oriented
  stream fails (`WEOF`).
- **Errors**: the error indicator is set by a failed read or write, and by reading a
  stream opened only for writing or writing one opened only for reading (with `errno`
  `EBADF`); failures of the system set `errno` to `ENOENT`, `EACCES`, `EEXIST`,
  `ENOTDIR`, `EISDIR`, `EINVAL`, `ESPIPE` or `EIO`.
- **Threads**: each stream has a lock, which every function takes for its whole
  operation, from the first thread a program starts on (until then there is one
  thread, and nothing to lock).
- **`fprintf`**: `%p` is `0x` and the address in hexadecimal digits (`0x0` for a null
  pointer), as `%#x` would write it; infinity is `inf`, NaN `nan` or `-nan` (no
  n-char-sequence: `_PRINTF_NAN_LEN_MAX` is 0); `%a` writes a normal value with the
  leading digit 1 and a subnormal with 0 and the smallest normal's exponent, in every
  floating format; a conversion the standard does not have makes the call fail. The
  decimal conversions are exact and correctly rounded at any precision, halfway cases
  to even; the current rounding direction is not consulted. C23's `%b` and `%B` and the
  `wN` and `wfN` length modifiers are accepted in every mode.
- **`fscanf`**: an input item is the longest prefix of a matching sequence, and only
  the character after it is given back (7.21.6.2p9), so a prefix that is not a whole
  number is a matching failure: `"0x"` for `%x`, `"1e+"` and `"100ergs"` (the example of
  7.21.6.2p20) for `%f`, and `%3c` when the input ends after two characters. The
  scanset `%[a-z]` is a range when `-` is neither first nor last. `%p` reads what `%p`
  writes. Floating conversions are correctly rounded for every format; `%i` reads C23's
  `0b` prefix in C23 mode (the header maps the functions to `__axc_..._c23` forms) and
  not before, and `%b` reads binary digits after an optional `0b`.
- **`perror`** writes `strerror`'s message. The streams are flushed at exit, and the
  files `tmpfile` created removed.

### setjmp and longjmp

`setjmp` is a macro for `__axc_setjmp`, which the linkers provide on every native target
(`cmd/axiom/src/jstub.ax`), with `longjmp`: they save and restore the registers the
platform's calling convention keeps across a call (with the floating-point ones:
`xmm6`-`xmm15`, MXCSR and the x87 control word on Windows, `d8`-`d15` on AArch64), the
stack pointer and the return address; `jmp_buf` is 32 words. The compiler knows that
`__axc_setjmp` returns twice: a function that calls it is not optimized (its objects
stay in memory, each in a slot of its own, and no value lives in a register across the
call), not inlined, and makes no sibling calls, at every level, so that the objects not
modified between `setjmp` and `longjmp` have their values after the jump (C17
7.13.2.1p3), as do volatile ones that were. Its address cannot be taken (`C3301`): a
call through a pointer would not be known to return twice.

- `longjmp` unwinds nothing. On Windows no exception handlers of the frames it leaves
  run (the system's unwinding is not used); the restored stack pointer drops the
  frames.
- Jumping back into a function whose variable length arrays were allocated after the
  `setjmp` call frees them (the stack pointer goes back); jumping out of, or over,
  Axiom functions that C called (an Axiom function on the stack between the `setjmp`
  and the `longjmp`) is undefined: their `defer`s do not run.
- WebAssembly cannot return to a frame without its exception handling, which the code
  generator does not use: `setjmp` there is `C3300`.
- `setjmp` saves no signal mask: a signal handler that leaves with `longjmp` leaves
  its signal blocked (on Linux and macOS).

### Signals

The six signals of `<signal.h>` have Linux's numbers on every target. A handler set
with `signal` stays set while it runs, and its signal waits until it returns
(7.14.1.1p3's second choice).

- Linux and macOS: the system delivers them (`rt_sigaction` through the runtime's
  system call stub, or `libSystem`'s `sigaction`), on the runtime's signal stack where
  the thread has one; `raise` sends the signal to the thread, and the system's
  default actions apply (killed by the signal). A handler of `SIGSEGV`, `SIGFPE` or
  `SIGILL` replaces the runtime's crash report while it is set; `SIG_DFL` puts back
  the action the program started with (the crash report).
- Windows: `SIGINT` is the console's Ctrl-C and Ctrl-Break (a console control
  handler, which the system calls on a thread of its own); the others arrive only from
  `raise`, which calls the handler at once; the processor's faults stay the crash
  reporter's. The default action of a raised signal ends the process with status 3
  for `SIGABRT` (as `abort`'s) and 128 + the signal's number for the others.
- WebAssembly: `raise` alone, as on Windows.

`abort` raises `SIGABRT`; if that returns (a handler returned, or the signal is
ignored), it sets the default action and raises it again, and ends the process with
status 3 if it still lives.

## Standards

| `--c-std=` | Standard |
|---|---|
| `c89`, `c90`, `iso9899:1990` | ISO/IEC 9899:1990 |
| `c95`, `iso9899:199409` | C90 with Amendment 1 |
| `c99`, `iso9899:1999` | ISO/IEC 9899:1999 |
| `c11`, `iso9899:2011` | ISO/IEC 9899:2011 |
| `c17`, `c18`, `iso9899:2017`, `iso9899:2018` | ISO/IEC 9899:2018 (the default) |
| `c23`, `iso9899:2024` | ISO/IEC 9899:2024 |
| `gnu89`, `gnu90`, `gnu99`, `gnu11`, `gnu17`, `gnu18`, `gnu23` | The standard of the number (C89, C99, C11, C17, C23) and the GNU extensions ("GNU dialects") |

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
| C11 | `_Generic`, `_Static_assert`, `_Noreturn`, `_Alignas`, `_Alignof`, `_Atomic`, `_Thread_local`, anonymous structures and unions, `u""`, `U""` and `u8""` literals and `u''` and `U''` constants, `<stdalign.h>`, `<stdatomic.h>`, `<stdnoreturn.h>`, the `__STDC_NO_...__` macros. |
| C17 | The same language as C11. |
| C23 | Attributes (`[[deprecated]]` and the other standard ones), `constexpr`, `auto` type inference, `typeof` and `typeof_unqual`, `nullptr`, `bool`, `true`, `false`, `alignas`, `alignof`, `static_assert` and `thread_local` as keywords, `_BitInt(N)` and the `wb` suffix, binary constants, digit separators, `#embed`, `__has_include`, `__has_embed`, `__has_c_attribute`, `#elifdef`, `#elifndef`, `#warning`, `__VA_OPT__`, `{}` initializers, enumerations with a fixed underlying type, labels before declarations, unnamed parameters in definitions, `u8''` constants, `static_assert` without a message, `<stdbit.h>`, `<stdckdint.h>`, `unreachable` and `nullptr_t` in `<stddef.h>`. No more trigraphs, K&R function definitions or `()` as an unprototyped declarator. |

Attributes are checked where the standard says (a `[[nodiscard]]` on an object is an
error). The standard ones, `deprecated`, `fallthrough`, `maybe_unused`, `nodiscard`,
`noreturn`, `unsequenced` and `reproducible`, are accepted; `deprecated`, `nodiscard`
and `noreturn` also give warnings (`C4050`, `C4051`, `C4053`). An attribute with a prefix
(`[[vendor::x]]`) is ignored; one without a prefix that is not a standard attribute is
ignored with the warning `C4052`.

`#pragma` lines and `_Pragma` operators are read in every mode ("Pragmas"): those this
implementation knows act, the others are ignored.

### Predefined macros

`__STDC__` (1), `__STDC_VERSION__` (above), `__DATE__`, `__TIME__` (the build's time, or
the `SOURCE_DATE_EPOCH` environment variable's, in UTC), `__FILE__`, `__LINE__`. From C99,
`__STDC_HOSTED__` is 1, or 0 with `--c-freestanding`. From C11, `__STDC_UTF_16__`,
`__STDC_UTF_32__` and `__STDC_NO_THREADS__` are 1 (there is no `<threads.h>` yet);
`__STDC_NO_ATOMICS__` and `__STDC_NO_COMPLEX__` are not defined: atomics (see "Atomics and
threads") and complex types are supported. In C23,
`__STDC_EMBED_NOT_FOUND__`, `__STDC_EMBED_FOUND__` and `__STDC_EMBED_EMPTY__` are 0, 1
and 2. `__STDC_IEC_559__`, `__STDC_IEC_60559_BFP__` and `__STDC_ISO_10646__` are not
defined, nor is `__STDC_IEC_559_COMPLEX__` (see "Known deviations").

The compiler's own macros say what the build chose:

| Macro | Value |
|---|---|
| `__AXIOM__` | 1 |
| `__AXIOM_INT_WIDTH__`, `__AXIOM_LONG_WIDTH__` | The widths of `int` and `long` in bits. |
| `__AXIOM_LDBL_WIDTH__` | The format of `long double`: 64 (binary64), 80 (the x87's) or 128 (binary128). |
| `__AXIOM_PTR_WIDTH__` | The width of a pointer: 32 or 64. |
| `__AXIOM_WCHAR_WIDTH__` | The width of `wchar_t`: 16 or 32. |
| `__AXIOM_WCHAR_SIGNED__` | 1 if `wchar_t` is signed, else 0. |
| `__AXIOM_<T>_TYPE__`, `__AXIOM_<T>_MIN__`, `__AXIOM_<T>_MAX__`, `__AXIOM_<T>_WIDTH__`, `__AXIOM_<T>_C__(c)` | For `<T>` one of `SIZE`, `PTRDIFF`, `INT64`, `UINT64`, `INTMAX`, `UINTMAX`, `INT_FAST16`, `UINT_FAST16`, `INT_FAST32`, `UINT_FAST32`: the type the platform gives that typedef (`__AXIOM_SIZE_TYPE__` is `unsigned long`, ...), its limits, width and constant macro, which the headers use. `__AXIOM_WINT_TYPE__`, `_MIN__`, `_MAX__` and `_WIDTH__` are `wint_t`'s; `__AXIOM_MAX_ALIGN__` is `max_align_t`'s alignment. |
| `__CHAR_UNSIGNED__` | 1, defined only if plain `char` is unsigned. |
| `__AXIOM_GNU__` | 1, defined only in a GNU dialect. |
| `__AXIOM_ASM_X86_64__`, `__AXIOM_ASM_AARCH64__` | 1, defined only in a GNU dialect on x86-64 or AArch64 targets: the language of inline assembly's templates ("Inline assembly"). |

No macro names the target (`__linux__`, `_WIN32`, `__x86_64__` and the like are not
defined): pass `-D` for what a program needs to know.

### Include path

`#include "file"` looks next to the including file, then in the `-I` directories in the
order given, then in the compiler's own headers. `#include <file>` looks in the same
places, without the first. An absolute path is used as it is. `#include` nests to 200
files.

## Implementation-defined behavior

### Per target

The defaults, which `--c-char`, `--c-int`, `--c-long` and `--c-long-double` change. The
typedefs of `<stddef.h>` and `<stdint.h>` are the types the platform's C ABI and C library
give them, so that a declaration that uses them is the platform's (the same type, not only
the same width):

| Target | `char` | `int` | `long` | `long long` | pointer | `size_t` | `ptrdiff_t` | `wchar_t` | `int64_t` |
|---|---|---|---|---|---|---|---|---|---|
| `x86_64` (Linux) | signed | 32 | 64 | 64 | 64 | `unsigned long` | `long` | `int` | `long` |
| `aarch64` (Linux) | unsigned | 32 | 64 | 64 | 64 | `unsigned long` | `long` | `unsigned int` | `long` |
| `aarch64-macos` | signed | 32 | 64 | 64 | 64 | `unsigned long` | `long` | `int` | `long long` |
| `x86_64-windows` | signed | 32 | 32 | 64 | 64 | `unsigned long long` | `long long` | `unsigned short` | `long long` |
| `wasm32` | signed | 32 | 32 | 64 | 32 | `unsigned long` | `long` | `int` | `long long` |
| `wasm64` | signed | 32 | 64 | 64 | 64 | `unsigned long` | `long` | `int` | `long long` |

| Target | `intmax_t` | `int_fast16_t` | `int_fast32_t` | `wint_t` (its limits) | `max_align_t` |
|---|---|---|---|---|---|
| `x86_64`, `aarch64` (Linux) | `long` | `long` | `long` | `unsigned int` | aligned to 16 |
| `aarch64-macos` | `long` | `short` | `int` | `int` | `long double` (8) |
| `x86_64-windows` | `long long` | `int` | `int` | `unsigned short` | `long double` (8) |
| `wasm32`, `wasm64` | `long long` | `int` | `int` | `int` | aligned to 16 |

`short` is 16 bits on every target; `intptr_t` and `uintptr_t` are `ptrdiff_t` and
`size_t`, `int_fast8_t` is `signed char` and `int_fast64_t` is `int64_t`. The headers learn
these types from the compiler's macros (`__AXIOM_SIZE_TYPE__`, `__AXIOM_INT64_TYPE__`,
... with their limits and constant macros). With `--c-int` or `--c-long` the platform's
type is kept when it still has the width its role needs; otherwise `size_t` is the first of
`int`, `long` and `long long` as wide as a pointer, `int64_t` and `intmax_t` the first of
64 bits, the fast types the narrowest wide enough, and there is no `int32_t` if neither
`int` nor `long` is 32 bits wide. Changing `--c-int` or `--c-long` changes the sizes in the
program and in calls, but the platform's library keeps its own: a program built with other
widths cannot pass such values to it.

`long double` is the format of the target's C ABI:

| Target | `long double` | Size, alignment | `LDBL_MANT_DIG` |
|---|---|---|---|
| `x86_64` (Linux) | the x87's 80-bit extended format | 16, 16 (10 bytes of value, 6 of padding) | 64 |
| `aarch64` (Linux), `wasm32`, `wasm64` | IEEE 754 binary128 | 16, 16 | 113 |
| `aarch64-macos`, `x86_64-windows` | binary64 (`double`'s) | 8, 8 | 53 |

The WebAssembly C ABI makes `long double` binary128 too; this implementation follows it, so
that the type has the size, alignment and range that C code built for WebAssembly
expects. `--c-long-double=64` makes it `double` (as on the others), `=128` binary128
(on `x86_64` Linux it then travels as the ABI's `__float128` does), `=80` the x87's.
As with `--c-int`, the platform's library keeps its own: a program built with another
format than the ABI's cannot pass `long double` values to it.

### On every target

| | |
|---|---|
| Integers | Two's complement, 8-bit bytes, little-endian. `short` 16 bits, `long long` 64. |
| Conversion to a signed type of a value out of range | Wraps modulo 2^N. |
| Signed overflow | Undefined. The code generated wraps, at every optimization level tried. |
| Right shift of a negative value | Arithmetic (sign-extending): `-8 >> 1` is `-4`. |
| Division | Truncates toward zero; `%` has the sign of the dividend. |
| Complex types | `float _Complex`, `double _Complex` and `long double _Complex` (C99 and later) are laid out as an array of two elements of the real type, the real part first: the size is twice the real type's, the alignment the real type's. There are no imaginary types. |
| Floating types | `float` and `double` are IEEE 754 binary32 and binary64; `long double` as the table above says. `FLT_EVAL_METHOD` is 0, `FLT_ROUNDS` is 1 (to nearest); the rounding mode cannot be changed. A `long double` wider than `double` is computed in software (`std/c/rt/ldouble.ax`, written from IEEE 754-2019): each operation and conversion correctly rounded to nearest, ties to even, with subnormals; constant expressions are folded with the same arithmetic, and floating constants read exactly. Conversions of such a value to an integer type truncate and saturate, as those of `float` and `double` do (the behavior is undefined when the value does not fit). A NaN result of an invalid operation is the format's default NaN (the x87's negative "indefinite"; binary128's positive quiet NaN); a NaN operand gives its own, quieted (of two x87 NaNs, the one with the larger significand). x87 encodings the x87 itself treats as invalid (unnormals, pseudo-infinities, pseudo-NaNs) are invalid operands. The x87 format's six bytes of padding are zero in the values the program computes. |
| Sizes and alignment | A scalar's alignment is its size. A structure is laid out naturally: each member at the next multiple of its alignment, the structure aligned as its strictest member, its size rounded up to that; `#pragma pack(n)` caps the members' alignments at n ("Pragmas"). |
| `max_align_t` | Aligned as the platform's (above): a structure of a `long long` and a `long double` aligned to 16 (size 32 where `long double` is 16 bytes), or `long double` (8 bytes). |
| Alignment specifiers | `_Alignas` takes any power of two up to 4096 (more is `C3071`). |
| Bit-fields | Laid out as the target's ABI lays them out: allocated from the least significant bit of a storage unit of the declared type, never straddling two units. On x86-64 Linux, macOS and WebAssembly (the System V rules) a bit-field goes at the next bit if it fits in the unit that holds it, an unnamed one adds nothing to the alignment of the structure, and one of width 0 moves the next member to the next unit of its type. On AArch64 Linux (AAPCS64) the same, but every bit-field's type adds to the alignment, unnamed ones and those of width 0 too. On Windows (x64) bit-fields are allocated in runs: a bit-field joins the open unit only if its type has the unit's size and it fits, else it starts a new unit after it; another member closes the unit; one of width 0 closes it (and aligns the next member to its type) and is ignored when no unit is open; in a union, bit-fields add their type's size but not its alignment. In a packed structure or under `#pragma pack` (GNU dialects; any `pack(n)`), a bit-field goes at the next bit (System V, AAPCS64; AAPCS64 still aligns the structure to a zero-width one's type), while Windows keeps its units, aligned to at most n. A plain `int` bit-field is signed. A `char` bit-field has `char`'s signedness. |
| Enumerations | The type is `unsigned int` if no enumerator is negative, else `int`, when all fit; the enumeration constants are `int`. In C23, if some enumerator does not fit `int`, the first of `long`, `long long` (`unsigned long`, `unsigned long long` for an unsigned type) that holds all of them, and then the constants have the enumerated type. Before C23 a value outside `int` is `C3013`. With a fixed underlying type (C23), that type. |
| `_BitInt(N)` | `BITINT_MAXWIDTH` is 64. An object is stored in the smallest of 8, 16, 32 or 64 bits that holds N (aligned as that integer type, as the x86-64 and AArch64 ABIs specify), extended to it. As an argument or result it is that integer; the bits beyond N that arrive are not relied on (the ABIs leave them unspecified). |
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
`std/c/include/`, and the C library's ("The C library"). Each is available from the
standard that introduced it; including it in an earlier mode is an `#error`. With
`--c-freestanding` only the ones of this table and `<string.h>` are found; the
library's are `C1081`.

| Header | Modes | Notes |
|---|---|---|
| `<float.h>` | all | `FLT_`, `DBL_` and `LDBL_` macros (the `LDBL_` ones and `DECIMAL_DIG` of the format of `long double`). No `FLT_SNAN`, `DBL_SNAN`, `LDBL_SNAN`. |
| `<limits.h>` | all | `LLONG_` from C99; `*_WIDTH`, `BOOL_MAX` and `BITINT_MAXWIDTH` (64) in C23. |
| `<stdarg.h>` | all | `va_list`, `va_start`, `va_arg`, `va_copy`, `va_end`. In C23 `va_start` needs no second argument. |
| `<stddef.h>` | all | `size_t`, `ptrdiff_t`, `wchar_t`, `NULL`, `offsetof`; `max_align_t` from C11; `nullptr_t` and `unreachable` in C23. |
| `<iso646.h>` | C95 and later | |
| `<stdbool.h>` | C99 and later | In C23, `bool`, `true` and `false` are keywords and the header only defines `__bool_true_false_are_defined`. |
| `<stdint.h>` | C99 and later | No `int32_t` family when no type is 32 bits wide. |
| `<complex.h>` | C99 and later | `complex`, `I` and `_Complex_I` (a constant of type `float _Complex`), `CMPLX`, `CMPLXF` and `CMPLXL` (C11; constant expressions when the operands are), and the declarations of every function. `creal`, `cimag` and `conj` (and their `f` and `l` forms) are also macros the compiler expands in line; the others are a library's, which is not provided. No `imaginary` or `_Imaginary_I`. |
| `<stdalign.h>` | C11 and later | |
| `<stdatomic.h>` | C11 and later | All of 7.17 (see "Atomics and threads"). `ATOMIC_VAR_INIT` until C17; `atomic_char8_t` in C23. |
| `<stdnoreturn.h>` | C11 and later | |
| `<stdbit.h>` | C23 | The `stdc_` functions as `static inline` functions and type-generic macros. |
| `<stdckdint.h>` | C23 | `ckd_add`, `ckd_sub`, `ckd_mul`. |
| `<stdio.h>` | all (hosted) | The C library's ("The C library"). `snprintf`, `vscanf` and C99's other names from C99; `gets` before C11 only; `__STDC_VERSION_STDIO_H__` and `_PRINTF_NAN_LEN_MAX` in C23. |

| `<string.h>` | all | C23's freestanding functions in every program (all but `strdup`, `strndup`, `strcoll`, `strxfrm` and `strerror`, which hosted programs have: "The C library"). `memccpy` and `memset_explicit` from C23; in C23 `memchr`, `strchr`, `strpbrk`, `strrchr` and `strstr` are also generic macros whose result points to `const` when the argument does. |

The library's headers are in "The C library"; any other is `C1081`. The C23 headers
also define `__STDC_VERSION_<NAME>_H__` as the standard says.

| `<locale.h>`, `<time.h>` | all | The C library's ([`docs/c-text.md`](c-text.md)). |
| `<wchar.h>`, `<wctype.h>` | C95 and later | The C library's ([`docs/c-text.md`](c-text.md)). |
| `<inttypes.h>` | C99 and later | The C library's ([`docs/c-text.md`](c-text.md)). |
| `<uchar.h>` | C11 and later | The C library's ([`docs/c-text.md`](c-text.md)). |

Every other header is `C1081`. C23 (4p7) also asks a
freestanding implementation for most of `<string.h>` and some functions of `<stdlib.h>`
(and `<fenv.h>` and `<math.h>` if it claims Annex F). These are a library's, and the
compiler does not provide them. The C23 headers also define `__STDC_VERSION_<NAME>_H__`
as the standard says.

## Not supported

- `_Imaginary` (the imaginary types, which are optional), decimal floating types and
  constants: each is `C3901` (a decimal constant is `C1019`).
- `<threads.h>` (`__STDC_NO_THREADS__`). Thread-local objects and atomics work with the
  threads a program has (see "Atomics and threads").
- `<math.h>` and `<fenv.h>`, and Annex F (exception flags and rounding modes). Annex K is
  not provided (`__STDC_LIB_EXT1__` is not defined).
- `setjmp` on WebAssembly (`C3300`).
- `#embed` parameters other than `limit`, `prefix`, `suffix` and `if_empty` (`C1084`).
- Extensions, under an ISO standard. These are all errors there: `__attribute__`, `asm`
  and `__asm__`, `__int128`, `__typeof__` (and `typeof` before C23), statement
  expressions, `?:` with an omitted middle operand, case ranges, computed `goto` and labels
  as values, `__builtin_*` functions other than those below ("Reserved identifiers"),
  `#include_next`, `#ident`. A GNU dialect has them ("GNU dialects"), but for those it
  rejects too.
- In every mode: inline assembly on WebAssembly targets (C3901), `__declspec`, `#import`,
  `#assert`, `__int128`, the floating types `_Float16`, `_FloatN`, `_FloatNx` and
  `__float128`, and vector types (the attribute `vector_size`): C3901 in a GNU dialect.

## GNU dialects

`--c-std=gnu89`, `gnu90`, `gnu99`, `gnu11`, `gnu17`, `gnu18` and `gnu23` are the standard
of their number with the extensions below, each checked as the rest of the language is
(constraint violations are errors). `__GNUC__` is not defined: it claims every extension of
another compiler and its version's builtins, which programs then rely on; a program tests
`__AXIOM_GNU__`, `__has_attribute`, `__has_builtin` and `__has_include` instead, which say
exactly what works (there is no option that defines `__GNUC__`).

### Language

| Extension | |
|---|---|
| Keywords | `__extension__` (ignored), `__inline`, `__inline__`, `__const`, `__const__`, `__volatile`, `__volatile__`, `__restrict`, `__restrict__`, `__signed`, `__signed__`, `__alignof`, `__alignof__` (also of an expression: `__alignof__ x`, a member's own alignment), `__typeof`, `__typeof__` and `typeof` in every dialect, `__thread`, `asm`, `__asm`, `__asm__`; `inline` in gnu89. |
| C99 and C11 in older dialects | From gnu89: `//` comments, `long long`, mixed declarations and code, designated initializers, compound literals, a trailing comma in an enumerator list, variable length arrays, flexible array members, variadic macros and empty macro arguments, hexadecimal floating constants, `_Pragma`, `__func__`, initializers of aggregates that are not constants; from gnu89 and gnu99: anonymous structures and unions; in every dialect: attributes `[[...]]` and `{}`. |
| `__auto_type x = e;` | x has e's type (as C23's `auto`). |
| `__label__ a, b;` | Labels local to the block (a statement expression in a macro can have its own). |
| `({ ... })` | A statement expression: its value is that of its last statement if that is an expression statement (after its labels), else none (void); its objects with cleanups are cleaned up after it is computed. Not outside a function (C3090). `break`, `continue`, `goto` and `return` may leave it. |
| `&&label`, `goto *p` | Labels as values: `&&label` is a `void *`, an address constant (so it may initialize a static object: `static void *t[] = { &&a, &&b };`), equal only to itself; `goto *p` jumps to the label whose address p is. A label's address is an address in a table of its function's (one byte for each label whose address is taken), and `goto *` is a switch on the distance from the table's start; so the values compare and subtract as addresses (`&&a - &&b` of one function's labels is an integer constant), and stay valid when the function is inlined. Not where a variable length array or an object with a cleanup is in scope (C3901). |
| `case lo ... hi:` | A case range (inclusive); an empty one (lo > hi) is the warning C4120; overlapping another case is C3041. |
| `a ?: b` | `a ? a : b` with a evaluated once. |
| `[lo ... hi] = v` | A designator range (several in one designation too); v must have no effects if the range has more than one element (it would be evaluated once: C3901). |
| `member: v` | The old designator `.member = v`. |
| Zero-length arrays | `T a[0]`: size 0 (as a structure's last member, a flexible array member's place). |
| Flexible array members | May be initialized in an object with static storage duration: `static struct s v = { 2, { 1, 2 } };` makes v as large as its initializer (sizeof still gives the type's size). |
| Structures and unions without members | Of size 0. |
| Compound literals of constants | May initialize an object with static storage duration as their lists would (`static struct p v = (struct p){ 1, 2 };`). |
| `void *` and function pointers | Arithmetic on them as on `char *`; `sizeof` and `__alignof__` of `void` and of a function type are 1. |
| Constants | Binary constants (`0b101`) and the escape `\e` (27) in every dialect. |
| `__FUNCTION__`, `__PRETTY_FUNCTION__` | Other names of `__func__` (`""` outside a function). |
| `T f(...) asm("name");` | The function's or object's symbol is `name` (an asm label; with external linkage). On an automatic object it is C3090, but on a `register` one: an explicit register variable ("Inline assembly"). |
| `asm`, `__asm__` statements; file-scope `asm("...")` | Inline assembly on x86-64 and AArch64 ("Inline assembly", below). |

### Preprocessor

`#include_next <file>` (and `__has_include_next`): the next file of the name in the
include path after the directory the current file was found in; `#warning` before C23
(C4103); `#ident "text"` (ignored). Variadic macros may name their variable arguments
(`#define F(fmt, args...) f(fmt, args)`), may be invoked without them before C23, and
`, ## __VA_ARGS__` is the comma and the variable arguments, or nothing when there are none.
`__COUNTER__` (0, 1, 2, ... at each use), `__INCLUDE_LEVEL__`, `__BASE_FILE__`,
`__TIMESTAMP__` (the current file's modification time, in UTC, as asctime writes it);
`__ATOMIC_RELAXED` ... `__ATOMIC_SEQ_CST` (0 to 5). In every mode, `__has_include`,
`__has_attribute` (1 for the GNU attributes below in a GNU dialect, and the standard ones'
values in C23) and `__has_builtin` (1 for the builtins that work in the mode).

### Attributes

`__attribute__((...))` in the places a GNU dialect allows them: among declaration
specifiers and after a declarator (they appertain to the declared entities, every one of
a declaration's), after `struct`, `union` or `enum` and after the closing brace of its list
(to the type), on members and bit-fields, parameters, enumerators, labels (`l:
__attribute__((unused));`) and statements (`__attribute__((fallthrough));`), after `*`.
`[[gnu::name]]` and `[[__gnu__::name]]` are the same in a GNU dialect (under an ISO
standard they are a vendor's: ignored). Each name may be written `__name__`. Arguments are
checked (C3077, C3071). An unknown attribute, or one where it cannot appertain, is warning
C4052 and is ignored.

| Attribute | Of | What it does |
|---|---|---|
| `aligned`, `aligned(n)` | objects, members, structures and unions | At least n bytes (no n: 16). On a typedef only its type's own alignment (another: C3901); on a function accepted (code is aligned as the target's). |
| `packed` | structures, unions, members, enumerations | Members at byte alignment (but those with an alignment of their own); bit-fields at the next bit, each reached through the smallest unit of 1, 2, 4 or 8 bytes in the structure that holds it (none: C3901); an enumeration's type is the smallest integer type of its values. Members of a packed structure are loaded and stored with the alignment they have. |
| `section("s")` | objects with static storage duration, functions | The object goes in section s of an ELF object (Linux); elsewhere, and for functions, accepted with no effect. The compiler's own linker does not define `__start_s` and `__stop_s`. |
| `used` | objects, functions | Kept though nothing refers to it. |
| `unused` | any declaration, labels | As `[[maybe_unused]]`. |
| `weak` | objects and functions with linkage | A weak definition, which gives way to another unit's that is not weak ("C and Axiom in one program"), or a weak reference (a null pointer if no unit defines it; on WebAssembly the host's import). `#pragma weak` too. |
| `alias("target")` | functions and objects | Another name for target, defined in the same unit (else C3090): references to it are to target. It has no symbol of its own: another unit cannot refer to it. |
| `visibility("v")` | objects, functions | default, hidden, protected or internal: accepted (no effect: programs are static executables, and objects are linked into them). |
| `noreturn` | functions | As `[[noreturn]]` (C4053). |
| `noinline`, `always_inline`, `cold` | functions | Given to the optimizer. |
| `naked` | functions | The function is its asm statements (basic, or extended with no operands: else C3096): their text after a label of its name, with no prologue or epilogue ("Inline assembly"). |
| `hot`, `const`, `pure`, `malloc`, `nothrow`, `leaf`, `artificial`, `flatten`, `noclone`, `noipa`, `externally_visible`, `no_instrument_function`, `returns_nonnull`, `alloc_size`, `alloc_align`, `assume_aligned`, `sentinel`, `format_arg`, `may_alias` | | Accepted; no effect (none is needed for correct code). |
| `format(archetype, i, j)` | functions | Arguments checked; the calls are not. |
| `nonnull`, `nonnull(i, ...)` | functions | A null pointer constant passed for one of those parameters (all pointers without indexes) is warning C4121. |
| `warn_unused_result` | functions | As `[[nodiscard]]` (C4051). |
| `deprecated`, `deprecated("m")`, `unavailable` | declarations | As `[[deprecated]]` (C4050). |
| `error("m")`, `warning("m")` | functions | A call is error C3091, or warning C4123. |
| `fallthrough` | an attribute statement | As `[[fallthrough]]`. |
| `gnu_inline` | functions | GNU89's inline semantics (below) in a later dialect. |
| `constructor`, `constructor(p)`, `destructor`, `destructor(p)` | functions without parameters | Called by the program's start-up before `main` (lowest priority first; without one, 65535; then in the order defined), and after `main` returns (highest priority first, the last defined first). `exit` (the C library's) runs the destructors too, after the `atexit` functions, once; a program whose `main` is Axiom's runs the constructors before it and the destructors when it returns; an object without `main` (`--emit=obj`), and a freestanding program that ends by calling the system's `exit`, run neither, or no destructors. |
| `cleanup(f)` | automatic objects | `f(&object)` when its scope is left: the end of its block, `break`, `continue`, `goto` out, `return` (after the value is computed), the end of a statement expression; innermost first. A jump into its scope is C3083. |
| `transparent_union` | unions (and their typedefs) | A parameter of the type takes an argument of any member's type (or a null pointer constant, or a pointer to void for a pointer member). It is passed as the union is (the platform ABIs pass a union of one pointer or integer as that member). |
| `mode(m)` | integer objects, members, typedefs | The integer type of m's width with the declared type's signedness: QI, HI, SI, DI, `byte`, `word`, `pointer` (also `__QI__` ...); TI (128 bits) is C3901. |
| `vector_size(n)` | | C3901: vector types are not supported. |

In gnu89 and gnu90 `inline` has GNU89's meaning: `inline` alone defines the external
function, `extern inline` is an inline definition (C99's `inline`).

### Inline assembly

On x86-64 (Linux, Windows) and AArch64 (Linux, macOS) targets; on WebAssembly an asm
statement is C3901. The compiler has its own assemblers (`cmd/axiom/src/asm/`), so no
other tool is needed:

- **x86-64**: the AT&T syntax (source operands first; `%reg`, `$imm`,
  `disp(base, index, scale)`, `%fs:`/`%gs:` overrides, `*` for indirect jumps and calls,
  size suffixes b, w, l, q or the size of a register operand, `lock`, `rep`, `repe`,
  `repne`); `{att|intel}` dialect alternatives give their first. Instructions: the
  general-purpose ones a program runs (moves, arithmetic, logic, shifts and rotates, bit
  operations, `cmovcc`, `setcc`, jumps, calls, `push`/`pop`, string instructions, `xchg`,
  `xadd`, `cmpxchg`, `cmpxchg8b`/`16b`, `bswap`, `cpuid`, `rdtsc`/`rdtscp`, `syscall`,
  fences, `pause`, `ud2`, `crc32`, `movbe`, `rdrand`/`rdseed`, prefetches, the multi-byte
  `nop`s ...), BMI1 and BMI2, and SSE through SSE4.1's moves, arithmetic, compares,
  conversions, logic, shuffles, packs, integer instructions and shifts.
- **AArch64**: the base integer instructions (with their aliases: `mov`, `cmp`, `lsl`,
  `ubfx`, `cset` ...), branches, loads and stores in every addressing form (pairs,
  exclusives, acquire/release, the LSE atomics), `svc`, `brk`, hints, barriers, `mrs`/`msr`
  of the registers programs use (`tpidr_el0`, `fpcr`, `fpsr`, `nzcv`, `cntvct_el0`, ... and
  `s<op0>_<op1>_c<n>_c<m>_<op2>`), scalar floating point, and the common vector
  instructions; immediates with or without `#`.
- Directives: `.byte`, `.short`/`.hword`/`.2byte`, `.word` (2 bytes on x86-64, 4 on
  AArch64), `.long`/`.int`, `.quad`/`.xword`, `.ascii`, `.asciz`/`.string`, `.float`,
  `.double`, `.zero`/`.skip`/`.space`, `.fill`, `.balign`, `.p2align`, `.align`, `.set`/`.equ`
  and `=`, `.inst` (AArch64); in file-scope asm also `.text`, `.data`, `.bss`,
  `.section`, `.pushsection`, `.popsection`, `.previous`, `.globl`, `.weak`, `.local`,
  `.type`, `.size`. Labels: `name:` and numeric local labels (`1:`, `1b`, `1f`).
  Relocations: x86-64 rel32 calls and jumps to symbols (`@PLT`), `sym(%rip)`
  (`@GOTPCREL`), absolute 32- and 64-bit addresses; AArch64 `b`/`bl` to symbols,
  `adrp` (`:got:`), `:lo12:` and `:got_lo12:` in `add` and loads and stores, `.quad` and
  `.word` of addresses. `.cfi_*`, `.file`, `.loc`, `.ident`, `.arch` and the like are
  accepted and do nothing. An instruction, operand form or directive that is not
  supported is C3097, naming it, at the statement.

`asm [volatile] [inline] [goto] ("template" : outputs : inputs : clobbers : labels)`, as
GCC's documentation describes extended asm, with these rules:

- Operands are numbered outputs first, then inputs, then (for `%l`) labels; `%[name]`
  names one. An output's constraint starts with `=` or `+` (also an input: it is read and
  written), and its operand is a modifiable lvalue; `&` is early clobber. Matching
  constraints (`"0"`, `"[name]"`) tie an input to an output in a register.
- Constraints (the first alternative of `"r,m"`): `r`, `g`, `X`, `m`, `o`, `V`, `i`, `n`,
  `s`, the digits; x86-64 `a b c d S D` (those registers), `q`, `R`, `Q`, `U`, `l`, `x`,
  `v`, `Y`, `I J K M N O e Z` (constants in their ranges); AArch64 `w` (any v register),
  `x` (v0-v15), `y` (v0-v7), `Q` (memory at a base register), `I J K L M N Z S`. An operand
  chooses a constant when it is one (and the constraint allows it), else a register, else
  memory. A register operand is an integer, pointer or floating value of 1, 2, 4 or 8
  bytes (a floating value in a general register is its bits, an integer in a vector
  register too). `A`, `t`, `u`, `f`, `y` (x87, MMX) and AVX-512 mask registers are C3901.
- Explicit register variables, `register long x asm("rax")` (or `"x8"`, `"w19"`, `"d0"`
  ...): such a variable is in that register when it is an asm operand in a register (the
  usual way to pass system call arguments); elsewhere it is an ordinary variable. The stack
  and frame pointers, and on AArch64 x16, x17 and x18, cannot be.
- Clobbers: registers (with or without `%` on x86-64), `"cc"` (always assumed on x86-64),
  `"memory"`; not the stack or frame pointer (C3096). A clobbered callee-saved register is
  saved by the function.
- Template modifiers: x86-64 `%b %w %k %q %h` (a register's widths, its high byte), `%x`
  (xmm), `%c %P %p %a` (a constant without `$`, an address), `%n` (negated), `%A` (`*`),
  `%H` (memory 8 bytes on), `%V`, `%z` (the size suffix); AArch64 `%w %x` (also of the
  constant 0: `wzr`, `xzr`), `%b %h %s %d %q`, `%c`, `%n`. Without a modifier an x86-64
  register is written at its operand's width, an AArch64 general register as `w` up to 32
  bits and `x` for 64, and a floating-point register as `s` or `d` by its type; an AArch64
  memory operand is `[xn]`. `%%`, `%=` (a number unique to each copy of the statement),
  `%l[label]`.
- `asm goto`'s template may jump to its labels (`%l[name]`); outputs are stored on every
  path. A jump out of a scope leaves it as `goto` would.
- Every asm statement is treated as `volatile` and as clobbering memory: none is removed,
  moved or merged, and none is reordered with memory accesses (the optimizer sees an
  opaque operation). Outputs in registers are stored to temporaries just after the
  template and then assigned to their lvalues. An output is never given an input's
  register (as though every output were early clobber), which is always correct.
- The template must leave the stack pointer where it was (pushes and pops in pairs are
  fine) and must not change the frame or the frame pointer: call frame information and
  stack traces describe the function as compiled (`.cfi_*` directives are ignored). A
  template that calls a function aligns the stack itself (as with other compilers). A
  function with an asm statement makes no sibling calls. Named labels in a template are
  local to it (the template is assembled on its own); a symbol a template refers to by
  name has external linkage (or is defined by file-scope asm), and is kept in the program.
  `.p2align` in a template aligns relative to the function's start (16 bytes on x86-64).
- File-scope `asm("...")` is a basic asm (operands are C3096) whose text is assembled into
  the unit's object: its labels are local symbols unless `.globl`, and C refers to its
  functions and objects by declaring them. On Windows it is assembled the same way (the
  calling convention in it is the template's business).

### Builtins

In a GNU dialect (under an ISO standard only those of "Reserved identifiers"); each name
`__has_builtin` gives 1 for. Their operands are checked (C3023, C3082).

| Builtins | |
|---|---|
| `__builtin_expect(x, c)`, `__builtin_expect_with_probability(x, c, p)` | x. |
| `__builtin_unreachable()`, `__builtin_trap()` | Undefined if reached; a trap. |
| `__builtin_constant_p(x)` | 1 if x is an arithmetic or address constant (it is not evaluated), else 0: a constant. |
| `__builtin_choose_expr(c, a, b)` | a if the integer constant c is nonzero, else b (unconverted; the other is not checked). |
| `__builtin_types_compatible_p(T1, T2)` | 1 if the types are compatible (top-level qualifiers aside). |
| `__builtin_offsetof(T, m)` | As `offsetof`. |
| `__builtin_object_size(p, t)`, `__builtin_dynamic_object_size` | The size is never known: `(size_t)-1` for t 0 and 1, 0 for 2 and 3. |
| `__builtin_clz`, `ctz`, `popcount`, `parity`, `ffs`, `clrsb`, with `l` and `ll` | Of `unsigned int` (ffs, clrsb: `int`) ... `unsigned long long`; clz and ctz of 0 give the type's width. |
| `__builtin_bswap16`, `bswap32`, `bswap64` | |
| `__builtin_add_overflow`, `sub_overflow`, `mul_overflow(a, b, r)` | As `ckd_add(r, a, b)`; the typed `__builtin_sadd_overflow` ... `umulll_overflow`; `__builtin_add_overflow_p(a, b, c)` ... (whether the result overflows c's type). |
| `__builtin_memcpy`, `memmove`, `memset` | The IR's operations; their value is the destination. |
| `__builtin_abs`, `fabs`, `sqrt`, `floor`, `ceil`, `trunc`, `rint`, `nearbyint`, `copysign`, `fma` (with `f` and `l`) | The IR's operations (`sqrt` of a negative number is a NaN; errno is not set; `fma` on WebAssembly calls the library's). Where `long double` is wider than `double`, the `l` forms but `fabsl` and `copysignl` (which change the sign bit) call the library's function of that name. |
| `__builtin_memcmp`, `strlen`, `strcmp`, `strncmp`, `strcpy`, `strncpy`, `strcat`, `strncat`, `strchr`, `strrchr`, `strstr`, `memchr`, `printf`, `sprintf`, `snprintf`, `puts`, `putchar`, `malloc`, `calloc`, `realloc`, `free`, `abort`, `exit`, `labs`, `llabs`, and the math functions (`sin`, `cos`, `tan`, `exp`, `log`, `pow`, `round`, `fmin`, `fmax`, `fmod`, `hypot`, `ldexp`, `frexp`, `modf`, `lround` ... with `f` and `l` where the library has them) | Calls of the C library's function of that name. |
| `__builtin_alloca(n)`, `__builtin_alloca_with_align(n, bits)` | n bytes of the function's frame, until it returns. |
| `__builtin_frame_address(0)`, `__builtin_return_address(0)` | The frame record's address and the return address (natively); on WebAssembly the stack pointer and a null pointer. Another level is C3090. |
| `__builtin_prefetch(p, ...)`, `__builtin_assume_aligned(p, a [, o])` | Nothing; p. |
| `__builtin_inf`, `huge_val`, `nan("")`, `nans("")` (with `f` and `l`) | Constants (a NaN's payload is not kept; a signaling NaN is quiet; a NaN is positive). |
| `__builtin_isnan`, `isinf`, `isfinite`, `isnormal`, `signbit`, `isinf_sign`, `fpclassify(a, b, c, d, e, x)`, `isgreater`, `isgreaterequal`, `isless`, `islessequal`, `islessgreater`, `isunordered` | Of the floating type of the operands (an integer is a double). |
| `__sync_fetch_and_op`, `__sync_op_and_fetch` (op: add, sub, or, and, xor, nand), `__sync_bool_compare_and_swap`, `__sync_val_compare_and_swap`, `__sync_lock_test_and_set`, `__sync_lock_release`, `__sync_synchronize` | Atomic, sequentially consistent. |
| `__atomic_load_n`, `store_n`, `exchange_n`, `compare_exchange_n`, `fetch_op`, `op_fetch`, `test_and_set`, `clear`, `thread_fence`, `signal_fence`, the generic `load`, `store`, `exchange`, `compare_exchange`, `always_lock_free`, `is_lock_free` | Every memory order is taken as `__ATOMIC_SEQ_CST` (a stronger order is a valid one). |

The atomic builtins take a pointer to an integer or pointer type of 1, 2, 4 or 8 bytes,
aligned to its size (the generic ones: to any complete object), and are the operations
of C11's atomics (see "Atomics and threads"; nand is a compare-and-swap loop): lock-free
on objects of 1, 2, 4 and 8 bytes on every target, and for other sizes (the generic
forms) under the same table of locks as `_Atomic` objects, so the two agree.

## Pragmas

Every mode reads `#pragma` and `_Pragma("...")`. The operands are not macro-replaced,
but those of `message`, `GCC warning` and `GCC error`. A known pragma that is malformed
is the warning C4112 and is ignored; an unknown one is ignored (the warning C4111 with
`--c-warn-unknown-pragmas`).

| Pragma | |
|---|---|
| `once` | The file is not included again. |
| `pack(n)`, `pack()`, `pack(push [, id] [, n])`, `pack(pop [, id or n])`, `pack(show)` | Members of the structures and unions whose member lists end after it are aligned to at most n bytes (1, 2, 4, 8 or 16; bit-fields as in a packed structure, in units aligned to at most n); `show` is the warning C4114 with the value. |
| `push_macro("M")`, `pop_macro("M")` | M's definition (or that it has none) saved, and made M's again. |
| `weak name`, `weak name = target` | As the attribute weak (and alias), if the unit declares name. |
| `message("text")` | The warning C4110 with the text. |
| `STDC FP_CONTRACT ON/OFF/DEFAULT` | Accepted: no floating operations are contracted (an FMA is only the `fma` the program calls), so both hold. |
| `STDC FENV_ACCESS ON/OFF/DEFAULT` | OFF holds; ON is the warning C4113: the floating-point environment cannot be changed or tested, and the optimizer assumes the default one. |
| `STDC CX_LIMITED_RANGE ON/OFF/DEFAULT` | Accepted (it allows, and asks for nothing). |
| `STDC FENV_ROUND dir` (C23) | FE_TONEAREST and FE_DYNAMIC hold; the other directions are C4113. |
| `GCC diagnostic push`, `pop`, `ignored "-Wx"`, `warning "-Wx"`, `error "-Wx"` | The warnings after it (in its file, and in the files it includes) that -Wx names are not shown, shown, or errors (keeping their codes). -Wx is one of the compiler's codes (`"C4050"`), `-Wall`, `-Wextra` and `-Wpedantic` (every warning), or a usual name that stands for one: `-Wdeprecated-declarations` and `-Wdeprecated` (C4050), `-Wunused-result` and `-Wunused-value` (C4051), `-Wattributes` (C4052), `-Winvalid-noreturn` (C4053), `-Wimplicit-function-declaration` (C4001), `-Wunused-label` (C4040), `-Wvarargs` (C4080), `-Wmultichar` (C4102), `-Wcpp` (C4103), `-Wunknown-pragmas` (C4111), `-Wpragmas` (C4112), `-Wnonnull` (C4121), `-Wswitch-outside-range` (C4120), `-Wattribute-warning` (C4123); others are ignored. An error is never made a warning. |
| `GCC visibility push(v)`, `pop` | Accepted (as the attribute visibility). |
| `GCC poison x y` | A later use of x or y is the error C1086 (not one a macro defined before makes). |
| `GCC system_header` | The rest of the file gives no warnings (in the main file it means nothing). |
| `GCC warning "m"`, `GCC error "m"` | The warning C4103, or the error C1040. |
| `GCC push_options`, `pop_options`, `optimize`, `target`, `unroll`, `ivdep`, `novector`, `dependency` | Ignored. |

`--emit=c-pp` writes the pragmas the later phases read, where they change what they say: `pack(n)` (the value then in force), `GCC visibility`, and the `weak` ones at the end; the others have acted already.

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
| `__builtin_atomic_...` | The generic functions of `<stdatomic.h>`. |
| `__axiom_c_thread_start`, `__axiom_c_thread_join` | The runtime's threads (see "Atomics and threads"). |
| `__axiom_atomic_lock`, `__axiom_atomic_unlock`, `__axiom_atomic_locks` | The lock of atomic objects that are not lock-free. |
| `__builtin_inff`, `__builtin_nanf` | `<float.h>`. |
| `__builtin_complex`, `__builtin_creal`, `__builtin_cimag`, `__builtin_conj` | `<complex.h>`: `__builtin_complex(x, y)` is `x + iy` for `x` and `y` of one real floating type. |
| `__axiom_cmul_f32`, `__axiom_cdiv_f64`, ... | The compiler's own functions for complex multiplication and division (internal to the program). |
| `__axiom_int64` | A keyword for a 64-bit integer type (`unsigned __axiom_int64` too) in every mode, for `size_t` in headers before C99 and for programs that need one in C89. |
| `__AXIOM_...`, `__axiom_...` | Macros and members of the headers. |
| `__axc_...`, `__AXC_...` | The C library's internal names (`__axc_setjmp`, `__axc_errno`, ...), and its private headers `axc/*.h`. |

## Programs

A C program's `main` is called by a start-up module of the runtime, which the build adds.
It gives `main` its arguments (`argv[argc]` is null) and uses its result as the exit
status: it calls the C library's `exit` with `main`'s value ("Start-up and exit"),
which runs the `atexit` functions; a freestanding program's start-up does so on
Windows (with `msvcrt.dll`'s), and elsewhere the system's own start-up ends the process
(on WebAssembly, the host).

A program that defines `memcpy`, `memset` and `memmove` itself builds for every target.
Names that the runtime uses for its own functions (such as `ax_write`) cannot be
defined: `C3062` (reported by `build`).

## Atomics and threads

`_Atomic` qualifies a type, and `_Atomic(T)` names the same atomic type. An atomic type
has the size of its non-atomic version; one of 1, 2, 4, 8 or 16 bytes is aligned to its
size (so `_Atomic struct { char c[2]; }` is aligned to 2), any other keeps its
alignment, on every target: the rule of the x86-64 and AArch64 psABIs, so atomic
objects are laid out as the platform's C compilers lay them out (checked against them on
Linux). An atomic type cannot be an array or function type (`C3092`), `_Atomic(T)`
cannot be of an array, function, atomic or qualified type (`C3092`), and a bit-field
cannot be atomic (implementation-defined, 6.7.2.1p5: `C3070`). An atomic type is not a
qualified version of its non-atomic type: `int *` and `_Atomic int *` are incompatible
(`C3022`), and a parameter or result keeps its `_Atomic` in the function's type. A
member of an atomic structure or union accessed (undefined) is the warning `C4090`, and
`void *p = &atomic_object` the warning `C4091`.

Reading an atomic object (its value used), assigning it, `++`, `--` and the compound
assignments are atomic operations (6.5.2.4, 6.5.16.2), all sequentially consistent.
`+=`, `-=`, `&=`, `|=`, `^=`, `++` and `--` on an atomic integer or pointer are one
read-modify-write instruction; the others (`*=`, `/=`, shifts, floating types, `_Bool`)
are a loop of compare-exchanges that computes the new value again if another thread
changed the object meanwhile. There is no floating-point environment, so the loop has no
exceptions to discard (6.5.16.2p3, note).

Objects of 1, 2, 4 and 8 bytes (every integer type and pointer) are lock-free: x86-64's
locked instructions, AArch64's exclusive or LSE instructions, WebAssembly's atomic
instructions. Others (a structure of 3 or of 16 bytes, a `double _Complex`) take one of 64 spinlocks, chosen
by the object's address, in a table that every module and object holding such code
defines once (linkonce). A signal handler must not touch such an object while the
interrupted thread may hold its lock, and the platform C compilers' code uses its own
library's locks for them: an object of this kind is atomic only among code compiled here.

`<stdatomic.h>` has all of 7.17. Its generic functions are the compiler's builtins
(`__builtin_atomic_load` and the others); atomic_flag's functions and the fences are
`static inline` functions in the header. Every `memory_order` is made sequentially
consistent, which is a valid implementation of each; `atomic_compare_exchange_weak` is
the strong form (it never fails spuriously); `atomic_signal_fence` is a full fence. The
`ATOMIC_*_LOCK_FREE` macros are 2 and `atomic_is_lock_free` says what the size gives.
On WebAssembly without the atomics feature (`--features=-atomics`) a module has one
thread, and the operations are ordinary loads and stores.

`_Thread_local` (C23: `thread_local`) objects have one instance per thread, each starting
with the object's initial value (which may hold addresses). On Linux they are ELF TLS
(local-exec, so in an executable or an object linked into one); the C library's threads
and the runtime's each get their copies. On Windows they are in the image's TLS
directory, so every thread has them. On macOS they are the runtime's thread-local
block, which the main thread and the runtime's threads have (threads the C library
creates directly do not: there is no `<threads.h>` yet). On WebAssembly, in a program
without threads they are ordinary globals; with threads, each thread's start makes its
block of them. `--emit=obj` objects for WebAssembly have them as ordinary globals.

Until `<threads.h>` exists, the runtime's threads are reached through two functions that
a program declares (the suite's threaded tests use them):

```c
void *__axiom_c_thread_start(void (*f)(void *), void *arg);  /* null if it fails */
void __axiom_c_thread_join(void *thread);
```

A program that names them has the start-up `std/internal/runtime/c_threads.ax` (which
defines them) instead of `c_start.ax`.

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

A `long double` travels as the ABI says for its format: the x87's in memory (System V's
classes X87 and X87UP; a structure or union holding one is classified with them), its
result in the x87 register `st(0)`; binary128 as AAPCS64's quad-precision type (in a `v`
register, homogeneous aggregates of up to four included), as System V's `__float128` (SSE
and SSEUP: one `xmm` register), and on WebAssembly as two `i64`s, its result through a
pointer first parameter.

`va_list` is the ABI's: an array of one structure on x86-64 System V, the AAPCS64
structure on AArch64 Linux, and `char *` elsewhere, so it can be passed to the library's
`v...` functions.

On Windows, an object that C code declares and no unit defines is a DLL's, by the same
rules as functions (`msvcrt.dll`'s unless its name is a Windows API's): the program
reads its address from the import address table, so `extern int _fmode;` is the C
library's own object. Its address is not an address constant there (it is known only
when the program runs): an initializer of an object with static storage duration that
takes it is `C3067`. A function whose stack pointer moves (one with a variable length
array) has unwind information that finds its saved registers from its frame pointer,
so the system's stack walks (exceptions, debuggers) go through it while the array is
live.

Arguments aligned more strictly than the stack: on x86-64 System V a call that passes one
aligned to more than 16 bytes on the stack aligns its stack pointer to that alignment,
named and variadic arguments alike; AAPCS64 and Windows pass such structures by reference,
and a function copies one into an object aligned as its type is (the caller's copy is
aligned to 16 at most). On macOS each variadic argument is on the stack at the next offset
aligned to its alignment, at most 16. On AAPCS64 a structure aligned to 16 starts at an
even register or a 16-byte stack offset.

Verified against the system's C compiler on x86-64 and AArch64 Linux, with
callers and callees compiled by either and at different optimization levels: thousands of
random structure and union shapes (`cmd/axiom/internal/tests/tools/gen_c_abi.py`: nested,
arrays, bit-fields, floating and integer members, `long double` and `long double
_Complex` ones (`--ld`), over-aligned members, 1 to about 64 bytes) passed and returned by value, through pointers and as variadic arguments read with
`va_arg`, `va_copy` and a `va_list` passed on; the layouts of bit-fields on every target
against the platforms' C compilers. The suite runs a subset (`c/run/abi_gen_*.c`). Windows
and macOS output is built but not run here. `long double` (alone, in
structures and unions, variadic, through function pointers, both ways, and the C
library's `printf("%Lg")` and `strtold`) is tested against the system's C compiler on
`x86_64` Linux and on `aarch64` Linux (under emulation), and with `--c-long-double=128`
on `x86_64` against its `_Float128`; the WebAssembly signatures were compared with
another compiler's for that target.

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

`--stack-traces=off` prints the crash line only. The trace is made of the call-site
records of the calls the crash is below: each line names the function that made a call
and where, and the first the function called (only as `<indirect call>` when it was
called through a pointer). The faulting instruction itself has no record, so its line
is not shown (a debugger finds it in the line tables: "Debugging").

At `-O2` and `-O3` the trace is correct for the frames that exist, and the calls the
optimizer inlined into them are in it too (an inlined call's record carries the calls
it was inlined into). What is absent: a function whose last act was a call
(`return f(x);`) leaves its frame first, so the trace lacks it; and a crash in code
inlined into a function that makes no call between it and its caller (all of a small
program inlined into `main`, say) shows only the frames from that function's caller
up, which can be none: then only the crash line is printed. At `-O0` and `-O1` the
functions are not inlined and the trace is complete in the programs tried. C code has none of
Axiom's run-time checks: no bounds or overflow checks, so a bad pointer is a crash if it
is one at all. On WebAssembly C functions leave no call records: a trap (such as a
division by zero) reports `__axiom_c_main` and the start-up, and no C frames.

## Debugging

`-g` (or `--debug-info=full`) writes DWARF 5 debug information, `--debug-info=lines`
the line tables and frames only, and `--debug-info=none` (the default) nothing. It does
not change the code generated, at any optimization level, and two builds are the same.

- **Lines**: each function's code is mapped to the lines of the source, `#line`
  directives followed (the presumed file and line, as diagnostics show them); code the
  optimizer inlined or moved keeps the line it came from. The prologue ends, for a
  breakpoint on a function, past the stores of its parameters. Columns are not
  recorded.
- **Frames**: call frame information for every instruction of every function
  (prologues, epilogues, sibling calls, code run before the frame is set up), so that
  a debugger can unwind the stack wherever it stops.
- **Types and variables** (`full`): the program's types (base types, pointers,
  qualifiers, arrays, structures and unions with their bit-fields, enumerations and
  their constants, function types); each function's result type, parameters and local
  variables; objects of static storage duration (a block-scope `static` one appears
  at the top level, beside the file's). A variable declared with a typedef name has
  the type the name stands for: typedefs are not described. All of a function's
  variables are in its scope: there are no lexical blocks, so two variables of the
  same name in one function cannot be told apart. At `-O0` every variable is in
  memory, in its frame slot, and has a location; at `-O1` and above variables have
  none ("optimized out"), never a wrong one. The object of a variable length array
  has no location, and its type no bound.

Where it goes, per target:

| Target | |
|---|---|
| `x86_64`, `aarch64` | The executable's (or object's) `.debug_*` sections: `.debug_info`, `.debug_abbrev`, `.debug_str`, `.debug_line`, `.debug_line_str`, `.debug_aranges`, `.debug_frame`. |
| `aarch64-macos` | The debug bundle `<out>.dSYM` beside the executable, which the platform's debugger finds by the executable's UUID; the executable is the same as without `-g`. |
| `x86_64-windows` | The PDB (`--pdb=on`, the default) has every function's line table, with or without `-g`; no types or variables. |
| `wasm32`, `wasm64` | Custom sections `.debug_info`, `.debug_line`, ... of the module, code addresses as offsets in its code section (lines; no frames, which WebAssembly has no use for). |

The objects of `--emit=obj` carry the same sections (ELF targets), with relocations,
for the system's linker. Axiom functions in the same program get lines and frames but
no variables.

## Optimization

`-O0` (the default) to `-O3` are the compiler's: C goes through the same passes as Axiom
code (README of `cmd/axiom/`, "Optimization"). All the units of a program are one module,
so `-O2` and `-O3` inline across them. The levels do not change the behavior of a
program that has no undefined behavior; they differ in speed and size, and in how
complete a stack trace is after a crash.

## Known deviations

These are the places where the implementation does not follow the standard it claims, or
claims less than the standard allows. None is hidden: each is said above or here.

- The implementation is hosted (`__STDC_HOSTED__` is 1) without `<math.h>`, `<fenv.h>`
  and `<threads.h>`: see "Not supported".
- Annex F is not claimed. The floating types have IEC 60559 formats (`FLT_IS_IEC_60559`
  in `<float.h>`), but there are no exception flags or rounding modes; signaling NaNs are
  not guaranteed to stay signaling, so `<float.h>` has no `FLT_SNAN`.
- Annex G is not claimed (`__STDC_IEC_559_COMPLEX__` is not defined): it needs Annex F,
  and before C23 the imaginary types, which are absent. Complex arithmetic follows it
  all the same where it can: a real operand of `+ - * /` with a complex one is not made
  complex (G.5: `2.0 * z` is `2.0 * creal(z) + 2.0 * cimag(z) * I`, and signed zeros of
  the other part are kept), and multiplication and division recover the infinities and
  zeros G.5.1 describes when the formulas give NaN + iNaN. Division uses Smith's method
  (no overflow forming the divisor's squared magnitude); it may differ in the last bit
  from other implementations' choices. `CX_LIMITED_RANGE` has no effect (the full
  algorithms are always used).
- Decimal floating types and `<threads.h>` are absent, as the feature
  macros say.
- `restrict` has no effect on the generated code; `#pragma STDC FENV_ACCESS ON` is not
  honoured (C4113).
- Signed overflow, which is undefined, wraps in the code the compiler generates today;
  a program must not rely on it.
- An object from `--emit=obj` of units with no `main` does not link into an Axiom
  program with `--link=` (above); the C sources build with it instead. There are no
  static archives of the compiler's own objects.
- In a GNU dialect: `__int128` and the `_FloatN` types are not supported; the
  destructors run when `main` returns and from the library's `exit`, not when the
  process ends otherwise (`_Exit`, `quick_exit`, a signal); an alias is another name in its unit only; the
  attribute `section` places only data, and only in ELF objects.
