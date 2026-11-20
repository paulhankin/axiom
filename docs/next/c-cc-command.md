## C

- `axiom cc` is a C compiler with the conventional Unix cc command line, for the makefiles
  and configure scripts of C programs (`./configure CC="axiom cc"`, `make CC="axiom cc"`);
  the compiler installed or linked as `axcc` is the same. It takes `-c`, `-o`, `-E`, `-I`,
  `-D`, `-std=`, the `-O`, `-g`, `-W`, `-f` and `-m` options (mapping the meaningful ones,
  accepting the others, and refusing those it cannot honour), `-l`, `-L`, `-Wl,...`,
  `-shared`, `-pthread`, and the dependency options `-M`, `-MM`, `-MD`, `-MMD`, `-MF`,
  `-MT`, `-MQ`, `-MP`; its defaults are a cc's: the host for the target, `gnu17`, the
  system's C library where the target has one, position-independent code on Linux
  (docs/c.md, "The cc command").
- New options of `axiom build` for it: `--c-deps=` (with `--c-deps-target=`,
  `--c-deps-phony`, `--c-deps-no-system`) and `--emit=c-deps` (make rules of the headers a
  source reads), `--emit=c-macros`, `--c-line-markers` (line markers in `--emit=c-pp`,
  which a GNU dialect reads back as `#line`), `--c-include=`, `--c-isystem=`,
  `--c-source=`, `--c-warnings=on|off|error`, `--c-wrapv`; and of the Linux linker:
  `--soname=`, `--rpath=` (`DT_RUNPATH`), `--version-script=`, `--allow-undefined` (a
  shared library's undefined references left to the dynamic linker) and
  `--export-dynamic`.
- An executable keeps the definitions that a shared library it links with refers to (a
  callback the library calls), and a program of objects alone takes an object whatever its
  name (`x.lo`).
- `axiom cc -std=cNN` is `--c-std=gnuNN --c-iso-names`: a conventional cc's strict
  standard, whose programs keep `__attribute__`, `__asm__` and the other extensions in the
  reserved namespace (`--c-iso-names` makes `asm` and `typeof` identifiers, defines
  `__STRICT_ANSI__` and not `unix` or `linux`). Under an ISO standard with
  `--c-lib=system`, a use of the system headers' macro that defines `__attribute__` (or
  another such keyword) away is the warning `C4152`, not silent.
