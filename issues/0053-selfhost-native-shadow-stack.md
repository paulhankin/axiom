# 0053: Self-hosted x86-64: calls still pay for the shadow stack (no frame tables)

- Type: feature (performance)
- Status: fixed (ccd1257)
- Area: compiler (selfhost: codegen, inline, x64, a64, elink, mlink, plink), runtime

One of a set of issues (0053-0064) from comparing the self-hosted compiler's x86-64 AVX-512
code with the C compiler's (LLVM) on the benchmark's kernels (`axiom/src/bin/bench_kernels.ax`,
`--target=native -O2`, ns per position, min of 7 on a loaded 4-core AVX-512 machine, so
about ±5% noise):

| kernel | C compiler | selfhost | selfhost `--stack-traces=off` |
|---|---|---|---|
| movegen | 699 | 1131 | 1025 |
| contact_inputs | 184 | 338 | 325 |
| race_inputs | 101 | 209 | 209-221 |
| pruning_inputs | 17.7 | 38.8 | 37.9 |
| contact_net | 298 | 386 | 376 |
| race_net | 194 | 264 | 251 |
| pruned_contact | 91 | 109 | 109-112 |
| pruned_race | 115 | 154 | 151 |
| contact_single | 279 | 378 | 378-390 |
| pruned_single | 120 | 141 | 141-152 |
| pruned_boards | 53 | 76 | 76-83 |

How the issues were found: both binaries were sampled by PC (a ptrace sampler, since
`perf` does not work on this machine) and the hot functions compared instruction by
instruction. Where it was cheap, the effect was measured by rebuilding with a patched
copy of the compiler or an edited copy of the source. The set, ranked by estimated gain
over the kernels (ns summed over the 11 kernels; the gap summed is about 1,000 ns):

| # | issue | estimate |
|---|---|---|
| 0054 | register allocation: hot values spilled, copies around spills | 150-250 ns |
| 0053 | this issue: the shadow stack | about 150 ns (measured) |
| 0055 | no full unrolling of constant-trip loops | 60-90 ns (partly measured) |
| 0056 | vector constants reloaded per use, no broadcasts | 50-80 ns |
| 0057 | scalar instruction selection (extensions, setcc, RMW, copies) | 50-80 ns |
| 0058 | AVX-512 compares expanded instead of k-masks | 30-50 ns |
| 0060 | shift guards and range-provable checks | 30-50 ns |
| 0059 | `cvtsi2ss` through the stack, `/ 2.0` as a division | 20-40 ns (measured) |
| 0061 | addressing modes, loop-invariant address arithmetic | 20-40 ns |
| 0062 | shuffles as `vpermt2*`, lane extracts, reductions | 10-20 ns |
| 0063 | correlated branches, branchy table lookups | 10-20 ns |
| 0064 | small memset/memcpy as `rep` string operations | under 10 ns |

## Today

Issue 0037 replaced the shadow stack with frame-pointer walks and a return-address table in
the C compiler (x86-64 Linux). The self-hosted backend still records every call site in
thread-local storage. Every call in `src/core/moves.ax` `MoveGenerator.generate_moves_sub`
(recursive) does this:

```
  mov    edx,DWORD PTR fs:0xffffffffffffeaf0     ; once per function: the ring index
  mov    QWORD PTR [rbp-0x58],rdx                ;   kept in three stack slots
  and    edx,0xff / movsxd rdx,edx / shl rdx,0x3 / add rdx,0x8
  mov    QWORD PTR [rbp-0x60],rdx
  ...                                            ; then per call:
  lea    rax,[rip+0x3cc49]                       ; site record
  mov    rdx,QWORD PTR [rbp-0x60]
  mov    QWORD PTR fs:[rdx*1-0x1510],rax
  mov    rax,QWORD PTR [rbp-0x68]
  mov    DWORD PTR fs:0xffffffffffffeaf0,eax     ; depth + 1
  lea    rax,[rip+0x3cbf1]
  mov    QWORD PTR fs:[rdx*1-0x1510],rax         ; a second record store for the same slot
  call   generate_moves_sub
  mov    rcx,QWORD PTR [rbp-0x58]
  mov    DWORD PTR fs:0xffffffffffffeaf0,ecx     ; depth restored
```

The C compiler's code for the same function has no `fs:` access at all. The cost is the
TLS stores and the stack slots and registers held across the function: 43 `fs:` accesses
in `generate_moves_sub`, 71 in `save_moves`, 130 in `calculate_half_inputs_with_masks`.
Most of those last are on trap paths, but each inlined frame recomputes the ring offset
(`and 0xff; movsxd; shl 3; add 8`) at function entry.

## Proposal

Port 0037's design to the self-hosted x86-64 (and AArch64) backends:

