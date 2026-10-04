# 0035: `@widen_low`/`@widen_high`/`@narrow` missing; no bool to integer

- Type: bug (C compiler), feature (language)
- Status: open
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
