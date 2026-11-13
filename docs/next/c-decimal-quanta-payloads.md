## C

- The decimal functions give more exact results the quantum C23 prefers
  (`atan2pid64(1, -1)` is `0.75`, `exp2d64(3.0dd)` is `8`, `compoundnd64(0.1dd, 2)` is
  `1.21`), and `strtod64` and `fscanf` keep a NaN's payload from its n-char-sequence.
