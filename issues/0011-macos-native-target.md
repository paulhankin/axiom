# 0011: Native macOS binaries from the self-hosted compiler

- Type: feature
- Status: open
- Area: compiler (selfhost): `target.ax`, `elf.ax`/`elink.ax`, `a64.ax`, `x64.ax`; `runtime/`

The self-hosted compiler builds native programs for Linux only
(`x86_64-unknown-linux-gnu`, `aarch64-unknown-linux-gnu`; spec §14.1). There
is no way to produce a Mach-O executable for macOS.

What is reusable: the IR, the optimizer, instruction selection and register
allocation (`mach.ax`), and the x86-64 and AArch64 encoders.

What is missing:

- **Targets.** `target.ax` knows only the Linux triples. It needs
  `aarch64-apple-darwin` and `x86_64-apple-darwin`, a way to pick them
  (`--target=aarch64-macos`, ...), and `host_triple()` on a Mac.
- **Object files and linking.** The compiler writes ELF objects and links them
  itself (`elf.ax`, `elink.ax`, `lstub.ax`). macOS needs Mach-O: sections and
  segments, relocations (`ARM64_RELOC_*`, `X86_64_RELOC_*`), the symbol and
  string tables, `LC_MAIN`, `LC_LOAD_DYLINKER` and `LC_LOAD_DYLIB` for
  `/usr/lib/libSystem.B.dylib`, chained fixups or dyld info for imports, and
  `LC_CODE_SIGNATURE`. Apple Silicon refuses to run unsigned code, but an
  ad-hoc signature (a SHA-256 hash of each page) is enough and can be written
  without Apple tools.
- **Runtime.** `runtime/native.ax` makes Linux system calls directly
  (`runtime/linux.ax`). Apple does not support raw system calls: a Darwin
  runtime must call `libSystem` for files, processes (`posix_spawn`),
  threads (`pthread_create`), waiting (`os_sync_wait_on_address`, or
  `__ulock_wait` before macOS 14.4), sockets, `kqueue` for poll and file
  watching, and `getentropy`. The host ABI (spec §15.2) does not change, so
  std needs no changes.
- **ABI details.** Symbols get a leading underscore. Apple's AArch64 ABI
  differs from AAPCS64: variadic arguments always go on the stack, arguments
  passed on the stack are packed to their natural alignment rather than 8
  bytes, and `x18` is reserved. Thread-locals use TLV descriptors (`__thread_vars`,
  a call through `tlv_get_addr`) rather than ELF TLS. That affects `threadlocal`
  globals and the shadow stack's per-thread record.
- **Traps.** Trap reporting and stack traces hook `SIGILL`/`SIGTRAP`/`SIGSEGV`
  via `sigaction`. The Darwin layouts of `ucontext`/`mcontext` differ from
  Linux's.
- **Testing.** No Mac is available here; CI on a macOS runner would be needed.
  Cross-building from Linux is possible once the linker exists. Running the
  result is not.

Steps: a Mach-O writer and linker, proved with a "hello" that calls `write`
and `exit` in `libSystem`; then the Darwin runtime; then the ABI details;
then the test suite on a Mac.

Related: 0012 (Windows).
