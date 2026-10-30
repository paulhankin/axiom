## Compiler

- Builds of large programs at `-O2` and `-O3` are faster: after inlining, the
  compiler finishes and compiles the program's functions in a few parts in parallel
  processes, and links their objects. On four cores the compiler builds itself in
  about 20% less time. The parts depend only on the program, so the executable is the same
  wherever it is built, though it is not the same as one compiled in one piece. The
  internal symbols of a program compiled in parts have the same names in its symbol
  table as before.
