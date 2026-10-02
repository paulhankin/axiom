# 0003: Parallel -O2 builds

- Type: feature
- Status: open
- Area: compiler (selfhost)

## Today

- At -O0/-O1, executables are compiled module by module in forked worker
  processes (`selfhost/src/units.ax`), up to one per CPU, with a cache of
  module objects: the self-hosted compiler builds in 1.7 s cold, 0.4 s warm.
- At -O2/-O3 the whole program is one unit (cross-module inlining), compiled
  by one process: about 8 s for the self-hosted compiler, of which the
  optimizer is about 6.5 s, machine code about 1.1 s.

## Tried

Machine code generation in parts: after whole-program optimization, fork
workers that each compile a fixed share of the functions (contiguous runs
balanced by instruction count; internal symbols become hidden globals
`name~index`; part 0 holds the data) and link the objects. It worked
(deterministic, bootstrap fixed point) but was no faster: the machine code is
only ~1.1 s, and forking a 400 MB process (page tables, copy-on-write faults
as the workers write to the IR) cost about as much as it saved. Reverted.

## Approaches

1. IR serialization: a compact binary form of a function's IR (with
   references to globals, functions and shared constants by index), so forked
   workers can optimize groups of functions and send them back. The
   per-function round of optimization and the loop passes parallelize
   directly; inlining could go level by level up the call graph.
2. Threads: the passes keep state in module globals (GVN alone has 17), and
   the language has no thread-locals by design (spec, rule 6), so each pass
   would take an explicit context. Shared tables (constants, types, interned
   strings) would need locks, and results must not depend on scheduling
   (constants are compared by pointer).
3. See also [0004](0004-optimizer-speed.md): a faster optimizer helps every
   build, parallel or not.
