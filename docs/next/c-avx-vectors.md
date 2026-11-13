## C

- On x86-64 System V, vectors of 32 bytes are passed and returned in `ymm` registers
  when AVX is enabled (`--cpu=x86-64-v3`, `--features=+avx`), and of 64 bytes in `zmm`
  registers with AVX-512F, as the psABI has it, so that libraries taking `__m256` or
  `__m512` values by value can be called (checked against the system's C compiler both
  ways).
