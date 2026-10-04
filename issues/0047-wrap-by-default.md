# 0047: Integer overflow wraps by default, in every build mode

- Type: language change (decision)
- Status: open
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
