## Compiler

- On x86-64 at `-O0` and `-O1`, a constant near the bottom of `i64` minus a 64-bit
  `@clz` (`-9223372036854775808 -% @clz(x)`), or one near the top plus a 32-bit
  `@clz`, no longer stops the compiler with an integer overflow (`T0002`) in its own
  code.
