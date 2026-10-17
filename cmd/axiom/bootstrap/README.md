# The bootstrap compiler

A compiler for [Axiom](../../../docs/spec.md) written in C11 with an LLVM backend. Its
job is to build the Axiom compiler in [`cmd/axiom/`](../README.md) (see
`cmd/axiom/bootstrap.sh`); use that one for everything else. It compiles `.ax` programs
to native executables or to WebAssembly modules.

[`cmd/axiom/`](../README.md) is the same compiler written in Axiom, which this
one bootstraps. The port has the same front end (identical diagnostics, so a change to
one belongs in both) and its own backends instead of LLVM; programs must behave the same
with either compiler. `cmd/axiom/difftest.sh` compares them.

```
$ make -C cmd/axiom/bootstrap
$ cmd/axiom/bootstrap/build/axiom build examples/dot_product.ax -O2 -o engine.wasm
$ cmd/axiom/bootstrap/build/axiom check examples/shapes.ax --format=agent-json
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
axiom build <file.ax> [options]    compile and link
axiom check <file.ax> [options]    parse and type-check only
axiom test  <file.ax> [options]    build and run the file's test blocks (spec §17)
                                      --filter=<text> --timeout=<ms> (default 10000)
axiom new   <name>                 create a package (axiom.toml, src/lib.ax)
axiom cache [path|clean]           the compiled-module cache
axiom profile <file.ax> [options]  build, run under the sampling profiler, report (spec §16.6)
                                      --rate=<hz> (default 999) --folded=<path>

build, check and test also take a package directory, or no path for the package in the
current directory (spec §4.6).

  --target=wasm32|wasm64|native   default wasm32 (alias: wasm32-freestanding)
  --cpu=<preset>                  processor extensions: native (the default for native), a baseline, or a
                                  preset of cmd/axiom/README.md (spec §13.4)
  --features=+ext,-ext            add or remove extensions
  -O0 | -O1 | -O2 | -O3           optimization level; never changes program behavior
  --overflow=wrap|trap|saturate   integer overflow (spec §7.6): wraps by default; trap (T0002) is
                                  an opt-in check for unintended overflows
  --release                       accepted for compatibility; changes nothing (was = saturate)
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
                                  or $AXIOM_STD)
  -o <path>                       output file
```

Exit codes: `0` success, `1` the program has errors (or linking failed), `2` bad invocation.
With `--format=agent-json`, stdout holds exactly one JSON object in the spec's
`axiom-diagnostics/1` schema, including machine-applicable fixes.

Run a wasm program with `node std/internal/runtime/host.mjs prog.wasm`. This provides the host ABI
used by `std/io` and `std/net` and calls the `main` export. Native programs that use
the host ABI get `std/internal/runtime/native_host.c` linked in automatically.

**Tests** (spec §17): `test "name" { … }` blocks with `@expect` / `@expect_eq`.
`axiom test` runs each test isolated, with a timeout and captured output: a fresh
instance in a node worker (`std/internal/runtime/test_runner.mjs`) or a forked process
(`std/internal/runtime/test_main.c`). It reports human-readable text or `axiom-tests/1` JSON.
Every trap reports its code, the failing expression and its location, in tests and in
normal runs.

**Separate compilation** (spec §16.7, `src/units.c`): each module becomes its own object,
cached under `~/.cache/axiom/obj` by a key that hashes the compiler, the options, the
module's source and the keys of its imports. So an edit recompiles only that module and
its importers, and those are compiled in parallel worker processes. Program-wide pieces
(globals, error codes, runtime state, exports) go in a small link unit that is
regenerated on every build. Error codes are link-time symbols in module objects, so
adding an error name never invalidates the cache.

**Stack traces** (spec §16.4): every trap and crash (including stack overflow and
faults in C code) prints the call path with source locations, and failing tests show
it too, or, for a test that returned an error, the error return trace. They come
from a per-thread shadow stack of call sites. Each call costs three stores and a
load; `--stack-traces=off` removes them. A call to an `inline fn` (always inlined, LLVM's
`alwaysinline`) passes its call-site record as an extra argument instead, which the
inlined body pushes only before a trap or a call that is not inline (`INL_SITES` in
`codegen.c`). On x86-64 Linux, except in profile and `--sanitize=address` builds,
calls cost nothing instead (issue 0037, `frame_tables_emit` in `codegen.c`): functions
keep frame pointers, each call gets a lexical-block `DILocation` of its own and
`nomerge`, and after optimization the inlinedAt chain of each remaining call's
location gives its records, which an inline-assembly label after the call puts in
the section `axiom_rets`. A trap or crash walks the frame pointers and fills the
shadow stack's ring from the table (`ax_stack_walk` in `std/internal/runtime/native_host.c`), so
everything that reads the shadow stack is unchanged. The debug information that
this needs is stripped after optimization without `-g`.

