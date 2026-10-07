# 0118: Self-hosted x86-64: saturating float -> int conversions branch three times

- Type: feature (performance)
- Status: fixed (a92501d)
- Area: compiler (selfhost: x64)

`@int_from_float` (IR `fptosi.sat`/`fptoui.sat`: NaN gives 0, values out of range the
nearest bound, as Rust's `as`) was lowered as a NaN test, a test against the lower bound
and one against the upper bound, each a branch around a block that loads the bound and
jumps to the end, and only then the conversion. In the common case (an ordinary value in
range) all three branches are taken: three taken branches per conversion, and the blocks
in between. LLVM lowers Rust's `as` casts without branches.

the benchmark's cache encodes each of an evaluation's five probabilities with one
(`encode_prob`: `@int_from_float(u32, p * 65535.0 + 0.5)`), twice per cached result.

Fix (`x64.ax` `gen_float_to_int`), for results of up to 32 bits: `t = max(x, lo)` (`maxss`
returns its second operand when either is a NaN, so a NaN becomes `lo`, as does anything
below it), then the conversion of `t` (with 64 bits for a `u32`, exact below 2^32), then
`cmovae` of the upper bound where `t >= hi`, and for a signed result a `cmovp` of 0 where `x`
is a NaN. 64-bit results keep the branches.

Effect, one trial of the benchmark's `gbr` rollout (`bench_search --mode gbr --limit 1 --trials 1`,
counted instruction by instruction): taken branches 2,286,414 -> 2,145,482 (-6.2%; the
batch evaluator's 423,631 -> 283,051), instructions 28,585,219 -> 28,475,691 (-0.4%). (Rust's
build: 1,740,310 taken branches.) At 1-ply and 2-ply, where little is cached, no change.
The `p <= 0.0 or p != p` test before it still branches twice (LLVM merges it into one
`fcmp ule`); not done here.
