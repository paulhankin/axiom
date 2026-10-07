# 0102: C compiler: aggregates copied through stack slots whose reloads straddle the stores (store-forwarding stalls)

- Type: feature (performance)
- Status: open
- Area: compiler (C: codegen)

Where Rust's LLVM code copies a small aggregate from where it is to where it goes, the
LLVM-built Axiom code often builds it in a stack slot first and then copies the slot with
wider moves at other offsets than the ones that filled it. A load that needs bytes from two
or more pending stores cannot be forwarded from the store buffer and waits until they
retire (a store-forwarding stall, about a dozen cycles or more on this Xeon). Sampled in
the benchmark's port at 1-ply (`bench_search --mode 1ply`, ptrace PC sampler, 8 runs of 1000
positions, the benchmark 1d00a0e), three shapes:

1. **A value argument of an inline function.** `ev.push(&mut v, b.swapped())` and
   `ev.push(&mut v, w.buf[i])` (`push[T](v: *mut Vec.[T], x: T)` is `inline`; `T` is the
   50-byte `Board`, two `[25]u8`): the argument is stored to a slot (the halves as 16-byte
   moves at 0, 9, 25 and 34), the `reserve` call comes between, and `v.buf[v.len] = x` copies
   the slot as two 32-byte moves at 0 and 18: the load at 0 straddles three stores. Also
   `buf[i] = b.swapped()` in a loop. In `find_best_moves_0ply_cross_roll_using` these were
   the hottest instructions of the whole run (`vmovdqu ymm1, [rbp-0x11ee]` 1.55% of all
   samples, two more sites 0.54% and 0.24%). Rust writes the board in place
   (`Board::push_swapped`), and the port now does too (the benchmark `Board.write_swapped`,
   `ev.push_swapped`, `ev.push_copy`), which took 1-ply from 1.078x to 1.047x Rust's time
   (min of 15).
2. **A struct with a leading flag.** `out.best_moves[r] = ScoredMoveOpt.some(m)`
   (`{ present: bool, value: ScoredMove }`, `value` at offset 8, 168 bytes): the slot is
   filled at offset 8 (32-byte moves at 8, 40, 72, ...), `present` is stored to the
   destination as a constant, and the rest is copied from offset 1 (32-byte loads at 1, 33,
   ...), so every load straddles two stores (0.18%). The same code in Rust (`Some(..)`) has
   no slot.
3. **An array filled by scalar stores, reloaded as a vector.**
   `evaluate_pruned_batch_prebucketed_into_with_scratch`: `sanity_check(board, &mut probs)`
   stores the five probabilities one by one, then `[5]f32` is reloaded as one 16-byte load
   (0.37%).

The self-hosted backend splits such copies at the stores that filled the slot (0074,
`copies.ax`), so the self-hosted build has few of these. Possible approaches in the C
compiler: emit a by-value argument of an inlined call as a reference to its source when the
source is not written before the last use (or let LLVM see that by passing it as an
`alloca` copy only when needed); copy structs field by field (as `split_copies` does), or
copy the whole struct including the flag so the copy's offsets match the slot's; LLVM's
SROA/memcpyopt do not do this across the intervening call or the partial overlap.
