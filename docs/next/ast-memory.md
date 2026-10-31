## Compiler

- The compiler's syntax tree takes less memory. An expression is 72 bytes plus a
  record of the fields its kind has (an identifier's declaration, a call's arguments,
  ...), instead of 400 bytes with every kind's fields; what only a few kinds have
  (string and float literal text, struct literal fields, builtin and generic
  arguments, `catch` captures and handlers) is kept apart. A statement is 136 bytes
  instead of 200 (`match` arms, loop labels, `errdefer` and compound assignments kept
  apart). Checking the compiler's own sources takes 88 MB instead of 171 MB.
