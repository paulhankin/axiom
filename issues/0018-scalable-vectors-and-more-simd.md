# 0018: SVE/SVE2, mask types, FP16/BF16 and the rest of wide SIMD

- Type: feature
- Status: open
- Area: language, compiler (selfhost and C), std

## Today

Issue 0015 added 256- and 512-bit vector types, AVX2 and AVX-512 code
generation on x86-64, conditional compilation (`@target_has` and kin) and
run-time dispatch (`@cpu_has`, `@features`, `@multiversion`); see spec
§13.4. What it left out:

- **SVE/SVE2 and SME.** The extensions are known (`--features=sve`), but
  nothing uses them: AArch64 splits every wide vector into 128-bit NEON
  operations. Scalable vectors need either a separate type family (with a
  length known only at run time) or length-agnostic loop helpers in std,
  plus predicate registers and the SVE calling convention.
- **Mask types.** Comparisons give vectors of all-ones/all-zeros lanes, as
  for 128 bits. AVX-512 `k` registers and SVE predicates are used only
  inside the compiler (k1 as scratch for compares).
- **FP16 and BF16 arithmetic types** (`f16x8`, ...): the extensions exist
  (`avx512fp16`, `avx512bf16`, `fp16`, `bf16`) but there are no types.
- **AVX-512VL**: 128- and 256-bit operations use VEX encodings, so they
  cannot use xmm16-31, masking, or the AVX-512-only instructions
  (`vpternlog`, `vprol`, 64-bit `vpmullq`, ...) at those widths.
- **relaxed-simd** on wasm is accepted but not used.
- **std**: only `str.index_of_byte` and the crypto code (issue 0016) have
  wide paths. Candidates: pixel loops in std/graphics, std/math/linalg and
  FFT, deflate matching, base64, UTF-8 validation.

## Notes

- qemu runs SVE (`qemu-aarch64 -cpu max,sve=on`) and AVX-512 for testing.
