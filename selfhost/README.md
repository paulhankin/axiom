# The self-hosted Phalange compiler

A port of the C compiler in [`compiler/`](../compiler/README.md) to Phalange. It has the
same command line, the same diagnostics and the same output: for every program in the
repository it prints byte-identical diagnostics and emits byte-identical LLVM IR. It calls
LLVM through its C API and the C library through `extern "C"`, and it targets native code
(Linux on x86-64 or AArch64).

## Bootstrapping

```
$ selfhost/bootstrap.sh
```

| Stage | Compiler | Built by |
|---|---|---|
| 0 | `compiler/build/phalange` (C) | `make -C compiler` |
| 1 | `selfhost/build/phalange1` | stage 0 |
| 2 | `selfhost/build/phalange2`, copied to `selfhost/build/phalange` | stage 1 |
| 3 | `selfhost/build/phalange3` | stage 2 |

The script fails unless stages 2 and 3 are bit-identical (the fixed point). Stage 1 also
comes out bit-identical to stage 2, because the C and Phalange compilers generate the same
code. `OPT=-O0` builds the stages without optimization, which is faster.

To build the compiler by hand with any existing Phalange compiler:

```
$ phalange build selfhost/src/main.ph --target=native -O2 -o phalange \
      -L$(llvm-config --libdir) $(llvm-config --libs)
```

`selfhost/build/phalange` finds the standard library and the runtime the same way the C
compiler does (`../../std` from the executable, or `--std`, or `$PHALANGE_STD`).

## Testing

```
$ PHALANGE=selfhost/build/phalange node tests/run.mjs     # the whole suite, any options
$ selfhost/difftest.sh                                    # compare with the C compiler
$ selfhost/build/phalange test selfhost/src/util.ph --target=native
```

`difftest.sh` checks every `.ph` file under `tests/`, `std/`, `examples/` and
`selfhost/src/`: the diagnostics (`check --format=agent-json`) and exit status, and the
unoptimized LLVM IR for wasm32, wasm64 and native, at `-O0`, `-O1` and `-O2`, in both
overflow modes and without stack traces.

## Layout

The modules follow the C sources. Phalange forbids import cycles, so the mutually
recursive parts of the C semantic analysis are one module, and the AST and the semantic
types share a module.

| Module | C source | What |
|---|---|---|
| `util.ph` | `util.c` | libc bindings, arena, `List[T]`, interned strings, `SymMap[V]`, `I128`, formatting, merge sort, files, spans |
| `diag.ph` | `diag.c` | diagnostics, human and agent-json output |
| `lexer.ph` | `lexer.c` | tokens |
| `ast.ph` | `ast.h`, `types.h`, `builtins.h` | syntax tree, semantic types, builtin table |
| `parser.ph` | `parser.c` | LL(1) parser, operator partial order |
| `types.ph` | `types.c` | hash-consed types, substitution, printing, layout, constraints |
| `package.ph` | `package.c` | `phalange.toml`, `pkg:` imports, git dependencies |
| `wasmops.ph` | `wasmops.c` | inline-wasm instruction table |
| `sema.ph` | `sema_decl.c`, `sema_expr.c`, `sema_builtins.c`, `sema_stmt.c` | module loading, name resolution, type checking |
| `llvm.ph` | LLVM C headers | the LLVM 18 C API functions and enum values used |
| `codegen.ph` | `codegen.c` | LLVM IR generation, monomorphization, traps, shadow stack |
| `header.ph` | `header.c` | C headers for `export fn`s |
| `units.ph` | `units.c` | separate compilation and the object cache |
| `main.ph` | `main.c` | the driver: `build`, `check`, `test`, `profile`, `new`, `cache` |

## How C idioms map to Phalange

- **NULL.** Pointers are non-nullable in the type system, so a C `NULL` is a pointer
  with address 0: `u.nil.[T]()` makes one and `u.isnil(p)` / `u.notnil(p)` test for it.
  An absent string is a slice with a 0 pointer (`u.nosym()`, `u.isnos(s)`).
- **Strings** are `[]u8`. Every string the compiler creates is followed by a NUL in
  memory, so `.ptr` can go to C. Interned strings (`u.intern`) compare by pointer
  (`u.same`).
- **Enums** are `i32` constants, and `switch` statements are `if`/`else if` chains,
  because `match` patterns must be literals.
- **`printf`**: messages are built with `u.f1` ... `u.f6`, which substitute `%s` only.
  Numbers are converted first (`u.idec`, `u.dec`, `u.udec`, `u.zdec`); `u.ffmt` formats
  one float with `snprintf`.
- **`__int128`** constant folding uses `u.I128`, a two's complement 128-bit integer with
  C's truncating division and wrapping multiplication.
- **`qsort`** is `u.sort`, a stable merge sort, like glibc's `qsort` for these arrays.
- **Processes** use `fork` / `execvp` / `waitpid` instead of `posix_spawn`, whose
  argument structures are platform-specific.
- **Evaluation order.** C leaves the order of function arguments unspecified, and GCC
  evaluates nested LLVM builder calls right to left, which decides the order of the
  emitted instructions. Phalange evaluates left to right, so where the C code nests two
  instruction-building calls in one argument list, the port builds them in the order GCC
  does. Those places are marked with a comment.

## Platform assumptions

- Linux: `/proc/self/exe`, and `readdir`'s `d_name` at offset 19 (glibc, 64-bit).
- The host target is initialized with `LLVMInitializeX86*` or `LLVMInitializeAArch64*`
  (the C API's `LLVMInitializeNativeTarget` is a header-only inline function), so the LLVM
  library must include the host's target, as distribution builds do.
