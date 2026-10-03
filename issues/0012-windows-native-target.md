# 0012: Native Windows binaries

- Type: feature
- Status: in progress (x86-64 done; runs on Windows 11 and under Wine; see below)
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
- **DLLs and Direct3D.** `extern "C" "user32" { ... }` imports from that DLL
  (codegen leaves `axiom.import.<function>` for plink). std/os/win32 (windows,
  events) and std/graphics/d3d11 (Direct3D 11, DXGI swap chains, HLSL via
  d3dcompiler_47, COM through method tables); examples/directx.ax draws with
  them, tested offscreen pixel by pixel under Wine + Xvfb.
- **Direct3D 12, Direct2D, XAudio2, XInput.** std/graphics/d3d12 (a direct
  queue, root constants, pipeline state objects, flip-model swap chains,
  readback), std/graphics/hlsl (shared by 11 and 12), std/graphics/d2d
  (Direct2D + DirectWrite on a window or on a Direct3D 11 texture),
  std/audio/xaudio2 (voices playing std/audio Sounds) and std/os/xinput
  (pads, rumble, battery), on std/os/com. examples/directx.ax draws a
  Direct2D caption over Direct3D 11; examples/game.ax is a small game on
  Direct3D 12, XInput and XAudio2. Tested under Wine (Direct3D 12 through
  vkd3d on Mesa's lavapipe; XAudio2 into a PulseAudio null sink; XInput
  with no pads connected).
- **The compiler on Windows.** It builds for `x86_64-windows`, runs, and
  reaches a fixed point there: built by the Windows compiler, it builds
  itself bit-identically (under Wine). `util.real_path` keeps drive letters.
- **Unwind tables.** Every function with a frame has unwind information
  (.xdata) and an entry in the function table (.pdata, the exception
  directory); frameless ones are leaf functions as Windows defines them
  (no pushes, rsp untouched). So exceptions DLLs raise pass through Axiom
  frames, and Windows' stack walks (RtlCaptureStackBackTrace, debuggers)
  go through to the entry point: checked on Windows 11 with a crash five
  calls deep, at -O0 and -O2.
- **Debug symbols.** plink.ax writes a PDB (pdb.ax) next to the executable
  and names it in a CodeView debug directory entry (GUID: a hash of the
  image, so builds stay deterministic): every function as a procedure and a
  public symbol, the section headers and map, and line tables. Lines come
  from the source: every IR instruction carries a file and line
  (ir.src_loc; codegen sets it per expression, passes inherit it, the
  inliner copies it), so does every machine instruction, and the encoder
  records where it changes into the object's .axiom.lines (kept in cached
  objects), which plink turns into DEBUG_S_LINES. Checked with dbghelp
  (SymFromName, SymFromAddr, SymGetLineFromAddr64) and DIA (Dia2Dump) on the
  compiler itself; the self-build stays a fixed point, PDB included.
  `--pdb=off` writes none; test and profile builds write none.
- **Real Windows.** The compiler, examples/game.ax and the other examples run
  on Windows 11.
- **`axiom profile`.** `runtime/profile_windows.ax`: a sampler thread in place
  of SIGPROF. Threads register as they start (`windows.ax`'s `thread_hook`);
  at each tick (a high-resolution waitable timer) every thread is credited
  the cycles it ran (`QueryThreadCycleTime`, calibrated against the
  performance counter at startup), and one with a period's worth is
  suspended, its shadow stack copied, and resumed. So the counts are CPU
  time, as on Linux, across all threads, and waiting is not sampled. The
  profile is written at exit (`ax_rt_fini`, `ax_exit`), at a trap and at a
  crash. Checked on Windows 11 with tests/profile/hot.ax, three threads
  doing 2:1:1 work (one sleeping first), `os.exit`, a trap and a stack
  overflow.
- **More Direct3D.** std/graphics/d3d11: textures from images (two samplers),
  depth buffers, depth, cull and blend states, compute shaders over
  structured storage buffers (read back, or read by vertex shaders).
  std/graphics/d3d12: textures from images, compute root signatures and
  pipelines, storage buffers (read back, or drawn as vertices), and an
  Overlay through D3D11On12 so that std/graphics/d2d draws over its
  targets. std/graphics/hlsl: DXIL (shader model 6) through the DirectX
  Shader Compiler, loaded at run time (the PATH, `$AXIOM_DXC`, or the newest
  Windows SDK, with dxil.dll to sign the code). tests/windows/d3d11.ax and
  d3d12.ax check them pixel by pixel offscreen; they pass on Windows 11
  (an RTX 4080), and the window path (back buffers, present, resize) runs.

## Not done

- **The test suite on real Windows.** tests/run.mjs has run under Wine only
  (it needs node). A Windows runner in CI (`windows-latest`, billed for
  private repositories) would cover what Wine does differently. Every std
  module's tests pass with `axiom test` on Windows 11; what they found that
  Wine hides: `fs.temp_dir` (`%TMP%`, `%TEMP%`, `%USERPROFILE%`, not `/tmp`)
  and std/os/path's drives and `\` (`clean` made `C:\x` into `/C:\x`);
  sockets must be overlapped (`WSA_FLAG_OVERLAPPED`), since Windows
  serializes every request on a synchronous handle, so `getsockname` or the
  close in `Server.stop` waited for another thread's `accept`; a refused
  connection takes Windows two seconds (it retries the SYN), so
  `ax_tcp_connect` tries a host's addresses staggered, RFC 8305 style, and
  "localhost" (`::1` first) is not slow for a server on 127.0.0.1;
  `connect_unix` to a missing directory fails with WSAENETDOWN (refused);
  Hyper-V reserves TCP port ranges that UDP hands out (std/net/dns's test
  retries).
- Linking C code and `-l` libraries. Calls to C
  variadic functions would also need floating-point arguments copied to the
  integer registers.
- Signals: only SIGINT (Ctrl+C, Ctrl+Break) and SIGTERM (console close,
  logoff, shutdown) can be caught; `kill` ends a process.
- Symbolic links need Developer Mode or the privilege (Wine cannot make
  them); without it `symlink` fails with AccessDenied.
- Unix-domain sockets need Windows 10 1803 or later (Wine has none).
- Debug information beyond names and lines: types, local variables and
  parameters (S_LOCAL, S_DEFRANGE_* and the type stream), inline-site
  records for inlined calls (an inlined call's lines show as its callee's
  file, in the caller), source checksums.
- Direct3D: mipmaps, texture formats other than 8-bit RGBA, rendering into
  a texture and then sampling it, compute shaders writing textures, MSAA;
  the overlay under Wine (no D3D11On12 there). Windows.Gaming.Input, raw
  input and DirectInput for other controllers; WASAPI capture; Media
  Foundation. A portable graphics layer over Direct3D, Metal and Vulkan.
- AArch64 Windows.
- Separate compilation in the compiler on Windows: it forks, so it falls
  back to one unit (0006).

Related: 0011 (macOS).
