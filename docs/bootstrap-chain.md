# The bootstrap chain

The compiler is written in Axiom, so building it takes an Axiom compiler: the *seed*.
`cmd/axiom/bootstrap.sh` compiles the compiler's sources with the seed (stage 1),
compiles them again with the result (stage 2) and once more (stage 3), and fails unless
stages 2 and 3 are the same bytes. Stage 2 does not depend on the seed: any seed that
compiles the sources correctly gives the same one.

## Where the seed comes from

`bootstrap.sh` takes the first of these that there is:

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
3. **The bootstrap compiler**, `cmd/axiom/bootstrap/`: a compiler for the language
   written in C, which uses LLVM 18. It is used while no file pins a release.

The pinned seed is `0.3.0-seed.1`: the compiler of commit `4fea9e94`, made by
`cmd/axiom/seed.sh` and published as a pre-release (which `axiom update` does not
offer). A seed is a release's compiler without the standard library, made between
releases when the compiler's sources need a newer language than the last release has.

## The rule a pinned seed brings

The compiler's sources (`cmd/axiom/src/`), and the standard library modules they import,
are written in the language the pinned release accepts. A new feature of the language can
be used in them once a release that has it is pinned. The build checks this by itself: a
seed that does not accept the sources fails at stage 1.

## The chain

Each release is built by `cmd/axiom/release.sh`, which runs `bootstrap.sh`. This table
records what seeded each one, so that any release can be rebuilt from the one before it,
back to the C compiler.

| Release | Commit | Seed |
|---|---|---|
| 0.1.0 | `3056b811` | the bootstrap compiler at that commit (C, LLVM 18) |
| 0.2.0 | `871db851` | the bootstrap compiler at that commit (C, LLVM 18) |
| 0.3.0-seed.1 (a seed) | `4fea9e94` | the bootstrap compiler at that commit (C, LLVM 18) |

The archives are deterministic (`docs/releasing.md`): rebuilding a release from its seed
gives the bytes its `SHA256SUMS` records.
