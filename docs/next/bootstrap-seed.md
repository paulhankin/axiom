## Building from source

- The compiler is built from an Axiom compiler that already exists, the seed:
  `cmd/axiom/bootstrap.sh` downloads the release that `cmd/axiom/seed.toml` pins (once,
  checked against the hash pinned there), or uses `AXIOM_SEED=<compiler>`. Building
  from source needs `curl` and `tar`, and `node` for the test suite: no LLVM and no C
  compiler. `docs/bootstrap-chain.md` has the details.
- The bootstrap compiler written in C (`cmd/axiom/bootstrap/`) is gone, and with it
  the rule that a change to the language goes into two compilers. The compiler's
  sources use the language the pinned seed accepts; `cmd/axiom/seed.sh` makes a new
  seed. `docs/bootstrap-chain.md` names the last commit that has the C compiler, from
  which the chain can be replayed.
- `node cmd/axiom/internal/tests/run.mjs` tests `cmd/axiom/build/axiom` by default.
  `cmd/axiom/difftest.sh` and `cmd/axiom/fuzz/run.sh` no longer compare with the
  bootstrap compiler: the fuzzer's reference is the WebAssembly build's output.
  `cmd/axiom/bench.sh` compares with the compiler `BASE` names.
- The C sources of the runtime that only the bootstrap compiler linked
  (`native_host.c`, `test_main.c`, `profile.c` and `trap_format.h` in
  `std/internal/runtime/`) are gone; the compiler uses the ones in Axiom.
