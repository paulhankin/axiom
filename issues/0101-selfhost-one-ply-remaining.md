# 0101: Self-hosted x86-64: what is left at 1-ply against the LLVM-built port

- Type: feature (performance)
- Status: open
- Area: compiler (selfhost: combine, slp, x64, mach)

A matrix run had the self-hosted build's 1-ply search at 1.23x Rust's time and the
LLVM-backed build at 0.84x. It was one run on a loaded machine: interleaved runs
(the benchmark 1d00a0e, axiom 53203b9, this machine's Xeon, AVX-512, min of 12-30 runs) give
Rust 134 µs/position, LLVM-backed 145-148 (1.08-1.10x), self-hosted 150-152 (1.12-1.13x),
the same gap as at 2-ply. There is nothing specific to 1-ply; the self-hosted code runs
about 10% more instructions than LLVM's (5.34M against 4.86M for five positions after a
warm-up), spread over the kernels. 0097-0099 took some of it; what is left, by function
(instructions for five positions, self-hosted against LLVM):

- `pruned_boards16` (the pruning net's board deltas, `delta16`): 662k against 529k + 95k.
  Its loop over changed points is 46 instructions against 34:
  - `side = 1 if pos >= 25` then `pos - 25 * side` and `(side * 100 + point * 4) * 64`:
    `setae; imul r, 25; sub` and `imul r, 100; add; shl`, where LLVM selects
    (`lea r, [pos - 25]; cmp; cmovb`, and the base address `cmovb` between `hw` and
    `hw + 0x1900`). A multiply by a 0/1 value (`zext i1`) could be a select, and a
    subtraction or addition of a select of constants a select of the results.
  - `vn[k] - vo[k]` for k in 0..4 (two rows of `INPVEC`) is eight scalar loads and four
    subtractions; LLVM loads both rows as vectors, subtracts once and broadcasts each
    lane (`vbroadcastss`, `vpermps`). The SLP pass seeds on stores, not on lanes that go
    to splats.
- `dim128_pair`: its hot loop (15k iterations) is two instructions longer than LLVM's:
  `mov r13, r11; add r13, r14; mov r14, r13; shl r14, 9` where LLVM adds into the `tzcnt`
  result and shifts in place after the broadcasts that read the unshifted index
  (scheduling or a `lea`); a `vpxord` zero is rebuilt in the mask loop.
- `sanity_check`: 0100.
- `cross_roll_collect_best`: a `ScoredMove` built from a `Move` is a `rep movsb` of 175
  bytes (0092's copies of a slot into a field of another).
- Sorting: `insertion_sort` and `partition_lt` (158k) against LLVM's `select_nth_unstable`
  with them inlined (130k).