**Line tables** (spec §16.1): `-g` / `--debug-info=lines` attaches a `DISubprogram` to
each function and a `DILocation` (from the same spans as the trap sites) to each
instruction, through LLVM's DIBuilder; inlined functions appear as
`DW_TAG_inlined_subroutine`.

**Profiling** (spec §16.6): `axiom profile` samples the shadow stack, using a
CPU-clock `SIGPROF` natively (`std/internal/runtime/profile.c`) or a worker reading shared memory
on wasm (`std/internal/runtime/profile_worker.mjs`). It reports self and total time per function,
inclusive time per call site, and folded stacks for flame graphs.

**Packages** (spec §4.6, `src/package.c`): a `axiom.toml` manifest (a TOML subset)
with path dependencies and git dependencies pinned to a commit, fetched once into
`$AXIOM_CACHE` (default `~/.cache/axiom`). Modules import them as `"pkg:name"` or
`"pkg:name/sub/mod"`; only declared dependencies are visible, and a build uses one
source per package name.

**C interop** (spec §14.5):
- `extern "C" { fn ...; }` calls C. On wasm, the C sources passed with `--link` are
  compiled to wasm and share the module's memory.
- `export fn` plus `--emit-header` makes Axiom callable from C. See
  `examples/c_interop/`.

Programs that use `std/thread` are linked with a shared, imported memory, and export
`axiom_thread_start`. `std/internal/runtime/host.mjs` creates the memory and runs each thread as
a node worker. Native builds use pthreads. Atomics are always available.

On wasm, `export fn` functions become WebAssembly exports, `import host "mod" { ... }`
functions become imports from `mod`, and the linear memory is exported as `memory`.
The shadow stack is placed first in memory (`--stack-first`), so a stack overflow
becomes an out-of-bounds trap instead of corrupting data.

## Tests

```
$ node cmd/axiom/internal/tests/run.mjs              # wasm32, -O0
$ node cmd/axiom/internal/tests/run.mjs -O2
$ node cmd/axiom/internal/tests/run.mjs --native     # native target (links cmd/axiom/internal/tests/native_rt.c)
$ make -C cmd/axiom/bootstrap asan && AXIOM=cmd/axiom/bootstrap/build-asan/axiom node cmd/axiom/internal/tests/run.mjs
$ node cmd/axiom/internal/tests/std_opt.mjs          # std's tests at -O2, wasm32 and native, in parallel
```

`cmd/axiom/internal/tests/run/*.ax` are run-and-compare tests. `// expect:` lines give the expected output,
`// expect-trap` means `run` must trap, and `// flags:` adds compiler options.
`cmd/axiom/internal/tests/errors/*.ax` must fail to compile with the `// expect-error:` codes.
`cmd/axiom/internal/tests/lowering/*.ax` check instruction selection: each `// lower: <flags> | <regex> | <count>`
line builds the file with `--emit=asm` and counts the regex's matches (for example, one
`vpmaddwd` per `@dot` of `i16x32` with AVX-512, issue 0036). The suite
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
- **Friendly to LLVM's -O0 fast path.** Overflow checks (`--overflow=trap`) use plain instructions instead
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
- implicit and explicit conversions, the three overflow modes (wrap, the default; trap;
  saturate), mathematical shifts
- `defer`, `match` exhaustiveness, `!` / `catch`
- generics with constraints, deduction and `const` parameters
- SIMD types and builtins
- host imports and exports
- the builtins in Appendix A
- trapping for the conditions in Appendix B
- threads and atomics (§12.8)

Not yet implemented (reported with a diagnostic where relevant):

- `axiom fmt` (only the self-hosted compiler has it: [`cmd/axiom/`](../README.md)),
  `axiom run` and `axiom explain` (spec §16.1).
- The debug trap map (`axiom.traps` custom section) and error-return traces
  (spec §16.4). Traps are plain `unreachable`, so the trap code isn't reported yet.
- Global initializers containing union values with a payload (`E0353`).
- **Native targets use real addresses for raw pointers.** Slices, arrays and all
  checked operations behave as on wasm, but a raw pointer that points outside mapped
  memory crashes with a signal instead of trapping deterministically. The
  bounds-checked linear-memory region of spec §12.1 is future work.
  On native targets linear memory is a region of address space reserved at the first
  `@memory_grow` (up to 1 TiB) and committed 64 KiB page at a time (std/internal/runtime/native_host.c).
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
