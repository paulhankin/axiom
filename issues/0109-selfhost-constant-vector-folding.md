# 0109: Self-hosted optimizer: integer vector operations of constant lanes, and float constant arithmetic, not folded

- Type: feature (performance)
- Status: fixed (1fb5609)
- Area: compiler (selfhost: opt, combine)

the benchmark's `half_point_sums_simd128` (the `--cpu=x86-64` build's `half_point_sums`, 0086
item 3) loops over four constant vectors of point numbers; unrolled, its masks
(`@lanes_ge(pj, six) & @lanes_le(pj, twenty_four)`, `pj +% one`, ...) are operations of
constants, which `fold` (opt.ax) only folded for scalars: each became a load of a constant,
a compare with another and a `pxor` with all ones, every call. Likewise a lane extracted
from a constant vector, and `+ - * /` of two float constants (`@float_from_int(f32, 36) /
36.0` on a path where the count is constant).

Fix: integer vector `+ - * & | ^`, shifts and compares whose lanes are all constants (a
constant vector, zero, or a splat of a constant) fold lane by lane; `x & ones`, `ones &
x`, `x | ones` simplify (combine.ax); an extract of a constant lane folds; scalar float
`+ - * /` of constants fold in the type's precision (f32 in f32: one rounding as at run
time), not when a NaN is involved.
