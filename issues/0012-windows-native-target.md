# 0012: Native Windows binaries

- Type: feature
- Status: open
- Area: compiler (selfhost and C); `runtime/`

Neither compiler builds Windows executables. Native output is Linux only
(spec §14.1). On Windows today, Axiom programs can be built for wasm32 and
run with `node runtime/host.mjs`, and since 0006 the compiler itself runs
that way too.

What a native Windows x64 (and later AArch64) target needs:

- **Object files and linking.** PE/COFF: section table, base relocations,
  the import directory (`kernel32.dll`, `ntdll.dll`, perhaps `ws2_32.dll`
  and `bcrypt.dll`), the exception directory (`.pdata`/`.xdata` unwind info,
  which Windows requires to unwind through and report crashes in any
  function), and a subsystem (console). A linker in Axiom, like `elink.ax`.
- **Calling convention.** The x86-64 backend implements only System V. The
  Windows x64 convention passes four arguments in `rcx`, `rdx`, `r8`, `r9`
  (one register sequence for integers and floats), needs 32 bytes of shadow
  space, makes `rdi`, `rsi` and `xmm6`-`xmm15` callee-saved, and passes
  aggregates larger than 8 bytes by reference. Stack probes (`__chkstk`
  equivalents) are needed for frames over 4 KiB.
- **Runtime.** A Windows implementation of the host ABI (spec §15.2) on the
  Win32 API: `CreateFileW` and UTF-16 paths, `CreateProcessW` (with Windows
  command-line quoting), threads (`CreateThread`), `WaitOnAddress`/`WakeByAddress*`
  for `@atomic_wait`/`notify`, Winsock, `ReadDirectoryChangesW` for watching,
  `BCryptGenRandom`, virtual memory with `VirtualAlloc` reserve/commit for
  the linear memory.
- **Traps.** Vectored exception handlers instead of signals, for trap
  reporting and stack traces.
- **Thread-locals.** The TLS directory and `_tls_index` (or keep thread
  state off TLS altogether).
- **The compiler itself.** It forks only for separate compilation and falls
  back to one unit without it (0006), so it would run on Windows once the
  runtime exists. Path handling assumes `/` separators throughout (std
  lookup, the module cache, `real_path`).
- **std.** `std/os/path` and `std/os/fs` assume POSIX paths and permissions;
  signals in `std/os` have no Windows meaning.

Larger than macOS (0011): a new calling convention, object format and
runtime, with no fork, no signals, and a different path model.

Testing needs a Windows machine or CI runner; Wine could run the result on
Linux for a first pass.
