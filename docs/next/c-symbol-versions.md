## C

- Shared libraries get symbol versions from their version scripts: named nodes
  (`LIB_1.0 { global: f; local: *; }; LIB_2.0 { g; } LIB_1.0;`) are the library's version
  definitions (`.gnu.version_d`, the base version being its soname) and each export has
  its node's version (`.gnu.version`), as the system's linker gives them. `extern "C++"`
  blocks are read, their patterns matched against demangled names.
- `.symver name, name2@VER` (and `@@VER`, `@@@VER`) in file-scope asm: a library exports
  an old version of a function beside the default one, and a program can refer to an old
  version.
- Programs and libraries record the versions they need of the libraries they link with
  (`.gnu.version_r`, now with its section headers, so `readelf -V` shows it), the system C library's
  among them: each reference is bound to the version its library defines as the default.
