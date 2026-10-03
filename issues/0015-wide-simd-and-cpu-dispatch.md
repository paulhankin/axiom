# 0015: Wider SIMD (AVX-512, SVE, ...), conditional compilation and runtime CPU dispatch

- Type: feature
- Status: fixed (5f8febd, f67d888)
- Area: language, compiler (selfhost and C), std

## Today

- SIMD types are 128-bit only (`i8x16` ... `f64x2`, `v128`; spec §5.8, §13).
  They map to SSE/AVX on x86-64, NEON on AArch64 and simd128 on wasm.
- Processor extensions are chosen at build time (`--cpu`, `--features`;
  selfhost/README.md). x86-64 knows SSE3 to AVX2, FMA, BMI and the like. AArch64
  knows LSE, RCPC, DOTPROD and CSSC; NEON is its baseline. An extension only
  changes how the backend lowers an operation, never what the program can
  express.
- No AVX-512, no SVE, no 256- or 512-bit vector types.
- No conditional compilation: `comptime` is a reserved word only.
- No runtime CPU detection. `--cpu=native` reads `/proc/cpuinfo` on Linux at
  build time, and the result runs only on CPUs with those extensions.

## Wanted

1. **More instruction sets.**
   - x86-64: AVX-512 (F, BW, DQ, VL, then VNNI, BF16 and FP16), with mask
     registers and EVEX encoding, and the AVX10 levels.
   - AArch64: the rest of NEON (dot products are partly there; i8mm, bf16,
     fp16 arithmetic), SVE/SVE2 (scalable vectors; predicates), and SME later.
   - wasm: relaxed-simd is accepted but used little.
   - Each one needs new `--features` names and `--cpu` presets
     (`x86-64-v4`, `sapphirerapids`, `znver4`, `neoverse-v1`/`-v2`,
     `apple-m4`), plus instruction selection and encoding.
2. **Wider vector types.** 256-bit (`f32x8`, `i32x8`, ...) and 512-bit
   (`f32x16`, ...) types. Where the target lacks the width they are lowered
   to 128-bit halves, as `lower.ax` already splits operations a target
   cannot do. Mask types for AVX-512 and SVE predicates. SVE's scalable
   vectors probably need a separate type family, or a library of
   length-agnostic loops rather than fixed-width types.
3. **Conditional compilation.** A compile-time test of the target and its
   extensions, for example:
   - `if @target_has("avx512f") { ... } else { ... }`, with the dead branch
     never type-checked for the other target's intrinsics;
   - `@target_os()` and `@target_arch()` as constants (`os.system()` is the
     run-time answer today).

   It should fold before code generation, so that a 128-bit fallback costs
   nothing on a machine with the wide path. The spec would need to say what
   a branch may reference that does not exist on the other target.
4. **Runtime CPU detection and dispatch.**
   - A std API that reports the running CPU's features (`cpuid` and
     `xgetbv` on x86-64; `getauxval(AT_HWCAP)` on Linux,
     `sysctlbyname("hw.optional...")` on macOS and
     `IsProcessorFeaturePresent` on Windows for AArch64), exposed as a host
     ABI function so wasm hosts can answer too.
   - Multi-versioned functions: one source function compiled once per listed
     feature set, e.g. `@multiversion("avx512f", "avx2", "baseline")`, with
     a dispatcher that picks the best at first call (an IFUNC-like resolver
     the linkers support: an indirect call through a GOT or data slot filled
     at start-up).
   - A binary built for a baseline can then use AVX-512 where present. A
     binary built with `--cpu=x86-64-v4` should check at start-up and exit
     with a clear message on an older CPU, rather than dying from an illegal
     instruction.
5. **std.** Use the above where it pays: `std/crypto` (AES-NI and SHA
   extensions, VAES and VPCLMULQDQ for GCM), `std/hash` (CRC32 with CLMUL or
   the ARM CRC instructions), `std/compress`, `std/graphics` (pixel loops),
   `std/math/linalg` and FFT, `std/str` searching. The scalar or 128-bit
   paths stay as the fallback and are tested on every target.

## Notes

- Both compilers must agree on the language part (conditional compilation,
  the new types and builtins). The C compiler can lean on LLVM for
  selection; the self-hosted backends (`x64.ax`, `a64.ax`, `wasm.ax`) need
  encoders. EVEX is a new prefix scheme, and SVE a new register file and
  calling-convention rules.
- Testing: QEMU user mode emulates AVX-512 (`-cpu max`) and SVE
  (`qemu-aarch64 -cpu max,sve=on`), so the suite can run each feature level
  on any host (the cross tests already run under qemu). Each multi-versioned
  function should be tested at every level it has, not only the host's.
- Windows and macOS ABIs preserve different vector registers. Windows
  preserves `xmm6`-`xmm15` but not their upper halves; AVX-512's `zmm16`-31
  and `k` registers are volatile everywhere. Prologues and calls must
  respect that.

## Resolution

Done in 5f8febd (the language and both compilers) and f67d888 (x86-64
code generation at 256 and 512 bits):

- **Extensions.** The table in `std/os/cpu.ax` (mirrored in
  `compiler/src/target.c`) adds AES, PCLMUL, SHA, the AVX-512 family, GFNI,
  VAES, VPCLMULQDQ, AVX-VNNI, SHA512, SM3/SM4 and AVX10.1 on x86-64, and
  AES, SHA2/3/512, SM4, CRC, RDM, FP16, FHM, BF16, I8MM, SVE, SVE2, DIT and
  SME on AArch64, with implications and `--cpu` presets (x86-64-v4,
  Haswell to Granite Rapids, Zen 3-5, Neoverse N1/N2/V1/V2, Apple M1-M4).
- **Wider vectors** (spec §5.8, §13.3): `i8x32` ... `f64x4`, `v256`,
  `i8x64` ... `f64x8`, `v512`, with every 128-bit operator and builtin.
  The self-hosted compiler computes them in ymm/zmm registers where the
  function's extensions allow (VEX.L, EVEX, k1 for compare masks) and
  splits the rest down to 128 bits (`selfhost/src/vsplit.ax`); AArch64 and
  wasm split everything.
- **Conditional compilation** (spec §13.4): `@target_has`, `@target_arch`,
  `@target_os` are constants; an `if` on them compiles one branch, and the
  other is type-checked but may use builtins the code is not compiled for
  (E0828 anywhere else).
- **Run-time detection and dispatch**: `@cpu_has` (cpuid/xgetbv on
  x86-64; auxv or sysctl on AArch64 through the `ax_cpu_features` host
  function), `@features` (traps with T0013 on a processor without them)
  and `@multiversion` (one version per set plus the build's own, chosen at
  each call by the processor's features). A program built for more than
  the baseline checks the processor before `main`.
- **std**: `str.index_of_byte` (AVX-512BW, AVX2), and the cryptographic
  code of issue 0016.

Not done, now issue 0018: SVE/SVE2 and SME (scalable vectors and
predicates), mask types, FP16/BF16 arithmetic types, AVX-512VL forms of
128- and 256-bit operations (they use VEX), relaxed-simd, and wide paths
in more of std (graphics, linalg, FFT, compress).
