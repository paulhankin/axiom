# 0012: Native Windows binaries

- Type: feature
- Status: in progress (x86-64 done and tested under Wine; see below)
- Area: compiler (selfhost): `target.ax`, `x64.ax`, `plink.ax`; `runtime/windows.ax`

## Done

`--target=x86_64-windows` (also `x86_64-pc-windows-msvc`, `windows`) builds a
PE32+ console executable from any host (spec §14.1, selfhost/README.md):

- **Calling convention.** On Windows every function uses the Windows x64
  convention, so Windows can call back into Axiom code (threads, the
  exception handler, the console control handler) with no thunks: four
  arguments by position in `rcx`, `rdx`, `r8`, `r9` / `xmm0`-`xmm3`, 32
  bytes of shadow space, `rsi`, `rdi` and `xmm6`-`xmm15` callee-saved. The
  backend already probed large frames page by page.
- **Linker.** `plink.ax`: sections, the import directory (a function's DLL
  follows from its name: kernel32, ntdll, ws2_32, bcrypt), thunks, base
  relocations (`DYNAMIC_BASE`, high-entropy ASLR), the TLS directory, an
  entry point that calls `main` and `ExitProcess`.
- **Thread-locals.** The loader's: TEB.ThreadLocalStoragePointer
  (`gs:[0x58]`) indexed by `_tls_index`.
- **Runtime.** `runtime/windows.ax`: the whole host ABI over Win32 (UTF-16
  paths, a handle table, `CreateProcessW` with C-runtime quoting, pipes,
  Winsock with `WSAPoll`, `ReadDirectoryChangesW`, `RtlWaitOnAddress`,
  `VirtualAlloc` reserve/commit for linear memory, `BCryptGenRandom`), and a
  vectored exception handler for crash reports and stack traces.
- **`axiom test`.** Each test runs in a new process of the test program
  (`runtime/test_main_windows.ax`), sharing the AXT reporting with the Linux
  runner (`runtime/test_report.ax`). On Linux it runs under Wine.
- **std.** `os.system()` / `os.is_windows()` (a new host ABI function,
  `ax_os`); `path.is_abs` knows drives on Windows; the POSIX-specific std
  tests have Windows versions (cmd.exe instead of sh).
- **Testing.** The whole native suite passes on `x86_64-windows` under Wine 9
  (`AXIOM_NATIVE_TARGET=x86_64-windows AXIOM_NATIVE_EXEC=wine64`), except
  what is not supported (below) and tests that call C, which are skipped.
  The run tests use `tests/native_rt.ax` (via `--link=<file.ax>`, which now
  builds Axiom sources into the program) in place of the C harness.
- **The compiler on Windows.** It builds for `x86_64-windows`, runs, and
  reaches a fixed point there: built by the Windows compiler, it builds
  itself bit-identically (under Wine). `util.real_path` keeps drive letters.

## Not done

- **A real Windows machine.** Everything has run under Wine only. A Windows
  runner in CI (`windows-latest`, billed for private repositories) would
  cover what Wine does differently.
- `axiom profile` (runtime/profile.ax samples with SIGPROF).
- Linking C code, `-l` libraries and `extern "lib"` imports. Calls to C
  variadic functions would also need floating-point arguments copied to the
  integer registers.
- Signals: only SIGINT (Ctrl+C, Ctrl+Break) and SIGTERM (console close,
  logoff, shutdown) can be caught; `kill` ends a process.
- Symbolic links need Developer Mode or the privilege (Wine cannot make
  them); without it `symlink` fails with AccessDenied.
- Unix-domain sockets need Windows 10 1803 or later (Wine has none).
- No `.pdata` unwind information: debuggers and Windows' own stack walks see
  only leaf frames (Axiom's traces use its shadow stack).
- AArch64 Windows.
- Separate compilation in the compiler on Windows: it forks, so it falls
  back to one unit (0006).

Related: 0011 (macOS).
