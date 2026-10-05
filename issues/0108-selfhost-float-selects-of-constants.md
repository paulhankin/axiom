# 0108: Self-hosted x86-64 without AVX-512: floats chosen between two constants by branches

- Type: feature (performance)
- Status: fixed (1fb5609)
- Area: compiler (selfhost: thread, x64)

0097 made float selects AVX-512 blends; without AVX-512 they stay branches, and
`if_convert_values` (thread.ax) does not make selects of float phis at all
(`branchless_float_select()` is false), so jump threading then turns the benchmark's
`calculate_race_inputs` (`one = 1.0 if nc == 1`, `two`, `three`, `more`: one point after
another) into a tree of data-dependent branches to merges of constants. In the
`--cpu=x86-64` build race_inputs took 135 ns against LLVM's 114 (which vectorizes two of
the four and branches on the others).

Fix: a float phi whose two values are constants is if-converted like an integer one, and
`gen_select` picks between the constants' bits in general registers (`cmov`, with the
parity cases of OEQ / UNE) and moves the result over (`movd` / `movq`). The four values of
a point then become one vector compare and select (SLP).

Effect: race_inputs 137 -> 97 ns (LLVM 114, Rust 115).
