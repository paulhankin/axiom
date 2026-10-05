# 0105: Self-hosted x86-64: what is left at 1-ply after 0101

- Type: feature (performance)
- Status: open
- Area: compiler (selfhost: gvn, mach, x64, slp)

After 0100 and 0101 (e3fee8f), five 1-ply positions run 5.00M instructions in the
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
- Sorting (`insertion_sort`, `partition_lt` and the rest for `IdxScore`): 168k / 135k.
  The f32 score is loaded into an xmm register and moved to a general register for its
  total-order key (LLVM loads it as an integer; the value is only bit-cast and stored);
  `v[j - 1]` is bounds-checked although `1 <= j < len`; `tmp`'s index goes through a
  stack slot.
- `dim128_pair`'s hot loop (15k iterations): `mov r14, r13; shl r14, 9` where LLVM
  shifts in place after the two broadcasts that read the unshifted index (they come after
  the eight weight loads in the source; nothing schedules them first).
- Lane broadcasts of a vector (`splat(extract(v, k))`, now made by `splat_groups`):
  `vpshufd` and `vpbroadcastd` per lane where LLVM uses one `vpermps` with the index
  vectors hoisted into registers.
- Smaller: `calculate_half_inputs_with_masks` +14k, `generate_moves_sub` +9k,
  `DedupTable.append_into` +8k.
