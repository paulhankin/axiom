# 0120: Self-hosted x86-64: EVEX 8-bit displacements scaled by the vector, not the memory operand

- Type: bug (wrong code)
- Status: fixed (27f8a8a)
- Area: compiler (selfhost: x64enc)

EVEX encodes an 8-bit displacement in units of N bytes ("disp8*N", Intel SDM vol. 2,
2.7.5), where N is the size of the instruction's memory operand: the whole vector for
loads, stores and arithmetic on full vectors, one element for a broadcast or an embedded
broadcast, 16 bytes for `vinserti128`, half a vector for a widening load, and so on.
`x64enc.evex` used the vector's size for everything but embedded broadcasts. So a
displacement that is a multiple of 64 within +-127 * 64 in, say, a `vpbroadcastd` from
memory to a zmm register was encoded as disp / 64, which the processor scales by 4:

    vpbroadcastd 0x100(%rcx,%rsi,4), %zmm18      (meant)
    vpbroadcastd  0x10(%rcx,%rsi,4), %zmm18      (encoded)

It went unseen because such instructions mostly use EVEX only for 512-bit registers and
their displacements are rarely multiples of 64 (when not a multiple of N the encoder fell
back to a 32-bit displacement, which is right). Found through the benchmark's `dim128_pair` with its
loop over four 64-bit words of input masks fully unrolled: the copies read the inputs at
`in1 + 256 * k`, and the contact net's outputs came out wrong (bench_kernels' checksum
5570.6 against 16852.4).

Fix: `disp8_unit` gives N for each instruction: the vector for the instructions that have
256- and 512-bit forms on full vectors (`vl_ok`), the element for `vpbroadcastb/w/d/q`, and
0 (no 8-bit displacement; the 32-bit one is always right) for the rest: scalars, lane
inserts and extracts, `vinserti128`/`vextracti128`/`vinserti64x4`/`vextracti64x4`, widening
loads (`vpmovzx*`), shifts by a count in a vector register, and instructions with a general
register operand. Embedded broadcasts keep their element size.

Test: `tests/run/evex_disp8.ax` (broadcasts 256, 512 and 384 bytes past a pointer in a
register; run where the processor has AVX-512BW and VL).
