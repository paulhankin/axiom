## Compiler

- Separate compilation (`--units=module`, the default at `-O0` and `-O1`)
  compiles modules on threads rather than in forked processes, so the compiled-module
  cache and parallel module builds now work on Windows and with the compiler built for
  WebAssembly (under node) too, which compiled every program as one unit before. The
  objects are the same as before. `AXIOM_JOBS` sets how many modules (or parts of an
  optimized program) are compiled at once; the number of processors by default.
- `axiom cache clean` no longer runs `rm`.
- The compiler built for WebAssembly loads under node: the linker joins
  data segments that are next to each other, which keeps their number under the
  engine's limit.
