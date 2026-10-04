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
| [0060](0060-selfhost-shift-guards-and-ranges.md) | feature | compiler (selfhost: codegen, combine, gvn) | Self-hosted optimizer keeps shift guards and checks that value ranges rule out |
| [0061](0061-selfhost-addressing-modes.md) | feature | compiler (selfhost: x64, gvn) | Self-hosted x86-64: addressing modes not folded, address arithmetic recomputed in loops (fixed in 0cbcfb6, 4b24f5d) |
| [0062](0062-selfhost-shuffles-and-reductions.md) | feature | compiler (selfhost: x64) | Self-hosted x86-64: constant shuffles as table permutes, lane extracts and reductions |
| [0063](0063-selfhost-correlated-branches.md) | feature | compiler (selfhost: thread) | Self-hosted optimizer: correlated branches not threaded, data-dependent table branches not if-converted (fixed in b18d0de, 360066b) |
| [0064](0064-selfhost-memset-and-zeroing.md) | feature | compiler (selfhost: x64, gvn) | Self-hosted x86-64: small constant-size memset/memcpy as `rep stosb`/`rep movsb`, arrays zeroed eight bytes at a time (fixed in 163832e, 4710d95) |
