# 0031: Code generation and shuffles: @select through bitwise operations, checked multiplication by a constant, no widening @shuffle

- Type: feature (performance)
- Status: fixed (d1b9c1a)
- Area: compiler (C), language (§13.2)

Three smaller things seen in the same AVX-512 kernels as issue 0030. Fixing
the first two (as local experiments) did not move the benchmark's benchmarks beyond
their noise (about ±10%), but each costs extra instructions in common code.

## 1. `@select` is emitted as and / and-not / or

`BI_SELECT` builds `(x & m) | (y & ~m)` on the bit patterns, the WASM
`v128.bitselect` meaning. When `m` comes from a lane comparison (`@lanes_*`,
emitted as `sext <N x i1>`), LLVM only partly turns that back into a blend.
the benchmark's sigmoid selects among four table lookups on two index bits:

```axiom
let m32 = @lanes_ne(idx & @as(u32x16, @splat(32)), none);
let m64 = @lanes_ne(idx & @as(u32x16, @splat(64)), none);
let e = @select(m64, @select(m32, r67, r45), @select(m32, r23, r01));
```

which becomes `vptestmd`, then `vpmovm2d` (mask register to vector), and per
inner select `vpandd`, `vpord`, `vmovdqa32 {k}`: eight instructions where
Rust's `_mm512_mask_blend_ps` gives three blends. On machines without mask
registers the same pattern can cost `pand`/`pandn`/`por` instead of one
`blendvps`.

Proposal: when the mask is (after looking through `&`, `|`, `^` and `~` of
such values) a sign extension of an `i1` vector, emit `select <N x i1>` on
it. The result is the same bit for bit, since every lane of the mask is all
ones or all zeros. Otherwise keep the bitwise form.

## 2. A checked `n * k` with a constant `k` divides

`mul64_overflow` detects overflow of a 64-bit multiply by dividing the
product back (no 128-bit multiply on freestanding wasm). With a constant
operand, as in a length check such as

```axiom
@expect(net.hidden_weights.len >= (input.len * 128));
```

that is a `div` (20–40 cycles) where a compare does it: `n > MAX / k` for
unsigned operands (and the analogous two bounds for signed ones). Narrower
multiplies are fine: they widen and compare.

Proposal: in `mul64_overflow`, when either operand is a constant, test the
other against the quotient bounds instead.

## 3. `@shuffle` cannot change the lane count

`@shuffle(a, b, idx)` returns a vector of `a`'s type, so there is no way to
widen an `f32x8` to an `f32x16` (x86's free `_mm512_zextps256_ps512`, LLVM's
`shufflevector` with a longer mask) or to take one half of a vector as a
narrower one (`_mm512_castps512_ps256`). the benchmark's 8-unit nets sum in `__m256`
registers and widen the result for the 16-lane sigmoid; the Axiom port has
to keep those sums in 16-lane vectors from the start, loading each 8-float
row with `@load_partial`.

Proposal: let the index list's length set the result's lane count, as LLVM's
`shufflevector` does: `@shuffle(a, zero, [0, 1, ..., 7, 8, 8, ...])` for a
zero-extension, `@shuffle(a, a, [8, ..., 15])` for the high half.

## Resolution

1. `@select` whose mask is a lane comparison, `&`/`|`/`^`/`~` of such masks, or an immutable local initialized with one (recognized on the AST, since at code generation locals are still loads) is emitted as `select` on the mask's sign bits, the same bit for bit; other masks keep the bitwise form. the benchmark's four-way sigmoid select is two `vptestnmd` and three masked moves (the table loads folded in).
2. A checked 64-bit multiply with a constant operand compares the other with `MAX / k` (unsigned) or the two quotient bounds (signed; `k = -1` checks for `MIN`) instead of dividing back: `n * 128` has no `div`.
3. `@shuffle`'s index count sets the result's lanes, any count making a 128-, 256- or 512-bit vector (spec §13.2), in both compilers: `@shuffle(a, zero, [0, ..., 7, 8, ..., 8])` widens an `f32x8`, `@shuffle(a, a, [8, ..., 15])` takes a half.
