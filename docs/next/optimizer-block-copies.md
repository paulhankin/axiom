## Compiler

- Copies and fills of a constant size from 256 bytes to 2 KiB on x86-64 move four
  vectors (16 bytes, or 32 with AVX) an iteration to destinations aligned to a cache
  line, the first 64 and the last bytes moved apart; before, one unaligned vector an
  iteration (half as fast, and twice as slow again when the destination was not
  aligned). On AArch64, those of 65 bytes (fills) or 129 bytes (copies) to 2 KiB are
  loops of two pairs of 128-bit loads and stores, not calls of `memcpy` and `memset`.
- A local structure built and then copied whole into its destination (C's `t` built and
  then `*r = t;`, a compound literal assigned, a result built in a local and returned)
  is built in the destination instead where nothing could tell: the destination is
  apart from everything else the code reads and writes meanwhile, and no other thread
  can see it partly built (a result's memory, a slot whose address goes nowhere; and in
  C, where a data race is undefined, also a global or a pointer's target). A program
  doing modular exponentiation of 1600-byte numbers ran in 0.42 s instead of 0.71 s.
