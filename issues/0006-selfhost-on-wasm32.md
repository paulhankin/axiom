# 0006: The self-hosted compiler does not build for wasm32

- Type: bug
- Status: fixed (commit "The self-hosted compiler builds and runs on WebAssembly")
- Area: compiler (selfhost)

```
$ selfhost/build/axiom check selfhost/src/main.ax --target=wasm32
selfhost/src/gvn.ax:2701:17: error[E0512]: @bit_cast size mismatch: i64 is 8 bytes, usize is 4 bytes
    uses.at(@bit_cast(usize, o.aux)).push(x);
selfhost/src/gvn.ax:2751:43: error[E0512]: ...
selfhost/src/wlink.ax:213:24: error[E0512]: ...
```

The compiler assumes 64-bit `usize` in a few places (`@bit_cast` between
`i64` and `usize`). `axiom check` defaults to wasm32, so checking the
compiler's own sources needs `--target=native`. Fixing the three casts
(`@int_cast` / `@truncate` instead of `@bit_cast`) would let the compiler
build for wasm32 (and wasm64), i.e. run in a browser; whether everything
else it needs (files, processes for the linker and module workers) is
available there is a separate question.

## Fix

Beyond the three casts, the compiler called the C library directly
(`extern "C"` in `util.ax`: malloc, stdio, fork/exec, opendir, strtod,
snprintf, ...), which WebAssembly does not have. It now reaches the system
only through the host ABI and std, like any Axiom program: memory from
`std/mem` (behind a size header, for `free(p)`), files and directories through
`ax_open`/`ax_stat`/`ax_readdir`, child processes through `std/os/process`,
float parsing and formatting through `std/fmt` (both correctly rounded, as
glibc's are). The one thing WebAssembly cannot do is fork, which separate
compilation uses to compile modules in parallel: a new host function,
`ax_fork`, returns -38 under node, and the compiler then builds the program
as one unit. One more 64-bit assumption turned up at run time
(`@int_cast(usize, a.cv)` on a non-parameter in `ipcp`).

Result: `node runtime/host.mjs axiom.wasm -- build selfhost/src/main.ax`
produces byte-identical output to the native compiler, for native and wasm32
targets. Natively the compiler is now a static executable with no C library;
self-build speed and memory are unchanged within noise.

Not done: wasm64. The compiler builds for it, but `runtime/host.mjs` cannot
run any wasm64 program (even hello world fails on 64-bit pointers in host
calls).
