# Issues

Known bugs and feature requests, one file per issue, kept in the repository so
they travel with the code they describe.

## Conventions

- File name: `NNNN-short-slug.md`, numbered in order of filing. Numbers are
  never reused.
- Each file starts with a small header:

  ```
  # NNNN: Title
  - Type: bug | feature
  - Status: open | in progress | fixed (commit) | won't fix
  - Area: compiler (selfhost) | compiler (C) | std/<module> | tools | ...
  ```

  followed by what happens, how to reproduce it (for bugs), what is known about
  the cause, and possible approaches.
- When an issue is fixed, set its status to `fixed (<commit>)` and move its row
  to the Closed table below; keep the file as a record.

## Open

| # | Type | Area | Title |
|---|------|------|-------|
| [0003](0003-parallel-o2-builds.md) | feature | compiler | Parallel -O2 builds |
| [0004](0004-optimizer-speed.md) | feature | compiler | Faster optimizer |
| [0005](0005-compiler-memory.md) | feature | compiler | Further cuts to the compiler's memory |
| [0008](0008-math-missing-commands.md) | feature | std/text/math | Missing TeX commands: `\xrightarrow`, `\substack`, `\sideset`, ... |
| [0009](0009-markdown-github-math-syntax.md) | feature | std/text/markdown | GitHub's `` $`...`$ `` math syntax |
| [0010](0010-typeset-without-math-table.md) | feature | std/text/math/typeset | Better typesetting with fonts that lack a MATH table |
| [0011](0011-macos-native-target.md) | feature | compiler | Native macOS binaries from the self-hosted compiler |
| [0012](0012-windows-native-target.md) | feature | compiler, runtime | Native Windows binaries (x86-64 done; runs on Windows 11) |
| [0018](0018-scalable-vectors-and-more-simd.md) | feature | language, compiler, std | SVE/SVE2, mask types, FP16/BF16 and the rest of wide SIMD |
| [0019](0019-faster-crypto-on-wide-vectors.md) | feature | std/crypto, std/net/tls | Faster cryptography with wide vectors and the remaining instructions |
| [0065](0065-c-compiler-scalable-vector-crash.md) | bug | compiler (C) | LLVM aborts with "Invalid size request on a scalable vector" on two widening shuffles at -O2 |
| [0086](0086-selfhost-x64-baseline-remaining.md) | feature | compiler (selfhost: x64, mach, inline), the benchmark port | Self-hosted x86-64 without SIMD extensions: what is left in the benchmark |
| [0087](0087-object-cache-eviction.md) | tooling | compiler, axiom cache | The module object cache is never evicted and grows without bound |
| [0094](0094-selfhost-after-0088.md) | feature | compiler (selfhost: x64, mach, gvn, slp) | Self-hosted x86-64: what is left after 0088 (code placement, reloads in loops, race inputs, bounds checks) |
| [0095](0095-selfhost-x64-code-alignment.md) | feature | compiler (selfhost: x64, x64enc) | Self-hosted x86-64: code alignment (functions, loop headers, the JCC erratum) |

## Closed

