# Axiom (`.ax`)

Axiom is a systems language designed to be written, read, and repaired by machine agents.
Its main target is WebAssembly. Its design mandate is:

- **zero hidden control flow**: no exceptions, destructors, operator overloading, or implicit allocation
- **context-compact grammar**: LL(1), context-free, no macros, one canonical form
- **zero undefined behavior**: every operation has a defined result or a deterministic trap
- **first-class WASM/SIMD**: 128-bit vector primitives and typed inline WebAssembly
- **arena-centric memory**: no global allocator; every allocation names an `Allocator`

## Contents

| Path | What |
|---|---|
| [`spec/axiom.md`](spec/axiom.md) | Language specification (v0.1 working draft) |
| [`examples/dot_product.ax`](examples/dot_product.ax) | SIMD dot product with a bump arena and WASM exports |
| [`examples/shapes.ax`](examples/shapes.ax) | Tagged unions, `match`, errors, inline WASM |

The spec's [Appendix C](spec/axiom.md#appendix-c-changes-from-the-design-notes) lists the
decisions that differ from the original design notes.
[Appendix D](spec/axiom.md#appendix-d-open-questions) lists open questions for the next
iteration.
