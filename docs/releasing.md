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

The compiler finds `std/` next to its `bin/`, and `axiom update` downloads the archives
by these names, and checks them against `SHA256SUMS`, so a release keeps them. All four archives are built on one Linux
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

One command does every step, from a clean, up-to-date checkout with `llvm-18-dev`,
`lld`, `node`, `python3`, push access and [`gh`](https://cli.github.com) logged in
(`gh auth login`):

```
cmd/axiom/release.sh release 0.N.0                 # from main
cmd/axiom/release.sh fix 0.N.M <commit>...         # a fix release: the commits, from main
```

Add `--dry-run` to stop after the local steps, to look at the notes and the archives
before anything is published; the same command without it then publishes what was
built. It asks before publishing, unless given `--yes`. `--draft` makes the GitHub
release a draft, and `--advisory <file.json>` also creates a draft security advisory
(the JSON body of GitHub's "create a repository security advisory" API). A step that
is done already is skipped, so after a failure (a cherry-pick that conflicts, a test
that fails) fix it, commit, and run the same command again.

These are its steps:

1. For a fix release, check out `release-0.N` (made from the tag `v0.N.(M-1)` if it
   does not exist yet) and cherry-pick the commits onto it (`git cherry-pick -x`).
2. Set the version in `cmd/axiom/src/version.ax` and
   `cmd/axiom/bootstrap/src/version.h`, run `cmd/axiom/release.sh notes <version>`,
   open the notes in `$EDITOR` (when there is a terminal), and commit them as
   "Axiom <version>".
3. `cmd/axiom/release.sh build <version>`: bootstraps, runs the test suite with both
   compilers and the standard library's fuzzer for five minutes (`docs/security.md`),
   builds every example for the targets it names and runs the ones that can run on
   the machine (`cmd/axiom/examples.sh`), and writes the archives and `SHA256SUMS` to
   `build/release/`.
4. Tag the commit `v<version>`, push the branch and the tag, and create a GitHub
   release for the tag with the notes and the files in `build/release/`.
5. For a new `0.N`, create the branch `release-0.N` from the tag, and set `main` to the
   next version (`0.N+1.0`). For a fix release, add its notes to `main`'s
   `docs/releases/`.

## Fix releases

Fixes for a release are made on `main` first and then cherry-picked onto
`release-0.N`, from which `0.N.M` is released (`cmd/axiom/release.sh fix`). Only the
latest release gets fixes (see `SECURITY.md`). A security fix also gets a GitHub
security advisory, published once the release is out.
