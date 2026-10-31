## Compiler

- The compiler's syntax tree takes less memory: an expression is 208 bytes instead of
  400, with what only a few kinds of expression have (string and float literal text,
  struct literal fields, builtin and generic arguments, `catch` captures and handlers)
  kept apart; a statement is 136 bytes instead of 200 (`match` arms, loop labels,
  `errdefer` and compound assignments likewise). Checking the compiler's own sources
  takes 112 MB instead of 171 MB.
