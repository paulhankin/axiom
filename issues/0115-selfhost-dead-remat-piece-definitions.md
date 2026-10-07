# 0115: Self-hosted x86-64: dead loads of rematerialized constants left behind by splitting

- Type: bug (performance)
- Status: fixed (d5317ce)
- Area: compiler (selfhost: mach)

Found in the benchmark's `axiom/` port (AVX-512 build), in `dim128_pair`'s sigmoid, which
looks up a 128-entry table held as eight `zmm` constants (`E_TABLE_128`, four
`vpermt2ps`). Under the kernel's register pressure the constants are rematerialized
(built again from memory where they are read), and the final code kept loads that nothing
read:

```
vmovdqu64 zmm16, [E_TABLE_128]        ; dead
vmovdqu64 zmm16, [E_TABLE_128+0x40]   ; dead
vmovdqu64 zmm11, [E_TABLE_128]
vpermt2ps zmm11, zmm4, [E_TABLE_128+0x40]
```

Six dead 64-byte loads per sigmoid (eight sigmoids per pair of positions).

Cause: when a rematerialized vreg is split (`split`), each piece's recorded definition
(`f.remat`) is a detached copy of the value's definition made for building it again; in the
code the piece is defined by the original instruction, its vreg renamed. When the piece was
later spilled, `spill_remat` did not recognise that instruction as the piece's definition
(`remat_defines` compared identities), so instead of deleting it, it treated it as a use:
it renamed its vreg and put a reload in front of it. The reload was dropped as dead
(`drop_dead_reloads`), the renamed definition stayed.

Fix: `remat_defines` also accepts an instruction that is the same as the recorded
definition (`same_inst`: opcode, size, condition, aux and operands). Instructions at 1-ply
(the benchmark `bench_search`, five positions): 4,880,211 -> 4,877,705; the binary 320 bytes smaller.
