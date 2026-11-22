# Contributing

Axiom is developed on `main` in this repository. Bugs and feature requests are GitHub
issues; a change that fixes or adds something a release's users should hear about gets
a note in `docs/next/` (see its README). Contributions are accepted under the
repository's licenses (MIT or Apache-2.0, at the user's option; see the README).

## What to install

| For | Ubuntu 24.04 packages | macOS |
|---|---|---|
| building the compiler from source | `curl`, `tar` (they fetch the seed, once) | included |
| the test suite | `nodejs` (18 or later); `clang` or `gcc` for the native tests, which link C host functions | `brew install node` |
| the std fuzzer and the release script | `python3` | included |
| the Windows section of the suite (optional) | `wine64` | not run |
| the AArch64 section of the suite (optional) | `qemu-user`, `libc6-arm64-cross`, `gcc-aarch64-linux-gnu` | not run |
| the lowering tests (optional) | `binutils` (`objdump`) | `objdump` from `binutils` |

The compiler itself needs none of these once built: it has its own backends and linkers.

## The checks

```
cmd/axiom/bootstrap.sh                                   # build the compiler (about a minute)
node cmd/axiom/internal/tests/run.mjs                    # the suite
cmd/axiom/difftest.sh                                    # the suite in several configurations, and the fuzzer
cmd/axiom/examples.sh                                    # the examples build, and run where they can
```

`bootstrap.sh` builds the compiler with a *seed* (the released compiler that
`cmd/axiom/seed.toml` pins, downloaded once and checked against its pinned hash, or the
compiler `AXIOM_SEED` names), then the compiler with the result, twice: stages 2 and 3
must be byte-identical ([`docs/bootstrap-chain.md`](docs/bootstrap-chain.md)). A suite
run takes a few minutes on four cores; `difftest.sh` runs the suite in several
configurations and fuzzes the backends against each other, and takes longer. Run the
suite before pushing; run `difftest.sh` after a change to the front end, the optimizer
or a backend. `node cmd/axiom/internal/tests/run.mjs --filter=<text>` runs
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
| `lowering/*.ax` | Instruction selection: each `// lower: <flags> \| <regex> \| <count>` line builds the file with those flags and counts the regex's matches in its disassembly. |
| `traces/`, `profile/`, `sanitize/`, `units/`, `packages/`, `testing/`, `fmt/` | Stack traces and crash reports, the profiler, the address sanitizer, separate compilation, packages and manifests, `axiom test` itself, canonical formatting. |
| `macos/`, `windows/` | Cross-builds for the other platforms; the binaries are inspected (and run under Wine where it is installed). |
| `tools/` | Generators of test data, the std fuzzer (`fuzz.py`), the allocator's fuzzer (`fuzz_alloc.py`) and the C-library benchmark. |

The suite also runs every `test` block in `std/`, builds the examples, and checks that
every `.ax` file is in canonical form (`axiom fmt --check`). A new language feature gets
a `run/` test and, for its diagnostics, an `errors/` test.

## Rules

- **The compiler builds itself from a pinned seed.** `cmd/axiom/` is the compiler,
  written in Axiom, and `cmd/axiom/seed.toml` pins the released compiler that builds
  it. The compiler's sources (`cmd/axiom/src/`) and the standard library modules they
  import use only the language that seed accepts. A newer feature can be used there
  once a seed that has it is made (`cmd/axiom/seed.sh`) and pinned; the build checks
  the rule by itself, since a seed that does not accept the sources fails at stage 1.
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
| `cmd/axiom/seed.toml`, `cmd/axiom/seed.sh` | The seed `bootstrap.sh` builds from, and the script that makes one ([`docs/bootstrap-chain.md`](docs/bootstrap-chain.md)). |
| `std/` | The standard library; `std/internal/runtime/` is the runtime (the host ABI for node, Linux, macOS and Windows). |
| `docs/` | The spec, this page's companions ([`cli.md`](docs/cli.md), [`c.md`](docs/c.md), [`tour.md`](docs/tour.md), [`editors.md`](docs/editors.md), [`install.md`](docs/install.md), [`releasing.md`](docs/releasing.md), [`security.md`](docs/security.md)). |
| `cmd/internal/tools/` | Generators of std's checked-in tables (Unicode, codecs, fonts); each says how it is run. |
| `tools/editors/` | Editor support (the VS Code extension). |
