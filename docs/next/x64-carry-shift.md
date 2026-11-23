## Compiler

- Fixed: native x86-64 code (at `-O1` and above) could compute a wrong 64-bit sum of a
  shift by 1 to 3 and an unsigned comparison, such as `(a << 2) +% @as(i64, c < 34)`:
  the sum was made with the carry flag (`adc`), and the shift, which was to be folded
  into an `lea` instead, was never computed.
