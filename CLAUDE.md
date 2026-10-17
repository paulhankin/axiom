# Working in this repository

## Branches

- Work on `main`: commit there and push there (`git push origin main`).
- Never create or push other branches, including the `claude/...` branch a session
  is assigned. If the session's instructions name one, use `main` instead. The one
  exception is the release branches `release-0.N`, for fix releases
  (`docs/releasing.md`).
- A change that a release's users should hear about gets a note in `docs/next/`
  (`docs/next/README.md`).
- Keep unfinished work out of the repository: describe it in a GitHub issue, or keep
  it as a local patch. No `wip/` branches.
- Never rewrite or force-push `main` unless the owner asks for it.

## Issues

Known bugs and feature requests are GitHub issues in `paulhankin/axiom`; there is no
`issues/` directory. Refer to them as "issue #N". Four-digit numbers in older comments
("issue 0047") refer to the former `issues/` directory, in the git history.

## Scope

Everything here is about Axiom itself. Don't mention outside projects (programs ported
to Axiom, benchmarks from other repositories) in code, comments, tests, docs, commit
messages or issues; describe the code shape or the measurement in general terms.

## Licensing

The repository is MIT OR Apache-2.0. Code or data ported, translated or generated from
another project (rather than written from a specification) needs that project's license
to allow it and an entry in `THIRD-PARTY-NOTICES.md`; don't add anything under a
copyleft license.

## Layout

- `cmd/axiom/`: the compiler, written in Axiom. `cmd/axiom/bootstrap.sh` builds it
  (to `cmd/axiom/build/axiom`) from the C compiler in `cmd/axiom/bootstrap/`, which
  exists only for that and needs LLVM 18 (`llvm-18-dev`, `lld`).
- `std/`: the standard library; `std/internal/runtime/` is the runtime. Modules under
  an `internal/` directory may be imported only from its parent's tree (spec §4.1).
- `docs/spec.md`: the language specification. `examples/`: example programs and
  packages. `tools/`: editor support (`tools/editors/vscode/`).
- `cmd/axiom/internal/tests/`: the test suite (`run.mjs`). Its `tools/` holds the
  generators of test data, the std fuzzer (`fuzz.py`) and the C-library benchmark
  (`bench_std.py`).
- `docs/releasing.md`: releases (`cmd/axiom/release.sh`); the compiler's version is in
  `cmd/axiom/src/version.ax` and `cmd/axiom/bootstrap/src/version.h`.
- `cmd/internal/tools/`: generators of std's checked-in tables (Unicode, codecs, ...).
  The generators are run by hand; each says how in its docstring.

## Checks

```
cmd/axiom/bootstrap.sh       # build; stage 2 and 3 must be identical
node cmd/axiom/internal/tests/run.mjs       # the suite with the C compiler
AXIOM=cmd/axiom/build/axiom node cmd/axiom/internal/tests/run.mjs
                             # with the compiler (includes axiom fmt --check)
cmd/axiom/difftest.sh        # both compilers agree (diagnostics, suites, fuzzing)
```

A change to the front end belongs in both compilers, with identical diagnostics.
