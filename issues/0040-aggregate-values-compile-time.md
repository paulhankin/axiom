# 0040: Large aggregates as LLVM values make -O2 builds take minutes

- Type: bug (compile time, and code size)
- Status: open
- Area: compiler (C)

## Today

The C compiler loads, stores and builds structs and arrays as first-class LLVM aggregate
values: `load [512 x i32]`, `insertvalue %DedupTable ...`, `store %CrossRollScratch`
(a struct of several `Vec`s, a 512-entry table and two more scratch structs). LLVM's
SelectionDAG expands each such value into one node per scalar and chains them, and its
combiner (`ReplaceAllUsesWith`, `SDNode::Profile`) is quadratic in that.

the benchmark's `axiom/src/bin/bench_search.ax` (the evaluator and search port, about 20k
lines) takes 3 min 57 s to build natively at `-O2`, single-threaded. A `perf` profile of
the compiler: 27% `SDNode::Profile` under `SelectionDAG::ReplaceAllUsesWith`, 14%
`SmallPtrSet` insertion, 4% `BaseIndexOffset::match`: all instruction selection on such
blocks. The unoptimized IR has 131 `load %Key128`, 92 `store %EvaluationResult`, 37 `store
%ScoredMove`, `load [512 x i32]`, `store %CrossRollScratch`, ... Unoptimized test builds
are slow for the same reason (the port's tests need longer than the 10 s default timeout).

## Proposal

As clang does: never treat an aggregate larger than a couple of registers as an SSA value.
- Copy aggregates (assignment, by-value arguments and returns, `p.* = s`) with
  `llvm.memcpy` between addresses (`memcpy` with a constant size is turned into the best
  sequence of loads and stores by LLVM's lowering, without the DAG blow-up).
- Build struct and array literals in place in their destination (GEPs and stores, or a
  `memset` for the zero parts) rather than an `insertvalue` chain stored at once.
- Pass and return large aggregates by pointer (sret / byval-like), which most ABIs do anyway.
- Keep small ones (up to 16 bytes, two registers) as values.

Measure: bench_search's -O2 build time before and after; it should drop to well under a
minute.
