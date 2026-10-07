# 0025: Per-lane shift counts

- Type: feature
- Status: fixed (107a199)
- Area: language, compiler (C and selfhost)

## Today

`v << k` and `v >> k` on integer vectors take one scalar count for every
lane (§13.1, as in WebAssembly). x86 has had per-lane counts since AVX2
(`vpsllvd`, `vpsrlvd`, `vpsravd`; `q` forms; `w` forms with AVX-512BW), and
AArch64 has `ushl`/`sshl` with a vector of counts.

Bit-board code needs them: the benchmark's hitting-shot feature (Rust's
`hitting_shots_not_on_bar_avx512`, which made the contact inputs about twice
as fast natively) evaluates the 21 dice rolls in lanes, shifting point masks
by each lane's die: `blots << d`, `blocked >> step`, `blot_mask >> (23 - k)`
with `k` found per lane. Without per-lane counts the Axiom port can only
keep the scalar version: its contact inputs take 420 ns against Rust's 137
on the same AVX-512 machine (with AVX2 only, both scalar: 436 and 389).

## Proposal

Allow the count to be an unsigned integer vector of the same shape:
`v << w`, `v >> w`, lane i shifted by `w[i]`, with §7.6 applied per lane
(a count of at least the lane width gives 0, or the sign fill for `>>` on
signed lanes). That is exactly `vpsllvd`/`vpsrlvd`/`vpsravd`'s behaviour, so
x86 needs no fix-ups; AArch64's `ushl` shifts right with negative counts
(negate the count); wasm32 and SSE2 shift lane by lane.

## Resolution

`v << w` and `v >> w` take a count vector of the same shape (unsigned lanes), lane i shifted by `w[i]` with §7.6 per lane (spec §13.1), also in `<<=`/`>>=`. The C compiler emits LLVM's vector shifts (`vpsllvd`/`vpsrlvd`/`vpsravd`, `ushl`/`sshl`) with a select for counts of at least the lane width; the self-hosted compiler shifts lane by lane.
