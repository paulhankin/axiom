# 0114: Self-hosted optimizer: `@vec` with constant lanes inserts every lane, one after the other

- Type: feature (performance)
- Status: fixed (73b2664)
- Area: compiler (selfhost: combine, x64)

Found with 0086 (the benchmark's `axiom/` port, 0-ply). `@vec(f32x16, x0, x1, x2, x3, x4, 0.0,
.., 0.0)` is a chain of sixteen lane inserts into an undefined vector. The eleven constant
lanes stayed inserts, so with AVX-512 the vector was sixteen masked broadcasts
(`kmovq k1, 1 << i; vpbroadcastd zmm{k1}, xmm`), each waiting for the one before: some 50
cycles for a vector LLVM builds with four `vinsertps` and one masked broadcast. the benchmark's
`dim128` (the 128-hidden nets, one position) builds its five outputs into one vector this
way for the sigmoid, once per evaluation.

Fix (combine.ax `sink_const_insert`): a constant inserted into a chain of inserts of other
lanes (each used only by the next) that starts from a constant vector goes into that
constant instead, and the other inserts are made again on it; an insert of the same lane
further down is dropped, and a constant the lane already holds is no insert at all. The
undefined lanes of the starting vector become zero. The variable lanes then go into a
vector known to be zero above them, which x64.ax already builds with `insertps`.

Effect (kernels, ns per evaluation, min of 9): `contact_single` 425 -> 402 (LLVM 396) with
AVX-512, 439 -> 416 (LLVM 413) at `--cpu=x86-64`; `pruned_single` 187 -> 184 (176) and
210 -> 190 (187). bench_search 0-ply with AVX-512 1.07 -> 1.01 of min(Rust, LLVM).
tests/lowering/x64_vec_const_lanes.ax checks the instructions, tests/run/vec_const_lanes.ax
the results (LLVM's).
