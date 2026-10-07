# 0105: Self-hosted x86-64: what is left at 1-ply after 0101

- Type: feature (performance)
- Status: fixed in part (f38490c, 8e2c7b9, edc899b, ea687eb, 602f027, d74a1cf, fce0821); see "Progress" at the end for what is left
- Area: compiler (selfhost: gvn, mach, x64, slp)

After 0100 and 0101 (e3fee8f; 79f60f2: 4.99M), five 1-ply positions run 5.00M instructions in the
self-hosted build against 4.86M in the LLVM-built one (both counts include single-stepped
`rep` iterations: about 0.10M here, 0.13M there; so about 3.6% more). By function
(self-hosted / LLVM):

- 0-ply cross-roll glue, `find_best_moves_0ply_cross_roll_using` (with
  `evaluate_pruned_batch_prebucketed_into_with_scratch` inlined): 478k / 351k + 95k. The
  per-move loops reload spilled values at the latch on every iteration (`mov r8, [rbp -
  0x268]; mov rsi, [rbp - 0x290]; lea r10, [ALL_21_ROLLS_DICE]` in the `ev.push` loop:
  values live across the loop, reloaded inside it, 0094's "reloads in loops"), and
  `Vec.push` loads the length and capacity again after the grow check.
- `sanity_check_with_endgame-table`: 228k / 205k. After `if out[1] > out[0] { out[1] = out[0]
  }` the next test loads `out[1]` again (LLVM keeps a phi of the two values). A GVN
  forwarding of such loads through the join was tried: the register allocator then gave
  the phis registers of their own, with copies on the common (not taken) path, a split
  edge and a jump each, which cost more than the loads. Phi coalescing that prefers the
  input on the likely edge would be needed first.
- `calculate_contact_inputs` + `eval_batch_cross_roll_into_with_scratch`: 336k / 313k.
- Sorting (`insertion_sort`, `partition_lt` and the rest for `IdxScore`): 168k / 135k
  (164k since 79f60f2, which loads the f32 score as an integer where it is only bit-cast
  for its total-order key). `v[j - 1]` is bounds-checked although `1 <= j < len`;
  `tmp`'s index is spilled to the stack around the inner loop (callee-saved registers
  are free); storing the score from the integer register (`store(bitcast(x))` as
  `store(x)`) was tried and moved it to a second spill.
- `dim128_pair`'s hot loop (15k iterations): `mov r14, r13; shl r14, 9` where LLVM
  shifts in place after the two broadcasts that read the unshifted index (they come after
  the eight weight loads in the source; nothing schedules them first).
- Lane broadcasts of a vector (`splat(extract(v, k))`, now made by `splat_groups`):
  `vpshufd` and `vpbroadcastd` per lane where LLVM uses one `vpermps` with the index
  vectors hoisted into registers.
- Smaller: `calculate_half_inputs_with_masks` +14k, `generate_moves_sub` +9k,
  `DedupTable.append_into` +8k.

## Progress (f38490c..fce0821)

Seven changes, each checked by instruction counts (deterministic, five 1-ply positions after
the warm-up, as above):

- f38490c: loop phis' lower and upper bounds proved by induction (`v[j - 1]` in the sorts no
  longer bounds-checked), and loop exits split so reloads go on the way out: 4.954M -> 4.915M.
- 8e2c7b9: loads at joins (PRE: `out[1]` after `if out[1] > out[0] { out[1] = out[0] }`,
  also from lanes of a vector stored over the location, with the load moved onto the one
  path that does not know it); a parameter pointer never aliases the function's own stack
  slots: -> 4.910M, then (with the next) 4.882M.
- edc899b: the readers of a copied source (or of an lea's base) are moved in front of the
  copy so the source dies there (`idx << 9` in place in `dim128_pair`); copy coalescing
  prefers copies in blocks of nothing but copies (split edges).
- ea687eb: jump threading through a switch on a value known on the incoming edge.
- 602f027: bit-reversal zero tests, btc/bts, (2^n - 1) - y as an xor: -> 4.858M.
- d74a1cf: `c ? x + 1 : x` on a 0/1 register as an add (`partition_lt`): -9k.
- fce0821: AVX-512 lane broadcasts as one vpermd/vpermps by a hoisted index vector: -21k.

Now 4.821M instructions against LLVM's 4.852M (-2.7% from 4.954M; 0.6% fewer than LLVM). One
2-ply position: 1.050M against LLVM 1.054M (base 1.078M); SIMD off (`--cpu=x86-64`) 1.334M
against 1.338M (base 1.363M).

Interleaved min-of-N search timings (ibench, the benchmark 3023b3d; time / Rust's; base = 5194e30):

| mode | SIMD on: llvm | base | now | SIMD off: llvm | base | now |
|------|------|------|-----|------|------|-----|
| 0ply | .899 | .922 | .929 | .854 | .947 | .941 |
| 1ply | 1.027 | 1.048 | 1.028 | .981 | .997 | .991 |
| 2ply | 1.047 | 1.093 | 1.091 | .907 | .916 | .934 |
| 3ply | 1.063 | 1.087 | 1.084 | .958 | .998 | .952 |
| 4ply | 1.042 | 1.042 | 1.029 | .941 | .967 | .959 |
| gbr | .983 | 1.067 | 1.045 | .925 | 1.089 | 1.035 |
| gbr+ | 1.122 | 1.139 | 1.147 | .897 | .988 | .986 |
| gbr++ | 1.041 | 1.022 | 1.003 | .970 | 1.014 | 1.009 |
| rollout | 1.029 | 1.067 | 1.055 | .947 | 1.008 | 1.005 |

(single runs of the whole table still vary by a few percent; the 2-ply SIMD-off figure is
within that: its instruction count went down 2.1%.)

What is left: instruction counts are now at or below LLVM's at 1 and 2 ply, with and without
SIMD, yet 2-ply, 3-ply, gbr and gbr+ (and SIMD-off 0-ply and gbr) remain 2-11% slower than
the LLVM build. The gap is per-instruction cost, not instruction count: look at IPC
(branch mispredictions: the self-hosted code takes fewer branches but may mispredict more;
code and data layout; loads from the evaluation cache), with the PC sampler rather than
instruction counts. Kernels (bench_kernels, min of 9, ns): contact_single 408 vs LLVM 382
and pruned_single 165 vs 148 are the ones still behind; movegen, race_inputs and
pruned_contact are ahead.
