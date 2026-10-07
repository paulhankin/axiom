# 0045: No closures

- Type: feature (design)
- Status: open
- Area: language

Rust's evaluator sorts with closures (`sort_unstable_by(|a, b| ...)`), and its search and
rollouts take callbacks that capture state. Axiom has function pointers but no closures, so
the port (the benchmark `axiom/src/eval/rsort.ax`, search, rollouts) passes a function pointer
and a context pointer, and the sort's comparisons become indirect calls that LLVM cannot
inline (Rust's are monomorphised and inlined).

Proposal: at least comptime function parameters (`fn sort[T, cmp: fn(*T, *T) bool](...)`,
monomorphised so the comparison inlines), which covers sorting and most callbacks without
capturing; capturing closures could follow as a struct of state plus a function.
