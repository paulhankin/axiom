## C compiler

- The AArch64 assembler (inline assembly and file-scope asm) now encodes SVE and
  SVE2: the `z0`-`z31`, `p0`-`p15` and `ffr` registers, the element-size suffixes
  and `/m` `/z` predication, and the integer, logical, shift, compare, multiply,
  reduction, permute, move, index, predicate, element-count, floating-point and
  conversion instructions, with `ldr`/`str` of a `z` or `p` register and the
  contiguous `ld1`/`st1` loads and stores. `z`, `p` and `ffr` may appear in an
  inline-asm clobber list. The gather/scatter, first/non-fault, structure and
  replicating loads, the SVE prefetches, and the SVE2 widening, narrowing,
  saturating and permute families are not yet encoded.
