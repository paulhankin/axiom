# 0047: Integer overflow wraps by default, in every build mode

- Type: language change (decision)
- Status: fixed (3d76aff)
- Area: language (§7.6, Appendix D Q1), compiler (C and selfhost), std, tests

## Decision

The project owner has settled Appendix D, question 1, as option (c): `+`, `-`, `*`, unary
`-`, `MIN / -1` (and the compound assignments) on integers compute their result modulo 2ⁿ,
in every build mode by default, as Go, Java and Rust's release builds do. Overflow no longer
traps (`T0002`) unless asked for.

## Why

Checked arithmetic was the largest remaining cost of the benchmark against Rust's release
build: move generation's core function has 85 conditional branches and 30 trap sites in
wasm where Rust's has 40 and 13 (the rest are overflow checks on checker counts and point
indices), and ports had to sprinkle `+%`/`*%` with comments to avoid checks in hot paths
(and a checked 64-bit multiply needed issue 0033 to stop dividing). The checks also kept
LLVM from vectorizing simple reductions (issue 0029).

## Changes

- §7.6: the default overflow behaviour is wrapping. `+%`, `-%`, `*%`, `-%` stay (same
  meaning; documented as explicit spellings of the default). Division by zero still traps
  (`T0003`) in every mode; `MIN % -1` stays 0; shifts unchanged.
- Build modes: `--overflow=wrap` (default), `--overflow=trap` (opt-in debugging mode: the
  old checks, `T0002`), `--overflow=saturate` (kept if cheap, else removed). `--release`
  no longer changes semantics. Update §16.1, the code registry (`T0002` only in trap mode),
  Appendix C (a new row) and Appendix D (Q1 resolved: record the decision).
- Integer SIMD vectors: allow `+`, `-`, `*` (wrapping) as for scalars (C12 required `+%`
  only because lane-wise trapping was impractical); keep `+%` valid.
- Constant evaluation: decide and document (keep `E0530` for overflow in `const`
  initializers and array lengths, as a compile-time error, since it is free there, unless
  the owner says otherwise).
- `@add_sat`/`@sub_sat`/`@mul_sat` and the `@*_checked` builtins unchanged: they are the
  way to ask for saturation or a checked result explicitly.
- Both compilers, std (anything relying on overflow traps for safety, e.g. size
  computations, must now use the checked builtins explicitly: audit std/mem and others),
  tests (tests expecting T0002 must run in `--overflow=trap` mode or change), docs.

## Resolution

Both compilers have an overflow mode, `wrap` (the default), `trap` or `saturate`
(`--overflow=wrap|trap|saturate`, spec §16.1). In `wrap` mode `+`, `-`, `*` and unary `-`
are plain instructions, and a signed `/` replaces a divisor of `-1` with 1 and negates
(as `%` already did), so `MIN / -1` is `MIN` and wasm's `i32.div_s` never sees it.
`@abs(MIN)` is `MIN`. `trap` mode is the old code (`T0002`), `saturate` stays (it costs
nothing to keep). `--release` is accepted and changes nothing. Integer vectors take `+`,
`-`, `*` (not on 8-bit lanes) and unary `-` (signed lanes; `E0520` on unsigned ones),
wrapping in every mode; `+%` `-%` `*%` stay valid everywhere. Constant evaluation keeps
`E0530` (spec §7.6 says so, with `+%` as the way to wrap a constant on purpose).
`@mul_sat`'s operands are now generated in the build's mode (they were generated in
saturate mode).

std: `std/mem` already sized with `@mul_checked` and refused huge requests (`too_big`);
`str.concat`, `str.join` and `str.replace` now compute sizes with `@add_checked` /
`@mul_checked` (`OutOfMemory` instead of a wrapped size that a bounds check would then
trap on), and GIF's per-file work limit saturates. The image decoders check dimensions
with `@mul_checked` against `MAX_PIXELS` already; elsewhere a wrapped size or index still
meets a bounds check, so memory stays safe.

Tests: the checked-arithmetic tests (`mul_overflow*`, `bounded_sums*`, `overflow_trap`)
run with `// flags: --overflow=trap`; `overflow_saturate` with `--overflow=saturate`.
New: `overflow_wrap` (every width, signed and unsigned: `MAX + 1`, `MIN - 1`, products,
`-MIN`, `MIN / -1`, `MIN % -1`, `@abs(MIN)`, compound assignments, `i32x4`/`u8x16`/`i16x8`/
`u64x2` `+ - *` and `-`), `overflow_trap_{div,neg,abs,wrapping}`, `overflow_release`,
`tests/testing/overflow_modes.ax` (`axiom test --overflow=trap`), and the vector
diagnostics (`int_vec_div`, `int_vec_mul8`, `uint_vec_neg`). The self-hosted backends'
fuzzer also checks `+ - * /` (with `MIN / -1`), `-` and `@abs` on scalars and `+ - *` and
`-` on vectors against LLVM. The self-hosted compiler reaches its fixed point (it relied
on no overflow trap: a trap would have crashed it).

the benchmark (`bench_kernels`, `-O2`, best of several runs on a loaded 4-core machine, so
about ±10% noise; checksums identical in every kernel): native movegen 625 → 611 ns,
classify 5.6 → 4.3, contact_inputs 200 → 143, race_inputs 108 → 86, contact_single 289 →
243, the nets within noise; wasm movegen 866 → 861 ns (the port already wrote `+%`/`*%` in
its hot paths), classify 7.4 → 5.9, contact_inputs 442 → 345, pruned_boards 102 → 86. The
executables shrink 12–14% (native 617 → 533 KB, wasm 300 → 263 KB). Its test files pass
as before; none expected `T0002`.

