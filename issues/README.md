# Issues

Known bugs and feature requests, one file per issue, kept in the repository so
they travel with the code they describe.

## Conventions

- File name: `NNNN-short-slug.md`, numbered in order of filing. Numbers are
  never reused.
- Each file starts with a small header:

  ```
  # NNNN: Title
  - Type: bug | feature
  - Status: open | in progress | fixed (commit) | won't fix
  - Area: compiler (selfhost) | compiler (C) | std/<module> | tools | ...
  ```

  followed by what happens, how to reproduce it (for bugs), what is known about
  the cause, and possible approaches.
- When an issue is fixed, set its status to `fixed (<commit>)` and move its row
  to the Closed table below; keep the file as a record.

## Open

| # | Type | Area | Title |
|---|------|------|-------|
| [0003](0003-parallel-o2-builds.md) | feature | compiler | Parallel -O2 builds |
| [0004](0004-optimizer-speed.md) | feature | compiler | Faster optimizer |
| [0005](0005-compiler-memory.md) | feature | compiler | Further cuts to the compiler's memory |
| [0008](0008-math-missing-commands.md) | feature | std/text/math | Missing TeX commands: `\xrightarrow`, `\substack`, `\sideset`, ... |
| [0009](0009-markdown-github-math-syntax.md) | feature | std/text/markdown | GitHub's `` $`...`$ `` math syntax |
| [0010](0010-typeset-without-math-table.md) | feature | std/text/math/typeset | Better typesetting with fonts that lack a MATH table |
| [0011](0011-macos-native-target.md) | feature | compiler | Native macOS binaries from the self-hosted compiler |
| [0012](0012-windows-native-target.md) | feature | compiler, runtime | Native Windows binaries (x86-64 done; runs on Windows 11) |
| [0015](0015-wide-simd-and-cpu-dispatch.md) | feature | language, compiler, std | AVX-512, SVE and wider vectors; conditional compilation; runtime CPU detection and dispatch |
| [0016](0016-native-crypto-instructions.md) | feature | compiler, std/crypto | AES-NI, PCLMULQDQ, SHA and CRC instructions (x86-64 and AArch64) used in std/crypto with portable fallbacks |
| [0017](0017-windows-absolute-import-paths.md) | bug | compiler | An import of an absolute Windows path (`C:/...`) is taken as relative |

## Closed

| # | Type | Area | Title |
|---|------|------|-------|
| [0001](0001-wasm-one-local-per-value.md) | bug | compiler: wasm backend | Every value gets its own wasm local; deep recursion exhausts the stack at -O2 |
| [0002](0002-o2-std-not-tested.md) | bug | tests | The test suite never builds `std` at -O2 |
| [0007](0007-profile-program-arguments.md) | feature | compiler: `axiom profile` | `axiom profile` cannot pass arguments to the program |
| [0006](0006-selfhost-on-wasm32.md) | bug | compiler | The self-hosted compiler does not build for wasm32 |
| [0013](0013-c-compiler-runtime-on-macos.md) | feature | runtime | The C compiler's native runtime does not build on macOS |
| [0014](0014-wasm64-host.md) | bug | runtime | wasm64 programs do not run under `runtime/host.mjs` |
