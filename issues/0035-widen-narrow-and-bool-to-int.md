# 0035: `@widen_low`/`@widen_high`/`@narrow` missing; no bool to integer

- Type: bug (C compiler), feature (language)
- Status: fixed (7f31aba)
- Area: compiler (C), language

## 1. Builtins listed but not implemented

Appendix A lists `@narrow`, `@widen_low` and `@widen_high` ("lane-width
changes per WASM"), but the C compiler rejects them:

```
error[E0120]: unknown builtin '@widen_low'
```

Programs widen with `@bit_cast(i16x8, @shuffle(v, zero, [0, 16, 1, 16,
...]))` instead, which LLVM does turn into `i16x8.extend_low_i8x16_u`
(and `vpmovzxbw`), so this is about the spec matching the compiler, and
about saying what one means. Either implement them (wasm's
`extend_low/high`, `narrow_*_sat`) or take them out of Appendix A.

## 2. No conversion from bool to an integer

Rust's `b as i32` (and C's implicit conversion) is common in branch-free
counting code: `hit_n1 += (n_chequers > 0) as i32`. Axiom has no builtin
for it (`@as(i32, b)` is rejected), so ports write a helper
`fn int_of(b: bool) i32 { if b { return 1; } return 0; }`, which LLVM folds
to a `zext` once inlined but which is a call otherwise (issue 0032).

Proposal: `@int_from_bool(b)` giving `u1`-valued 0 or 1 in any integer type
by context (or `@as(T, b)` for integer `T`).

## Resolution

1. Implemented in both compilers and specified (§13.2, Appendix A). `@widen_low(v)` and `@widen_high(v)` take an integer vector with 8-, 16- or 32-bit lanes, take half its lanes, and extend each to twice the width by the lane type's sign (`u8x16 → u16x8`, `i32x8 → i64x4`). `@narrow(T, a, b)` gives the lanes of `a` then those of `b`, each saturated to `T`'s lane range. Here `a` and `b` are integer vectors of one type with 16-, 32- or 64-bit lanes, and `T` has twice as many lanes of half the width. Source lanes are read by their own type, so `@narrow(u8x16, a, b)` is `i8x16.narrow_i16x8_u` for `i16x8` operands and clamps to 255 for `u16x8`. C compiler: a shuffle and `sext`/`zext` for the widenings (`vpmovzxbw`, `i16x8.extend_low_i8x16_u`). Narrowing is a clamp and `trunc`, which x86 turns into `vpmovswb`/`vpmovuswb`/`packuswb`; on wasm it uses the `narrow` intrinsics per pair of 128-bit pieces. Self-hosted: lane by lane. Errors: `E0812`, and `E0412` for operands of different types.
2. `@as(T, b)` converts a `bool` to any integer type (`false` 0, `true` 1; a constant for a literal). It is a lossless conversion that names its type, so it needs no new builtin and no inference from context: `n += @as(u32, x > 0)`.

tests/run/widen_narrow.ax compares every lane with the scalar conversion on edge values (128- and 512-bit vectors, all four sign combinations, 64-bit lanes). tests/errors/widen_narrow.ax checks the errors.
