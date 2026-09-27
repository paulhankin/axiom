# Phalange (`.ph`)

Phalange is a systems language designed to be written, read, and repaired by machine agents.
Its main target is WebAssembly. Its design mandate is:

- **zero hidden control flow**: no exceptions, destructors, operator overloading, or implicit allocation
- **context-compact grammar**: LL(1), context-free, no macros, one canonical form
- **zero undefined behavior**: every operation has a defined result or a deterministic trap
- **first-class WASM/SIMD**: 128-bit vector primitives and typed inline WebAssembly
- **arena-centric memory**: no global allocator; every allocation names an `Allocator`

## Quick start

```
make -C compiler                      # needs llvm-18-dev, lld
./compiler/build/phalange build examples/dot_product.ph -O2 -o engine.wasm
node tests/run.mjs                    # run the test suite
```

## Contents

| Path | What |
|---|---|
| [`spec/phalange.md`](spec/phalange.md) | Language specification (v0.1 working draft) |
| [`examples/dot_product.ph`](examples/dot_product.ph) | SIMD dot product with a bump arena and WASM exports |
| [`examples/shapes.ph`](examples/shapes.ph) | Tagged unions, `match`, errors, inline WASM |
| [`examples/generics.ph`](examples/generics.ph) | Generic functions and types, constraints, const parameters |
| [`compiler/`](compiler/README.md) | The compiler: C11 + LLVM, targets wasm32 and native |
| [`std/`](std/mem.ph) | Standard library (`std/mem`: `Allocator`, `Arena`, `FixedBuffer`, `page_allocator`) |
| [`tests/`](tests/run.mjs) | Test suite: run-and-compare programs and expected compile errors |

The spec's [Appendix C](spec/phalange.md#appendix-c-changes-from-the-design-notes) lists the
decisions that differ from the original design notes.
[Appendix D](spec/phalange.md#appendix-d-open-questions) lists open questions for the next
iteration.
