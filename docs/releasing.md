# Releases

Axiom is released as archives of the compiler and the standard library, one per
platform, in the way Go is. There is no compatibility promise: a release may change the
language, the standard library or the tools in ways that break programs, and its release
notes say how.

## Versions

Releases are numbered `0.N.M`. `N` counts releases with new features (and possible
breaking changes); `M` counts fix releases of `0.N` (security and critical fixes only).
The compiler knows its version (`axiom version`), kept in `cmd/axiom/src/version.ax`
and, for the bootstrap compiler, `cmd/axiom/bootstrap/src/version.h`. Between
releases, `main` carries the version of the next release.

A package says which compiler it needs in its `axiom.toml`:

```toml
[package]
name = "geometry"
axiom = "0.2"        # the oldest compiler that builds it
```

An older compiler refuses it (`E0220`). `axiom new` writes the current compiler's
`0.N`.

## What a release contains

For each of `linux-x86_64`, `linux-aarch64`, `macos-aarch64` and `windows-x86_64`, an
archive `axiom-<version>-<os>-<arch>.tar.gz` (`.zip` for Windows) holding:

```
axiom-<version>-<os>-<arch>/
  bin/axiom              (axiom.exe on Windows)
  std/                   the standard library's sources, the runtime among them, without test data
  LICENSE-MIT  LICENSE-APACHE  THIRD-PARTY-NOTICES.md
  RELEASE-NOTES.md  README.txt
```

The compiler finds `std/` next to its `bin/`. All four archives are built on one Linux
machine (the compiler cross-compiles itself), and they are deterministic: building
the same commit gives the same bytes, which `SHA256SUMS` records, so anyone can check a
release by building it.

## Release notes

Changes that a release's users should hear about (new features, anything that breaks
programs, notable fixes) get a note in `docs/next/` when they land: one small Markdown
file per change, named for it (see `docs/next/README.md`). At release time
`cmd/axiom/release.sh notes <version>` gathers them into `docs/releases/<version>.md`,
which is then edited into shape.

## Making a release

From a clean checkout of `main` (or, for a fix release, of `release-0.N`), with
`llvm-18-dev`, `lld`, `node` and `python3`:

1. Set the version in `cmd/axiom/src/version.ax` and
   `cmd/axiom/bootstrap/src/version.h`, run `cmd/axiom/release.sh notes <version>`,
   edit the notes, and commit.
2. `cmd/axiom/release.sh build <version>`: bootstraps, runs the test suite with both
   compilers, and writes the archives and `SHA256SUMS` to `build/release/`.
3. Tag the commit `v<version>` and push the tag.
4. Create a GitHub release for the tag with the notes, and attach the files in
   `build/release/`.
5. On `main`, set the version to the next one (`0.N+1.0`) and commit. For a new
   `0.N`, create the branch `release-0.N` from the tag.

## Fix releases

Fixes for a release are made on `main` first and then cherry-picked onto
`release-0.N`, from which `0.N.M` is released as above. Only the latest release gets
fixes (see `SECURITY.md`).
