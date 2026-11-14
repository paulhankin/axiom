# Compiling C

`axiom build file.c` compiles C. The C front end reads C translation units and hands them
to the same optimizer, code generators and linkers that Axiom programs use, so a C program
is built for every target of the compiler (WebAssembly, x86-64 and AArch64 Linux, x86-64
Windows, AArch64 macOS, and x86-64 macOS and Windows on ARM64, whose output has never
been run: "Calling conventions and interoperability") with no C compiler and no external
linker: the compiler's own linkers link the platform's C library too, and on Linux the objects, archives and shared
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
  - On WebAssembly, it is an import of the module `env`, with the name of the function
    (but for a weak reference, which is null when nothing linked defines it).
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
| `--c-long-double=64\|80\|128` | The format of `long double` (default: the target's C ABI; see "Floating types"). `80` is for x86-64 Linux and macOS only; `128` for those, `aarch64` Linux and WebAssembly. |
| `-DNAME`, `-DNAME=value` | Define a macro (`NAME` is `NAME=1`). `-D'F(x)=x+1'` defines a function-like macro. |
| `-UNAME` | Undefine a macro. The predefined macros of the standard cannot be undefined (`C1053`); the target's ("Predefined macros") can, and `-D` replaces one. |
| `-Apred=answer`, `-Apred(answer)` | A GNU dialect's assertion, as `#assert pred(answer)` before the source ("GNU dialects"); `-A-pred=answer` takes it back (`-A-pred` every answer of pred), `-A-` every assertion before it, the target's `cpu`, `machine` and `system` too. Under an ISO standard, which has no assertions, it is an error. |
| `--c-asm-dialect=att\|intel` | The syntax of x86-64 inline and file-scope assembly (default `att`; "Inline assembly"). |
| `--c-warn-unknown-pragmas` | An unknown `#pragma` is the warning `C4111` (it is ignored either way). |
| `--c-keep-null-checks` | The optimizer is not told that a parameter the attribute `nonnull` names, or one declared `[static n]`, and the result of a function with `returns_nonnull`, are not null pointers: tests of them stay ("Optimization"). |
| `--c-warn-format=0\|1\|2` | The checks of format strings ("Attributes": `format`): 0 none, 1 (the default) what is undefined or fails on any target (C4130, C4131, C4133), 2 also what is suspect (C4132, C4134, C4135). |
| `-Idir` | Add a directory to the include path. |
| `--c-freestanding` | A freestanding program: `__STDC_HOSTED__` is 0, only the freestanding headers are found (another is `C1081`), and no C library is linked ("The C library"). |
| `--c-lib=axiom\|system` | Whose C library the program has: the compiler's (the default), or the system's, whose headers are then the hosted headers ("The system's C library", below). |
| `--c-system-include=dir` | With `--c-lib=system`: a directory of the system's headers (instead of the system's own, `/usr/include` and its target directory on Linux). |

`-D`, `-U`, `-A` and `-I` apply in the order given. The C options are errors for an Axiom
source. The unknown ones are errors too: there are no `-W...`, `-std=`, `-c` or `-f...`
options; a warning is silenced only by `#pragma GCC diagnostic` ("Pragmas").

### Options of `axiom build` that apply

`--target`, `--cpu`, `--features`, `-O0`..`-O3` (see "Optimization"), `--stack-traces`
(see "Stack traces"), `--format=agent-json`, `--std=<dir>` (the standard library's
directory, where the compiler finds the C headers in `c/include/`), `-o`, `--emit`,
`--link`, `-l`, `-L`, `--subsystem`, `--icon`, `--pdb`, `--stack-size`, `--pic`,
`--function-sections`, `--data-sections`, `--gc-sections` (see "Code in sections"),
`--unwind-tables` (see "Objects and libraries"), `--macos-tls` (see "Atomics and
threads"), and `-g` / `--debug-info=`, `--debug-macros`, `--debug-language` (see
"Debugging"); [`docs/cli.md`](cli.md) describes each. The default
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
`-o`. `--emit=asm` is not supported (there is no assembly output: `--emit=obj` and a
disassembler, or `--emit=ir`). Without `-o`, the output is named after the first
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

On every target a source whose name ends in `.s` or `.S` is a translation unit of
assembly (on WebAssembly in the text format: below), beside C units and an
Axiom program (`axiom build main.ax x.s`), or alone (`--emit=obj`). Its text is assembled
as file-scope asm is ("Inline assembly"): AT&T syntax, or Intel under
`--c-asm-dialect=intel` and after `.intel_syntax`. A `.S` source is first preprocessed
by the C preprocessor with `__ASSEMBLER__` defined as 1, `-D`, `-U` and `-I`, the
predefined macros (the target's, "Predefined macros", and `__AXIOM_ASM_X86_64__`, `__AXIOM_ASM_AARCH64__` or `__AXIOM_ASM_WASM__` in any mode), its
directives done and its macros replaced, its other text as written; a line whose `#`
does not begin a directive is a comment (as is `#` first on a line on AArch64, `//`
anywhere there, `#` anywhere on x86-64, and `;;` and `(; ;)` on WebAssembly). A `.s`
source is not preprocessed. On WebAssembly (`__AXIOM_ASM_WASM__`) the source is module
fields in the specification's text format (functions, imports, globals, tags, passive
data and element segments, exports, a start function, types), optionally inside
`(module ...)`, as file-scope asm is there ("Inline assembly"); it sees no C
declarations, so it imports what it uses: `(import "env" "name" (func $id ...))` is the
program's function `name` (another unit's, the C library's), and so for a global or a
tag (`(global $id t)`, `(tag $id (param ...))`); another module's import is the host's.

- Its global symbols (`.globl`, `.weak`) are external names of the program: C and
  Axiom (`extern "C"`) call its functions and read its data; a name it defines that a
  C unit or the Axiom program defines too is an error of the build; the C library's
  functions it calls are linked for it (its references are
  the library's roots as a C unit's are). On macOS the leading underscore is added to
  every name, as for C. A name it defines keeps the C library's definition of that name
  out, as does a global symbol that a C unit's file-scope asm defines: the program's
  definition wins, as a C unit's does.
- Nothing of an assembly source is stripped: its sections are the object's whatever
  refers to them (dead-code removal does not look into assembly), and the functions and
  objects it names are kept.
- Under `-g` its code has line information: each instruction's line in the source (a
  `.S` source's lines are its own, a line of an included file or of a macro's
  expansion being the line it is used on), or the place the last `.loc file line`
  gave (`.file N "name"` names the files), on every object format: in the DWARF line
  table on Linux and macOS and in the PDB's (CodeView) on Windows;
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
  directly gets a copy relocation, and a function whose address it takes so gets the
  program's PLT stub as its address in every module (the `.dynsym` entry's value; the stub
  itself is bound by an `R_*_JUMP_SLOT`, which the dynamic linker binds past it): unless
  the library's definition is protected (`STV_PROTECTED`: its own references bind to it,
  which would see another object or address), which is then an error that asks for
  position-independent code. Sections whose names are C identifiers are placed
  together, between `__start_<name>` and `__stop_<name>` when something refers to those;
  `.preinit_array`, `.init_array[.N]` and `.fini_array[.N]` are placed by priority between
  the symbols `__init_array_start` ... `__fini_array_end` and run by the start-up ("Start-up
  and exit"). `.eh_frame` is kept, relocated, and the linker makes `.eh_frame_hdr`, the
  table of every FDE's function by its address, sorted for a binary search (the Linux
  Standard Base's format: the FDEs of code the link left out are not in it), which
  `PT_GNU_EH_FRAME` names in executables and shared libraries, as the platform's unwinder
  (its runtime's backtraces, C++'s exceptions) finds it. The objects' read-only
  `.eh_frame` sections are placed together, ended by a zero terminator, and the
  executable's section headers name `.eh_frame` and `.eh_frame_hdr` (for the tools that
  find them so). This compiler's own objects have an FDE in `.eh_frame` for every
  function, with or without `-g` (`cmd/axiom/src/macunwind.ax`), from the call frame
  states the code generators record: right at every instruction (prologues and
  epilogues, the pushes and stack adjustments, a shrink-wrapped function's code before
  its frame, frameless functions, the epilogues of sibling calls before a function's
  end, the outlined cold parts, which are functions of their own, and a frame whose
  stack pointer moves, by a variable length array, `alloca` or a large frame's stack
  probe, described from the frame pointer), the same rules as `-g`'s `.debug_frame`. An
  object has one `.eh_frame`, a CIE (augmentation `zR`, pc-relative 4-byte pointers)
  and an FDE per function that refers to its code by the code section's symbol and the
  function's offset, so that a function in a section of its own (`--function-sections`,
  a link-once function's COMDAT group) whose section a link leaves out
  (`--gc-sections`, a dropped COMDAT copy) leaves an FDE that describes nothing, which
  this linker leaves out of `.eh_frame_hdr` and the system's linkers drop. The programs
  are short: an epilogue remembers the body's state (`DW_CFA_remember_state`) and the
  code after it restores it, a register given back is `DW_CFA_restore`, entries are
  4-aligned (the growth: about 9% for a program that prints a line, mostly its
  runtime's many small functions; 2% for the compiler itself). `--unwind-tables=off`
  leaves the FDEs out (of the compiled code, the linker's stubs and assembly
  procedures; `.eh_frame_hdr` too, unless objects of other compilers bring FDEs): the
  program's own stack traces are the same, but the system's unwinder cannot pass
  through this compiler's frames (a backtrace stops at them, another language's
  exceptions cannot be thrown through them). The linker's own
  functions (the system call stubs, `setjmp`) are leaves with FDEs, but for the thread
  entry (whose child's frame is the thread's first), `longjmp` (which leaves its frame
  before its end) and the signal return `ax_sigreturn`, whose instructions (`mov rax,
  15; syscall`, AArch64 `mov x8, 139; svc 0`) the platform's unwinder takes for a
  signal frame where no FDE covers them. So the system's unwinder walks through this
  compiler's frames: a backtrace, another language's exceptions (`c/link/eh_frame_hdr`,
  `c/link/eh_frame_sections`, and `dwarf.mjs` checks every function's FDE against its
  `.debug_frame` one in the debug tests), and out of a signal handler through the signal
  frame of the compiler's C library to the interrupted code (`c/link/eh_frame_signal`).
  The FDEs of other compilers' objects keep their language-specific data areas (in an
  ordinary read-only section, or one of the function's own) and their CIEs' personality
  routines (reached through a pointer in a COMDAT group of its own) with the function
  they describe, under `--gc-sections` too; the routines and the rest of the language's
  run time come from its libraries. So C++ code throws through this compiler's C
  functions, at `-O0` and `-O2` and with variable length arrays, and catches above them
  (`c/link/cxx_throw`, x86-64 and AArch64). Such a program's C library is the system's
  (`--c-lib=system`, "One C library": the C++ run-time library is built against it).
  Indirect functions
  (`STT_GNU_IFUNC`, global and local, of x86-64 and AArch64; the C attribute `ifunc`): a
  PLT stub is the function's address wherever it is taken (calls, the GOT, data), and
  jumps through a GOT slot that an `R_*_IRELATIVE` relocation gives the resolver's result:
  in a static program the start-up applies them first (calling the resolvers with nothing
  on x86-64 and on AArch64 with `AT_HWCAP | _IFUNC_ARG_HWCAP` and a pointer to
  `{24, AT_HWCAP, AT_HWCAP2}`, from the auxiliary vector), otherwise the dynamic linker
  (they come last, after `DT_JMPREL`'s `JUMP_SLOT` relocations). A shared library's
  definition of default visibility is preemptible ("Shared libraries"); such an indirect
  function is exported as one, the dynamic linker's to resolve.
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
  linker's own symbols (`__tls_get_addr`, AArch64's `__stack_chk_guard`) are found too. The
  program's definitions that such a library refers to or defines too are exported
  (`.dynsym`), so that the library's references bind to them (the program's definition
  preempts a library's): not for the system C library's own (`libc.so.6`, `libm.so.6`,
  the dynamic linker, ...), whose references, to `malloc` and its kin among them, stay its
  own.
- **C sources** given with `--link=` are compiled, after `--emit-header`'s header is
  written (which they may include), for Linux by the system's C compiler (`cc`, or the
  cross compiler for the target: `aarch64-linux-gnu-gcc`) with `-O2` and the `-I`
  directories: code of the platform's toolchain, which is what such a source is for (a
  library's helper, a test's oracle); for macOS and Windows, which have no C compiler
  here, by this compiler, in a process of its own, into an object in the target's format
  (`--emit=obj`: Mach-O, COFF), with `-O2`, the build's C options (`--c-std`, `--c-lib`,
  `-D`, `-U`, the `-I` directories, ...), `--cpu` and `--features`.

When objects of the platform's toolchain are linked, so are its C compiler's run-time
support archives (of its newest version installed, the native or the cross one), from which a member is linked only where nothing else defines
what an object needs (complex arithmetic, 128-bit division and conversions, AArch64's
atomics called out of line), as the system's linker links them.

