## Compiler

- Fixed: an object from `--emit=obj` for Linux could not be linked into a C program
  (`cc main.c lib.o`), as `examples/c_interop` does: it lacked the system call stubs the
  runtime calls (`ax_syscall`, `ax_tls_info`, …), which the compiler's own linker adds.
  The object now holds them.
- Fixed: compiling a program for WebAssembly at `-O1` or above could crash the
  compiler (an index out of bounds in its optimizer, which read a fact through a pointer
  into a table that had since grown). Building `examples/c_interop` for `wasm32` did.

