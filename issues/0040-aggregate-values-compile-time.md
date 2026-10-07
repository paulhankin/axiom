# 0040: Large aggregates as LLVM values make -O2 builds take minutes

- Type: bug (compile time, and code size)
- Status: fixed (4a3a354)
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

## Resolution

Fixed in the C compiler (codegen.c) as proposed. Aggregates larger than 16 bytes are never SSA values: such a value is the address of memory holding it, either a place (read before any other code runs) or a temporary (`agg.tmp`) that only the expression that made it can reach, or an LLVM constant. Assignments, `let`, `p.* = s`, by-value arguments and returns copy with `llvm.memcpy` (from a temporary) or `llvm.memmove` (from a place, which may overlap the destination); a zero constant is a `memset` and another constant a `memcpy` from a private global. Struct and array literals are built in place with GEPs and stores: directly in a new local, a result slot (sret, written before the defers run), an argument's temporary, or an assigned place when the literal reads only constants and locals whose address is never taken (otherwise in a temporary that is then copied, so `s = S.{ a: s.b, b: s.a }` still swaps). `[x; n]` evaluates `x` once and fills its destination in a loop (a `memset` when `x` is zero), so `[DEDUP_EMPTY; 512]` writes the table directly. Fields of rvalues, `try`/`catch` payloads and error codes are GEPs and loads; `catch` joins addresses.

The ABI is unchanged: aggregates over 32 bytes were already passed and returned by pointer (by_ptr), `export fn` and C functions included, and those of 17 to 32 bytes are loaded only at the call or `return` that passes them in registers. Aggregates of up to 16 bytes (slices, `Key128`) stay values.

Measured on the benchmark's `axiom/src/bin/bench_search.ax` (a snapshot of the tree, `-O2`, the 4-core machine shared with other builds):

| Build | Before | After |
|---|---|---|
| native `-O2` | 235.7 s | 7.6 s |
| wasm32 `-O2` | 325.0 s | 7.0 s |
| native `-O0` | 3.0 s | 2.6 s |
| bench_kernels native / wasm32 `-O2` | 4.2 s / 2.8 s | 3.5 s / 2.2 s |

`bench_search --mode 2ply --positions ../data/bench_suite_2ply_500.txt --limit 50` gives byte-identical `results` before and after, natively and on wasm32. Speed is unchanged within the noise of the shared machine: native median 2075 us/position before, 2085 after (15 runs each, interleaved; minimum 1745 and 1792); wasm32 5675 before, 5553 after (9 runs). bench_kernels' checksums are all unchanged and its kernels within noise (best of three: native contact_net 212.8 to 208.8 ns, movegen 701 to 667 ns; wasm32 contact_net 684 to 698 ns, contact_single 788 to 698 ns).

The test suite passes in every mode (wasm, `--native`, `-O2`, `-g`, `--units=module`), except three `--native -O2` tests that fail the same way on main without this change, since 0047 made overflow wrap: `forever(n + 1) + 1` and the traces' recursion no longer overflow the stack at -O2 (tests/traces/traces.ax "recursion", tests/traces/crash.ax, which now loops forever, and tests/testing/stack.ax "main thread overflow"). The self-hosted compiler reaches its fixed point and `selfhost/difftest.sh` passes.