| # | Type | Area | Title |
|---|------|------|-------|
| [0001](0001-wasm-one-local-per-value.md) | bug | compiler: wasm backend | Every value gets its own wasm local; deep recursion exhausts the stack at -O2 |
| [0002](0002-o2-std-not-tested.md) | bug | tests | The test suite never builds `std` at -O2 |
| [0007](0007-profile-program-arguments.md) | feature | compiler: `axiom profile` | `axiom profile` cannot pass arguments to the program |
| [0006](0006-selfhost-on-wasm32.md) | bug | compiler | The self-hosted compiler does not build for wasm32 |
| [0013](0013-c-compiler-runtime-on-macos.md) | feature | runtime | The C compiler's native runtime does not build on macOS |
| [0014](0014-wasm64-host.md) | bug | runtime | wasm64 programs do not run under `runtime/host.mjs` |
| [0017](0017-windows-absolute-import-paths.md) | bug | compiler | An import of an absolute Windows path (`C:/...`) is taken as relative |
| [0015](0015-wide-simd-and-cpu-dispatch.md) | feature | language, compiler, std | AVX-512, SVE and wider vectors; conditional compilation; runtime CPU detection and dispatch |
| [0016](0016-native-crypto-instructions.md) | feature | compiler, std/crypto | AES-NI, PCLMULQDQ, SHA and CRC instructions (x86-64 and AArch64) used in std/crypto with portable fallbacks |
| [0020](0020-ice-literal-comparison.md) | bug | compiler (C) | Internal compiler error comparing two integer literals |
| [0021](0021-selfhost-vsplit-crash.md) | bug | compiler (selfhost) | Self-hosted compiler crashes splitting wide vectors (fill_phis) |
| [0022](0022-profile-program-arguments.md) | bug | tools, compiler (C) | `axiom profile` (C compiler) does not pass arguments to the program |
| [0023](0023-aligned-allocation.md) | feature | std/mem | Aligned allocation in std/mem |
| [0024](0024-lane-permutes.md) | feature | language, compiler (C and selfhost) | Runtime permutes of 16-, 32- and 64-bit lanes (and fast wide byte swizzles) |
| [0025](0025-variable-vector-shifts.md) | feature | language, compiler (C and selfhost) | Per-lane shift counts |
| [0026](0026-lanewise-bit-counts-and-conversions.md) | feature | language, compiler (C and selfhost) | Lane-wise @clz/@ctz/@popcount and int/float conversions on vectors |
| [0027](0027-partial-vector-loads.md) | feature | language, compiler (C and selfhost) | Loads and stores of a vector's first lanes (masked tails) |
| [0028](0028-reduce-add-order.md) | feature | language (spec §13.2), compiler (C and selfhost) | @reduce_add's fixed order: fold the halves |
| [0029](0029-codegen-checks-in-hot-code.md) | feature | compiler (C) | Code generation: address offsets, stack checks and overflow checks in hot code |
| [0030](0030-vector-float-to-int-scalarized.md) | bug | compiler (C) | Vector `@int_from_float` is scalarized on x86 |
| [0031](0031-select-and-constant-multiply-checks.md) | feature | compiler (C), language | `@select` through bitwise operations, checked multiplication by a constant through a division, no widening `@shuffle` |
| [0032](0032-forced-inlining.md) | feature | language, compiler (C and selfhost) | Forced inlining (`inline fn`) |
| [0033](0033-checked-multiply-on-native.md) | feature | compiler (C) | Checked 64-bit multiplication divides back natively |
| [0034](0034-debug-line-info.md) | feature | compiler (C), tools | Line tables (DWARF) for native profilers |
| [0035](0035-widen-narrow-and-bool-to-int.md) | bug, feature | compiler (C), language | `@widen_low`/`@widen_high`/`@narrow` unimplemented; no bool to integer |
| [0036](0036-dot-lowering.md) | bug | compiler (C) | `@dot` on `i16` lanes does not become `pmaddwd` / `i32x4.dot_i16x8_s` |
| [0037](0037-native-stack-trace-cost.md) | feature | compiler (C), runtime | Stack traces cost 15% in call-heavy native code |
| [0038](0038-pointer-offsets.md) | feature | language, compiler (C and selfhost) | Pointer offsets (`@ptr_add`): in-bounds element offsets for raw pointers |
| [0039](0039-selfhost-tls-sse-operand.md) | bug | compiler (selfhost) | Self-hosted x86-64 backend: "TLS operand in an SSE instruction" |
| [0040](0040-aggregate-values-compile-time.md) | bug | compiler (C) | Large aggregates as LLVM values make -O2 builds take minutes |
| [0041](0041-prefetch.md) | feature | language, compiler | No prefetch builtin |
| [0042](0042-u128.md) | feature | language, compiler | No 128-bit integers |
| [0043](0043-optional-pointers.md) | feature | language | No optional pointers or optional values |
| [0044](0044-global-initializers-and-thread-exit.md) | feature | language, std/thread | Restricted global initializers; no thread-exit cleanup for thread-locals |
| [0045](0045-closures.md) | feature | language | No closures |
| [0046](0046-selfhost-wide-vectors.md) | feature | compiler (selfhost) | Self-hosted x86-64 backend: 512-bit vector code is about 10x slower than LLVM's |
| [0047](0047-wrap-by-default.md) | language change | language, compilers, std | Integer overflow wraps by default, in every build mode |
| [0048](0048-constant-array-lengths.md) | feature | language, compilers | Array lengths cannot be constant expressions |
| [0049](0049-address-of-temporary.md) | feature | language | Cannot take the address of a temporary |
| [0050](0050-test-output.md) | feature | tools | `axiom test` hides what a test prints |
| [0051](0051-stack-overflow-tests-loop.md) | bug | tests | Stack-overflow tests turn into infinite loops at -O2 with wrapping arithmetic |
| [0052](0052-runtime-cpu-dispatch.md) | feature | language, compiler, std | No runtime CPU feature dispatch (filed in error: `@features`, `@multiversion`, `@cpu_has`, §13.4; fixed in a749803: inline helpers get a version's extensions) |
| [0053](0053-selfhost-native-shadow-stack.md) | feature | compiler (selfhost: codegen, inline, x64, a64, linkers), runtime | Self-hosted x86-64: calls still pay for the shadow stack (no frame tables) |
| [0054](0054-selfhost-regalloc-spills.md) | feature | compiler (selfhost: mach) | Self-hosted register allocator spills hot values and copies around its spill code (fixed in 3530ca4) |
| [0055](0055-selfhost-full-unrolling.md) | feature | compiler (selfhost: opt/passes) | Self-hosted optimizer does not unroll small constant-trip loops (fixed in 0dd4738, b18d0de) |
| [0056](0056-selfhost-vector-constants.md) | feature | compiler (selfhost: x64, opt) | Self-hosted x86-64: vector constants reloaded at every use, splats not broadcast |
| [0057](0057-selfhost-scalar-isel.md) | feature | compiler (selfhost: x64, x64enc, opt layout) | Self-hosted x86-64: scalar instruction selection leaves extensions, flag materialisation and copies (fixed in 0cbcfb6, 9e2bb29) |
| [0058](0058-selfhost-avx512-masks.md) | feature | compiler (selfhost: x64, combine) | Self-hosted x86-64: AVX-512 compares are expanded to vectors instead of used as k-masks |
| [0059](0059-selfhost-int-to-float.md) | feature | compiler (selfhost: x64, combine) | Self-hosted x86-64: int-to-float conversions go through the stack; float division by powers of two (fixed in 68528df) |
| [0060](0060-selfhost-shift-guards-and-ranges.md) | feature | compiler (selfhost: codegen, combine, gvn) | Self-hosted optimizer keeps shift guards and checks that value ranges rule out (fixed in bd1a230) |
| [0061](0061-selfhost-addressing-modes.md) | feature | compiler (selfhost: x64, gvn) | Self-hosted x86-64: addressing modes not folded, address arithmetic recomputed in loops (fixed in 0cbcfb6, 4b24f5d) |
| [0062](0062-selfhost-shuffles-and-reductions.md) | feature | compiler (selfhost: x64) | Self-hosted x86-64: constant shuffles as table permutes, lane extracts and reductions |
| [0063](0063-selfhost-correlated-branches.md) | feature | compiler (selfhost: thread) | Self-hosted optimizer: correlated branches not threaded, data-dependent table branches not if-converted (fixed in b18d0de, 360066b) |
| [0064](0064-selfhost-memset-and-zeroing.md) | feature | compiler (selfhost: x64, gvn) | Self-hosted x86-64: small constant-size memset/memcpy as `rep stosb`/`rep movsb`, arrays zeroed eight bytes at a time (fixed in 163832e, 4710d95) |
| [0066](0066-selfhost-regalloc-movegen.md) | feature | compiler (selfhost: mach, x64, gvn) | Self-hosted register allocator: reloads around calls in movegen's recursive loop (fixed in 50cbebd) |
| [0067](0067-selfhost-booleans-in-branches.md) | feature | compiler (selfhost: combine, thread, opt, x64, mach) | Self-hosted optimizer: booleans materialised as 0/1 and tested again (fixed in b5484e8, c35afe7, c77857d) |
| [0068](0068-selfhost-input-kernels-unrolling.md) | feature | compiler (selfhost: unroll) | Self-hosted optimizer: the benchmark's input encoders keep long point loops rolled (fixed in 95c7657) |
| [0069](0069-selfhost-input-kernels-remaining.md) | feature | compiler (selfhost: opt layout, x64, mach) | Self-hosted x86-64: what is left between the input encoders and LLVM's (fixed in 7133181, 62d2e71, f8011f3, f40fe96) |
| [0070](0070-selfhost-copies-around-reloads.md) | feature | compiler (selfhost: mach, x64) | Self-hosted register allocator: copies after reloads and rematerialized arguments, call arguments shuffled (fixed in d5130b0, 30d8bc2, 106658d) |
| [0071](0071-selfhost-half-inputs-remaining.md) | feature | compiler (selfhost: x64, combine, thread) | Self-hosted x86-64: what is left in the benchmark's contact half-inputs (fixed in d5130b0, 30d8bc2, 106658d) |
| [0078](0078-selfhost-half-inputs-idioms.md) | feature | compiler (selfhost: x64, combine, thread) | Self-hosted x86-64: idioms left in the benchmark's contact half-inputs (bswap, byte compares, kortest) (fixed in b42340e) |
| [0088](0088-selfhost-mask-registers.md) | feature | compiler (selfhost: mach, x64) | Self-hosted register allocator: mask registers and what is left after 0077 (fixed in part in 14c2504: rematerialized values split, phi operations sunk; the rest in 0094) |
| [0072](0072-selfhost-sse2-shuffles.md) | feature | compiler (selfhost: x64) | Self-hosted x86-64 without AVX: constant shuffles through memory (fixed in b2dccf2) |
| [0073](0073-selfhost-outlining-inline-helpers.md) | bug | compiler (selfhost: outline) | Self-hosted outliner: inline helpers' hot bodies outlined as cold functions (fixed in 4b49fd9) |
| [0074](0074-selfhost-piecewise-slots-and-slot-copies.md) | feature | compiler (selfhost: copies, x64) | Self-hosted optimizer: copies out of slots filled piece by piece, and whole-slot copies between locals (fixed in 7810756, 6e9b5ac) |
| [0075](0075-selfhost-tail-recursion.md) | feature | compiler (selfhost: passes) | Self-hosted optimizer: calls of a function itself in tail position stay calls (fixed in 50ef28c) |
| [0076](0076-selfhost-outlining-in-loops.md) | bug | compiler (selfhost: outline) | Self-hosted outliner: hot loop bodies outlined as cold functions (fixed in 9f11191) |
| [0077](0077-selfhost-regalloc-remaining.md) | feature | compiler (selfhost: mach, x64) | Self-hosted register allocator: what is left after 0070 (movegen, mask registers, loop splits) (fixed in part in 635bcd1: shrink-wrapping, global addresses hoisted; the rest in 0088) |
| [0079](0079-selfhost-sinking-lengthens-lives.md) | bug | compiler (selfhost: gvn) | Self-hosted optimizer: sinking makes operands live instead of results (fixed in d00826d) |
| [0080](0080-selfhost-ctlz-without-lzcnt.md) | feature | compiler (selfhost: x64, gvn) | Self-hosted x86-64 without LZCNT: k - ctlz(x) and the shifts it bounds (fixed in b16d1ee) |
| [0081](0081-selfhost-regalloc-split-gaps.md) | feature | compiler (selfhost: mach) | Self-hosted register allocator: values split at every block boundary (fixed in 56caaec) |
| [0082](0082-selfhost-if-conversion-or-tests.md) | feature | compiler (selfhost: thread) | Self-hosted if-conversion: stack-slot loads and `if (a or b)` arms (fixed in 3ca2d14) |
| [0083](0083-selfhost-x64-baseline-selection.md) | feature | compiler (selfhost: x64, x64enc) | Self-hosted x86-64: rel32 jumps, two-micro-op cmovs, and SSE2 details (fixed in 32f7a7f, d97dcc9) |
| [0084](0084-selfhost-padded-board-loads.md) | feature | compiler (selfhost: gvn, x64) | Self-hosted optimizer: loads of a buffer filled by memcpy, and vector and-not without AVX-512 (fixed in dbca74e) |
| [0085](0085-selfhost-offset-facts.md) | feature | compiler (selfhost: gvn) | Self-hosted optimizer: x - d checks after a test of k - x (fixed in 92ab39d) |
| [0089](0089-c-vector-float-to-u32-avx512.md) | feature | compiler (C) | Vector `@int_from_float` to `u32` is five instructions on AVX-512 where two do (fixed in 4c1d5fc) |
| [0090](0090-selfhost-conditional-table-reductions.md) | feature | compiler (selfhost: combine) | Reductions of table loads under a condition (the benchmark's Zobrist hash): no gathers (slow here); the select goes (fixed in bf39971) |
| [0091](0091-c-compiler-gathers-on-skylake-sp.md) | feature | compiler (C) | LLVM 18 vectorizes with gathers on Skylake-SP/Cascade Lake, where they are slow (fixed in cd8a2a4, c38fb46) |
| [0092](0092-selfhost-stack-slot-liveness.md) | feature | compiler (selfhost: slots, passes) | Self-hosted optimizer: stack-slot liveness (copies between slots, frames that share space) (fixed in bd0e213) |
| [0093](0093-prefetch-for-write.md) | feature | language, compiler | `@prefetch` cannot ask for a line to be written (Rust's the benchmark cache uses `_MM_HINT_ET0`): `@prefetch_write` |
