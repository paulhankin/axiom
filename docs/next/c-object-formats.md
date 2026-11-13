## C and linking

- `--emit=obj` and `--emit=lib` write the target's own object format: Mach-O for macOS
  (arm64, x86-64) and COFF for Windows (x64, ARM64), which the platforms' linkers take
  (an object of C units alone has no runtime helpers in it). The compiler's linkers for
  macOS and Windows read these and the platforms' toolchains' objects and static
  archives, as the Linux one reads ELF: on macOS also dynamic libraries (`.dylib`, by
  their install names and exports) and libraries named by `-l` alone (bound by name), the
  platform's thread-local variables and `__mod_init_func`; on Windows import libraries,
  DLLs (their exports), COMDAT sections, weak externals, `__imp_` names, `.CRT$XC*`
  constructors and `.tls$`. `-l` on Windows finds `name.lib`, `libname.a` or `name.dll`
  in the `-L` directories.
- `--emit=shared` makes a shared library of C units (with `--c-lib=system` or
  `--c-freestanding`): an ELF `.so` (exports, relocations, initial-exec thread-locals,
  `DT_INIT_ARRAY`), a DLL (export directory; an entry point that runs the constructors and
  calls `DllMain`), or a Mach-O `.dylib` (install name, export trie). `--pic` makes code for
  shared libraries that other linkers link.
- An archive's members are taken for what the program's own C units and `extern "C"`
  blocks need too, not only for the other objects' references.
