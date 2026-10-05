# 0091: C compiler: LLVM 18 vectorizes with gathers on Skylake-SP/Cascade Lake, where they are slow

- Type: feature (performance)
- Status: fixed (cd8a2a4, c38fb46)
- Area: compiler (C)

With `--target=native` the C compiler passes the host CPU (`LLVMGetHostCPUName()`,
`cascadelake` on the benchmark machine) and its features to LLVM 18. LLVM 18's
cost model takes gathers as fast on every AVX-512 CPU (`fast-gather`), so its loop
vectorizer turns the benchmark's Zobrist hash (`compute_board_hash`, see 0090) into twelve
masked `vpgatherdq`/`vpgatherqq` a call. On this machine a gather runs in microcode
(the Gather Data Sampling mitigation of Skylake-SP through Tiger Lake): 25-31 cycles
each, against about one cycle a scalar load.

Rust (LLVM 21, `target-cpu=native`) emits no gather there: newer LLVMs tune those
CPUs with `prefer-no-gather`. Building `bench_kernels` with the C compiler patched to
add `+prefer-no-gather` to the x86-64 features: movegen 936 -> 887 ns a position
(Rust 841, self-hosted 835; min of 9 runs). clang 18 shows the same: `-march=cascadelake`
gives 12 gathers for the hash, as does `-march=icelake-server`.

Approach: add `+prefer-no-gather` (and `+prefer-no-scatter`) for the CPUs newer LLVMs
tune that way (Skylake-SP through Tiger Lake; check LLVM's `X86.td` for the list) when the
CPU comes from the host or `--cpu`, where `codegen.c` makes the target machine and in
the per-function `target-features` attribute.

## Fixed

cd8a2a4, c38fb46 (`codegen.c`, `x86_tuning_features`): when the processor is
one of Skylake(-SP) to Tiger Lake (the host's name with `--target=native`, or
the `skylake`/`icelake-*` `--cpu` presets), `+prefer-no-gather,+prefer-no-scatter`
are added to the target machine's features and to every function's
`target-features` attribute. Axiom has no gather builtin, so only LLVM's
vectorizers are affected. Test: `tests/lowering/x64_no_gather.ax`.

On what LLVM does: no LLVM marks these processors `prefer-no-gather`, not even
Rust's LLVM 21 (the statement above is wrong). rustc 1.94 with
`-C target-cpu=cascadelake` still vectorizes an indexed loop
(`out[i] = t[idx[i]]`) with four gathers, and only `+prefer-no-gather` stops it;
it keeps the benchmark's hash scalar through its newer cost model, not through tuning.
So the list is ours: the processors with the Gather Data Sampling mitigation.

Measured (the benchmark bc2f24b `bench_kernels` movegen, ns a position, min of 9):
934.1 -> 888.2 (Rust 843.3). `bench_search` results are unchanged.
