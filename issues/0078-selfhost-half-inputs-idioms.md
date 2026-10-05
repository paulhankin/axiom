# 0078: Self-hosted x86-64: idioms left in the benchmark's contact half-inputs (bswap, byte compares, kortest)

- Type: feature (performance)
- Status: open
- Area: compiler (selfhost: x64, combine, thread)

Follow-up to 0071. After it, `contact_inputs` (300 positions, the benchmark
`axiom/src/bin/bench_kernels.ax`, counted by single-stepping) executes 963k instructions
against LLVM's 992k in all, but `calculate_half_inputs_with_masks` alone still runs 490k
against LLVM's 448k. The items of 0071 that are left:

1. **Bit reversal** (0071 item 2): `reverse24` written with five swap stages is 30
   instructions; LLVM uses `bswap` plus three stages. Needs an `ir.IN_BSWAP` intrinsic
   (also for `@byte_swap`) and recognition of the last two stages in combine.
2. **Escapes computed twice** (0071 item 3): `escapes_from_mask` and
   `escapes1_from_mask` both compute `escapes_index(mask, n)` under the same `n <= 0`
   test; jump threading does not merge the two diamonds.
3. **Byte compares not vectorized** (0071 item 4): the six `an_board[i] == 2` tests are a
   `sete`/`shl`/`or` chain where LLVM does one `vpcmpeqb` on a 4-byte load (SLP), and
   the race crossover sums could use `vpsadbw`.
4. **`@any_true`/`@all_true` of 512-bit vectors** (left from 0067) still go through
   `kmovq`, a compare and `setcc` where `kortest` and a jump would do.

Mask registers (0071 item 1) moved to 0077 with the rest of the register allocator.
