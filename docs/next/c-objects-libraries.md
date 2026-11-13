## C and linking

- The compiler's own linker links Linux programs with the objects, static archives and
  shared libraries of the platform's toolchain (`axiom build main.c util.o libx.a`,
  `--link=`, `-l`, `-L`), and of this compiler: no system linker is used, and
  `--link=file.c` only needs the system's C compiler to compile the file. All the x86-64
  and AArch64 relocations of such objects are applied, their TLS models relaxed, COMDAT
  groups, common symbols, `__start_`/`__stop_` sections, initialization and termination
  arrays and copy relocations handled; archive members are linked when needed; shared
  libraries (and linker scripts such as the system's `libc.so`) are found by name; the
  toolchain's `libgcc.a` supplies its helpers.
- A program has one C library. By default it is the compiler's, which objects compiled
  against the system's headers use too (their `stdout`, `errno`, `<ctype.h>` tables,
  fortified functions, `__isoc99_scanf`, ... are this library's), and what would mix two
  C libraries is an error that names the object. `--c-lib=system` makes the system's C
  library the program's, with its headers as the hosted headers (on macOS and Windows,
  given with `--c-system-include`: there the functions are `libSystem`'s or
  `msvcrt.dll`'s).
- `--emit=obj` objects link with each other and with programs (their shared state is
  link-once, their constructors are in `.init_array`), and `--emit=lib` makes a static
  library of them.
- An Axiom program whose `main` returns runs its C code's `atexit` functions and
  destructors and flushes its streams, as C's `exit` does.
- Structures cross between Axiom and C by value (`extern "C"` functions and `export fn`
  take and return them as the target's C ABI says), and objects are shared: `let` in an
  `extern "C"` block declares C's, `export let` defines the program's (spec §14.5.3,
  §14.5.6).
