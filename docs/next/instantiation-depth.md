## Compiler

- A generic instantiation nested deeper than 64 levels (an instance of a function that
  instantiates it again with a larger type, without end) is now reported as E0465, as
  spec §10.6 says, instead of compiling forever. The spec's table of diagnostic codes
  (§16.3) now lists every code the compilers emit.
