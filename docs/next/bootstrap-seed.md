## Building from source

- `cmd/axiom/bootstrap.sh` builds the compiler from an Axiom compiler that already
  exists, the seed: `AXIOM_SEED=<compiler>`, or the release that `cmd/axiom/seed.toml`
  pins (downloaded once and checked against the hash pinned there). The result is the
  same, byte for byte, as from the C bootstrap compiler, which is still the seed while
  no release is pinned. `docs/bootstrap-chain.md` has the details.
