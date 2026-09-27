# phalange — the Phalange compiler

A small, fast compiler for [Phalange](../spec/phalange.md), written in C11 with an LLVM
backend. It compiles `.ph` programs to WebAssembly modules (the primary target) or to
native executables.

```
$ make -C compiler
$ ./compiler/build/phalange build examples/dot_product.ph -O2 -o engine.wasm
$ ./compiler/build/phalange check examples/shapes.ph --format=agent-json
```

## Requirements

| Needed for | Package (Ubuntu 24.04) |
|---|---|
| building the compiler | a C11 compiler, `llvm-18-dev` (LLVM C API headers + `llvm-config`) |
| linking `.wasm` output | `lld` (`wasm-ld`) |
| linking native output | `cc` |
| running the test suite | `node` (≥ 18) |

## Usage

```
phalange build <file.ph> [options]    compile and link
phalange check <file.ph> [options]    parse and type-check only

  --target=wasm32|wasm64|native   default wasm32 (alias: wasm32-freestanding)
  -O0 | -O1 | -O2 | -O3           optimization level; never changes program behavior
  --release                       integer overflow saturates (= --overflow=saturate)
  --overflow=trap|saturate        default: trap
  --emit=exe|obj|llvm|asm         default exe: a .wasm module or native executable
  --format=human|agent-json       diagnostics format (spec §17.2)
  --link=<file>                   extra object/C file for the linker (repeatable)
  --std=<dir>                     standard library directory (default: found next to the binary,
                                  or $PHALANGE_STD)
  -o <path>                       output file
```

Exit codes: `0` success, `1` the program has errors (or linking failed), `2` bad invocation.
With `--format=agent-json`, stdout holds exactly one JSON object in the spec's
`phalange-diagnostics/1` schema, including machine-applicable fixes.

Run a wasm program with `node runtime/host.mjs prog.wasm`. This provides the host ABI
used by `std/io` and `std/net` and calls the `main` export. Native programs that use
the host ABI get `runtime/native_host.c` linked in automatically.

Programs that use `std/thread` are linked with a shared, imported memory, and export
`phalange_thread_start`. `runtime/host.mjs` creates the memory and runs each thread as
a node worker. Native builds use pthreads. Atomics are always available.

On wasm, `export fn` functions become WebAssembly exports, `import host "mod" { ... }`
functions become imports from `mod`, and the linear memory is exported as `memory`.
The shadow stack is placed first in memory (`--stack-first`), so a stack overflow
becomes an out-of-bounds trap instead of corrupting data.

## Tests

```
$ node tests/run.mjs              # wasm32, -O0
$ node tests/run.mjs -O2
$ node tests/run.mjs --native     # native target (links tests/native_rt.c)
$ make -C compiler asan && PHALANGE=compiler/build-asan/phalange node tests/run.mjs
```

`tests/run/*.ph` are run-and-compare tests. `// expect:` lines give the expected output,
`// expect-trap` means `run` must trap, and `// flags:` adds compiler options.
`tests/errors/*.ph` must fail to compile with the `// expect-error:` codes. The suite
passes on wasm32 and native at every optimization level, and under ASan/UBSan.

## Architecture

```
lexer.c    whole-file tokenizer (maximal munch; lexical mode switch inside wasm { })
parser.c   LL(1) recursive descent -> AST; operator partial order (§7.4) resolved
           after a flat parse; E0150 with machine fixes
sema_*.c   module loading, name registration, type resolution (decl), expressions
           and generics (expr), builtins, statements / match / defer / inline-wasm
           validation (stmt)
types.c    hash-consed types (type equality is pointer equality), substitution,
           layout (spec §5), constraints
codegen.c  LLVM IR via the C API; monomorphization; LLVM passes; object emission
main.c     driver: sema -> codegen -> wasm-ld / cc
```

Design choices that keep it fast and small:

- **Arena allocation and interning.** Nothing is freed individually, and identifiers
  are compared by pointer.
- **Generics are checked once and instantiated in codegen.** Each generic body is
  type-checked a single time against its constraints (spec §10.5), with type
  parameters as abstract types. Codegen then emits one LLVM function per distinct
  `(declaration, type arguments)` pair by substituting the arguments into the
  already-typed AST.
- **Lazy code generation.** Only functions reachable from `export fn`s (and the
  globals they touch) are lowered, so unused library code costs nothing in LLVM.
- **Friendly to LLVM's -O0 fast path.** Overflow checks use plain instructions instead
  of `llvm.*.with.overflow`, and traps go through one cold helper function instead of
  inline `llvm.trap`. Both keep the wasm backend on its fast instruction selector. At
  `-O1` and above LLVM turns them back into the efficient forms.
- **No runtime library.** The module provides its own `__multi3` for 64-bit overflow
  checks on wasm, and uses bulk-memory instructions for copies and fills, so
  `wasm32-freestanding` output links with no libc and no compiler-rt.

Rough speed on a 4-core container: type-checking 30,000 lines takes about 0.08 s. A
full `-O0` build of 2,000 functions takes about 0.8–1.5 s, which is as fast as
`clang -O0` on equivalent C. Nearly all of that time is LLVM's backend.

## Status against the specification

Implemented: the whole language described in spec v0.1-draft.3:
- lexical structure, grammar, operator partial order
- modules and imports, `pub` / `export`, the no-shadowing rule
- all primitive, pointer, slice, array, struct, union, error-union and function-pointer
  types, with the spec's layouts
- implicit and explicit conversions, both overflow modes, mathematical shifts
- `defer`, `match` exhaustiveness, `!` / `catch`
- generics with constraints, deduction and `const` parameters
- SIMD types and builtins
- inline WASM, host imports and exports
- the builtins in Appendix A
- trapping for the conditions in Appendix B
- threads and atomics (§12.8)

Not yet implemented (reported with a diagnostic where relevant):

- `phalange fmt`, `phalange run` and `phalange explain` (spec §17.1).
- The debug trap map (`phalange.traps` custom section) and error-return traces
  (spec §17.4). Traps are plain `unreachable`, so the trap code isn't reported yet.
- Structured control flow (`block` / `loop` / `if` / `br`) inside inline wasm (`E0903`).
- Global initializers containing union values with a payload (`E0353`).
- **Native targets use real addresses for raw pointers.** Slices, arrays and all
  checked operations behave as on wasm, but a raw pointer that points outside mapped
  memory crashes with a signal instead of trapping deterministically. The
  bounds-checked linear-memory region of spec §12.1 is future work.
  `@memory_grow` / `@memory_size` are emulated on native with a 256 MiB reserved region.
- **Data races at -O1 and above.** The spec (§12.8.2) forbids optimizations that are
  valid only for race-free programs. At `-O0` the generated code honors this. At `-O1`
  and above the compiler relies on LLVM, whose IR memory model formally treats a
  racing non-atomic load as `undef`. In practice LLVM rarely exploits this, but it
  isn't guaranteed. The planned fix is to lower accesses that may be shared as
  `unordered` atomics in threaded builds.
- `@atomic_wait` / `@atomic_notify` on native targets use Linux futexes. Other POSIX
  systems aren't supported yet.
- The prelude errors `OutOfMemory` and `Overflow` are always declared, because builtins
  and `std/mem` produce them.
