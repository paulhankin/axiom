# 0054: Self-hosted register allocator spills hot values and copies around its spill code

- Type: feature (performance)
- Status: fixed (3530ca4)
- Area: compiler (selfhost: mach, register allocation)

Part of the set 0053-0064 (the benchmark `bench_kernels`, selfhost against the C compiler's
LLVM code; see 0053 for the numbers). The biggest gap is movegen (about 1130 ns against
700 ns). Most of what is left after 0053 is spill code in `MoveGenerator.generate_moves_sub`
and `MoveGenerator.save_moves` (`src/core/moves.ax`).

## Today

Stack reloads and stores (`mov r,[rbp-x]` / `mov [rbp-x],r`) in the hot functions:

| function | selfhost loads/stores | LLVM loads/stores |
|---|---|---|
| `MoveGenerator.generate_moves_sub` | 107 / 25 | 24 / 11 |
| `MoveGenerator.save_moves` | 82 / 20 | 1 / 1 |
| `features.calculate_half_inputs_with_masks` | 77 / 37 | 4 / 33 |
| `simd_avx512.dim128_pair` output layer | 16 zmm reloads per output | 0 |

**1. Loop-carried values live in memory.** In `generate_moves_sub`, `mask` (the loop's own
induction variable, `while mask != 0 { i = 31 - clz(mask); mask ^= 1 << i; ...
recursive call ... }`) is spilled because it is live across the recursive call. Every
iteration goes through memory, and it is the function's hottest code (PC samples on the
left):

```
   41  mov    rdx,QWORD PTR [rbp-0xb8]     ; 1 << i, also spilled
   33  xor    ecx,edx
    9  mov    QWORD PTR [rbp-0xb0],rcx     ; store mask
   21  mov    rcx,QWORD PTR [rbp-0xb0]     ; and reload it at once (the loop header)
   35  test   ecx,ecx
       jne    loop
   16  mov    rcx,QWORD PTR [rbp-0xb0]     ; loop top: reload again
       lzcnt  ecx,ecx
```

