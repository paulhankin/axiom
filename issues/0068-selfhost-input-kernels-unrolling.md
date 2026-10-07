# 0068: Self-hosted optimizer: the benchmark's input encoders keep long point loops rolled

- Type: feature (performance)
- Status: fixed (95c7657)
- Area: compiler (selfhost: unroll)

Follow-up to 0055 and 0057 (the benchmark `axiom/src/bin/bench_kernels.ax`, the
`race_inputs`, `contact_inputs` and `pruning_inputs` kernels of
`src/eval/features.ax`). Before this, with the self-hosted compiler against the C compiler
(LLVM), ns per position (min of 9-15 runs on a loaded 4-core machine, about ±5%):

| kernel | selfhost | LLVM | Rust |
|---|---|---|---|
| race_inputs | 118 | 87 | 101 |
| contact_inputs | 190 | 143 | 160 |
| pruning_inputs | 21.5 | 15.5 | 18 |

## Causes

Experiments on a copy of `features.ax` built by both compilers (each change measured
alone) and the two compilers' code for the hot functions:

1. **Loops over the 23-25 points stay rolled.** 0055's full unrolling stops at 300
   instructions; LLVM unrolls these loops partially (`calculate_race_inputs`' point loop
   by 2, the pruning encoder's by 8). Unrolling the race point loop by two by hand took the
   selfhost's kernel from 136 to 126 ns, fully from 136 to 119; LLVM's was unchanged.
2. **A loop with a bounds or cast check is not unrolled when its result is used after
   it.** `timing_home_board` (6 iterations, `while idx > 0 { idx -= 1; ... }`, inlined
   into `calculate_half_inputs_with_masks` twice) has an `@int_cast` and an index check,
   whose failing sides leave the loop; unroll.ax took any such exit as one past which the
   loop's values may not be used. The rolled loop, with its checks, was about 10% of the
   samples in the half-inputs function.
3. **The men-off flags of `calculate_race_inputs`** (`for k in 0..14 { if men_off == k + 1
   { inputs[off + k] = 1.0 } }`) became 11 tests and stores, then one range check and
   indexed store for the last three (0063's one-hot stores): the scan for chains went in
   block order, which met the chain's tail first, and merging it left the head unmergeable.
   LLVM makes the whole chain one store.
4. **Dead `add`s** of offsets folded into addressing modes (`lea r11,[r9+1]` ahead of
   `movzx r11d,[rdi+r9+1]`): machine-level dead code removal kept anything that sets the
   flags.

Not causes:

- **Code alignment.** `race_inputs` and `contact_inputs` move by up to 10% between
  processes and with the code's placement, with the code byte-identical. Building the same
  program with the text section shifted by 0, 16, 32 and 48 bytes, with and without
  aligning loop tops to 16 bytes (as LLVM does), gave the same minimum over 25 runs per
  binary within ±2% (`contact_inputs` 181-186, `race_inputs` 99-104); the spread between
  single runs is the process's data placement and the machine's load, not alignment.
  Loop alignment was not adopted.
- **The crossover sum** of `calculate_race_inputs` (LLVM: `vpsadbw`/`vpdpbusd`; selfhost:
  18 loads, multiplies and adds): removing it changed the selfhost's kernel by less than
  the noise.
- **The rest of the half-inputs gap** is mostly checks that ranges rule out (0060) and
  isel/branch layout (0069).

## Resolution

Fixed in 95c7657 (`unroll.ax`):

- A loop with a constant trip count too large to unroll fully is unrolled partially: its
  body up to four times, within 120 instructions, the `n % factor` iterations left over
  peeled off in front; at most once per loop.
- Exits to blocks that only trap and use nothing from the loop but phi inputs do not stop
  the header's values being used after the loop.
- One-hot store chains are looked for in reverse postorder, so from their first test.
- (Cause 4, the dead `add`s, was fixed at the same time by 0066's `flags_dead_after` in
  `x64.ax`.)

Measured (ns per position, min of 15 runs in one session; base = origin/main with 0060's
and 0066's fixes, before this one; an earlier measurement against the tree before 0066
gave race 124 -> 99, contact 188 -> 168, pruning 21.5 -> 19.3):

| kernel | base | fixed | LLVM |
|---|---|---|---|
| race_inputs | 128.1 | 118.4 | 100.0 |
| contact_inputs | 196.6 | 192.2 | 155.2 |
| pruning_inputs | 23.8 | 21.7 | 17.5 |

(0060's fix took most of the checks out of the rolled `timing_home_board` loop, so its
unrolling gains less than it did before.) The other kernels are unchanged within the noise. `bench_search --mode 2ply` results are
bit-identical; code size grows by 0.5% (the benchmark's kernels) and 0.2% (the compiler). What is
left is in 0069.
