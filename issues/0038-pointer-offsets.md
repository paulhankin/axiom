# 0038: Pointer offsets: no way to say an address stays inside its object

- Type: feature (language, performance)
- Status: fixed (60a6470)
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

## Resolution

Implemented in both compilers and specified (§5.7, Appendix A, C41). `@ptr_add(p, n)`, with `p` a `*T` or `*mut T` (`T` not `void`) and `n` a `usize`, is the pointer `n` elements of `T` past `p`, of `p`'s own type (so `*mut` stays writable). `@ptr_sub(p, n)` is `n` elements before it. Neither traps: each is the program's promise that the result stays inside the object `p` points into, or one past its end. A result outside it is a logic bug like a wrong `@ptr_from_int` address, with a weaker guarantee, since code compiled on the promise may have moved the accesses: addresses and comparisons through it are unspecified. Accesses through it are still checked against linear memory where §12.1 applies. Errors: `E0822` for a non-pointer or `*void` operand, and `n` is checked as a `usize` (`E0412` for a `u32`).

`@ptr_sub` is included because walking back from an end pointer (`@ptr_sub(end, 1)`) is as common as walking forward, and through `@int_from_ptr` it loses the promise. There is no signed variant: the name gives the direction, and `n` stays a `usize` like every length and index.

C compiler: `getelementptr inbounds [size x i8], p, n` (with `n` negated for `@ptr_sub`). On wasm32 at -O2, a dim128-style loop with `let w = @ptr_add(w0, i * 8)` and eight `@ptr_add(w, k).*` loads compiles to `v128.load`, `v128.load offset=16`, ..., `offset=112` from one address. The loop has 3 `br_if` (the `continue`, the checked `i * 8`, and the back edge; 1 with `i *% 8`). The `@ptr_from_int(..., @int_from_ptr(w) + k * 16)` version has 10. Natively it is `vmulps -112(%rdi), ...` through `(%rdi)`. Self-hosted: the same checks, lowered to its `ptradd`, which its wasm backend also folds into `offset=`.

tests/run/ptr_add.ax covers scalars, a 12-byte struct, bool, f32x4 rows from a raw address, generic element types, `@ptr_sub` from one past the end, and writes through `*mut`. tests/errors/ptr_add.ax checks the errors. The suite cannot inspect generated code, so the folding was checked with wasm2wat and `--emit=asm`.
