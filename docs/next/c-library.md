## C

- C programs are hosted: `__STDC_HOSTED__` is 1 and the compiler's own C library is
  linked, on every target, from no DLL on Windows; `--c-freestanding` gives the
  freestanding implementation, with only the freestanding headers. The library has
  every header of C17 and C23:
  - its core: `<assert.h>`, `<ctype.h>`, `<errno.h>` (`errno` is each thread's),
    `<setjmp.h>` (on WebAssembly with the exception-handling extension), `<signal.h>`, `<stdlib.h>` (correctly rounded
    `strtod`, `strtof` and `strtold` in every `long double` format, decimal and
    hexadecimal; an introspective `qsort`; `malloc` on the Axiom allocator, or the system
    C library's heap where the program has it; `atexit`, `quick_exit`, `getenv`,
    `system`, `rand`, ...) and `<string.h>`;
  - `<stdio.h>` and `<wchar.h>`'s input/output: streams with full, line and no
    buffering, every `fopen` mode, text-mode line ends on Windows, 64-bit positions, a
    lock per stream; `fprintf` and `fscanf` with every conversion, floating-point
    conversions exact and correctly rounded in both directions;
  - `<locale.h>` (`"C"`, `"POSIX"`, `"C.UTF-8"` and UTF-8 names), `<wchar.h>`,
    `<wctype.h>` (Unicode's properties in a UTF-8 locale), `<uchar.h>`, `<time.h>` (the
    system's time zone) and `<inttypes.h>`.
- `exit` runs the `atexit` functions, then the GNU dialects' destructors, then flushes
  the streams; constructors and destructors also run around a `main` that is Axiom's.
- Only the library's units that a program uses are compiled (`std/c/lib/index`, which
  `cmd/internal/tools/gen_c_lib_index.mjs` writes): a hosted program builds about as fast
  as a freestanding one, and `printf("hello\n")` adds about 40 KB (x86-64 Linux).
- The runtime's `ax_open` opens files for reading and writing created or truncated
  (`w+`), for reading and appending (`a+`), and with exclusive creation (`x`); its
  `ax_clock_ns` has a clock 2, the processor time of the process.
