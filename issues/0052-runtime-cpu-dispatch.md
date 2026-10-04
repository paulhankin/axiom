# 0052: No runtime CPU feature dispatch

- Type: feature
- Status: open
- Area: language, compiler (C and selfhost), std

`@target_has("avx512f")` is a compile-time constant, so an Axiom program uses the vector
instructions its `--cpu` names and no others. A binary built for baseline x86-64
(`--cpu=x86-64`, what a distributed binary has to be) never runs AVX2 or AVX-512 code, even on
a machine that has it. Rust builds the same binary and still runs AVX-512 where the CPU has it:
`#[target_feature(enable = "avx512f")]` compiles one function for a feature the target does not
assume, and `is_x86_feature_detected!("avx512f")` (cpuid, cached) chooses it at run time.

Found in the benchmark's benchmark matrix (`benchmarks/matrix.py`, 2026-10-04 run). Built for
x86-64, Rust's the benchmark-eval detects AVX-512 at run time. The Axiom port uses SSE2. At 1-ply
through 4-ply the Axiom build is 2.4–2.9× slower than Rust's. With both built for the native
CPU, Axiom is 0.98–1.18×.

Proposal:

- `@target_feature("avx512f", "avx512bw") fn ...` (or a `target_feature` attribute) compiles
  the function as if those features were on: LLVM's `"target-features"` function attribute,
  and the selfhost backend's feature set for that function. Inside it, `@target_has` answers
  for the function's features. Calling it is allowed only where the caller has the features,
  or inside a `@cpu_has(...)` check, as Rust's `unsafe` call.
- `@cpu_has("avx512f")` reads cpuid once and caches the result (std/os on x86-64; on
  wasm32 it is the compile-time `@target_has` answer).
- The selfhost backend needs per-function ISA selection. Its AVX-512 lowering (0046) exists, so
  this is choosing the lowering per function rather than per build.