What the inputs refer to and do not define, the program does: the C library ("The C
library") supplies it, its units and run-time modules compiled with the program as for
the program's own references, and what the inputs refer to is kept ("What is linked").

On WebAssembly the compiler's linker takes relocatable objects of the tool conventions'
format (this compiler's, and the system's C compiler's for the same target) and archives of them, found
with `-L` and `-l` (`libname.a`), with the same member selection; a C source given with
`--link=` is compiled by `clang` first. A weak reference that nothing linked defines (and
that no import names explicitly) is null: a function's table index is 0 and its calls
reach a stub of its type that traps (`unreachable`), an object's address is 0; this
compiler's objects leave such a function's import without a name of their own, so that
the system's WebAssembly linker makes it so too. The constructors of such objects (their
linking sections' `INIT_FUNCS`) are run before `main`, by priority (of one priority in
the order of the objects), by `__wasm_call_ctors`, which the linker makes when an
object has some (the tool conventions' name: the C start-up calls it). This compiler's
WebAssembly objects list their C units' constructors so, and their destructors as
constructors of their priorities that register them with `atexit` (which runs them in
the reverse order: after `main`, by `exit`), so that the programs the system's
WebAssembly linker links run them too. On macOS (`mlink.ax`) and Windows (`plink.ax`) the compiler's linkers take the objects and
static archives of the platform's toolchain and of this compiler, in the platform's
format, for both processors (`macho.ax`, `coff.ax` read them), with the same member
selection; what the objects refer to and do not define comes from the libraries:

- **Mach-O objects** (arm64, x86-64): every relocation type of both (`ARM64_RELOC_*`
  with `ARM64_RELOC_ADDEND`, `X86_64_RELOC_*`, `SUBTRACTOR` pairs to a symbol of the same
  section, section-relative ones), common symbols, weak definitions and references,
  `__mod_init_func` and `__mod_term_func` (run by the start-up as the initialization
  arrays are), and the platform's thread-local variables (`__thread_vars`,
  `__thread_data`, `__thread_bss`: the executable gets the sections and the platform's
  dynamic linker makes the blocks); `__compact_unwind` and `__eh_frame` go into the executable's unwind tables
  (a function's FDE found by the address it covers, which the platform's assembler
  leaves without a relocation), with their personality routines and LSDAs (C++'s
  exceptions, C's cleanups under `-fexceptions`): a compact entry's routine is one of
  the `__unwind_info` table's personalities (at most three, the GOT slots of their
  addresses) and its LSDA an entry of the table's LSDA index, and an FDE's CIE points
  at its routine's GOT slot (`ARM64_RELOC_POINTER_TO_GOT`, `X86_64_RELOC_GOT`)
  (`c/objfmt/unwind_personality`, checked by `macho.mjs`; built only). A universal
  object is not read (give the slice).
- **Dynamic libraries**: a `.dylib` given (or found by `-l` in the `-L` directories; of a
  universal file, the target's slice) is read for its install name and exports, and the
  program is bound to it by its ordinal (two-level namespace). A library named with `-l`
  that is not a file here (`-lfoo`, the SDK's `.tbd` stubs, which are not read) is loaded by
name (`/usr/lib/libfoo.dylib`, a framework's path, ...), and what no library is known to
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
`__getauxval`; `dl_iterate_phdr` (the LSB's) and `_dl_find_object` (the dynamic
linker's, as its manual gives it), through which the platform's unwinder finds the
program's call frame tables: they walk the executable's program headers, which the
auxiliary vector names, and in a dynamically linked program those of the shared objects
in the dynamic linker's list (`DT_DEBUG`'s `r_debug`), so that the unwinder links with
this library, statically or not. A weak reference of such code to a name that this
library rejects below (the unwinder's to the system library's `__pthread_key_create`, by
which it tells whether the program has threads) is left null rather than bound to the
system's library. The limit is what such code took from the system library's headers that
this library does not share: a reference to another of that library's own names (those
reserved to the implementation: `__overflow`, `__uflow`, ...) or to one of POSIX's
functions of streams (`fileno`, `fdopen`, `popen`, `getline`, ...: they take or give the
system library's `FILE`) is an error (`E9004`, after `link error: x.o refers to
'__overflow', a name of the system's C library's own implementation` or `x.o calls
'fileno', which works on the system's C library's streams`), as is anything else that
would give the program two C libraries, and so is a call of a function whose objects the
system's headers lay out smaller than this library does (`fgetpos` and `fsetpos`, whose
`fpos_t` is 16 bytes there and 40 here; the restartable multibyte functions, `mbrtowc`
and its kin, whose `mbstate_t` is 8 bytes there and 24 here). A shared library of the
platform's that refers to such names was built against the system's library, and linking
it is an error too (`link error: <library> is built against the system's C library (it
refers to ...)`): a program linked with a language's run-time library, as C++ code is with
the shared C++ run-time library, has the system's C library (`--c-lib=system`), whose
streams, locales and multibyte state that library shares, and to which its references
to the standard's functions bind as well. Macros the code expanded
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
limits, the compiler's `<limits.h>` including the system's and adding what C23 gives
it that the system's leaves to the compiler: `BITINT_MAXWIDTH`, `BOOL_MAX`,
`BOOL_WIDTH` and `__STDC_VERSION_LIMITS_H__`, where it has not defined them).
Those headers are compiled as the system's: the macros of the platform they
test are the target's, as in every mode ("Predefined macros": `__x86_64__` or
`__aarch64__`, `__linux__`, `__unix__`, `__ELF__`, `__LP64__`, `__SIZEOF_<T>__`,
`__BYTE_ORDER__`, `__CHAR_BIT__` ...), and `__AXIOM_SYSTEM_C_LIB__` is defined; the definitions they make of the implementation's own macros
(`__STDC_ISO_10646__`, ...) keep the compiler's, as do those they make of the macros of
the compiler's headers (C23's `INFINITY` and `NAN`, which `<float.h>` and their
`<math.h>` both define), their `<math.h>`'s comparison macros (`isgreater` ...,
statement expressions for a compiler that is not theirs) are the compiler's built-ins,
the floating constants beyond their types' range in their macros (`HUGE_VAL` is
`1e10000` there) are the infinities they mean, their warnings and the errors that say a
feature is a later standard's (or an empty declaration) are not shown, and in a GNU
dialect `_FloatN` and `__float128` are not keywords there (they define them as types),
while the definitions they make of the dialect's keywords (`__attribute__(x)` as nothing,
`__extension__`, ...) are not made: those stay keywords, in the program's code too.
Under an ISO standard their own text may still use the GNU dialects' spellings of types
and named variadic macro parameters (`name...`), which they take their compiler to have
(AArch64 Linux's `<signal.h>` reaches kernel headers with `__signed__` and
`__uint128_t`): in them, and in their macros' expansions, those are accepted.
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
  externs (`N_PEXT`), weak definitions `N_WEAK_DEF`, weak references `N_WEAK_REF`.
  Thread-locals are the platform's thread-local variables, as its compilers write them
  (an object implies `--macos-tls=platform`: "Thread-locals" below): the variable's symbol is its descriptor in `__DATA,__thread_vars` (three words,
  relocated to `__tlv_bootstrap` and to the local `<name>$tlv$init`), its initial value
  in `__DATA,__thread_data` or `__thread_bss`, and code reaches the descriptor with
  `ARM64_RELOC_TLVP_LOAD_PAGE21`/`_PAGEOFF12` (x86-64: `X86_64_RELOC_TLV`) and calls
  its first word, which returns the variable's address. The system's linker links them
  with the platform's objects, and this compiler's linker the platform's with ours.
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

The system's linkers for those platforms read these (the suite's `c/objfmt` tests link
them with such linkers where they are installed). An object of C units alone has no runtime: linked
by the system's toolchain into a C program (whose C library is the system's), it may use
only the standard's functions and objects; one that uses this library's private names
(the macros of `<stdio.h>` for `stdin`, `stdout`, `stderr`, `getc` and `putc`, `errno`'s,
... `__axc_*` names) needs this library. Objects with Axiom modules carry the runtime, which this
compiler's linkers link.

### Shared libraries

`--emit=shared` links C units (with `--c-lib=system` or `--c-freestanding`), and the
objects and libraries given, into a shared library; the compiler's C library and the
Axiom runtime are an executable's (one per process: its allocator, threads and
thread-local blocks), so Axiom modules and the default C library are an error (`E9001`).

- **ELF** (`libprog.so`; `elink.ax`): position-independent (`--pic`, which
  `--emit=shared` implies: thread-locals in the initial-exec model, their offsets in GOT
  slots that the dynamic linker fills; the rest of the code generators' code is
  position-independent always), its definitions of default and protected visibility
  exported (`.dynsym`, a hash table; `STV_PROTECTED` kept). Those of default visibility
  are preemptible, the generic ABI's rule: a module loaded before the library (the program,
  which may define the name too, or have a copy of the object: a copy relocation) may
  define them, so the library's own references reach them through its GOT and PLT, bound
  by the dynamic linker (`R_*_GLOB_DAT`, `R_*_64` and `R_*_TPOFF64` against the symbol),
  and a reference that cannot (one not through the GOT in code that is not
  position-independent) is an error. The compiler takes the definition it compiles to be
  the one that runs when it optimizes (it may inline the function). Protected and hidden
  definitions bind within the library. Every address it holds is relocated where
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
  `__mod_init_func` and `__mod_term_func` (the platform's dynamic linker runs them), rebase and bind information as
  for an executable, ad-hoc signed; its thread-locals are the platform's (`--emit=shared`
  implies `--macos-tls=platform`; the platform's dynamic linker makes their blocks), and those it exports are its
  descriptors. Built and checked only: nothing
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
keep nothing themselves, except that an FDE of `.eh_frame` keeps, with the function it
describes, its LSDA and its CIE's personality routine (by the pointer the CIE has). In
COFF, whose `.xdata` names its handlers (the C runtime's, imports of its DLL), a handler
is kept only if something else keeps it. A program's own C units and Axiom modules are compiled as one
unit, whose unreferenced functions are not made at all: `--gc-sections` matters for the
objects and archives a program links.

Of a COFF object's COMDAT sections a link keeps one copy by its selection: "any" the
first object's, "largest" the largest, "same size" and "exact match" the first, the
others then having to be of its size or its bytes, and a second copy of one of "no
duplicates" is an error (`duplicate symbol`).

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
specify: 4, 8 and 16 bytes, aligned to their sizes (and Annex H's `_Decimal64x`, in
decimal128's format: "Annex H: the interchange and extended types"). Their arithmetic is software of the
compiler's (`std/c/rt/decimal.ax`), the same on every target (`_Decimal32`'s and
`_Decimal64`'s `+`, `*` and `/` in 64- and 128-bit integers, `_Decimal128`'s `+`, `-`, `*`
and `/` in 128- and 256-bit integers (Knuth's Algorithm D for the quotients), where the
result's exponent is far from the format's limits; elsewhere the general code, whose
results and flags these paths give bit for bit in every direction):

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
  "invalid" out of range (undefined, C23 6.3.1.4); to and from `_Float16`, `float`, `double`
  and `long double` (every format: binary16, binary32, binary64, the x87's, binary128;
  `_Float32`, `_Float64`, `_Float128` and the extended types are one of them) correctly
  rounded both ways, once (to a decimal type in the decimal rounding direction, the
  exact value's exponent the one nearest 0; to a binary type in the binary direction:
  to nearest, or under strict floating point the environment's, "The floating-point
  environment"), with the exceptions of that one rounding, and between the decimal
  types. A
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
  and round once: within 1 ulp (measured against an arbitrary-precision reference, a few hundred arguments of
  every magnitude per function and format: the largest error 0.500 ulp). For
  `_Decimal32` and `_Decimal64` they first take a binary path (`std/c/rt/dfp_fast.ax`):
  the argument converted to binary128 (within 2^-111, by a table of the powers of ten,
  `std/c/rt/dfp_tables.ax`), the binary128 function (`math_ld.ax`), and the result
  converted back with a rounding test that allows for the error of both and the
  function's condition number at the argument; where the test cannot decide, or the
  result may be exact (it then needs its preferred quantum), or the argument or the
  result is out of the ranges that path takes, the decimal computation does. So these
  results are correctly rounded in every direction (checked against an arbitrary-precision reference on 760,000
  results, 38 functions, both formats, the five directions: the only differences from
  the decimal computation were 270 results of tiny arguments in directed modes that it
  rounded wrong, which this path rounds correctly), and the flags are those of the
  decimal computation. `_Decimal128` (and `_Decimal64x`), whose 34 digits binary128 does
  not hold, takes the same path in `math_ld.ax`'s wide form: the argument's significand
  to 128 bits (within 2^-126), the functions' triple-double values before their rounding
  (within about 2^-118, taken as 2^-117) and their top 128 bits, converted by a 256-bit
  product with the power of ten; the test then cannot decide about once in 5 to 20
  calls (the bound is a 2^-3 to 2^-6 of the last digit's unit), and those take the
  decimal computation (the same results, flags and `errno` in the five directions on
  fixed-seed arguments, 38 functions). An exact result has the quantum C23 prefers (`sqrtd64(1.44dd)` is `1.2`,
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
  take the length modifiers `H`, `D` and `DD` with `a e f g` and their upper case (and,
  for `_Decimal64x`, which the standard gives none and which has `_Decimal128`'s format
  and is passed as it is, `DD` too: the format check takes it there): `%Da`
  writes the coefficient and the quantum exactly (`1.20`, `1.2e+03`: style f when
  -(n + 5) <= q <= 0, else e; a precision rounds the coefficient), `e f g` round the
  value's digits as for `double`, both in the decimal rounding direction
  (`fe_dec_setround`, which has the halfway case away from zero too, not the binary
  one); `strfromd32` ... format as `snprintf` does.
  `<float.h>` has the `DEC32_` ... macros; `<fenv.h>` `FE_DEC_DOWNWARD`,
  `FE_DEC_TONEAREST`, `FE_DEC_TONEARESTFROMZERO`, `FE_DEC_TOWARDZERO`, `FE_DEC_UPWARD`,
  `fe_dec_getround` and `fe_dec_setround`, and `fenv_t` and `femode_t` hold the decimal
  direction too.
- **Speed** (x86-64, `-O2`): an operation is a call of the software, about 100 ns for a
  `_Decimal64` multiplication, addition or division, 75 to 200 ns for `_Decimal128`'s
  (were 350 to 630); an elementary function of `_Decimal32` or `_Decimal64` about 0.5 to
  1 us (were 40 to 140 us), of `_Decimal128` 2 to 15 us and `powd128` 30 (`expd128` 2.3,
  `logd128` 4.3, `sind128` 5.5; were 25 to 180: most of the time is in the calls the
  rounding test sends to the decimal computation), `lgammad128` and `tgammad128` 0.3 to
  0.8 ms (were 1.7), a trigonometric one
  of an argument beyond 10^17 milliseconds (the exact reduction).

### Annex H: the interchange and extended types

C23's Annex H is claimed (`__STDC_IEC_60559_TYPES__` is `202311L` in C23 with the
library); its library identifiers are declared with `__STDC_WANT_IEC_60559_TYPES_EXT__`
(H.8):

- **`_Decimal64x`** (H.2.4: required with the decimal types) has decimal128's format
  (H.2.4p4: "may have the same format as `_Decimal128`"; Table H.3's least precision
  and range, 22 digits and 6145, are exceeded), BID, 16 bytes aligned to 16, and is a
  type of its own (`C3230` with a binary operand, no complex type: H.2.6). Its constants
  take `d64x` or `D64x` (and Annex H.5.2's `d32`, `d64`, `d128`, `D32` ... are
  `df`, `dd`, `dl`'s equivalents; C23 only, not hexadecimal: `C1019`); `_Decimal64x` is
  a keyword in C23 (`__axiom_decimal64x` in every mode). In the usual arithmetic
  conversions (H.4.3) `_Decimal32` and `_Decimal64` convert to it and it converts to
  `_Decimal128`, whose values are the same (the interchange type wins); a conversion
  between the two changes no bits (a signaling NaN stays one), folded or at run time.
  Its arithmetic is decimal128's, every ABI passes it as `_Decimal128`, and debug
  information names it (`DW_ATE_decimal_float`). `<float.h>` has `DEC64X_MANT_DIG` ...
  `DEC64X_SNAN`, `<math.h>` `HUGE_VAL_D64X` and every `d64x` function of H.11.2 (those of
  `_Decimal128`, `std/c/lib/math/fn_d64x.c`: the same values and quanta), with the
  narrowing operations `d32addd64x`, `d64addd64x` and `d64xaddd128` ... (one rounding),
  `<stdlib.h>` `strtod64x` and `strfromd64x`, `<wchar.h>` `wcstod64x`.
- **The binary types' functions**: every function of H.11.2 for `_Float16`, `_Float32`, `_Float64`, `_Float128`,
  `_Float32x` and `_Float64x` (`std/c/include/axc/math_floatn.h`, generated by
  `cmd/internal/tools/gen_floatn.py`, as are the definitions and the test
  `c/lib/math/floatn_names.c`): `_Float16`'s and `_Float128`'s the library's own
  (binary128's exact functions correctly rounded, its elementary ones in its precision;
  binary16's elementary ones correctly rounded: the double function's value rounded
  once, or the `_Float128` function's where that value is too near a rounding boundary;
  `m_f16_c23.c`, `m_f128_c23.c` for C23's additions, `nanf16` and `nanf128` in
  `m_nan.c`); those of `_Float32`, `_Float64`, `_Float32x` and `_Float64x` functions of
  their types that call the `float`, `double`, `long double` or `_Float128` function of
  their format (`fn_f32.c` ...). The narrowing operations between them (`f16addf32` ...
  `f32xsqrtf64x`, the 90 of H.11.2's 7.12.14) are exact and rounded once in the current
  direction (`fn_narrow.c`, `math_narrow.ax` to binary16, 32, 64 and the x87 format);
  the encoding functions `encodefN`, `decodefN`, `fMencfN`, `dMencdecdN` and
  `dMencbindN` (H.11.3; `fn_enc.c`) exist, and `encodedecdN` ... `decodebindN` take
  `unsigned char` arrays as C23 7.12.16 says. `<stdlib.h>`'s `strtofN` and `strfromfN`
  and `<wchar.h>`'s `wcstofN` exist for every binary type (`fn_str.c`; `wcstof16` and
  `wcstof128` read the wide string's characters below 128). `<complex.h>` has every
  complex function of each type (`fn_cx_f32.c` ...), and `<tgmath.h>` chooses each type's
  own functions, real and complex, whose results have that type. `<math.h>` has
  `long_double_t`, `_Float16_t` (`float`), `_Float32_t` ... `_Float128_t` and
  `_Decimal32_t` ... `_Decimal128_t` (H.11, `FLT_EVAL_METHOD` and `DEC_EVAL_METHOD` 0).
- **The string conversions of encodings** (H.12.4, H.12.5): `strfromencfN`, `strtoencfN`
  and `wcstoencfN` for binary16 ... binary128, and `strfromencdecdN`,
  `strfromencbindN`, `strtoencdecdN`, `strtoencbindN`, `wcstoencdecdN` and
  `wcstoencbindN` for decimal32 ... decimal128 (`fn_strenc.c`): the encoding's bytes in
  memory's order (little-endian, `__STDC_ENDIAN_NATIVE__`) decoded and converted by
  `strfromfN` (or `strfromdN`), a string's value converted by `strtofN` (`strtodN`,
  `wcstofN`, `wcstodN`) and encoded, so rounded as those are.
- **`<tgmath.h>`**: an integer argument counts as `_Decimal64x` with a `_Decimal64x`
  argument and as `_Float32x` with a binary extended one (H.13p3); the narrowing macros
  of C23 7.27p11 (`d32add` ... `d64sqrt`) and of H.13p5 (`f16add` ... `f64xsqrt`,
  `d64xadd` ...) call the function of the arguments' type, or of the first type the
  prefix lists whose values include it (`f32add(f32, f32)` is `f32addf64`); `quantize`,
  `samequantum`, `quantum` and `llquantexp` are there (7.27p12).
- **Rounding modes** (H.10): the static directions of `FENV_ROUND` and `FENV_DEC_ROUND`
  apply to the narrowing operations, the encoding conversions and the string
  conversions of encodings called in their scope, the binary or the decimal one by the
  radix.

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

- `longjmp` unwinds nothing on Linux and macOS: the restored stack pointer drops the
  frames. On Windows (x64 and ARM64) the hosted library's `longjmp` has the system
  unwind the frames in between first (`RtlUnwindEx` to the `setjmp` caller's frame,
  with `STATUS_LONGJUMP`; `jmp_buf` is the system's `_JUMP_BUFFER` of each; on ARM64 the
  caller's frame is the one `RtlVirtualUnwind` gives at the point `setjmp` returned
  to), so that the
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
  handler, which the system calls on a thread of its own). On x64 and ARM64, `SIGSEGV`,
  `SIGFPE` and `SIGILL` handlers get the processor's exceptions in the program's own
  code (an access violation, an integer division fault or an unmasked floating-point
  exception, an illegal instruction): a vectored exception handler redirects the
  faulting thread as if the faulting instruction had called a trampoline (whose unwind
  information lets the system's walks, and `longjmp`, go on through the faulting
  function); a handler that returns runs the instruction again. A stack overflow, a
  fault in a DLL, and any fault while the signal's action is `SIG_DFL` or `SIG_IGN`
  are the crash reporter's. (On ARM64 the trampoline is entered with the stack pointer
  at a copy of the faulting thread's `CONTEXT`, which its unwind code
  `MSFT_OP_CONTEXT` names, so the walks have every register of the faulting function.
  The ARM64 forms of `longjmp`'s unwinding and of the fault delivery are built and
  checked for their structure (`pe.mjs` decodes the unwind codes), but nothing here runs
  Windows on ARM64: they have not been executed.) The others
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
  (`log10` and `log1p` from log's table, `log10` with a table of its own, `log2` from
  log's intervals with c/ln 2 as a double-double, which gives r/ln 2 without a product
  to keep exact, `asin`
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
  The usual case of `pow`, `powf`, `log2` and `acosh` is a path with no call (the
  special values and the results near overflow, underflow or a halfway point leave it
  by one call), which needs no stack frame. Against the x86-64 Linux system C library
  (`-O2`, one core, the time of a call on a stream of arguments, measured on a loaded
  machine: CPU time, best of seven): most functions take 0.5 to 1.3 times as long,
  `erf`, `erfc`, `exp10`, `lgamma`, `lgammaf`, `asinh` and `tan` about 1.15 to 1.3,
  `acosh`, `atanh`, `pow` and `powf` 1.4 to 1.5, and `log2` 1.5 to 1.55 (its 0.5 +
  2^-6 ulp needs the exact reduction and the double-double sum: the system library's,
  within 0.53 ulp, has about two thirds of its floating-point operations).
  `std/math` has the same kernels: against its former functions, every one is as fast or
  faster, or within 10% (`atan`, `atan2`, `tan`, `cbrt`; their former versions were less
  accurate: up to 2 ulps for `tan` and `sinh`). `atan2`'s 7% is the latency of its two
  divisions in a row (the quotient picks the table row of the second): an estimate of
  the quotient without a division, by Newton's iteration, made it slower.
- **The complex functions** follow Annex G's special values (G.6) and Kahan's methods
  (`std/c/lib/math/complex_impl.h`, one text for every type), in the forms where nothing
  cancels: `ctanh` from `sinh x cosh x / (sinh^2 x + cos^2 y)` and `sin y cos y / (sinh^2 x
  + cos^2 y)`, `casin` and `cacos` from the square roots of `1 - z` and `1 + z`, `catanh`'s
  real part from `log1p` (near +-1 from the difference of two logarithms), `e^x` beyond the
  overflow of its products as `(e^(x/2))^2`. Each part is computed in pairs of the real
  type (Dekker's error-free transformations) from real functions that give their values
  in such pairs (double: `std/c/rt/math_cx.ax`, from the kernels of `std/internal/math`
  before their last rounding, within 2^-56 or better; the wider types: `math_ld.ax`'s
  `__axc_ldcx`, its triple-double kernels), and rounded once. Measured against an
  arbitrary-precision reference (24,000 arguments per double function over the plane:
  random bits, the unit circle, near +-1 and +-i, along and across the branch cuts, the
  poles and zeros of `ctan` and `ccos`, the overflow and underflow thresholds of `e^x`,
  huge and mixed magnitudes): each part within 0.5 to 0.93 ulp where it is normal, up to
  1.4 ulps where it is subnormal (rounded twice there); the float and `_Float16` forms
  within 0.500 ulp (6,000 arguments per function), the x87 format's and binary128's
  (`long double`, `_Float128`) within 0.50 to 0.58 ulp where the part is normal and 1.1
  where it is subnormal (600 arguments per function). The float and `_Float16` functions are the double ones' values
  rounded to the type. `cpow` is `e^(w log z)` with `log z` and its product with `w` in
  pairs, and where `B = (|Re w| + |Im w|)(|log |z|| + |arg z|)` is beyond what the
  pairs hold to the precision the angle `Im(w log z)` needs (2^12 for double, 2^50 for
  the x87 format), in triple-double (`std/c/rt/math_ld.ax`'s `__axc_ldcpow`: `log |z|`
  and `arg z` within 2^-150, the angle reduced exactly by multiples of pi/2, a part at a
  time): each part within 2 ulps while `B` is below 2^90 for double, 2^120 for float
  (`_Float16`: everywhere), 2^70 for the x87 format and 2^26 for binary128 (whose pairs
  are as precise as triple-double already). Beyond those bounds the error grows as `B
  2^-150` relative: the angle modulo 2 pi then needs `log z` and `arg z` to more than
  150 bits, which would take kernels and tables wider than triple-double (not written).
  Also left to the pairs (and their bound) are a `w` with a part of 2^800 or more and a
  complex `w` with `|arg z|` below 2^-600 or `|log |z||` below 2^-600; a real `w` and an
  argument of `z` too small for the angle's pair take that angle scaled. The double
  functions' finite computations (`cexp`, `csqrt`, `csinh`, `ccosh`, `ctanh`, `casin`,
  `cacos`, `catanh` and those made of them) are `complex_impl.h`'s operations in one
  call of `math_cx.ax`'s `__axc_cxd`, with the processor's fused multiply-add where it
  has one: the same bits (compared on 1.1 million arguments per function, with and
  without the extension). Against the former, less accurate functions (`-O2`, x86-64,
  best of seven): `cexp` and `clog` 1.1 times their time, `csqrt` 1.35, `csinh`,
  `ccosh`, `ctanh` 1.4 to 1.5, `catanh` 1.7, `cpow` 1.75, `casin` and `cacos` 2.1.
- **`long double`** where it is `double` (Windows, macOS on Apple silicon; and with
  `--c-long-double=64`) has the double functions. Where it is wider (the 80-bit x87 format
  on x86-64 Linux and macOS, binary128
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
  (measured: 0.500 for each, both formats). An integer power `powl(x, n)` whose exact
  value fits 128 bits is computed exactly and rounded once (so `x * x`'s halfway cases
  round to even). For the x87 format these functions (and the four below) first take a
  double-double path (`std/c/rt/math_ldx.ax`: the same methods with fewer terms, the
  `asin` and `acos` kernel from Taylor polynomials about j/32, within 2^-84 to 2^-98;
  the logarithms, `asinl`, `acosl`, the inverse hyperbolic functions and `lgammal` of a
  positive argument within 2^-85 to 2^-90, with the sums of their recurrences left
  unnormalized between the steps, so that the chain of dependences through the high
  parts is one product and one addition a step: their time is that chain's latency)
  with Ziv's rounding test: where it cannot decide the 64-bit result (about once in
  2^18 to 2^30 calls, and at exact halfway results) the triple-double path does, so the
  results are the same bits. Speed (x86-64, `-O2`, against the x87 hardware's functions
  of the system library): `sinl`, `cosl`, `tanl`, `acosl`, `powl`, `tgammal` 0.45 to 0.8
  times their time, the others 1.15 to 2.2 (`logl` 1.25, `log1pl` 1.7, `lgammal` 1.25 to
  1.35, `expl` 1.8, `asinl` 1.9, `atanl` 2.0, `asinhl` 2.2); binary128's (triple-double only) take 0.2 to 1.2 times the system
  library's `_Float128` functions (most 0.3 to 0.7). The complex `l` functions are
  `complex_impl.h`'s on these (above). `erfl` and `erfcl` (Taylor's series below 1/2, by
  Horner's rule on a table of its coefficients; above, `erfc x = e^-x^2 erfcx(x)` with
  `erfcx` from Taylor polynomials of degree 31 about the middles of 64 pieces of
  [1/2, 128), whose coefficients `cmd/internal/tools/gen_math_tables.py` computes from
  `erfcx`'s differential equation), `lgammal` and `tgammal` (Stirling's series from 24 up, the
  Taylor series of `lgamma(2 + z)` and the recurrence below, the reflection for negative
  arguments, with the Taylor series about each zero of `lgamma` in (-34, -2) where the
  reflection cancels) compute in triple-double too: within 0.55 ulp of binary128 and
  0.5 + 2^-6 of the x87's (measured on 900 arguments each, near the zeros of `lgamma`
  included). `_Float128`'s `erff128` ... `tgammaf128` are the same.
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
(`[[vendor::x]]`) is ignored, as C23 has an attribute an implementation does not
recognize be; one without a prefix that is not a standard
attribute is ignored with the warning `C4052`.

`#pragma` lines and `_Pragma` operators are read in every mode ("Pragmas"): those this
implementation knows act, the others are ignored (6.10.7p1: a pragma the
implementation does not recognize is ignored).

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
not on WebAssembly. In C23 with the library (hosted, not `--c-lib=system`),
`__STDC_IEC_60559_TYPES__` is `202311L` (Annex H, "Annex H: the interchange and
extended types" below), on every target: on WebAssembly by its decimal part (H.1p2).

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
| `__AXIOM_ASM_X86_64__`, `__AXIOM_ASM_AARCH64__`, `__AXIOM_ASM_WASM__` | 1, defined only in a GNU dialect on x86-64, AArch64 or WebAssembly targets: the language of inline assembly's templates ("Inline assembly"). |

The target's conventional macros are defined in every standard and dialect, with the
compiler's C library and with `--c-lib=system` alike (`c/ctarget.ax`, one table). They
are not the standard's, so `-U` and `-D` (and `#undef`, `#define`) change them, a
definition replacing the target's; `c/pp/predef_target_*.c` show them for each target.

| Macros | Defined |
|---|---|
| `__x86_64__`, `__x86_64`, `__amd64__`, `__amd64` | x86-64 targets; on Windows also `_M_X64` and `_M_AMD64` (100). |
| `__aarch64__`, `__AARCH64EL__` | AArch64 targets; on macOS also `__arm64__` and `__arm64`, on Windows `_M_ARM64`. |
| `__wasm`, `__wasm__`, and `__wasm32`, `__wasm32__` or `__wasm64`, `__wasm64__` | WebAssembly. No system's macros: a module's only system is its host's imports. |
| `__linux__`, `__linux`, `__unix__`, `__unix`, `__ELF__` | Linux; in a GNU dialect also `unix` and `linux`. Not `__gnu_linux__`: the C library is not that system's. |
| `__APPLE__`, `__MACH__`, `__ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__`, `__ENVIRONMENT_OS_VERSION_MIN_REQUIRED__` | macOS; the last two are 110000, the oldest system the executables run on (macOS 11.0), which the system's headers read. |
| `_WIN32`, `_WIN64` | Windows (the platform's own spellings: no `__WIN32__`, `WIN32` or the like). |
| `__LP64__`, `_LP64` | 32-bit `int`, 64-bit `long` and pointers (Linux, macOS, wasm64). |
| `__ILP32__`, `_ILP32` | 32-bit `int`, `long` and pointers (wasm32). |
| `__CHAR_UNSIGNED__`, `__WCHAR_UNSIGNED__` | Plain `char` (AArch64 Linux, `--c-char=unsigned`), `wchar_t` (AArch64 Linux, Windows) is unsigned. |
| `__CHAR_BIT__`; `__SIZEOF_SHORT__`, `__SIZEOF_INT__`, `__SIZEOF_LONG__`, `__SIZEOF_LONG_LONG__`, `__SIZEOF_FLOAT__`, `__SIZEOF_DOUBLE__`, `__SIZEOF_LONG_DOUBLE__`, `__SIZEOF_POINTER__`, `__SIZEOF_SIZE_T__`, `__SIZEOF_PTRDIFF_T__`, `__SIZEOF_WCHAR_T__`, `__SIZEOF_WINT_T__` | The sizes in bytes, after `--c-int`, `--c-long` and `--c-long-double`. In a GNU dialect also `__SIZEOF_INT128__` and `__SIZEOF_FLOAT128__` (16), whose types only those dialects have. |
| `__SCHAR_MAX__`, `__SHRT_MAX__`, `__INT_MAX__`, `__LONG_MAX__`, `__LONG_LONG_MAX__`, `__<T>_WIDTH__` | `<limits.h>`'s values (`__SCHAR_WIDTH__` ... `__LONG_LONG_WIDTH__`). |
| `__SIZE_TYPE__`, `__PTRDIFF_TYPE__`, `__WCHAR_TYPE__`, `__WINT_TYPE__`, `__CHAR16_TYPE__`, `__CHAR32_TYPE__`, `__SIG_ATOMIC_TYPE__`, `__INTMAX_TYPE__`, `__UINTMAX_TYPE__`, `__INTPTR_TYPE__`, `__UINTPTR_TYPE__`, `__INT<N>_TYPE__`, `__UINT<N>_TYPE__`, `__INT_LEAST<N>_TYPE__`, `__INT_FAST<N>_TYPE__` (and unsigned) | The types the headers give those typedefs (`<stddef.h>`, `<stdint.h>`, `<wchar.h>`, `<uchar.h>`, `<signal.h>`), with N 8, 16, 32 (only when a type has 32 bits) and 64; each with `__<T>_MAX__` (and `__WCHAR_MIN__`, `__WINT_MIN__`, `__SIG_ATOMIC_MIN__`) and `__<T>_WIDTH__` for the least, fast, pointer and widest types, `__SIZE_WIDTH__` ..., of the type of the header's limit; `__INT<N>_C(c)`, `__UINT<N>_C(c)`, `__INTMAX_C(c)`, `__UINTMAX_C(c)` as `INT<N>_C` ... |
| `__FLT_<X>__`, `__DBL_<X>__`, `__LDBL_<X>__`; `__FLT_RADIX__`, `__FLT_EVAL_METHOD__`, `__DECIMAL_DIG__` | `<float.h>`'s values: `MANT_DIG`, `DIG`, `DECIMAL_DIG`, `MIN_EXP`, `MIN_10_EXP`, `MAX_EXP`, `MAX_10_EXP`, `MAX`, `NORM_MAX`, `MIN`, `EPSILON`, `DENORM_MIN` (`TRUE_MIN`), `HAS_DENORM`, `HAS_INFINITY`, `HAS_QUIET_NAN`, `IS_IEC_60559`. |
| `__ORDER_LITTLE_ENDIAN__` (1234), `__ORDER_BIG_ENDIAN__` (4321), `__ORDER_PDP_ENDIAN__` (3412), `__BYTE_ORDER__`, `__FLOAT_WORD_ORDER__`, `__LITTLE_ENDIAN__` | Every target is little-endian. |
| `__BIGGEST_ALIGNMENT__` | The largest alignment of a type: 16, or on x86-64 32 with AVX and 64 with AVX-512F (the vector types'). |
| `__USER_LABEL_PREFIX__`, `__REGISTER_PREFIX__` | The prefix of a C name's symbol in assembly (`_` on macOS, else empty), of a register's (empty). |
| `__MMX__`, `__SSE__`, `__SSE2__`, `__FXSR__`, `__SSE_MATH__`, `__SSE2_MATH__` | x86-64 (its baseline); with `--cpu`/`--features` also `__SSE3__`, `__SSSE3__`, `__SSE4_1__`, `__SSE4_2__`, `__POPCNT__`, `__LZCNT__`, `__BMI__`, `__BMI2__`, `__AVX__`, `__AVX2__`, `__FMA__`, `__AES__`, `__PCLMUL__`, `__SHA__`, `__F16C__`, `__MOVBE__`, `__ADX__`, `__AVX512F__`, `__AVX512CD__`, `__AVX512BW__`, `__AVX512DQ__`, `__AVX512VL__`, `__AVX512IFMA__`, `__AVX512VBMI__`, `__AVX512VBMI2__`, `__AVX512VNNI__`, `__AVX512BITALG__`, `__AVX512VPOPCNTDQ__`, `__AVX512BF16__`, `__AVX512FP16__`, `__GFNI__`, `__VAES__`, `__VPCLMULQDQ__`, `__AVXVNNI__`, `__SHA512__`, `__SM3__`, `__SM4__`, `__AVX10_1__`, `__PRFCHW__` for the extensions enabled. |
| `__ARM_ARCH` (8), `__ARM_ARCH_ISA_A64`, `__ARM_ARCH_PROFILE` (`'A'`), `__ARM_64BIT_STATE`, `__ARM_PCS_AAPCS64`, `__ARM_NEON`, `__ARM_FP` (0xE), `__ARM_FEATURE_FMA`, `_CLZ`, `_IDIV`, `_UNALIGNED`, `_NUMERIC_MAXMIN`, `_DIRECTED_ROUNDING`, `__ARM_SIZEOF_WCHAR_T`, `__ARM_SIZEOF_MINIMAL_ENUM` | AArch64 (the Arm C Language Extensions' macros of what every A64 processor has); for the extensions enabled `__ARM_FEATURE_ATOMICS` (lse), `_RCPC`, `_DOTPROD`, `_AES`, `_SHA2`, `_CRYPTO` (both), `_SHA3`, `_SHA512`, `_SM3` and `_SM4` (sm4), `_CRC32`, `_QRDMX` (rdm), `_FP16_SCALAR_ARITHMETIC` and `_FP16_VECTOR_ARITHMETIC` (fp16), `_FP16_FML` (fhm), `_BF16`, `_MATMUL_INT8` (i8mm), `_SVE`, `_SVE2`, `_SVE2p1`, `_SME`, `_SME2`, `_SME2p1`, `_SME_F64F64`, `_SME_I16I64`, `_SME_F16F16`, `_SVE_B16B16`. |
| `__wasm_simd128__`, `__wasm_bulk_memory__`, `__wasm_nontrapping_fptoint__`, `__wasm_sign_ext__`, `__wasm_mutable_globals__`, `__wasm_atomics__`, `__wasm_multivalue__`, `__wasm_relaxed_simd__`, `__wasm_tail_call__`, `__wasm_exception_handling__` | WebAssembly, for the proposals enabled. |
| `__OPTIMIZE__`, `__NO_INLINE__` | `-O1` and above, else `-O0`. |
| `__PIC__`, `__pic__` (2) | Position-independent code: `--pic` and `--emit=shared` on Linux, always on macOS; `__PIE__`, `__pie__` (2) on macOS for an executable, which is position-independent. Not on Windows or WebAssembly. |

The feature macros say what the target's processor has; the headers of its intrinsic
functions are in "Intrinsics" below. No macro names a compiler (`__GNUC__`, `__clang__`, `_MSC_VER`, `__VERSION__` are not
defined; a GNU dialect's `__ATOMIC_*` and `__BFLT16_*` are feature macros): programs
test features, and the compiler's own macros are the `__AXIOM_` ones above.

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
| Signed overflow | Undefined. The `+`, `-` and `*` of signed integers (in the promoted type; not of `__int128` or `_BitInt`) are marked for the optimizer as not overflowing (`ir.F_NSW`), and from `-O1` on it uses that: a loop counter is widened to 64 bits and its sign extensions folded (`loops.ax`), the range of an index is taken from its counter (`gvn.ax`), `sext(a + b)` may be computed as `sext(a) + sext(b)`. So an overflow does not always wrap: a program must not rely on it. At `-O0` the instructions wrap. |
| Right shift of a negative value | Arithmetic (sign-extending): `-8 >> 1` is `-4`. |
| Division | Truncates toward zero; `%` has the sign of the dividend. |
| Complex types | `float _Complex`, `double _Complex` and `long double _Complex` (C99 and later; also of `_Float16` ... `_Float64x`) are laid out as an array of two elements of the real type, the real part first: the size is twice the real type's, the alignment the real type's. The imaginary types (`float _Imaginary`, `double _Imaginary`, `long double _Imaginary`, C99 and later, Annex G) have their real type's representation (the imaginary part); their arithmetic is G.5's (an imaginary times an imaginary is real, a real plus an imaginary complex, ...), their conversions G.4's (an imaginary converted to a real type is 0, a complex one to an imaginary type its imaginary part). |
| Interchange and extended floating types | (C23 Annex H, and in every mode before C23.) `_Float16` is binary16, `_Float32` binary32, `_Float64` and `_Float32x` binary64, `_Float128` binary128 (`__float128` in a GNU dialect), `_Float64x` the x87 format where `long double` has it, else binary128; each a type of its own. `_Float16`'s operations are rounded to binary16 each (`FLT_EVAL_METHOD` 0: computed in `float`, which is exact enough that the one rounding is correct), `_Float128`'s and `_Float64x`'s are `long double`'s software arithmetic in that format. The usual arithmetic conversions take the type with more values, then of two with the same values an interchange type over a standard one over an extended one (`float + _Float32` is `_Float32`, `double + _Float32x` is `double`). Constants take `f16`, `f32`, `f64`, `f128`, `f32x`, `f64x` (either case). `<float.h>` has their macros (`FLT16_MAX` ...) under `__STDC_WANT_IEC_60559_TYPES_EXT__`. `_Decimal64x` (C23, Annex H.2.4) is decimal128 (H.2.4p4 allows it), 16 bytes aligned to 16, passed as `_Decimal128` is; a type of its own ("Annex H: the interchange and extended types"). |
| Floating types | `float` and `double` are IEEE 754 binary32 and binary64; `long double` as the table above says. `FLT_EVAL_METHOD` is 0, `FLT_ROUNDS` is 1 (to nearest): it is the start-up's mode, which `fesetround` changes on x86-64 and AArch64 (`<fenv.h>`, below; `FLT_ROUNDS` itself stays 1). A `long double` wider than `double` is computed in software (`std/c/rt/ldouble.ax`, written from IEEE 754-2019): each operation and conversion correctly rounded to nearest, ties to even, with subnormals (`+ - * /` of normal operands with a normal result by 64-bit integer fast paths, division by Knuth's Algorithm D); constant expressions are folded with the same arithmetic, and floating constants read exactly. Conversions of such a value to an integer type truncate and saturate, as those of `float` and `double` do (the behavior is undefined when the value does not fit). A NaN result of an invalid operation is the format's default NaN (the x87's negative "indefinite"; binary128's positive quiet NaN); a NaN operand gives its own, quieted (of two x87 NaNs, the one with the larger significand). x87 encodings the x87 itself treats as invalid (unnormals, pseudo-infinities, pseudo-NaNs) are invalid operands. The x87 format's six bytes of padding are zero in the values the program computes. |
| Sizes and alignment | A scalar's alignment is its size. A structure is laid out naturally: each member at the next multiple of its alignment, the structure aligned as its strictest member, its size rounded up to that; `#pragma pack(n)` caps the members' alignments at n ("Pragmas"). |
| `max_align_t` | Aligned as the platform's (above): a structure of a `long long` and a `long double` aligned to 16 (size 32 where `long double` is 16 bytes), or `long double` (8 bytes). |
| Alignment specifiers | `_Alignas` takes any power of two up to 4096 (more is `C3071`). |
| Bit-fields | Laid out as the target's ABI lays them out: allocated from the least significant bit of a storage unit of the declared type, never straddling two units. On x86-64 Linux, macOS and WebAssembly (the System V rules) a bit-field goes at the next bit if it fits in the unit that holds it, an unnamed one adds nothing to the alignment of the structure, and one of width 0 moves the next member to the next unit of its type. On AArch64 Linux (AAPCS64) the same, but every bit-field's type adds to the alignment, unnamed ones and those of width 0 too. On Windows (x64 and ARM64) bit-fields are allocated in runs: a bit-field joins the open unit only if its type has the unit's size and it fits, else it starts a new unit after it; another member closes the unit; one of width 0 closes it (and aligns the next member to its type) and is ignored when no unit is open; in a union, bit-fields add their type's size but not its alignment. In a packed structure or under `#pragma pack` (GNU dialects; any `pack(n)`), a bit-field goes at the next bit (System V, AAPCS64; AAPCS64 still aligns the structure to a zero-width one's type), while Windows keeps its units, aligned to at most n. A plain `int` bit-field is signed. A `char` bit-field has `char`'s signedness. |
| Enumerations | The type is `unsigned int` if no enumerator is negative, else `int`, when all fit; the enumeration constants are `int`. In C23, if some enumerator does not fit `int`, the first of `long`, `long long` and, in a GNU dialect, `__int128` (`unsigned long`, `unsigned long long`, `unsigned __int128` for an unsigned type) that holds all of them, and then the constants have the enumerated type; while the list is read, a constant outside `int` has the type of its expression (a `_BitInt` too), and an implicit one (the previous plus 1) the previous one's type, or if that does not hold it the first of those wider types of its signedness that does (6.7.2.2p11). No such type: `C3013`. Before C23 a value outside `int` is `C3013`. With a fixed underlying type (C23), that type, which may be `__int128` or `unsigned __int128` in a GNU dialect (with any value of it). |
| `_BitInt(N)` | `BITINT_MAXWIDTH` is 65535. For N up to 64 an object is stored in the smallest of 8, 16, 32 or 64 bits that holds N (aligned as that integer type, as the x86-64 and AArch64 ABIs specify), extended to it; as an argument or result it is that integer. Wider, it is stored in 64-bit limbs, the least significant first, as many as hold N, aligned to 8 (the x86-64 System V psABI's and Windows' array of 8-byte chunks; WebAssembly the same), or on AArch64 (Linux and macOS) in 16-byte units aligned to 16 (AAPCS64: `_BitInt(65)` to `_BitInt(128)` as `__int128`): `sizeof(_BitInt(129))` is 24, 32 on AArch64. The value is sign- or zero-extended through its storage; the bits beyond N that arrive (arguments, results, and objects in memory, which code compiled elsewhere writes) are not relied on: the ABIs leave them unspecified. |
| `__int128` | GNU dialects: `__int128` and `unsigned __int128` are 16 bytes aligned to 16 on every target, an extended integer type whose rank is above `long long` and above `_BitInt(128)`'s, below `_BitInt(129)`'s. `__AXIOM_INT128__` is defined (1). `intmax_t` stays 64 bits (C23 7.22.1.5 lets an extended type be wider), and so does the preprocessor's arithmetic. |
| Integers wider than 64 bits | `__int128` and `_BitInt(N)` for N > 64. Their arithmetic is exact to the type's width: two-limb values (up to 128 bits) inline (+ and - as add/adc and sub/sbb on x86-64, adds/adc and subs/sbc on AArch64, * with the 64-by-64-bit product's high half, shifts, comparisons, the bitwise operations; / and % inline when both operands are below 2^64, else a function: on x86-64 one generated into the program that uses the 128-by-64-bit `div`, elsewhere a run-time function), wider ones by the functions of `std/c/rt/wideint.ax` (generated into a program only when it calls them); division is Knuth's Algorithm D. Constant expressions are folded with the same functions, in the type's precision. Conversions to `float` and `double` are correctly rounded (to nearest, ties to even), as are those to a wide `long double` (`std/c/rt/ldouble.ax`); from them they truncate toward zero and saturate (undefined when the value does not fit, 6.3.1.4). A signed result that overflows wraps (undefined: a program must not rely on it; these operations are not marked as not overflowing, as the narrower types' are), but in a constant expression it is `C3036`. Bit-fields of these types are laid out as the ABIs say: in a unit of the type's size aligned as the type is, the last such window at or before the next free bit that holds it, else the next (System V, and for `__int128` AAPCS64; checked against the system's compilers on x86-64 and, for `__int128`, AArch64 Linux), on Windows in its runs of units of the type's size at its alignment; in a packed structure (System V, AAPCS64, WebAssembly) at the next bit, reached through the bytes it spans ("Attributes": `packed`). An enumeration may have `__int128` or `unsigned __int128` for its type ("Implementation-defined behavior": enumerations); one whose fixed underlying type is a `_BitInt` is `C3075` (C23 6.7.2.2p5). A `switch` on them takes case ranges. An integer constant expression where a value of at most 64 bits is needed (an alignment, a bit-field's width, an array's length: `C3009`) whose value is wider is `C3212`. `<stdbit.h>`'s type-generic functions take `unsigned __int128` and the unsigned `_BitInt` types (C23 7.18.1p4: those whose width is a standard or extended type's; here any above 64 bits too), `<stdckdint.h>`'s and the GNU `__builtin_*_overflow` functions any of them. An `_Atomic` one of 16 bytes is lock-free where 16-byte atomics are ("Atomics and threads"), a wider one under the lock; the GNU `__atomic` builtins' generic forms take them, the others only objects of 1, 2, 4 or 8 bytes. `printf` and `scanf` have no conversion for them: C23's `%wN` length modifiers are for the widths of the `intN_t` types `<stdint.h>` defines, which stop at 64. `intmax_t` and `uintmax_t` stay 64 bits on every target (C23 7.22.1.5 lets an extended or bit-precise type be wider), as do `INTMAX_WIDTH`, `%jd` and the preprocessor's arithmetic, in which a `wb` constant wider than 64 bits is `C1016`. |
| Null pointer | All bits zero. |
| Source and execution character sets | UTF-8. Narrow string literals are UTF-8 bytes (invalid UTF-8 in a literal is `C1024`). `L"..."` is UTF-16 on Windows (`wchar_t` is 16 bits) and UTF-32 elsewhere; `u"..."` is UTF-16 and `U"..."` UTF-32. |
| Multi-character constants | `'ab'` is `'a' * 256 + 'b'` as an `int` of the build's width (the first character is the most significant); a warning, `C4102`. A wide one has the value of its last character. More characters than the type holds is `C1011`. |
| `main` | `int main(void)` and `int main(int, char **)` only (`C3061`). Reaching the end of `main` returns 0, in every mode. |
| Inline functions | An inline definition (6.7.4p7) is compiled with internal linkage, for the unit's own calls: the external function is another unit's. |
| `volatile` | Accesses are marked volatile for the optimizer. |
| `restrict` | Used by the optimizer on pointer parameters, on block-scope pointer objects (automatic and static), on file-scope ones and on structure members ("Optimization"). The restricted pointer a pointer to one (`int *restrict *p`) points to is designated by no declaration, which 6.7.3.1p1 needs: no rule follows from it, and it has no effect. |

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
| `<float.h>` | all | `FLT_`, `DBL_` and `LDBL_` macros (the `LDBL_` ones and `DECIMAL_DIG` of the format of `long double`). In C23 `FLT_SNAN`, `DBL_SNAN` and `LDBL_SNAN` (signaling NaNs that stay signaling in the objects they initialize), and the `DEC32_`, `DEC64_` and `DEC128_` macros (`_SNAN` too: decimal signaling NaNs stay signaling) and `DEC_EVAL_METHOD` (0). |
| `<limits.h>` | all | `LLONG_` from C99; `*_WIDTH`, `BOOL_MAX` and `BITINT_MAXWIDTH` (65535) in C23. |
| `<stdarg.h>` | all | `va_list`, `va_start`, `va_arg`, `va_copy`, `va_end`. In C23 `va_start` needs no second argument. |
| `<stddef.h>` | all | `size_t`, `ptrdiff_t`, `wchar_t`, `NULL`, `offsetof`; `max_align_t` from C11; `nullptr_t` and `unreachable` in C23. |
| `<iso646.h>` | C95 and later | |
| `<stdbool.h>` | C99 and later | In C23, `bool`, `true` and `false` are keywords and the header only defines `__bool_true_false_are_defined`. |
| `<stdint.h>` | C99 and later | No `int32_t` family when no type is 32 bits wide. |
| `<complex.h>` | C99 and later | `complex`, `I` and `_Complex_I` (a constant of type `float _Complex`), `imaginary` and `_Imaginary_I` (of type `const float _Imaginary`), `CMPLX`, `CMPLXF` and `CMPLXL` (C11; constant expressions when the operands are), and the declarations of every function. `I` is `_Imaginary_I` where Annex G is claimed (x86-64, AArch64: `__STDC_IEC_559_COMPLEX__`), else `_Complex_I`. `creal`, `cimag` and `conj` (and their `f` and `l` forms) are also macros the compiler expands in line; the others are the library's ("`<math.h>`, `<fenv.h>`, `<tgmath.h>` and `<complex.h>`'s functions"). |
| `<math.h>` | all | C89's double functions; from C99 everything of C17 7.12, in the three types, and C23's listed below. |
| `<fenv.h>` | C99 and later | All of C17 7.6; C23's `fegetmode`, `fesetmode`, `fesetexcept`, `fetestexceptflag`. |
| `<tgmath.h>` | C99 and later | Every type-generic macro of C17 7.25 (and of C23's functions the library has), through `__builtin_axiom_tg`, in C99 too. Decimal arguments choose the `d32`, `d64`, `d64x` and `d128` functions (C23 7.27p3, H.13p3); the narrowing macros `d32add` ... (7.27p11; Annex H's `f32add` ... `d64xsqrt`, H.13p5) and the quantum macros (7.27p12) are there; arguments of the interchange and extended types (Annex H.11) their common real type's own functions (`sqrtf16` ... `sqrtf64x`, `cexpf32` ..., declared with `__STDC_WANT_IEC_60559_TYPES_EXT__`; without it float's function for `_Float16` and the standard function of the format for `_Float32` ... `_Float64x`). |
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

## Intrinsics

The processors' intrinsic functions, in every mode (ISO standards too) and with or
without the C library, are the compiler's headers:

| Header | What |
|---|---|
| `<mmintrin.h>` | MMX's `__m64` functions (`_mm_add_pi8`, `_m_paddb` ...), computed in the low half of an SSE register (the MMX registers are never used, and `_mm_empty` does nothing), and the types of the others. |
| `<xmmintrin.h>`, `<emmintrin.h>` | SSE and SSE2, complete, with `_MM_SHUFFLE`, `_MM_TRANSPOSE4_PS`, the MXCSR macros (`_MM_SET_ROUNDING_MODE` ...), `_mm_prefetch` and its hints, `_mm_malloc` and `_mm_free` (`<mm_malloc.h>`, in a hosted program). |
| `<pmmintrin.h>`, `<tmmintrin.h>`, `<smmintrin.h>`, `<nmmintrin.h>` | SSE3, SSSE3, SSE4.1 (with `_MM_FROUND_*`), SSE4.2 (the string compares, `_SIDD_*`, CRC32) and POPCNT. |
| `<wmmintrin.h>` | AES and PCLMULQDQ. |
| `<immintrin.h>` | All of the above, and AVX, AVX2 (with the gathers), FMA, F16C, BMI1, BMI2, LZCNT, POPCNT, ADX (and the carry chains `_addcarry_u32` ... of any processor), SHA, RDRAND, RDSEED, `_xgetbv`, `_rdtsc`/`__rdtsc`, `__rdtscp`, `_mm_pause`, the bit operations `_bswap`, `_bit_scan_forward`, `_rotl` ... and `_castf32_u32` .... |
| `<x86intrin.h>` | The same as `<immintrin.h>`. |
| `<cpuid.h>` | `__cpuid(leaf, a, b, c, d)` and `__cpuid_count`, `__get_cpuid_max`, `__get_cpuid`, `__get_cpuid_count`, and the feature bits `bit_SSE3` ... of leaves 1, 7 and 0x80000001. |
| `<arm_neon.h>` | AArch64's Advanced SIMD, complete: the types `int8x8_t` ... `float64x2_t`, `poly8x8_t` ... `poly64x2_t`, `float16x4_t`, `bfloat16x4_t` ..., the structures `int8x8x2_t` ... `x4`, and `poly128_t`; every intrinsic of the arithmetic, saturating, halving and rounding forms, widening and narrowing, multiply-accumulate (by vectors, scalars and lanes), compares, shifts by vectors and immediates, the bit operations and `vbsl`, counts and reversals, min and max with the NaN-aware forms, pairwise operations and reductions (`vaddv`, `vmaxnmv` ...), conversions with each rounding mode and to and from fixed point, the estimates and steps (`vrecpe`, `vrsqrts` ...), `vld1` ... `vld4` and `vst1` ... `vst4` with their lane, duplicating and `_x2` ... `_x4` forms, lane get, set, copy and duplicate, `vcombine`, `vget_low`/`high`, `vext`, `vzip`/`vuzp`/`vtrn` and their halves, `vtbl`, `vtbx`, `vqtbl`, `vqtbx`, every `vreinterpret`; and of the extensions: FP16's vector and scalar arithmetic, BF16's (`vbfdot`, `vbfmmla`, `vbfmlalb` ..., conversions), FHM's `vfmlal_low_f16` ..., RDM's `vqrdmlah`, the dot products (`vdot`, and `vusdot`, `vsudot`, `vmmla` of i8mm), AES, SHA-1, SHA-256, SHA-512, SHA-3 (`veor3q`, `vbcaxq`, `vrax1q`, `vxarq`), SM3, SM4 and `vmull_p64`. |
| `<wasm_simd128.h>` | WebAssembly's vector instructions (simd128) and relaxed-simd: `v128_t`, and `wasm_v128_load` ... (with the splatting, zero-extending, widening and lane forms), `wasm_v128_store` and its lane forms, `wasm_i8x16_make`, `_const`, `_const_splat`, `_splat`, `_extract_lane`, `_replace_lane` of every shape, the shuffles and `wasm_i8x16_swizzle`, the bit operations, `wasm_v128_bitselect` and `wasm_v128_any_true`, every shape's arithmetic (saturating, `wasm_u8x16_avgr`, `wasm_i16x8_q15mulr_sat`, the extended multiplies, the pairwise sums, `wasm_i32x4_dot_i16x8`), compares, `_all_true`, `_bitmask`, shifts, `wasm_i8x16_popcnt`, narrowing and extension, the floating ones (`min`, `max`, `pmin`, `pmax`, `sqrt`, the roundings), the conversions with their saturating truncations, demotion and promotion; and relaxed-simd's `wasm_i8x16_relaxed_swizzle`, `_relaxed_trunc`, `_relaxed_madd`, `_relaxed_nmadd`, `_relaxed_laneselect`, `_relaxed_min`, `_relaxed_max`, `wasm_i16x8_relaxed_q15mulr` and the relaxed dot products. The unsigned names (`wasm_u32x4_add` ...) of the operations that do not depend on the sign are there too. |
| `<arm_fp16.h>`, `<arm_bf16.h>` | `float16_t` and the scalar FP16 intrinsics (`vaddh_f16`, `vcvth_s32_f16` ...); `bfloat16_t` and its scalar conversions. `<arm_neon.h>` includes both. |
| `<arm_acle.h>` | `__crc32b` ... `__crc32cd` (CRC), `__clz`, `__cls`, `__rev`, `__rev16`, `__revsh`, `__rbit`, `__ror` and their `l` and `ll` forms, `__dmb`, `__dsb`, `__isb`, `__nop`, `__yield`, `__wfe`, `__wfi`, `__sev`, `__sevl`, `__pld`, `__pli`, `__pldx`, `__plix`, `__sqrt`, `__sqrtf`, `__fma`, `__fmaf`, `__rintn`, `__rintnf`. |

On another target each is an `#error`. The x86-64 types are `__m64`, `__m128`, `__m128d`,
`__m128i`, `__m256`, `__m256d`, `__m256i` (vectors of `vector_size`, which may alias any
object: no type-based aliasing is assumed anyway) and their unaligned forms `__m128_u`
... `__m256i_u` (aligned to 1: the `loadu`/`storeu` intrinsics are accesses through
them). An intrinsic is a static function, always inlined, or a macro where an operand
must be a constant (an immediate: one that is not is `C3241`); each compiles to its
instruction:

- what C's vector operators say (add, sub, mul, the bit operations, compares, lane
  access, `set`, loads and stores, casts) is plain C on the vector types, and the
  instruction selector picks the instruction (`paddd`, `cmpltps`, `movdqu` ...);
- what the IR has an operation for (saturating arithmetic, integer min and max, abs,
  sqrt, floor and ceil, fma, the widening conversions of SSE4.1, shuffles, blends,
  unpacks, shifts by an immediate with the processor's rule that a count of the lane's
  width or more gives 0, the lanes' top bits of `movemask`, `pmaddwd`) is that
  operation (`__builtin_axiom_simd`, `__builtin_axiom_shufflevector`,
  `__builtin_axiom_convertvector`): the optimizer sees through it, and it is selected as
  the instruction (`paddusb`, `pmaddwd`, `pmovmskb` ...);
- everything else is the instruction itself (`pshufb`, `psadbw`, `packsswb`, the
  horizontal adds, `cvtps2dq` and `cvttps2dq` with their out-of-range result
  0x80000000, `minps` and `maxps` with their rule for NaNs and zeros, `roundps`,
  `dpps`, `pclmulqdq`, `aesenc`, the SHA instructions, `crc32`, `pcmpistri` ...,
  `vpermd`, the gathers, `pdep`, `rdtsc` ...): a pure asm statement made from a
  table of the instructions' forms (`cmd/axiom/src/c/csimd_tab.ax`), which the
  assembler checks and encodes, and the optimizer still removes when unused, merges
  and moves. In code compiled with AVX the VEX form of each SSE instruction is used.

An intrinsic needs its extension: called from a function compiled without it (the
build's `--cpu` and `--features`, and the function's attribute `target`, which in an
ISO mode is spelled `__axiom_attribute__((__target__("avx2")))`) it is `C3240`, at the
call, never an instruction the processor may lack. A program built for the baseline
can call intrinsics of more extensions from functions with `target`, after checking the
processor (`<cpuid.h>`, or `__builtin_cpu_supports` in a GNU dialect). The 256-bit
intrinsics that are instructions of their own (`_mm256_hadd_ps`, `_mm256_round_ps`,
`_mm256_cvtps_epi32` ...) need AVX2 as well as AVX or FMA: the code generator computes
256-bit vectors in one register only with AVX2, and in pieces of 128 bits with AVX
alone, which an instruction of its own cannot be (the plain ones, `_mm256_add_ps` ...,
need AVX only).

The AArch64 headers are made the same way (`cmd/internal/tools/gen_neon_intrin.py`
writes them, the table's AArch64 part and their tests). On 128-bit vectors what C's
operators and the IR's operations say is plain C (`vaddq_s32` is `add`, `vqaddq_u8`
`uqadd`, `vmlaq_s32` `mla`, `vfmaq_f32` `fmla`, `vcgeq_f32` `fcmge`, `vshrq_n_s32` `sshr`,
`vmovl_high_s8` `sxtl2`, `vzip1q_s8` `zip1`, `vld1q_u8` and `vst1q_u8` `ldr` and `str`
...), selected by the instruction selector and seen through by the optimizer;
everything else, and every operation of 64-bit vectors (`int8x8_t` ..., which the IR
has no vector type of: it holds one as a D register's bits), is its instruction, a pure
asm statement (`ld2` ... `st4` and their lane forms, which read or write memory, are
volatile ones). The extensions are `--features`' (`dotprod`, `i8mm`, `rdm`, `fp16`,
`fhm`, `bf16`, `aes`, `sha2`, `sha3`, `sha512`, `sm4`, `crc`) and the attribute
`target`'s, with the same rule: an intrinsic of one the calling function is not compiled
for is `C3240`. A lane or a shift count must be a constant in its range (`C3241`). The
`vmla` and `vmls` of floating vectors round the product (they are not fused; `vfma` is).
Not provided, for want of the extension among `--features`: the complex arithmetic of
FCMA (`vcaddq_rot90_f32`, `vcmlaq_f32` ...), FRINTTS's `vrnd32x` ..., JSCVT's `__jcvt`,
RNG's `__rndr`, MTE's and TME's intrinsics; and `__arm_rsr` and `__arm_wsr` (whose
system register is named by a string). Plain C on 64-bit vectors (`a + b` of two
`int8x8_t`) is computed lane by lane: the intrinsics are the vector instructions.

Deviations, all of them of what no program can see but a debugger or a timer:
`_mm_stream_*` and `_mm_stream_load_*` are ordinary loads and stores (no non-temporal
hint), `_mm_maskmoveu_si128` and `_mm_maskmove_si64` store the selected bytes one by
one, `_mm_load_*` of an aligned type does not check the alignment, and `_mm_comieq_ss`
and `_mm_comineq_ss` raise invalid for a signaling NaN only (C's `==`). RDRAND and
RDSEED are not among the extensions of `--features`: their intrinsics are not checked
(test `bit_RDRND` and `bit_RDSEED` of `<cpuid.h>` first). Not provided: the
AVX-512 intrinsics, AMX, KL, the system instructions' (`_xsave`, `_fxsave` ...),
and `<arm_sve.h>` (until then, a program that includes it when `__ARM_FEATURE_SVE` is
defined needs its portable path chosen with `-U`).

`<wasm_simd128.h>` (made by `cmd/internal/tools/gen_wasm_intrin.py`) needs simd128,
which every WebAssembly build has unless `--features=-simd128` (then it is an `#error`).
Its operations are plain C or the IR's where they say it (add, mul, the bit operations,
compares, shifts, with WebAssembly's count modulo the lane width, saturating add, min,
max, abs, sqrt, rounding, the lanes, loads and stores, shuffles, `convert`), else the
instruction, an asm statement in the text format. A shuffle's indices and a lane must be
constants in range (`C3241`, or `C3070` from the range's check). The relaxed-simd
intrinsics need `+relaxed-simd` (`C3240` otherwise); on WebAssembly an attribute
`target("relaxed-simd")` asserts that the engine has it, the features being the
module's. The `_const` macros accept any value, constant or not.

## Not available

Each of these is what the target, the standard or the chosen dialect leaves out, not
work left undone:

- Annex F and Annex G on WebAssembly (no floating-point environment: no rounding modes,
  no exception flags), and with `--c-lib=system` or `--c-freestanding`: there
  `__STDC_IEC_559__`, `__STDC_IEC_60559_BFP__`, `__STDC_IEC_559_COMPLEX__` and
  `__STDC_IEC_60559_COMPLEX__` are not defined. On x86-64 and AArch64 with the library
  they are (`docs/c-conformance.md`, "Annexes"). Annex H (`__STDC_IEC_60559_TYPES__`)
  is not claimed with `--c-lib=system` or `--c-freestanding`, nor before C23.
- `setjmp` on WebAssembly without its exception-handling extension (`C3300`): the
  return to it is a WebAssembly exception, and a module built without the extension
  has no way to unwind.
- `#embed` parameters other than the standard's `limit`, `prefix`, `suffix` and
  `if_empty` (`C1084`): C23 leaves other parameters to the implementation, and this one
  defines none, so any other is a parameter it does not know (an error).
- Extensions, under an ISO standard, which they are not part of: they are errors
  there, as a strict mode makes them (the GNU dialects have them, "GNU dialects"): `__attribute__`, `asm`
  and `__asm__`, `__int128`, `__typeof__` (and `typeof` before C23), statement
  expressions, `?:` with an omitted middle operand, case ranges, computed `goto` and labels
  as values, `__builtin_*` functions other than those below ("Reserved identifiers"),
  `#include_next`, `#ident`.
- Under an ISO standard, for the same reason: `#import`, `#assert`, `#unassert`,
  `#if #pred(answer)` and `-A`, and `__declspec` but when the target is Windows (whose
  headers use it: there it is a keyword in every mode, a name reserved to the
  implementation). In a GNU dialect: `__ibm128` (the double-double format of
  a target this compiler does not have).

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
| Keywords | `__extension__` (ignored), `__inline`, `__inline__`, `__const`, `__const__`, `__volatile`, `__volatile__`, `__restrict`, `__restrict__`, `__signed`, `__signed__`, `__alignof`, `__alignof__` (also of an expression: `__alignof__ x`, a member's own alignment), `__typeof`, `__typeof__` and `typeof` in every dialect, `__thread`, `__int128_t` and `__uint128_t` (`__int128` and `unsigned __int128`, as built-in type names), `asm`, `__asm`, `__asm__`; `inline` in gnu89. |
| C99 and C11 in older dialects | From gnu89: `//` comments, `long long`, mixed declarations and code, designated initializers, compound literals, a trailing comma in an enumerator list, variable length arrays, flexible array members, variadic macros and empty macro arguments, hexadecimal floating constants, `_Pragma`, `__func__`, initializers of aggregates that are not constants; from gnu89 and gnu99: anonymous structures and unions; in every dialect: attributes `[[...]]` and `{}`. |
| `__auto_type x = e;` | x has e's type (as C23's `auto`). |
| `_Decimal32`, `_Decimal64`, `_Decimal128` before C23 | The decimal floating types and their constants (`1.5dd`), as in C23 ("Decimal floating types"); `__STDC_IEC_60559_DFP__` is defined in C23 modes only. |
| `__label__ a, b;` | Labels local to the block (a statement expression in a macro can have its own). |
| `({ ... })` | A statement expression: its value is that of its last statement if that is an expression statement (after its labels), else none (void); its objects with cleanups are cleaned up after it is computed. Not outside a function (C3090). `break`, `continue`, `goto` and `return` may leave it. |
| `&&label`, `goto *p` | Labels as values: `&&label` is a `void *`, an address constant (so it may initialize a static object: `static void *t[] = { &&a, &&b };`), equal only to itself; `goto *p` jumps to the label whose address p is. A label's address is an address in a table of its function's (one byte for each label whose address is taken), and `goto *` is a switch on the distance from the table's start; so the values compare and subtract as addresses (`&&a - &&b` of one function's labels is an integer constant), and stay valid when the function is inlined. Where variable length arrays or objects with cleanups are in scope, `goto *` leaves the scopes a `goto` to its label would leave (the arrays freed, the cleanups called, the innermost first); to a label in the scope of one that the `goto *` is not in (a jump into the scope, 6.8.6.1p1, which the label's address makes possible at run time) it traps. |
| `case lo ... hi:` | A case range (inclusive); an empty one (lo > hi) is the warning C4120; overlapping another case is C3041. |
| `a ?: b` | `a ? a : b` with a evaluated once. |
| `__real__ e`, `__imag__ e` | The real and imaginary parts of a complex value (of every complex type), lvalues when e is one (`__imag__ z = 1;`, `&__real__ z`, of e's qualifiers), constant for a constant e; of a real e, e and 0; of an imaginary one, 0 and its coefficient. |
| `[lo ... hi] = v` | A designator range (several in one designation too). v is evaluated once, however many elements the range designates: each expression in it with effects (a call, an assignment, an increment, a volatile read, a statement expression, a compound literal) is evaluated into a temporary before the list's other initializers (6.7.9p23 leaves their order unspecified), even when later designators override every element, and each element gets the temporary's value converted to its type. In an object with static storage duration such a v is not a constant (C3031). |
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
| Nested functions | A function defined in a block: it sees the enclosing functions' objects (in their frames, through a chain of frame records passed to it as a hidden last argument) and may be called through a pointer while its enclosing call runs (a trampoline: one of a pool of 128 compiled thunks per nested function whose address is taken, claimed atomically and released when the enclosing call returns, so no code is written at run time and no stack is executable; more than 128 live at once traps). `auto` declares one ahead of its definition. It sees the enclosing functions' variable length arrays (their addresses stored in the frame record when they are allocated) and variably modified types (typedefs, pointers to variable length arrays, `typeof`, its own parameters' types whose lengths name the enclosing functions' objects: the record holds the addresses of the slots their lengths are kept in). It may be variadic: a direct call passes the chain as a hidden argument after the named ones (so va_start skips it, on every target's variadic convention) to an entry function that starts a `va_list` of the variable arguments and calls the nested function with it, which its `va_start` copies; a trampoline does the same from the platform's C convention. A `goto` in it to a local label (`__label__`) of an enclosing function (it sees no other label of it: C3043) is a non-local goto: the nested function's cleanups are called and its variable length arrays freed, as for a return, its call and those between it and the enclosing function's are left (those of other nested functions of the enclosing one calling their cleanups and giving back their trampolines as the jump passes; those of other functions as a `longjmp` leaves them), and the jump goes on as a `goto` in the enclosing function from the call it is making then, calling the cleanups of the scopes it leaves and freeing their variable length arrays; into the scope of a variable length array or an object with a cleanup that call is not in (6.8.6.1p1), it traps. The enclosing function arms a `jmp_buf` in its frame before each of its calls and is compiled as a function that calls `setjmp` is (not optimized), as is a nested function in it that makes calls with cleanups in scope or trampolines held; on WebAssembly it needs the exception-handling extension (C3300). Every target. |
| `__declspec(...)` | The Windows spelling of attributes, on every target, among declaration specifiers and after `struct`, `union` or `enum`: modifiers separated by spaces (or commas), each the GNU attribute that does what it says: `align(n)` `aligned(n)`, `allocate("s")` and `code_seg("s")` `section("s")`, `deprecated` and `deprecated("text")`, `naked`, `noinline`, `noreturn`, `nothrow`, `restrict` (of a function) `malloc`, `selectany` `weak`; `thread` is the storage-class specifier `_Thread_local` (elsewhere than among declaration specifiers `C2143`); `dllexport`, `dllimport` and `noalias` change nothing (a DLL exports its definitions of default visibility, an object another module defines is imported whatever its declarations say: "Shared libraries", "Objects and libraries"); another modifier is an unknown attribute (C4052). Under an ISO standard `__declspec` is an identifier. |
| `__bf16` | bfloat16 (the brain floating format): 1 sign bit, 8 exponent bits as binary32's, 7 fraction bits; a real floating type of 2 bytes aligned to 2, distinct from `_Float16`, on every target. Conversions to it round once from the source's own value (float, double, long double, integers, the decimal types: never through another format), to nearest with ties to even or, under `FENV_ACCESS`, in the dynamic direction with IEEE 754's exceptions (tininess after rounding); conversions from it are exact. Arithmetic (`+ - * /`, comparisons) is float's, the result rounded to `__bf16` after each operation (as `_Float16`'s: `FLT_EVAL_METHOD` 0; float's 24 bits make that the correctly rounded result); constants are folded the same way. An operation with a `_Float16` operand (real or complex) is `C3231`: neither type's values hold the other's. It has no constants of its own (no suffix). `_Complex __bf16` (and `_Imaginary`) and vectors of it (`vector_size`) are computed as `_Float16`'s are: a complex value's parts in float, rounded to `__bf16` when it is stored, each vector operation's lanes in float and rounded; both are passed and returned as the `_Float16` types are (no other compiler has a calling convention for the complex type). The type-generic macros take it as they take a double. Passed and returned as `_Float16` is (the low 16 bits of an SSE register on x86-64, an `h` register on AArch64, as the platforms' C compilers do); an aggregate with a `__bf16` member is no homogeneous floating-point aggregate on AArch64. `__AXIOM_BF16__` is defined, and `__BFLT16_MANT_DIG__` (8), `__BFLT16_DIG__`, `__BFLT16_DECIMAL_DIG__`, `__BFLT16_MIN_EXP__`, `__BFLT16_MIN_10_EXP__`, `__BFLT16_MAX_EXP__`, `__BFLT16_MAX_10_EXP__`, `__BFLT16_MAX__`, `__BFLT16_NORM_MAX__`, `__BFLT16_MIN__`, `__BFLT16_EPSILON__`, `__BFLT16_DENORM_MIN__` (the floating ones `__bf16` casts of float constants), `__BFLT16_HAS_DENORM__`, `__BFLT16_HAS_INFINITY__`, `__BFLT16_HAS_QUIET_NAN__` (1) and `__BFLT16_IS_IEC_60559__` (0: it is not an IEEE 754 format), with `<float.h>`'s meanings. |
| `__float128`, constants `1.0q`/`1.0Q`, `2.0i`/`2.0j` | `__float128` is `_Float128`; `q` is its constant suffix; `i` or `j` makes an imaginary constant of the type (C17 Annex G's imaginary types, which every mode has: "Floating types"). |
| `enum E : __int128` | An enumeration of a 128-bit underlying type (C23's fixed type), with constants of any value of it; without a fixed type (C23) an enumeration whose values `long long` and `unsigned long long` do not hold has the type `__int128` or `unsigned __int128` ("Implementation-defined behavior"). A `switch` on `__int128` takes case ranges. |
| Vector types | `T __attribute__((vector_size(n)))` (or `mode(V4SI)` ...): n bytes (a power of two times T's size, at most 4096) of lanes of T, an integer or real floating type. The arithmetic, bitwise and shift operators act lane by lane (a scalar operand is converted to the element type, if that keeps its value, and taken for every lane); `==` ... `>=` give the signed integer vector of the same size, -1 where true and 0; `!` is C3020; `v[i]` reads and writes a lane; `?:` with a vector condition selects lane by lane; initializers and compound literals give the lanes in order (missing ones 0; designators are C3072); a cast between vectors, or a vector and an integer, of one size keeps the bytes (another is C3028). `__builtin_shuffle(a, [b,] mask)`, `__builtin_shufflevector(a, b, i...)` and `__builtin_convertvector(v, T)` (below). A vector is aligned to its size, at most 16 bytes on AArch64 and on x86-64 Linux and macOS (32 with AVX, 64 with AVX-512F; on Windows and WebAssembly its size); IR generation uses the target's vector instructions for up to 64 bytes, and lane-by-lane code where there are none (conversions between integer and floating lanes on WebAssembly, a shift by a vector of counts). |

### Preprocessor

`#import "file"` (or `<file>`): `#include` of a file that is then not included again (by
`#import` or `#include`), as if it had `#pragma once`.
`#include_next <file>` (and `__has_include_next`): the next file of the name in the
include path after the directory the current file was found in; `#warning` before C23
(C4103); `#ident "text"` (ignored); assertions: `#assert pred(answer)` gives the
predicate pred the answer (any tokens, compared one by one whatever whitespace is between
them), `#unassert pred(answer)` takes it back and `#unassert pred` every answer, and in an
`#if` expression `#pred(answer)` is 1 if pred has that answer and `#pred` 1 if it has any,
else 0 (neither is macro-expanded); `cpu` and `machine` have the target's architecture
(`x86_64`, `aarch64`, `wasm32`), and on Linux `system` has `linux`, `unix` and `posix`.
A malformed one is `C1087`. Variadic macros may name their variable arguments
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
| `packed` | structures, unions, members, enumerations | Members at byte alignment (but those with an alignment of their own); bit-fields at the next bit, each reached through the smallest unit of 1, 2, 4 or 8 bytes in the structure that holds it, or, when none does (it spans more than 8 bytes, or the structure is smaller than the unit) and for a type wider than 64 bits, through exactly the bytes it spans (read and written in pieces of 8, 4, 2 and 1 bytes from the lowest address; for a volatile one those pieces are its accesses, a store reading them all before writing them), so that no other member's bytes are touched; atomic bit-fields do not exist (C3070); an enumeration's type is the smallest integer type of its values. Members of a packed structure are loaded and stored with the alignment they have. |
| `section("s")` | objects with static storage duration, functions | The object or the function's code goes in section s ("Code in sections": Mach-O's `"segment,section"`, COFF's `s$m`); the linkers define `__start_s` and `__stop_s` for a C identifier's. Not on WebAssembly (accepted with no effect). |
| `used` | objects, functions | Kept though nothing refers to it. |
| `unused` | any declaration, labels | As `[[maybe_unused]]`. |
| `weak` | objects and functions with linkage | A weak definition, which gives way to another unit's that is not weak ("C and Axiom in one program"), or a weak reference (a null pointer if nothing linked defines it: on WebAssembly too, table slot 0 and no import of the host's, a call of it reaching a stub of the linker's that traps; weak data there is at address 0). `#pragma weak` too. |
| `alias("target")` | functions and objects | Another name for target, defined in the same unit (else C3090): a symbol of its own (with `weak`, a weak one) at target's place in every object format, which other objects refer to; the program's other units' references to the name are to target (unless a weak alias's name is defined otherwise, which then wins). An object for other links keeps target for it. |
| `visibility("v")` | objects, functions | default, hidden, protected or internal (another is C3077): recorded in the IR (`ir.Func.visibility`, `ir.Global.visibility`), as `#pragma GCC visibility` gives it to the declarations it covers, and written to objects and shared libraries (ELF: `STV_HIDDEN` for hidden and internal, `STV_PROTECTED`, `STV_DEFAULT`; Mach-O: hidden as a private extern; COFF and Mach-O have no protected symbols: default there). With `--pic` (and `--emit=shared`) on ELF a definition of default visibility is preemptible, its address taken from the GOT and its calls through the PLT; hidden and protected ones are reached directly ("Shared libraries"). No effect on an executable's own code. |
| `ifunc("resolver")` | function declarations of external linkage | An indirect function (ELF's `STT_GNU_IFUNC`; Linux targets only, else C3090, and `__has_attribute(ifunc)` is 0): the symbol of its name is at resolver, a function the unit defines (else C3090), taking no arguments (on AArch64 it may take the hardware capabilities: "Objects and libraries") and returning the function's address, which the program's start-up (a static program's, before anything else runs) or the dynamic linker calls once; the function's calls and its address are its name's, one address wherever it is taken. A definition of it with a body is a redefinition (C3002); it is not also an alias. |
| `noreturn` | functions | As `[[noreturn]]` (C4053). |
| `noinline`, `always_inline` | functions | Given to the optimizer. |
| `cold`, `hot` | functions | Given to the optimizer and the code generator: a block that calls a cold function (and the branch `__builtin_expect` says is unlikely) is laid out apart, as cold code, and a call in it is inlined only when that makes the code smaller; the calls in a hot function, and those of one, are inlined up to twice the usual size. |
| `optimize(n)`, `optimize("On")` | functions | The optimizer's level for the function (0 to 3; `"O"` and `"Og"` are 1, `"Os"` and `"Oz"` 2; other options are ignored, a malformed argument is C3077): its own passes run at that level, and below 2 it is not inlined, nor is anything inlined into it, nor are its parameters given constants. Code generation is the program's. |
| `target("a,b")` | functions | Processor extensions its code may use besides the target's (the names of `--features`; one the target does not have is warning C4052, and is ignored): the code generator selects instructions with them, as for Axiom's `@multiversion` versions, and it is not inlined into code compiled for fewer. The optimizer's vector width stays the target's. |
| `naked` | functions | The function is its asm statements (basic, or extended with no operands: else C3096): their text after a label of its name, with no prologue or epilogue ("Inline assembly"). |
| `const`, `pure` | functions | Given to the optimizer ("Optimization": what a function that promises to write nothing lets it do). |
| `malloc`, `alloc_size(i)`, `alloc_size(i, j)`, `returns_nonnull` | functions returning a pointer | Given to the optimizer ("Optimization"). `malloc` with a deallocator argument is a pairing and says nothing to it. |
| `nothrow`, `leaf`, `artificial`, `flatten`, `noclone`, `noipa`, `externally_visible`, `no_instrument_function`, `alloc_align`, `assume_aligned`, `sentinel`, `may_alias` | | Accepted; no effect (none is needed for correct code). The alignments `alloc_align` and `assume_aligned` promise are recorded in the IR (`ir.Func.align_arg`, `ret_align`); the code generators' accesses do not depend on alignment (unaligned vector loads and stores cost what aligned ones do on the targets), so no pass needs them. |
| `format(archetype, i, j)`, `format_arg(i)` | functions | The calls' format strings are checked (`--c-warn-format`): archetype `printf`, `scanf` or `strftime` (also `gnu_printf`, ...), the format parameter i, the first argument checked j (0: none, as for `vprintf`). The conversions are C23's (with `b`, `B`, `wN`, `wfN`, and the decimal floating types' `H`, `D` and `DD`) and POSIX's `%n$` positions. A literal format is checked against the arguments' types as this target has them: C4130 (an argument the conversion does not take), C4131 (too few), C4133 (not a conversion specification); at level 2 also C4132 (arguments left over), C4134 (a format that is not a literal) and C4135 (another type of the same size and kind, a flag the conversion ignores, `%n`). A call of a `format_arg` function with a literal is checked as that literal. The C library's declarations (`printf`, `scanf`, `strftime`, their kin) carry the attribute in every mode. |
| `nonnull`, `nonnull(i, ...)` | functions | A null pointer constant passed for one of those parameters (all pointers without indexes) is warning C4121; in the function's definition the optimizer takes them for pointers that are not null ("Optimization"). |
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

On x86-64 (Linux, macOS, Windows), AArch64 (Linux, macOS, Windows) and WebAssembly
(wasm32, wasm64) targets. The compiler has its own assemblers (`cmd/axiom/src/asm/`), so
no other tool is needed:

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
  F16C, AVX-512 in full (F, VL, BW, DQ, CD, IFMA, VBMI, VBMI2, VNNI, BITALG,
  VPOPCNTDQ, VP2INTERSECT, BF16, FP16 with its maps 5 and 6, ER, PF, 4FMAPS, 4VNNIW: with
  masking `{%k1}{z}`, broadcast `{1to16}`, embedded rounding `{rn-sae}` and `{sae}`; the
  `ymm`/`zmm`/`k` registers), AVX-VNNI and its INT8/INT16 forms, AVX-IFMA,
  AVX-NE-CONVERT, SHA512, SM3, SM4, GFNI/VAES/VPCLMULQDQ at every length, CMPccXADD,
  FMA4, the `kshift`/`kunpck` mask instructions, AMX (the tile registers `tmm0`-`tmm7`,
  `ldtilecfg`, `sttilecfg`, `tilerelease`, `tilezero`, `tileloadd`, `tileloaddt1` and
  `tilestored` of a base and index address, always with a SIB byte and neither
  rip-relative nor vector-indexed, and the dot products of BF16, INT8, FP16 and
  COMPLEX, whose three tiles must differ), and the system instructions as plain
  encodings (`cli`, `wrmsr`, `mov` to and from control and debug registers, `iretq`,
  `swapgs`, ...). A rounding operand stands right before the first vector register in
  the AT&T syntax (`vcvtusi2ss %eax, {rn-sae}, %xmm0, %xmm1`; elsewhere an error) and
  after the last register in the Intel one (before an immediate), and `SIZE BCST` names
  the broadcast element's size (another size is an error). The instruction tables are
  generated by `cmd/internal/tools/gen_x64_table.py` (with `gen_x64_table_vec.py` and
  `gen_x64_table_more.py`); every encoding of the VEX and EVEX maps that the reference
  disassembler decodes and the reference assembler takes assembles to the same bytes
  (APX's registers aside). Not yet: APX (`r16`-`r31`, REX2, the promoted and NDD/NF
  forms), AMD's XOP encoding, and the legacy-map leftovers (`lar`, `lsl`,
  `lfs`/`lgs`/`lss`, `ud0`/`ud1`, `bnd*`, KeyLocker, ENQCMD, MOVDIRI/MOVDIR64B, RAO-INT,
  the VMX instructions, `getsec`, `rdpmc`, `int1`, branch hints): C3097.
- **AArch64**: the base integer instructions (with their aliases: `mov`, `cmp`, `lsl`,
  `ubfx`, `cset` ...), branches, loads and stores in every addressing form (pairs,
  exclusives, acquire/release, the LSE atomics), `crc32`, pointer authentication and
  `bti`, `svc`, `brk`, hints, barriers, `mrs`/`msr` of the system registers by name (a
  table generated by `cmd/internal/tools/gen_a64_sysregs.py` of the architecture's
  1210 named registers, each named in the directions it may be accessed: `msr` of a
  register only read, an ID register, and `mrs` of one only written are errors, and
  `mrrs`/`msrr` name only the 128-bit ones; and the generic
  `s<op0>_<op1>_c<n>_c<m>_<op2>` form, op0 2 or 3), `msr <pstatefield>, #imm`, scalar floating
  point, and Advanced SIMD in full from a table of instruction forms generated by
  `cmd/internal/tools/gen_a64_table.py`, each with exactly its valid arrangements: vector
  and scalar, integer and floating point in single, double and half precision (three
  same, three different, two-register miscellaneous, across lanes, pairwise, shifts by
  immediate, permutes, `ext`, `tbl`/`tbx`, by element), the dot-product (`sdot`, `udot`,
  `usdot`, `sudot`), matrix-multiply (`smmla` ...), complex-number (`fcmla`, `fcadd`),
  BF16, `fmlal`-class and `pmull` instructions, FAMINMAX's `famax`/`famin`, FP8's
  (`f1cvtl`/`f2cvtl`/`bf1cvtl`/`bf2cvtl` and their `2` forms, `fcvtn`/`fcvtn2` to FP8,
  `fscale`, `fdot` to half and single precision, vector and by element, `fmlalb`,
  `fmlalt`, `fmlallbb`...`fmlalltt`) and LUT's `luti2`/`luti4` (`luti2 v0.16b, {v1.16b},
  v2[3]`); the structure loads and stores
  `ld1`-`ld4`, `st1`-`st4` and `ld1r`-`ld4r` of whole registers and single lanes in
  every addressing form; the modified immediates (`movi`, `mvni`, `orr`, `bic`, `fmov`
  of a vector constant); and the copy instructions (`dup`, `ins`, `umov`, `smov` and
  their `mov` aliases); the cryptographic instructions (AES, SHA1, SHA-256, SHA-512,
  SHA3's `eor3`/`rax1`/`xar`/`bcax`, SM3, SM4). The rest of the base and system set:
  pointer authentication in full (`pacia`...`autdb` and their `z` forms, `pacga`,
  `xpaci`/`xpacd`, the `1716`/`sp`/`z` hints, `retaa`/`retab`, `braa`...`blrabz`,
  `eretaa`/`eretab`, `ldraa`/`ldrab`, and PAuth_LR's `paciasppc`, `pacibsppc`,
  `pacnbiasppc`, `pacnbibsppc`, `pacia171615`, `pacib171615`, `autia171615`,
  `autib171615`, `pacm`, `autiasppc`/`autibsppc`/`retaasppc`/`retabsppc` of a label at
  most 65535 instructions back and their register forms `autiasppcr Xn`/`retaasppcr Xm`
  ..., also written `autiasppc Xn`), the checked pointer arithmetic (`addpt`, `subpt`,
  `maddpt`, `msubpt`), `bc.cond`, the prefetch operations of the system level cache
  (`pldslckeep` ...) and the range prefetch `rprfm`, the unprivileged `ldtr`/`sttr` family, RCpc and
  LSE2 (`ldapur*`/`stlur*` of general and of b-q registers, `ldiapp`/`stilp`, `stlr` and
  `ldapr` with writeback, `ldap1`/`stl1`), `casp`, MTE (`irg`, `gmi`, `addg`, `subg`,
  `subp`, `subps`, `cmpp`, `stg`, `stzg`, `st2g`, `stz2g`, `stgp`, `ldg`, `ldgm`,
  `stgm`, `stzgm`), the memory-copy and memory-set instructions (`cpy[f]{p,m,e}` with
  every option, `set[g]{p,m,e}[t][n]`; the registers checked distinct), the 128-bit
  atomics (`ldclrp`, `ldsetp`, `swpp`), read-check-write (`rcw[s]{cas,clr,set,swp}[p]`),
  CSSC (`abs`, `cnt`, `ctz`, `smax`...`umin` of registers and immediates), the SYS
  aliases by operation name (`at`, `dc`, `ic`, `tlbi` with the range and nXS forms,
  `tlbip`, the stage-2 `tlbi vmallws2e1*`, `cfp`/`dvp`/`cosp`/`cpp`, `brb iall`/`brb inj`;
  a table generated by `cmd/internal/tools/gen_a64_sysops.py`), `sys`, `sysl`, `sysp`, `mrrs`, `msrr`,
  `dsb` with nXS, `wfet`/`wfit`, the flag manipulation instructions (`cfinv`, `rmif`,
  `setf8`/`setf16`, `xaflag`, `axflag`), transactional memory (`tstart`, `tcommit`,
  `tcancel`, `ttest`), the guarded control stack's (`gcspushm`, `gcspopm`, `gcsss1`,
  `gcsss2`, `gcspushx`, `gcspopx`, `gcspopcx`, `gcsstr`, `gcssttr`, `gcsb dsync`), the
  64-byte accesses (`ld64b`, `st64b`, `st64bv`, `st64bv0`), `chkfeat`, `trcit`,
  `clrbhb`, `sb`, `ssbb`, `pssbb`, `psb csync`, `tsb csync`, `hvc`, `smc`, `dcps1`-`3`,
  `smstart`/`smstop`, the fixed-point `scvtf`/`ucvtf`/`fcvtzs`/`fcvtzu` and
  `frint32z`-class instructions, `fjcvtzs`. An encoding the architecture's pseudocode
  makes CONSTRAINED UNPREDICTABLE is an error, not an instruction: a load or store with
  writeback to a register it transfers (`ldr x1, [x1], #8`, `ldp x0, x1, [x1, #16]!`,
  `ldraa x0, [x0, #8]!`), a pair loaded into one register (`ldp x0, x0, [x1]`, `ldnp`,
  `ldxp`), a store exclusive whose status register is its data or base register; and a
  shift amount beyond the operation's width (`lsl #32` of a 32-bit one) is an error. Not
  yet: FEAT_FPRCVT's conversions between floating-point registers and integers of
  another size held in SIMD registers (`fcvtzs s0, d1` ...) and the extensions of the
  architecture's 2024 release after it (FEAT_LSFE, FEAT_LSUI, FEAT_OCCMO, FEAT_CMPBR,
  the FP8 matrix multiplies ...): no reference assembler of the development host
  encodes them, and every encoding of these tables was taken from one. A name that reads as a register where an
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
  `:gottprel:`, `:gottprel_lo12:`, `:gottprel_g1:`, `:gottprel_g0_nc:` (the large model's
  initial exec), `:tlsdesc:` (in `adrp`, and the tiny model's literal `ldr` and `adr`),
  `:tlsdesc_lo12:`, `:tlsdesc_off_g1:`, `:tlsdesc_off_g0_nc:`, `.tlsdesccall`,
  `.tlsdescldr` and `.tlsdescadd` (the large model's descriptor), `:tlsgd:` and
  `:tlsgd_lo12:` (the ELF linker's general-dynamic sequence), `:tlsldm:` and
  `:tlsldm_lo12_nc:` (local dynamic's module: `adrp`/`add` or `adr`, then `bl
  __tls_get_addr`); the
  Mach-O spellings `sym@PAGE`, `@PAGEOFF`, `@GOTPAGE`, `@GOTPAGEOFF` (the same
  relocations) and `@TLVPPAGE`/`@TLVPPAGEOFF` (a thread-local's descriptor, as the
  platform toolchain addresses one: `adrp` and a 64-bit `ldr` of a GOT slot holding its
  address, which the linker makes an `add` of the descriptor itself where the variable is
  the image's own; a variable of `--macos-tls=platform`, a library's or the platform's
  compilers': of the default scheme's, a link error), and
  COFF's `:secrel_lo12:`/`:secrel_hi12:` (a thread-local's offset in
  its block). `adr`, `adrp`, `b`, `bl`, `b.cond`, `cbz`, `cbnz`, `tbz`, `tbnz`, `prfm`
  and literal loads reach symbols of other sections and units (and `adrp`/`adr` labels
  of the template) through relocations. The linkers apply them all, with the ABI's
  range checks as link errors; an executable's own thread-locals are reached by local
  exec, the initial-exec and TLS-descriptor sequences relaxed to it (an initial-exec
  `ldr` whose register is not its base's cannot be, and is an error), and `:dtprel_*:`
  is the offset in the module's block; the large and tiny models' sequences and local
  dynamic's module are relaxed likewise (local dynamic's module is the thread pointer
  plus the TCB's 16 bytes). On Mach-O and PE, whose images the loader puts at an
  address only it knows (above 4 GiB) and whose formats have no relocation of a
  move-wide group, the complete absolute sequence of an address (`movz` of one of
  `:abs_g3:` ... `:abs_g0_nc:` and three `movk` of the others, one symbol to one
  register) becomes `adrp`, `add` and two `nop`s, which give the same value; another
  absolute move-wide group, and `.word`/`.hword` of an address, are C3097 there (a
  difference, `.word a - b`, is not an address). An operator in an instruction it does not fit is an error
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
  stands for `dupm` only where no `dup` immediate gives the value). SME, SME2, SME2.1
  and SVE2.1 in full, from a table generated by `cmd/internal/tools/gen_a64_table.py`
  (SMETAB) with exactly the operands each encoding has: the ZA array and its operands
  (`za`, a tile `za3.s`, its slices `za0h.b[w12, 0]` and `za1v.s[w13, 0:1]`, the
  array's vectors `za.d[w8, 0, vgx2]`, `za.s[w9, 0:3]`, `za[w12, 0]`), the
  predicate-as-counter registers `pn0`-`pn15` (`pn8.b`, `pn8/z`, `pn8[1]`), the lookup
  table `zt0` (`zt0[8]`, `{zt0}`), lists of 2 or 4 consecutive registers (`{z0.s-z3.s}`,
  `{z0.s, z1.s}`, wrapping at z31) and SME2's strided ones (`{z0.b, z8.b}`, `{z0.h,
  z4.h, z8.h, z12.h}`), predicate pairs (`{p0.b, p1.b}`), and the elements `z1[2]` and
  `p2.b[w12, 0]`; the instructions: `mova` and its `mov` aliases to and from tiles,
  slices and array vectors (one, two or four), `movaz`, the tile loads and stores
  (`ld1b`...`ld1q`, `st1b`...`st1q` of `{za0h.s[w12, 1]}`), `ldr`/`str` of ZA vectors
  and of `zt0`, `zero` of a list of tiles (`{za0.d, za2.d}`, `{za1.s}`, `{za}`), of
  array vectors (`za.d[w8, 0:1, vgx2]`) and of `zt0`, the outer products (`fmopa`,
  `fmops`, `bfmopa`, `bfmops`, `smopa`, `umopa`, `sumopa`, `usmopa`, `bmopa` and their
  `...ops`, of every element size, the widening and non-widening ones), `addha`,
  `addva`, `smstart`/`smstop`, `rdsvl`, `addsvl`, `addspl`; SME2's multi-vector forms:
  the ZA array's `add`, `sub`, `fadd`, `fsub`, the dot products (`sdot`, `udot`,
  `usdot`, `sudot`, `fdot`, `bfdot`, `svdot`, `uvdot`, `suvdot`, `usvdot`, `fvdot`,
  `bfvdot`) and multiply-adds (`fmla`, `fmls`, `bfmla`, `bfmls`, `fmlal`, `fmlsl`,
  `bfmlal`, `bfmlsl`, `smlal`, `umlal`, `smlsl`, `umlsl`, `smlall`, `umlall`, `smlsll`,
  `umlsll`, `sumlall`, `usmlall`) of one, two or four vectors by a vector, a list or an
  element, the multi-vector arithmetic (`add`, `smax`, `umin`, `fmax`, `fminnm`,
  `srshl`, `sqdmulh` ... of a list by a vector or a list), the clamps, `sel`, `zip`,
  `uzp`, `sunpk`, `uunpk`, the conversions (`fcvt`, `fcvtn`, `bfcvt`, `bfcvtn`,
  `fcvtzs`, `scvtf` ..., `frinta`...), the narrowing `sqcvt`/`sqrshr` families,
  `luti2`/`luti4` (one, two or four registers, consecutive or strided), `movt`, the
  multi-vector loads and stores (`ld1b`...`ld1d`, `ldnt1b`...`ldnt1d`, `st1b`...,
  `stnt1b`... of two or four registers with a `pn` governing predicate, every
  addressing form); SVE2.1's `psel`, `pext`, `ptrue pn8.b`, `cntp` of a `pn`,
  the register-pair and predicate-as-counter `while*`, the two-way `sdot`/`udot`/`fdot
  z.s, z.h` (vector and by element), `bfmlslb`/`bfmlslt`, `sqrshrn`/`uqrshrn`/
  `sqrshrun`/`sqcvtn`... of pairs, `revd`, the quadword forms (`dupq`, `extq`, `tblq`,
  `tbxq`, `zipq1`..`uzpq2`, the `...qv` reductions to a vector register), `pmov`, the
  gathers and scatters `ld1q`/`st1q`, `ld2q`-`ld4q`, `st2q`-`st4q`, `ld1w`/`ld1d`
  of `.q` elements; and B16B16's `bfadd`, `bfsub`, `bfmul`, `bfmla`, `bfmls`, `bfmax`
  ..., `bfclamp` (SVE and SME forms). Each needs its extension: `--features=` (or the
  `--cpu`) with `sme`, `sme2`, `sme2p1`, `sve2p1`, `sme-f64f64`, `sme-i16i64`,
  `sme-f16f16` or `b16b16` (C3097 names the one missing), or the directive
  `.arch_extension name{, name}` in the assembly, which allows them from there to the
  end of the template or file (`noname` takes one away), and adds nothing to what the
  program needs at run time (a program built with `--features` checks for them when it
  starts). Where the conventional assemblers disagree, or take more than the
  architecture writes, this one follows the architecture: a list `{z0.s-z1.d}` whose
  ends differ in element size, and `x31` for `xzr`, are errors. The SVE and SVE2 forms
  need no extension named.
- **WebAssembly**: the text format of the WebAssembly specification (Core Specification
  2.0, chapter 6): plain instructions and folded ones (`(i32.add (local.get %1) (i32.const
  1))`, `(if (result i32) cond (then ...) (else ...))`), `block`, `loop`, `if`/`else`,
  `end` with symbolic labels (`block $out ... br $out`, `end $out`) or depths, `br`,
  `br_if`, `br_table`, block types `(result t)` and with the multivalue extension any
  `(param ...) (result ...)`, the integer literals (decimal, `0x`, `_` between digits,
  signed or not), the floating ones (decimal, hexadecimal `0x1.8p3` rounded once to the
  type, `inf`, `nan`, `nan:0x...`), memargs `offset=N align=N` (the alignment at most the
  access's size, an atomic access's its size), lane indices, `v128.const` of every shape,
  `i8x16.shuffle`, `;;` and `(; ;)` comments. Instructions: the numeric, conversion,
  memory, `memory.size`/`memory.grow`, `select` (and `select (result t)`), `drop`,
  `nop`, `unreachable`, `call`, `call_indirect (param ...) (result ...)` or `(type $t)`
  (of the program's function table; the index is an `i32`, also on wasm64), `local.get`/
  `set`/`tee`, `global.get`/`global.set` of `$__stack_pointer` and of file-scope asm's
  globals (`global.set` of an immutable one is C3097); and of the extensions `--features`
  enables (another's instruction is C3097, naming the flag): sign-ext, nontrapping-fptoint,
  mutable-globals (a mutable global imported from the host or exported), bulk-memory
  (`memory.copy`, `memory.fill`, `memory.init` and `data.drop` of a passive data segment,
  `table.init` and `elem.drop` of an element segment, `table.copy`), simd128 (all of it),
  relaxed-simd, atomics (`memory.atomic.notify`, `wait32`/`wait64`, `atomic.fence`, the
  atomic loads, stores and read-modify-writes), multivalue (block types and functions of
  several results), tail-call (`return_call`, `return_call_indirect`) and
  exception-handling (`throw $tag`, `try_table` with `catch $tag l`, `catch_ref $tag l`,
  `catch_all` and `catch_all_ref`, whose labels must take the tag's parameters (and the
  `exnref`), `throw_ref`, the `exnref` type, `ref.null exn` and `ref.is_null`; and the
  extension's legacy form, which engines still take: `try bt ... catch $tag ... catch_all
  ... end`, `try bt ... delegate l`, `rethrow l` of a catch, flat and folded as `(try bt
  (do ...) (catch $tag ...) (catch_all ...))` and `(try bt (do ...) (delegate l))`). So
  every instruction that the ten extensions of `--features` can enable assembles and is
  validated. A table of them is generated from the specification's opcode tables by
  `cmd/internal/tools/gen_wasm_table.py`. Not, by rule: the reference types' instructions
  (`table.get`, `table.set`, `table.size`, `table.grow`, `table.fill`, `ref.func`, `ref.null
  func` and `funcref` values), an extension that `--features` has no flag for. Every
  template is validated as the specification's validation algorithm says, its operands'
  types known: an operand of the wrong type, a stack left with values, a block not
  ended, a label that is not there are C3097, so that the module is always valid. The
  names a text uses (`$name`) are the program's: `call $f` calls C's (or file-scope
  asm's) function f, with its signature as the C ABI gives it (a variadic function's
  arguments buffer last), and `i32.const $x` (`i64.const` on wasm64) is the address of
  object x or the table index of function f (C's pointer to it); a name not declared
  where the asm is is C3097. In an asm statement:
  - the operands in locals: `r` (and `g`, `X`) is a local of the operand's type (`i32`
    for integers of up to 32 bits, extended as their type's signedness says, and for
    pointers on wasm32; `i64` for 64-bit ones and pointers on wasm64; `f32`, `f64`; a
    16-byte vector a `v128`); `m` (`o`, `V`) a local holding the object's address (the
    template loads and stores through it: `local.get %0 i32.load`); outputs are locals
    the template sets (`=`; `+` is set from the value first), an input's local may not be
    written (C3097), and one tied to an output (`"0"`) is the output's local (of the same
    type: C3097 otherwise). `%0` and `%[name]` write the local where an instruction
    takes one (`local.get %0`);
  - constants: `i` and `n` are a number (`i32.const %1`, `offset=%1`, a lane), and `i`
    and `s` of an address are that symbol (`i32.const %1` gives it; `call %1` of a
    function calls it, with the signature of its C type);
  - the clobbers `"memory"` and `"cc"` (which means nothing here); WebAssembly has no
    registers (another clobber, a machine constraint of another target and an
    explicit register variable are errors); the modifier `%c` (a constant as itself);
  - the template is a block of no parameters or results (`br 0` at its top leaves it);
    it must leave the stack empty (its results are its outputs' locals), and cannot
    `return` or `return_call` (the function's epilogue would not run); `asm goto`'s
    `%l0`, `%l[name]` are labels around the template, `br %l[name]` (or `br_if`) goes to
    that C label;
  - a template that calls a function (`call`, `call_indirect`) is as one that calls on
    the other targets (its function makes no sibling calls).
  File-scope asm is module fields (asm statements see the names they declare, which are
  the unit's: each kind its own namespace, by name only, since the index spaces are the
  program's):
  - `(func $name (export "n")? (type $t)? (param $p? t)* (result t)* (local $l? t)*
    instructions)`, a function of the program named `name` (C declares it to call it;
    `(export "n")` exports it from the module too), whose body may also `return`;
  - `(import "module" "name" (func|global|tag $id ...))` (as in an assembly source:
    above): module `env` names the program's function, global or tag of that name
    (another unit's; a global `__stack_pointer` is the stack pointer), another module the
    host's, imported by the module;
  - `(global $g (export "n")? t|(mut t) init)`, a global of the program named `g`, whose
    initial value is one constant of its type (`i32.const` ... `v128.const`, `ref.null
    exn` of an `exnref` one) or `global.get` of an immutable imported global (the linker
    takes an `env` one's constant value: one defined by an immutable global with a
    constant); another unit imports it from `env` with its type and mutability, else a
    link error;
  - `(tag $e (export "n")? (type $t)? (param t*))`, a tag of the program named `e`;
  - `(data $d "bytes"*)`, a passive data segment (`memory.init`, `data.drop`), and `(elem
    $e declare? func $f*)`, a passive or declarative element segment, whose functions get
    slots of the function table as pointers to them would (`table.init` writes the
    program's table, which has as many slots as the program's function pointers and does
    not grow). An active segment, `(data (i32.const N) ...)` or `(elem (i32.const N)
    ...)`, is C3097 by rule: the memory's and the table's layouts are the program's (the
    stack, the objects and the functions' slots the linker places); objects are defined in
    C;
  - `(export "n" (func|global|tag $x))` of the text's own definitions (the linker exports
    the memory as `memory`), `(start $f)` (of type `[] -> []`, one in a unit: the module's
    start function calls it when the module is instantiated, before the program's entry:
    in a program with threads, in each thread's instance too), `(type $t (func ...))`;
  - `(memory i32|i64? n m? shared?)` and `(table $t? n m? funcref)` only declare the
    program's memory (of the target's index type) and function table again (`$t` then
    names table 0); another is C3097.
  The objects of such units have what the tool conventions give (tags, globals, exports
  of each kind, the start section, passive segments) and, for `memory.init`/`data.drop`
  and `table.init`/`elem.drop`, relocations of our own linker's (the conventions have
  none). A function of file-scope asm is never stripped, and one that C or Axiom code
  defines too is an error. A naked function
  (`__attribute__((naked))`) has its asm statements, without operands, as its code: its
  parameters are the locals 0 up, and it returns what it leaves on the stack.
- Directives: `.byte`, `.short`/`.hword`/`.2byte`, `.word` (2 bytes on x86-64, 4 on
  AArch64), `.long`/`.int`, `.quad`/`.xword`, `.octa`, `.ascii`, `.asciz`/`.string`,
  `.float`, `.double`, `.uleb128`, `.sleb128`, `.zero`/`.skip`/`.space` (a size and
  a fill), `.fill` (a count, a size up to 8 and a value: units of the value's low 4
  bytes, then zeros; an empty size is an error), `.balign`, `.p2align`, `.align` (an
  alignment, a fill, a maximum; `.align n` is `2^n` bytes on AArch64 Linux and in
  Mach-O, `n` bytes on x86-64 Linux and in COFF, as the platforms' assemblers have it),
  `.balignw`/`.p2alignw` and `.balignl`/`.p2alignl` (a fill of 2 and 4 bytes, the
  padding whole units of it; code padding uses the multi-byte NOPs), `.org` (in
  file-scope asm and assembly sources: on to a constant offset of the section, or to a
  label of it plus one, with a fill; in file-scope asm the offset is from where the
  assembly's part of the section starts; backward is an error, and so is `.org` in a
  function's template, whose place the compiler chooses), `.set`/`.equ`,
  `.equiv` and `=`, `.inst` (AArch64); the assembler language `.macro`/`.endm` (arguments,
  defaults, `:req`, `:vararg`, `\@`), `.rept`, `.irp`, `.irpc`, `.if`/`.ifdef`/`.ifc`/
  `.else`/`.elseif`/`.endif`; in file-scope asm also `.text`, `.data`, `.bss`,
  `.section` in the object format's syntax (ELF: `name, "flags", @type` with the flags
  `a w x M S G T o R`, the types `@progbits @nobits @note @init_array @fini_array
  @preinit_array` or `%type`, `M`'s entry size (the entries are not merged), `G`'s
  group and `comdat` (a COMDAT group, whose signature is a local symbol at the section's
  start when the assembly defines none), `o`'s section (`SHF_LINK_ORDER`), `R`
  (`SHF_GNU_RETAIN`); Mach-O: `segment,section[,type[,attributes]]` with the types
  `regular`, `zerofill`, `cstring_literals`, the literals' (written as regular ones),
  `mod_init_funcs` and `mod_term_funcs` (named `__mod_init_func` and `__mod_term_func`,
  joined to the C units' initializers and terminators), and the
  attributes `pure_instructions`, `no_dead_strip` (`S_ATTR_NO_DEAD_STRIP`), ..., or an
  ELF name (`.rodata`, ..., or a name with ELF's flags, placed as the C units' sections
  are); COFF: `name, "flags"` of `b d n r s w x y i D` (`d` without `r` writable) and
  ELF's `a`, ELF's `@progbits` or `@nobits`, and the COMDAT selection `discard` with
  its symbol, which must be defined in the section (sections keep their names, but
  ELF's `.text*` goes in `.text`, `.rodata*` in `.rdata`, `.data*` in `.data`, `.bss*`
  in `.bss`, and a C identifier's section is `name$m`, as the C units' are).
  Errors: another flag, type or
  selection, a section both writable and executable, an initialization array not
  named for one, `,unique,N`, a thread-local Mach-O section),
  `.pushsection`, `.popsection`, `.previous`,
  `.subsection`, `.globl`, `.weak`, `.local`, `.hidden`, `.protected`, `.internal`
  (visibility: ELF's `STV_HIDDEN` for `.hidden` and `.internal`, `STV_PROTECTED`;
  Mach-O's private extern for the hidden ones, Mach-O having no protected visibility,
  its image's references to its own definitions binding within it; nothing in COFF,
  where only a DLL's exports leave the image), `.type`, `.size`,
  `.comm`, `.lcomm`, `.incbin` (within the include directories), COFF's `.rva` and
  (x64) `.secrel32`. Labels: `name:` and numeric local labels (`1:`, `1b`, `1f`).
  Relocations: x86-64 rel32 calls and jumps to symbols (`@PLT`), `sym(%rip)`
  (`@GOTPCREL`), on macOS `sym@TLVP(%rip)` in a `movq` (a thread-local's descriptor,
  which the linker makes a `leaq` for the image's own), absolute 32- and 64-bit
  addresses; AArch64's operators above,
  `.quad` and `.word` of addresses; in 4-byte data `sym@GOTPCREL` (the offset of the
  symbol's GOT slot from the place, plus a constant; COFF's `.refptr` pointer;
  ELF, Mach-O and x64 COFF), `sym@PLT - .` (x86-64 ELF), `sym@IMGREL` (COFF). Other
  modifiers in data (`@TPOFF`, `@DTPOFF`, `@GOTOFF`, `@SIZE`, ...) are errors, as is
  `sym@PLT` without `- .`. `.ident`, `.arch` and the like are accepted and do
  nothing; `.file` and `.loc` give the line information under `-g` ("Assembly sources"). An instruction, operand form or directive that is not
  supported is C3097, naming it, at the statement.
- Call frame information: in file-scope asm `.cfi_startproc [simple]` ... `.cfi_endproc`
  make a procedure whose `.cfi_def_cfa`, `.cfi_def_cfa_register`, `.cfi_def_cfa_offset`,
  `.cfi_adjust_cfa_offset`, `.cfi_offset`, `.cfi_rel_offset`, `.cfi_val_offset`,
  `.cfi_restore`, `.cfi_undefined`, `.cfi_same_value`, `.cfi_register`,
  `.cfi_remember_state`, `.cfi_restore_state`, `.cfi_escape` and on AArch64
  `.cfi_negate_ra_state` (`.cfi_window_save`) and `.cfi_negate_ra_state_with_pc` are its
  FDE's program (registers by name or DWARF number); `.cfi_signal_frame` gives it a CIE
  with the augmentation `S`, and `.cfi_label name` defines a label at its place. What
  each object format gets:
  - ELF (Linux): an FDE in `.eh_frame` always (unless `--unwind-tables=off`) and in
    `.debug_frame` under `-g`; `.cfi_sections .eh_frame, .debug_frame` (either or both)
    chooses for the procedures that follow. `.cfi_personality enc, sym[+n]` and
    `.cfi_lsda enc, sym[+n]` give the procedure a CIE with the augmentations `P` (the
    personality routine's pointer) and `L`, and its FDE the LSDA's pointer, in the
    encoding given: absolute or pc-relative (`DW_EH_PE_absptr`, `udata4`, `sdata4`,
    `udata8`, `sdata8`, with `pcrel`), optionally `indirect` (0x80: the address of a word
    that holds the pointer, as compilers' `DW.ref.` words are), relocated as the
    platform's assemblers do; 255 is none. `.cfi_return_column reg` (version 3 when it is
    above 255) and on AArch64 `.cfi_b_key_frame` (`B`) and `.cfi_mte_tagged_frame` (`G`)
    are the CIE's too. The platform's unwinder calls such a routine with its LSDA and
    lands in the handler it chooses (`c/link/asm_personality`).
  - Mach-O (macOS): the procedure's entry in `__compact_unwind` and its FDE in
    `__eh_frame` (`.cfi_sections` without `.eh_frame` leaves them out), and under `-g`
    in the debug bundle's `.debug_frame`; a personality
    routine, an LSDA, a return address column and `B`/`G` are C3097 there (the assembler
    writes a personality and an LSDA only into ELF's `.eh_frame`; the linker takes those
    of the platform's compilers' objects: "Objects and libraries"), as on Windows.
  - PE (Windows): `.cfi_*` are checked and put nowhere (the debug information is the
    PDB's, which takes no call frame information from them): the unwind information is
    `.seh_*`'s (below).
  Not written, C3097: the encodings relative to data, text or the function, 2-byte
  pointers, `.cfi_val_encoded_addr` and `.cfi_inline_lsda` (no compiler of these targets
  writes them) and directives of other processors.
  On Windows, `.seh_proc` ... `.seh_endproc` make the procedure's unwind
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
  takes), `k` (the stack pointer: an input whose operand is the register variable
  declared `asm("sp")`, written `sp`; the compiler moves nothing into it), `I J K L M
  N Z`, `Y` and `Dz` (the constant zero, also the floating `0.0`,
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
  and frame pointers, and on AArch64 x16, x17 and x18, cannot be, but AArch64's `sp`,
  which is the operand of the constraint `k` only (its value there is the stack pointer;
  it is never written). On WebAssembly, which has no registers, there are none.
- Clobbers: registers (with or without `%` on x86-64; `ymm0-15` and `zmm0-15` are their
  xmm register; `xmm16-31`, `ymm16-31`, `zmm16-31`, `k0-7`, `mm0-7` and AMX's `tmm0-7`, which the compiler
  keeps no value in across an asm statement, are accepted; `st` and `st(n)` say which x87
  inputs the template pops), `"cc"` (always assumed on x86-64), `"memory"`; not the stack or frame pointer, nor on macOS and Windows for AArch64 x18,
  the platform's (C3096); on AArch64 also `za` (the ZA array) and `zt0` (the ZT0
  table), the SME state, which the compiler keeps nothing in across an asm, and
  `pn0`-`pn15` (the names `p0`-`p15` have as predicate-as-counter registers: the
  same as those). A clobbered callee-saved register is saved by the function. The
  compiler's code runs in non-streaming mode and never uses ZA: an asm that enters
  streaming mode (`smstart` or `smstart sm`) must leave it before it ends, and since
  entering and leaving it set every `z` and `p` register and `ffr` to zero, it must
  list as clobbers the vector registers whose values it does not restore (`v0`-`v31`
  or `z0`-`z31`; the callee-saved `d8`-`d15` are then saved by the function) and
  `p`/`ffr` as for SVE; the SVE instructions it runs in streaming mode act on the
  streaming vector length (`rdsvl`), and an instruction that streaming mode does not
  allow faults there. `za` and `zt0` must be clobbered by an asm that changes them or
  turns ZA on or off (`smstart za`, which zeroes them, or `smstop za`).
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
| `__builtin_object_size(p, t)`, `__builtin_dynamic_object_size` | The bytes from p to the end of the object (t 0, 2) or of the closest member (t 1, 3; a member array at the end of its structure counts to the object's end), when the expression shows the object: a declared object of a complete non-VLA type, a string literal or a compound literal, its address, its array decayed, an element at a constant index, a member, or such a pointer converted or offset by a constant (0 past the end). Otherwise, when the argument has no side effects, the optimizer looks for the object after it has propagated the values (inlined, folded): a stack object, a global no other unit's definition can replace, a variable length array, the result of a call of a function with `alloc_size` (the argument, or the product of the two), with p at a constant offset from its start; for t 3 (the closest member's lower bound) it does not. A size known only when the program runs (a variable length array's, `alloc_size` of arguments that are not constants) is the answer of `__builtin_dynamic_object_size` only, computed there. Else unknown: `(size_t)-1` for t 0 and 1, 0 for 2 and 3 (an object whose type has a flexible array member is unknown; so is everything at `-O0` but what the expression shows and the variable length arrays). |
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
| `STDC FP_CONTRACT ON/OFF/DEFAULT` | Honoured: no floating operations are contracted (an FMA is only the `fma` the program calls), which is what OFF asks and what ON allows. |
| `STDC FENV_ACCESS ON/OFF/DEFAULT` | Honoured: see "The floating-point environment". |
| `STDC CX_LIMITED_RANGE ON/OFF/DEFAULT` | Accepted (it allows, and asks for nothing). |
| `STDC FENV_ROUND dir` (C23) | Honoured, at translation and at run time ("The floating-point environment"); `FE_TONEARESTFROMZERO`, and a directed rounding on WebAssembly, are C4113. |
| `STDC FENV_DEC_ROUND dir` (C23) | Honoured, every direction, on every target. |
| `GCC diagnostic push`, `pop`, `ignored "-Wx"`, `warning "-Wx"`, `error "-Wx"` | The warnings after it (in its file, and in the files it includes) that -Wx names are not shown, shown, or errors (keeping their codes). -Wx is one of the compiler's codes (`"C4050"`), `-Wall`, `-Wextra` and `-Wpedantic` (every warning), or a usual name that stands for one: `-Wdeprecated-declarations` and `-Wdeprecated` (C4050), `-Wunused-result` and `-Wunused-value` (C4051), `-Wattributes` (C4052), `-Winvalid-noreturn` (C4053), `-Wimplicit-function-declaration` (C4001), `-Wunused-label` (C4040), `-Wvarargs` (C4080), `-Wmultichar` (C4102), `-Wcpp` (C4103), `-Wunknown-pragmas` (C4111), `-Wpragmas` (C4112), `-Wnonnull` (C4121), `-Wswitch-outside-range` (C4120), `-Wattribute-warning` (C4123), `-Wformat` (C4130 to C4135), `-Wformat-extra-args` (C4132), `-Wformat-nonliteral` and `-Wformat-security` (C4134), `-Wformat-signedness` (C4135); others are ignored. An error is never made a warning (nor a check `--c-warn-format` does not make enabled). |
| `GCC visibility push(v)`, `pop` | The declarations with external linkage up to the matching `pop` get visibility v (as the attribute, which overrides it). |
| `GCC poison x y` | A later use of x or y is the error C1086 (not one a macro defined before makes). |
| `GCC system_header` | The rest of the file gives no warnings (in the main file it means nothing). |
| `GCC warning "m"`, `GCC error "m"` | The warning C4103, or the error C1040. |
| `GCC optimize (options)`, `GCC target (extensions)` | The functions defined after it get them, as the attributes `optimize` and `target` give them ("Attributes"), added to those in force: options are strings (comma-separated lists) or numbers, in parentheses or not; of `optimize` the levels count (the last one), others are ignored; an extension the target does not have is C4112 and is ignored. A function's own attribute `optimize` wins over the pragma's level; its `target` adds to the pragma's extensions. |
| `GCC push_options`, `pop_options`, `reset_options` | The options in force saved, given back (a `pop_options` without a `push_options` is C4112), or made none. |
| `GCC unroll n` | Before a `for`, `while` or `do` statement: 0 and 1 keep the loop from being unrolled; n > 1 unrolls it by n (fully if it runs at most n times), as far as a size of 4096 instructions and a factor of 64 allow; a loop whose trip count is not known is unrolled by two, whatever its size. n is a constant of at most 65534 (macros replaced). |
| `GCC ivdep` | Before a loop: the program says no dependence between its iterations keeps them from running at once, and a vector loop made of it has no check for overlapping stores and loads. |
| `GCC novector` | Before a loop: it is not vectorized (it may still be unrolled). |
| `GCC dependency "file" [text]` | The warning C4115 (with the text) if the file, looked for as `#include "file"` is, was modified after the current one; one not found is C4112. |

`--emit=c-pp` writes the pragmas the later phases read, where they change what they say: `pack(n)` (the value then in force), `GCC visibility`, the functions' options (as `GCC reset_options` and the `GCC optimize` and `target` that give those in force), the loop pragmas before their loops, the floating-point ones, and the `weak` ones at the end; the others have acted already.

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
| `__builtin_axiom_simd("op", ...)`, `__builtin_axiom_shufflevector`, `__builtin_axiom_convertvector` | The intrinsics' operations ("Intrinsics"): an operation of the compiler's table, and the dialects' `__builtin_shufflevector` and `__builtin_convertvector`, in every mode. |
| `__AXIOM_...`, `__axiom_...` | Macros and members of the headers, and the intrinsics headers' own vector types (`__axiom_i32x4` ...) and functions. |
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
stores back what it read, which makes the read single-copy atomic; with LSE,
`--features=+lse` or a processor that has it, a compare-exchange is one `CASP`) and on x86-64 (`lock cmpxchg16b`): inline when the target's extensions have it
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
directory, so every thread has them. On macOS there are two schemes, chosen with
`--macos-tls=fast|platform`:

- `fast`, the default (and what Axiom programs have run with on Macs): the runtime's thread-local block, an
  offset in a block whose address is in the TSD slot of the runtime's pthread key (arm64:
  `mrs` of `TPIDRRO_EL0`; x86-64: `gs`), about five instructions an access. The main
  thread and the runtime's threads have it: every thread of `thrd_create` is one of the
  runtime's (a thread something else creates, such as the system's C code with
  `pthread_create`, has none).
- `platform`: the platform's thread-local variables (descriptors in `__thread_vars`,
  whose thunk, the platform's dynamic linker's, makes a thread's block when the thread first uses one: every
  thread has them, those of `pthread_create` too, and they interoperate with the
  platform's objects and libraries both ways). Code finds one by calling the thunk
  (arm64: `adrp`/`ldr` of the descriptor's address, which the linker makes an `add` for
  the image's own, and `blr` through its first word; x86-64: `movq v@TLVP(%rip), %rdi`
  made a `leaq`, and `call *(%rdi)`). **It has never been executed**: it is built and
  checked here against the platform's object formats, linker and debug information,
  and nothing here runs macOS. It assumes that the thunk changes no register but x0, x16,
  x17, the link register and the flags on arm64 (x9 is saved too), and none but rax, rdi
  and the flags on x86-64, as the platform's compilers' code does; the code saves what
  else could change around the call, 160 (x86-64: 136 and 24) bytes below the stack
  pointer. An access costs about 14 instructions and the thunk's call (the thunk's fast
  path is a few more), against `fast`'s five. A function without a frame of its own that
  reaches a thread-local cannot be unwound past while the thunk runs (a profiler's
  sample there). The default may become `platform` once it has run on a Mac.

`--emit=obj`, `--emit=lib` and `--emit=shared` for macOS imply `platform` (their output
is the platform toolchain's to link, or a library's, whose thread-locals the runtime
cannot make). A link may have objects of both schemes, each variable reached by its own
scheme's code; code of one scheme reaching a variable of the other (a `fast` program's
code using an object's or a library's thread-local, a descriptor reference to a `fast`
variable) is a link error that says to build with `--macos-tls=platform`, as is an
undefined thread-local reached by its descriptor that no library given to the link can
define. On WebAssembly, in a program
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
against the platforms' C compilers. The suite runs a subset (`c/run/abi_gen_*.c`).
Windows x64 programs are run under a Windows compatibility layer where one is installed
(the suite's cross-target runs and its `c/objfmt` tests; otherwise they are built and
checked only); macOS output and Windows ARM64 output are built and checked, never run.

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
(crash.c:2)`), from a line table per function that the frame table then refers to
(`cmd/axiom/src/ftab.ax`: no DWARF is read at run time), also where no call was
recorded (all of a program inlined into `main`); without them the table and the code
are as before. The line tables are read-only data without relocations, two or three
bytes a row and a record of 8 bytes per line and function (with the records of the
calls it was inlined at, shared); the compiler's own executable grows by about 7 MB
with them (its DWARF is about 16 MB more), its writable data not at all.

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
is one at all. On WebAssembly C's calls write their call-site records on the shadow
stack as Axiom's do (three stores and a load per call; `--stack-traces=off` leaves
them out), and the optimizer's inlining keeps them: a trap (such as a division by
zero) shows the C frames at every optimization level. `setjmp` keeps the shadow
stack's depth in the `jmp_buf` and the code after a `longjmp` puts it back, so a later
trace shows none of the frames the jump left.

## Debugging

`-g` (or `--debug-info=full`) writes DWARF 5 debug information, `--debug-info=lines`
the line tables and frames only, and `--debug-info=none` (the default) nothing. It does
not change the instructions generated, at any optimization level, and two builds are
the same. With line tables the frame table also refers to the line tables of crash
traces ("Stack traces"), which enlarge the program's loaded read-only data: an
object's code and its relocations are the same as without `-g` (the suite compares
them), but in an executable what the tables push further on is at other addresses,
and so are the addresses the instructions that refer to it hold (the code's size and
its other bytes do not change; the writable data is the same). An executable is
therefore not the same as without `-g` (on macOS either, where the debug information
itself is in the `.dSYM`).

- **Lines**: each function's code is mapped to the lines of the source, `#line`
  directives followed (the presumed file and line, as diagnostics show them); code the
  optimizer inlined or moved keeps the line it came from. Each file of a line table has
  the MD5 of its contents, unless one of the unit's files is a name `#line` gave
  (then none has); the predefined macros' `<built-in>` is the same in every build
  (`__DATE__` and `__TIME__` are not defined in its text: the preprocessor replaces
  them). The prologue ends, for a
  breakpoint on a function, past the stores of its parameters. Columns are not
  recorded.
- **Frames**: call frame information for every instruction of every function
  (prologues, epilogues, sibling calls, code run before the frame is set up), so that
  a debugger can unwind the stack wherever it stops (in `.debug_frame`; the same rules
  are in `.eh_frame` with or without `-g`: "Objects and libraries").
- **Types and variables** (`full`): the program's types (base types, `_BitInt` with its
  width in bits, pointers, qualifiers, arrays, structures and unions with their
  bit-fields, enumerations and their constants with their exact values, also those a
  signed 64-bit integer does not hold: an unsigned one in `DW_FORM_udata`, a wider one
  in `DW_FORM_block1`, as in memory (not `DW_FORM_data16`, which makes a debugger
  that does not read that form in `.debug_info` drop the whole unit); function types, typedefs); each
  function's result type, parameters and local variables; objects of static storage
  duration, thread-local ones included (by their offset in the thread's TLS block; on
  macOS through a descriptor of the platform's form for `DW_OP_form_tls_address`, which
  the platform's debugger reads: with `--macos-tls=fast`, where the thread's block is the
  value of a pthread key, one of three words, 0, the key, which the runtime writes at
  start-up, and the offset; with `platform` the variable's own descriptor in
  `__thread_vars`, as the platform's compilers write it: neither tried in that debugger,
  which needs a Mac). A variable, parameter, typedef or object declared with a typedef name
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
    where nothing holds a parameter's value any more, it is its entry value
    (`DW_OP_entry_value` of its argument register), which a debugger finds from the
    caller's call site (below);
  - a value nothing holds any more, made by an integer operation (addition,
    subtraction, the bitwise operations, shifts, extensions and truncations, a
    pointer plus an offset) from values that are held, is computed from them
    (`DW_OP_bregN`, `DW_OP_fbreg`, `DW_OP_entry_value` and the stack machine's
    operations, `DW_OP_stack_value`), as long as none of them was made again since;
    so is a value not made yet in its block whose operands are those it will be made
    from, and one no instruction makes (folded into the instructions of those
    computed from it, or a copy that took its operand's register) from the
    assignment on; through up to eight operations, a multiplication of 64 bits
    too (from its operands' 32-bit halves); not divisions. On x86-64 Linux a
    floating-point value (`float`, `double`) is computed so too, with DWARF 5's
    typed operations, whose operands name base types of the unit (the unit's own
    of that encoding and size, else one added for them): from the registers that
    hold its operands (`DW_OP_regval_type`), their spill slots
    (`DW_OP_deref_type`) and constants (`DW_OP_const_type`), by an addition,
    subtraction, multiplication, division or negation (in the value's format,
    as the program computes it), a conversion between the formats or from an
    integer (`DW_OP_convert`) or a bit cast to or from an integer of its width
    (`DW_OP_reinterpret`); not a conversion to an integer (the program's
    saturates), nor from an entry value. Not on AArch64, whose system debugger
    evaluates none of the typed operations, nor in macOS's debug information
    or the PDB. The system's debugger computes `DW_OP_mul` and `DW_OP_shl`
    exactly and keeps the low bits of a negative result's magnitude when it is
    below -2^63, so the expressions multiply and shift left only values they
    made not negative (masked to the bits that matter); a shift's count is
    taken modulo the width the machine's shift reads;
  - a register or spill slot holds a value until something writes it: an instruction
    that only reads a slot (a reload folded into an arithmetic instruction, a
    comparison with it) leaves it, and every slot the code spills to is followed;
    a machine instruction's own start sees the registers it writes as they were;
  - a vector, a `long double` of 16 bytes or a 128-bit value in one vector register
    is in it (DWARF's number for the register names all of it); a 128-bit integer
    or a software `long double` in two general registers, and a vector wider than the
    target's registers (computed in 128-bit pieces), are in pieces (`DW_OP_piece`);
  - a variable left in memory (its address taken, a structure not taken apart) is in
    its frame slot when all its stores remain where they were and no other object
    shares the slot.

  - **Call sites**: every call is a `DW_TAG_call_site` of its function (which says
    `DW_AT_call_all_calls`): where it returns (`DW_AT_call_return_pc`; a call in tail
    position: where it is, `DW_AT_call_pc`, flagged `DW_AT_call_tail_call`), its
    callee's address (`DW_AT_call_target`: a function's, or, for a call through a
    pointer loaded from a datum, read from there), and its arguments in integer
    registers whose values the caller's frame keeps while the callee runs
    (`DW_TAG_call_site_parameter`: constants, callee-saved registers, frame slots,
    values computed from those): what the callee's entry values need. A call in tail
    position has no parameters (its caller's frame is gone).

  What this relies on of the register allocator (`cmd/axiom/src/dloc.ax`): a machine
  instruction's operands, clobbers, copies, spills and reloads say every register and
  spill slot it changes. `cmd/axiom/internal/tests/dwarf.mjs` checks the location
  lists' shapes (inside their blocks, in order, in registers that hold values, a
  parameter's entry in its argument register);
  `cmd/axiom/internal/tests/tools/debug_coverage.mjs` counts the variables with a
  location. In development, randomly generated functions (assignments in branches,
  loops and blocks, structures, pointers into frame objects, address-taken variables,
  small functions inlined into them) stepped instruction by instruction in the
  system's debuggers, every value they show (in the function, in the functions
  inlined there, and in its caller at each call) compared with the program's own log
  of its assignments, showed no difference (x86-64; AArch64 under emulation), and the
  same comparison catches a deliberately wrong rule for the registers a call changes.
  Of the instructions stepped in such programs at `-O2` where a variable has been
  assigned (in its lifetime: a variable of a loop's body from the round's start; an
  inlined function's in the instance it was assigned in), the debugger shows a value
  at 92.9% on x86-64 and 91.0% on AArch64 for C, and at 92.0% and 92.2% for Axiom
  programs of the same shapes, counting the copies inlined. The rest are values
  nothing holds any more: merges and phis of values whose registers were reused
  (the largest part), floating-point values on AArch64 and those computed from
  values nothing holds either, values computed through more operations than
  the expressions follow, and parameters whose entry value the caller's call site
  does not give. (A value reached first through a longer chain of the values it is
  computed from is followed again from where it is reached more directly.) AArch64 trails because its calls clobber the floating-point
  registers that pass a logged value (a structure's `double` member is lost once it
  is passed) and its scheduler moves more of the code it computes ahead of where it
  is assigned.
- **Macros** (`full`, C): `.debug_macro` (DWARF 5) has each unit's definitions and
  undefinitions in order with their lines (predefined ones and `-D`/`-U` at line 0,
  before the unit's own file), and each file as it is included (from its
  `#include`'s line, a file of the line table); the texts are in `.debug_str`. Not
  the C library's own units, nor `__DATE__` and `__TIME__` (two builds are the
  same). `--debug-macros=off` leaves them out (a program including `<stdio.h>` gets
  about 4 KB of them).
- **Cost**: `-g` at `-O2` compiles C programs about a sixth slower (processor
  time), with about an eighth more memory; the compiler itself (a large program, in
  its parts: "Optimization") with about a seventh more processor time and a sixth
  more memory (the most any one process needs). The time is the location analysis
  (`dloc.ax`: what each register and slot holds, and what each variable is, followed
  through the machine code; its work at an instruction is that of what the
  instruction changes, and at a join that of what is known there), the debug
  information written, and the optimizer's records of the variables' assignments;
  most of the memory those records, which every part of a program compiled in parts
  starts with (made smaller before the parts start). The executable's debug sections
  are about the size of its code; its frame table's line tables a third of the line
  tables' DWARF ("Stack traces").
Where it goes, per target:

| Target | |
|---|---|
| `x86_64`, `aarch64` | The executable's (or object's) `.debug_*` sections: `.debug_info`, `.debug_abbrev`, `.debug_str`, `.debug_line`, `.debug_line_str`, `.debug_aranges`, `.debug_frame`, `.debug_names` (an index of the unit's subprograms, static variables, named types and enumerators), and when needed `.debug_rnglists` and `.debug_loclists`. With or without `-g`, every function's call frame information is in `.eh_frame` too ("Objects and libraries"). |
| `aarch64-macos`, `x86_64-macos` | The debug bundle `<out>.dSYM` beside the executable, which the platform's debugger finds by the executable's UUID; the executable's instructions are those without `-g`, but for the addresses of the data its frame table's line rows move. The DWARF of each object given to the link (this compiler's, or the platform's compilers' with DWARF 2 to 5) is carried in at its place: Mach-O objects refer from one debug section into another by offsets without relocations, which the linker moves by where each object's sections land (`cmd/axiom/src/dwrebase.ax`). The executable has unwind tables, with or without `-g` (`cmd/axiom/src/macunwind.ax`): `__unwind_info` gives every function a compact encoding (arm64: no frame, or the frame record with the callee-saved pairs below it, which the code lays out as the encoding has them; x86-64: no frame, or rbp's frame with up to five pushed registers), or one that names its entry in `__eh_frame` (DWARF's call frame information, exact at every instruction) where the encoding cannot say the frame (vector registers saved whole), where the stack pointer moves after the prologue, where a sibling call's jump is before the function's end, where code runs before the frame is set up, and for assembly procedures with `.cfi_` rules. Mach-O objects carry `__compact_unwind` and `__eh_frame` in the form the platform's assembler writes, and the linker merges those of the platform's compilers' objects too (a function an object gives no entry has an entry without information). `macho.mjs` decodes the tables of every macOS executable the suite builds; nothing here has run an unwinder over them. |
| `x86_64-windows` | The PDB (`--pdb=on`, the default) has every function's line table, with or without `-g`, a module per object, every function's procedure and public symbol. With `-g` also, in CodeView form (`codeview.ax`, from the same description of the program as DWARF), the types (TPI, with its hash stream: a forward reference to a structure is found by its name), ids (IPI: the functions' ids, where each structure is defined), each function's type, frame, lexical blocks (a block in several pieces of code is a block per piece) and variables, and the static objects (`S_GDATA32`, `S_LDATA32`, thread-locals as `S_GTHREAD32` at their offset in the template, which starts `.data`) and typedefs and tags (`S_UDT`) at file and block scope. At `-O0` a local is at its offset from the frame pointer (`S_REGREL32`), a parameter an `S_LOCAL` located from the call frame's register at each instruction; in optimized code a variable is an `S_LOCAL` with the ranges where it is all in a register, in a frame slot or in memory at an address held in a register, or an `S_CONSTANT` where it is one constant throughout. Not described ("optimized out" there, never a wrong value): a structure taken apart into registers (the system's symbol handler reads CodeView's subfield ranges as the whole variable), a value computed from a register (a register plus a constant, an address of a frame object or a datum), the value at an address held in a frame slot (at `-O0`: a variable length array, a parameter passed by reference); a variable length array's bound. Typedefs are `S_UDT` names (what is declared with one has the type it names); an enumerator's value that a signed 64-bit integer does not hold is `LF_UQUADWORD` when it is an unsigned 64-bit one, and a wider one is left out of its enumeration's field list (CodeView's 128-bit leaves, `LF_OCTWORD` and `LF_UOCTWORD`, make the tools that read the type records take them for corrupt); `_Atomic T` is `T`; a base type CodeView has no primitive for (decimal floating types, a bit-precise integer narrower than its storage) is a structure of its name and size holding its bytes; an Axiom union a structure of its tag and each payload at its offset. The function table (`.pdata`) describes every function with a frame (a function without one makes no call: every call on Windows reserves the shadow space, which gives the caller a frame), right at every instruction: a large frame's stack probe does not move `rsp`, and a function with sibling calls uses the frame-register form with an entry of its own for each jump (the code after it chained), so that the unwinder is right whether or not it takes a jump out of a function for the end of an epilogue. The suite decodes every unwind record as the system does before each Windows program runs (`pe.mjs`) and single-steps a walk through every kind of frame (`c/mix/win_walk`). |
| `aarch64-windows` | As `x86_64-windows`; the function table's ARM64 records describe every function with a frame (on Windows every function that calls has one). |
| `wasm32`, `wasm64` | Custom sections `.debug_info`, `.debug_line`, ... of the module, code addresses as offsets in its code section, a compilation unit per unit compiled (each object carries its own, which the linker merges; a function it did not keep is at the tombstone address -1). With `-g` the types and variables too: at `-O0` each variable in its frame slot (`DW_OP_fbreg`), the frame base being the local that holds the frame's address (`DW_OP_WASM_location 0`, or the stack pointer, global 0, without a frame); in optimized code a variable is in the function's local that holds its value (`DW_OP_WASM_location 0`: a parameter's, or the local a value was given while it holds that value: locals are shared by values not live at once), a constant, or a frame object's address, followed over the blocks as on native targets (not computed from other values, no entry values or call sites); a static object at its address, a thread-local none (each thread's instance is elsewhere). No call frame information, which WebAssembly has no use for. |

The objects of `--emit=obj` carry the same sections (ELF targets), with relocations,
for the system's linker. Axiom functions in the same program are described in the same
compilation unit, whose language is C's. An Axiom program's own units are marked as
C11 too (`DW_AT_language` 0x1d; the producer, `axiom <version>`, says which compiler
wrote them), not with a code of DWARF's user range: a debugger with no type system for
a code it does not know shows none of their variables, and under C's both of the
system debuggers tried show Axiom's variables, structures, slices (`ptr`, `len`),
enumerations and error unions, set breakpoints on functions by their `module.name`
(one reads a global by its name quoted, `'m.g'`; the other finds none by such a name), and
evaluate arithmetic on them; one shows a tagged union's variant part (the payload of
the variant the tag names), the other only its tag. `--debug-language=axiom` gives the
units Axiom's own code, 0x8a78 (`docs/cli.md`, `-g`), for tools that tell them apart
by it. The PDB's compile records (`S_COMPILE3`) say C (`CV_CFL_C`) for every module.

The line of a crash's faulting instruction is in the stack trace when the program has
line tables (above).

## Optimization

`-O0` (the default) to `-O3` are the compiler's: C goes through the same passes as Axiom
code (README of `cmd/axiom/`, "Optimization"). All the units of a program are one module,
so `-O2` and `-O3` inline across them. A `restrict`-qualified pointer parameter (passed in
a register) tells the optimizer that what it reaches is not reached through another
parameter, a global or a local object while the function runs (6.7.3.1): loads move out
of loops past stores through it, and copy and vector loops need no overlap check. A
pointer loaded from memory may hold its value, and is not taken as apart from it.

Within a block that declares restrict-qualified pointer objects (automatic ones, in a
block with no label in it, which a jump could enter it by), an access through one of them
(through any pointer based on it: arithmetic, casts, copies, conditionals, a loop's
pointer, an inner block's restricted pointer) is taken as apart from one through another
of them, and from one through anything not based on it (a parameter, a global, a local
object, another restricted pointer), while the block runs (6.7.3.1p4), in each of its
executions. Only accesses made in the block count: a copy of the pointer used after the
block, an access after it, one in another iteration of a loop the block is in, and one
through a pointer that went through memory, an integer or a call (which may be based on
it) are not taken as apart; nor are any of an object whose address is taken (`&p`). When
a function with restrict parameters is inlined, its call is such a block for its body,
with its parameters for those objects, and the code of an inlined function is in the
blocks its call is in, so what they tell survives inlining. A block's `static` restricted pointer is one of its block's too (only
the values the block gives it count; the value it keeps from an earlier execution is
not taken as based on it there), except in a function with nested functions. Structure
assignments and fills (`memcpy`, `memset`) are accesses like any others. The block
of a file-scope restricted pointer, and of a restrict-qualified member of a structure
type declared at file scope, is `main`'s (6.7.3.1p2): what runs before `main`
(constructors) or after it (exit handlers, destructors) is outside it, and only `main`'s
own code (and what is inlined into it) is known to be inside, so only there are they
taken as apart: accesses through a file-scope restricted pointer from everything not
based on it, and accesses through two restrict-qualified members of one structure object,
or through one and to the structure object itself, from each other. A local structure
with such members is still taken apart into its members' values. The members of a
structure type declared in a block are not used (the block may run again, each time
another execution). "Based on" is the flow of the
pointer's value: an integer added to another pointer, and a comparison, do not make an
address based on the restricted one. The IR marks these with `IN_RSCOPE`,
`IN_RESTRICT` and the tags of loads, stores and calls (`ir.ax`; the argument that this
is sound is in `c/cgen_restrict.ax`), which the last pass removes.

A parameter that the attribute `nonnull` names (a GNU dialect), or that is declared
`[static n]` (6.7.6.3p7), and the result of a call of a function declared
`returns_nonnull`, are taken not to be null pointers: a test of one against null is
folded (`--c-keep-null-checks` keeps them). A `[static n]` parameter's first n elements
may also be read before a loop that reads them, whether or not the loop runs. The result
of a function declared `malloc` is taken to point to an object that nothing that exists
when the call returns points into: accesses through it are apart from accesses through
the function's parameters, other stack and static objects and other such results
(not from a pointer read from memory, which may hold its value).

A path that ends in `unreachable()` (C23's `<stddef.h>`) or `__builtin_unreachable()`
is one the program never takes. What the condition that leads to it says holds below
the test (a value compared with a constant, or tested for its sign: later tests of it
fold, and a signed division or remainder by a positive constant of a value that is not
negative is the unsigned one); a switch whose default is such a path, or whose value
those conditions keep among its cases' values, has no range check before its table
or jump table; and once the switches are made, the tests that lead to such a path
are left out. A path that only reaches a trap (`__builtin_trap()`, a call of a function
that does not return) is not one of these: its test stays.

A function declared with the attribute `const` (a GNU dialect), or with C23's
`[[unsequenced]]` after its parameter list, promises that its calls read no memory and
write none; `pure`, or `[[reproducible]]`, that they write none (they may read any).
Either promise also says that a call returns: it does not loop forever, end the program
or `longjmp` out (this compiler's rule for these attributes; a function also declared
`_Noreturn` or `[[noreturn]]` is not taken at its word). On it the optimizer removes a
call whose result is unused, makes one call of two with the same arguments (for `pure`
and `[[reproducible]]` only when nothing that may write what the function can read comes
between them: a store to a global, through a pointer the program computed, a call that
may write; not a store to a local whose address is never taken, nor one through a
`restrict` parameter of the caller that no argument may be based on), and makes a call
in a loop before the loop when the loop writes nothing it reads and the call runs each
time the loop is entered (in its first block, before anything that may stop the loop; a
call on a path that some iterations skip, or in a loop whose body may not run at all, is
not made where it was not). C23's attributes are used only on the function declarator
of a declaration or definition of a function whose parameters all have arithmetic
types and that is not variadic (C23 lets a call of such a function access, and an
effectless one modify, what its pointer parameters point to); through a pointer to a
function, or a typedef, they are not used. A call of a function of another type than
its declaration's, or through a pointer, is not acted on, nor one whose result is a
structure or union, or that passes one in registers (one passed in memory, which the
callee reads, makes `const` a `pure`). Type-based
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
  `CX_LIMITED_RANGE` changes nothing: `ON` only permits the simpler formulas (7.3.4p1),
  and the full algorithms are always used.
- The decimal floating types: on WebAssembly their exceptions are not recorded (no flags
  there, as for binary).
- `FLT_ROUNDS` is always 1: it does not follow `fesetround` (C17 5.2.4.2.2p8 asks it to
  give the current rounding mode; "Implementation-defined behavior", floating types).
- (Not a deviation, but often asked: in a GNU dialect the destructors run as `atexit`'s
  functions do, when `main` returns and from `exit`, and so not from `_Exit`,
  `quick_exit` or a signal, which end the process without them.)

## Known gaps

Everything this implementation does not do yet, or does only in part, in one place: one
line each, with the section that says more. What a target, the standard or a dialect
leaves out by design is in "Not available"; where the implementation departs from a
standard it claims, "Known deviations".

**Language and front end**

- No assembly output: `--emit=asm` is an error ("`--emit`").
- No Microsoft extensions but `__declspec` ("GNU dialects", Language).
- At most 128 trampolines of one nested function live at once; one more traps ("GNU
  dialects", Language: nested functions).
- No option sets the stack of the threads `thrd_create` starts (8 MiB natively, 1 MiB on
  WebAssembly: "Threads", Stacks).
- Annex L (analyzability) is not claimed (`docs/c-conformance.md`, "Annexes").

**Library**

- Locales: the `"C"` conventions only, collation by code values, Unicode 15.1's classes
  ([`docs/c-text.md`](c-text.md), "Not provided, or limits").
- Every hosted program's build analyzes the time zone module (`std/c/rt/text_time.ax`,
  `std/time/tz`) whether it uses it or not (`docs/c-text.md`).
- POSIX's streams (`fmemopen`, `fileno`, `popen`, ...) are not provided ("Standard
  input/output", Exit).
- With the compiler's C library, objects built against the system's headers may not use
  the system library's own names, its POSIX stream functions, `fgetpos`/`fsetpos` or the
  restartable multibyte functions, and shared libraries built against it cannot be linked
  (`E9004`: "One C library").
- Mutexes are neither fair nor robust ("Threads").
- On macOS under `--macos-tls=fast` a thread that the runtime did not start (the system's
  `pthread_create`) has no thread-locals ("Atomics and threads").
- Intrinsics not provided: AVX-512's, AMX's, Key Locker's, the system instructions'
  (`_xsave` ...), `<arm_sve.h>`, FCMA, FRINTTS, JSCVT, RNG, MTE and TME, `__arm_rsr` and
  `__arm_wsr`; RDRAND and RDSEED intrinsics are not checked against `--features`; the
  non-temporal hints are dropped ("Intrinsics").

**Mathematics**

- WebAssembly has no floating-point environment: rounding to nearest only, no exception
  flags (binary or decimal), so no Annexes F and G there ("The floating-point
  environment", "Known deviations").
- `FLT_ROUNDS` does not follow `fesetround` ("Known deviations").
- `cpow` beyond its bounds (`B` above 2^90 for double, 2^120 for float, 2^70 for the
  x87 format, 2^26 for binary128) loses accuracy as `B 2^-150`: wider kernels are not written (the
  complex functions, "`<math.h>` ...").
- A complex result's part that is subnormal is within up to 1.4 ulps (rounded twice)
  (the complex functions).
- The decimal elementary functions of trigonometric arguments beyond 10^17 take
  milliseconds (exact reduction: "Decimal floating types", Speed).
- Speed against the system's library: `log2` 1.5 times its time, `pow`, `powf`, `acosh`,
  `atanh` 1.4 to 1.5; the x87 format's `long double` functions up to 2.2 times
  ("`<math.h>` ...", Speed, `long double`).

**Assembler**

- x86-64: APX, AMD's XOP and some legacy and system instructions (`lar`, `lsl`, Key
  Locker, ENQCMD, MOVDIRI, RAO-INT, VMX, ...) are C3097 ("Inline assembly", x86-64).
- Call frame directives not written: data-, text- and function-relative encodings,
  2-byte pointers, `.cfi_val_encoded_addr`, `.cfi_inline_lsda`; `.seh_handler`;
  `.cfi_personality` and `.cfi_lsda` on macOS and Windows; in asm statements the
  `.cfi_*` that change the frame and every `.seh_*` (so on Windows a register a template
  saves is not described) ("Inline assembly", call frame information).
- AArch64 on macOS and Windows: an absolute address in a move-wide group other than the
  complete sequence, or in `.word`/`.hword`, is C3097 ("Inline assembly", AArch64).
- WebAssembly: active data and element segments, and memories or tables other than the
  program's, are C3097 by rule ("Inline assembly", WebAssembly).

**Linking**

- A universal (multi-architecture) Mach-O object is not read, nor are the SDK's `.tbd`
  stubs (such a library is bound by name) ("Objects and libraries").
- A long-format Windows import library is an error ("Objects and libraries").
- `--emit=shared` takes no Axiom modules and not the compiler's C library: `--c-lib=system`
  or `--c-freestanding` ("Shared libraries").
- A C source given with `--link=` needs the system's C compiler on Linux, and `clang` on
  WebAssembly ("Objects and libraries").
- macOS: nothing here has run an unwinder over the executables' unwind tables, nor
  thrown through the platform objects' personality routines (`c/objfmt/unwind_personality`
  is built only: "Objects and libraries", "Debugging").

**Debug information**

- Columns are not recorded in the line tables ("Debugging", Lines).
- Optimized code: at about 7% (x86-64) to 9% (AArch64) of the instructions where a C
  variable has been assigned, the debugger shows it "optimized out": merges whose registers were reused, values
  computed through more operations than the expressions follow, missing entry values
  ("Debugging", locations at `-O1` and above).
- Floating-point values computed from their operands only on x86-64 Linux: not on
  AArch64, not in macOS's debug information or the PDB ("Debugging").
- WebAssembly: optimized code's variables are not computed from other values, have no
  entry values or call sites; thread-locals have no location ("Debugging", table).
- The PDB does not describe structures taken apart into registers, values computed from
  a register, values at an address held in a frame slot (variable length arrays,
  parameters passed by reference), variable length arrays' bounds, or enumerators wider
  than 64 bits ("Debugging", table).
- macOS: the thread-locals' descriptors in the debug information (both schemes) have not
  been tried in the platform's debugger ("Debugging", Types and variables).
- The C library's own units have no macros in `.debug_macro` ("Debugging", Macros).

**Code generation and run time**

- At `-O2` and `-O3` a C function whose last act is a call is missing from stack traces,
  and without line tables a crash in code inlined into a function that makes no call
  can show no frames at all ("Stack traces").
- The optimizer treats every atomic operation as sequentially consistent (it forgoes
  what weaker orders would allow) ("Atomics and threads", Memory orders).
- A function that calls `setjmp` (and one whose nested functions may jump to its local
  labels) is not optimized ("setjmp and longjmp", "GNU dialects").
- `--unwind-tables=off` stops the system's unwinder at this compiler's frames ("Objects
  and libraries").
- `--macos-tls=platform`: a function without a frame that reaches a thread-local cannot
  be unwound past while the thunk runs ("Atomics and threads").

**Targets and output never executed**

- `x86_64-macos` and `aarch64-windows`: built and checked structurally, never run
  ("Calling conventions and interoperability").
- Windows on ARM64's `longjmp` unwinding and fault delivery to signal handlers: never
  run ("Signals").
- macOS output of the C front end (programs, objects, `.dylib`s, unwind tables): built
  and checked only ("Shared libraries", "Debugging").
- `--macos-tls=platform`, and so every macOS object, static library and dynamic library
  (which imply it): never executed ("Atomics and threads").
- Windows x64 programs are run only under a compatibility layer where one is installed;
  otherwise built and checked only ("Calling conventions and interoperability").
