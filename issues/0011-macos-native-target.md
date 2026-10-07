# 0011: Native macOS binaries from the self-hosted compiler

- Type: feature
- Status: done (runs on a Mac; bootstraps; the suite passes natively)
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

## Run on a Mac (macOS 26, Apple M4)

- The Mach-O output ran first time; the compiler bootstraps to a fixed point
  (stages 2 and 3 differ only in the code signature's identifier, the file
  name: `bootstrap.sh` now gives them the same name).
- **A miscompile** (every AArch64 target): `ptradd p, (mul x, 2^k)` was folded
  into an add whose scale is kept in a `u8`, so strides of 256 and more lost
  their multiplication (PDBs written on AArch64 hosts were scrambled).
  Fixed in `a64.ax`; `tests/run/large_strides.ax`.
- **std/mem** (every target): a large block that grew in place into the
  wilderness did not move `clean`, so `alloc_zeroed` later handed out that
  memory unzeroed. Fixed, with a test.
- `axiom test`: `runtime/test_main_darwin.ax` (fork, then exec of the test
  binary for each test: XPC services such as Metal's shader compiler do not
  work in a forked process; and a forked child had no signal stack).
- `axiom profile`: `runtime/profile_darwin.ax`, a sampler thread metering
  threads' CPU time through Mach (`thread_info`), suspending a thread to copy
  its shadow stack.
- File watching reports entries' names: each watched directory keeps a
  snapshot, diffed when kqueue says it changed, and each file in it has a
  watch of its own (IN_MODIFY, IN_ATTRIB).
- Frameworks and dylibs (`extern "C" "Metal"`), with other systems' libraries
  linked to `ax_unavailable`; std/os/objc, cocoa, gamecontroller,
  std/graphics/metal, quartz, std/audio/coreaudio, and the portable
  std/graphics/gpu, std/os/window, gamepad, std/audio/output. The examples
  game, trench and swarm run on both systems; examples/metal.ax is
  directx.ax's counterpart.

## Not done

- CI on a macOS runner.
- Linking C code and `-l` libraries. That also needs
  Apple's packing of named arguments passed on the stack (natural alignment
  rather than 8 bytes); no `libSystem` function the runtime calls has more
  than eight arguments.
- Structs passed to or returned from C functions (only Axiom's own
  convention, which matches Apple's for up to four doubles or two words).
- `notify` with a count wakes one or all waiters.

Related: 0012 (Windows).
