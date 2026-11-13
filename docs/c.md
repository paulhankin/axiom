# Compiling C

`axiom build file.c` compiles C. The C front end reads C translation units and hands them
to the same optimizer, code generators and linkers that Axiom programs use, so a C program
is built for every target of the compiler (WebAssembly, x86-64 and AArch64 Linux, x86-64
Windows, AArch64 macOS) with no C compiler and no external linker: the compiler's own
linkers link the platform's C library too, and on Linux the objects, archives and shared
libraries of the platform's toolchain ("Objects and libraries").

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

[`docs/c-diagnostics.md`](c-diagnostics.md) lists the diagnostics, and
[`docs/c-conformance.md`](c-conformance.md) is the clause-by-clause account against the
standards, with every implementation-defined choice of their Annex J.3. How the front end is
built is in [`docs/c-frontend.md`](c-frontend.md). C units and Axiom modules build into
one program ("C and Axiom in one program", below). Programs linked with further C
sources (`--link=file.c`), and with objects and libraries, are in "Objects and libraries",
below.

## What a C program is here

- **The library is the compiler's, as far as it goes.** The headers and functions of
  "The C library" (below) are linked into every program that uses them, on every target.
- **Other undefined functions are the platform's.** A function a program declares and
  that neither it nor the library defines is looked up outside the program:
  - On Linux, by the compiler's linker, in the system's C library (`libc.so.6`,
    `libm.so.6`, and the libraries `-l` names): the program is then linked dynamically,
    against them, with no C compiler or linker of the system's. A program that defines
    every function it calls is a static executable that needs nothing.
  - On macOS, in `libSystem`. On Windows, in a system DLL by the name's rules: a
    capitalized name is `kernel32.dll`'s (`WSA...` and the socket functions are
    `ws2_32.dll`'s, `Rtl...`, `Nt...` and `Nls...` are `ntdll.dll`'s, `BCrypt...` are
    `bcrypt.dll`'s, `ConvertStringSecurityDescriptor...` are `advapi32.dll`'s); any
    other name is an undefined symbol at link time: nothing comes from the system's C
    library (`msvcrt.dll`), unless the program's C library is the system's
    (`--c-lib=system`, "Objects and libraries"), whose names are then `msvcrt.dll`'s.
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
| `--c-asm-dialect=att\|intel` | The syntax of x86-64 inline and file-scope assembly (default `att`; "Inline assembly"). |
| `--c-warn-unknown-pragmas` | An unknown `#pragma` is the warning `C4111` (it is ignored either way). |
| `--c-warn-format=0\|1\|2` | The checks of format strings ("Attributes": `format`): 0 none, 1 (the default) what is undefined or fails on any target (C4130, C4131, C4133), 2 also what is suspect (C4132, C4134, C4135). |
| `-Idir` | Add a directory to the include path. |
| `--c-freestanding` | A freestanding program: `__STDC_HOSTED__` is 0, only the freestanding headers are found (another is `C1081`), and no C library is linked ("The C library"). |
| `--c-lib=axiom\|system` | Whose C library the program has: the compiler's (the default), or the system's, whose headers are then the hosted headers ("The system's C library", below). |
| `--c-system-include=dir` | With `--c-lib=system`: a directory of the system's headers (instead of the system's own, `/usr/include` and its target directory on Linux). |

`-D`, `-U` and `-I` apply in the order given. The C options are errors for an Axiom
source. The unknown ones are errors too: there are no `-W...`, `-std=`, `-c` or `-f...`
options; a warning is silenced only by `#pragma GCC diagnostic` ("Pragmas").

### Options of `axiom build` that apply

`--target`, `--cpu`, `--features`, `-O0`..`-O3` (see "Optimization"), `--stack-traces`
(see "Stack traces"), `--format=agent-json`, `--std=<dir>` (the standard library's
directory, where the compiler finds the C headers in `c/include/`), `-o`, `--emit`,
`--link`, `-l`, `-L`, `--subsystem`, `--icon`, `--pdb`, `--function-sections`,
`--data-sections`, `--gc-sections` (see "Code in sections"), and `-g` /
`--debug-info=` (see "Debugging"). The default
target is `wasm32`. `--link=file.c` links a further C source, compiled to an object by
the system's C compiler for Linux, by this compiler for macOS and Windows, by `clang` for
WebAssembly; `--link=` with an object, an archive or a shared library, or such a file among
the inputs (`x.o`, `libx.a`, `libx.so`), links it, and `-l` and `-L` name libraries and
the directories to find them in ("Objects and libraries").

These are errors with C sources, since they mean something only for Axiom code:
`--overflow`, `--sanitize`, `--coverage`, `--units`, `--emit-header`. An Axiom program
with C units takes `--overflow` and `--emit-header`, which apply to its Axiom code (and
the header is written before the C units are compiled, so they may include it). So are
`axiom test`, `axiom profile`, `axiom ide` and `axiom fmt` on a C source, and `axiom ide`
of an Axiom program with C units. `axiom test` and `axiom profile` take an Axiom program
or package with C units (its tests may call C; the C library's streams are flushed after
each test).

A missing source, a missing include directory (`-I`) and an output that is a directory are
errors before anything is compiled.

### `--emit`

| Value | Output |
|---|---|
| `exe` | The default: an executable (a `.wasm` module for WebAssembly targets). |
| `obj` | An object file, in the target's format: ELF for Linux, Mach-O for macOS, COFF for Windows (the tool conventions' for WebAssembly). |
| `lib` | A static library: an archive of the object, and of the objects among the inputs (`libprog.a`, `prog.lib` for Windows). |
| `shared` | A shared library of C units: `libprog.so` (ELF), `libprog.dylib` (Mach-O), `prog.dll` ("Shared libraries", below). |
| `ir` | The compiler's IR as text. |
| `c-pp` | The preprocessed text: the tokens of the translation unit, on their lines. |
| `c-tokens` | The tokens, one to a line, with their kinds and values. |
| `c-ast` | The syntax tree, as text. No semantic analysis is done. |

`c-pp`, `c-tokens` and `c-ast` take one source, and print to standard output without
`-o`. `--emit=asm` is not supported. Without `-o`, the output is named after the first
source with `.c` removed: `prog`, `prog.wasm`, `prog.o` or `prog.ir`.

An object or IR of translation units that do not define `main` holds their code alone:
there is no start-up code and no runtime (not even the runtime's helpers, such as
`axiom_shadow_stack`). Objects link with each other and into programs,
by the compiler and by the system's toolchain (`cc main.c lib.o`): "Objects of this
compiler", below.

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

### Assembly sources

```
axiom build main.c memops.S table.s -o prog
```

On x86-64 and AArch64 targets (Linux, macOS, Windows; on WebAssembly C3901) a source
whose name ends in `.s` or `.S` is a translation unit of assembly, beside C units and an
Axiom program (`axiom build main.ax x.s`), or alone (`--emit=obj`). Its text is assembled
as file-scope asm is ("Inline assembly"): AT&T syntax, or Intel under
`--c-asm-dialect=intel` and after `.intel_syntax`. A `.S` source is first preprocessed
by the C preprocessor with `__ASSEMBLER__` defined as 1, `-D`, `-U` and `-I`, the
predefined macros (`__AXIOM_ASM_X86_64__` or `__AXIOM_ASM_AARCH64__` in any mode), its
directives done and its macros replaced, its other text as written; a line whose `#`
does not begin a directive is a comment (as is `#` first on a line on AArch64, `//`
anywhere there, and `#` anywhere on x86-64). A `.s` source is not preprocessed.

- Its global symbols (`.globl`, `.weak`) are external names of the program: C and
  Axiom (`extern "C"`) call its functions and read its data; a name it defines that a
  C unit or the Axiom program defines too is an error of the build; the C library's
  functions it calls are linked for it (its references are
  the library's roots as a C unit's are). On macOS the leading underscore is added to
  every name, as for C. A name it defines keeps the C library's definition of that name
  out.
- Nothing of an assembly source is stripped: its sections are the object's whatever
  refers to them (dead-code removal does not look into assembly), and the functions and
  objects it names are kept.
- Under `-g` its code has line information: each instruction's line in the source (a
  `.S` source's lines are its own, a line of an included file or of a macro's
  expansion being the line it is used on), or the place the last `.loc file line`
  gave (`.file N "name"` names the files), for debuggers and in the PDB on Windows;
  its parts from one label symbol to the next are subprograms of that name.
  `.cfi_*` and `.seh_*` procedures describe its frames ("Inline assembly").

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
- **Structures** cross by pointer and by value: an Axiom `struct` is laid out as C lays
  out the structure with the same members (spec §5.9), so a `*Point` is a `struct point
  *`, and a `Point` parameter or result of an `extern "C"` function or an `export fn`
  is passed as the target's C ABI passes `struct point` (spec §14.5.3), both ways. The
  C front end does it: for each such function the compiler writes a C unit of wrappers
  (a C function that takes the structures by pointer and calls the one that takes them
  by value, or for an export the C function itself, which calls the Axiom function by
  pointer), which is compiled with the program, so that the structures travel exactly
  as between C units (`c/cgen_abi.ax`).
- **Data** is shared both ways: an `extern "C"` block declares C's objects (`let mut
  count: i32;`) and `export let` defines objects of the program that C code uses by
  name (spec §14.5.6); the header (`--emit-header`) declares those.
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
alone. A program linked with objects and libraries ("Objects and libraries") keeps
besides what those refer to (the compiler's linker reads them before the program is
compiled). An object (`--emit=obj`), a static or shared library (`--emit=lib`,
`--emit=shared`), and a program that the system's linker links, keep every definition
of external linkage of the program's own units and what it reaches (a static function
nothing calls, such as a header's helper, is left out); the C library's units ("The C
library") are stripped there too.

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
   program's own: a C unit, or an Axiom module's `export fn exit`). The library's `exit` runs the `atexit`
   functions, then the GNU dialects' destructors, then the standard I/O's flushing, and ends the
   process through the runtime: `std/os.exit`, which finishes the runtime
   (`ax_rt_fini`) and exits with the status. Its argument has the width of `int`
   (`--c-int`).
4. Otherwise `main`'s value goes back to the runtime's entry point, which finishes the
   runtime and returns it to the system (on Linux with the C library linked, to the C
   library's start-up, which calls its `exit`).

A program whose `main` is Axiom's does what step 3's `exit` does before the process ends
when that `main` returns (the atexit functions, the destructors, the streams flushed:
`__axc_exit_handlers`), so that its C code's output is written. A program linked with
objects that have initialization and termination arrays (the platform toolchain's, or this
compiler's objects: "Objects and libraries") runs them too: `.preinit_array` and
`.init_array` before the C units' constructors, in the order of their priorities, and
`.fini_array` backward after the destructors; their functions are called with no
arguments.

A C call of `exit` anywhere is the same function as in step 3. A freestanding C program
on Linux that calls the system C library's `exit` (it defines none) ends through the C
library, without the runtime's finishing step, which only writes a profile (`axiom
profile` does not take C programs). Under WebAssembly a hosted program ends by calling
the host's `ax_exit` (`std/internal/runtime/host.mjs`: `process.exit`, or the
embedder's `exit` option) rather than by returning from the module's `main`.

## Objects and libraries

A program (a C program, or an Axiom program, with or without C units) is linked with the
objects, static libraries and shared libraries it is given, by the compiler's own linker,
with no linker of the system's:

```
axiom build main.c util.o libparse.a -o prog          # inputs by their names
axiom build main.ax --link=fast.o -L/opt/x/lib -lx    # --link=, -l and -L
axiom build main.c --link=helper.c -o prog            # compiled to an object first (Linux: by cc)
axiom build util.c parse.c --emit=lib -o libutil.a    # a static library of this compiler's
```

An input whose name ends in `.o`, `.obj`, `.a`, `.lib`, `.so` (or `.so.N`), `.dylib`,
`.dll` or `.tbd` is linked, as with `--link=`. What a file is is read from its contents:

- **Relocatable objects** of this compiler (`--emit=obj`) and of the platform's
  toolchain for the same target, ELF on Linux. The linker applies every relocation the
  x86-64 and AArch64 psABIs give an executable's objects, at every optimization level
  and code model (`-fPIC`, `-fno-pie`, `-mcmodel=large`), and the TLS models: local-exec
  and initial-exec as they are, general- and local-dynamic and TLS descriptors (and
  AArch64's traditional general-dynamic sequence) relaxed to local-exec for the
  executable's own thread-locals and to initial-exec for a shared library's. Of COMDAT
  groups with one signature the first object's is kept. Common symbols are weak
  definitions in `.bss`. A shared library's data that position-dependent code refers to
  directly gets a copy relocation. Sections whose names are C identifiers are placed
  together, between `__start_<name>` and `__stop_<name>` when something refers to those;
  `.preinit_array`, `.init_array[.N]` and `.fini_array[.N]` are placed by priority between
  the symbols `__init_array_start` ... `__fini_array_end` and run by the start-up ("Start-up
  and exit"). `.eh_frame` is kept, relocated (there is no `.eh_frame_hdr`: C++ exceptions
  thrown through such code are not supported). `STT_GNU_IFUNC` symbols are an error.
- **Static archives** (the `ar` format of System V and GNU, of BSD and of PE/COFF, and
  thin ones): a member is linked when it defines a name that is undefined so far (a strong
  reference: a weak one takes no member), and what it needs is looked for in turn, in every
  archive given (the archives are one group: their order does not matter).
- **Shared libraries**, by path or by name: `-lname` finds `libname.so`, else `libname.a`,
  in the `-L` directories and then the system's (`/lib/x86_64-linux-gnu`, ... or the
  AArch64 ones, `/usr/aarch64-linux-gnu/lib` among them). A linker script there (as the
  system's `libc.so` is one) is read for its `INPUT` and `GROUP`, `AS_NEEDED` included;
  another command is an error. The program is linked dynamically against the libraries
  it uses (`DT_NEEDED`), with the symbol versions they define, eagerly bound; the dynamic
  linker's own symbols (`__tls_get_addr`, AArch64's `__stack_chk_guard`) are found too.
- **C sources** given with `--link=` are compiled, after `--emit-header`'s header is
  written (which they may include), for Linux by the system's C compiler (`cc`, or the
  cross compiler for the target: `aarch64-linux-gnu-gcc`) with `-O2` and the `-I`
  directories: code of the platform's toolchain, which is what such a source is for (a
  library's helper, a test's oracle); for macOS and Windows, which have no C compiler
  here, by this compiler, in a process of its own, into an object in the target's format
  (`--emit=obj`: Mach-O, COFF), with `-O2`, the build's C options (`--c-std`, `--c-lib`,
  `-D`, `-U`, the `-I` directories, ...), `--cpu` and `--features`.

When objects of the platform's toolchain are linked, so are its C compiler's run-time
archives (`libgcc.a` and `libgcc_eh.a` of its newest version, under `/usr/lib/gcc` or
`/usr/lib/gcc-cross`), from which a member is linked only where nothing else defines
what an object needs (complex arithmetic, 128-bit division and conversions, AArch64's
atomics called out of line), as the system's linker links them.

What the inputs refer to and do not define, the program does: the C library ("The C
library") supplies it, its units and run-time modules compiled with the program as for
the program's own references, and what the inputs refer to is kept ("What is linked").

On WebAssembly the compiler's linker takes relocatable objects of the tool conventions'
format (this compiler's, and `clang`'s for the same target) and archives of them, found
with `-L` and `-l` (`libname.a`), with the same member selection; a C source given with
`--link=` is compiled by `clang` first. The constructors of such objects (their
linking sections' `INIT_FUNCS`) are run before `main`, by priority (of one priority in
the order of the objects), by `__wasm_call_ctors`, which the linker makes when an
object has some (the tool conventions' name: the C start-up calls it). This compiler's
WebAssembly objects list their C units' constructors so, and their destructors as
constructors of their priorities that register them with `atexit` (which runs them in
the reverse order: after `main`, by `exit`), so that `wasm-ld` runs them too. On macOS (`mlink.ax`) and Windows (`plink.ax`) the compiler's linkers take the objects and
static archives of the platform's toolchain and of this compiler, in the platform's
format, for both processors (`macho.ax`, `coff.ax` read them), with the same member
selection; what the objects refer to and do not define comes from the libraries:

- **Mach-O objects** (arm64, x86-64): every relocation type of both (`ARM64_RELOC_*`
  with `ARM64_RELOC_ADDEND`, `X86_64_RELOC_*`, `SUBTRACTOR` pairs to a symbol of the same
  section, section-relative ones), common symbols, weak definitions and references,
  `__mod_init_func` and `__mod_term_func` (run by the start-up as the initialization
  arrays are), and the platform's thread-local variables (`__thread_vars`,
  `__thread_data`, `__thread_bss`: the executable gets the sections and dyld makes the
  blocks). `__compact_unwind` and `__eh_frame` are not kept (the executable has no
  unwind tables of other compilers' code), nor is a universal object read (give the slice).
- **Dynamic libraries**: a `.dylib` given (or found by `-l` in the `-L` directories; of a
  universal file, the target's slice) is read for its install name and exports, and the
  program is bound to it by its ordinal (two-level namespace). A library named with `-l`
  that is not a file here (`-lz`, the SDK's `.tbd` stubs, which are not read) is loaded by
  name (`/usr/lib/libz.dylib`, a framework's path, ...), and what no library is known to
  export is then looked up in every library loaded (flat lookup); without such libraries
  it is `libSystem`'s.
- **COFF objects** (x64, ARM64; the big format too): every relocation type of both
  (`IMAGE_REL_AMD64_*`, `IMAGE_REL_ARM64_*`, with their implicit addends), COMDAT sections
  (one copy of a symbol's section is kept by its selection, its associative sections
  with it: "Code in sections"), weak externals (a weak definition where the
  default is defined, else a weak reference), common symbols, grouped sections
  (`name$suffix`) by their names, `.CRT$XC*`/`.CRT$XI*` as initialization arrays and
  `.CRT$XT*`/`.CRT$XP*` as termination ones in their suffixes' order, which the
  start-up runs forward as the C runtime does (the C runtime's bounds dropped), `.tls$` as the thread-local template (`IMAGE_REL_*_SECREL` offsets in
  it, `_tls_index` the linker's). Not kept: `.drectve`, `.debug$S`, `.debug$T`, and the
  sections the linker is to remove. The linker supplies `__chkstk` (both processors'
  stack probes), `_fltused` and `__ImageBase`.
- **Import libraries** (their short import records: an archive member that names a DLL's
  function or object, by name, undecorated name or ordinal) and **DLLs** given (or found by
  `-l` as `name.dll`: their export tables): the program imports the functions from the
  DLL they name, `__imp_<name>` being the import address table's slot. A long-format
  import library (whose members define `__imp_` names in `.idata` sections) is an error.

### One C library

A program has one C library: the compiler's (`--c-lib=axiom`, the default), or the
system's (`--c-lib=system`).

**The compiler's.** The code compiled by the system's C compiler (its objects and
libraries), against the system's headers, uses this library: its calls of the standard's functions are this library's
functions, whose interfaces the C standard fixes, and the names the system's headers
expand to that the Linux Standard Base specifies are this library's too
(`std/c/lib/compat/`): `stdin`, `stdout` and `stderr` as objects of type `FILE *`;
`errno`'s `__errno_location`; the tables of the `<ctype.h>` macros (`__ctype_b_loc`,
`__ctype_tolower_loc`, `__ctype_toupper_loc`, the "C" locale's classes with the LSB's
bits); `__assert_fail`; the fortified functions of `_FORTIFY_SOURCE` (`__printf_chk`,
`__sprintf_chk`, `__snprintf_chk`, `__vfprintf_chk`, `__memcpy_chk`, `__strcpy_chk`, ...,
which fail through `__chk_fail` as they must); `__isoc99_scanf` and its kin, and C23's
`__isoc23_strtol`, `__isoc23_scanf` and theirs; `__stack_chk_fail`, AArch64's
`__stack_chk_guard` (on x86-64 the guard is at `%fs:0x28`, which the runtime's thread
control blocks keep zero); `_setjmp` and `__sigsetjmp` (this library's `setjmp`: its
`jmp_buf` is smaller than the system's), `__longjmp_chk`; `_IO_putc`, `_IO_getc`;
`__getauxval`. The limit is what such code took from the system library's headers that
this library does not share: a reference to another of that library's own names (those
reserved to the implementation: `__overflow`, `__uflow`, ...) or to one of POSIX's
functions of streams (`fileno`, `fdopen`, `popen`, `getline`, ...: they take or give the
system library's `FILE`) is an error (`E9004`, after `link error: x.o refers to
'__overflow', a name of the system's C library's own implementation` or `x.o calls
'fileno', which works on the system's C library's streams`), as is anything else that
would give the program two C libraries, and so is a call of a function whose objects the
system's headers lay out smaller than this library does (`fgetpos` and `fsetpos`, whose
`fpos_t` is 16 bytes there and 40 here; the restartable multibyte functions, `mbrtowc`
and its kin, whose `mbstate_t` is 8 bytes there and 24 here). Macros the code expanded
(`BUFSIZ`, `EOF`, the `errno` codes, which are Linux's here too) are its values, and a
structure it reads that this library fills must be laid out alike: this library's
`struct tm` has the standard's members, at the system's offsets, without the system's
`tm_gmtoff` and `tm_zone` after them. The other POSIX functions (`write`, `getpid`,
`fork`, ...) are the system library's, and `errno` is one object for both ("How the
library is linked").

**The system's** (`--c-lib=system`, native targets). The compiler's library is left out:
the C units' and the objects' calls are the system library's, through its shared
objects (`libc.so.6`, `libm.so.6`). The hosted headers are the system's: `#include <x>`
looks in the `-I` directories, then in the system's (the `--c-system-include`
directories, else `/usr/include/<target>` and `/usr/include`, or for another processor
than the machine's `/usr/<target>/include`), then in the compiler's for the headers of
the language itself, which are the compiler's whatever the order (`<float.h>`,
`<stdarg.h>`, `<stddef.h>`, `<stdbool.h>`, `<stdalign.h>`, `<stdnoreturn.h>`,
`<iso646.h>`, `<stdatomic.h>`, `<stdbit.h>`, `<stdckdint.h>`, `<complex.h>`,
`<tgmath.h>`; the system's `<limits.h>` and `<stdint.h>` are its, with their POSIX
limits). Those headers are compiled as the system's: the macros of the platform they
test are defined (`__x86_64__` or `__aarch64__`, `__linux__`, `__unix__`, `__ELF__`,
`__LP64__`, `__SIZEOF_<T>__`, `__BYTE_ORDER__`, `__CHAR_BIT__`, and
`__AXIOM_SYSTEM_C_LIB__`), the definitions they make of the implementation's own macros
(`__STDC_ISO_10646__`, ...) keep the compiler's, their warnings and the errors that say a
feature is a later standard's (or an empty declaration) are not shown, and in a GNU
dialect `_FloatN` and `__float128` are not keywords there (they define them as types).
`__GNUC__` is not defined, so they use what they have for other compilers. The standard
must be C99 or later, the C options the platform's ABI's (`--c-int`, `--c-long`), and
`--c-freestanding` is not given. `setjmp` and the other functions POSIX says return twice
(`_setjmp`, `sigsetjmp`, `vfork`, `getcontext`) make their callers return twice. The
program's constructors and destructors run as they do with this library; returning from
`main` ends the program through the system library's `exit`.
On macOS and Windows, whose system headers are not on a Linux machine, the
`--c-system-include` directories name them; the functions are then `libSystem`'s or
`msvcrt.dll`'s, by name. On every target the program's `setjmp` and `longjmp` are the
system library's (the compiler's, whose `jmp_buf` is another, are left out:
`cmd/axiom/src/jstub.ax`).

