## Compiler

- Builds at `-O2` and `-O3` are about 5% faster: after inlining, a function is
  re-optimized with only the passes that the inlined code gives work to, not the
  whole pipeline again. The speed of the generated code is unchanged in the
  compiler's benchmarks.
