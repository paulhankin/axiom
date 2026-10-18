# Axiom (`.ax`)

Axiom is a systems language designed to be written, read, and repaired by machine agents.
It compiles to native code for x86-64 and AArch64 (Linux, macOS and Windows) and to
WebAssembly. Its design mandate is:

- **zero hidden control flow**: no exceptions, destructors, operator overloading, or implicit allocation
- **context-compact grammar**: LL(1), context-free, no macros, one canonical form
- **no undefined behavior outside raw pointers**: arithmetic, slices, arrays and every other checked
  operation have a defined result or a deterministic trap; a raw pointer must stay inside its object (spec §12.1)
- **first-class SIMD**: vector primitives of 128, 256 and 512 bits, with the same (WebAssembly's) semantics on every target
- **explicit allocation**: one global allocator (`std/mem`, written in Axiom), called explicitly; no language construct allocates

## Install

Download the archive for your platform from the
[releases page](https://github.com/paulhankin/axiom/releases), unpack it and put its
`bin/` on the `PATH`; [`docs/install.md`](docs/install.md) has the details, the platform
notes and how to build from source.

```sh
tar -xzf axiom-0.1.0-linux-x86_64.tar.gz
export PATH="$PWD/axiom-0.1.0-linux-x86_64/bin:$PATH"
axiom version
```

[`docs/tour.md`](docs/tour.md) is a guided tour of the language in a few pages, with
programs the test suite builds.

## A first program

```axiom
import "std/io" as io;

export fn main() i32 {
    io.stdout().write_all("hello, world\n") catch return 1;
    return 0;
}
```

```sh
axiom build hello.ax --target=native -o hello && ./hello
axiom build hello.ax -o hello.wasm                 # WebAssembly; run it with node and std/internal/runtime/host.mjs
```

## Commands

```
axiom build prog.ax --target=native -O2 -o prog   # native code for this machine (also x86_64, aarch64, aarch64-macos, x86_64-windows)
axiom build prog.ax -O2 -o prog.wasm              # WebAssembly (wasm32; also wasm64)
axiom build prog.ax --target=aarch64 --cpu=neoverse-n1 -o prog
axiom test mylib.ax                               # run a file's `test` blocks
axiom new geometry                                # a package: axiom.toml + src/lib.ax (spec §4.6)
axiom test geometry                               # build, check and test also take a package directory
axiom fmt src/                                    # canonical layout, imports tidied (spec §16.5)
axiom profile prog.ax                             # sampling profiler: hot functions, call sites, flame graphs
axiom build prog.ax --sanitize=address            # trap on use after free, double free, overflows; report leaks
axiom build app --verbose                         # modules compile separately and are cached (spec §16.7)
axiom lsp                                         # the language server, for editors (docs/editors.md)
```

[`docs/cli.md`](docs/cli.md) lists every command, flag and environment variable.

The compiler is written in Axiom and has its own backends (WebAssembly, x86-64, AArch64)
and linkers, so it and the programs it builds need only the C library. It builds native
programs for Linux, Windows and macOS on Apple silicon (where it also runs, tests and
profiles them, and links system frameworks such as AppKit, Metal and Core Audio).
Building it from source takes the bootstrap compiler, which needs LLVM 18
([`docs/install.md`](docs/install.md)); `node cmd/axiom/internal/tests/run.mjs` runs
the test suite.

## Repository

| Path | What |
|---|---|
| [`docs/install.md`](docs/install.md) | Installing a release, a first program, building from source |
| [`docs/tour.md`](docs/tour.md) | A tour of the language: values, slices, structs and unions, errors, memory, generics, tests, SIMD |
| [`docs/cli.md`](docs/cli.md) | The `axiom` command: every command and flag, environment variables, the cache, exit codes |
| [`docs/editors.md`](docs/editors.md) | Editor support: the VS Code extension, and `axiom lsp` with Neovim, Helix and Emacs |
| [`docs/spec.md`](docs/spec.md) | The language specification (v0.1 working draft) |
| [`docs/comparison.md`](docs/comparison.md) | Axiom compared with Zig, Rust, C++ and C |
| [`docs/releasing.md`](docs/releasing.md) | How releases are made, versioned and packaged; their notes are in [`docs/releases/`](docs/releases) |
| [`docs/security.md`](docs/security.md) | How the standard library behaves on untrusted input: its rules, defences and checks |
| [`cmd/axiom/`](cmd/axiom/README.md) | The compiler, written in Axiom: its own SSA IR, backends (wasm32/wasm64, x86-64, AArch64) and linkers; no LLVM, no C toolchain |
| [`cmd/axiom/bootstrap/`](cmd/axiom/bootstrap/README.md) | A compiler in C11 with LLVM, used only to bootstrap the one above (to a fixed point) |
| [`cmd/axiom/internal/tests/`](cmd/axiom/internal/tests/run.mjs) | The test suite: run-and-compare programs, expected compile errors, `axiom test`, packages; its `tools/` generates test data and holds the std fuzzer and benchmark |
| [`cmd/internal/tools/`](cmd/internal/tools) | Generators of the standard library's checked-in tables (Unicode, codecs, fonts) |
| [`std/`](std/README.md) | The standard library. At the top: `mem`, `str`, `fmt`, `io`, `os`, `hash`, `unicode`, `math`, `time`, `thread`, `sync`, `net`, `audio` (WAV, AIFF, FLAC, Ogg Vorbis, Opus, MP3, AAC). Grouped: `collections/` (`vec`, `map`, `set`, …), `os/` (`fs`, `path`, `process`, …), `net/` (`http`, `http2`, `websocket`, `tls`, `dns`), `encoding/` (`json`, `toml`, `yaml`, `xml`, `cbor`, …), `text/` (`regex`, `html`, `markdown`, …), `compress/`, `archive/`, `math/`, `graphics/` (`image`, `draw`, `font`, `shape`, `text`, `pdf`, `svg`, `chart`), `crypto/` |
| [`std/internal/runtime/`](std/internal/runtime/host.mjs) | The runtime: the host ABI for node (`host.mjs`, also a runner), Linux (`native.ax`, `linux.ax`, no libc), macOS (`darwin.ax`) and Windows (`windows.ax`); test runners; the profiler |
| [`examples/`](examples) | Example programs and packages (below) |
| [`tools/`](tools/editors/vscode/README.md) | Editor support: a VS Code extension (highlighting, and diagnostics and navigation through `axiom lsp`) |

Known bugs and feature requests are [GitHub issues](https://github.com/paulhankin/axiom/issues);
[`CONTRIBUTING.md`](CONTRIBUTING.md) says how the repository is built, tested and
changed. To report a vulnerability, see [`SECURITY.md`](SECURITY.md).

## Examples

| Path | What |
|---|---|
| [`examples/dot_product/dot_product.ax`](examples/dot_product/dot_product.ax) | SIMD dot product with the global allocator and WASM exports |
| [`examples/shapes/shapes.ax`](examples/shapes/shapes.ax) | Tagged unions, `match`, errors |
| [`examples/generics/generics.ax`](examples/generics/generics.ax) | Generic functions and types, constraints, const parameters |
| [`examples/c_interop/`](examples/c_interop/stats.ax) | An Axiom library called from C through a generated header |
| [`examples/wordcount/wordcount.ax`](examples/wordcount/wordcount.ax) | Word frequencies of stdin, for WebAssembly or native |
| [`examples/http_server/http_server.ax`](examples/http_server/http_server.ax) | A small JSON API over HTTP (std/net/http): notes listed, added, read and deleted |
| [`examples/texmath/texmath.ax`](examples/texmath/texmath.ax) | TeX math typeset as SVG, PNG or PDF with a math font, or as MathML or Unicode text |
| [`examples/pi/pi.ax`](examples/pi/pi.ax) | π to 2000 places (Machin's formula in big integers) on an A4 PDF page with typeset mathematics |
| [`examples/charts/charts.ax`](examples/charts/charts.ax) | Bar, line, scatter, stacked-area and donut charts (std/graphics/chart) drawn as one PNG, one SVG and a four-page PDF |
| [`examples/md2pdf/md2pdf.ax`](examples/md2pdf/md2pdf.ax) | Markdown to PDF (std/text/markdown/pdf): headings in the outline, lists, tables, code, links, TeX math and images |
| [`examples/metal/metal.ax`](examples/metal/metal.ax) | Metal on macOS: a window with a spinning triangle drawn by MSL shaders and a Core Text caption (std/os/cocoa, std/graphics/metal, std/graphics/quartz) |
| [`examples/directx/directx.ax`](examples/directx/directx.ax) | Direct3D 11 on Windows: a window with a spinning triangle drawn by HLSL shaders and a Direct2D caption (std/os/win32, std/graphics/d3d11, std/graphics/d2d) |
| [`examples/game/game.ax`](examples/game/game.ax) | A small game on Windows and macOS: Direct3D 12 or Metal graphics, keyboard and controllers (with rumble), sound effects and music (std/graphics/gpu, std/os/window, std/os/gamepad, std/audio/output) |
| [`examples/trench/trench.ax`](examples/trench/trench.ax) | A trench run in 3D on Windows and macOS, after the vector arcade games of the early 1980s: Direct3D 12 or Metal with a depth buffer, culling, additive glows, sun shadows from a shadow map and dynamic point lights, surfaces textured in the pixel shader, an autopilot |
| [`examples/swarm/swarm.ax`](examples/swarm/swarm.ax) | A 3D formation shooter on Windows and macOS in which everything is made of small cubes: voxel aliens with flashing lights, anything destroyed breaking into its cubes, shadows and point lights from shots and explosions, screen shake; one vertex buffer of cubes written in place each frame |
| [`examples/drumkin/`](examples/drumkin/src/main.ax) | A drum machine on macOS (a package): eight drums of FM and noise through a morphing state-variable filter and ADSR envelopes, two settings of each blended between, a 16-step sequencer with swing and accents, knobs drawn with Quartz over Metal, sound through Core Audio from a thread of its own |
| [`examples/pdfview/`](examples/pdfview/src/main.ax) | A PDF viewer on macOS (a package): continuous scrolling, a thumbnail sidebar, zoom from 10% to 800% that stays sharp (the part of a page in the window drawn again at full detail), rotated pages; Core Graphics draws the pages, the window is Quartz over Metal |
| [`examples/pinball/`](examples/pinball/README.md) | A pinball table in 3D on macOS (a package): the ball's physics at 2400 steps a second (spin, friction, rolling; solenoid flippers with inertia), a ramp onto a six-wire habitrail, bumpers, slingshots, drop and standup targets, a spinner, an orbit, a lock and three-ball multiball; Metal with PBR shading, HDR, bloom, shadows and a chrome ball reflecting the playfield; a dot-matrix display; synthesized sound |

## Disclaimer

Axiom is experimental software, provided "as is", without warranty of any kind, express
or implied. Use it at your own risk. The language, the compiler and the standard library
are young, change without notice (releases make no compatibility promise), and have not
been audited: they may contain bugs that produce wrong programs, wrong results or
security vulnerabilities. Do not rely on them where failure could cause harm, loss of
data or money, or injury. The authors are not liable for any damage arising from their
use; see the licenses below for the full terms.

## License

Axiom is licensed under either of the [MIT license](LICENSE-MIT) or the
[Apache License, Version 2.0](LICENSE-APACHE), at your option. Contributions are accepted
under the same terms, unless stated otherwise.

Some of the standard library is derived from other projects and keeps their terms; see
[`THIRD-PARTY-NOTICES.md`](THIRD-PARTY-NOTICES.md). In particular the AAC decoder
(`std/audio/aac.ax` and `std/audio/internal/aac/`) follows FFmpeg's and is licensed under the LGPL 2.1 or later; a program
includes it only if it imports `std/audio/aac` (`std/audio/formats` reads AAC only once
the program adds `aac.DECODER` to it).