### Objects of this compiler

`--emit=obj` makes an object that links with other objects of this compiler, with those of
the platform's toolchain and into programs, whoever links it:

- Whatever several objects may each have, they share. The state of the program (its
  modules' mutable globals, the runtime's shadow stack and tables, the standard library's
  allocator) is link-once: weak global symbols named after the declarations, so two
  objects that use the standard library's allocator use one heap; the runtime's
  functions, its helpers (`axiom_shadow_stack`, `axiom_frame_table`, ...) and the system
  call stubs are link-once too, one copy of each linked. A module's own functions stay the
  object's (their copies may differ in what they inlined), and its constants too; the
  program's `export fn`s are its strong definitions.
- C units' constructors and destructors are entries of the object's `.init_array` and
  `.fini_array` (in `.init_array.NNNNN` for a priority), which the program that links it
  runs, the compiler's start-up or the platform's.

`--emit=lib` puts the object, and the objects among the inputs, in a static library with
a symbol table of what each defines (as the objects spell the names: with the Mach-O
underscore), laid out as the target's toolchain lays out its own (System V's for Linux
and WebAssembly, BSD's `__.SYMDEF SORTED` for macOS, PE/COFF's two linker members for
Windows).

The objects are in the target's format (`--emit=obj`, `--emit=lib`):

- **Mach-O** (`macho.ax`): one segment of the sections (`__TEXT,__text`, `__TEXT,__const`,
  `__DATA,__const` for read-only data with addresses, `__DATA,__data`, `__DATA,__bss`,
  `__DATA,__mod_init_func` and `__mod_term_func` with the entries in the order of their
  priorities, a section attribute's `"segment,section"` or a C identifier's `__DATA,<name>`,
  the debug information in `__DWARF`), `MH_SUBSECTIONS_VIA_SYMBOLS` (each function and object
  an atom; a reference into an atom is to its symbol plus an offset), `LC_BUILD_VERSION`
  (macOS 11.0), the names of C with the underscore, `__start_X`/`__stop_X` as the system
  linker's `section$start$__DATA$X` and `section$end$__DATA$X`, hidden symbols private
  externs (`N_PEXT`), weak definitions `N_WEAK_DEF`, weak references `N_WEAK_REF`. The
  thread-locals of this compiler (an offset in the block the runtime makes: they are not
  the platform's thread-local variables) are in `__DATA,__axiom_tdata` and
  `__DATA,__axiom_tbss`, and their offsets' relocations have type numbers Mach-O leaves
  unused (arm64 14 and 15, x86-64 15), which only this compiler's linker reads: the
  system's refuses an object that uses them.
- **COFF** (`coff.ax`): `.text`, `.rdata`, `.data`, `.bss`, `.tls$` (the thread-local
  template, the zero-filled part as zeros), `.pdata` and `.xdata`, `.CRT$XCU` (`.CRT$XCT<NNNNN>`
  for a priority) and `.CRT$XTT<65535 - priority>` (`.CRT$XTT00000` without one: the C
  runtime runs the tables forward, in the order of the names, so the destructors run in
  the reverse order of their priorities, the plain ones first, under the system's
  linker as under the compiler's), a C identifier's section as
  `X$m` with its bounds' symbols in `X$a` and `X$z` (selected once), `.axiom.lines` and
  `.axiom.cv` (the line tables and CodeView records for the program database,
  `IMAGE_SCN_LNK_REMOVE`). A weak definition is a
  weak external whose default, `.weak.<name>.default.<tag>`, is the definition; a weak
  reference's default is an absolute 0. A load through the GOT reads the object's
  `.refptr.<name>` (a pointer in `.rdata$.refptr.<name>`, selected once). Functions carry
  their sizes (function definition records). No `.drectve` is written.

The system's linkers read these (`ld64.lld`, `lld-link`, the platforms' own: the
suite's `c/objfmt` tests link them). An object of C units alone has no runtime: linked
by the system's toolchain into a C program (whose C library is the system's), it may use
only the standard's functions and objects; one that uses this library's private names
(the macros of `<stdio.h>` for `stdin`, `stdout`, `stderr`, `getc` and `putc`, `errno`'s,
... `__axc_*` names) needs this library, and on macOS one with thread-locals needs this
compiler's linker (above). Objects with Axiom modules carry the runtime, which this
compiler's linkers link.

### Shared libraries

`--emit=shared` links C units (with `--c-lib=system` or `--c-freestanding`), and the
objects and libraries given, into a shared library; the compiler's C library and the
Axiom runtime are an executable's (one per process: its allocator, threads and
thread-local blocks), so Axiom modules and the default C library are an error (`E9001`).

