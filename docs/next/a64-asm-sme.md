## C front end

- The AArch64 assembler (inline and file-scope asm, assembly sources) has SME, SME2,
  SME2.1 and SVE2.1 in full, and B16B16: the ZA array, tile and slice operands
  (`za.s[w8, 0, vgx2]`, `za1h.s[w12, 0:1]`, `za3.d`), `zt0`, the predicate-as-counter
  registers `pn0`-`pn15`, strided register lists (`{z0.b, z8.b}`) and predicate pairs;
  `mova`/`mov`, `movaz`, the tile and multi-vector loads and stores, `ldr`/`str` of ZA
  and ZT0, `zero`, the outer products, `addha`/`addva`, and SME2's multi-vector
  arithmetic, dot product, multiply-add, conversion, clamp, `sel`, `zip`/`uzp`,
  `luti2`/`luti4` and `movt`; SVE2.1's `psel`, `pext`, `ptrue`/`cntp`/`while*` of a
  `pn`, predicate-pair `while*`, the two-way dot products, `bfmlslb`/`bfmlslt`, the
  narrowing pairs, the quadword permutes, reductions, loads and stores, and `pmov`.
  A table generated from the Arm ARM's encodings (`cmd/internal/tools/gen_a64_table.py`),
  checked by round-trip fuzzing over the SME and SVE encoding spaces.
- These instructions need their extension, now known to `--features` and `--cpu`:
  `sme2`, `sme2p1`, `sve2p1`, `sme-f64f64`, `sme-i16i64`, `sme-f16f16`, `b16b16`
  (and `sme`). `smstart`, `smstop`, `rdsvl`, `addsvl`, `addspl`, `revd`, `sclamp`,
  `uclamp` and `fclamp`, which were accepted for any AArch64 target, now need theirs
  too. The directive `.arch_extension name` allows an extension's instructions in the
  rest of an assembly without changing what the program needs at run time.
- The inline-asm clobbers `pn0`-`pn15` (the same registers as `p0`-`p15`).
