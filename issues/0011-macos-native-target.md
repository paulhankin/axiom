# 0011: Native macOS binaries from the self-hosted compiler

- Type: feature
- Status: in progress (implemented; not yet run on a Mac)
- Area: compiler (selfhost): `target.ax`, `mlink.ax`, `a64.ax`; `runtime/darwin.ax`

Scope: Apple Silicon only (`aarch64-apple-darwin`). Apple has sold no Intel
Mac since 2023 and macOS 26 is the last release for them, so
`x86_64-apple-darwin` is not planned.

## Done

`--target=aarch64-macos` (also `aarch64-apple-darwin`, `macos`) builds a Mach-O
executable from any host (spec §14.1, selfhost/README.md):

- **Targets.** `target.ax` has an operating system beside the architecture;
  host detection reads the compiler's own executable (Mach-O magic means a
  Mac), so `--target=native` on a Mac picks this target. CPU `apple-m1`.
- **Linker.** `mlink.ax` links the compiler's ELF objects into a PIE
  executable against `/usr/lib/libSystem.B.dylib`: `__PAGEZERO`, `__TEXT`,
  `__DATA`, `__LINKEDIT` on 16 KiB pages; imports through stubs and a GOT
  bound with `LC_DYLD_INFO_ONLY` opcodes (dyld on macOS 11+ accepts these;
  chained fixups are not needed); rebases; the symbol table, `LC_MAIN`,
  `LC_UUID`, `LC_BUILD_VERSION` (minos 11.0); and an ad-hoc code signature.
- **ABI.** Symbols get a leading underscore. Variadic arguments go on the
  stack in 8-byte slots. `x18` is not used.
- **Thread-locals.** Instead of TLV descriptors, each thread's block (a copy
  of the template) hangs off a pthread key: `mrs TPIDRRO_EL0` gives the TSD
  array, the key indexes it. `ax_rt_args` creates the key and the main
  thread's block before any code runs; `ax_thread_spawn` does it for others.
- **Runtime.** `runtime/darwin.ax` implements the host ABI (spec §15.2) over
  `libSystem`: files, directories, processes (`posix_spawn`), threads,
  `__ulock_wait2`/`__ulock_wake`, sockets, `poll`, `kqueue` for file
  watching, `getentropy`, crash reports with stack traces. Error and signal
  numbers are translated to the Linux values the ABI uses, so std is
  unchanged.
- **Tests.** `tests/run.mjs` cross-builds a thread-local program, three
  examples and the compiler itself, and checks them with `tests/macho.mjs`
  (load commands, layout, every rebase and bind, symbols, entry point, every
  page hash). The constants and struct layouts in `darwin.ax` were checked
  against Apple's headers (with `zig cc -target aarch64-macos`), and every
  import against `libSystem.tbd`.

## Not done

- **Running it.** No Mac is available here. A macOS runner in CI (GitHub's
  `macos-14`, billed for private repositories) would run the test suite.
- `axiom test` and `axiom profile` for this target (`runtime/test_main.ax`
  and `runtime/profile.ax` are written against `linux.ax`).
- Linking C code, `-l` libraries and `extern "lib"` imports. That also needs
  Apple's packing of named arguments passed on the stack (natural alignment
  rather than 8 bytes); no `libSystem` function the runtime calls has more
  than eight arguments.
- File watching reports changes to a directory but not the names of the
  entries that changed (`kqueue` does not say). `notify` with a count wakes
  one or all waiters.

Related: 0012 (Windows).
