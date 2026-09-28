# phalange — the Phalange compiler

A small, fast compiler for [Phalange](../spec/phalange.md), written in C11 with an LLVM
backend. It compiles `.ph` programs to WebAssembly modules (the primary target) or to
native executables.

[`selfhost/`](../selfhost/README.md) has a port of this compiler to Phalange, which this
one bootstraps. The port has the same front end (identical diagnostics, so a change to
one belongs in both) and its own backends instead of LLVM; programs must behave the same
with either compiler. `selfhost/difftest.sh` compares them.

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
phalange test  <file.ph> [options]    build and run the file's test blocks (spec §17)
                                      --filter=<text> --timeout=<ms> (default 10000)
phalange new   <name>                 create a package (phalange.toml, src/lib.ph)
phalange cache [path|clean]           the compiled-module cache
phalange profile <file.ph> [options]  build, run under the sampling profiler, report (spec §16.6)
                                      --rate=<hz> (default 999) --folded=<path>

build, check and test also take a package directory, or no path for the package in the
current directory (spec §4.6).

  --target=wasm32|wasm64|native   default wasm32 (alias: wasm32-freestanding)
  -O0 | -O1 | -O2 | -O3           optimization level; never changes program behavior
  --release                       integer overflow saturates (= --overflow=saturate)
  --overflow=trap|saturate        default: trap
  --emit=exe|obj|llvm|asm         default exe: a .wasm module or native executable
  --format=human|agent-json       diagnostics format (spec §16.2)
  --link=<file>                   C source, object or library to link in (repeatable). On wasm, C
                                  sources are compiled with clang for the same target
  -l<lib> -L<dir> -I<dir>         C libraries, library directories, include directories
  --stack-traces=on|off           call-site shadow stack for crash traces and profiles (default on)
  --units=module|program          compile modules separately, with a cache (default at -O0/-O1),
                                  or the program as one unit (default at -O2/-O3)
  --verbose                       report compiled and cached modules
  --emit-header=<path>            write a C header for the `export fn`s (also works with `check`)
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

**Tests** (spec §17): `test "name" { … }` blocks with `@expect` / `@expect_eq`.
`phalange test` runs each test isolated, with a timeout and captured output: a fresh
instance in a node worker (`runtime/test_runner.mjs`) or a forked process
(`runtime/test_main.c`). It reports human-readable text or `phalange-tests/1` JSON.
Every trap reports its code, the failing expression and its location, in tests and in
normal runs.

**Separate compilation** (spec §16.7, `src/units.c`): each module becomes its own object,
cached under `~/.cache/phalange/obj` by a key that hashes the compiler, the options, the
module's source and the keys of its imports. So an edit recompiles only that module and
its importers, and those are compiled in parallel worker processes. Program-wide pieces
(globals, error codes, runtime state, exports) go in a small link unit that is
regenerated on every build. Error codes are link-time symbols in module objects, so
adding an error name never invalidates the cache.

**Stack traces** (spec §16.4): every trap and crash (including stack overflow and
faults in C code) prints the call path with source locations, and failing tests show
it too, or, for a test that returned an error, the error return trace. They come
from a per-thread shadow stack of call sites. Each call costs three stores and a
load; `--stack-traces=off` removes them.

**Profiling** (spec §16.6): `phalange profile` samples the shadow stack, using a
CPU-clock `SIGPROF` natively (`runtime/profile.c`) or a worker reading shared memory
on wasm (`runtime/profile_worker.mjs`). It reports self and total time per function,
inclusive time per call site, and folded stacks for flame graphs.

**Packages** (spec §4.6, `src/package.c`): a `phalange.toml` manifest (a TOML subset)
with path dependencies and git dependencies pinned to a commit, fetched once into
`$PHALANGE_CACHE` (default `~/.cache/phalange`). Modules import them as `"pkg:name"` or
`"pkg:name/sub/mod"`; only declared dependencies are visible, and a build uses one
source per package name.

**C interop** (spec §14.5):
- `extern "C" { fn ...; }` calls C. On wasm, the C sources passed with `--link` are
  compiled to wasm and share the module's memory.
- `export fn` plus `--emit-header` makes Phalange callable from C. See
  `examples/c_interop/`.

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
lexer.c    whole-file tokenizer (maximal munch)
parser.c   LL(1) recursive descent -> AST; operator partial order (§7.4) resolved
           after a flat parse; E0150 with machine fixes
sema_*.c   module loading, name registration, type resolution (decl), expressions
           and generics (expr), builtins, statements / match / defer (stmt)
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
- host imports and exports
- the builtins in Appendix A
- trapping for the conditions in Appendix B
- threads and atomics (§12.8)

Not yet implemented (reported with a diagnostic where relevant):

- `phalange fmt` (only the self-hosted compiler has it: [`selfhost/`](../selfhost/README.md)),
  `phalange run` and `phalange explain` (spec §16.1).
- The debug trap map (`phalange.traps` custom section) and error-return traces
  (spec §16.4). Traps are plain `unreachable`, so the trap code isn't reported yet.
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
