# Contributing

Axiom is developed on `main` in this repository. Bugs and feature requests are GitHub
issues; a change that fixes or adds something a release's users should hear about gets
a note in `docs/next/` (see its README). Contributions are accepted under the
repository's licenses (MIT or Apache-2.0, at the user's option; see the README).

## What to install

| For | Ubuntu 24.04 packages | macOS |
|---|---|---|
| building the compiler from source | `clang` or `gcc`, `llvm-18-dev`, `lld` | `brew install llvm@18` (`llvm-config` on the `PATH`) |
| the test suite | `nodejs` (18 or later) | `brew install node` |
| the std fuzzer and the release script | `python3` | included |
| the Windows section of the suite (optional) | `wine64` | not run |
| the AArch64 section of the suite (optional) | `qemu-user`, `libc6-arm64-cross`, `gcc-aarch64-linux-gnu` | not run |
| the lowering tests (optional) | `binutils` (`objdump`) | `objdump` from `binutils` |

The compiler itself needs none of these once built: it has its own backends and linkers.

## The checks

```
cmd/axiom/bootstrap.sh                                   # build the compiler (about a minute)
node cmd/axiom/internal/tests/run.mjs                    # the suite with the bootstrap compiler
AXIOM=cmd/axiom/build/axiom node cmd/axiom/internal/tests/run.mjs   # with the compiler
cmd/axiom/difftest.sh                                    # both compilers agree (several suite runs)
cmd/axiom/examples.sh                                    # the examples build, and run where they can
```

`bootstrap.sh` builds the bootstrap compiler (C, LLVM) in `cmd/axiom/bootstrap/build/`,
then the compiler with it, then the compiler with itself, twice: stages 2 and 3 must
be byte-identical. Each suite run takes a few minutes on four cores; `difftest.sh`
checks diagnostics on every source file, runs the suite in several configurations and
fuzzes the backends against LLVM, and takes about ten minutes. Run the two suite
commands before pushing; run `difftest.sh` after a change to the front end, the
optimizer or a backend. `node cmd/axiom/internal/tests/run.mjs --filter=<text>` runs
the tests whose path contains `text`.

## The suite

`cmd/axiom/internal/tests/run.mjs` drives everything. Its options: `--native` (build
and run native code instead of WebAssembly), `-O2`, `--units=module|program`, `-g`,
`--filter=<text>`, `--list` (print the names of the tests that would run, without
running them) and `--verbose` (name the tests skipped, and why). `AXIOM_TEST_JOBS` sets
the number of tests run at a time.

| Directory | Convention |
|---|---|
| `run/*.ax` | Programs with an `export fn run()`; `// expect: <line>` comments give the output, in order, and `// expect-trap` means the run must trap. `// flags: <options>` adds compiler options; `// wasm-only` and `// native-only` restrict the target. |
| `errors/*.ax` | Programs that must fail to compile with every `// expect-error: E0nnn` code. |
| `lowering/*.ax` | Instruction selection: each `// lower: <flags> \| <regex> \| <count>` line builds the file with `--emit=asm` and counts the regex's matches. |
| `traces/`, `profile/`, `sanitize/`, `units/`, `packages/`, `testing/`, `fmt/` | Stack traces and crash reports, the profiler, the address sanitizer, separate compilation, packages and manifests, `axiom test` itself, canonical formatting. |
| `macos/`, `windows/` | Cross-builds for the other platforms; the binaries are inspected (and run under Wine where it is installed). |
| `tools/` | Generators of test data, the std fuzzer (`fuzz.py`) and the C-library benchmark. |

The suite also runs every `test` block in `std/`, builds the examples, and checks that
every `.ax` file is in canonical form (`axiom fmt --check`). A new language feature gets
a `run/` test and, for its diagnostics, an `errors/` test.

## Rules

- **Two compilers, one front end.** `cmd/axiom/` is the compiler, written in Axiom;
  `cmd/axiom/bootstrap/` is the bootstrap compiler in C, which exists only to build it.
  A change to the language (lexer, parser, type checker, diagnostics) goes into both,
  with identical diagnostic codes and messages, and `difftest.sh` checks that they
  agree. A change to code generation belongs only in the compiler.
- **The spec is the reference.** `docs/spec.md` describes the language; a language
  change updates it in the same commit, including the diagnostics table when a code is
  added.
- **Canonical form.** Every `.ax` file in the repository is formatted by `axiom fmt`;
  the suite fails otherwise.
- **Release notes.** A change users should hear about gets a short file in `docs/next/`
  (`docs/next/README.md` says how). `cmd/axiom/release.sh` gathers them into a release's
  notes (`docs/releasing.md`).
- **Ported code.** Code or data taken from another project needs a license that allows
  it, an entry in `THIRD-PARTY-NOTICES.md`, and must not be under a copyleft license
  (the AAC decoder, LGPL, is the one exception and is included only when imported).
- **Standard library conventions** are in `std/README.md` (Conventions): naming, errors,
  ownership of returned memory, `deinit`.
- **Scope.** Everything in the repository is about Axiom itself; describe measurements
  and code shapes in general terms rather than naming outside projects.

## Where things are

| Path | |
|---|---|
| `cmd/axiom/src/` | The compiler: `lexer.ax`, `parser.ax`, `sema.ax`, `codegen.ax` (to its IR), the backends (`wasm.ax`, `x64*.ax`, `a64*.ax`), the linkers, `main.ax` (the driver), `tester.ax` and `profiler.ax` (`axiom test` and `axiom profile`), `lsp.ax` and `ide.ax`. `cmd/axiom/README.md` describes the design. |
| `cmd/axiom/bootstrap/src/` | The bootstrap compiler, in C11 on LLVM. |
| `std/` | The standard library; `std/internal/runtime/` is the runtime (the host ABI for node, Linux, macOS and Windows). |
| `docs/` | The spec, this page's companions ([`cli.md`](docs/cli.md), [`tour.md`](docs/tour.md), [`editors.md`](docs/editors.md), [`install.md`](docs/install.md), [`releasing.md`](docs/releasing.md), [`security.md`](docs/security.md)). |
| `cmd/internal/tools/` | Generators of std's checked-in tables (Unicode, codecs, fonts); each says how it is run. |
| `tools/editors/` | Editor support (the VS Code extension). |
