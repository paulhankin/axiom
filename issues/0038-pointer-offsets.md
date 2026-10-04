# 0038: Pointer offsets: no way to say an address stays inside its object

- Type: feature (language, performance)
- Status: open
- Area: language, compiler (C and selfhost)

## Today

Spec §5.7: "There is no pointer arithmetic operator. Write
`@ptr_from_int(*T, @int_from_ptr(p) + k)`." Kernels that read rows of a weight
matrix through raw addresses (after checking the matrix's length once) then
choose between two costs on wasm32:

- `base +% 16`: wrapping, so LLVM may not fold the 16 into the load's
  `offset=` (wasm folds only when the add cannot wrap), and every load
  computes its address with an extra `i32.const`/`i32.add`;
- `base + 16`: folds, but every add is an overflow check, a compare and
  branch.

the benchmark's wasm `dim128` (the benchmark `axiom/src/eval/net.ax`) loads eight
`f32x4` per non-zero input from `w`, `w + 16`, ..., `w + 112`. Its inner loop
has seven `i32.const -k; i32.ge_u; br_if` checks per input, beside eight
loads and eight multiply-adds. The hidden-layer loop is 64% of the contact
net's time, and the port runs at 1.15–1.25x Rust's wasm build, whose
`hw.add(i * 128 + base)` and `w.add(j * 4)` are in-bounds pointer offsets
(`getelementptr inbounds`): no check, and folded offsets.

## Proposal

`@ptr_add(p, n)`: `p` a pointer `*T` / `*mut T`, `n` a `usize`; the pointer
`n` elements of `T` past `p` (C's `p + n`, Rust's `p.add(n)`), lowered to
`getelementptr inbounds`. Like reading through `@ptr_from_int`, it is the
program's promise that the result stays within the object `p` points into
(or one past it); it never traps, and an out-of-object result is as unchecked
as a wrong `@ptr_from_int` address is today. With it, `@ptr_add(row, 4)`
reads like Rust and compiles like it: on wasm32 `v128.load offset=16`, natively
an addressing-mode displacement. Perhaps also `@ptr_sub` or a signed variant.

(An alternative that needs no new builtin is for the compiler to merge the
checks of `x + c1`, ..., `x + cn` with constants into one check of the largest,
but that keeps one check per row and changes which span a trap reports.)
