# The bootstrap chain

The compiler is written in Axiom, so building it takes an Axiom compiler: the *seed*.
`cmd/axiom/bootstrap.sh` compiles the compiler's sources with the seed (stage 1),
compiles them again with the result (stage 2) and once more (stage 3), and fails unless
stages 2 and 3 are the same bytes. Stage 2 does not depend on the seed: any seed that
compiles the sources correctly gives the same one.

## Where the seed comes from

`bootstrap.sh` takes the first of these that there is, and fails if there is neither:

1. **`AXIOM_SEED`**, a compiler on this machine. It must accept the language the
   compiler's sources are written in, so it is a build of a recent commit or the pinned
   release. This is the way to build offline.
2. **The pinned release.** `cmd/axiom/seed.toml` names a release and the SHA-256 of its
   archive for each platform:

   ```toml
   [seed]
   version = "0.3.0-seed.1"
   linux-x86_64 = "<64 hex digits>"
   linux-aarch64 = "<64 hex digits>"
   macos-aarch64 = "<64 hex digits>"
   ```

   The script downloads the archive for this machine from the release's page
   (`AXIOM_UPDATE_URL` names a mirror, as it does for `axiom update`), refuses it unless
   its hash is the pinned one, and keeps it unpacked in the cache
   (`$AXIOM_CACHE/seed/`, by hash), so that later builds need no network. The hashes are
   those of the release's `SHA256SUMS`.

The pinned seed is `0.3.0-seed.1`: the compiler of commit `4fea9e94`, made by
`cmd/axiom/seed.sh` and published as a pre-release (which `axiom update` does not
offer). A seed is a release's compiler without the standard library, made between
releases when the compiler's sources need a newer language than the last release has.

Seeds exist for the platforms the compiler is built on: Linux on x86-64 and AArch64,
and macOS on Apple silicon. Another platform's compiler is built by cross-compiling
(`axiom build cmd/axiom/src/main.ax --target=...`) from one of these.

## The rule a pinned seed brings

The compiler's sources (`cmd/axiom/src/`), and the standard library modules they import,
are written in the language the pinned seed accepts. A new feature of the language can
be used in them once a seed that has it is pinned: the feature lands first, then
`cmd/axiom/seed.sh <0.N.M-seed.K> --publish` makes a seed of that commit and prints the
`seed.toml` that pins it, and then the sources may use the feature. A change of
meaning needs sources that are correct under both meanings until the new seed is
pinned. The build checks the rule by itself: a seed that does not accept the sources
fails at stage 1.

## The chain

Each release is built by `cmd/axiom/release.sh`, which runs `bootstrap.sh`. This table
records what seeded each one, so that any release can be rebuilt from the one before it,
back to the C compiler.

The first links were built by the *bootstrap compiler*: a compiler for the language
written in C (20,000 lines, on LLVM 18), which mirrored the front end and existed only
to build stage 1. It is no longer in the tree. The last commit that has it is
`8d7656f7`, in `cmd/axiom/bootstrap/`, where `make -C cmd/axiom/bootstrap` builds it and
`SEED_TOML=/nonexistent cmd/axiom/bootstrap.sh` builds the compiler from it (stage 2
does not depend on the seed, so that is the compiler the pinned seed gives there too).
So the chain can be replayed from C and LLVM: check out the commit of a row below
whose seed is the bootstrap compiler, build it there, and go down the table from it.

| Release | Commit | Seed |
|---|---|---|
| 0.1.0 | `3056b811` | the bootstrap compiler at that commit (C, LLVM 18) |
| 0.2.0 | `871db851` | the bootstrap compiler at that commit (C, LLVM 18) |
| 0.3.0-seed.1 (a seed) | `4fea9e94` | the bootstrap compiler at that commit (C, LLVM 18) |

The archives are deterministic (`docs/releasing.md`): rebuilding a release from its seed
gives the bytes its `SHA256SUMS` records.
