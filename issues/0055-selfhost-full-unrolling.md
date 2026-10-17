# 0055: Self-hosted optimizer does not unroll small constant-trip loops

- Type: feature (performance)
- Status: fixed (0dd4738, b18d0de)
- Area: compiler (selfhost: opt / passes, then combine and gvn)

Part of the set 0053-0064 (the benchmark `bench_kernels`, selfhost against LLVM; see 0053).

## Today

The selfhost keeps every loop rolled. LLVM fully unrolls short loops with constant trip
counts and then folds the induction variable: shifts by constants, constant
displacements, idioms. Examples from `src/eval/features.ax`:

**`half_containment`** (inlined into `calculate_half_inputs_with_masks`, run twice per
contact position):

```
for c in @as(u32, 0)..10 {
    let af = @as(usize, (blocked_mask_own >> (9 - c)) & 0xFFF);
    min_esc = @min(min_esc, ESCAPES[af]);
    min_upto[@as(usize, c) + 1] = min_esc;
}
```

selfhost (one iteration, rolled, 21 instructions plus a variable-shift guard, issue 0060):

```
  mov    esi,0x9 / sub esi,eax / cmp esi,0x20 / mov r8d,0x0 / mov r9,rsi / cmp esi,0x20
  cmovae r9d,r8d / mov r8,QWORD PTR [rbp-0x80] / shrx r8d,r8d,r9d / mov r9d,0x0
  cmp    esi,0x20 / cmovae r8d,r9d / and r8d,0xfff / mov esi,r8d
  lea    r8,[rip+ESCAPES] / mov esi,DWORD PTR [r8+rsi*4] / mov r8,rdx / cmp edx,esi
  cmova  r8d,esi / mov edx,eax / add rdx,0x1 / lea rsi,[rbp-0x78] / mov [rsi+rdx*4],r8d
  add    eax,0x1 / mov rdx,r8 / cmp eax,0xa / jb
```

LLVM (one of ten unrolled copies; the shift is a constant, and `& 0xFFF` times 4 folds
into one mask):

```
  mov    edx,esi / shr edx,0x5 / and edx,0x3ffc / mov edx,DWORD PTR [rdx+rax*1]
  cmp    r9d,edx / cmovae r9d,edx / mov DWORD PTR [rbp-0x54],r9d
```

**`calculate_race_inputs`**: LLVM turns the men-off loop
`for k in 0..14 { if men_off == k + 1 { inputs[OFF + k] = 1.0 } }` into one bounds test
and one indexed store. It unrolls the 3x6 crossover sum (and vectorizes it with
`vpsadbw`/`vpdpbusd`) and unrolls the 23-point loop by two. The selfhost runs the 14
iterations, the nested 3x6 loop and the 23-point loop as written.

The same applies to `base_inputs_with_masks` (24 iterations, 2 table copies each; LLVM
unrolls by 4), `dim128_pair`'s output loop (`for k in 0..5`, also duplicated by loop
splitting), `board16`'s `for k in 0..5` and `output_x8`'s `for k in 0..5`.

## Measured

Emulated by unrolling in a copy of the source and building with the selfhost (min of 9,
±5% noise):

| change | kernel | before | after |
|---|---|---|---|
| `half_containment` loop unrolled by hand | contact_inputs | 339 | 301-319 (-6 to -11%) |
| race men-off loop as one store, crossovers unrolled | race_inputs | 209 | 180 (-14%) |

## Proposal

A full-unroll pass for innermost loops whose trip count is a compile-time constant and
whose unrolled size stays under a budget (LLVM's default is about 300 instructions; trip
count at most about 32). It runs after inlining (`passes.ax`), and is followed by
combine, GVN and CFG simplification, so that:
- shifts and indices by the induction variable become constants (the shift guard
  disappears, and table indexes fold into displacements);
- stores to a local array at constant indices become SSA values for SROA (`min_upto`
  above);
- `if iv == x { store a[iv] }` over the full range becomes one guarded store (a separate
  small combine; LLVM gets it from unrolling, then if-conversion and store merging).

Partial unrolling (by 2-4) of hot counted loops with small bodies, as LLVM does for the
24-point and 23-point loops, is a smaller follow-up.

## Impact

Measured: -6 to -11% `contact_inputs` from one loop, -14% `race_inputs`. With
`pruning_inputs` (24 iterations), the output layers and the three 5-iteration loops,
about 60-90 ns summed over the kernels.

## Resolution

Fixed in 0dd4738 and b18d0de. `cmd/axiom/src/unroll.ax` unrolls, after inlining
(`passes.ax`, the last per-function phase, before loop splitting), every loop whose header
tests its induction variable against a constant: the trip count is found by simulating the
variable (start a constant on every way in, the same step on every back edge), and the loop
is copied that many times when the count times its size (instructions without phis, jumps,
address additions and extensions) is at most 300, LLVM's budget. Inner loops go first;
each copy folds its constants as it is made, so an inner loop whose start depends on the
outer variable (the 3x6 crossover sums) is unrolled next. Side exits (`break`, failed
checks) are copied with their phi inputs. The function is then optimized again (SROA takes
`min_upto` apart, the shifts become constants). Integer-to-float conversions of constants
now fold (`opt.ax`), which the unrolled `@float_from_int(f32, i)` needed.

The one-hot loop (`if men_off == k + 1 { inputs[OFF + k] = 1.0 }`) is, once unrolled and
threaded (0063), a chain of tests storing one value at addresses linear in the constant;
`unroll.onehot_stores` makes it a range check and one store.

Measured (bench_kernels, min of 41, against main at 51b2245, noisy machine): race_inputs
191 -> 147 ns (-23%), contact_inputs 280 -> 242 ns (-13%); the other kernels within noise.
bench_search results identical. The compiler's size, built from the same source, +0.5%; its
build time unchanged (the time jump threading took was halved, below).

Partial unrolling of the 23- and 24-point loops is left.
