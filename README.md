# Axiom (`.ax`)

Axiom is a systems language designed to be written, read, and repaired by machine agents.
It compiles to native code for x86-64 and AArch64 (Linux, macOS and Windows) and to
WebAssembly. Its design mandate is:

- **zero hidden control flow**: no exceptions, destructors, operator overloading, or implicit allocation
- **context-compact grammar**: LL(1), context-free, no macros, one canonical form
- **zero undefined behavior**: every operation has a defined result or a deterministic trap
- **first-class SIMD**: 128-bit vector primitives, with the same (WebAssembly's) semantics on every target
- **explicit allocation**: one global allocator (`std/mem`, written in Axiom), called explicitly; no language construct allocates

## Quick start

```
cmd/axiom/bootstrap.sh                # build the compiler (needs llvm-18-dev and lld, for the bootstrap)
export PATH=$PWD/cmd/axiom/build:$PATH
axiom build examples/dot_product.ax -O2 -o engine.wasm
node runtime/host.mjs engine.wasm     # run a program (calls its `main` export)
axiom build examples/wordcount.ax --target=native -O2 -o wc
axiom test mylib.ax                # run a file's `test` blocks
axiom new geometry                 # a package: axiom.toml + src/lib.ax (spec §4.6)
axiom test geometry                # build/check/test also take a package directory
axiom profile prog.ax              # sampling profiler: hot functions, call sites, flame graph input
axiom build prog.ax --sanitize=address   # trap on use after free, double free, overflows; report leaks
axiom build prog.ax --target=native -O2 -g   # DWARF line tables, for perf annotate, gdb, addr2line -i
axiom build app --verbose          # modules compile separately and are cached (spec §16.7)
axiom build prog.ax --target=aarch64 --cpu=neoverse-n1 -o prog
axiom fmt src/                     # canonical layout, imports tidied (spec §16.5)
node tests/run.mjs                    # run the test suite
```

The compiler (`cmd/axiom/build/axiom`) is written in Axiom and has its own backends for
WebAssembly, x86-64 and AArch64 and its own linkers: it needs only the C library.
It builds native programs for Linux, Windows and macOS (Apple silicon, where it also
runs, tests and profiles them, and links system frameworks: AppKit, Metal, Core Audio).
On a Mac, `cmd/axiom/bootstrap.sh` needs `llvm@18` and `node` (Homebrew), with
`llvm-config` on the `PATH`.

## Contents

| Path | What |
|---|---|
| [`spec/axiom.md`](spec/axiom.md) | Language specification (v0.1 working draft) |
| [`docs/comparison.md`](docs/comparison.md) | Axiom compared with Zig, Rust, C++ and C |
| [`docs/security.md`](docs/security.md) | A security review of the standard library: findings, fixes and what is open |
| [`docs/performance.md`](docs/performance.md) | A performance review of the standard library, with comparisons to C libraries |
| [`issues/`](issues/README.md) | Known bugs and feature requests, one file each |
| [`examples/dot_product.ax`](examples/dot_product.ax) | SIMD dot product with the global allocator and WASM exports |
| [`examples/shapes.ax`](examples/shapes.ax) | Tagged unions, `match`, errors |
| [`examples/generics.ax`](examples/generics.ax) | Generic functions and types, constraints, const parameters |
| [`examples/c_interop/`](examples/c_interop/stats.ax) | An Axiom library called from C through a generated header |
| [`examples/pi.ax`](examples/pi.ax) | π to 2000 places (Machin's formula in big integers) on an A4 PDF page with typeset mathematics |
| [`examples/charts.ax`](examples/charts.ax) | Bar, line, scatter, stacked-area and donut charts (std/graphics/chart) drawn as one PNG, one SVG and a four-page PDF |
| [`examples/md2pdf.ax`](examples/md2pdf.ax) | Markdown to PDF (std/text/markdown/pdf): headings in the outline, lists, tables, code, links, TeX math and images |
| [`examples/metal.ax`](examples/metal.ax) | Metal on macOS: a window with a spinning triangle drawn by MSL shaders and a Core Text caption (std/os/cocoa, std/graphics/metal, std/graphics/quartz) |
| [`examples/directx.ax`](examples/directx.ax) | Direct3D 11 on Windows: a window with a spinning triangle drawn by HLSL shaders and a Direct2D caption (std/os/win32, std/graphics/d3d11, std/graphics/d2d) |
| [`examples/game.ax`](examples/game.ax) | A small game on Windows and macOS: Direct3D 12 or Metal graphics, keyboard and controllers (with rumble), sound effects and music (std/graphics/gpu, std/os/window, std/os/gamepad, std/audio/output) |
| [`examples/trench.ax`](examples/trench.ax) | A trench run in 3D on Windows and macOS, after the vector arcade games of the early 1980s: Direct3D 12 or Metal with a depth buffer, culling, additive glows, sun shadows from a shadow map and dynamic point lights, surfaces textured in the pixel shader, an autopilot |
| [`examples/swarm.ax`](examples/swarm.ax) | A 3D formation shooter on Windows and macOS in which everything is made of small cubes: voxel aliens with flashing lights, anything destroyed breaking into its cubes, shadows and point lights from shots and explosions, screen shake; one vertex buffer of cubes written in place each frame |
| [`examples/drumkin/`](examples/drumkin/src/main.ax) | A drum machine on macOS (a package): eight drums of FM and noise through a morphing state-variable filter and ADSR envelopes, two settings of each blended between, a 16-step sequencer with swing and accents, knobs drawn with Quartz over Metal, sound through Core Audio from a thread of its own |
| [`examples/pdfview/`](examples/pdfview/src/main.ax) | A PDF viewer on macOS (a package): continuous scrolling, a thumbnail sidebar, zoom from 10% to 800% that stays sharp (the part of a page in the window drawn again at full detail), rotated pages; Core Graphics draws the pages, the window is Quartz over Metal |
| [`examples/pinball/`](examples/pinball/README.md) | A pinball table in 3D on macOS (a package): the ball's physics at 2400 steps a second (spin, friction, rolling; solenoid flippers with inertia), a ramp onto a six-wire habitrail, bumpers, slingshots, drop and standup targets, a spinner, an orbit, a lock and three-ball multiball; Metal with PBR shading, HDR, bloom, shadows and a chrome ball reflecting the playfield; a dot-matrix display; synthesized sound |
| [`cmd/axiom/`](cmd/axiom/README.md) | The compiler, written in Axiom, with its own SSA IR, backends (wasm32/wasm64, x86-64, AArch64) and linkers: no LLVM, no C toolchain |
| [`cmd/axiom/bootstrap/`](cmd/axiom/bootstrap/README.md) | A compiler in C11 + LLVM, used only to bootstrap the one in `cmd/axiom/` (to a fixed point) |
| [`std/`](std/README.md) | Standard library. At the top: `mem`, `str`, `fmt`, `io`, `os`, `hash`, `unicode`, `math`, `time`, `thread`, `sync`, `net`, `audio` (WAV, AIFF, FLAC, Ogg Vorbis, Opus, MP3, AAC). Grouped: `collections/` (`vec`, `map`, `set`, …), `os/` (`fs`, `path`, `process`, …), `net/` (`http`, `http2`, `websocket`, `tls`, `dns`), `encoding/` (`json`, `toml`, `yaml`, `xml`, `cbor`, …), `text/` (`regex`, `html`, `markdown`, …), `compress/`, `archive/`, `math/`, `graphics/` (`image`, `draw`, `font`, `shape`, `text`, `pdf`, `svg`, `chart`), `crypto/` |
| [`runtime/`](runtime/host.mjs) | Host ABI implementations: node (`host.mjs`, also a runner) and POSIX (`native.ax` and `linux.ax`, libc-free, for the self-hosted compiler; `darwin.ax` for macOS (Apple silicon), `windows.ax` for Windows; `native_host.c` for the C one); test runners; the profiler |
| [`tests/`](tests/run.mjs) | Test suite: run-and-compare programs, expected compile errors, `axiom test`, packages |

The spec's [Appendix C](spec/axiom.md#appendix-c-changes-from-the-design-notes) lists the
decisions that differ from the original design notes.
[Appendix D](spec/axiom.md#appendix-d-open-questions) lists open questions for the next
iteration.
