# 0097: Self-hosted x86-64: selects of floats are branches, and threading turns `if c { x = k }` runs into branch trees

- Type: feature (performance)
- Status: fixed (0753e8d)
- Area: compiler (selfhost: x64, thread, passes)

(Found independently of 0096, filed at the same time: this is its `thread.ax` item and the
selects; the vectorization of the neighbouring stores stays in 0096.)

the benchmark's race inputs (the benchmark `axiom/src/eval/features.ax`, `calculate_race_inputs`) write
every point's four inputs unconditionally, as Rust does:

```
let mut one: f32 = 0.0;   (two, three, more likewise)
if nc == 1 { one = 1.0; }
if nc == 2 { two = 1.0; }
if nc >= 3 { three = 1.0; }
if nc > 3 { more = @float_from_int(f32, nc - 3) / 2.0; }
inputs[k] = one; ... inputs[k + 3] = more;
```

Two causes made this a tree of data-dependent branches (up to four per point, 30k taken
branches in 300 calls against LLVM's 8k; bench_kernels `race_inputs` 138 ns, LLVM 93):

1. **Jump threading ran before if-conversion** (`passes.ax` cleanup: fold, simplify, thread,
   if-convert). The merge after `if nc == 1` tests `nc == 2`, which the edge from the `then`
   arm decides, so threading jumped from each arm past the next test, and the merges became
   phis of constants with five inputs that if-conversion does not take.
2. **A select of floats was a branch around a move** (`x64.ax` `gen_select`), even with
   AVX-512, where LLVM uses a masked move (`kmov k1, cond; vmovss/vpblendmd x{k1}`).

Fix (0753e8d):
- x64: with AVX-512 (F, BW, VL) a scalar float select is the condition as 0/1 (`setcc`,
  or the materialized i1) into `k1` and `vpblendmd`/`vpblendmq` (no branch). Without it a
  branch is still what LLVM does too (SSE has no select on a flag).
- thread/passes: before threading, a first if-conversion (`if_convert_values`) converts the
  diamonds and triangles whose phis are all values: not conditions (i1 phis, which
  threading may decide the tests of), not vectors, and floats only where the target
  selects them without a branch (`tg.branchless_float_select`: WebAssembly, AArch64,
  x86-64 with AVX-512). The if-conversion after threading stays as it was. (Converting
  everything first made movegen and other kernels worse: an i1 phi became a value tested
  again instead of a threaded branch; and on SSE2 targets the race inputs went from 144 to
  164 ns, each select a branch.)

Test: `tests/run/float_selects.ax` (the race pattern, f32 and f64 selects on integer
and float compares with NaNs, a condition made in another block).
