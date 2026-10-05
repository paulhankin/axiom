# 0091: C compiler: LLVM 18 vectorizes with gathers on Skylake-SP/Cascade Lake, where they are slow

- Type: feature (performance)
- Status: open
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
