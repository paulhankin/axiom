## Compiler

- Integer division and remainder by a constant (at `-O1` and above, every target,
  Axiom and C, 8 to 64 bits, signed and unsigned) no longer use the divide
  instruction: they are a multiplication by a reciprocal and shifts (a shift, a mask or
  a comparison for powers of two and divisors above half the range), and `x % c == 0`
  is a multiplication and a comparison. A quotient and a remainder of the same
  operands share the multiplication, and, for a divisor that is not constant, the one
  divide instruction. C pointer differences divide exactly (a shift and a
  multiplication). On WebAssembly a 64-bit division by a constant is left to the
  engine, which does the same. A C benchmark that spends its time producing decimal
  digits ran in less than half the time, and one that converts big numbers between
  bases in two thirds.
