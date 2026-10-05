# 0111: Self-hosted x86-64 without BMI1: trailing zeros of a mask that cannot be zero still test for zero

- Type: feature (performance)
- Status: fixed (see below)
- Area: compiler (selfhost: x64)

Without `tzcnt`, `@ctz(x)` was `mov w, 32; bsf d, x; cmove d, w` everywhere. the benchmark's loops
over the set bits of a mask (`while m != 0 { i = @ctz(m); m &= m - 1; ... }`: base
inputs, mobility, the move generator) never see zero there, but `nonzero_at` only
recognized a block reached through a branch on `x != 0`, and the loop's mask is a phi of
the value tested before the loop and the one tested at its (rotated) end. 90 `bsf` in the
`--cpu=x86-64` bench_search had the `cmove`, LLVM's none (it emits `rep bsf` alone).

Fix: `nonzero_phi`: a phi is nonzero where it dominates if each incoming value is a nonzero
constant or comes along the nonzero side of a branch testing it against zero; and a `ctz`
of a value narrower than 32 bits (with a bit set above it) needs no test either. 3 of the
90 remain.