LLVM keeps `mask` in a callee-saved register (`btc r15d,r12d; test r15d,r15d; jne`) and
spills only values that are cold around the call. The selfhost pushes all five
callee-saved registers but gives them to short-lived temporaries (`r14` holds a 0/1 `hit`
flag) while the loop variable goes to memory. The spill weight (`mach.ax` `weight`: uses
times 10^depth divided by the interval's length) penalises exactly the long, loop-spanning
intervals that are used every iteration. There is no live-range splitting: an interval is
either in one register for its whole life or reloaded at every use.

**2. Arguments spilled on entry, reloaded in the hot loop.** `save_moves` spills `self`,
`c_moves` and `c_pip` at entry. It then reloads `self` inside the dedup probe loop, which
has no calls (the calls are on cold paths after it):

```
  mov    rax,rdi
  mov    QWORD PTR [rbp-0x58],rax      ; self, through rax
  mov    rax,rsi
  mov    QWORD PTR [rbp-0x60],rax
  ...
loop:
  mov    esi,DWORD PTR [rcx+r12*4]
  ...
  mov    rdi,QWORD PTR [rbp-0x58]      ; self reloaded in the probe loop
  mov    rdi,QWORD PTR [rdi]
  cmp    rsi,QWORD PTR [rdi+0x10]
```

**3. Copies around spill code.** A spilled value's definition is copied into a temporary and
then stored, and a reload into a temporary is then copied to where it is needed (often
an argument register):

```
  mov    rax,rdi ; mov QWORD PTR [rbp-0x30],rax         (should be: mov [rbp-0x30],rdi)
  mov    rcx,QWORD PTR [rbp-0x30] ; mov rdi,rcx          (should be: mov rdi,[rbp-0x30])
```

Across the the benchmark functions there are 126 reload-then-copy pairs and 72 copy-then-spill
pairs; LLVM's code has 8 and 0.

**4. The 16 activations in `dim128_pair`'s output loop are reloaded every iteration**
(`vmovdqu64 zmm25,[rbp-0x400]` ... 16 per output). Eight weights, two accumulators and
16 activations fit in 32 zmm registers, but the sigmoid code before the loop holds many
registers with rematerialised constants (issue 0056), and the activations are spilled
for the whole loop.

## Proposal (mach.ax)

- **Spill weight.** Weight uses by block frequency (loop depth) without dividing by the
  whole interval length, or at least not by positions inside cold blocks. Prefer to keep
  values that are used inside the innermost loop around them.
- **Split intervals at calls and loop boundaries.** Before spilling an interval
  everywhere, split it: in a register inside the loop or the call-free region, spilled
  around the call or across the cold region, with moves at the split points. A
  second-chance or splitting linear scan (Wimmer & Franz) is the usual design.
- **Fold copies into spill code.** When the value comes from a copy of a physical register
  (an incoming argument, a call result), store that register directly. When the reload
  is only copied to a fixed register (an outgoing argument), reload straight into it.
  `rewrite_spills` and the coalescing hints can do this.
- **Callee-saved registers.** Prefer them for intervals that cross calls, and caller-saved
  registers for the rest, so that `push r14`/`r15` buy something.

## Impact

Hard to measure without the change. Of movegen's remaining gap after 0053 (about 300 ns),
spill traffic is the largest single part of `generate_moves_sub` (1.6x the samples of
LLVM's version) and `save_moves` (1.3x). Estimate: 10-20% of movegen (100-200 ns), about 5%
of `contact_inputs` and of the net kernels' output layers. Together that is about
150-250 ns summed over the kernels, the largest item after 0053.

## Fixed (3530ca4)

`mach.ax` now splits live ranges instead of reloading a spilled value at every use:

- **Splitting.** A spilled interval gets a stack slot and is split: across the calls
  it is live over and along loop entries/exits it is in the slot, and each connected
  stretch in between becomes a new vreg that competes again in the next round (so a
  value can be in a caller-saved register between calls and in a loop while it is in
  memory across them). Pieces that lose again are split at every block boundary
  (reloaded before their first use in the block, copied along edges between
  adjacent pieces, which coalesce through hints); only those are spilled everywhere.
  A region that defines the value stores it after each definition or where it leaves
  the region, whichever runs less often; stretches that never use it stay in memory,
  and after a call the value is reloaded where it is next used (in a successor), not
  in bulk after the call.
- **Spill weights by block frequency.** `block_freqs` estimates per-block frequencies
  on the machine CFG (natural loops x10 per entry, branches split evenly, loops
  rarely left, trap paths cold) instead of 10^depth; weights are uses x frequency per
  (size + 50) positions, and rematerializable values (0056) weigh half.
- **Copies folded into spill code.** A copy from a spilled value becomes a load into
  the copy's destination (`mov rdi,[slot]`), a copy to one a store of its source;
  dead reloads left by splitting are deleted.
- Target hooks: `spill_move`, `is_label`, and `MBlock.cold` (x64.ax, a64.ax).

Deterministic counts (ptrace single-stepping a fixed number of kernel calls, the benchmark
`bench_kernels`, against main before this commit):

| kernel (calls) | instructions | stack loads | stack stores |
|---|---|---|---|
| movegen (100 `generate_moves_into`) | 1361k -> 1261k (-7%) | 189k -> 109k (-43%) | 57.5k -> 44.6k (-22%) |
| `generate_moves_sub` alone | 925k -> 837k | 167k -> 96k | 42k -> 29k |
| `save_moves` alone | 252k -> 244k | 11.5k -> 6.1k | 7.7k -> 7.8k |
| contact_net (16 batches) | 1106k -> 1088k | 27.7k -> 7.5k | 6.9k -> 6.7k |
| pruned_single (400 evals) | 321k -> 319k | 5.6k -> 4.4k | 4.4k -> 4.4k |

`dim128_pair`'s output loop no longer reloads activations (item 4): its remaining
stack accesses run once per call. Timings (min of 15 on a loaded machine): movegen
992 -> 884 ns (LLVM 709), contact_net 320 -> 304 (LLVM 272), others within noise
(race_inputs moved 198 -> 214 with byte-identical `calculate_race_inputs`: code
placement). AArch64: the compiler built for aarch64 by itself, run under qemu,
produces the same x86-64 and aarch64 compilers. the benchmark results bit-identical.

Left: generate_moves_sub still has about 4x LLVM's stack loads. The loop keeps
~17 values live across its recursive call (more than LLVM's IR does), and the
in-loop pieces of the losers are reloaded per block; a register-pressure-aware
region split (LLVM's spill placement) or a priority (largest-first) allocator
would pick better. Critical edges into loop latches block per-edge splitting.
