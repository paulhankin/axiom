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
`8d7656f7` (the tag `bootstrap-final`), in `cmd/axiom/bootstrap/`, where `make -C cmd/axiom/bootstrap` builds it and
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

### The archives' hashes

The SHA-256 of each release's archives, as its `SHA256SUMS` records them:

| Release | Archive | SHA-256 |
|---|---|---|
| 0.1.0 | `axiom-0.1.0-linux-aarch64.tar.gz` | `a34a2f879b4b3cdb320d31d9a2339720f24ade6f39e64b2e317a323e71e9b5cb` |
| 0.1.0 | `axiom-0.1.0-linux-x86_64.tar.gz` | `fd79593fa4c235bf7c418a3713614181b7a3b0a93f7807c876221f4cbc5bffbf` |
| 0.1.0 | `axiom-0.1.0-macos-aarch64.tar.gz` | `a03feb17247f12e80dc6bb79969efd4cf0a8a5ffb23d2cb02ac8de526b397baf` |
| 0.1.0 | `axiom-0.1.0-windows-x86_64.zip` | `0fdc042de0d49474b8bbbb32b7ffe153f9c072da0a83b357067e8629dd3cc0a0` |
| 0.2.0 | `axiom-0.2.0-linux-aarch64.tar.gz` | `ab86cd1c165ae775f2eff6c2f2198b2201a002c84bf804e4b66f64e02534bddc` |
| 0.2.0 | `axiom-0.2.0-linux-x86_64.tar.gz` | `66fb2a67dfa7ac65ef46197fbeae60144a261cbd2ea510b2449fb0cb57eff6a1` |
| 0.2.0 | `axiom-0.2.0-macos-aarch64.tar.gz` | `09b7e66016e3b680d682c235b603e93001ce289f9cff0e551f9a40d5c1287cc0` |
| 0.2.0 | `axiom-0.2.0-windows-x86_64.zip` | `0f963b3af5f1ca26c8c78dff8ca0eb710ef0a300056d2ec8667daf35b0e2836e` |
| 0.3.0-seed.1 | `axiom-0.3.0-seed.1-linux-aarch64.tar.gz` | `d51f7ca8a5e44fef1396412891caa6e7a6920c1620cc55ebc617d236c600aae9` |
| 0.3.0-seed.1 | `axiom-0.3.0-seed.1-linux-x86_64.tar.gz` | `4a0394e1acdbc260692617d578806122d363d0a784c0788de4e2dd474cf25176` |
| 0.3.0-seed.1 | `axiom-0.3.0-seed.1-macos-aarch64.tar.gz` | `05953038c18c0f1ac719c9f145ccfec66c768a813ece5ee7c581fb9faf28a54e` |

The seed's are the ones `cmd/axiom/seed.toml` pins. A new release adds its rows here.

