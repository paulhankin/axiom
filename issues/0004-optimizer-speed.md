# 0004: Faster optimizer

- Type: feature
- Status: in progress
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

## Progress

Part 1 (commit "Faster optimizer, part 1"), all with byte-identical output
except the cleanup schedule (output 0.005% smaller, same speed):

- `apply_forwarding` returns at once when no value has been forwarded
  since it last rewrote the function (a global forwarding epoch).
- `cleanup` stops once every sub-pass has run on the current IR without
  change, instead of always repeating a whole round; GVN is re-run only
  when the cleanup after it changed something (2,588 re-runs found
  something 131 times, always after such a change).
- `fold_combine` no longer counts uses every round: the one rewrite that
  reads them counts lazily.
- Inlining: callee cost and inlinability cached per call-graph node;
  function addresses in global initializers collected once (string
  literals are large constant aggregates); fewer calls per operand.
- CFG analyses: `compute_preds` dedupes in O(1); `simplify_cfg`,
  `jump_thread` and `if_convert` skip recomputing predecessors that are
  known to be current; `jump_thread` checks cheaply whether any edge can
  be threaded before computing dominators.
- GVN's hash buckets persist between calls (they are empty after a walk).
- `Value`'s hot fields share its first two cache lines.

Measured on the -O2 self-build: 24.9 G → 21.5 G instructions (valgrind),
about 7.4 s → 5.8 s wall. Use `axiom profile selfhost/src/main.ax -- build
...` (issue 0007), callgrind (self costs are reliable; its caller tree is
confused by tail jumps), or `valgrind --tool=lackey --basic-counts=yes` for
noise-free comparisons. Wall-clock timing on a shared machine varies ±5%.

Tried and rejected: a light pre-inlining pipeline (mem2reg, cleanup, SROA,
GVN) with the full one only after inlining, as LLVM does. -O2 builds got
~10% faster, but inlining decisions made on less-optimized callers changed
(call counts differ, so e.g. `util.arena_alloc` was no longer inlined) and
the generated compiler ran 5% more instructions.

Still open:

- The full pipeline after inlining (`resimplify`) is ~1/3 of -O2 time.
- `build_graph` walks every operand of the module six times per build
  (ipcp, inlining, dead-function removal, twice each); the cost is cache
  misses on operands.
- Each cleanup sub-pass recomputes predecessors and/or orderings; a CFG
  version counter would let them reuse analyses, but every CFG edit would
  have to bump it.
