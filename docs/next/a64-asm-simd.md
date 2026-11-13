## C front end

- The AArch64 inline and file-scope assembler encodes Advanced SIMD from a
  generated table of instruction forms (`cmd/internal/tools/gen_a64_table.py`),
  each with exactly the arrangements the architecture allows: three same (integer,
  logical, floating point), three different (the long, wide and narrowing forms and
  their `2` variants), two-register miscellaneous (with the compares with zero, the
  narrowing moves, `shll`, `fcvtn`/`fcvtl`/`fcvtxn`), across lanes, shifts by
  immediate (with `sxtl`/`uxtl` and the fixed-point conversions), permutes, `tbl` and
  `tbx`, and the by-element multiplies; the floating-point forms in single, double
  and half precision.
