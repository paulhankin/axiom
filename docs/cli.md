# The `axiom` command

Every command of the compiler, its flags, the environment variables it reads, where it
keeps its cache, and its exit codes. The stable interfaces for tools are in the
specification: diagnostics as JSON (spec §16.2), the test report (§17.2) and canonical
formatting (§16.5). This page describes the compiler as it is; the spec says what is
promised.

```
axiom build   <file.ax | package dir> [options] -o <out>
axiom check   <file.ax | package dir> [options]
axiom build   <file.c>... [options] [-o <out>]     (C sources: docs/c.md)
axiom check   <file.c>... [options]
axiom test    <file.ax | package dir> [options] [--filter=<text>] [--timeout=<ms>] [--show-output] [--no-run=<file>]
axiom profile <file.ax | package dir> [options] [--rate=<hz>] [--folded=<path>] [-- <program args>]
axiom fmt     <file.ax | dir>... [--check] [--stdout]
axiom new     <name> [--format=agent-json]
axiom lsp     [--target=<t>] [--std=<dir>]
axiom cache   [path | size | clean]
axiom version
axiom update  [<version> | latest | head]
```

With no arguments, `axiom` prints a summary of this page and exits with status 2.

## Commands

### `axiom build`

Compiles a program. The input is a `.ax` file, a package directory (one with an
`axiom.toml`, whose entry module is built), nothing, meaning the package in the current
directory, or one or more `.c` files, which are C translation units of one program
(see [C sources](#c-sources)). The output (`-o`) is an executable for native targets, a
`.wasm` module for WebAssembly, or an object or text file with `--emit`. Without `-o` a
package builds to a file named after the package. Modules are compiled separately and
cached by default at `-O0` and `-O1` (spec §16.7; `--units`).

### `axiom check`

Parses and type-checks the program, generating no code. Tests are checked too (spec
§17.1). The same diagnostics as `build`; the language server runs this on every edit.

### `axiom test`

Builds the program's `test` blocks and runs them, each in its own process or
WebAssembly instance, in parallel (spec §17.2). Given a package directory, every module
under its `src/` is loaded, so tests in modules the entry module does not import still
run; dependencies' tests do not.

| Flag | |
|---|---|
| `--filter=<text>` | Run only the tests whose name contains `text`. |
| `--timeout=<ms>` | Fail a test that runs longer (default 10000). |
| `--show-output` | Print what each test wrote under its result line. Without it, only a failing test's output is shown. |
| `--no-run=<file>` | Build the test binary into `<file>` (and, for Windows, its PDB) without running it: for a target this machine cannot run. |

WebAssembly tests need [Node.js](https://nodejs.org) on the `PATH`; `--target=native`
needs nothing else.

### `axiom profile`

Builds the program and runs its `main` under the sampling profiler, then reports the
hottest functions and call sites (spec §16.6). Arguments after `--` go to the program;
its standard input is the profiler's.

| Flag | |
|---|---|
| `--rate=<hz>` | Samples per second (default 999). |
| `--folded=<path>` | Write folded stacks (one line per stack and its count), the format flame graph tools read. |

### `axiom fmt`

Rewrites files in the canonical layout and tidies their imports (spec §16.5). It takes
files and directories (searched for `.ax` files).

| Flag | |
|---|---|
| `--check` | Change nothing; print the files that are not canonical and exit 1 if there are any. |
| `--stdout` | Print the formatted text of one file instead of rewriting it. |

### `axiom new <name>`

Creates a package: a directory `name/` with an `axiom.toml` and a `src/lib.ax` that has
a test (spec §4.6). `--format=agent-json` reports the files created as JSON.

### `axiom lsp`

The language server, over standard input and output ([`docs/editors.md`](editors.md)).
`--target` and `--std` apply to every check it runs.

### `axiom cache`

The cache of compiled modules (spec §16.7): `path` (the default) prints its directory,
`size` prints the directory, the number of objects, their size and the limit, and
`clean` removes it.

### `axiom version`

Prints `axiom <version>`.

### `axiom update`

Replaces the toolchain it belongs to with another version: `0.N.M` (or `v0.N.M`), the
`latest` release, or `head` (in any case: `HEAD`), the head of `main` on GitHub.
[`docs/install.md`](install.md#updating) has the details:

- An install from a release archive updates to a release (the latest by default),
  downloaded with `curl`, checked against the release's `SHA256SUMS`, unpacked with
  `tar` and swapped in place of the old one. It cannot update to `head`, which is not
  a release (`E9001`).
- A source checkout (the compiler at `cmd/axiom/build/axiom` of a git clone) moves to
  the version's tag, or to the head of `main` (the default), and rebuilds the compiler.

A failure is `E9008`, and leaves the old toolchain in place.

## Options

Options take the form `--name=value`; they may come before or after the input. Every
command accepts `--format`, `--std` and `--target`.

### Target and processor

| Option | |
|---|---|
| `--target=<t>` | `wasm32` (the default), `wasm64`, `x86_64`, `aarch64` (both Linux), `aarch64-macos`, `x86_64-windows`, `x86_64-macos`, `aarch64-windows`, or `native`: this machine, with its processor's extensions. `x86_64-macos` (Intel Macs) and `aarch64-windows` (Windows on ARM64) build, and their executables are checked structurally, but they have not yet been run on real systems. |
| `--cpu=<name>` | A processor preset for the extensions to use (`x86-64-v3`, `neoverse-n1`, …; spec §13.4). |
| `--features=+ext,-ext` | Enable or disable extensions one by one (`+avx512bw`). |

A program built for a plain `x86_64` or `aarch64` target runs on any processor of that
architecture; `native` and `--cpu` tune it to one and it traps at start on a processor
that lacks the extensions it was built with.

### Code generation

| Option | |
|---|---|
| `-O0` … `-O3` | Optimization level (default `-O0`). |
| `--overflow=wrap\|trap\|saturate` | Integer overflow mode (spec §7.6; default `wrap`). |
| `--stack-traces=on\|off` | Record call sites for crash traces and profiles (default on; spec §16.4). |
| `--sanitize=address\|none` | Check heap accesses: use after free, double free, overflows, leaks (spec §16.8). |
| `--coverage=afl\|none` | Native Linux targets: edge coverage counters for a coverage-guided fuzzer, and its fork server (spec §16.9; default `none`). |
| `-g`, `--debug-info=full\|lines\|none` | DWARF 5 debug information for debuggers and other tools (`-g` is `full`; default `none` at every optimization level: stack traces need none, and without it nothing is added). `lines`: a compile unit per object with a subprogram per function, line tables (C's `#line` followed; code inlined keeps its own lines), call frame information (`.debug_frame`) for every instruction; `full` adds the types, each function's result, parameters and locals in their lexical blocks, and the static objects and globals, for C and Axiom (locations at `-O0`, where every variable stays in its frame slot; at `-O1` and above, location lists that follow each variable through its assignments, and frame slots for the variables left in memory: never a wrong one, the others are "optimized out"). Axiom's types: integers, floats, bool, pointers, arrays, slices (a structure of `ptr` and `len`), structures, enumerations, unions (a tag and a DWARF variant part), error unions (`error`, `value`), function types; functions named `module.function` (an exported one by its own name), the units' language C11's (0x1d: debuggers evaluate Axiom's names, structures, slices and arithmetic under it, and one that knows no other code shows no variables otherwise; the producer, `axiom <version>`, names Axiom), or with `--debug-language=axiom` Axiom's own code of DWARF's user range (0x8a78). The instructions are the same with and without it: an object's code and relocations are identical, while in an executable the frame table's line rows (for crash traces) move the data after them, and with it the addresses instructions hold. ELF targets: in the executable or object; macOS: in `<out>.dSYM`; WebAssembly: `.debug_*` custom sections (code-section offsets); Windows: the PDB (`--pdb`) has the line tables either way, and with `full` the types and variables in CodeView form. See [debugging](c.md#debugging). (The bootstrap compiler writes line tables for `-g`.) |
| `--debug-macros=on\|off` | With `-g`: C's macros in the debug information (`.debug_macro`; default `on`). |
| `--debug-language=c\|axiom` | With `-g`: an Axiom program's units marked as C11 (`c`, the default), which debuggers read, or with Axiom's own language code (`axiom`). |
| `--release` | Accepted for compatibility; changes nothing. |
| `--units=module\|program` | Compile modules separately and cache them, or the program as one unit (default: `module` at `-O0`/`-O1`, `program` at `-O2`/`-O3`). |
| `--verbose` | Report the modules compiled and taken from the cache, and evictions. |

### Output

| Option | |
|---|---|
| `-o <out>` | The output file. |
| `--emit=exe\|obj\|lib\|shared\|ir` | An executable (or `.wasm` module), one object file (ELF, Mach-O or COFF: the target's format), a static library of it (and of the objects among the inputs), a shared library of C units (an ELF `.so`, a Mach-O `.dylib` or a DLL: [`docs/c.md`](c.md), "Shared libraries"), or the compiler's IR as text. `--emit=asm` is an error: there is no assembly output (use `--emit=obj` and a disassembler, or `--emit=ir`). |
| `--pic` | Code for a shared library made by another linker: on Linux, thread-locals use the initial-exec model (`--emit=shared` implies it; the rest of the code is position-independent already). |
| `--emit-header=<file.h>` | Also write a C header for the program's `export`ed functions (spec §14.5). |
| `--format=human\|agent-json` | Diagnostics as text on stderr, or as one JSON object on stdout (spec §16.2). |

### C sources

Options for `axiom build` and `axiom check` with C sources (`.c`, and assembly sources
`.s` and `.S`), which are compiled by the compiler's own C front end
([`docs/c.md`](c.md), "C options"). They are errors with Axiom inputs.

| Option | |
|---|---|
| `--c-std=<std>` | `c89` (`c90`), `c95`, `c99`, `c11`, `c17` (`c18`) or `c23`, the `iso9899:` names of those, or a GNU dialect `gnu89` (`gnu90`), `gnu99`, `gnu11`, `gnu17` (`gnu18`), `gnu23`; default `c17`. |
| `--c-char=signed\|unsigned` | Whether plain `char` is signed (default: the target's C ABI). |
| `--c-int=16\|32\|64` | The width of `int` (default 32). |
| `--c-long=32\|64` | The width of `long` (default: the target's C ABI; at least `--c-int`). |
| `--c-long-double=64\|80\|128` | The format of `long double` (default: the target's C ABI): `80` on x86-64 Linux and macOS only, `128` there, on AArch64 Linux and WebAssembly. |
| `-D<name>[=<value>]`, `-U<name>` | Define or undefine a macro (`-D name` and `-U name` too). |
| `-A<pred>=<answer>`, `-A<pred>(<answer>)`, `-A-<pred>[=<answer>]`, `-A-` | A GNU dialect's assertion (`#assert`), or its removal; an error under an ISO standard. |
| `-I<dir>` | An include directory (`-I dir` too). `-D`, `-U`, `-A` and `-I` apply in the order given. |
| `--c-asm-dialect=att\|intel` | The syntax of x86-64 asm statements, file-scope asm and assembly sources (default `att`). |
| `--c-fenv-access` | Every function as under `#pragma STDC FENV_ACCESS ON`. |
| `--c-keep-null-checks` | Keep the tests against null of pointers that `nonnull`, `returns_nonnull` or `[static n]` say are not null. |
| `--c-warn-format=0\|1\|2` | The checks of format strings: none, the clear errors (the default), or also what is suspect. |
| `--c-warn-unknown-pragmas` | An unknown `#pragma` is the warning `C4111`. |
| `--c-freestanding` | A freestanding program: no C library is linked, and only the freestanding headers are found. |
| `--c-lib=axiom\|system` | The program's C library: the compiler's (the default) or the system's, whose headers are then the hosted headers (native targets). |
| `--c-system-include=<dir>` | With `--c-lib=system`: a directory of the system's headers (required for macOS and Windows targets). |
| `--emit=c-pp\|c-tokens\|c-ast` | The preprocessed text, the tokens, or the syntax tree, of one source. |

The summary `axiom` prints with no arguments lists most of these; `--c-freestanding`,
`--c-lib`, `--c-system-include`, `--c-warn-format`, `--c-warn-unknown-pragmas` and `-A`
are accepted though it does not show them.

With C sources, `--emit=obj` and `--emit=ir` of units that define no `main` give their
code alone. `--overflow`, `--sanitize`, `--coverage`, `--units` and `--emit-header` are
errors, except that an Axiom program with C units (`axiom build main.ax x.c`) takes
`--overflow` and `--emit-header`. `axiom build main.ax x.c...` builds an Axiom program
with C translation units, `axiom build main.c... x.ax...` a C program with Axiom modules,
into one program (docs/c.md, "C and Axiom in one program"); a package's manifest names
its C sources with `c-sources = "dir"`.

### Linking

| Option | |
|---|---|
| `--link=<file>` | Link a C source, object, archive, shared library or `.ax` module into the program (an object or library may also be given as an input: `x.o`, `libx.a`, `libx.so`). A C source is compiled first: for Linux by the system's C compiler (`cc`, or the cross compiler for the target), for macOS and Windows by this compiler, for WebAssembly by `clang`. The compiler's own linkers link the objects and libraries, on every target ([`docs/c.md`](c.md), "Objects and libraries"). |
| `-l<lib>`, `-L<dir>` | A library to link and a directory to search before the system's: on Linux `lib<lib>.so`, else `lib<lib>.a`; on macOS `lib<lib>.a`, else `lib<lib>.dylib`, else bound by name; on Windows `<lib>.lib`, `lib<lib>.a`, `lib<lib>.dll.a` or `<lib>.dll` in the `-L` directories. |
| `-I<dir>` | An include directory, for C sources given with `--link` and for the C front end's inputs. |
| `--function-sections`, `--data-sections` | Native targets: each function, each datum in a section of its own ([`docs/c.md`](c.md), "Code in sections"). |
| `--gc-sections`, `--no-gc-sections` | Native targets: the compiler's linkers leave out the sections of the linked objects and archives that nothing refers to (default: they keep them). |

Programs for native targets are linked by the compiler's own linkers, with the objects
and libraries they are given; one that uses no shared library (on Linux, none of the
system's C library either) is a static executable. A WebAssembly program whose
`extern "C"` blocks name libraries is linked by `wasm-ld`.

### Platforms

| Option | |
|---|---|
| `--subsystem=console\|windows` | Windows: a console program (default) or a GUI one. |
| `--pdb=on\|off` | Windows: debug symbols in `<out>.pdb` (default on). |
| `--icon=<file.ico>` | Windows: the program's icon. |
| `--macos-tls=fast\|platform` | macOS thread-locals: `fast` (the default) is the runtime's per-thread block, found through a TSD slot, about five instructions an access, as Axiom programs have run on Macs; `platform` is the platform's thread-local variables (descriptors in `__thread_vars` whose thunk the platform's dynamic linker provides), which interoperate with the platform's objects and libraries and which every thread has, but which have **never been executed** (built and checked only), assume what the thunk preserves (arm64: all but x0, x16, x17, lr and the flags; x86-64: all but rax, rdi and the flags), cost about 14 instructions and a call an access, and cannot be unwound past a frameless function while the thunk runs. `--emit=obj`, `lib` and `shared` imply `platform`; code of one scheme reaching the other's variable is a link error. The default may change once `platform` has run on a Mac ([`docs/c.md`](c.md), "Atomics and threads"). |
| `--unwind-tables=on\|off` | Linux: call frame information (`.eh_frame`) for every function the compiler makes and for the linker's stubs, which the platform's unwinder walks by (default `on`). With `off` there is none (and no `.eh_frame_hdr`, unless objects of the platform's compilers bring FDEs): the program is smaller (about 9% for one that prints a line, 2% for the compiler), and its own stack traces are the same (they walk the frame pointers and the frame tables), but the system's unwinder cannot pass through this compiler's frames: a backtrace of the system's library stops at them, and another language's exceptions cannot be thrown through them. `-g`'s `.debug_frame` is there either way ([`docs/c.md`](c.md), "Objects and libraries"). |
| `--stack-size=<n>[K\|M\|G]` | The main thread's stack, in bytes (any target): on Windows the image's stack reserve (default 8 MiB; 64 KiB committed at first; the threads of `std/thread` and `thrd_create` have 8 MiB whatever it says); on macOS `LC_MAIN`'s stack size (default: the system's 8 MiB); on WebAssembly the stack in linear memory (default 1 MiB). On Linux the main thread's stack is the system's limit (`ulimit -s`) and the option has no effect. |

### The standard library

| Option | |
|---|---|
| `--std=<dir>` | The standard library's directory. Otherwise `$AXIOM_STD`, else `std/` next to the compiler's `bin/` directory, else `../share/axiom/std`, else the `std/` of the current directory. |

## Environment variables

| Variable | |
|---|---|
| `AXIOM_STD` | The standard library's directory, when `--std` is not given. |
| `AXIOM_CACHE` | The cache directory: compiled modules in `<cache>/obj/`, git dependencies in `<cache>/`. Otherwise `$XDG_CACHE_HOME/axiom`, else `~/.cache/axiom`. |
| `AXIOM_UPDATE_URL` | The repository `axiom update` updates from, instead of `https://github.com/paulhankin/axiom`: a mirror whose `releases/download/v<version>/` holds the release archives and `SHA256SUMS`, and which git can fetch from. |
| `AXIOM_JOBS` | How many modules (`--units=module`), or parts of an optimized program, are compiled at once: the number of processors by default. The output does not depend on it. |
| `AXIOM_CACHE_LIMIT` | The size the compiled-module cache is kept under after a build: a number of bytes, or one with a `k`, `m` or `g` suffix; `0` for no limit; 2 GiB by default (spec §16.7). |

The variables that switch on debugging output inside the compiler are listed in
[`cmd/axiom/README.md`](../cmd/axiom/README.md).

## Exit codes

| Code | |
|---|---|
| 0 | Success. For `axiom test`, every selected test passed; for `fmt --check`, every file is canonical. |
| 1 | The program has errors, a test failed, or a file is not canonical. |
| 2 | Invalid invocation (an unknown option, a missing input, a tool the command needs), or an internal compiler error. |

A program built by `axiom build` exits with `main`'s result; a trap or a crash exits
with a report and a stack trace on stderr (spec §16.4) and a non-zero status.

## Running WebAssembly output

The runtime's node host runs a `.wasm` program: `node <std>/internal/runtime/host.mjs
program.wasm [args]`, where `<std>` is the standard library's directory. It provides the
host functions the standard library uses (files, sockets, time, threads) on Node.js 18
or later.
