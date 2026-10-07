# 0041: No prefetch builtin

- Type: feature
- Status: fixed (4a3a354)
- Area: language, compiler (C and selfhost)

Rust's the benchmark evaluator prefetches the position cache's bucket for each board of a batch
before evaluating the batch (`core::arch::x86_64::_mm_prefetch`, `prefetch_board` in
the benchmark-eval's cache.rs), so the lookups that follow hit L1. Axiom has no way to say so; the
port's `prefetch_board` is empty (the benchmark `axiom/src/eval/cache.ax`).

Proposal: `@prefetch(p)` (and perhaps `@prefetch(p, .read|.write, locality)`), a hint with
no semantics: `llvm.prefetch` natively (`prefetcht0`, `prfm`), nothing on wasm32 (which has
no prefetch). It never traps, whatever `p` is.

## Resolution

Implemented in both compilers and specified (§5.7, Appendix A, C44) in its simplest form: `@prefetch(p)`, with `p` any pointer (`*T` or `*mut T`, `*void` included), is a hint that the program will soon read `p`'s memory, with no other effect. `p` is evaluated (its side effects happen once) but nothing is read: whatever the address, it never traps. Errors: `E0822` for a non-pointer (an integer, a slice, an array), `E0420` for a wrong number of arguments, and its value is `void`.

Only the read, all-cache-levels form: it is the one the benchmark's `prefetch_board` uses (`_mm_prefetch(.., _MM_HINT_T0)`), and Rust's `prefetch_read_data(p, 3)`. Write and locality variants can be added as further arguments later without breaking this form.

C compiler: `llvm.prefetch(p, 0, 3, 1)`, which is `prefetcht0` on x86 (checked with `--emit=asm`) and `prfm pldl1keep` on Arm. On WebAssembly, which has no prefetch instruction, it emits nothing beyond evaluating `p`. Self-hosted: checked identically; it evaluates `p` and emits nothing (allowed by the spec).

tests/run/prefetch.ax prefetches table buckets for a batch of keys before looking them up, locals through `*` and `*mut`, `*void`, addresses outside memory, misaligned and one past the end, and checks `p` is evaluated once. tests/errors/prefetch.ax checks the errors.
