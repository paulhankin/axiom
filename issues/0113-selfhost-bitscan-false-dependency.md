# 0113: Self-hosted x86-64: bsf/bsr/tzcnt/lzcnt/popcnt wait for their destination's old value

- Type: feature (performance)
- Status: fixed (548b145)
- Area: compiler (selfhost: x64)

Found with 0086 (the benchmark's `axiom/` port built with `--cpu=x86-64`). `bsf` and `bsr`
leave the destination unchanged when the source is zero, so the processor makes them
read it; Intel cores of the Skylake family (Cascade Lake, the benchmark machine, among
them) give `tzcnt`, `lzcnt` and `popcnt` the same false dependency on the destination.
LLVM clears the destination first (`xor d, d`), the selfhost did not.

It matters in loops over the set bits of a mask (`while m != 0 { i = @ctz(m); m &= m - 1;
... }`). The register allocator often gives the scan's result the register of the
previous iteration's result, which is the end of a chain of loads and multiplies in
the benchmark's `calculate_half_inputs_with_masks`, so every iteration waited for the one before
and the loop ran at the latency of that chain rather than its throughput.

Fix (x64.ax `bitscan`): every one of these instructions is preceded by `mov d, 0` (a zero
idiom when it can be `xor`) and reads `d`, so `d` is also never the source's register.

Effect (`--cpu=x86-64`, kernels, ns per position): contact_inputs 462 -> 457 (LLVM 446),
movegen 819 -> 809.