- Keep frame pointers (the backend already sets up `rbp` frames). After each call, emit a
  label and a `{label, chain}` entry in an `axiom_rets` section; the chain lists the
  call's site record and those of the inline calls that led to it (the inliner knows the
  inline chain). `elink.ax` keeps the section, and `runtime/native.ax` walks the
  frame-pointer chain at a trap or crash, filling the thread's shadow-stack record the
  way the C runtime's `ax_stack_walk` does.
- Keep the shadow stack where 0037 kept it: `axiom profile` builds (the sampler reads it),
  `--sanitize=address`, and wasm.
- `tests/traces` must pass natively at every `-O` level, as for 0037.

A cheaper interim step: per call, store the record and depth once (the second record
store above is dead), and keep the ring offset in one slot instead of three.

## Impact

`--stack-traces=off` approximates the gain: movegen -9 to -12% (about 105-140 ns of its
430 ns gap) in three separate runs. The other kernels change by less than the noise
(0-5%), so about 150 ns summed over the kernels.

## Resolution

0037's design, in the self-hosted compiler for every native target: x86-64 and AArch64
Linux, `aarch64-macos` and `x86_64-windows` (spec §17.4.1, "Frame tables"; cmd/axiom/README.md).
Calls no longer touch the shadow stack:

- `codegen.ax` (`frame_tables`: native, traces on, not `axiom profile` and not
  `--sanitize=address`) gives each call it used to record a chain (`ir.Chain`, a field of the
  call instruction) holding its site record, instead of the stores. The inliner appends the
  inlined call's chain to the chains of the calls it copies (trap calls start with none), so
  after optimization each call's chain is its own record and then those of the calls inlined
  around it, innermost first: the inline frames of `inline fn`s and of whatever the
  optimizer inlined, traps in inlined code included. Jump threading, loop-range splitting
  and outlining copy the chain with the call.
- The backends (`ftab.ax`, used by `x64.ax` and `a64.ax`) write each distinct chain once
  (null-terminated, in `.data.rel.ro`) and, after each call that has one, an 8-byte entry in
  the section `axiom_rets`: the return address and the chain, both relative. Entries are
  in code order. The x86-64 backend still gives no frame to a leaf whose only calls are
  traps (they do not return): its calls' entries say so, and such functions are bracketed by
  marker entries, so a walk (or a crash inside one) finds their return address on the
  stack. AArch64 gives every function that calls a frame (a call clobbers the link register).
- `elink.ax`, `mlink.ax` and `plink.ax` place the objects' `axiom_rets` sections together, in
  the order of their code, between `__start_axiom_rets` and `__stop_axiom_rets` (as GNU ld
  does when C code is linked with the system's toolchain, whose C code is now compiled with
  `-fno-omit-frame-pointer`); `axiom_frame_table()` returns them.
- `axiom.trap` calls the runtime's `ax_stack_walk` with its own frame (`ir.IN_FRAME_ADDR`),
  and the crash handlers (`native.ax`, `test_main.ax`, `darwin.ax`, `windows.ax`) walk from
  the signal's or exception's context. `trap_format.ax`'s `walk_frames` follows the
  frame-pointer chain within the thread's stack, looks each return address up (binary
  search; the table is checked to be sorted on first use) and fills the thread's
  shadow-stack record (calls and depth) as the stores would have, so every report is made by
  unchanged code. Error-return traces keep using the record's ring.

`tests/traces` passes natively at `-O0` to `-O3` and with `--units=module` on x86-64 Linux,
on AArch64 Linux (under qemu) and on Windows (under Wine); the macOS build was only checked
statically (its table maps every return address to the instruction after a `bl`/`blr`).
Code generated for `generate_moves_sub` is now identical to the `--stack-traces=off` build.
the benchmark's `bench_search` results are bit-identical.

Measured (native, `-O2`; `bench_kernels` min of 11 per kernel, built by origin/main and by this
change, on a machine with a load average of 10-12 on 4 cores, so differences of several
percent are noise; checksums unchanged):

| | shadow stack | frame tables | `--stack-traces=off` | C compiler |
|---|---|---|---|---|
| recursive fib(38) | 383 ms | 223 ms | 222 ms | |
| movegen | 1166 ns | 1048 ns | 1052 ns | 744 ns |
| contact_inputs | 320 | 299 | 301 | 190 |
| race_inputs | 227 | 202 | 195 | 109 |
| contact_net | 424 | 386 | 426 | 278 |
| contact_single | 422 | 403 | 405 | 345 |
| all 12 kernels summed | 3368 | 3120 | 3175 | |

Executables are smaller than with the shadow stack (bench_kernels 577 -> 550 KB): the table
costs less than the stores did.

Not identical to the shadow stack, as with 0037: a fault that is not a check inside code the
optimizer inlined is reported in the function it was inlined into, and a crash in a
function's first instructions (before its frame is set up) misses that function's caller.

