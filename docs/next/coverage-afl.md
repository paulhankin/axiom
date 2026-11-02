## Compiler

- `--coverage=afl` builds a native Linux program (`x86_64`, `aarch64`) for
  coverage-guided fuzzing: an edge counter at the start of every block, added after
  optimization, and a runtime that attaches the fuzzer's coverage map and serves its
  fork server (spec §16.9). Run outside a fuzzer, the program behaves as usual. The
  compiler itself can be built this way to fuzz `axiom check`.
