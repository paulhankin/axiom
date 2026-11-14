## Linking

- `--emit=obj` and `--emit=lib` write the target's own object format: Mach-O for macOS
  (arm64, x86-64) and COFF for Windows (x64, ARM64), which the platforms' linkers take
  (an object of C units alone has no runtime helpers in it). The compiler's linkers for
  macOS and Windows read these and the platforms' toolchains' objects and static
  archives, as the Linux one reads ELF: on macOS also dynamic libraries (`.dylib`, by
  their install names and exports) and libraries named by `-l` alone (bound by name), the
  platform's thread-local variables and `__mod_init_func`; on Windows import libraries,
  DLLs (their exports), COMDAT sections, weak externals, `__imp_` names, `.CRT$XC*`
  constructors and `.tls$`. `-l` on Windows finds `name.lib`, `libname.a` or `name.dll`
  in the `-L` directories. The macOS output, and the Windows ARM64 output, are built and
  checked structurally; they have not been run.
- A C source given with `--link=file.c` for a macOS or Windows target is compiled by the
  compiler itself (in a process of its own, with the build's target and C options) into
  an object in the target's format; for Linux targets the system's C compiler compiles
  it.
- `--emit=shared` makes a shared library of C units (with `--c-lib=system` or
  `--c-freestanding`): an ELF `.so` (exports, relocations, initial-exec thread-locals,
  `DT_INIT_ARRAY`), a DLL (export directory; an entry point that runs the constructors and
  calls `DllMain`), or a Mach-O `.dylib` (install name, export trie). `--pic` makes code for
  shared libraries that other linkers link.
- The destructors of this compiler's COFF objects run in the same order (the reverse of
  their priorities, the plain ones first) whether the compiler's linker or the system's
  links them: their `.CRT$XT*` sections are named for the C runtime's forward order, and
  the compiler's start-up runs the termination table forward too.