- **ELF** (`libprog.so`; `elink.ax`): position-independent (`--pic`, which
  `--emit=shared` implies: thread-locals in the initial-exec model, their offsets in GOT
  slots that the dynamic linker fills; the rest of the code generators' code is
  position-independent always), its definitions of default visibility exported (`.dynsym`,
  a hash table), its own references bound to its own definitions (as protected symbols
  are: within the library nothing is preempted), every address it holds relocated where
  it is loaded (`R_*_RELATIVE`), `DT_SONAME` its file name, `DT_INIT_ARRAY` and
  `DT_FINI_ARRAY` its constructors and destructors, and `DT_NEEDED` the libraries it uses.
  Code that is not position-independent (an absolute address in code or read-only data,
  narrower than 64 bits, the local-exec model, a library's object reached without the GOT)
  is an error that names its object. Programs of this compiler link with it (`-L. -lprog`),
  and the system's `dlopen` loads it.
- **DLL** (`prog.dll`; `plink.ax`): its export directory names the definitions of default
  visibility, by name; its entry point (the linker's) runs the constructors when a process
  loads it and the destructors when it is unloaded, then calls the program's `DllMain` if
  it defines one (else it says TRUE); base relocations and the TLS directory as for an
  executable. Programs link with the DLL itself (it is read for its exports; no import
  library is written), and `LoadLibrary` and `GetProcAddress` find its functions.
- **Mach-O** (`libprog.dylib`; `mlink.ax`): `MH_DYLIB`, its install name
  `@rpath/libprog.dylib`, an export trie of its definitions of default visibility,
  `__mod_init_func` and `__mod_term_func` (dyld runs them), rebase and bind information as
  for an executable, ad-hoc signed. Thread-locals of this compiler are an error there
  (the runtime that makes their blocks is an executable's). Built and checked only: nothing
  here runs macOS.

### Code in sections

A function's code goes in a section of the object as a data object's does:

- The attribute `section` names it (a GNU dialect): every function naming one section is
  in it, between `__start_<name>` and `__stop_<name>` when the name is a C identifier and
  something refers to those. Mach-O takes `"segment,section"` (and a C identifier as
  `__TEXT,<name>`); COFF names a C identifier's section `<name>$m`.
- A cold function (the attribute `cold`, and the paths the optimizer moves out of a
  function into one of their own because they are seldom run) is in `.text.unlikely`, a
  hot one (`hot`) in `.text.hot`; the compiler's linkers place those after and before
  the rest of the code. (Mach-O and COFF have one section of code: there they are
  `__text` and `.text`.)
- `--function-sections` puts each function in a section of its own (`.text.<name>`,
  `.text.unlikely.<name>`, ...), `--data-sections` each datum (`.data.<name>`,
  `.bss.<name>`, `.rodata.<name>`, `.tdata.<name>`, ...). In COFF such a function's
  section is a COMDAT section with the selection "no duplicates", so that the system's
  linker may leave it out (`/OPT:REF`); a Mach-O object's functions and data are each
  an atom anyway.
- An object for other links (`--emit=obj`, `--emit=lib`, `--emit=shared`) has each
  link-once definition (the runtime's helpers, the C units' helpers of atomics wider
  than the processor's and of 128-bit division) in a section of its own in a COMDAT
  group named after it (ELF `SHT_GROUP`; COFF a COMDAT section of the selection "any"),
  with what only that code refers to: its jump tables, its frame table and (Windows)
  its unwind information (in COFF associative to it). A link keeps one group of a name,
  whichever linker it is. In Mach-O it is a weak definition.

What describes code follows it into its section: the frame tables (one `axiom_rets`
section per section of code, `SHF_LINK_ORDER` to it; the compiler's linkers sort the
whole table by address, and a system linker places them in their code's order), the
DWARF (a unit whose code is in several sections has `DW_AT_ranges`, and
`.debug_aranges` an entry per section), the line tables of the Windows program database
(keyed by symbol, not by section number) and the Windows unwind information (`.pdata`
and `.xdata` that go with their code's section).

`--gc-sections` makes the compiler's linkers leave out the sections nothing they keep
refers to (the system's linkers have their own: `-Wl,--gc-sections`, `/OPT:REF`,
`-dead_strip`). Kept are the sections that define what the linker itself needs
(`main`, the start-up, the runtime's thread-local records), a shared library's or a DLL's
exports, the initialization and termination arrays, sections named as C identifiers,
sections of `SHF_GNU_RETAIN`, and everything that is not code or data; then whatever a
relocation in a kept section refers to, the sections that go with a kept one
(`SHF_LINK_ORDER`), and the other sections of a kept one's COMDAT group. The unwind
tables (`.eh_frame`, `.pdata`, `.xdata`) of other compilers' objects are kept whole, but
keep nothing themselves. A program's own C units and Axiom modules are compiled as one
unit, whose unreferenced functions are not made at all: `--gc-sections` matters for the
objects and archives a program links.

Of a COFF object's COMDAT sections a link keeps one copy by its selection: "any" the
first object's, "largest" the largest, "same size" and "exact match" the first, the
others then having to be of its size or its bytes, and a second copy of one of "no
duplicates" is an error (`duplicate symbol`).

### Not done yet

- On WebAssembly a weak reference to a function nothing defines, while null, is still an
  import of `env` for its calls.
- `visibility` is written to ELF objects
  as `hidden` (hidden, internal) or default (protected, default: a shared library binds
  its references to its own definitions, as protected ones are), to Mach-O objects as
  private externs, and a hidden symbol is not a DLL's or a shared library's export.
- The DWARF of Mach-O objects given to a link is not carried into the dSYM correctly when
  several objects have some (their offsets into each other's debug sections are numbers,
  the platform's convention, which the linker would have to rebase by parsing them).
- `.eh_frame` is kept without `.eh_frame_hdr` and `PT_GNU_EH_FRAME`.

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
| mathematics | `<math.h>`, `<fenv.h>`, `<tgmath.h>`, `<complex.h>`'s functions | `std/c/lib/math/`, `std/c/rt/math*.ax` (below) |
| threads | `<threads.h>`, and C23's `call_once` of `<stdlib.h>` | `std/c/lib/threads/`, `std/c/rt/threads.ax`, `std/c/rt/threads_spawn.ax` (`threads_spawn_none.ax`), `axc/threads_impl.h`, `axc/once.h` ("Threads", below) |
| bounds-checking interfaces | Annex K: the `_s` functions of `<stdio.h>`, `<stdlib.h>`, `<string.h>`, `<time.h>` and `<wchar.h>`, `errno_t`, `rsize_t`, `RSIZE_MAX` | `std/c/lib/ext1/`, `axc/ext1.h`, `axc/ext1_impl.h` ("Annex K", below) |

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
  of them, but for the creation of threads (`threads_spawn.ax`, whose
  `std/thread` gives a WebAssembly program a shared memory), which it has only
  if its C sources name `thrd_create`, `thrd_exit`, `thrd_join` or
  `thrd_detach`. `std/c/rt/ldouble.ax` (the arithmetic of a `long double` wider than `double`)
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
  integers cross instead). A run-time module that imports another brings that
  module's C functions too (the index lists them under it). A run-time module
  `X_none.ax` stands in for `X.ax` where the target cannot have threads
  (WebAssembly without the atomics extension): the index's `A` lines.
- What a program does not reach is not in its executable ("What is linked"). A program
  that returns 3 from `main` builds within about 20 ms of a freestanding one (x86-64
  Linux) and is a few KB larger (the exit path).
- `exit` runs the `atexit` functions (last registered first), then the GNU dialects'
  destructors (highest priority first), then flushes and closes the streams and removes
  `tmpfile`'s files; the constructors run before `main` (lowest priority first), also
  in a program whose `main` is Axiom's, where the destructors run when it returns.

Under Linux a program that still calls a function of the system's C library (one this
library does not have, such as POSIX's `fork`) is linked with it, as before,
and the library's functions (`exit`, `qsort`, ...) are the ones the program uses. Its
`malloc` and the others of `<stdlib.h>`'s memory management then use the system
library's heap (`__libc_malloc` and its kin), as they always do on macOS
(`libSystem`'s), so that memory one library allocates the other frees; in a program that
needs nothing of the system's (a static one on Linux; Windows; WebAssembly) they use the
Axiom allocator. `errno` is then that library's too (its `__errno_location`, on macOS
`__error`: `std/c/rt/core_errno.ax`), so that both libraries' functions set one object.
Code compiled by the system's C compiler (its objects and libraries) uses this library,
the program's: "Objects and libraries" says how. On
Windows nothing comes from `msvcrt.dll` (the suite checks every Windows executable's
imports) unless `--c-lib=system` makes it the program's library. On macOS the runtime reaches
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
| `<stdlib.h>` | All of it (the multibyte functions and `MB_CUR_MAX` are the text part's). Its header comment lists the choices: `malloc(0)` and the like give a unique pointer; memory is the Axiom allocator's (`std/mem`, thread-safe) or the system C library's (above), aligned to 16; `aligned_alloc` takes any power of two as the alignment and any size, a multiple of it or not, in every mode (C11 left another size undefined; C17's DR 460 and C23 take any), and fails (a null pointer, `EINVAL`) for another alignment; `realloc(p, 0)` (implementation-defined in C17, undefined in C23) frees p's object and returns a unique pointer to an object of no bytes, which `free` takes; `calloc` checks its product; `rand` is a 64-bit linear congruential generator (`RAND_MAX` 2147483647: the state's high 31 bits, so the results' low bits are as good as their high ones; the period is 2^64; successive values have a linear congruential generator's lattice structure, so `rand` is no source for cryptography or for simulations that need independent values; one state for every thread, which may race, 7.24.2.1p3); `qsort` is an introspective sort (O(n log n) at worst, O(log n) stack), which a comparison function that is not consistent leaves in an unspecified order without touching anything outside the array; `qsort` and `bsearch` with no elements call no comparison function, null pointers included; `strtod`, `strtof` and `strtold` (in `long double`'s format, whichever the target has) are correctly rounded for every input, decimal and hexadecimal, in the current rounding direction, and set `ERANGE` on overflow and on a zero or subnormal result; their decimal point is `.` in every locale (each has the "C" locale's numeric conventions); in C23 mode `strtol` and its kin take `0b`; `atexit` and `at_quick_exit` take any number of functions, from any thread (their table has a lock); `getenv` is free of data races with other calls of `getenv` (its strings are kept under a lock and stay valid until the variable's value is asked for again after it changed), but not with a change of the environment from another thread (POSIX's `setenv`, `std/os.setenv`), which C23 7.24.4.6p2 allows; `system` uses `/bin/sh -c` (Linux, macOS) or `%ComSpec% /s /c "command"` (Windows, else `cmd.exe`), which receive the command's text as it is, and returns POSIX's encoding of the status (the exit status times 256, or the signal's number) or Windows' exit status; under WebAssembly there is no command processor (`system(NULL)` is 0, a command -1); `strfromd`, `strfromf` and `strfroml` (C23) format with `snprintf`, the standard I/O's. |
| `<string.h>` | The hosted functions: `strerror` (the library's messages: one of its own for each code of `<errno.h>`, which are those the library sets, and `Unknown error` for another number), `strcoll` and `strxfrm` (the order of the bytes, in every locale), and in C23 `strdup` and `strndup`; in C89 mode too (the headers test `__AXIOM_HOSTED__`). |

Measured on x86-64 Linux against the system's C library (both at their best options):
`qsort` of integers and of 24-byte records 1.1 times its time, `strtod` 1.0 times,
`malloc` and `free` of small blocks 0.96 times; `fgets` of 1.5 million short lines 0.8
times, `fscanf("%d")` 0.9 times, `getc` and `putc` 0.5 times (their macros).

### Annex K: the bounds-checking interfaces

All of Annex K (C11 K.3, unchanged in C17 and C23), in every hosted program;
`__STDC_LIB_EXT1__` is `201112L` from C11 on (not with `--c-freestanding`).

- **Asking for them** (K.3.1.1): a program defines `__STDC_WANT_LIB_EXT1__` as 1
  before it includes a header to have the header declare its Annex K names
  (`errno_t` in `<errno.h>`, `rsize_t` in `<stddef.h>`, `RSIZE_MAX` in
  `<stdint.h>`, and the functions, types and macros of K.3.5 to K.3.9); as 0, or
  not at all (the implementation's choice), the names are not declared and stay the
  program's. Another value is an error, and so is an inclusion of one of these
  headers that sees the macro otherwise than the first did (K.3.1.1p4: `C1040`, as
  `#error` gives). They are declared in every mode, C89 included, when asked for.
- **Types and limits**: `errno_t` is `int`, `rsize_t` is `size_t`, `RSIZE_MAX` is
  `SIZE_MAX >> 1` (K.3.4p4's recommendation); `L_tmpnam_s` and `TMP_MAX_S` are
  `L_tmpnam` and `TMP_MAX`.
- **Runtime-constraint violations** call the handler (K.3.1.4): the one
  `set_constraint_handler_s` registered (atomically, from any thread), or the
  default, `abort_handler_s` (K.3.6.1.1p3's choice), which writes
  `runtime-constraint violation: <msg>` to standard error and calls `abort`. The
  message is `function: what was wrong` (`strcpy_s: s2 does not fit in s1max
  characters`); `ptr` is a null pointer; `error` is the function's result where it
  returns an `errno_t` (`EINVAL` for a null pointer, `ERANGE` for a size out of
  range or too small, `EINVAL` for overlapping objects), else `EINVAL`. When the
  handler returns (`ignore_handler_s`, or the program's), each function does what
  K.3 says it does after a violation (stores a null character, zeros, sets
  `*retval` to `(size_t)-1` ...) and returns its failure. Overlap is checked on
  the bytes each copy reads and writes.
- **Formatted I/O**: `%n` in the format, and a null pointer for `%s` or `%ls`, are
  violations of `fprintf_s` and its kin (the output stops there). In `fscanf_s` and
  its kin, `%c`, `%s` and `%[` take the receiving array's number of elements (an
  `rsize_t`) after its pointer (not when assignment is suppressed); an input item
  that does not fit, with `%s`'s and `%[`'s null character, is a matching failure
  after which the array holds the empty string; a null pointer to store through is
  a violation (`EOF`). In C23 mode `%i` takes `0b` as `fscanf`'s does. `sprintf_s`
  returns 0 after a violation (an output that does not fit included), `swprintf_s`
  a negative value for an output that does not fit (K.3.9.1.4p4), the `snprintf_s`
  forms a negative value.
- **Files**: `fopen_s` and `freopen_s` take a mode that may begin with `u`. A file
  opened for writing is opened for exclusive access where the system has the
  notion (Windows: no other handle may open it meanwhile); a file they create is
  private to its owner (permissions 0600 on Linux and macOS, a protected access
  control list that admits only the owner on Windows; on WebAssembly the node
  host's 0600) unless the mode begins with `u`, which gives the system's default
  permissions. `tmpfile_s` is `tmpfile` (which is exclusive and private too).
- **Other choices**: `getenv_s` returns `ENOENT` for a name that is not set and
  `ERANGE` for a value that does not fit; `wctomb_s` keeps its own conversion state
  (and returns `EILSEQ`, `*status` -1, for a wide character that has no multibyte
  form); `qsort_s` is `qsort`'s introspective sort; `tmpnam_s` returns `EEXIST`
  when no name can be made; `strerror_s` cuts a long message with `...`
  (K.3.7.4.2p4); `asctime_s` takes the years 0 to 9999 and members in their normal
  ranges (`tm_sec` up to 60).

### Wide characters, locales and time

See [`docs/c-text.md`](c-text.md). `__STDC_ISO_10646__` is `202309L` (Unicode 15.1)
where `wchar_t` is 32 bits; with Windows' 16-bit `wchar_t` (UTF-16) it is not defined.

### `long double`

`strtold`, `wcstold`, `strfroml`, and `fprintf`'s and `fscanf`'s `L` conversions are
exact (correctly rounded) in the format `long double` has: binary64, the x87 80-bit
format or binary128 (`LDBL_MANT_DIG` 53, 64 or 113, "Floating types"), read from and
written to the value's bits by the standard I/O's engines (`fmtfloat.c`, `scanfloat.c`).

### Decimal floating types

C23's `_Decimal32`, `_Decimal64` and `_Decimal128` (`__STDC_IEC_60559_DFP__` is
`202311L` in C23 mode) are IEEE 754-2019's decimal32, decimal64 and decimal128 formats,
encoded in BID (binary integer decimal), the encoding the C ABIs of x86-64 and AArch64
specify: 4, 8 and 16 bytes, aligned to their sizes. Their arithmetic is software of the
compiler's (`std/c/rt/decimal.ax`), the same on every target (`_Decimal32`'s and
`_Decimal64`'s `+` and `*` in 64-bit integers where the exact result's coefficient fits
128 bits and its exponent is far from the format's limits):

- **Constants** (`1.20df`, `1.2E+3dd`, `5.dl`; C23 only, not hexadecimal: `C1019`) are
  rounded once to the type (to nearest), with the quantum they are written with:
  `1.20dd` has the coefficient 120 and the exponent -2. Constant expressions and static
  initializers are folded exactly by the compiler, with the run time's operations.
- **Operations** (`+ - * /`, the comparisons, `++`, `--`, compound assignment, the
  conversions) are correctly rounded in the thread's decimal rounding direction
  (`fe_dec_setround`, `<fenv.h>`), with IEEE 754's quanta: an exact result is the member
  of its cohort with the operation's preferred exponent (the lesser of the operands'
  for `+` and `-`, their sum for `*`, their difference for `/`), an inexact one has the
  format's precision. Subnormal results, overflow (to infinity or the largest value, as
  the direction says), NaN payloads and signaling NaNs are IEEE 754's; `<` `<=` `>` `>=`
  are signaling ("invalid" for a quiet NaN), `==` and `!=` quiet. The exceptions are
  raised in `<fenv.h>`'s flags where the target has flags (x86-64, AArch64): the
  software keeps them per thread and `fetestexcept` and the other functions see them
  with the hardware's.
- **Conversions**: from an integer of any width (`__int128`, `_BitInt(N)` too) exact or
  rounded, with exponent 0; to an integer truncated toward zero, saturated with
  "invalid" out of range (undefined, C23 6.3.1.4); to and from `float`, `double` and
  `long double` (every format: binary64, the x87's, binary128) correctly rounded both
  ways (to a decimal type in the decimal rounding direction, the exact value's exponent
  the one nearest 0; to a binary type to nearest), and between the decimal types. A
  decimal operand does not mix with a binary floating (`_FloatN` too), complex or imaginary one in the usual
  arithmetic conversions (`C3230`, C23 6.3.1.8p1); a cast or an assignment converts
  between them (to a complex type: the real part). `_Generic`, `_Atomic` (lock-free for 4
  and 8 bytes; `+=` and the others a compare-exchange loop), `constexpr` (the value
  representable exactly) and bit-exact layout work as for the other types.
- **The library**: `<math.h>`'s functions with the suffixes `d32`, `d64` and `d128`
  (C23 7.12): every one C23 lists, among them the quantum functions (`quantized64`,
  `samequantumd64`, `quantumd64`, `llquantexpd64`), the encodings (`encodedecd64` and
  `decodedecd64`: densely packed decimal; `encodebind64`, `decodebind64`), total order,
  payloads, `fromfp` and its kin, the narrowing operations (`d32addd64` ...),
  `canonicalize`; `<tgmath.h>` chooses them for decimal arguments (C23 7.27p3); the
  classification and comparison macros take them. The exact functions (rounding,
  `fmod`, `remainder`, `fma`, `sqrt`, `scalbn`, `frexp`, `nextafter`, ...) are correctly
  rounded; the elementary ones (`exp`, `log`, `pow`, `sin`, `atan2`, `erf`, `tgamma`, ...)
  compute in decimal arithmetic of the format's precision and 12 more digits (and as
  many as cancellation needs; trigonometric arguments reduced exactly, up to 10^6145)
  and round once: within 1 ulp (measured against mpmath, a few hundred arguments of
  every magnitude per function and format: the largest error 0.500 ulp). An exact result has the quantum C23 prefers (`sqrtd64(1.44dd)` is `1.2`,
  `expd64(0)` is `1`, `powd64(2.0dd, 10)` is `1024.0000000000`, `atan2pid64(1, -1)` is
  `0.75`, `exp2d64(-3.0dd)` `0.125`, `tanpid64(0.25dd)` `1`, `powd64(2.25dd, 0.5dd)` `1.5`,
  `compoundnd64(0.1dd, 2)` `1.21`: the exact values of the `pi` functions, `exp2`,
  `exp10` and their `m1` forms of integers, `pow` of an exact square root and
  `compoundn`'s exact powers are found); errors set `errno` as
  for the binary functions (`math_errhandling`). `strtod32`, `strtod64`, `strtod128`
  and the `wcstod` forms (and `fscanf`'s decimal conversions: a NaN's n-char-sequence,
  read as an unsigned integer as `nan` reads it, is its payload when that is canonical)
  are correctly rounded in the decimal rounding direction, with
  the quantum as written (`ERANGE` on overflow and underflow); `fprintf` and `fscanf`
  take the length modifiers `H`, `D` and `DD` with `a e f g` and their upper case: `%Da`
  writes the coefficient and the quantum exactly (`1.20`, `1.2e+03`: style f when
  -(n + 5) <= q <= 0, else e; a precision rounds the coefficient), `e f g` round the
  value's digits as for `double`, both in the decimal rounding direction
  (`fe_dec_setround`, which has the halfway case away from zero too, not the binary
  one); `strfromd32` ... format as `snprintf` does.
  `<float.h>` has the `DEC32_` ... macros; `<fenv.h>` `FE_DEC_DOWNWARD`,
  `FE_DEC_TONEAREST`, `FE_DEC_TONEARESTFROMZERO`, `FE_DEC_TOWARDZERO`, `FE_DEC_UPWARD`,
  `fe_dec_getround` and `fe_dec_setround`, and `fenv_t` and `femode_t` hold the decimal
  direction too.
- **Speed** (x86-64, `-O2`): an operation is a call of the software, about 200 ns for a
  `_Decimal64` multiplication or addition, 600 ns for a `_Decimal128` division; an
  elementary function tens of microseconds (`expd64` about 50 us), a trigonometric one
  of an argument beyond 10^17 milliseconds (the exact reduction). Speed was not this
  implementation's aim.

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
  on Windows `TMP`, `TEMP`, ...) exclusively (no file of the name may exist), private
  to its owner (permissions 0600; on Windows an owner-only access control list and
  exclusive access), and removes it at once (on Windows when it is closed or the
  program exits); `tmpnam` gives names there that no file has: `axc-` and 12 random
  letters and digits (about 62 bits from the system's random source), which another
  program cannot guess, though it may create a file of the name between the call and
  its use (`tmpfile` and `fopen`'s `x` exclude that). `rename` replaces a file of the
  new name, on every target (7.21.4.2p2 leaves it to the implementation), and an
  empty directory on Linux, macOS and Windows. `remove` removes an empty directory
  too. On WebAssembly the files are the host's: the node host's are node's own, or
  with `--root=dir` those under `dir`, which the program sees as `/` (no path, `..` or
  symbolic link leads out of it), or with `--files=memory` a file system in memory
  shared by the program's threads (`std/internal/runtime/memfs.mjs`, which needs
  nothing of node's: a browser's host can use it as it is).
- **`freopen` with a null file name** (7.21.5.4p3) changes the stream's mode to any
  whose reading and writing the file was opened for (text and binary may change, and
  appending, which then seeks to the end before each write where the file was not
  opened for appending); its position and buffers are as after `fflush`, and its
  orientation is reset. Another mode fails and closes the stream.
- **Exit**: `exit` (and the return from `main`) flushes and closes every stream;
  `_Exit`, `quick_exit` and `abort` write no buffered data and remove no `tmpfile`
  files (7.22.4.5p2's choice). `fmemopen` and the other POSIX streams are not
  provided (they are not ISO C's).
- **Large transfers**: one `fread` or `fwrite` call may move more than 2^31 bytes
  (the system's calls are made in pieces); a formatted output of more than `INT_MAX`
  characters fails with `EOVERFLOW` (7.21.6.1p14).
- **`getc`, `putc`, `getchar` and `putchar`** are also macros (7.21.7.5p2,
  7.21.7.8p2), which evaluate their arguments once: while the program has one thread
  a byte of a byte-oriented stream's buffer is taken or stored in line, and the
  functions are called otherwise (an empty or full buffer, a new-line of a
  line-buffered stream, any stream once a thread has started).
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
  decimal and hexadecimal conversions are exact and correctly rounded at any precision
  in the current rounding direction (F.5: halfway cases to even when to nearest). C23's `%b` and `%B` and the
  `wN` and `wfN` length modifiers are accepted in every mode.
- **`fscanf`**: an input item is the longest prefix of a matching sequence, and only
  the character after it is given back (7.21.6.2p9), so a prefix that is not a whole
  number is a matching failure: `"0x"` for `%x`, `"1e+"` and `"100ergs"` (the example of
  7.21.6.2p20) for `%f`, and `%3c` when the input ends after two characters. The
  scanset `%[a-z]` is a range when `-` is neither first nor last. `%p` reads what `%p`
  writes. Floating conversions are correctly rounded for every format, in the current
  rounding direction (as `strtod`'s are; an overflow toward zero gives the largest
  finite value); `%i` reads C23's
  `0b` prefix in C23 mode (the header maps the functions to `__axc_..._c23` forms) and
  not before, and `%b` reads binary digits after an optional `0b`. C23's digit
  separators (`1'000`) are a constant's only: neither `fscanf` nor `strtod` reads them.
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

- `longjmp` unwinds nothing on Linux, macOS and Windows ARM64: the restored stack
  pointer drops the frames. On Windows x64 the hosted library's `longjmp` has the
  system unwind the frames in between first (`RtlUnwindEx` to the `setjmp` caller's
  frame, with `STATUS_LONGJUMP`; `jmp_buf` is the system's `_JUMP_BUFFER`), so that the
  termination handlers of system frames in between run (a callback a DLL called, left
  by `longjmp`); the registers are then restored from `jmp_buf`
  (`std/c/rt/core_jump.ax`). A freestanding program's `longjmp` restores them at once,
  which is right for the program's own frames (they have no handlers), and its last
  instruction moves the stack pointer and returns in one step, so that the system's
  unwinder is right at every instruction of it.
- Jumping back into a function whose variable length arrays were allocated after the
  `setjmp` call frees them (the stack pointer goes back); jumping out of, or over,
  Axiom functions that C called (an Axiom function on the stack between the `setjmp`
  and the `longjmp`) is undefined: their `defer`s do not run.
- On WebAssembly, `setjmp` needs the exception-handling extension
  (`--features=+exception-handling`, not among the default features; else `C3300`),
  in its standard form (`try_table`, `throw`, a tag; node 22 and current browsers have
  it). `longjmp` stores its value in the `jmp_buf` and throws an exception that carries
  the `jmp_buf`'s address; the function that called `setjmp` runs its body in a
  `try_table` that catches it, and if the `jmp_buf` was set by that very activation
  (`setjmp` stored the activation's frame address in it, with the call's number and
  the stack pointer), runs its body again from a dispatch to that `setjmp` call, with
  the stack pointer back as it was there; otherwise it throws it on. Values the
  expression around a `setjmp` call computed before it are kept in memory, as the
  objects are. A `longjmp` to a `jmp_buf` whose function has returned ends the program
  as a trap. Frames between are left without running anything, as natively; a
  signal handler that `raise` called may `longjmp`.
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
  handler, which the system calls on a thread of its own). On x64, `SIGSEGV`,
  `SIGFPE` and `SIGILL` handlers get the processor's exceptions in the program's own
  code (an access violation, an integer division fault or an unmasked floating-point
  exception, an illegal instruction): a vectored exception handler redirects the
  faulting thread as if the faulting instruction had called a trampoline (whose unwind
  information lets the system's walks, and `longjmp`, go on through the faulting
  function); a handler that returns runs the instruction again. A stack overflow, a
  fault in a DLL, and any fault while the signal's action is `SIG_DFL` or `SIG_IGN`
  are the crash reporter's. On ARM64 the faults stay the crash reporter's. The others
  arrive only from `raise`, which calls the handler at once. The default action of a raised signal ends the process with status 3
  for `SIGABRT` (as `abort`'s) and 128 + the signal's number for the others.
- WebAssembly: `raise` alone, as on Windows.

`abort` raises `SIGABRT`; if that returns (a handler returned, or the signal is
ignored), it sets the default action and raises it again, and ends the process with
status 3 if it still lives.

## `<math.h>`, `<fenv.h>`, `<tgmath.h>` and `<complex.h>`'s functions

The library's mathematics (C17 7.12, 7.6, 7.25, 7.3.5-7.3.9, and C23's additions to
them) are its own: written from the standard and the published methods each source file
cites, with polynomial coefficients and tables computed by
`cmd/internal/tools/gen_math_tables.py` (in arbitrary precision; `std/internal/math/tables.ax`).
The elementary double and float functions are kernels shared with Axiom's `std/math`
(`std/internal/math/`), which report their errors through the library's wrappers
(`std/c/rt/math*.ax`); C23's that compute are `std/c/rt/math_c23.ax`, its narrowing
operations `math_narrow.ax`; those that take or return `int` or `long`, the bit-level ones
(payloads, total order, `fromfp`), the `long double` forms, `<fenv.h>` and the complex
functions are C (`std/c/lib/math/`).

- **Accuracy.** `sqrt`, `fma`, the rounding functions, `fmod`, `remainder`, `remquo`,
  `ldexp`/`scalbn`, `frexp`, `nextafter` and the other exact ones are exact or correctly
  rounded, in every rounding mode. The elementary functions are within 1 ulp over their
  whole domains (the table below). Trigonometric arguments are reduced
  exactly (Payne and Hanek's method beyond 2^20 pi/2), so `sin(1e300)` is accurate. The
  float functions compute in double, most with kernels of their own (short polynomials
  to about 2^-37: `expf`, `exp2f`, `exp10f`, `expm1f`, `logf`, `log2f`, `log10f`,
  `log1pf`, `sinf`, `cosf`, `tanf`, `asinf`, `acosf`, `atanf`, `atan2f`, the hyperbolic
  ones, `cbrtf`, `hypotf`, `erff`, `erfcf`, `powf`), the others with the double
  function, and round once (within 0.5 + 2^-12 ulp; measured 0.500 for each). Errors
  are measured against an arbitrary-precision reference on 200000 random and boundary
  arguments per function.
- **Speed.** The kernels compute in plain double where the error bound allows
  (`log2`, `log10` and `log1p` from log's table, `log10` with a table of its own, `asin`
  and `acos` with their tails in double, `cbrt` from a table of 1/c and cbrt(c) and a
  polynomial without a division, `sinh` and `cosh` from the exponential's parts, the
  inverse hyperbolic functions from double-double arguments with one division, `erf`,
  `erfc` and the gamma pieces by Estrin's scheme with the first terms compensated, `erf`
  and `erfc` taking their piece from the argument's bits), and in double-double only
  where cancellation needs it, with Ziv's strategy (an estimate in double, with fused
  multiply-adds where the hardware has them, and a rounding test, the double-double
  kernel only when the test fails) in `atan`, `atan2` (reduced directly from y and x)
  and `powf` (whose float result is correctly rounded or computed by the double
  function); `tan` takes its odd form from a table of -cot c rather than a branch.
  Against the x86-64 Linux system C library (`-O2`, one core, the time of a call on a
  stream of arguments, measured on a loaded machine: CPU time, best of seven): most
  functions take 0.5 to 1.5 times as long, `erf`, `erfc` and `exp10` about 1.15 to 1.3,
  `lgamma`, `tan`, `asinh`, `pow` and `powf` 1.4 to 1.65, and `log2`, `acosh` 1.7 to 1.8.
  `std/math` has the same kernels: against its former functions, every one is as fast or
  faster, or within 15% (`atan`, `atan2`, `tan`, `cbrt`, `lgamma`; their former versions
  were less accurate: up to 2 ulps for `tan` and `sinh`).
- **The complex functions** follow Annex G's special values (G.6) and Kahan's methods;
  each part is within a few ulps, not correctly rounded: the steps where roundings
  add up (Kahan's `1 + t^2` and `1 + b s^2` of `ctanh`, the products of the square roots
  in `casin` and `cacos`, `catanh`'s denominators, `csqrt`'s sum and its Newton step) are
  computed in pairs of the type (Dekker's error-free products, from `fma`), so that the
  error is mostly that of the real functions called (double, measured on 20000 arguments
  each: `clog` 0.5; `csqrt` 0.84; `catan`, `catanh` 1.35; `cexp`, `csin`, `ccos`, `csinh`,
  `ccosh` 2.1; `casin`, `cacos`, `casinh`, `cacosh` 2.5; `ctan`, `ctanh` 2.7; the float
  forms are the double ones rounded, within half an ulp and a fraction). `cpow` is `cexp(w clog z)`, whose error grows with
  the size of the result's exponent.
- **`long double`** where it is `double` (Windows, macOS; and with `--c-long-double=64`) has
  the double functions. Where it is wider (the 80-bit x87 format on x86-64 Linux, binary128
  on AArch64 Linux and WebAssembly), the exact `l` functions (`fabsl`, `copysignl`, the
  rounding functions, `fmodl`, `remainderl`, `remquol`, `frexpl`, `ldexpl`, `scalbnl`,
  `ilogbl`, `logbl`, `modfl`, `nextafterl`, `nexttowardl`, `nextupl`, `nextdownl`, `fmaxl`,
  `fminl`, `fdiml`, the `fmaximum` family) work on the format's bits and are exact, `sqrtl`
  is correctly rounded (`std/c/lib/math/m_ldwide.c`), and so is `fmal` (exact in 512-bit
  integers). The elementary ones (`expl`, `exp2l`, `exp10l`, `expm1l`, `logl`, `log2l`,
  `log10l`, `log1pl`, `sinl`, `cosl`, `tanl`, `asinl`, `acosl`, `atanl`, `atan2l`,
  `sinhl`, `coshl`, `tanhl`, `asinhl`, `acoshl`, `atanhl`, `powl`, `hypotl`, `cbrtl`)
  compute in triple-double arithmetic (about 150 bits: binary128 needs more than a
  double-double's 106; the x87's 64 would not, but one implementation serves both),
  from the format's bits taken apart exactly, with tables of triple-doubles and an exact
  reduction of the trigonometric arguments over the whole exponent range, and round once
  (`std/c/rt/math_ld.ax`): within 0.5 + 2^-6 ulp of binary128, 0.5 + 2^-55 of the x87's
  (measured: 0.500 for each, both formats). They take about 0.3 to 3 us a call (x86-64,
  x87 format), 3 to 80 times the x87 hardware's functions of the system library (most
  5 to 20 times). The complex `l` functions are the double ones' methods on these (each part within
  3.3 ulps of the format, measured). `erfl` and `erfcl` (Taylor's series below 2 and the
  series of positive terms times e^-x^2 from 2 to 6, by Horner's rule on tables of
  their coefficients; the continued fraction above, its deeper levels in
  double-double), `lgammal` and `tgammal` (Stirling's series from 24 up, the
  Taylor series of `lgamma(2 + z)` and the recurrence below, the reflection for negative
  arguments, with the Taylor series about each zero of `lgamma` in (-34, -2) where the
  reflection cancels) compute in triple-double too: within 0.55 ulp of binary128 and
  0.5 + 2^-6 of the x87's (measured on 900 arguments each, near the zeros of `lgamma`
  included); 3 to 5 us a call. `_Float128`'s `erff128` ... `tgammaf128` are the same.
- **Errors** (7.12.1): `math_errhandling` is `MATH_ERRNO | MATH_ERREXCEPT` on x86-64 and
  AArch64 and `MATH_ERRNO` on WebAssembly. A domain error sets `errno` to `EDOM` and raises
  "invalid"; a pole error `ERANGE` and "divide-by-zero"; an overflow `ERANGE` and
  "overflow"; an underflow (a result that is subnormal or zero and inexact) `ERANGE` and
  "underflow". `lrint`, `lround` and the like with a result outside their type raise
  "invalid", set `EDOM` and return the type's minimum.
- **Classification and comparison macros** are builtins (`__builtin_axiom_isnan`, ...):
  each operand is evaluated once, the classification is done on the bits, the comparisons
  are quiet (no exception for a quiet NaN), and they are constant expressions for constant
  operands. `<tgmath.h>` works in C99 as in later standards (`__builtin_axiom_tg`).
- **`nan("n-char-sequence")`** reads the sequence as an unsigned integer (decimal, `0x`
  hexadecimal or `0` octal) and puts its low bits (51, or 22 for `nanf`) in the payload;
  any other sequence gives the payload 0.
- **C23 functions**: all of 7.12's, each in three types: `exp10`, `exp2m1`, `exp10m1`,
  `log2p1`, `log10p1`, `logp1`, `rootn`, `pown`, `powr`, `compoundn`, `rsqrt`, `sinpi`,
  `cospi`, `tanpi`, `asinpi`, `acospi`, `atanpi`, `atan2pi` (within about 0.52 ulp, the
  arguments of their exponentials and logarithms as double-doubles; `long double`'s in
  its precision), `roundeven`, `nextup`, `nextdown`, `llogb`, `fromfp`, `ufromfp`,
  `fromfpx`, `ufromfpx`, the `fmaximum` family, the narrowing operations (`fadd`, `fsub`,
  `fmul`, `fdiv`, `ffma`, `fsqrt` of doubles and of long doubles to float, `daddl` ... to
  double: exact in 512-bit integer arithmetic and rounded once in the current direction,
  tininess after rounding), `getpayload`, `setpayload`, `setpayloadsig` (the payload is
  the fraction below the quiet bit, as `nan` writes it), `totalorder`, `totalordermag`
  (on the encodings), `canonicalize` (the x87's pseudo-denormals made canonical; its
  invalid encodings give a nonzero result), and the macros `iseqsig` (a domain error and
  "invalid" for a NaN), `issubnormal`, `iszero`, `issignaling`, `iscanonical`,
  `FP_INT_UPWARD` ... `FP_INT_TONEAREST`, `FP_LLOGB0`, `FP_LLOGBNAN`. `FP_FAST_FMA` and the
  other `FP_FAST_` macros are not defined: each of those operations is a call of the
  library's function, slower than the operations it fuses. `<fenv.h>`'s `fegetmode`,
  `fesetmode`, `fesetexcept`, `fetestexceptflag`.

| Largest error measured (ulps) | |
|---|---|
| `log`, `log2`, `log10`, `atan2`, `acosh`, `tan`, `atan`, `pow`, `hypot` | 0.50-0.51 |
| `exp`, `exp2`, `exp10`, `log1p`, `cosh`, `sinh`, `tanh`, `asinh`, `tgamma`, `cbrt`, the C23 functions | 0.51-0.53 |
| `atanh`, `erfc`, `expm1` | 0.54-0.56 |
| `acos`, `asin` | 0.65-0.66 |
| `lgamma`, `sin`, `cos`, `erf` | 0.69-0.77 |
| every float function | 0.500 |

The results are the same bits on every target (the error-free transformations the
functions use give the same bits with and without fused multiply-add).

### The floating-point environment

On x86-64 `<fenv.h>` reads and writes MXCSR, on AArch64 FPCR and FPSR (the compiler's
builtins `__builtin_axiom_fenv_get` and `__builtin_axiom_fenv_set`, the backends' `stmxcsr`,
`ldmxcsr`, `mrs` and `msr`): all four rounding modes and the five exception flags. Exceptions
are never trapped. WebAssembly has no floating-point environment: rounding is to nearest,
`fesetround` fails for the other modes, no flag is ever set (`fetestexcept` is 0) and
`feraiseexcept` fails for a nonzero argument; the macros are defined all the same. The
other modes are not done in software: the program's own arithmetic would need it too, and a `fesetround` that succeeded while
`a + b` still rounded to nearest would mislead the programs that use it (7.6.4.3 lets it
fail): so Annexes F and G are not claimed there.

**`#pragma STDC FENV_ACCESS ON`** (7.6.1), and `--c-fenv-access` for every function, is
honoured. A function with floating operations in its scope (anywhere in its body) has
strict floating point (the IR function's `strict_fp`, which the passes consult): its
floating operations (arithmetic, comparisons, conversions, sqrt and fma) are not folded
(`opt.ax`, `combine.ax`), merged (`gvn.ax`), moved out of loops (its LICM), sunk to their
uses or fused into a later branch across a call (`gvn.ax`, `x64.ax`, `a64.ax`), packed
into vectors (`slp.ax`), interleaved with other iterations (`unroll.ax`'s jamming) or
removed when their results are unused (`ir.has_effects`, and the AArch64 backend's own
dead code removal); a function inlined into another makes that one strict, and outlined
code stays strict (`inline.ax`, `outline.ax`; `divconst.ax` turns only integer divisions
into multiplications). In the C front end, an operation on floating values in the
pragma's scope is done at run time though its operands are constants (F.8.4, F.8.5:
automatic objects' initializers too; static ones are folded at translation time, in the
static direction of a `FENV_ROUND` in whose scope they are).
Outside its scope, and without the option, the optimizer assumes the default environment
as before: it folds constants as rounding to nearest with no flags, and may move or drop
floating operations. The relational operators `<`, `<=`, `>` and `>=` signal "invalid"
for a NaN operand (F.9.3; `comisd`, `fcmpe`), the macros `isless` ... do not. Vectors (a GNU
dialect) follow the same rules lane by lane: their arithmetic is strict in a strict
function, their relational operators signal (`cmpltps`, `fcmgt`; the IR's vector `fcmp`
carries `F_SIGNALING` as the scalar one does), `==` and `!=` do not
(`c/lib/math/fenv_vector.c`, float, double and `_Float16` lanes).

**`#pragma STDC FENV_ROUND direction`** (C23 7.6.2) sets the static rounding direction
of its scope: each full expression in it (a statement's expression, a condition, an
initializer) is evaluated with the direction set in the environment's rounding bits and
the dynamic mode's restored after it (the flags it raised kept); a call in it of a
function that 7.6.2p4's table does not list (other than `<math.h>`'s and the
conversions of `<stdlib.h>`, `<stdio.h>` and `<wchar.h>`) runs in the dynamic mode, which
is read again after the call (the table's: `<math.h>`'s functions of 7.6.2p4 with every
suffix, Annex H's `f16` ... `f64x` too, `atof`, `strtod` ... `strtold` and their `_FloatN`
and `wcs` forms, `strfromd` ... and the formatted input/output functions). The library's
functions of the table follow the mode they are called in, in every format: `sqrt`,
`fma`, `rint`, `nearbyint`, `lrint`, `llrint`, `scalbn`, `ldexp`, `fdim`, the narrowing
operations, the conversions of `<stdlib.h>` and `<stdio.h>` (for a wide `long double` and
`_Float128`, `m_ldwide.c` and `math_ld.ax`'s `fma` read the direction and raise the
exceptions). At translation time (F.8.2), floating constants in the scope of a static
direction are rounded in it, and so are the constant expressions folded there (static
initializers: `static double x = 1.0 / 3.0;` is the value the division has at run time
in that direction), every operation and conversion of every floating type, complex ones
included (`cldbl.ax`'s `g_dir`, which semantic analysis sets for each declaration and
block item). `FE_DYNAMIC` is the dynamic mode; `FE_TONEARESTFROMZERO` is `C4113` (no
target has it for binary arithmetic, and C23 asks for it only for the decimal types), and
so is any directed rounding on WebAssembly.

**`#pragma STDC FENV_DEC_ROUND direction`** (C23 7.6.3) is the same for the decimal
types, in every direction (`FE_DEC_DOWNWARD`, `FE_DEC_TONEAREST`,
`FE_DEC_TONEARESTFROMZERO`, `FE_DEC_TOWARDZERO`, `FE_DEC_UPWARD`; `FE_DEC_DYNAMIC`) and
on every target (the decimal arithmetic is software): decimal constants and folded
constant expressions round in it at translation time; at run time each full expression
sets the thread's decimal direction (`fe_dec_setround`'s) to it and restores the dynamic
one after it, and a call of a function its table does not list (the `d32`, `d64` and
`d128` functions of `<math.h>`, `strtodN`, `wcstodN`, `strfromdN`, the formatted
input/output) runs in the dynamic one.

**The software arithmetic** (a `long double` wider than `double`, `_Float128`,
`_Float64x`, `_Float16`'s rounding steps, the conversions of integers wider than 64 bits
and of the decimal types to binary ones) follows the environment too: in a function with
strict floating point (`FENV_ACCESS ON`, `--c-fenv-access`, or a static `FENV_ROUND`
direction in it) each operation reads the environment's register, rounds in its
direction and raises its exceptions in it (invalid, divide-by-zero, overflow, underflow
for a tiny inexact result with tininess detected after rounding, inexact), as the
hardware's `float` and `double` operations do: `std/c/rt/ldouble.ax`'s `e_` functions.
Elsewhere it rounds to nearest and raises nothing, at no cost for the environment: code
translated with `FENV_ACCESS` OFF runs in the default mode or has undefined behaviour,
and its flags may not be tested (7.6.1p2), so the results are the same. A `_Float16`
operation is computed in `float` in the dynamic mode and rounded again in it (two
roundings in one directed mode are one); widening a signaling NaN raises "invalid", and
`<` `<=` `>` `>=` on `_Float16` signal as on `float`. Tests:
`c/lib/math/fenv_wide_q.c`, `fenv_wide_x.c`, `fenv_half.c` (every operation in the four
directions, bits and flags, against exact rational arithmetic:
`cmd/axiom/internal/tests/tools/gen_fenv_wide_tests.py`).

**FP_CONTRACT**: no floating expression is ever contracted (an FMA is only made where the
program calls `fma`), so the default is OFF, and both `#pragma STDC FP_CONTRACT ON` and
`OFF` hold: results are the same bits at every optimization level and on every target.

The library's functions raise their exceptions through the environment itself, not by
arithmetic the optimizer could fold, and honor the rounding mode where the standard asks
(`rint`, `lrint`, `nearbyint`, `sqrt`, `fma`, `scalbn` and the narrowing operations round
in it).

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
| C23 | The decimal floating types and constants (`_Decimal64`, `1.20dd`), attributes (`[[deprecated]]` and the other standard ones), `constexpr`, `auto` type inference, `typeof` and `typeof_unqual`, `nullptr`, `bool`, `true`, `false`, `alignas`, `alignof`, `static_assert` and `thread_local` as keywords, `_BitInt(N)` and the `wb` suffix, binary constants, digit separators, `#embed`, `__has_include`, `__has_embed`, `__has_c_attribute`, `#elifdef`, `#elifndef`, `#warning`, `__VA_OPT__`, `{}` initializers, enumerations with a fixed underlying type, labels before declarations, unnamed parameters in definitions, `u8''` constants, `static_assert` without a message, `<stdbit.h>`, `<stdckdint.h>`, `unreachable` and `nullptr_t` in `<stddef.h>`. No more trigraphs, K&R function definitions or `()` as an unprototyped declarator. |

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
`__STDC_UTF_32__` are 1, and in a hosted program `__STDC_LIB_EXT1__` is `201112L`
("Annex K"); `__STDC_NO_ATOMICS__`, `__STDC_NO_COMPLEX__` and
`__STDC_NO_THREADS__` are not defined: atomics, threads (see "Atomics and threads") and
complex types are supported. In C23,
`__STDC_EMBED_NOT_FOUND__`, `__STDC_EMBED_FOUND__` and `__STDC_EMBED_EMPTY__` are 0, 1
and 2, and `__STDC_IEC_60559_DFP__` is `202311L` (the decimal floating types, "Decimal
floating types" below). From C99, on x86-64 and AArch64 with the library (hosted, not
`--c-lib=system`), `__STDC_IEC_559__` and `__STDC_IEC_559_COMPLEX__` are 1, and in C23
`__STDC_IEC_60559_BFP__` and `__STDC_IEC_60559_COMPLEX__` are `202311L` (Annexes F and G);
not on WebAssembly. `__STDC_IEC_60559_TYPES__` is not defined (see "Known deviations").

The compiler's own macros say what the build chose:

| Macro | Value |
|---|---|
| `__AXIOM__` | 1 |
| `__AXIOM_HOSTED__` | 1, defined only in a hosted program (not with `--c-freestanding`), in every mode: C89 has no `__STDC_HOSTED__`. |
| `__AXIOM_INT_WIDTH__`, `__AXIOM_LONG_WIDTH__` | The widths of `int` and `long` in bits. |
| `__AXIOM_LDBL_WIDTH__` | The format of `long double`: 64 (binary64), 80 (the x87's) or 128 (binary128). |
| `__AXIOM_PTR_WIDTH__` | The width of a pointer: 32 or 64. |
| `__AXIOM_WCHAR_WIDTH__` | The width of `wchar_t`: 16 or 32. |
| `__AXIOM_WCHAR_SIGNED__` | 1 if `wchar_t` is signed, else 0. |
| `__AXIOM_BITFIELD_LAYOUT__` | Whose rules lay out bit-fields ("Implementation-defined behavior"): 0 the System V ABI's (x86-64 Linux, macOS, WebAssembly), 1 AAPCS64's (AArch64 Linux), 2 Windows x64's. |
| `__AXIOM_<T>_TYPE__`, `__AXIOM_<T>_MIN__`, `__AXIOM_<T>_MAX__`, `__AXIOM_<T>_WIDTH__`, `__AXIOM_<T>_C__(c)` | For `<T>` one of `SIZE`, `PTRDIFF`, `INT64`, `UINT64`, `INTMAX`, `UINTMAX`, `INT_FAST16`, `UINT_FAST16`, `INT_FAST32`, `UINT_FAST32`: the type the platform gives that typedef (`__AXIOM_SIZE_TYPE__` is `unsigned long`, ...), its limits, width and constant macro, which the headers use. `__AXIOM_WINT_TYPE__`, `_MIN__`, `_MAX__` and `_WIDTH__` are `wint_t`'s; `__AXIOM_MAX_ALIGN__` is `max_align_t`'s alignment. |
| `__CHAR_UNSIGNED__` | 1, defined only if plain `char` is unsigned. |
| `__AXIOM_WASM__` | 1, defined only for a WebAssembly target. |
| `__AXIOM_WASM_BULK_MEMORY__`, `__AXIOM_WASM_EXCEPTION_HANDLING__` | 1, defined only for a WebAssembly target with that extension (`--features`): the C library's `memcpy`, `memmove` and `memset` are `memory.copy` and `memory.fill` with the first; `setjmp` needs the second. |
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
| `aarch64-macos`, `x86_64-macos` | signed | 32 | 64 | 64 | 64 | `unsigned long` | `long` | `int` | `long long` |
| `x86_64-windows`, `aarch64-windows` | signed | 32 | 32 | 64 | 64 | `unsigned long long` | `long long` | `unsigned short` | `long long` |
| `wasm32` | signed | 32 | 32 | 64 | 32 | `unsigned long` | `long` | `int` | `long long` |
| `wasm64` | signed | 32 | 64 | 64 | 64 | `unsigned long` | `long` | `int` | `long long` |

| Target | `intmax_t` | `int_fast16_t` | `int_fast32_t` | `wint_t` (its limits) | `max_align_t` |
|---|---|---|---|---|---|
| `x86_64`, `aarch64` (Linux) | `long` | `long` | `long` | `unsigned int` | aligned to 16 |
| `aarch64-macos` | `long` | `short` | `int` | `int` | `long double` (8) |
| `x86_64-macos` | `long` | `short` | `int` | `int` | `long double` (16) |
| `x86_64-windows`, `aarch64-windows` | `long long` | `int` | `int` | `unsigned short` | `long double` (8) |
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
| `x86_64` (Linux), `x86_64-macos` | the x87's 80-bit extended format | 16, 16 (10 bytes of value, 6 of padding) | 64 |
| `aarch64` (Linux), `wasm32`, `wasm64` | IEEE 754 binary128 | 16, 16 | 113 |
| `aarch64-macos`, `x86_64-windows`, `aarch64-windows` | binary64 (`double`'s) | 8, 8 | 53 |

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
| Complex types | `float _Complex`, `double _Complex` and `long double _Complex` (C99 and later; also of `_Float16` ... `_Float64x`) are laid out as an array of two elements of the real type, the real part first: the size is twice the real type's, the alignment the real type's. The imaginary types (`float _Imaginary`, `double _Imaginary`, `long double _Imaginary`, C99 and later, Annex G) have their real type's representation (the imaginary part); their arithmetic is G.5's (an imaginary times an imaginary is real, a real plus an imaginary complex, ...), their conversions G.4's (an imaginary converted to a real type is 0, a complex one to an imaginary type its imaginary part). |
| Interchange and extended floating types | (C23 Annex H, and in every mode before C23.) `_Float16` is binary16, `_Float32` binary32, `_Float64` and `_Float32x` binary64, `_Float128` binary128 (`__float128` in a GNU dialect), `_Float64x` the x87 format where `long double` has it, else binary128; each a type of its own. `_Float16`'s operations are rounded to binary16 each (`FLT_EVAL_METHOD` 0: computed in `float`, which is exact enough that the one rounding is correct), `_Float128`'s and `_Float64x`'s are `long double`'s software arithmetic in that format. The usual arithmetic conversions take the type with more values, then of two with the same values an interchange type over a standard one over an extended one (`float + _Float32` is `_Float32`, `double + _Float32x` is `double`). Constants take `f16`, `f32`, `f64`, `f128`, `f32x`, `f64x` (either case). `<float.h>` has their macros (`FLT16_MAX` ...) under `__STDC_WANT_IEC_60559_TYPES_EXT__`; `__STDC_IEC_60559_TYPES__` is not defined (it needs Annex F). |
| Floating types | `float` and `double` are IEEE 754 binary32 and binary64; `long double` as the table above says. `FLT_EVAL_METHOD` is 0, `FLT_ROUNDS` is 1 (to nearest): it is the start-up's mode, which `fesetround` changes on x86-64 and AArch64 (`<fenv.h>`, below; `FLT_ROUNDS` itself stays 1). A `long double` wider than `double` is computed in software (`std/c/rt/ldouble.ax`, written from IEEE 754-2019): each operation and conversion correctly rounded to nearest, ties to even, with subnormals (`+ - * /` of normal operands with a normal result by 64-bit integer fast paths, division by Knuth's Algorithm D); constant expressions are folded with the same arithmetic, and floating constants read exactly. Conversions of such a value to an integer type truncate and saturate, as those of `float` and `double` do (the behavior is undefined when the value does not fit). A NaN result of an invalid operation is the format's default NaN (the x87's negative "indefinite"; binary128's positive quiet NaN); a NaN operand gives its own, quieted (of two x87 NaNs, the one with the larger significand). x87 encodings the x87 itself treats as invalid (unnormals, pseudo-infinities, pseudo-NaNs) are invalid operands. The x87 format's six bytes of padding are zero in the values the program computes. |
| Sizes and alignment | A scalar's alignment is its size. A structure is laid out naturally: each member at the next multiple of its alignment, the structure aligned as its strictest member, its size rounded up to that; `#pragma pack(n)` caps the members' alignments at n ("Pragmas"). |
| `max_align_t` | Aligned as the platform's (above): a structure of a `long long` and a `long double` aligned to 16 (size 32 where `long double` is 16 bytes), or `long double` (8 bytes). |
| Alignment specifiers | `_Alignas` takes any power of two up to 4096 (more is `C3071`). |
| Bit-fields | Laid out as the target's ABI lays them out: allocated from the least significant bit of a storage unit of the declared type, never straddling two units. On x86-64 Linux, macOS and WebAssembly (the System V rules) a bit-field goes at the next bit if it fits in the unit that holds it, an unnamed one adds nothing to the alignment of the structure, and one of width 0 moves the next member to the next unit of its type. On AArch64 Linux (AAPCS64) the same, but every bit-field's type adds to the alignment, unnamed ones and those of width 0 too. On Windows (x64 and ARM64) bit-fields are allocated in runs: a bit-field joins the open unit only if its type has the unit's size and it fits, else it starts a new unit after it; another member closes the unit; one of width 0 closes it (and aligns the next member to its type) and is ignored when no unit is open; in a union, bit-fields add their type's size but not its alignment. In a packed structure or under `#pragma pack` (GNU dialects; any `pack(n)`), a bit-field goes at the next bit (System V, AAPCS64; AAPCS64 still aligns the structure to a zero-width one's type), while Windows keeps its units, aligned to at most n. A plain `int` bit-field is signed. A `char` bit-field has `char`'s signedness. |
| Enumerations | The type is `unsigned int` if no enumerator is negative, else `int`, when all fit; the enumeration constants are `int`. In C23, if some enumerator does not fit `int`, the first of `long`, `long long` (`unsigned long`, `unsigned long long` for an unsigned type) that holds all of them, and then the constants have the enumerated type. Before C23 a value outside `int` is `C3013`. With a fixed underlying type (C23), that type. |
| `_BitInt(N)` | `BITINT_MAXWIDTH` is 65535. For N up to 64 an object is stored in the smallest of 8, 16, 32 or 64 bits that holds N (aligned as that integer type, as the x86-64 and AArch64 ABIs specify), extended to it; as an argument or result it is that integer. Wider, it is stored in 64-bit limbs, the least significant first, as many as hold N, aligned to 8 (the x86-64 System V psABI's and Windows' array of 8-byte chunks; WebAssembly the same), or on AArch64 (Linux and macOS) in 16-byte units aligned to 16 (AAPCS64: `_BitInt(65)` to `_BitInt(128)` as `__int128`): `sizeof(_BitInt(129))` is 24, 32 on AArch64. The value is sign- or zero-extended through its storage; the bits beyond N that arrive (arguments, results, and objects in memory, which code compiled elsewhere writes) are not relied on: the ABIs leave them unspecified. |
| `__int128` | GNU dialects: `__int128` and `unsigned __int128` are 16 bytes aligned to 16 on every target, an extended integer type whose rank is above `long long` and above `_BitInt(128)`'s, below `_BitInt(129)`'s. `__AXIOM_INT128__` is defined (1). `intmax_t` stays 64 bits (C23 7.22.1.5 lets an extended type be wider), and so does the preprocessor's arithmetic. |
| Integers wider than 64 bits | `__int128` and `_BitInt(N)` for N > 64. Their arithmetic is exact to the type's width: two-limb values (up to 128 bits) inline (+ and - as add/adc and sub/sbb on x86-64, adds/adc and subs/sbc on AArch64, * with the 64-by-64-bit product's high half, shifts, comparisons, the bitwise operations; / and % inline when both operands are below 2^64, else a function: on x86-64 one generated into the program that uses the 128-by-64-bit `div`, elsewhere a run-time function), wider ones by the functions of `std/c/rt/wideint.ax` (generated into a program only when it calls them); division is Knuth's Algorithm D. Constant expressions are folded with the same functions, in the type's precision. Conversions to `float` and `double` are correctly rounded (to nearest, ties to even), as are those to a wide `long double` (`std/c/rt/ldouble.ax`); from them they truncate toward zero and saturate (undefined when the value does not fit, 6.3.1.4). A signed result that overflows wraps, as for the narrower types (undefined: a program must not rely on it), but in a constant expression it is `C3036`. Bit-fields of these types are laid out as the ABIs say: in a unit of the type's size aligned as the type is, the last such window at or before the next free bit that holds it, else the next (System V, and for `__int128` AAPCS64; checked against the system's compilers on x86-64 and, for `__int128`, AArch64 Linux), on Windows in its runs of units of the type's size at its alignment; in a packed structure one that no 8-byte unit holds is `C3901`. Not supported (`C3901`): an enumeration whose underlying type is `__int128` (one that is a `_BitInt` is `C3075`, C23 6.7.2.2p5), case ranges in a `switch` on them; an integer constant expression where a value of at most 64 bits is needed (an alignment, a bit-field's width, an array's length: `C3009`) whose value is wider is `C3212`. `<stdbit.h>`'s type-generic functions take `unsigned __int128` and the unsigned `_BitInt` types (C23 7.18.1p4: those whose width is a standard or extended type's; here any above 64 bits too), `<stdckdint.h>`'s and the GNU `__builtin_*_overflow` functions any of them. An `_Atomic` one of 16 bytes is lock-free where 16-byte atomics are ("Atomics and threads"), a wider one under the lock; the GNU `__atomic` builtins' generic forms take them, the others only objects of 1, 2, 4 or 8 bytes. `printf` and `scanf` have no conversion for them: C23's `%wN` length modifiers are for the widths of the `intN_t` types `<stdint.h>` defines, which stop at 64. `intmax_t` and `uintmax_t` stay 64 bits on every target (C23 7.22.1.5 lets an extended or bit-precise type be wider), as do `INTMAX_WIDTH`, `%jd` and the preprocessor's arithmetic, in which a `wb` constant wider than 64 bits is `C1016`. |
| Null pointer | All bits zero. |
| Source and execution character sets | UTF-8. Narrow string literals are UTF-8 bytes (invalid UTF-8 in a literal is `C1024`). `L"..."` is UTF-16 on Windows (`wchar_t` is 16 bits) and UTF-32 elsewhere; `u"..."` is UTF-16 and `U"..."` UTF-32. |
| Multi-character constants | `'ab'` is `'a' * 256 + 'b'` as an `int` of the build's width (the first character is the most significant); a warning, `C4102`. A wide one has the value of its last character. More characters than the type holds is `C1011`. |
| `main` | `int main(void)` and `int main(int, char **)` only (`C3061`). Reaching the end of `main` returns 0, in every mode. |
| Inline functions | An inline definition (6.7.4p7) is compiled with internal linkage, for the unit's own calls: the external function is another unit's. |
| `volatile` | Accesses are marked volatile for the optimizer. |
| `restrict` | A pointer parameter's tells the optimizer what it reaches is not reached otherwise ("Optimization"); others are accepted and have no effect. |

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
| `<float.h>` | all | `FLT_`, `DBL_` and `LDBL_` macros (the `LDBL_` ones and `DECIMAL_DIG` of the format of `long double`). No `FLT_SNAN`, `DBL_SNAN`, `LDBL_SNAN`. In C23 the `DEC32_`, `DEC64_` and `DEC128_` macros (`_SNAN` too: decimal signaling NaNs stay signaling) and `DEC_EVAL_METHOD` (0). |
| `<limits.h>` | all | `LLONG_` from C99; `*_WIDTH`, `BOOL_MAX` and `BITINT_MAXWIDTH` (65535) in C23. |
| `<stdarg.h>` | all | `va_list`, `va_start`, `va_arg`, `va_copy`, `va_end`. In C23 `va_start` needs no second argument. |
| `<stddef.h>` | all | `size_t`, `ptrdiff_t`, `wchar_t`, `NULL`, `offsetof`; `max_align_t` from C11; `nullptr_t` and `unreachable` in C23. |
| `<iso646.h>` | C95 and later | |
| `<stdbool.h>` | C99 and later | In C23, `bool`, `true` and `false` are keywords and the header only defines `__bool_true_false_are_defined`. |
| `<stdint.h>` | C99 and later | No `int32_t` family when no type is 32 bits wide. |
| `<complex.h>` | C99 and later | `complex`, `I` and `_Complex_I` (a constant of type `float _Complex`), `imaginary` and `_Imaginary_I` (of type `const float _Imaginary`), `CMPLX`, `CMPLXF` and `CMPLXL` (C11; constant expressions when the operands are), and the declarations of every function. `I` is `_Imaginary_I` where Annex G is claimed (x86-64, AArch64: `__STDC_IEC_559_COMPLEX__`), else `_Complex_I`. `creal`, `cimag` and `conj` (and their `f` and `l` forms) are also macros the compiler expands in line; the others are the library's ("`<math.h>`, `<fenv.h>`, `<tgmath.h>` and `<complex.h>`'s functions"). |
| `<math.h>` | all | C89's double functions; from C99 everything of C17 7.12, in the three types, and C23's listed below. |
| `<fenv.h>` | C99 and later | All of C17 7.6; C23's `fegetmode`, `fesetmode`, `fesetexcept`, `fetestexceptflag`. |
| `<tgmath.h>` | C99 and later | Every type-generic macro of C17 7.25 (and of C23's functions the library has), through `__builtin_axiom_tg`, in C99 too. Decimal arguments choose the `d32`, `d64` and `d128` functions (C23 7.27p3); arguments of the interchange and extended types (Annex H.11) their common real type's: `_Float16`'s and `_Float128`'s `f16` and `f128` functions (declared with `__STDC_WANT_IEC_60559_TYPES_EXT__`; without it float's function for `_Float16`), and for `_Float32`, `_Float64` and `_Float32x` the standard function of their format, whose result has the standard type. |
| `<stdalign.h>` | C11 and later | |
| `<stdatomic.h>` | C11 and later | All of 7.17 (see "Atomics and threads"), with memory orders. `ATOMIC_VAR_INIT` until C17; `atomic_char8_t` in C23. |
| `<stdnoreturn.h>` | C11 and later | |
| `<stdbit.h>` | C23 | The `stdc_` functions as `static inline` functions and type-generic macros. |
| `<stdckdint.h>` | C23 | `ckd_add`, `ckd_sub`, `ckd_mul`. |
| `<stdio.h>` | all (hosted) | The C library's ("The C library"). `snprintf`, `vscanf` and C99's other names from C99; `gets` before C11 only; `__STDC_VERSION_STDIO_H__` and `_PRINTF_NAN_LEN_MAX` in C23. |
| `<threads.h>` | C11 and later (hosted) | The C library's ("Threads"). `thread_local` before C23; `__STDC_VERSION_THREADS_H__` in C23. |

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

- Annex F and Annex G on WebAssembly (no floating-point environment: no rounding modes,
  no exception flags), and with `--c-lib=system` or `--c-freestanding`: there
  `__STDC_IEC_559__`, `__STDC_IEC_60559_BFP__`, `__STDC_IEC_559_COMPLEX__` and
  `__STDC_IEC_60559_COMPLEX__` are not defined. On x86-64 and AArch64 with the library
  they are (`docs/c-conformance.md`, "Annexes"). Annex H (`__STDC_IEC_60559_TYPES__`) is
  not claimed: the known deviations below.
- `setjmp` on WebAssembly without its exception-handling extension (`C3300`).
- `#embed` parameters other than `limit`, `prefix`, `suffix` and `if_empty` (`C1084`).
- Extensions, under an ISO standard. These are all errors there: `__attribute__`, `asm`
  and `__asm__`, `__int128`, `__typeof__` (and `typeof` before C23), statement
  expressions, `?:` with an omitted middle operand, case ranges, computed `goto` and labels
  as values, `__builtin_*` functions other than those below ("Reserved identifiers"),
  `#include_next`, `#ident`. A GNU dialect has them ("GNU dialects"), but for those it
  rejects too.
- In every mode: inline assembly on WebAssembly targets (C3901), `__declspec`, `#import`,
  `#assert`. In a GNU dialect: a jump from a nested function to a label of its enclosing
  function (C3901), and the decimal and `__bf16`/`__ibm128`
  types.

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
| `__real__ e`, `__imag__ e` | The real and imaginary parts of a complex value (of every complex type), lvalues when e is one (`__imag__ z = 1;`, `&__real__ z`, of e's qualifiers), constant for a constant e; of a real e, e and 0; of an imaginary one, 0 and its coefficient. |
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
| Nested functions | A function defined in a block: it sees the enclosing functions' objects (in their frames, through a chain of frame records passed to it as a hidden last argument) and may be called through a pointer while its enclosing call runs (a trampoline: one of a pool of 128 compiled thunks per nested function whose address is taken, claimed atomically and released when the enclosing call returns, so no code is written at run time and no stack is executable; more than 128 live at once traps). `auto` declares one ahead of its definition. Not variadic (C3901); a `goto` to a label of the enclosing function is C3901. |
| `__float128`, constants `1.0q`/`1.0Q`, `2.0i`/`2.0j` | `__float128` is `_Float128`; `q` is its constant suffix; `i` or `j` makes an imaginary constant of the type (C17 Annex G's imaginary types, which every mode has: "Floating types"). |
| `enum E : __int128` | An enumeration of a 128-bit underlying type (C23's fixed type); its constants' values must fit 64 bits (C3212). A `switch` on `__int128` takes case ranges. |
| Vector types | `T __attribute__((vector_size(n)))` (or `mode(V4SI)` ...): n bytes (a power of two times T's size, at most 4096) of lanes of T, an integer or real floating type. The arithmetic, bitwise and shift operators act lane by lane (a scalar operand is converted to the element type, if that keeps its value, and taken for every lane); `==` ... `>=` give the signed integer vector of the same size, -1 where true and 0; `!` is C3020; `v[i]` reads and writes a lane; `?:` with a vector condition selects lane by lane; initializers and compound literals give the lanes in order (missing ones 0; designators are C3072); a cast between vectors, or a vector and an integer, of one size keeps the bytes (another is C3028). `__builtin_shuffle(a, [b,] mask)`, `__builtin_shufflevector(a, b, i...)` and `__builtin_convertvector(v, T)` (below). A vector is aligned to its size, at most 16 bytes on AArch64 and on x86-64 Linux and macOS (32 with AVX, 64 with AVX-512F; on Windows and WebAssembly its size); IR generation uses the target's vector instructions for up to 64 bytes, and lane-by-lane code where there are none (conversions between integer and floating lanes on WebAssembly, a shift by a vector of counts). |

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
| `aligned`, `aligned(n)` | objects, members, structures and unions, typedefs | At least n bytes (no n: 16). On a typedef the type it names has that alignment (an array of an over-aligned element whose size is not a multiple of it is C3008); on a function accepted (code is aligned as the target's). |
| `packed` | structures, unions, members, enumerations | Members at byte alignment (but those with an alignment of their own); bit-fields at the next bit, each reached through the smallest unit of 1, 2, 4 or 8 bytes in the structure that holds it (none: C3901); an enumeration's type is the smallest integer type of its values. Members of a packed structure are loaded and stored with the alignment they have. |
| `section("s")` | objects with static storage duration, functions | The object or the function's code goes in section s ("Code in sections": Mach-O's `"segment,section"`, COFF's `s$m`); the linkers define `__start_s` and `__stop_s` for a C identifier's. Not on WebAssembly (accepted with no effect). |
| `used` | objects, functions | Kept though nothing refers to it. |
| `unused` | any declaration, labels | As `[[maybe_unused]]`. |
| `weak` | objects and functions with linkage | A weak definition, which gives way to another unit's that is not weak ("C and Axiom in one program"), or a weak reference (a null pointer if no unit defines it; on WebAssembly the host's import). `#pragma weak` too. |
| `alias("target")` | functions and objects | Another name for target, defined in the same unit (else C3090): a symbol of its own (with `weak`, a weak one) at target's place in every object format, which other objects refer to; the program's other units' references to the name are to target (unless a weak alias's name is defined otherwise, which then wins). An object for other links keeps target for it. |
| `visibility("v")` | objects, functions | default, hidden, protected or internal (another is C3077): recorded in the IR (`ir.Func.visibility`, `ir.Global.visibility`), as `#pragma GCC visibility` gives it to the declarations it covers, and written to `--emit=obj` and `--emit=lib` objects (ELF: `STV_HIDDEN` for hidden and internal, `STV_DEFAULT` for the others), where another link sees it; no effect on an executable's own code. |
| `noreturn` | functions | As `[[noreturn]]` (C4053). |
| `noinline`, `always_inline` | functions | Given to the optimizer. |
| `cold`, `hot` | functions | Given to the optimizer and the code generator: a block that calls a cold function (and the branch `__builtin_expect` says is unlikely) is laid out apart, as cold code. |
| `naked` | functions | The function is its asm statements (basic, or extended with no operands: else C3096): their text after a label of its name, with no prologue or epilogue ("Inline assembly"). |
| `const`, `pure`, `malloc`, `nothrow`, `leaf`, `artificial`, `flatten`, `noclone`, `noipa`, `externally_visible`, `no_instrument_function`, `returns_nonnull`, `alloc_size`, `alloc_align`, `assume_aligned`, `sentinel`, `may_alias` | | Accepted; no effect (none is needed for correct code). The optimizer is not told what `const`, `pure`, `malloc`, `returns_nonnull`, `alloc_size`, `assume_aligned` and `nonnull` promise: it would act on a promise a program breaks (a `const` function that writes memory, a null result), and the IR has no way to say them. |
| `format(archetype, i, j)`, `format_arg(i)` | functions | The calls' format strings are checked (`--c-warn-format`): archetype `printf`, `scanf` or `strftime` (also `gnu_printf`, ...), the format parameter i, the first argument checked j (0: none, as for `vprintf`). The conversions are C23's (with `b`, `B`, `wN`, `wfN`, and the decimal floating types' `H`, `D` and `DD`) and POSIX's `%n$` positions. A literal format is checked against the arguments' types as this target has them: C4130 (an argument the conversion does not take), C4131 (too few), C4133 (not a conversion specification); at level 2 also C4132 (arguments left over), C4134 (a format that is not a literal) and C4135 (another type of the same size and kind, a flag the conversion ignores, `%n`). A call of a `format_arg` function with a literal is checked as that literal. The C library's declarations (`printf`, `scanf`, `strftime`, their kin) carry the attribute in every mode. |
| `nonnull`, `nonnull(i, ...)` | functions | A null pointer constant passed for one of those parameters (all pointers without indexes) is warning C4121. |
| `warn_unused_result` | functions | As `[[nodiscard]]` (C4051). |
| `deprecated`, `deprecated("m")`, `unavailable` | declarations | As `[[deprecated]]` (C4050). |
| `error("m")`, `warning("m")` | functions | A call is error C3091, or warning C4123. |
| `fallthrough` | an attribute statement | As `[[fallthrough]]`. |
| `gnu_inline` | functions | GNU89's inline semantics (below) in a later dialect. |
| `constructor`, `constructor(p)`, `destructor`, `destructor(p)` | functions without parameters | Called by the program's start-up before `main` (lowest priority first; without one, 65535; then in the order defined), and after `main` returns (highest priority first, the last defined first). `exit` (the C library's) runs the destructors too, after the `atexit` functions, once; a program whose `main` is Axiom's runs the constructors before it and the destructors when it returns; an object without `main` (`--emit=obj`), and a freestanding program that ends by calling the system's `exit`, run neither, or no destructors. |
| `cleanup(f)` | automatic objects | `f(&object)` when its scope is left: the end of its block, `break`, `continue`, `goto` out, `return` (after the value is computed), the end of a statement expression; innermost first. A jump into its scope is C3083. |
| `transparent_union` | unions (and their typedefs) | A parameter of the type takes an argument of any member's type (or a null pointer constant, or a pointer to void for a pointer member). It is passed as the union is (the platform ABIs pass a union of one pointer or integer as that member). |
| `mode(m)` | objects, members, typedefs | The integer type of m's width with the declared type's signedness: QI, HI, SI, DI, TI (`__int128`), `byte`, `word`, `pointer` (also `__QI__` ...); of a floating type: HF (`_Float16`), SF, DF, XF (the x87 format's `long double` or `_Float64x`), TF (binary128: `long double` where that is its format, else `_Float128`), and SC, DC, TC ... their complex types; a vector mode (V4SI, V2DF, ...): the vector of those lanes. |
| `vector_size(n)` | objects, members, typedefs | The vector type of n bytes of the declared type ("Language": vector types). |

In gnu89 and gnu90 `inline` has GNU89's meaning: `inline` alone defines the external
function, `extern inline` is an inline definition (C99's `inline`).

### Inline assembly

On x86-64 (Linux, Windows) and AArch64 (Linux, macOS) targets; on WebAssembly an asm
statement is C3901: file-scope asm and `asm goto` have nothing to map to there (a module
is not free-form code, and its control flow is structured: a template cannot jump to a
C label), and templates in the WebAssembly text instructions with operands as locals
(`r`), linear-memory addresses (`m`) and constants (`i`), which would map soundly, are
not implemented. The compiler has its own assemblers (`cmd/axiom/src/asm/`), so no
other tool is needed:

- **x86-64**: the AT&T syntax (source operands first; `%reg`, `$imm`,
  `disp(base, index, scale)`, `%fs:`/`%gs:` overrides, `*` for indirect jumps and calls,
  size suffixes b, w, l, q or the size of a register operand, `lock`, `rep`, `repe`,
  `repne`), and the Intel syntax, after `.intel_syntax [noprefix|prefix]` (until
  `.att_syntax`) or everywhere under `--c-asm-dialect=intel`: the destination first,
  registers with or without `%`, immediates without `$`, memory as
  `SIZE PTR seg:disp[base + index*scale + disp]` (`BYTE` `WORD` `DWORD` `QWORD`
  `MMWORD` `TBYTE` `XMMWORD` `YMMWORD` `ZMMWORD`; `DWORD BCST [m]` a broadcast),
  `OFFSET [FLAT:]sym` for an address, an expression alone as memory at that address
  (a constant as an immediate; a jump's or call's operand as its target), mnemonics
  without size suffixes (the memory operand's size keyword gives the size),
  `movsx`/`movzx`/`movsxd`, `{k1}{z}`, `{1to16}`, `{rn-sae}` (also written on the last
  register), the string instructions with their operands written. A redundant segment
  override (`ds:` of an address, `ss:` at rsp or rbp) is no prefix, in both syntaxes.
  The comparison pseudo-ops `vcmpltps`, `vcmpnge_uqpd` ..., `vpcmpnleud` ... are the
  compare with that predicate. Instructions: the
  general-purpose ones a program runs (moves, arithmetic, logic, shifts and rotates, bit
  operations, `cmovcc`, `setcc`, jumps, calls, `push`/`pop`, string instructions, `xchg`,
  `xadd`, `cmpxchg`, `cmpxchg8b`/`16b`, `bswap`, `cpuid`, `rdtsc`/`rdtscp`, `syscall`,
  fences, `pause`, `ud2`, `crc32`, `movbe`, `rdrand`/`rdseed`, prefetches, the multi-byte
  `nop`s ...), BMI1 and BMI2, the x87 FPU, MMX, SSE2 through SSE4.2, AVX and AVX2, FMA,
  F16C, the AVX-512 F/VL/BW/DQ instructions (with masking `{%k1}{z}`, broadcast
  `{1to16}` and embedded rounding `{rn-sae}`; the `ymm`/`zmm`/`k` registers), and the
  system instructions as plain encodings (`cli`, `wrmsr`, `mov` to and from control and
  debug registers, `iretq`, `swapgs`, ...). The instruction tables are generated from
  Intel's SDM by `cmd/internal/tools/gen_x64_table.py`.
- **AArch64**: the base integer instructions (with their aliases: `mov`, `cmp`, `lsl`,
  `ubfx`, `cset` ...), branches, loads and stores in every addressing form (pairs,
  exclusives, acquire/release, the LSE atomics), `crc32`, pointer authentication and
  `bti`, `svc`, `brk`, hints, barriers, `mrs`/`msr` of the system registers by name (a
  table generated by `cmd/internal/tools/gen_a64_sysregs.py`, and the generic
  `s<op0>_<op1>_c<n>_c<m>_<op2>` form), `msr <pstatefield>, #imm`, scalar floating
  point, and Advanced SIMD in full from a table of instruction forms generated by
  `cmd/internal/tools/gen_a64_table.py`, each with exactly its valid arrangements: vector
  and scalar, integer and floating point in single, double and half precision (three
  same, three different, two-register miscellaneous, across lanes, pairwise, shifts by
  immediate, permutes, `ext`, `tbl`/`tbx`, by element), the dot-product (`sdot`, `udot`,
  `usdot`, `sudot`), matrix-multiply (`smmla` ...), complex-number (`fcmla`, `fcadd`),
  BF16, `fmlal`-class and `pmull` instructions; the structure loads and stores
  `ld1`-`ld4`, `st1`-`st4` and `ld1r`-`ld4r` of whole registers and single lanes in
  every addressing form; the modified immediates (`movi`, `mvni`, `orr`, `bic`, `fmov`
  of a vector constant); and the copy instructions (`dup`, `ins`, `umov`, `smov` and
  their `mov` aliases); the cryptographic instructions (AES, SHA1, SHA-256, SHA-512,
  SHA3's `eor3`/`rax1`/`xar`/`bcax`, SM3, SM4). The rest of the base and system set:
  pointer authentication in full (`pacia`...`autdb` and their `z` forms, `pacga`,
  `xpaci`/`xpacd`, the `1716`/`sp`/`z` hints, `retaa`/`retab`, `braa`...`blrabz`,
  `eretaa`/`eretab`, `ldraa`/`ldrab`), the unprivileged `ldtr`/`sttr` family, RCpc and
  LSE2 (`ldapur*`/`stlur*` of general and of b-q registers, `ldiapp`/`stilp`, `stlr` and
  `ldapr` with writeback, `ldap1`/`stl1`), `casp`, MTE (`irg`, `gmi`, `addg`, `subg`,
  `subp`, `subps`, `cmpp`, `stg`, `stzg`, `st2g`, `stz2g`, `stgp`, `ldg`, `ldgm`,
  `stgm`, `stzgm`), the memory-copy and memory-set instructions (`cpy[f]{p,m,e}` with
  every option, `set[g]{p,m,e}[t][n]`; the registers checked distinct), the 128-bit
  atomics (`ldclrp`, `ldsetp`, `swpp`), read-check-write (`rcw[s]{cas,clr,set,swp}[p]`),
  CSSC (`abs`, `cnt`, `ctz`, `smax`...`umin` of registers and immediates), the SYS
  aliases by operation name (`at`, `dc`, `ic`, `tlbi` with the range and nXS forms,
  `tlbip`, `cfp`/`dvp`/`cosp`/`cpp`; a table generated by
  `cmd/internal/tools/gen_a64_sysops.py`), `sys`, `sysl`, `sysp`, `mrrs`, `msrr`,
  `dsb` with nXS, `wfet`/`wfit`, the flag manipulation instructions (`cfinv`, `rmif`,
  `setf8`/`setf16`, `xaflag`, `axflag`), transactional memory (`tstart`, `tcommit`,
  `tcancel`, `ttest`), the guarded control stack's (`gcspushm`, `gcspopm`, `gcsss1`,
  `gcsss2`, `gcspushx`, `gcspopx`, `gcspopcx`, `gcsstr`, `gcssttr`, `gcsb dsync`), the
  64-byte accesses (`ld64b`, `st64b`, `st64bv`, `st64bv0`), `chkfeat`, `trcit`,
  `clrbhb`, `sb`, `ssbb`, `pssbb`, `psb csync`, `tsb csync`, `hvc`, `smc`, `dcps1`-`3`,
  `smstart`/`smstop`, the fixed-point `scvtf`/`ucvtf`/`fcvtzs`/`fcvtzu` and
  `frint32z`-class instructions, `fjcvtzs`. A name that reads as a register where an
  address must be (`b x1`, `ldr x0, s1`) is a symbol. The literal pool: `ldr Rt, =value`
  (x, w, s, d, q registers and `ldrsw`) loads a literal of its section's pool, placed at
  `.ltorg`/`.pool` or at the section's end, one entry per value and size, each aligned
  to its size (in inline asm the pool follows the template, with a branch around it).
  Immediates with or without `#`. The relocation operators of the ELF for the Arm
  64-bit Architecture ABI: `:lo12:` (in `add` and in loads and stores of every size),
  `:pg_hi21:`, `:pg_hi21_nc:`, `:got:` (in `adrp` and in a literal `ldr`), `:got_lo12:`,
  the move-wide groups `:abs_g0:`-`:abs_g3:` with `_nc` and `_s`, `:prel_g0:`-`:prel_g3:`
  with `_nc`, and the thread-local ones: `:tprel_hi12:`, `:tprel_lo12:`,
  `:tprel_lo12_nc:`, `:tprel_g2:`-`:tprel_g0_nc:`, the same of `:dtprel_...:`,
  `:gottprel:`, `:gottprel_lo12:`, `:gottprel_g1:`, `:gottprel_g0_nc:`, `:tlsdesc:`,
  `:tlsdesc_lo12:`, `:tlsdesc_off_g1:`, `:tlsdesc_off_g0_nc:`, `.tlsdesccall`, `:tlsgd:`
  and `:tlsgd_lo12:` (the ELF linker's general-dynamic sequence); the
  Mach-O spellings `sym@PAGE`, `@PAGEOFF`, `@GOTPAGE`, `@GOTPAGEOFF` (the same
  relocations), and COFF's `:secrel_lo12:`/`:secrel_hi12:` (a thread-local's offset in
  its block). `adr`, `adrp`, `b`, `bl`, `b.cond`, `cbz`, `cbnz`, `tbz`, `tbnz`, `prfm`
  and literal loads reach symbols of other sections and units (and `adrp`/`adr` labels
  of the template) through relocations. The linkers apply them all, with the ABI's
  range checks as link errors; an executable's own thread-locals are reached by local
  exec, the initial-exec and TLS-descriptor sequences relaxed to it (an initial-exec
  `ldr` whose register is not its base's cannot be, and is an error), and `:dtprel_*:`
  is the offset in the module's block. The absolute move-wide groups and `.word`/
  `.hword` of addresses are the ELF linker's only (Mach-O and PE images are
  position-independent). An operator in an instruction it does not fit is an error
  (where the conventional assembler makes some other relocation of it). SVE and SVE2
  (the `z0`-`z31`, `p0`-`p15` and `ffr` registers, the element-size suffixes, `/m` and
  `/z` predication): the integer arithmetic (predicated and unpredicated, the immediate
  and by-element forms), the logical, shift, compare, multiply, reduction, permute,
  move (`dup`, `cpy`-class `mov`, `insr`, `movprfx`), index and bitwise-immediate
  instructions; the predicate operations (`ptrue`/`ptrues` with every pattern, `pfalse`,
  `ptest`, `pfirst`, `pnext`, the predicate logical ops, `brk*`, `rdffr`/`wrffr`/`setffr`,
  the `while*` comparisons, `cntp`); the element counts (`cnt[bhwd]`, `inc*`/`dec*` with
  their saturating forms, `addvl`/`addpl`/`rdvl`); the floating point (arithmetic,
  compares, the fused multiply-adds, rounding, the estimates, complex `fcadd`/`fcmla`,
  and the conversions, the reductions and `fadda`); the dot products (`sdot`, `udot`,
  `usdot`, `sudot`, vector and by element), the matrix multiplies (`smmla`, `ummla`,
  `usmmla`, `fmmla`, `bfmmla`) and BF16 arithmetic; `adr` with vector bases; every load,
  store and prefetch: `ldr`/`str` of a `z` or `p` register, the contiguous `ld1`/`st1`
  (with the sign-extending forms), the gathers and scatters (scalar plus vector with
  32-bit `uxtw`/`sxtw` or 64-bit offsets, scaled and unscaled, and vector plus
  immediate), the first-fault `ldff1*` and non-fault `ldnf1*` loads, the structure
  loads and stores `ld2`-`ld4`/`st2`-`st4`, the non-temporal `ldnt1*`/`stnt1*` (with
  SVE2's vector plus scalar), the replicating `ld1r*`, `ld1rq*` and `ld1ro*`, and
  `prfb`/`prfh`/`prfw`/`prfd` in all their addressing forms; and SVE2: the widening,
  narrowing and long arithmetic (`saddlb`, `sabdlb`, `smullb`, `addhnb`, `shrnb`,
  `sqxtnb`, ...), the saturating and rounding forms (`sqadd` predicated, `sqdmulh`,
  `sqrdmlah`, the `sqshl` family, ...), the pairwise ops, `match`/`nmatch`,
  `histcnt`/`histseg`, `bdep`/`bext`/`bgrp`, the polynomial (`pmul`, `pmullb`,
  `eorbt`), complex integer (`cadd`, `sqcadd`, `cmla`, `sqrdcmlah`, `cdot`) and
  crypto (`aese`, `aesmc`, `sm4e`, `rax1`, ...) instructions, `xar`, `eor3`, `bcax`, the
  `bsl` family, `sqabs`/`sqneg`, `urecpe`/`ursqrte`, `flogb` and the SVE2 conversions.
  The `mov` aliases follow the architecture's preferred-alias rules (`mov Zd.T, #imm`
  stands for `dupm` only where no `dup` immediate gives the value). Not yet: SVE2.1 and
  SME2's additions (predicate-as-counter `pn` registers, register-pair `while*`,
  `psel`, `*clamp`, the two-way dot products). Not yet either:
  `@TLVPPAGE`/`@TLVPPAGEOFF` (Mach-O's thread-local descriptors), `:tlsldm:` (local
  dynamic's module base), `:gottprel_g1:`/
  `:gottprel_g0_nc:` and `:tlsdesc_off_*:` at link time (they assemble; the linkers
  reject them).
- Directives: `.byte`, `.short`/`.hword`/`.2byte`, `.word` (2 bytes on x86-64, 4 on
  AArch64), `.long`/`.int`, `.quad`/`.xword`, `.octa`, `.ascii`, `.asciz`/`.string`,
  `.float`, `.double`, `.uleb128`, `.sleb128`, `.zero`/`.skip`/`.space`, `.fill`,
  `.balign`, `.p2align`, `.align` (code padding uses the multi-byte NOPs), `.set`/`.equ`,
  `.equiv` and `=`, `.inst` (AArch64); the assembler language `.macro`/`.endm` (arguments,
  defaults, `:req`, `:vararg`, `\@`), `.rept`, `.irp`, `.irpc`, `.if`/`.ifdef`/`.ifc`/
  `.else`/`.elseif`/`.endif`; in file-scope asm also `.text`, `.data`, `.bss`,
  `.section` (with flags and types), `.pushsection`, `.popsection`, `.previous`,
  `.subsection`, `.globl`, `.weak`, `.local`, `.hidden`, `.protected`, `.type`, `.size`,
  `.comm`, `.lcomm`, `.incbin` (within the include directories). Labels: `name:` and
  numeric local labels (`1:`, `1b`, `1f`).
  Relocations: x86-64 rel32 calls and jumps to symbols (`@PLT`), `sym(%rip)`
  (`@GOTPCREL`), absolute 32- and 64-bit addresses; AArch64's operators above,
  `.quad` and `.word` of addresses. `.file`, `.loc`, `.ident`, `.arch` and the like are
  accepted and do nothing. An instruction, operand form or directive that is not
  supported is C3097, naming it, at the statement.
- Call frame information: in file-scope asm `.cfi_startproc [simple]` ... `.cfi_endproc`
  make a procedure whose `.cfi_def_cfa`, `.cfi_def_cfa_register`, `.cfi_def_cfa_offset`,
  `.cfi_adjust_cfa_offset`, `.cfi_offset`, `.cfi_rel_offset`, `.cfi_restore`,
  `.cfi_undefined`, `.cfi_same_value`, `.cfi_register`, `.cfi_remember_state`,
  `.cfi_restore_state`, `.cfi_escape`, `.cfi_signal_frame` (a CIE with the augmentation
  `S`) and on AArch64 `.cfi_negate_ra_state` are its FDE in `.debug_frame` under `-g`
  (registers by name or DWARF number); `.cfi_personality` and `.cfi_lsda` only with
  the encoding 255 (none: there are no personality routines), `.cfi_sections` changes
  nothing. Without `-g` there is no call frame information for any function (the
  stack traces walk frame pointers), and none is in `.eh_frame`: nothing in a program
  unwinds by it. On Windows, `.seh_proc` ... `.seh_endproc` make the procedure's unwind
  record in `.xdata` and its `.pdata` entry: x86-64 `.seh_pushreg`, `.seh_stackalloc`,
  `.seh_setframe`, `.seh_savereg`, `.seh_savexmm`, `.seh_pushframe`,
  `.seh_endprologue`; ARM64 `.seh_stackalloc`, `.seh_save_fplr`, `.seh_save_fplr_x`,
  `.seh_save_reg`, `.seh_save_reg_x`, `.seh_save_regp`, `.seh_save_regp_x`,
  `.seh_save_lrpair`, `.seh_save_freg`, `.seh_save_freg_x`, `.seh_save_fregp`,
  `.seh_save_fregp_x`, `.seh_set_fp`, `.seh_add_fp`, `.seh_nop`, `.seh_save_next`,
  `.seh_pac_sign_lr`, `.seh_endprologue`, `.seh_startepilogue`, `.seh_endepilogue` (each
  directive one instruction's code, as the ARM64 format counts them); exception
  handlers (`.seh_handler`) are not supported.

Under `--c-asm-dialect=intel` an asm statement's template, its `{att|intel}`
alternatives giving their second, is in the Intel syntax, and so are its operands as
they are written into it: registers without `%`, constants without `$`, an address
constant as `OFFSET FLAT:sym` (with `%a`, `sym[rip]`), memory as `DWORD PTR [rbp-8]`
with the size keyword of its object's size (1, 2, 4, 8 or 16 bytes, `TBYTE` for the
80-bit long double; none for another size or with `%a`), `%A` without `*`, `%z`
nothing; file-scope asm starts in the Intel syntax too. Without the option `{att|intel}`
gives its first alternative.

`asm [volatile] [inline] [goto] ("template" : outputs : inputs : clobbers : labels)`, as
The GNU dialect's documentation describes extended asm, with these rules:

- Operands are numbered outputs first, then inputs, then (for `%l`) labels; `%[name]`
  names one. An output's constraint starts with `=` or `+` (also an input: it is read and
  written), and its operand is a modifiable lvalue; `&` is early clobber. Matching
  constraints (`"0"`, `"[name]"`) tie an input to an output in a register.
- Constraints: of several alternatives (`"r,m"`, `"=r,r" : "!r,m"`) one is chosen for
  all the operands, the one they fit at the least cost: for each operand a constant
  where the alternative allows it 0, a register 1, memory 2, and each `?` 1 and `!` 600;
  the first of the cheapest (an alternative an operand cannot take is never chosen;
  operands with different numbers of alternatives are C3096). Letters: `r`, `g`, `X`, `m`, `o`, `V`, `i`, `n`,
  `s`, the digits; x86-64 `a b c d S D` (those registers), `A`, `q`, `R`, `Q`, `U`, `l`, `x`,
  `v`, `Y`, `I J K M N O e Z` (constants in their ranges); AArch64 `w` (any v register),
  `x` (v0-v15), `y` (v0-v7), `Q`, `Ump`, `Umq` and the SVE memory forms `Utf`, `Utr`,
  `Utv`, `Utq`, `Uty` (memory: an operand is always `[xn]`, which each of those forms
  takes), `I J K L M N Z`, `Y` and `Dz` (the constant zero, also the floating `0.0`,
  written `#0.0`), `Dn Dh Dq Ds Dd Dl Dr` and `Uc0`-`Uc2` (integer constants: the
  instruction that uses one checks it), `S`, `Usa` and `Ush` (a symbol's address, plus a
  constant), and `Upa` (any SVE predicate register), `Upl` (p0-p7). An operand
  chooses a constant when it is one (and the constraint allows it), else a register, else
  memory. A register operand is an integer, pointer or floating value of 1, 2, 4 or 8
  bytes (a floating value in a general register is its bits, an integer in a vector
  register too). On x86-64 also `Yz` (xmm0), and operands in registers the compiler does
  not allocate, whose values go through memory: the moves are written around the template
  (from the operand's memory to the register before it, back after it): `k` (an AVX-512 mask
  register, of an integer of 1 to 8 bytes: `kmov`), `y` (an MMX register, of 4 or 8
  bytes), and the x87 stack's `t` (st(0)), `u` (st(1)) and `f` (any), of `float`,
  `double` and the 80-bit `long double`, with the stack's rules as the dialect's documentation
  gives them: the inputs are loaded so that `t` is on top and `u` below it (`f` after
  them), an output tied to an input (`"=t"(r) : "0"(x)`) replaces it, the inputs the
  template pops are clobbered (`"st(1)"`) and must be the topmost (C3096 otherwise; also a
  `u` operand without a `t` one), the outputs are popped after it, `t` first, and then the
  inputs the template left on the stack. An `asm goto` cannot have such outputs. `A`:
  a value of up to 64 bits in rax, a 128-bit integer (`__int128`) in the rdx:rax pair,
  rdx its high half (an output, an input, `+`, or tied with a matching constraint to an
  output in the pair); the template names the pair as rax. On AArch64 likewise the
  registers the compiler does not allocate: an object that is no scalar of 1, 2, 4 or 8
  bytes (a vector, an array in a structure, a `long double`, ...) under `w`, `x` or `y` is
  in a vector register through memory: of up to 16 bytes with `ldr q`/`str q` (no SVE
  needed; written `v<n>` without a modifier), of up to 256 (SVE's largest vector) in an
  SVE `z` register with the whole-register `ldr z`/`str z` (written `z<n>`); `Upa` and
  `Upl` put an object of up to 32 bytes in an SVE predicate register (`ldr p`/`str p`).
  Each is a compiler temporary of the register's largest size (16, 256 or 32 bytes),
  the value at its start and zeros after it; the `z` registers are chosen from z31 down
  (within `x`'s z0-z15 and `y`'s z0-z7) among those no clobber or other operand names,
  and are clobbered (their `v` registers keep nothing across the statement), the `p`
  registers from p15 (`Upl` p7) down. A `z<n>` clobber is its `v<n>`'s; `p<n>` and
  `ffr` keep the operands out of those registers. An `asm goto` cannot have such outputs.
- Explicit register variables, `register long x asm("rax")` (or `"x8"`, `"w19"`, `"d0"`
  ...): such a variable is in that register when it is an asm operand in a register (the
  usual way to pass system call arguments); elsewhere it is an ordinary variable. The stack
  and frame pointers, and on AArch64 x16, x17 and x18, cannot be.
- Clobbers: registers (with or without `%` on x86-64; `ymm0-15` and `zmm0-15` are their
  xmm register; `xmm16-31`, `ymm16-31`, `zmm16-31`, `k0-7` and `mm0-7`, which the compiler
  keeps no value in across an asm statement, are accepted; `st` and `st(n)` say which x87
  inputs the template pops), `"cc"` (always assumed on x86-64), `"memory"`; not the stack or frame pointer, nor on macOS and Windows for AArch64 x18,
  the platform's (C3096). A clobbered callee-saved register is saved by the function.
- Template modifiers: x86-64 `%b %w %k %q %h` (a register's widths, its high byte), `%x`
  (xmm), `%t` (ymm), `%g` (zmm), `%c %P %p %a` (a constant without `$`, an address), `%n` (negated), `%A` (`*`),
  `%H` (memory 8 bytes on), `%V`, `%z` (the size suffix); AArch64 `%w %x` (also of the
  constant 0: `wzr`, `xzr`), `%b %h %s %d %q`, `%z` (a vector register as SVE's `z<n>`),
  `%c`, `%n`. Without a modifier an x86-64
  register is written at its operand's width, an AArch64 general register as `w` up to 32
  bits and `x` for 64, and a floating-point register as `s` or `d` by its type; an AArch64
  memory operand is `[xn]`. `%%`, `%=` (a number unique to each copy of the statement),
  `%l[label]`.
- `asm goto`'s template may jump to its labels (`%l[name]`); outputs are stored on every
  path. A jump out of a scope leaves it as `goto` would.
- What the optimizer may do with an asm statement (an asm without outputs, an `asm
  goto` and a basic asm are `volatile`; a basic asm clobbers memory):
  - not `volatile`, no `"memory"` clobber and no memory operands (nor operands in mask,
    MMX or x87 registers, which go through memory, nor the rdx:rax pair): a function of
    its inputs. It is removed when its outputs are unused, merged with an equal one (the
    same template, constraints, operands' kinds and clobbers) on the same inputs, and
    moved (out of a loop too, so that it may run before the loop's first iteration even
    when that never comes); it is never run in a branch it was not in.
  - `volatile`, no `"memory"` clobber and no memory operands: kept where it is, once
    each time it is reached, but no barrier to memory: values in memory may stay in
    registers across it.
  - with memory operands and no `"memory"` clobber: kept where it is, reading exactly
    its input memory operands' objects and writing its output ones' (`+m` both), as the
    dialect's rule says, and no other memory: values of other objects stay in registers
    across it, stores to them before it may be removed when later ones overwrite them,
    and the objects it names keep their earlier stores and are loaded again after it
    (value numbering, load forwarding, dead stores and keeping loop values in registers
    reason this way; the other passes, the vectorizer among them, treat it as below).
    An operand that goes through memory (mask, MMX, x87, SVE and AArch64 vector
    operands) is the compiler's own temporary, which only it reads and writes. An
    `asm goto` and a template that calls a function are as below.
  - with a `"memory"` clobber (and a basic asm): kept where it is, and as reading and
    writing any memory (stores before it are done, loads after it done again).
  An output in a register is the statement's result in that register, which the register
  allocator places (no store and load around it), then assigned to its lvalue (in
  `asm goto`, on every path, the label's too). An early-clobber output (`&`) is in no
  input's register; another may be in an input's register (the template must read its
  inputs before it writes such an output, as the dialect says).
- The template must leave the stack pointer where it was (pushes and pops in pairs are
  fine) and must not change the frame or the frame pointer. A function whose asm uses the
  stack (an instruction pushes, pops, calls or names the stack pointer) has a frame, its
  CFA the frame pointer's (rbp + 16, x29 + 16), so its call frame information and stack
  traces stay exact while the template moves the stack pointer; on Windows the frame
  pointer is its unwind information's frame register. In a template
  `.cfi_adjust_cfa_offset` says how far it has moved the stack pointer (in all, they must
  add up to 0: C3097), `.cfi_rel_offset reg, off` that it saved a register at the stack
  pointer + off (the function's FDE then gives the register that rule, as an expression
  on the stack pointer that follows the template's later adjustments) and `.cfi_restore`
  that the register has its value again (each saved register must be restored); the
  other `.cfi_*` directives, which would change the frame, are C3097 there, and the
  `.seh_*` ones too (the Windows unwind codes describe prologues only, so on Windows a
  register a template saves is not described). A template that calls a function aligns
  the stack itself (as with other compilers). A function whose asm calls or uses the stack
  makes no sibling calls.
- Symbols and sections: a template's named labels (but `.L` and numeric ones) are local
  symbols of the unit's object, its `.globl` and `.weak` ones global and weak symbols
  (`.type` and `.size` apply); C (an `extern` declaration), other templates and
  file-scope asm refer to them by name, as to file-scope asm's labels. A template's
  `.pushsection`/`.popsection`, `.section` ... `.previous` (back to `.text`: by
  `.popsection` or `.previous`) put its bytes in the object's sections of those names,
  after what is there, with relocations to the template's labels (in its function's
  code) and to symbols; numeric labels (`1:`, `1f`) refer across them. A template that
  defines symbols is never copied by the optimizer (its function is not inlined, its
  loop not unrolled, its block not duplicated), so each label has one definition; a
  name defined twice that something refers to is an error. A template or file-scope asm
  that names a function or object of internal linkage (or a block's static object)
  visible where it is refers to it (to the symbol the compiler gives it, made unique in
  the program), which is kept though only the text refers to it; any other symbol it
  names has external linkage (or is defined by asm), and is kept in the program.
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
| `__builtin_object_size(p, t)`, `__builtin_dynamic_object_size` | The bytes from p to the end of the object (t 0, 2) or of the closest member (t 1, 3; a member array at the end of its structure counts to the object's end), when the expression shows the object: a declared object of a complete non-VLA type, a string literal or a compound literal, its address, its array decayed, an element at a constant index, a member, or such a pointer converted or offset by a constant (0 past the end). Otherwise unknown: `(size_t)-1` for t 0 and 1, 0 for 2 and 3 (no data flow is followed; an object whose type has a flexible array member is unknown). |
| `__builtin_clz`, `ctz`, `popcount`, `parity`, `ffs`, `clrsb`, with `l` and `ll` | Of `unsigned int` (ffs, clrsb: `int`) ... `unsigned long long`; clz and ctz of 0 give the type's width. |
| `__builtin_bswap16`, `bswap32`, `bswap64` | |
| `__builtin_add_overflow`, `sub_overflow`, `mul_overflow(a, b, r)` | As `ckd_add(r, a, b)`; the typed `__builtin_sadd_overflow` ... `umulll_overflow`; `__builtin_add_overflow_p(a, b, c)` ... (whether the result overflows c's type). |
| `__builtin_memcpy`, `memmove`, `memset` | The IR's operations; their value is the destination. |
| `__builtin_abs`, `fabs`, `sqrt`, `floor`, `ceil`, `trunc`, `rint`, `nearbyint`, `copysign`, `fma` (with `f` and `l`; `fabs` and `copysign` also with `f16`, `f32`, `f64`, `f128`, `f32x`, `f64x` and `q`) | The IR's operations (`sqrt` of a negative number is a NaN; errno is not set; `fma` on WebAssembly calls the library's). Where `long double` is wider than `double`, the `l` forms but `fabsl` and `copysignl` (which change the sign bit) call the library's function of that name. |
| `__builtin_memcmp`, `strlen`, `strcmp`, `strncmp`, `strcpy`, `strncpy`, `strcat`, `strncat`, `strchr`, `strrchr`, `strstr`, `memchr`, `printf`, `sprintf`, `snprintf`, `puts`, `putchar`, `malloc`, `calloc`, `realloc`, `free`, `abort`, `exit`, `labs`, `llabs`, and the math functions (`sin`, `cos`, `tan`, `exp`, `log`, `pow`, `round`, `fmin`, `fmax`, `fmod`, `hypot`, `ldexp`, `frexp`, `modf`, `lround` ... with `f` and `l` where the library has them) | Calls of the C library's function of that name. |
| `__builtin_alloca(n)`, `__builtin_alloca_with_align(n, bits)` | n bytes of the function's frame, until it returns. |
| `__builtin_frame_address(n)`, `__builtin_return_address(n)` | The frame record n links up the chain of frame records and its return address (natively: the compiler's functions keep frame records; a caller compiled without them gives what its frame holds, as with other compilers); on WebAssembly the stack pointer for level 0, else a null pointer. A level above 65535 is C3090. |
| `__builtin_setjmp(buf)`, `__builtin_longjmp(buf, 1)` | `<setjmp.h>`'s setjmp and longjmp with a `jmp_buf` in the calling function's frame, whose address `buf[0]` holds (buf is at least one pointer; five, as other compilers take, is fine): 0, then 1 after the jump; the jump is valid while that function runs. The second argument must be 1 (C3082). Not on WebAssembly (C3300; `__has_builtin` is 0 there). |
| `__builtin_cpu_supports("name")`, `__builtin_cpu_is("intel" or "amd")`, `__builtin_cpu_init()` | The running processor, as `std/os/cpu.ax` detects it (`@cpu_has`'s answer): a name of its table for the target's architecture (another architecture's is 0; an unknown one C3082), and on x86-64 also `cmov`, `mmx`, `sse`, `sse2`, `x86-64` (1) and the levels `x86-64-v2` ... `x86-64-v4`; an extension the build assumes is the constant 1; on WebAssembly every answer is the build's. `cpu_is` is x86-64's only (cpuid's vendor; another name is C3090); `cpu_init` does nothing (detection is done when first asked). |
| `__builtin_ia32_pause()`, `__builtin_ia32_mfence()`, `lfence`, `sfence` | On every target: nothing (a spin-wait hint), and a full memory fence. |
| `__builtin_shuffle(a, mask)`, `__builtin_shuffle(a, b, mask)` | The vector of a's (then b's) lanes that mask's lanes (an integer vector of as many lanes and the same lane size) select, modulo their count. |
| `__builtin_shufflevector(a, b, i, ...)` | The vector of the lanes the constant indexes select from a then b (-1: any value); as many lanes as indexes. |
| `__builtin_convertvector(v, T)` | v's lanes converted to vector type T's element type (as a cast of each), T of as many lanes. |
| `__builtin_prefetch(p, ...)`, `__builtin_assume_aligned(p, a [, o])` | Nothing; p. |
| `__builtin_inf`, `huge_val`, `nan("")`, `nans("")` (with `f`, `l`, `f16`, `f32`, `f64`, `f128`, `f32x`, `f64x` and `q`) | Constants (a NaN's payload is not kept; a signaling NaN is quiet; a NaN is positive). |
| `__builtin_isnan`, `isinf`, `isfinite`, `isnormal`, `signbit`, `isinf_sign`, `fpclassify(a, b, c, d, e, x)`, `isgreater`, `isgreaterequal`, `isless`, `islessequal`, `islessgreater`, `isunordered` | Of the floating type of the operands (an integer is a double). |
| `__sync_fetch_and_op`, `__sync_op_and_fetch` (op: add, sub, or, and, xor, nand), `__sync_bool_compare_and_swap`, `__sync_val_compare_and_swap`, `__sync_lock_test_and_set`, `__sync_lock_release`, `__sync_synchronize` | Atomic, sequentially consistent. |
| `__atomic_load_n`, `store_n`, `exchange_n`, `compare_exchange_n`, `fetch_op`, `op_fetch`, `test_and_set`, `clear`, `thread_fence`, `signal_fence`, the generic `load`, `store`, `exchange`, `compare_exchange`, `always_lock_free`, `is_lock_free` | A constant memory order (`__ATOMIC_RELAXED` ... `__ATOMIC_SEQ_CST`, hint bits above the low 16 ignored) is the operation's, as for C11's atomics; another is `__ATOMIC_SEQ_CST`. The weak argument of `compare_exchange` makes the weak form when it is a nonzero constant. `__sync_lock_test_and_set` is an acquire, `__sync_lock_release` a release. |

The atomic builtins take a pointer to an integer or pointer type of 1, 2, 4 or 8 bytes,
aligned to its size (the generic ones: to any complete object), and are the operations
of C11's atomics (see "Atomics and threads"; nand is a compare-and-swap loop): lock-free
on objects of 1, 2, 4 and 8 bytes on every target, of 16 aligned to 16 where `_Atomic`
ones are, and for other sizes (the generic forms) under the same table of locks as
`_Atomic` objects, so the two agree.

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
| `STDC FENV_ACCESS ON/OFF/DEFAULT` | Honoured: see "The floating-point environment". |
| `STDC CX_LIMITED_RANGE ON/OFF/DEFAULT` | Accepted (it allows, and asks for nothing). |
| `STDC FENV_ROUND dir` (C23) | Honoured, at translation and at run time ("The floating-point environment"); `FE_TONEARESTFROMZERO`, and a directed rounding on WebAssembly, are C4113. |
| `STDC FENV_DEC_ROUND dir` (C23) | Honoured, every direction, on every target. |
| `GCC diagnostic push`, `pop`, `ignored "-Wx"`, `warning "-Wx"`, `error "-Wx"` | The warnings after it (in its file, and in the files it includes) that -Wx names are not shown, shown, or errors (keeping their codes). -Wx is one of the compiler's codes (`"C4050"`), `-Wall`, `-Wextra` and `-Wpedantic` (every warning), or a usual name that stands for one: `-Wdeprecated-declarations` and `-Wdeprecated` (C4050), `-Wunused-result` and `-Wunused-value` (C4051), `-Wattributes` (C4052), `-Winvalid-noreturn` (C4053), `-Wimplicit-function-declaration` (C4001), `-Wunused-label` (C4040), `-Wvarargs` (C4080), `-Wmultichar` (C4102), `-Wcpp` (C4103), `-Wunknown-pragmas` (C4111), `-Wpragmas` (C4112), `-Wnonnull` (C4121), `-Wswitch-outside-range` (C4120), `-Wattribute-warning` (C4123), `-Wformat` (C4130 to C4135), `-Wformat-extra-args` (C4132), `-Wformat-nonliteral` and `-Wformat-security` (C4134), `-Wformat-signedness` (C4135); others are ignored. An error is never made a warning (nor a check `--c-warn-format` does not make enabled). |
| `GCC visibility push(v)`, `pop` | The declarations with external linkage up to the matching `pop` get visibility v (as the attribute, which overrides it). |
| `GCC poison x y` | A later use of x or y is the error C1086 (not one a macro defined before makes). |
| `GCC system_header` | The rest of the file gives no warnings (in the main file it means nothing). |
| `GCC warning "m"`, `GCC error "m"` | The warning C4103, or the error C1040. |
| `GCC push_options`, `pop_options`, `optimize`, `target`, `unroll`, `ivdep`, `novector`, `dependency` | Ignored: the optimization level and the extensions are the command line's for the whole program (a function that needs more extensions is Axiom's `@features`). |

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
| `__axiom_atomic_lock`, `__axiom_atomic_unlock`, `__axiom_atomic_locks` | The lock of atomic objects that are not lock-free. |
| `__builtin_inff`, `__builtin_nanf` | `<float.h>` (`NAN` is a positive quiet NaN). |
| `__builtin_axiom_fpclassify`, `__builtin_axiom_isnan`, ..., `__builtin_axiom_isgreater`, ... | `<math.h>`'s classification and comparison macros. |
| `__builtin_axiom_tg(n, f, d, l, cf, cd, cl, args...)` | `<tgmath.h>`: the function of the six (0 for none) that the types of the first n arguments choose (C17 7.25p3), called with the arguments. |
| `__builtin_axiom_fenv_kind()`, `__builtin_axiom_fenv_get()`, `__builtin_axiom_fenv_set(v)` | `<fenv.h>`: the target's floating-point environment (0 none, 1 MXCSR, 2 FPCR and FPSR), its registers read and written. |
| `__axc_...` | The C library's internal functions and objects. |
| `__builtin_complex`, `__builtin_creal`, `__builtin_cimag`, `__builtin_conj` | `<complex.h>`: `__builtin_complex(x, y)` is `x + iy` for `x` and `y` of one real floating type. |
| `__axiom_cmul_f32`, `__axiom_cdiv_f64`, ... | The compiler's own functions for complex multiplication and division (internal to the program). |
| `__axiom_int64` | A keyword for a 64-bit integer type (`unsigned __axiom_int64` too) in every mode, for `size_t` in headers before C99 and for programs that need one in C89. |
| `__axiom_decimal32`, `__axiom_decimal64`, `__axiom_decimal128` | Keywords for the decimal floating types in every mode (the C library's sources, which are C17). |
| `__builtin_axiom_dec(w, hi, lo)` | A constant of the decimal floating type of w bits with that encoding (BID): `<math.h>`'s and `<float.h>`'s infinities and NaNs. |
| `__AXIOM_...`, `__axiom_...` | Macros and members of the headers. |
| `__axc_...`, `__AXC_...` | The C library's internal names (`__axc_setjmp`, `__axc_errno`, ...), and its private headers `axc/*.h`. |

## Programs

A C program's `main` is called by a start-up module of the runtime, which the build adds.
It gives `main` its arguments (`argv[argc]` is null) and uses its result as the exit
status: it calls the C library's `exit` with `main`'s value ("Start-up and exit"),
which runs the `atexit` functions; in a freestanding program the start-up returns
`main`'s value, and the process ends with it (on WebAssembly, the host).

A program that defines `memcpy`, `memset` and `memmove` itself builds for every target.
Names that the runtime uses for its own functions (such as `ax_write`) cannot be
defined: `C3062` (reported by `build`).

## Atomics and threads

`_Atomic` qualifies a type, and `_Atomic(T)` names the same atomic type. Its layout is the
platform's C compilers' (checked against them: `c/run/atomic_layout_<target>.c`): on
Linux (x86-64 and AArch64, the psABIs' rule) an atomic type has the size of its
non-atomic version, and one of 1, 2, 4, 8 or 16 bytes is aligned to its size (so
`_Atomic struct { char c[2]; }` is aligned to 2); on macOS and Windows a size up to 16
bytes is also padded to the next of those sizes and aligned to it
(`_Atomic(struct of 12 bytes)` is 16 bytes aligned to 16, one of 3 bytes is 4), and on
WebAssembly the same up to 8 bytes (16 bytes keep their alignment); any other keeps its
size and alignment. The padding of an atomic object is zeros, which every operation
here writes. An atomic type cannot be an array or function type (`C3092`), `_Atomic(T)`
cannot be of an array, function, atomic or qualified type (`C3092`), and a bit-field
cannot be atomic (implementation-defined, 6.7.2.1p5: `C3070`). An atomic type is not a
qualified version of its non-atomic type: `int *` and `_Atomic int *` are incompatible
(`C3022`), and a parameter or result keeps its `_Atomic` in the function's type. A
member of an atomic structure or union accessed (undefined) is the warning `C4090`, and
`void *p = &atomic_object` the warning `C4091`.

Reading an atomic object (its value used), assigning it, `++`, `--` and the compound
assignments are atomic operations (6.5.2.4, 6.5.16.2), all sequentially consistent.
`+=`, `-=`, `&=`, `|=`, `^=`, `++` and `--` on an atomic integer or pointer of up to 8
bytes are one read-modify-write instruction; the others (`*=`, `/=`, shifts, floating
types, `_Bool`, 16-byte integers) are a loop of compare-exchanges that computes the new
value again if another thread changed the object meanwhile. There is no floating-point
environment, so the loop has no exceptions to discard (6.5.16.2p3, note).

**Memory orders.** The functions of `<stdatomic.h>` (and the GNU dialects' `__atomic`
builtins) have the order they are given when it is a constant: the compiler's IR carries
it, and the backends choose instructions by the C11 mappings. On x86-64 a store that is
not sequentially consistent is a plain `mov` (a sequentially consistent one `xchg`),
loads are plain `mov`s, read-modify-writes are locked instructions, and only a
sequentially consistent fence is `mfence`. On AArch64 a relaxed load or store is `LDR`
or `STR`, an acquire load `LDAR` (`LDAPR` where the target has RCpc: `--cpu=neoverse-n1`
and later), a sequentially consistent one `LDAR`, a release or stronger store `STLR`; a
read-modify-write's exclusive pair (`LDXR`/`LDAXR`, `STXR`/`STLXR`) or LSE instruction
(`LDADD`, `LDADDA`, `LDADDL`, `LDADDAL`, `SWP`..., `CAS`...) has the acquire and release
forms its order needs; an acquire fence is `DMB ISHLD`, a release, acq_rel or
sequentially consistent one `DMB ISH`. A compare-exchange's instruction is as strong as
both its orders. WebAssembly's atomic instructions are all sequentially consistent.
`memory_order_consume` is `memory_order_acquire`; an order that is not a constant (a
variable) is `memory_order_seq_cst`; a relaxed fence is nothing. `atomic_signal_fence`
is no instruction, but a barrier to the compiler (nothing is moved across it). The
optimizer treats every atomic operation and fence, whatever its order, as a sequentially
consistent one: it does not remove, merge or move memory accesses across any of them
(valid for every order, since a weaker one only allows more behaviours; it forgoes the
optimizations relaxed atomics would allow). An order that an operation does not take is
the warning `C4092`, and the operation is made at least as strong as asked: a store's
consume, acquire or acq_rel (7.17.7.1p2), a load's release or acq_rel (7.17.7.2p2), a
compare-exchange's failure order of release or acq_rel, and before C23 (and always for
the `__atomic` builtins) a failure order stronger than the success order (7.17.7.4p2),
and a value that is no order. `atomic_compare_exchange_weak` fails spuriously only on
AArch64 without LSE (one exclusive attempt, no loop: it returns false when the
exclusive store fails); elsewhere it is the strong form. Axiom's own atomics
(`@atomic_load` ...) have no orders (spec §12.8.3: sequentially consistent only).

**Lock-free sizes.** Objects of 1, 2, 4 and 8 bytes (every integer type and pointer) are
lock-free on every target: x86-64's locked instructions, AArch64's exclusive or LSE
instructions, WebAssembly's atomic instructions. Objects of 16 bytes aligned to 16 (a
structure of two `long long`, `__int128`, an 80-bit or 128-bit `long double`, `double
_Complex`) are lock-free on AArch64 (an exclusive pair `LDAXP`/`STLXP` loop; a load
stores back what it read, which makes the read single-copy atomic; LSE's `CASP` is not
used) and on x86-64 (`lock cmpxchg16b`): inline when the target's extensions have it
(`--cpu=x86-64-v2` and later, or `--features=+cmpxchg16b`); otherwise through a function
that asks the processor once (cpuid) and uses the instruction, or the lock where the
processor lacks it, so `atomic_is_lock_free` of such an object asks the processor (and
`__atomic_always_lock_free(16, 0)` is 0, `__atomic_is_lock_free(16, 0)` 0 too: they are
constants). Every 16-byte operation is a compare-exchange of 16 bytes or a loop of them,
so an atomic load writes the object (one in read-only memory faults). Other objects (a
structure of 3 bytes on Linux, or of 24) take one of 64 spinlocks, chosen by the
object's address, in a table that every module and object holding such code defines
once (linkonce). A signal handler must not touch such an object while the interrupted
thread may hold its lock. `ATOMIC_*_LOCK_FREE` are 2 (the types they name are 8 bytes at
most).

**Code from the platform's compilers.** It does atomic operations on objects that are not
lock-free by calls of the standard out-of-line interface: `void __atomic_load(size_t n,
void *p, void *ret, int order)`, `__atomic_store(n, p, val, order)`,
`__atomic_exchange(n, p, val, ret, order)`, `bool __atomic_compare_exchange(n, p,
expected, desired, success, failure)`. When such code may be linked into the program
(`--emit=obj`, or any `--link=` input or `-l` library, which the system's linker or ours
links),
this compiler's code calls them too for such objects, and defines them (weak, global: in
every object that calls them) over its lock table; they route sizes of 1, 2, 4, 8 and 16
bytes at addresses aligned to the size to the lock-free operations, as this compiler's
code does them. So both sides take the same lock: the program's definitions come before
the system's atomic library, which the program then does not need (and if an object of
ours is linked with that library and without our definitions, all calls go to the
library's). Of the sized forms, the 16-byte ones (`__atomic_load_16`,
`__atomic_compare_exchange_16`, `__atomic_fetch_add_16` and the other read-modify-writes)
are defined too in an executable linked with such code on the System V x86-64 and AAPCS64
targets, over the same 16-byte compare-exchange as this compiler's code, so that both
sides' operations on a 16-byte object are atomic with each other (Windows passes
`__int128` otherwise: not there); the smaller sizes are lock-free on both sides, which
share nothing but the object. The objects of every type are covered: the decimal floating
types, `_Float128` and the other `_FloatN` types, imaginary and vector types are accessed
as any other object of their size (lock-free at 1, 2, 4, 8 or 16 bytes aligned so). An
executable that only this compiler's code makes uses the lock directly.

`<stdatomic.h>` has all of 7.17. Its generic functions are the compiler's builtins
(`__builtin_atomic_load`, `__builtin_atomic_compare_exchange_weak` and the others);
atomic_flag's functions and the fences are `static inline` functions in the header and
also macros of the same names (so that a constant order stays one). On WebAssembly
without the atomics feature (`--features=-atomics`) a module has one thread, and the
operations are ordinary loads and stores.

`_Thread_local` (C23: `thread_local`) objects have one instance per thread, each starting
with the object's initial value (which may hold addresses). On Linux they are ELF TLS
(local-exec, so in an executable or an object linked into one); the system C library's
threads and the runtime's each get their copies. On Windows they are in the image's TLS
directory, so every thread has them. On macOS they are the runtime's thread-local
block, which the main thread and the runtime's threads have: every thread of
`thrd_create` is one of the runtime's (a thread something else creates, such as the
system's C code with `pthread_create`, has none). On WebAssembly, in a program
without threads they are ordinary globals; with threads, each thread's start makes its
block of them. `--emit=obj` objects for WebAssembly have them as ordinary globals.
The GNU dialects' `__thread` is `_Thread_local`.

### Threads

`<threads.h>` (C11 7.26, C17, and C23's changes: `thread_local` is a macro only before
C23, where it is a keyword; `call_once`, `once_flag` and `ONCE_FLAG_INIT` are also in
C23's `<stdlib.h>`) is the C library's threads part, on every target:
`std/c/lib/threads/<name>.c` over `std/c/rt/threads.ax` (mutexes, condition variables, `call_once`,
thread-specific storage, `thrd_current`, `thrd_sleep`, `thrd_yield`) and
`std/c/rt/threads_spawn.ax` (`thrd_create`, `thrd_join`, `thrd_detach`, `thrd_exit`),
which build on `std/thread` and Axiom's atomics and `@atomic_wait`/`@atomic_notify` (the
protocols of `std/sync`'s `Mutex`, `Condvar` and `Once`; `std/sync` itself is not
imported, as it would make every WebAssembly program that uses a mutex threaded).
`__STDC_NO_THREADS__` is not defined. The header's comment lists the choices; the
main ones:

- **Threads**: a thread of `thrd_create` is one of the Axiom runtime's (`std/thread.spawn`):
  natively the runtime's own (`clone` on Linux, `pthread_create` with the system C library
  or on macOS, `CreateThread` on Windows), on WebAssembly a worker on the shared memory.
  It has the program's thread-locals (and its own `errno`), the runtime's crash reporting
  and the allocator's per-thread caches. `thrd_t` points to the thread's record;
  `thrd_current` on a thread `thrd_create` did not start (the main thread, or one Axiom
  code started) gives a record of that thread's own, which cannot be joined by itself or
  detached (`thrd_error`).
- **Thread end**: the return of the start function, or `thrd_exit`, runs the thread's tss
  destructors (below), records the result for `thrd_join`, and ends the thread through
  `std/thread.exit` (the host's `ax_thread_exit`: the kernel's exit of the thread, or
  `pthread_exit`, `ExitThread`, or the end of the worker), which gives back its stack
  and records: at `thrd_join`, or for a detached thread when the next thread is created,
  detached or ends. `thrd_exit` leaves the C frames it is called in without running
  anything of them (as `longjmp` would); called in Axiom code that C called it leaves
  that code's `defer`s unrun too.
- **`thrd_exit` on the main thread** (7.26.5.5): its tss destructors run, its result is
  recorded (another thread may `thrd_join` it), then it waits until every thread
  `thrd_create` started has ended, joined or detached, and calls `exit(EXIT_SUCCESS)`
  (the `atexit` functions run then, on the main thread). The main thread does not end
  before the others; threads that Axiom code started (`std/thread.spawn`) are waited
  for too (`std/thread.wait_all`).
- **`exit` from any thread**: the first thread to call it runs the `atexit` functions
  (once), the destructors and the flushing, and ends the process with its status; a
  second thread's `exit` meanwhile waits for the end. The end is at once, whatever the
  other threads do, on WebAssembly too: the host ends the program from any thread
  (`ax_exit`, spec §12.8.5, "Ending the program"); so do `_Exit`, `quick_exit` and
  `abort`'s.
- **Mutexes**: `mtx_plain`, `mtx_timed`, and either with `mtx_recursive`; another type
  is `thrd_error`. A futex-style lock (no system call unless contended; while no thread
  has been started, not even an atomic instruction); `mtx_timedlock` takes any mutex.
  `mtx_unlock` of a mutex that is not locked (a recursive one: by the caller) is
  `thrd_error`, as is `cnd_wait` with one.
- **Time**: the deadlines of `mtx_timedlock` and `cnd_timedwait` are `TIME_UTC` (the
  real-time clock) times, checked against it after each wake; one that has passed is a
  timeout at once; a `timespec` whose `tv_nsec` is outside 0..999999999 is `thrd_error`.
  `thrd_sleep` returns 0, or on Linux and macOS -1 when a signal handler ran and ended
  it early, storing the time left in `*remaining` (if not null), or -2 for a negative or
  invalid duration (Windows and WebAssembly have no signals that end it); `thrd_yield`
  is `sched_yield` (`SwitchToThread` on Windows; on WebAssembly a sleep of no time).
- **Thread-specific storage**: any number of keys; a key reused after `tss_delete` does
  not see the values of the deleted one. At a thread's end, for each key with a
  destructor whose value is not null, in the order of the keys, the value is set to null
  and the destructor called with it; again while the destructors set values, at most
  `TSS_DTOR_ITERATIONS` (4) rounds. They do not run at `exit` or `main`'s return
  (7.26.6.1). A key used after `tss_delete` (undefined, 7.26.6.3) is defined here:
  `tss_get` gives null and `tss_set` `thrd_error`, in every thread.
- **Stacks**: a thread of `thrd_create` has 8 MiB of stack on Linux (the runtime's own
  threads: below it a guard region of 64 KiB that is never mapped, and a signal stack
  of its own for the report of a stack overflow; with the system C library linked, its
  threads' default stack and guard), macOS (8 MiB, `pthread_attr_setstacksize`; the
  system's guard page) and Windows (8 MiB reserved); on WebAssembly 1 MiB of the
  memory, which the compiler's stack checks guard. There is no option to change it.
  Ended threads' stacks are kept for the next ones (up to four, Linux), so that a
  thread's start and end take no system calls but its creation's and the waits'.
- **Mutexes** are not fair (a thread that unlocks may take the mutex again before a
  waiter woken for it) and not robust (one that a thread holds when it ends stays
  locked; there is no owner-death detection); a recursive one counts its owner's locks.
- **Without threads**: on WebAssembly without the atomics extension (`--features=-atomics`)
  `thrd_create` returns `thrd_error`, `thrd_join` and `thrd_detach` too
  (`threads_spawn_none.ax` is linked instead), and the rest works on the one thread. A
  program that uses `<threads.h>` but never creates a thread (no `thrd_create` or
  `thrd_exit`) has no shared memory on WebAssembly; its waits, which nothing can end
  early, sleep until their deadlines.

The library is thread-safe where the standard asks: each stream has a lock, which
every standard I/O function takes for its whole operation (formatted output to one
stream from many threads comes out a call at a time); `malloc` and its kin are the Axiom
allocator's, which is thread-safe (or the system C library's); `errno` is each thread's;
`atexit` and `at_quick_exit` may be called from any thread (their table has a lock);
`getenv`'s strings are kept under a lock; the library's own lazy initializations (the
time zone, the wide character classes' table, the standard streams' first use) are
done once, whichever threads ask at once, at the cost of an atomic load once done.
What the standard lets race, races: `rand` (one state, 7.22.2.1p3), `strtok`, `asctime`,
`ctime`, `gmtime`, `localtime` and `strerror`'s static results, `setlocale` and
`localeconv` (7.11.1.1p5), `signal` (undefined in a program with threads, 7.14.1.1p7),
`tmpnam` with a null argument. The multibyte functions' internal states (`mblen`,
`mbtowc`, `wctomb`, and the restartable ones with a null state pointer) are each
thread's own.

Measured on x86-64 Linux against the system's C library and its threads (at -O2, a
loaded machine, best of five runs): an uncontended `mtx_lock` and `mtx_unlock` take
about 0.25 times its time in a program that has not started a thread and about 1.1
times once one has; a `thrd_create` and `thrd_join` of a thread that returns at once,
about the same processor time (0.97 times, down from 1.8: cached stacks, and wake-ups
only when a thread waits).

## Calling conventions and interoperability

C functions use the target's C ABI, for structures and unions passed and returned by
value and for variadic functions as well, so that C compiled here calls the platform's C
library and is called by it:

| Target | ABI |
|---|---|
| `x86_64`, `x86_64-macos` | x86-64 System V. |
| `x86_64-windows` | Windows x64. |
| `aarch64` | AAPCS64. |
| `aarch64-macos` | Apple's variant of AAPCS64. |
| `aarch64-windows` | Windows ARM64: AAPCS64, but for variadic functions (below). |
| `wasm32`, `wasm64` | The WebAssembly C ABI. |

A complex value is passed and returned as the structure of its two parts would be, as
each ABI says (`float _Complex` in one SSE register and `double _Complex` in two on
x86-64 System V, a homogeneous aggregate in two floating-point registers on AArch64, by
value in an integer register or by reference on Windows, by reference on WebAssembly);
checked against the system's C compiler on `x86_64` and `aarch64` Linux.

An integer wider than 64 bits travels as the structure of its 64-bit limbs would, aligned
as it is (16 bytes for `__int128` everywhere and for `_BitInt` on AArch64, else 8): on
x86-64 System V in two integer registers when it is 16 bytes (both or none: the psABI's
INTEGER, INTEGER; on the stack aligned as it is), in memory when larger; on AArch64 in a
pair of `x` registers that starts at an even-numbered one (AAPCS64's rounded NGRN; the
stack aligned to 16), by reference when larger; on Windows x64 by reference (not 1, 2, 4
or 8 bytes), its result through the hidden pointer, but an `__int128` result in `xmm0`
(the platform's convention for that type); on WebAssembly as two `i64`s when it is 16
bytes, by reference when larger, its result through a pointer first parameter (variadic:
in the buffer, at the type's alignment). Variadic arguments are read the same way.
`__int128` is checked against the system's C compiler on `x86_64` and `aarch64` Linux,
both ways (`c/run/wint_abi_host.c`, and `gen_c_abi.py --int128`). `_BitInt(65)` to
`_BitInt(128)` (`gen_c_abi.py --bitint=128`) were compared with another compiler on
x86-64 (Linux), both ways; wider ones only where that compiler agrees with the psABI (it
lays out arrays of them with another stride), and on AArch64 the alignment of 16 follows
AAPCS64, which that compiler does not. The Windows and WebAssembly signatures were compared
with another compiler's for those targets.

The decimal floating types (BID, as the ABIs of x86-64 and AArch64 say) travel as the
binary floating types of their sizes do, as types of their own: x86-64 System V's class
SSE for `_Decimal32` and `_Decimal64`, SSE and SSEUP (one `xmm` register) for
`_Decimal128`, as the psABI specifies; AAPCS64's `s`, `d` and `q` registers, and
homogeneous aggregates of one decimal type (a `_Decimal32` and a `float` are not one);
variadic arguments as these (not promoted: `_Decimal32` stays itself). Windows x64,
whose documentation names no decimal types, and the WebAssembly C ABI, which names
none: this implementation follows the de facto convention of the platform's other
compiler that has them on Windows (the integers of their sizes: `_Decimal32` and
`_Decimal64` in integer registers, `_Decimal128` by reference and its result through
the hidden pointer), and on WebAssembly passes them as `f32`, `f64` and binary128's two
`i64`s. Checked against the system's C compiler on `x86_64` and `aarch64` Linux, both
ways (`gen_c_abi.py --dec`, thousands of random shapes).

A vector travels as the ABI says: on x86-64 System V one SSE eightbyte of 8 bytes, SSE and
SSEUP of 16, and with AVX (`--cpu`, `--features`) one of 32 bytes in a `ymm` register,
with AVX-512F one of 64 in a `zmm` register (SSE and SSEUP; in memory without the
extension, and as a variadic argument), lanes of smaller ones as a structure's; on AArch64 a short vector (8 or 16 bytes) in a `v` register, homogeneous
aggregates of them included, larger ones by reference; on Windows x64 by reference (an 8
byte one in a register), a 16-byte result in `xmm0`; on WebAssembly a 16-byte vector as
`v128` with `simd128`, else as its bytes would be. `_Float16` is passed as a `float`
register holding its bits (System V's and AAPCS64's `_Float16`, in homogeneous
aggregates too), `_Float128` as the binary128 `long double` below (on Windows x64 by
reference, its result in `xmm0`). Checked against the system's C compiler on `x86_64`
and `aarch64` Linux, both ways (`gen_c_abi.py --f16 --f128 --vec`, `c/run/abi_gen_106.c`).

A `long double` travels as the ABI says for its format: the x87's in memory (System V's
classes X87 and X87UP; a structure or union holding one is classified with them), its
result in the x87 register `st(0)`; binary128 as AAPCS64's quad-precision type (in a `v`
register, homogeneous aggregates of up to four included), as System V's `__float128` (SSE
and SSEUP: one `xmm` register), and on WebAssembly as two `i64`s, its result through a
pointer first parameter.

`va_list` is the ABI's: an array of one structure on x86-64 System V, the AAPCS64
structure on AArch64 Linux, and `char *` elsewhere, so it can be passed to the library's
`v...` functions.

On Windows ARM64 every argument of a variadic function, the named ones too, goes as in
the image of a stack of 8-byte slots whose first 64 bytes are `x0`-`x7` (the
platform's rule: AAPCS64's C.12-C.15 applied to that image): floating-point values as
their bits in integer registers, no homogeneous aggregates, a structure of at most 16
bytes as its memory, a larger one by reference, a type aligned to 16 at an even slot. A
structure can therefore straddle `x7` and the stack, as the specification of the
platform says (the other compiler available here as an oracle passes such a structure
whole on the stack, which its own `va_arg` then misreads; this implementation follows
the specification). A variadic definition stores the registers after its named
arguments in 64 bytes it reserves just below its stack arguments, so that `va_list` walks
over both. Results are returned as in AAPCS64 (homogeneous aggregates included).

On Windows, an object that C code declares and no unit defines is a DLL's, by the same
rules as functions (`ntdll.dll`'s `NlsAnsiCodePage`, say): the program
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
and macOS output is built but not run here.

`x86_64-macos` and `aarch64-windows` have never been run on their systems. For
`x86_64-macos`, objects of random shapes built for it interoperate with the system's C
compiler on x86-64 Linux (the same System V convention), linked and run here; layouts
and types were checked against a compiler for `x86_64-apple-darwin`. For
`aarch64-windows`, the callers and callees of random shapes were compiled for it, linked
as AArch64 Linux programs (its code but for thread-locals is the same) and run under
emulation, against each other and against another compiler's Windows ARM64 code (its IR
for that target, given the Windows calling convention on Linux): they agree but for the
structures that straddle `x7` and the stack (above); layouts and types were checked against
a compiler for `aarch64-pc-windows-msvc`. `long double` (alone, in
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
called through a pointer). The faulting instruction itself has no record; with line
tables (`-g`, `--debug-info=lines`) its line is shown all the same (`at deref
(crash.c:2)`), from rows the frame table then holds (`cmd/axiom/src/ftab.ax`: no DWARF
is read at run time), also where no call was recorded (all of a program inlined into
`main`); without them the table and the code are as before.

At `-O2` and `-O3` the trace is correct for the frames that exist, and the calls the
optimizer inlined into them are in it too (an inlined call's record carries the calls
it was inlined into), each such frame marked with the function it was inlined into:

```
stack trace (innermost first):
  at <indirect call>
  at early (crash2.c:9:16) (inlined into tailer)
  at tailer (crash2.c:15:12) (inlined into deep)
  at deep (crash2.c:18:16) (inlined into main)
  at main (crash2.c:22:12)
```

With line tables the code a crash is in is shown with the functions inlined there too
(the frame table's line rows hold the inline chain: the line, then the call each
inlined function was inlined at), also where no call was recorded:

```
stack trace (innermost first):
  at deref (crash.c:2) (inlined into middle)
  at middle (crash.c:5) (inlined into main)
  at main (crash.c:9) (inlined into main)
  at main (std/internal/runtime/c_start.ax:50)
```

(C's `main` is inlined into the start-up's; a call the compiler made, without a
location, is not shown.) What is absent: a function whose last act was a call
(`return f(x);`) leaves its frame first, so the trace lacks it; and without line tables
a crash in code inlined into a function that makes no call between it and its caller
(all of a small program inlined into `main`, say) shows only the frames from that
function's caller up, which can be none: then only the crash line is printed. At `-O0` and `-O1` the
functions are not inlined and the trace is complete in the programs tried. C code has none of
Axiom's run-time checks: no bounds or overflow checks, so a bad pointer is a crash if it
is one at all. On WebAssembly C functions leave no call records: a trap (such as a
division by zero) reports `__axiom_c_main` and the start-up, and no C frames.

## Debugging

`-g` (or `--debug-info=full`) writes DWARF 5 debug information, `--debug-info=lines`
the line tables and frames only, and `--debug-info=none` (the default) nothing. It does
not change the instructions generated, at any optimization level, and two builds are
the same. With line tables the frame table also holds the line rows of crash traces
("Stack traces"), which enlarge the program's loaded data: an object's code and its
relocations are the same as without `-g` (the suite compares them), but in an
executable what the rows push further on is at other addresses, and so are the
addresses the instructions that refer to it hold (the code's size and its other bytes
do not change). An executable is therefore not the same as without `-g` (on macOS
either, where the debug information itself is in the `.dSYM`).

- **Lines**: each function's code is mapped to the lines of the source, `#line`
  directives followed (the presumed file and line, as diagnostics show them); code the
  optimizer inlined or moved keeps the line it came from. Each file of a line table has
  the MD5 of its contents, unless one of the unit's files is a name `#line` gave
  (then none has). The prologue ends, for a
  breakpoint on a function, past the stores of its parameters. Columns are not
  recorded.
- **Frames**: call frame information for every instruction of every function
  (prologues, epilogues, sibling calls, code run before the frame is set up), so that
  a debugger can unwind the stack wherever it stops.
- **Types and variables** (`full`): the program's types (base types, `_BitInt` with its
  width in bits, pointers, qualifiers, arrays, structures and unions with their
  bit-fields, enumerations and their constants, function types, typedefs); each
  function's result type, parameters and local variables; objects of static storage
  duration, thread-local ones included (by their offset in the thread's TLS block;
  not on macOS). A variable, parameter, typedef or object declared with a typedef name
  has the typedef as its type (or one derived from it: `T *p`).
- **Scopes**: a block that declares something is a lexical block, with the code of its
  statements (in pieces at `-O2`, `.debug_rnglists`), its variables, its block-scope
  `static` objects, typedefs, structures, unions and enumerations: a name declared
  again inside a block is told apart from the outer one.
- **Inlined functions**: code the optimizer inlined is an inline instance of the
  function (`DW_TAG_inlined_subroutine`: its code, the call's file and line, in the
  scope of the call), of an abstract instance of it in the unit (`DW_AT_inline`, its
  name, declaration, result, parameters and variables), with the instance's
  parameters and variables located as any (below) and its lexical blocks; instances
  inlined into instances are nested. An instruction's scope says its inline chain
  (`dbginfo.ax`: inlining copies the scopes of the code it copies, `inline.ax`), at
  `--debug-info=lines` too, where the instances are without variables. The debuggers
  show inlined frames in a backtrace, with their arguments, and `step` enters them
  (x86-64; AArch64 under emulation). The name index has the abstract and inline
  instances. The PDB has the same as inline sites (`S_INLINESITE`, its binary
  annotations giving its code and lines from the function's declaration, which the
  module's `DEBUG_S_INLINEELINES` gives; the inlined function's id, `LF_FUNC_ID`; its
  variables), at `-g`.
- **Locations at `-O0`**: every variable is in memory, in its frame slot (`-O0` keeps
  it there, with or without `-g`), from the end of the prologue on. A parameter passed
  by reference (a large structure on AArch64 and Windows) and a variable length array
  are found through the address an object of the frame holds; a variable length
  array's bound is the variable holding its length.
- **Locations at `-O1` and above**, never a wrong one; what has none is "optimized out".
  Each assignment of a variable that the optimizer keeps in registers (its
  initializer, each later store, each merge of the values that reach a join) is a mark
  in the code that follows the instructions around it as they move
  (`cmd/axiom/src/ir.ax`, "Assignments of variables"); from each mark on, until the
  next one on every path, the variable is the value assigned, wherever that value is:
  - in the registers and spill slots that hold it, at every instruction where every path
    since has made it and nothing has written the register since; a value made again
    (a loop's next round) leaves the variable without a location until the next
    assignment; a value the optimizer moved past the assignment is the variable once it
    is made;
  - a constant (`DW_OP_implicit_value`), the address of a frame object or of a datum,
    a value plus a constant (a value folded into its uses: in its base's register or
    slot, `DW_OP_breg`), the low bits of a wider value (a loop counter widened to a
    pointer's width);
  - at a join, where every path agrees, or for a merge (a phi) in the registers that
    hold each path's input at its end, else none;
  - a structure taken apart is in pieces (`DW_OP_piece`), each followed alike; a
    variable length array is found through the register or slot that holds its address;
  - in a callee-saved register across a call, which a debugger reads through the
    callee's frame;
  - copied code (unrolled loops, duplicated blocks) has its own marks, with the copies
    of the values where the pass that copied them knows them, else none;
  - a parameter is also in its argument's register at the function's entry, or (never
    assigned) in its stack slot (but in a function with a call in tail position);
  - a variable left in memory (its address taken, a structure not taken apart) is in
    its frame slot when all its stores remain where they were and no other object
    shares the slot.

  What this relies on of the register allocator (`cmd/axiom/src/dloc.ax`): a machine
  instruction's operands, clobbers, copies, spills and reloads say every register and
  spill slot it changes. `cmd/axiom/internal/tests/dwarf.mjs` checks the location
  lists' shapes (inside their blocks, in order, in registers that hold values, a
  parameter's entry in its argument register);
  `cmd/axiom/internal/tests/tools/debug_coverage.mjs` counts the variables with a
  location. In development, randomly generated functions (assignments in branches,
  loops and blocks, structures, pointers into frame objects, address-taken variables)
  stepped instruction by instruction in the system's debuggers, every value they show
  (in the function, and in its caller at each call) compared with the program's own
  log of its assignments, showed no difference (x86-64; AArch64 under emulation), and
  the same comparison catches a deliberately wrong rule for the registers a call
  changes.
Where it goes, per target:

| Target | |
|---|---|
| `x86_64`, `aarch64` | The executable's (or object's) `.debug_*` sections: `.debug_info`, `.debug_abbrev`, `.debug_str`, `.debug_line`, `.debug_line_str`, `.debug_aranges`, `.debug_frame`, `.debug_names` (an index of the unit's subprograms, static variables, named types and enumerators), and when needed `.debug_rnglists` and `.debug_loclists`. |
| `aarch64-macos`, `x86_64-macos` | The debug bundle `<out>.dSYM` beside the executable, which the platform's debugger finds by the executable's UUID; the executable's instructions are those without `-g`, but for the addresses of the data its frame table's line rows move. It has no unwind information (`__unwind_info`, `__eh_frame`): the platform's ABI keeps a frame record (x29, x30; on x86-64 rbp and the return address) in every function that calls, and the crash reporter and `backtrace(3)` walk those records, so stack traces through our frames need none (a leaf without a frame has its caller in x30, or on x86-64 at the stack pointer). Only an unwinder that needs the tables (C++ exceptions through our frames, which do not happen) would. |
| `x86_64-windows` | The PDB (`--pdb=on`, the default) has every function's line table, with or without `-g`, a module per object, every function's procedure and public symbol. With `-g` also, in CodeView form (`codeview.ax`, from the same description of the program as DWARF), the types (TPI, with its hash stream: a forward reference to a structure is found by its name), ids (IPI: the functions' ids, where each structure is defined), each function's type, frame, lexical blocks (a block in several pieces of code is a block per piece) and variables, and the static objects (`S_GDATA32`, `S_LDATA32`, thread-locals as `S_GTHREAD32` at their offset in the template, which starts `.data`) and typedefs and tags (`S_UDT`) at file and block scope. At `-O0` a local is at its offset from the frame pointer (`S_REGREL32`), a parameter an `S_LOCAL` located from the call frame's register at each instruction; in optimized code a variable is an `S_LOCAL` with the ranges where it is all in a register, in a frame slot or in memory at an address held in a register, or an `S_CONSTANT` where it is one constant throughout. Not described ("optimized out" there, never a wrong value): a structure taken apart into registers (the system's symbol handler reads CodeView's subfield ranges as the whole variable), a value computed from a register (a register plus a constant, an address of a frame object or a datum), the value at an address held in a frame slot (at `-O0`: a variable length array, a parameter passed by reference); a variable length array's bound. Typedefs are `S_UDT` names (what is declared with one has the type it names); `_Atomic T` is `T`; a base type CodeView has no primitive for (decimal floating types, a bit-precise integer narrower than its storage) is a structure of its name and size holding its bytes; an Axiom union a structure of its tag and each payload at its offset. The function table (`.pdata`) describes every function with a frame (a function without one makes no call: every call on Windows reserves the shadow space, which gives the caller a frame), right at every instruction: a large frame's stack probe does not move `rsp`, and a function with sibling calls uses the frame-register form with an entry of its own for each jump (the code after it chained), so that the unwinder is right whether or not it takes a jump out of a function for the end of an epilogue. The suite decodes every unwind record as the system does before each Windows program runs (`pe.mjs`) and single-steps a walk through every kind of frame (`c/mix/win_walk`). |
| `aarch64-windows` | As `x86_64-windows`; the function table's ARM64 records describe every function with a frame (on Windows every function that calls has one). |
| `wasm32`, `wasm64` | Custom sections `.debug_info`, `.debug_line`, ... of the module, code addresses as offsets in its code section, a compilation unit per unit compiled (each object carries its own, which the linker merges; a function it did not keep is at the tombstone address -1). With `-g` the types and variables too: at `-O0` each variable in its frame slot (`DW_OP_fbreg`), the frame base being the local that holds the frame's address (`DW_OP_WASM_location`); optimized code's variables have no location, and a thread-local none (each thread's instance is elsewhere). No call frame information, which WebAssembly has no use for. |

The objects of `--emit=obj` carry the same sections (ELF targets), with relocations,
for the system's linker. Axiom functions in the same program are described in the same
compilation unit, whose language is C's (an Axiom program's own units have Axiom's:
`docs/cli.md`, `-g`).

The line of a crash's faulting instruction is in the stack trace when the program has
line tables (above).

## Optimization

`-O0` (the default) to `-O3` are the compiler's: C goes through the same passes as Axiom
code (README of `cmd/axiom/`, "Optimization"). All the units of a program are one module,
so `-O2` and `-O3` inline across them. A `restrict`-qualified pointer parameter (passed in
a register) tells the optimizer that what it reaches is not reached through another
parameter, a global or a local object while the function runs (6.7.3.1): loads move out
of loops past stores through it, and copy and vector loops need no overlap check. A
pointer loaded from memory may hold its value, and is not taken as apart from it. Type-based
aliasing is not used (an `int *` and a `float *` may reach the same bytes). The levels do not change the behavior of a
program that has no undefined behavior; they differ in speed and size, and in how
complete a stack trace is after a crash.

## Known deviations

These are the places where the implementation does not follow the standard it claims, or
claims less than the standard allows. None is hidden: each is said above or here.

- Annexes F and G are claimed on x86-64 and AArch64 (with the library), not on
  WebAssembly, which has no floating-point environment.
- Annex G: division uses Smith's method (no overflow forming the divisor's squared
  magnitude); it may differ in the last bit from other implementations' choices.
  `CX_LIMITED_RANGE` has no effect (the full algorithms are always used).
- The decimal floating types: on WebAssembly their exceptions are not recorded (no flags
  there, as for binary).
- Signed overflow, which is undefined, wraps in the code the compiler generates today;
  a program must not rely on it.
- Annex H is not claimed (`__STDC_IEC_60559_TYPES__` is not defined): C23's additions to
  7.12 have no `_Float16` or `_Float128` forms (`exp2m1f128`, `sinpif16`, `totalorderf128`,
  ..., `nanf16`, `nanf128`), nor are there the narrowing operations between the
  interchange types; and see below for `_Float32` .... The `_Float16` and `_Float128` functions of `<math.h>` and the conversions of
  `<stdlib.h>` are the library's under `__STDC_WANT_IEC_60559_TYPES_EXT__`: binary128's
  exact functions correctly rounded, its elementary functions in its precision;
  binary16's elementary ones correctly rounded (the double function's value rounded once,
  or the `_Float128` function's where that value is too near a rounding boundary: checked
  for every binary16 argument); those of
  `_Float32`, `_Float64`, `_Float32x` and `_Float64x` are macros for the `float`,
  `double`, `long double` or `_Float128` functions, whose results have those types (a
  `_Generic` on one sees `float`, not `_Float32`). The complex ones are `<complex.h>`'s
  likewise (`cexpf16`, `cexpf128`: `complexf16.c`, the double functions' parts rounded
  once; `complexf128.c`, `complex_impl.h`'s methods in binary128's precision; macros for
  the others); `<tgmath.h>` chooses `_Float16`'s and `_Float128`'s, real and complex, and the
  standard ones for the others.
- In a GNU dialect: the
  destructors run when `main` returns and from the library's `exit`, not when the
  process ends otherwise (`_Exit`, `quick_exit`, a signal).
