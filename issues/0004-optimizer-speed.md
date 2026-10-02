# 0004: Faster optimizer

- Type: feature
- Status: open
- Area: compiler (selfhost): `passes.ax`, `gvn.ax`, `opt.ax`, `inline.ax`

The optimizer is about 6.5 s of an 8 s -O2 build of the self-hosted
compiler. Per-pass timings from an instrumented build (one run; the
machine was noisy):

| | ms | calls |
|---|---|---|
| re-optimizing functions after inlining (`resimplify`) | ~3,300 | 1,720 |
| `gvn.gvn` | ~2,500 | 10,335 |
| `cleanup` (fold/combine, CFG simplification, jump threading, if-conversion, to a fixed point) | ~1,900 | 16,014 |
| `sroa` | ~560 | 7,237 |
| `mem2reg` | ~450 | 7,237 |

Leads:

- `resimplify` runs all of `optimize_func` on a function after every
  inlining step; a lighter pipeline (or re-running only what the inlined
  code can change) may do.
- `cleanup` loops up to 8 rounds of every sub-pass; most rounds after the
  first likely change little.
- GVN rebuilds dominators and its tables per call; check what dominates
  inside it.

Any change must keep the bootstrap a fixed point and should be measured
against the generated code (size, benchmarks in `tests/bench`), not only
compile time.
