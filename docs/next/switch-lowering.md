## Compiler

- Faster multiway branches on every target: a `match` on integers or enums, and a C
  `switch`, become jump tables where their cases are dense (an indirect jump through a
  table of 32-bit offsets in read-only data on x86-64 and AArch64, a `br_table` on
  WebAssembly), bit tests where a few targets share cases within 64 values, and a
  balanced search over the rest, rather than a chain of compares (Axiom) or a search
  without tables (C on native targets). A 200-way `match` in a loop over random values
  runs more than twice as fast.
