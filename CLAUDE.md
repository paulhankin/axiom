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
copyleft license. One recorded exception: the AAC decoder (`lib/audio/aac.ax` and
`lib/audio/internal/aac/`) keeps its license (LGPL), and a program includes it only by
importing it; nothing else under a copyleft license is added.

## Layout

- `cmd/axiom/`: the compiler, written in Axiom. `cmd/axiom/bootstrap.sh` builds it
  (to `cmd/axiom/build/axiom`) from a *seed*: the released compiler that
  `cmd/axiom/seed.toml` pins (downloaded once into `$AXIOM_CACHE/seed`), or the
  compiler `AXIOM_SEED` names (`docs/bootstrap-chain.md`).
- `std/`: the standard library; `std/internal/runtime/` is the runtime. Modules under
  an `internal/` directory may be imported only from its parent's tree (spec §4.1).
- `lib/`: the libraries (`import "lib/..."`), beside `std/`; `lib/c/` is the C front
  end's headers, C library and runtime.
- `docs/spec.md`: the language specification. `examples/`: example programs and
  packages. `tools/`: editor support (`tools/editors/vscode/`).
- `cmd/axiom/internal/tests/`: the test suite (`run.mjs`). Its `tools/` holds the
  generators of test data, the std fuzzer (`fuzz.py`), the allocator's fuzzer
  (`fuzz_alloc.py`) and the C-library benchmark (`bench_std.py`).
- `docs/releasing.md`: releases (`cmd/axiom/release.sh`); the compiler's version is in
  `cmd/axiom/src/version.ax`.
- `cmd/internal/tools/`: generators of std's checked-in tables (Unicode, codecs, ...).
  The generators are run by hand; each says how in its docstring.

## Checks

```
cmd/axiom/bootstrap.sh       # build; stage 2 and 3 must be identical
node cmd/axiom/internal/tests/run.mjs       # the suite (includes axiom fmt --check)
cmd/axiom/difftest.sh        # the suite in its configurations, and the backends' fuzzer
```

The compiler's sources (`cmd/axiom/src/`) and the std modules they import use only the
language the pinned seed accepts. A newer feature needs a new seed first, made with
`cmd/axiom/seed.sh` and pinned in `cmd/axiom/seed.toml`.
