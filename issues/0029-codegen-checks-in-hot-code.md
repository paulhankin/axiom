# 0029: Code generation: address offsets, stack checks and overflow checks in hot code

- Type: feature (performance)
- Status: open
- Area: compiler (C)

Found benchmarking the benchmark's Axiom port against its Rust original
(the benchmark `axiom/BENCHMARKS.md`): on wasm32 under V8 the vector kernels
run at Rust's speed, but scalar code is 1.3x to 3.3x slower, for these
reasons.

1. **Constant offsets are not folded into wasm loads and stores.** 20% of
   the module's memory instructions take `i32.const k; i32.add` before them,
   against 7% in Rust's build of the same code (e.g. `sum_25_checkers`
   loading `p+1`, `p+2`, ... each with its own add). wasm folds an offset
   into `offset=` only when the add cannot wrap; Rust marks element
   addresses `inbounds`/`nuw`. An index that has been bounds-checked cannot
   wrap, so element address arithmetic can carry `nuw`. (It would also make
   the porting advice "use a checked `+` on raw addresses so the offset
   folds" unnecessary; natively that checked `+` costs a compare and branch
   per offset.)
2. **Stack checks in leaf functions, kept after inlining.** Every function
   starts with a stack-overflow check, even a leaf with no frame
   (`is_side_crashed`, `sum_25_checkers`), and inlining keeps the callee's
   check, so a classification loop runs two checks per position, also with
   `--stack-traces=off`. Leaf functions without a frame can skip it; an
   inlined check can be merged into the caller's or hoisted out of loops.
3. **Overflow checks that cannot fail stop vectorization.** Summing 25
   `u8`s into a `u32` checks every add (25 add/compare/branch), so it is
   neither vectorized nor merged; Rust's build vectorizes it. The sum cannot
   exceed 25 × 255. Range analysis of reductions of widened narrow values
   over a known trip count (or emitting the add as `nuw` when the bound is
   provable) would remove the checks; with `+%` by hand the classification
   kernel goes from 18.4 to 13 ns (-30%).
