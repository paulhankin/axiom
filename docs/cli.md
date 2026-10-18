# The `axiom` command

Every command of the compiler, its flags, the environment variables it reads, where it
keeps its cache, and its exit codes. The stable interfaces for tools are in the
specification: diagnostics as JSON (spec §16.2), the test report (§17.2) and canonical
formatting (§16.5). This page describes the compiler as it is; the spec says what is
promised.

```
axiom build   <file.ax | package dir> [options] -o <out>
axiom check   <file.ax | package dir> [options]
axiom test    <file.ax | package dir> [options] [--filter=<text>] [--timeout=<ms>] [--show-output]
axiom profile <file.ax | package dir> [options] [--rate=<hz>] [--folded=<path>] [-- <program args>]
axiom fmt     <file.ax | dir>... [--check] [--stdout]
axiom new     <name> [--format=agent-json]
axiom lsp     [--target=<t>] [--std=<dir>]
axiom cache   [path | size | clean]
axiom version
```

With no arguments, `axiom` prints a summary of this page and exits with status 2.

## Commands

### `axiom build`

Compiles a program. The input is a `.ax` file, a package directory (one with an
`axiom.toml`, whose entry module is built), or nothing, meaning the package in the
current directory. The output (`-o`) is an executable for native targets, a `.wasm`
module for WebAssembly, or an object or text file with `--emit`. Without `-o` a package
builds to a file named after the package. Modules are compiled separately and cached by
default at `-O0` and `-O1` (spec §16.7; `--units`).

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

WebAssembly tests need [Node.js](https://nodejs.org) on the `PATH`; `--target=native`
needs nothing else.

### `axiom profile`

Builds the program and runs its `main` under the sampling profiler, then reports the
hottest functions and call sites (spec §16.6). Arguments after `--` go to the program;
its standard input is the profiler's.

| Flag | |
|---|---|
| `--rate=<hz>` | Samples per second (default 999). |
| `--folded=<path>` | Write folded stacks for `flamegraph.pl`, inferno or speedscope. |

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

## Options

Options take the form `--name=value`; they may come before or after the input. Every
command accepts `--format`, `--std` and `--target`.

### Target and processor

| Option | |
|---|---|
| `--target=<t>` | `wasm32` (the default), `wasm64`, `x86_64`, `aarch64` (both Linux), `aarch64-macos`, `x86_64-windows`, or `native`: this machine, with its processor's extensions. |
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
| `-g`, `--debug-info=lines\|none` | DWARF line tables (a compile unit per object, a subprogram per function, a line and column per instruction, inlined functions included), for `perf annotate`, debuggers and `addr2line -i`. Off by default at every optimization level: stack traces need no debug information. Today only the bootstrap compiler's LLVM backend emits them; the compiler's own backends accept the option and emit none. |
| `--release` | Accepted for compatibility; changes nothing. |
| `--units=module\|program` | Compile modules separately and cache them, or the program as one unit (default: `module` at `-O0`/`-O1`, `program` at `-O2`/`-O3`). |
| `--verbose` | Report the modules compiled and taken from the cache, and evictions. |

### Output

| Option | |
|---|---|
| `-o <out>` | The output file. |
| `--emit=exe\|obj\|ir\|asm` | An executable (or `.wasm` module), one object file, the compiler's IR as text, or assembly. |
| `--emit-header=<file.h>` | Also write a C header for the program's `export`ed functions (spec §14.5). |
| `--format=human\|agent-json` | Diagnostics as text on stderr, or as one JSON object on stdout (spec §16.2). |

### Linking

| Option | |
|---|---|
| `--link=<file>` | Link a C source, object, archive or `.ax` module into the program. C sources are compiled with `cc`; the compiled runtime objects are cached. |
| `-l<lib>`, `-L<dir>` | A library to link and a directory to search, passed to the system linker (native targets with C code). |
| `-I<dir>` | An include directory for C sources given with `--link`. |

Programs for native targets that link no C code are linked by the compiler's own
linkers and are static executables; with C code or `-l` the system's `cc` links them.

### Windows

| Option | |
|---|---|
| `--subsystem=console\|windows` | A console program (default) or a GUI one, for `--target=x86_64-windows`. |
| `--pdb=on\|off` | Debug symbols in `<out>.pdb` (default on). |
| `--icon=<file.ico>` | The program's icon. |

### The standard library

| Option | |
|---|---|
| `--std=<dir>` | The standard library's directory. Otherwise `$AXIOM_STD`, else `std/` next to the compiler's `bin/` directory, else `../share/axiom/std`, else the `std/` of the current directory. |

## Environment variables

| Variable | |
|---|---|
| `AXIOM_STD` | The standard library's directory, when `--std` is not given. |
| `AXIOM_CACHE` | The cache directory: compiled modules in `<cache>/obj/`, git dependencies in `<cache>/`. Otherwise `$XDG_CACHE_HOME/axiom`, else `~/.cache/axiom`. |
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
