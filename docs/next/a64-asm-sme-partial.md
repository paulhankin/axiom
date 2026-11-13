## C front end

- The AArch64 assembler gains the SVE2.1 / SME forms that fit the SVE encoding
  machinery: `rdsvl`, `addsvl`, `addspl` (the streaming vector length), `revd`
  (reverse doublewords in 128-bit elements), and the clamps `sclamp`, `uclamp`,
  `fclamp`. Verified against the architecture's encodings and by round-trip fuzzing.
