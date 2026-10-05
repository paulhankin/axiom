# 0095: Self-hosted x86-64: code alignment (functions, loop headers, the JCC erratum)

- Type: feature (performance)
- Status: in progress
- Area: compiler (selfhost: x64, x64enc)

Item 1 of 0094. The self-hosted x86-64 backend aligns each function to 16 bytes and
nothing inside one. Padding every function's start by 1, 16, 32 or 48 bytes moves the benchmark's
`pruned_contact` kernel (the benchmark `axiom/src/bin/bench_kernels.ax`) between 136 and
154 ns, so kernel comparisons under about 10% are unreliable, and code placement probably
costs real time.

On Skylake through Cascade Lake and Comet Lake, Intel's microcode fix for the JCC
erratum keeps a jump (jcc, a macro-fused cmp/test/add/sub/and/inc/dec + jcc, jmp, call,
ret, indirect jumps) that crosses or ends on a 32-byte boundary out of the decoded-µop
cache: its 32-byte window is decoded by the legacy decoders every time. GNU as mitigates
this with `-mbranches-within-32B-boundaries` (prefixes or nops in front of such jumps);
LLVM has `-x86-branches-within-32B-boundaries` (off by default) and aligns functions to
16 bytes and loop headers to 16 bytes (MachineBlockPlacement's `alignBlocks`: a block in
a loop, not cold against its loop's header, that its layout predecessor does not fall
into often).

To do:

1. A layout-independent measurement: builds whose functions start at other places (a
   padding knob), timings reported as mean and minimum over them.
2. Function alignment (16 or 32 bytes), loop-header alignment for hot loops (with size
   limits, from the block frequencies and loop depths the register allocator already
   has), and JCC-erratum padding for the affected CPUs (by target CPU or a flag), each
   measured against the LLVM build and Rust on the kernels and on `bench_search`.
   Branch relaxation (`relax_jumps`) assumes distances only shrink, which padding
   breaks: it must allow for it.
3. Whether the C compiler (LLVM) should pass `-x86-branches-within-32B-boundaries` for
   those CPUs.
