# 0041: No prefetch builtin

- Type: feature
- Status: open
- Area: language, compiler (C and selfhost)

Rust's the benchmark evaluator prefetches the position cache's bucket for each board of a batch
before evaluating the batch (`core::arch::x86_64::_mm_prefetch`, `prefetch_board` in
the benchmark-eval's cache.rs), so the lookups that follow hit L1. Axiom has no way to say so; the
port's `prefetch_board` is empty (the benchmark `axiom/src/eval/cache.ax`).

Proposal: `@prefetch(p)` (and perhaps `@prefetch(p, .read|.write, locality)`), a hint with
no semantics: `llvm.prefetch` natively (`prefetcht0`, `prfm`), nothing on wasm32 (which has
no prefetch). It never traps, whatever `p` is.
