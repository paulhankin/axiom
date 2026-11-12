## Compiler

- Integer division and remainder by a constant (at `-O1` and above, every target,
  Axiom and C, 8 to 64 bits, signed and unsigned) no longer use the divide
  instruction: they are multiplications by a reciprocal and shifts, a mask or a
  comparison for powers of two and divisors above half the range, and `x % c == 0`
  is a multiplication and a comparison. A quotient and a remainder of the same
  operands share the multiplication (and, for a variable divisor, the one divide).
  Code that produces decimal digits is two or more times as fast. On WebAssembly a
  64-bit division by a constant is left to the engine, which does the same.
