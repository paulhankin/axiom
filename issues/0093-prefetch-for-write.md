# 0093: `@prefetch` cannot ask for a line to be written

- Type: feature
- Status: open
- Area: language (§5.7), compiler (C and selfhost)

`@prefetch(p)` (issue 0041) is a hint that the program will soon read the line at `p`
(x86 `prefetcht0`, Arm `prfm pldl1keep`). the benchmark's evaluation cache prefetches each bucket of
a batch before it looks the bucket up, then locks it and may update it. Rust's engine asks for
those lines in a state ready for writing:

- on x86-64, `_mm_prefetch::<_MM_HINT_ET0>`, which is `prefetchw` where the CPU has PRFCHW;
- on AArch64, `prfm pstl1keep`.

(crates/the benchmark-eval/src/cache.rs `prefetch_for_write`.) Without this, a line fetched for
reading is shared, and the write that follows needs a second coherence transaction for
ownership. The port (the benchmark axiom/src/eval/cache.ax, `prefetch_board`) can only
prefetch for reading.

Proposal: an optional second argument, `@prefetch(p, .write)` (default `.read`), or a separate
builtin `@prefetch_write(p)`, whichever fits the builtin conventions better. Lowering:

- x86-64: `prefetchw` when the function's extensions include `prfchw` (add it to std/os/cpu's
  list if it is missing; Broadwell and later, and every AMD64 CPU, have it), else `prefetcht0`;
- AArch64: `prfm pstl1keep`;
- WebAssembly: nothing.

As for `@prefetch`, the hint never traps and has no other effect.
