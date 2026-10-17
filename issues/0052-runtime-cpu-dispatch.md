# 0052: No runtime CPU feature dispatch

- Type: feature
- Status: fixed (a749803): filed in error, the feature exists (issue 0015, spec §13.4); two bugs found while checking it are fixed
- Area: language, compiler (C and selfhost), std

`@target_has("avx512f")` is a compile-time constant, so an Axiom program uses the vector
instructions its `--cpu` names and no others. A binary built for baseline x86-64
(`--cpu=x86-64`, what a distributed binary has to be) never runs AVX2 or AVX-512 code, even on
a machine that has it. Rust builds the same binary and still runs AVX-512 where the CPU has it:
`#[target_feature(enable = "avx512f")]` compiles one function for a feature the target does not
assume, and `is_x86_feature_detected!("avx512f")` (cpuid, cached) chooses it at run time.

Found in the benchmark's benchmark matrix (`benchmarks/matrix.py`, 2026-10-04 run). Built for
x86-64, Rust's the benchmark-eval detects AVX-512 at run time. The Axiom port uses SSE2. At 1-ply
through 4-ply the Axiom build is 2.4–2.9× slower than Rust's. With both built for the native
CPU, Axiom is 0.98–1.18×.

Proposal:

- `@target_feature("avx512f", "avx512bw") fn ...` (or a `target_feature` attribute) compiles
  the function as if those features were on: LLVM's `"target-features"` function attribute,
  and the selfhost backend's feature set for that function. Inside it, `@target_has` answers
  for the function's features. Calling it is allowed only where the caller has the features,
  or inside a `@cpu_has(...)` check, as Rust's `unsafe` call.
- `@cpu_has("avx512f")` reads cpuid once and caches the result (std/os on x86-64; on
  wasm32 it is the compile-time `@target_has` answer).
- The selfhost backend needs per-function ISA selection. Its AVX-512 lowering (0046) exists, so
  this is choosing the lowering per function rather than per build.

## Resolution

Filed in error: the language has had this since issue 0015 (5f8febd, f67d888), in spec
§13.4, under other names:

- `fn f(...) T @features("avx512f,avx512bw") { ... }` is the proposed `@target_feature`: the
  body is compiled with those extensions added (LLVM's `"target-features"` attribute; the
  self-hosted backends' per-function `ir.Func.features`, which selects the whole-width
  AVX-512 lowering of 0046 for that function alone), and `@target_has` inside it answers
  for them. A call checks the processor and traps with T0013 if it lacks them (the check
  disappears where the build has them), so it belongs after `if @cpu_has(...)`.
- `fn f(...) T @multiversion("avx512f,avx512bw", "avx2") { ... }` compiles the body once per
  set and once for the build, and each call runs the first version the processor has.
- `@cpu_has("avx512f")` is cpuid (and xgetbv for the OS's AVX and AVX-512 state), read once
  by std/os/cpu and cached; the constant `true` where the build has the extension, and
  `false` under WebAssembly.

Checking it on a `--cpu=x86-64` build with a version for
`avx512f,avx512bw,avx512cd,avx512dq,avx512vl` turned up two bugs, in both compilers,
fixed in a749803:

- **`inline fn` helpers did not get the version's extensions.** A helper is inlined into an
  AVX-512 version, but its body was generated once, for the build, so `@target_has` inside it
  answered for the build (and in the C compiler, lowering that depends on extensions, such
  as `@dot` and permutes, chose the build's sequences). Now a direct call compiles the
  inline function with its caller's extensions: one instance per set it is called with
  (`get_callee` in cmd/axiom/bootstrap/src/codegen.c and cmd/axiom/src/codegen.ax; spec §9.5, §13.4).
- **A function first reached from a version took the version's extensions.** Instances
  took their extensions from the current ones (`g_features`), which are the version's
  while its body is generated, so a function that is not inline, first called from an
  AVX-512 version, answered `@target_has("avx512f")` with `true` for every caller,
  including baseline code on any processor (the C compiler also gave it no
  `"target-features"` attribute). Instances now take the build's extensions.
  The same mistake hit std/os/cpu, which must be compiled for the bare architecture: in
  the C compiler, a function of it first reached from another of its functions got no
  `"target-features"` attribute (its extensions, none, equalled the current ones), so it
  was compiled for the build's. With `--cpu=x86-64-v4`, `cpu.features` (which `@cpu_has`
  and the start-up check call) held about 190 AVX-512 instructions, and on a processor
  without AVX-512 the start-up check would have died of an illegal instruction instead of
  printing its message. `tests/lowering/avx512_whole.ax` had counted two of them (lane
  extracts in `dependents`) as expected; it now expects none.

Test: `tests/run/multiversion_inline.ax`, built natively for `--cpu=x86-64` on an x86-64 host
(a new `x86-64-flags:` directive in tests/run.mjs), so on an AVX-512 processor the versions
run AVX-512 code.
