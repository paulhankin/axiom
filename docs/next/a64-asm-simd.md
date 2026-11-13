## C front end

- The AArch64 inline and file-scope assembler encodes Advanced SIMD in full from a
  generated table of instruction forms (`cmd/internal/tools/gen_a64_table.py`), each
  with exactly the arrangements the architecture allows: vector and scalar, integer and
  floating point in single, double and half precision, the dot-product,
  matrix-multiply, complex-number, BF16 and `fmlal`-class extensions, `pmull` and the
  cryptographic instructions (AES, SHA1/SHA-256/SHA-512, SHA3, SM3, SM4); the
  structure loads and stores `ld1`-`ld4`, `st1`-`st4` and `ld1r`-`ld4r` in every
  addressing form; the vector immediates (`movi`, `mvni`, `orr`, `bic`, `fmov`); and the
  copy instructions, whose element indexes and views are now checked.
