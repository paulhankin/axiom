# 0071: Self-hosted x86-64: what is left in the benchmark's contact half-inputs

- Type: feature (performance)
- Status: fixed (d5130b0, 30d8bc2, 106658d)
- Area: compiler (selfhost: x64, combine, thread)

Follow-up to 0069. After it, `race_inputs` and `pruning_inputs` run about as many
instructions as LLVM's code (and fewer taken branches); `contact_inputs` does not:
`calculate_half_inputs_with_masks` executes about 18% more instructions than LLVM's (523k
against 445k for 300 positions, counted by single-stepping; the benchmark
`axiom/src/bin/bench_kernels.ax`). By section of the function (instructions between the
stores of its outputs; LLVM's in brackets; measured before 0067's fixes):
hitting shots 177k (140k), point sums, timing and moment2 138k (113k), back
anchors 23k (14k), back escapes 14k (4k), the rest about equal. What the code shows:

1. **One mask register.** AVX-512 compares all go to `k1`, so where two masks are used by
   turns (the hitting-shot lanes: `vptestmd k1,zmm25,zmm25` / `vpaddd ..{k1}` /
   `vptestmd k1,zmm25,zmm26` / ... / `vptestmd k1,zmm25,zmm25` again) each is computed
   again before each use. LLVM keeps them in `k1`-`k4`. 0069 drops a compare into `k1`
   that repeats one with nothing between that writes `k1` (`reuse_k1`), which is only
   the simplest case; the rest needs mask registers in the register allocator (a fifth
   class, with spills to a GPR or memory).
2. **Bit reversal.** `reverse24` (`x.reverse_bits() >> 8` written with five swap
   stages) is 30 instructions; LLVM recognises the idiom and does `bswap` plus three
   stages (19). A bswap intrinsic (`ir.IN_BSWAP`, also for `@byte_swap`) and the
   recognition of the last two stages as one (combine) would do the same.
3. **Escapes computed twice.** `escapes_from_mask` and `escapes1_from_mask` share
   `escapes_index(mask, n)` under the same `n <= 0` test; LLVM computes the index once
   and loads both tables. The selfhost leaves two diamonds on `n <= 0` (the second is a
   correlated branch that threading does not take: the join between them stores and
   converts floats) and computes the index in each.
4. **Byte compares not vectorized.** The six `an_board[i] == 2` tests that build a mask
   (`sete` / `shl` / `or` chains, 25 instructions) are one `vpcmpeqb` on a 4-byte load
   plus a reduction in LLVM's code (SLP vectorization), and the crossover sums of
   `calculate_race_inputs` use `vpsadbw` and `vpdpbusd`. 0069 adds the terms of one
   weight first (`p*c + q*c = (p + q)*c`), which took those sums from 47 to 37
   instructions a side (LLVM: about 35); a byte-sum idiom (zero-extended adjacent bytes
   added: `vpsadbw` on a partial load) would take a few more.
5. **Reductions copy their low half.** `@reduce_add` of a zmm vector extracts the low
   256 and 128 bits with `vmovaps ymm0,ymm19` / `vmovaps xmm1,xmm0` copies (a shuffle
   to a narrower class, which coalescing cannot merge); LLVM reads the low half of the
   wider register directly. Two copies per reduction, six reductions a call.

## Resolution

Item 5 is done (a reduction reads the low half of the wide register, `low_alias`).
Besides it: equal values propagated through branches (gvn), vector loads assembled
from stored lanes and memsets sunk to the paths that use them, `insertps` and masked
broadcasts for inserts, embedded broadcasts in FMAs, `blsr`/`blsmsk`/`blsi`, integer
vector min/max under a mask, constants gathered outermost in sums and products. 300
`contact_inputs` calls went from 999k to 963k instructions (LLVM: 992k); the
half-inputs from 522k to 490k (448k). Items 2-4 moved to 0078, item 1 (mask registers)
to 0077.
