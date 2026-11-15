## Compiler

- Optimized builds take about half the memory at their peak and less time: 2.30 to
  1.09 GB, and 25.3 to 22.2 s, for the compiler building itself for x86-64 at -O2.
  Value numbering asked for vector types many times for each load it looked at, and
  each time made a name for the type that was kept until the end of the build.
