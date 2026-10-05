# 0081: Self-hosted register allocator: values split at every block boundary

- Type: feature (performance)
- Status: fixed (56caaec)
- Area: compiler (selfhost: mach)

A value that loses its register is split at calls and loop boundaries; a piece
that loses again was split at every block boundary, each block's piece stored
after its definitions and reloaded at the start of the block. In a function of
many small blocks with more live values than registers (the benchmark's hitting shots:
a dozen shifted masks live throughout, plus each roll's counts) the values
that lost were often the hot ones: `n_chequers` of a roll, which coalescing had
joined with the long-lived `k0 >= 0` it starts from, went through its stack
slot between every two of the roll's blocks (`mov [rbp-0x100],rdx` /
`mov rdx,[rbp-0x100]` around each diamond).

Fix: a level between the two (`SPLIT_GAPS`) splits a value between the
stretches of code that use it: uses in blocks laid out in order, at most 16
instructions apart, form one cluster (with the blocks between them); edges
between different clusters are cut. Each cluster is a piece that can keep a
register across its blocks, reloaded where it is entered; a long gap between
uses is spent in memory.

Effect (the benchmark, `--cpu=x86-64 -O2`): bench_kernels `movegen` 939 -> 907
ns, `contact_inputs` 564 -> 559; static stack references in
`hitting_shots_not_on_bar` 638 -> 447.

0077's item 3 (values split at loop edges reloaded after every inner loop)
is about the first level of splitting; this is the last one.
