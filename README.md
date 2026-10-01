# Axiom (`.ax`)

Axiom is a systems language designed to be written, read, and repaired by machine agents.
Its main target is WebAssembly. Its design mandate is:

- **zero hidden control flow**: no exceptions, destructors, operator overloading, or implicit allocation
- **context-compact grammar**: LL(1), context-free, no macros, one canonical form
- **zero undefined behavior**: every operation has a defined result or a deterministic trap
- **first-class WASM/SIMD**: 128-bit vector primitives with WebAssembly semantics
- **explicit allocation**: one global allocator (`std/mem`, written in Axiom), called explicitly; no language construct allocates

## Quick start

```
make -C compiler                      # needs llvm-18-dev, lld
./compiler/build/axiom build examples/dot_product.ax -O2 -o engine.wasm
node runtime/host.mjs engine.wasm     # run a program (calls its `main` export)
axiom test mylib.ax                # run a file's `test` blocks
axiom new geometry                 # a package: axiom.toml + src/lib.ax (spec §4.6)
axiom test geometry                # build/check/test also take a package directory
axiom profile prog.ax              # sampling profiler: hot functions, call sites, flame graph input
axiom build prog.ax --sanitize=address   # trap on use after free, double free, overflows; report leaks
axiom build app --verbose          # modules compile separately and are cached (spec §16.7)
node tests/run.mjs                    # run the test suite
selfhost/bootstrap.sh                 # build the self-hosted compiler (stage 1, 2, 3)
selfhost/build/axiom build prog.ax --target=aarch64 --cpu=neoverse-n1 -o prog
selfhost/build/axiom fmt src/      # canonical layout, imports tidied (spec §16.5)
```

The self-hosted compiler (`selfhost/build/axiom`) has its own backends for
WebAssembly, x86-64 and AArch64 and its own linkers: it needs only the C library.

## Contents

| Path | What |
|---|---|
| [`spec/axiom.md`](spec/axiom.md) | Language specification (v0.1 working draft) |
| [`docs/comparison.md`](docs/comparison.md) | Axiom compared with Zig, Rust, C++ and C |
| [`docs/security.md`](docs/security.md) | A security review of the standard library: findings, fixes and what is open |
| [`docs/performance.md`](docs/performance.md) | A performance review of the standard library, with comparisons to C libraries |
| [`examples/dot_product.ax`](examples/dot_product.ax) | SIMD dot product with the global allocator and WASM exports |
| [`examples/shapes.ax`](examples/shapes.ax) | Tagged unions, `match`, errors |
| [`examples/generics.ax`](examples/generics.ax) | Generic functions and types, constraints, const parameters |
| [`examples/c_interop/`](examples/c_interop/stats.ax) | A Axiom library called from C through a generated header |
| [`compiler/`](compiler/README.md) | The compiler: C11 + LLVM, targets wasm32 and native |
| [`selfhost/`](selfhost/README.md) | The same compiler written in Axiom, bootstrapped from the C one to a fixed point, with its own SSA IR, backends (wasm32/wasm64, x86-64, AArch64) and linkers: no LLVM, no C toolchain |
| [`std/`](std/README.md) | Standard library. At the top: `mem`, `str`, `fmt`, `io`, `os`, `hash`, `unicode`, `math`, `time`, `thread`, `sync`, `net`, `audio` (WAV, AIFF, FLAC, Ogg Vorbis, Opus, MP3, AAC). Grouped: `collections/` (`vec`, `map`, `set`, …), `os/` (`fs`, `path`, `process`, …), `net/` (`http`, `http2`, `websocket`, `tls`, `dns`), `encoding/` (`json`, `toml`, `yaml`, `xml`, `cbor`, …), `text/` (`regex`, `html`, `markdown`, …), `compress/`, `archive/`, `math/`, `graphics/` (`image`, `draw`, `font`, `shape`, `text`, `pdf`), `crypto/` |
| [`runtime/`](runtime/host.mjs) | Host ABI implementations: node (`host.mjs`, also a runner) and POSIX (`native.ax` and `linux.ax`, libc-free, for the self-hosted compiler; `native_host.c` for the C one); test runners; the profiler |
| [`tests/`](tests/run.mjs) | Test suite: run-and-compare programs, expected compile errors, `axiom test`, packages |

The spec's [Appendix C](spec/axiom.md#appendix-c-changes-from-the-design-notes) lists the
decisions that differ from the original design notes.
[Appendix D](spec/axiom.md#appendix-d-open-questions) lists open questions for the next
iteration.
