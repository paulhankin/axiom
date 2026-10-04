# 0033: Checked 64-bit multiplication divides back natively

- Type: feature (performance)
- Status: fixed (7f31aba)
- Area: compiler (C)

## Today

`mul64_overflow` detects overflow of a checked 64-bit multiply by dividing
the product back (`codegen.c`), because freestanding wasm has no 128-bit
multiply and the comment notes that `llvm.umul.with.overflow` takes a slow
path at -O0 on wasm. Issue 0031 removed the division for a constant
operand; with two variables it remains, natively too, where the overflow
flag of one `mul` (or `mulx`/`imul` + `jo`) gives it for free. A 64-bit
`div` is 25–40 cycles.

the benchmark's `fast_div_small` (a reciprocal multiplication, `@as(u64, num) *
RECIP_MUL[den]`) runs four times per contact position, and each one divides:
measured as 3% of the native contact inputs (the benchmark
`axiom/src/eval/features.ax`; with `*%` the time drops from 205 to 199 ns).

## Proposal

- On native targets (and on wasm at -O1 and above, where the comment's
  concern does not apply), emit `llvm.umul.with.overflow.i64` /
  `llvm.smul.with.overflow.i64`. x86-64 lowers them to `mul`/`imul` and `seto`.
- Independently: when both operands are zero-extended from 32 bits (as
  here, `@as(u64, u32)` times a value loaded from a table of `u64` the
  compiler cannot bound), at least one 32-bit operand halves the range; when
  both are, the product cannot overflow and needs no check at all.

## Resolution

`mul64_overflow` now uses `llvm.umul.with.overflow.i64` / `llvm.smul.with.overflow.i64` natively and on wasm at `-O1` and above. On x86-64 that is `mul`/`imul` and `jo`, with no `div`. On wasm LLVM lowers it through `__multi3`, which the compiler already provides. The divide-back stays only for wasm at `-O0`, where the comment's concern is real (FastISel falls back to SelectionDAG). A constant operand still compares against the quotients (issue 0031). If both operands are extensions from at most 32 bits (`@as(u64, a) * @as(u64, b)` with `u32` operands; signed: sign extensions from 32 bits or zero extensions from 31), there is no check at all. The issue's `@as(u64, num) * RECIP_MUL[den]` has only one such operand, so it gets the intrinsic. tests/run/mul_overflow_o2.ax checks both forms at `-O2`, on edge values.
