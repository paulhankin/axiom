# 0095: Self-hosted x86-64: code alignment (functions, loop headers, the JCC erratum)

- Type: feature (performance)
- Status: fixed (COMMITS): the knobs and the measurement are in; none is on by default, as none measurably helps (below)
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

## Outcome

The self-hosted backend now has (selfhost/src/x64.ax, read from the environment and
part of the object cache's key):

- `AXIOM_X64_FPAD=n`: n bytes in front of every function, for item 1. Building one
  program with several n and reporting per kernel the median, minimum and maximum over
  the builds of each build's best time separates code changes from placement luck.
- `AXIOM_X64_FALIGN=a` (default 16): function alignment.
- `AXIOM_X64_LALIGN=a`, `_LMAX`, `_LMODE`: hot loop tops aligned to a bytes with at
  most `_LMAX` bytes of nops; mode 1 aligns the targets of jumps back, mode 2 any
  block in a loop, run at least a fifth as often as its header, that its layout
  predecessor seldom falls into (LLVM's rule).
- `AXIOM_X64_JCC=1`: no jcc (with the cmp/test/add/sub/and it fuses with), jmp, call,
  ret or memmove loop jump crosses or ends on a 32-byte boundary (nops in front).
  `.text` is aligned to 64 bytes so that the encoder knows where each instruction will
  sit, and branch relaxation now gives a rel32 back to a rel8 jump that padding left
  out of reach (`unrelax_jumps`) and encodes again.

Measured on a Xeon Cascade Lake (AVX-512, the benchmark 1d00a0e), 6-8 placements
(`AXIOM_X64_FPAD` 0..58) per variant, median over placements of each placement's
best time; the machine is shared, so differences under ~2% are noise:

| | defaults | JCC=1 | LALIGN=16 LMODE=2 | LALIGN=32 | FALIGN=32 | LLVM | LLVM + `-x86-branches-within-32B-boundaries` |
|---|---|---|---|---|---|---|---|
| kernels: movegen (ns) | 802 | 823 | 806 | 809 | (830 mean) | 904 | 907 |
| contact_net | 382 | 382 | 385 | 386 | | 374 | 365 |
| pruned_contact | 159 | 160 | 158 | 154 | | 173 | 171 |
| contact_single | 433 | 434 | 434 | 434 | | 398 | 398 |
| bench_search 1ply (ms) | 30.1 | 30.0 | 30.2 | | 30.0 | 29.2 | 29.5 |
| 2ply | 121.5 | 122.0 | 118.2 | | 117.6 | 139.7 | 120.7 |
| 3ply | 739 | 749 | 740 | | 741 | 735 | 733 |
| gbr | 965 | 1002 | 963 | 964 | | 936 | |

No knob is faster beyond the noise; the JCC padding is slower by 1-4% (the nops cost
more than they save). The spread over placements stays about the same with loop or
function alignment (2ply: 116-137 by default, 116-120 with them, but the minima are
equal), so it does not come from loop tops: it is more likely the placement of data
or of whole functions relative to each other. A microbenchmark (a 77-byte loop ending
in `dec; jnz`, started at each of the 32 offsets in a 64-byte line,
scratch `jt/`) shows no reproducible penalty at the offsets where the fused jump
crosses a 32-byte boundary (about 1.02-1.1 ns per iteration at every offset in the
best of three runs): on this machine (a VM) the erratum's microcode penalty is not
measurable, so padding for it buys nothing here. Defaults are therefore unchanged
(functions at 16 bytes, nothing inside one), and nothing is made CPU-specific.

Item 3: the LLVM-backed C compiler built with `-x86-branches-within-32B-boundaries`
(through `LLVMParseCommandLineOptions`) measured the same as without it on the kernels
and on bench_search, so it does not pass it.
