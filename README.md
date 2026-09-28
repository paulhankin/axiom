# Phalange (`.ph`)

Phalange is a systems language designed to be written, read, and repaired by machine agents.
Its main target is WebAssembly. Its design mandate is:

- **zero hidden control flow**: no exceptions, destructors, operator overloading, or implicit allocation
- **context-compact grammar**: LL(1), context-free, no macros, one canonical form
- **zero undefined behavior**: every operation has a defined result or a deterministic trap
- **first-class WASM/SIMD**: 128-bit vector primitives with WebAssembly semantics
- **arena-centric memory**: no global allocator; every allocation names an `Allocator`

## Quick start

```
make -C compiler                      # needs llvm-18-dev, lld
./compiler/build/phalange build examples/dot_product.ph -O2 -o engine.wasm
node runtime/host.mjs engine.wasm     # run a program (calls its `main` export)
phalange test mylib.ph                # run a file's `test` blocks
phalange new geometry                 # a package: phalange.toml + src/lib.ph (spec §4.6)
phalange test geometry                # build/check/test also take a package directory
phalange profile prog.ph              # sampling profiler: hot functions, call sites, flame graph input
phalange build app --verbose          # modules compile separately and are cached (spec §16.7)
node tests/run.mjs                    # run the test suite
selfhost/bootstrap.sh                 # build the self-hosted compiler (stage 1, 2, 3)
selfhost/build/phalange build prog.ph --target=aarch64 --cpu=neoverse-n1 -o prog
```

The self-hosted compiler (`selfhost/build/phalange`) has its own backends for
WebAssembly, x86-64 and AArch64 and its own linkers: it needs only the C library.

## Contents

| Path | What |
|---|---|
| [`spec/phalange.md`](spec/phalange.md) | Language specification (v0.1 working draft) |
| [`examples/dot_product.ph`](examples/dot_product.ph) | SIMD dot product with a bump arena and WASM exports |
| [`examples/shapes.ph`](examples/shapes.ph) | Tagged unions, `match`, errors |
| [`examples/generics.ph`](examples/generics.ph) | Generic functions and types, constraints, const parameters |
| [`examples/c_interop/`](examples/c_interop/stats.ph) | A Phalange library called from C through a generated header |
| [`compiler/`](compiler/README.md) | The compiler: C11 + LLVM, targets wasm32 and native |
| [`selfhost/`](selfhost/README.md) | The same compiler written in Phalange, bootstrapped from the C one to a fixed point, with its own SSA IR, backends (wasm32/wasm64, x86-64, AArch64) and linkers: no LLVM, no C toolchain |
| [`std/`](std/README.md) | Standard library: `mem`, `vec`, `hash`, `map`, `matrix`, `io`, `net` |
| [`runtime/`](runtime/host.mjs) | Host ABI implementations: node (`host.mjs`, also a runner) and POSIX (`native.ph` for the self-hosted compiler, `native_host.c` for the C one); test runners; the profiler |
| [`tests/`](tests/run.mjs) | Test suite: run-and-compare programs, expected compile errors, `phalange test`, packages |

The spec's [Appendix C](spec/phalange.md#appendix-c-changes-from-the-design-notes) lists the
decisions that differ from the original design notes.
[Appendix D](spec/phalange.md#appendix-d-open-questions) lists open questions for the next
iteration.
