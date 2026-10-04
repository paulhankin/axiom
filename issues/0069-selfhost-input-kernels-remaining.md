# 0069: Self-hosted x86-64: what is left between the input encoders and LLVM's

- Type: feature (performance)
- Status: open
- Area: compiler (selfhost: opt layout, x64, mach)

Follow-up to 0068. After it, `race_inputs` takes about 118 ns per position against
LLVM's 100, `contact_inputs` 192 against 155, `pruning_inputs` 21.7 against 17.5
(the benchmark `axiom/src/bin/bench_kernels.ax`). What the machine code still shows:

1. **Branch layout of equality tests.** In `calculate_race_inputs`' point loop
   (`if nc == 1 {..} if nc == 2 {..} if nc >= 3 {..} if nc > 3 {..}`), the selfhost
   lays each then-block out right after its test (reverse postorder), so an empty point,
   the common case, takes three taken branches; LLVM predicts `==` false and puts the
   stores out of line (one or two taken branches). Writing the stores branch-free took
   the selfhost's kernel from 135 to 91 ns and LLVM's from 105 to 64, so the branches
   are a third of either; how they are laid out is part of the remaining gap.
2. **Redundant compares**: `cmp r8b,0x3 / jb / mov [..] / cmp r8b,0x3 / jbe` — the
   second compare is the same as the first, with only a store between, but in the next
   block (the store's), and `reuse_flags` works within a block.
3. **Constants rematerialised per loop**: after partial unrolling and peeling, the
   constant 1.0 and 0.5 are loaded again in the loop's preheader although an outer value
   holds them (`vmovss xmm3, [1.0]` beside `xmm0`), and `mov r9d,0xf` before each
   `cmova` in `calculate_pruning_inputs`' unrolled loop instead of one register.
4. **Copies in unrolled straight-line code**: `mov rcx,rax / imul ecx,ecx,5` (three
   operand `imul ecx,eax,5`), `mov r9,rdx ... mov rdx,r9` around a `cmov` pair in
   `timing_home_board`.
5. **Byte sums**: sums of `zext(load u8)` times small constants over contiguous bytes
   (`calculate_race_inputs`' crossovers) are scalar chains; LLVM uses `vpsadbw` and
   `vpdpbusd`. Small in these kernels.
6. In `calculate_half_inputs_with_masks`, checks that value ranges rule out (`@int_cast`
   after `popcnt`/`lzcnt`, bounds checks after a `@min`): 0060.
