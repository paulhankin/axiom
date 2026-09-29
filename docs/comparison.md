# Phalange compared with Zig, Rust, C++ and C

Phalange is a systems language meant to be written, read and repaired by machine agents,
and its main target is WebAssembly. This document puts its design next to the four
languages it is most often weighed against. It compares **languages**, not ecosystems. The
other four are described as they are in their current standards and releases (C23, C++23
with notes on C++26, Rust 2024 edition, Zig 0.15). Phalange is described by its
[specification](../spec/phalange.md) (v0.1 draft), with notes where the implementation
does not match it yet.

Every language here is a sensible choice for some job. The point is to show where
Phalange's choices differ and what each choice costs.

## At a glance

| | Phalange | Zig | Rust | C++ | C |
|---|---|---|---|---|---|
| **Primary aim** | Code that agents can write and check locally; WebAssembly first | A better C: explicit, simple, fast to compile | Memory and thread safety without a garbage collector | Zero-cost abstraction on top of C | A portable assembler |
| **Undefined behavior** | None: every operation has a defined result or traps | "Illegal behavior": checked in Debug/ReleaseSafe, undefined in ReleaseFast/ReleaseSmall | None in safe code; possible in `unsafe` | Pervasive | Pervasive |
| **Memory safety** | Bounds-checked slices and linear memory; a dangling pointer reads defined but stale bytes | Checked slices in safe modes; lifetimes are up to you | Ownership and borrow checking | Up to you (RAII, smart pointers, optional hardening) | Up to you |
| **Allocation** | One global allocator (`std/mem`, written in Phalange), always called explicitly | Allocators passed explicitly; no global one | Global allocator, implicit in `Box`/`Vec`/`String` | Global `new`, implicit in containers | `malloc` |
| **Cleanup** | `defer` | `defer`, `errdefer` | Destructors (`Drop`) | Destructors (RAII) | By hand (or compiler extensions) |
| **Errors** | `!T` with one program-wide error set; postfix `!` propagates; `catch` | Error unions with per-function error sets; `try`; `catch` | `Result<T, E>` and `?`; panics | Exceptions; `std::expected` (C++23) | Return codes, `errno` |
| **Null** | No null; pointers are never optional | Non-null pointers; `?T` optionals | No null; `Option<T>` | `nullptr` | `NULL` (`nullptr` in C23) |
| **Generics** | Monomorphized; a closed set of builtin constraints; bodies checked at the definition | `comptime` parameters; checked at each instantiation | Traits; checked at the definition | Templates, concepts; checked at each instantiation | Macros, `_Generic`, `void *` |
| **Metaprogramming** | None: no macros, no compile-time execution | `comptime` code | Declarative and procedural macros, `const fn` | Templates, `constexpr`/`consteval`, preprocessor | Preprocessor |
| **Hidden control flow** | None: no exceptions, destructors, overloading or implicit calls | None (by design) | Destructors, `Deref`, overloading, panics | Destructors, overloading, conversions, exceptions | Almost none (`setjmp`/`longjmp` aside) |
| **Integer overflow** | Traps (default) or saturates (`--release`); `+%` wraps | Checked in safe modes; `+%` wraps, `+\|` saturates | Panics in debug, wraps in release | Signed: undefined; unsigned: wraps | Signed: undefined; unsigned: wraps |
| **Data races** | Defined (each byte reads some written value); sequentially consistent atomics only | Undefined; atomics with orderings | Prevented by `Send`/`Sync` in safe code | Undefined; atomics with orderings | Undefined; atomics with orderings |
| **SIMD** | 128-bit vector primitives (`f32x4`, `i8x16`, …) with WebAssembly semantics everywhere | `@Vector(N, T)` of any width | Intrinsics (`core::arch`); portable `std::simd` is unstable | Intrinsics, vector extensions; `std::simd` in C++26 | Intrinsics, vector extensions |
| **Grammar** | LL(1), context-free, no macros; operator precedence is a partial order | Simple, no macros | Large; macros | Very large, context-sensitive | Small, but the preprocessor sits on top |
| **Standard library** | Small: containers, strings and Unicode (normalization, segmentation, case), files and paths, structured logging, `{}` formatting, time, threads, TCP, an HTTP/1.1 server and client, TLS 1.3 and the cryptography under it, big integers, gzip/zlib compression, colour models, image codecs (PNG, JPEG, BMP, QOI, PNM, Radiance HDR, OpenEXR), anti-aliased 2D drawing, TrueType/OpenType fonts and text rendering, sound files (WAV, AIFF and FLAC read and written; Ogg Vorbis and MP3 read) and resampling, JSON, linear-time regular expressions | Broad (containers, formatting, JSON, an HTTP client with TLS and a server, crypto); no regular expressions | Deliberately small (no JSON, HTTP or regex: crates) | Large (containers, algorithms, `std::format`, `<regex>`, `<chrono>`); no JSON or HTTP | Minimal (`stdio`, `string.h`, `time.h`) |
| **Formatter** | `phalange fmt` (one canonical layout, imports tidied) | `zig fmt` | `rustfmt` | `clang-format` (configurable) | `clang-format` (configurable) |
| **Toolchain** | One binary: build, check, test, profile, fmt, packages, C headers | One binary: build system, tests, fmt, C compiler | `cargo` and friends | Many tools, no standard build or package system | Many tools, no standard build or package system |
| **Diagnostics** | Stable codes; `--format=agent-json` with machine-applicable fixes | Human-readable | Human-readable; JSON with suggestions | Human-readable; SARIF in some compilers | Human-readable; SARIF in some compilers |

## The same function in each language

Sum the digits of a string, failing on anything that is not a digit.

**Phalange**

```phalange
error { InvalidInput }

fn parse_digit(c: u8) !u32 {
    if (c < '0') or (c > '9') {
        return error.InvalidInput;
    }
    return @as(u32, c - '0');
}

fn sum_digits(s: []u8) !u32 {
    let mut total: u32 = 0;
    for c in s {
        total += parse_digit(c)!;
    }
    return total;
}
```

**Zig**

```zig
const ParseError = error{InvalidInput};

fn parseDigit(c: u8) ParseError!u32 {
    if (c < '0' or c > '9') return error.InvalidInput;
    return c - '0';
}

fn sumDigits(s: []const u8) ParseError!u32 {
    var total: u32 = 0;
    for (s) |c| total += try parseDigit(c);
    return total;
}
```

**Rust**

```rust
#[derive(Debug)]
enum ParseError { InvalidInput }

fn parse_digit(c: u8) -> Result<u32, ParseError> {
    if !c.is_ascii_digit() { return Err(ParseError::InvalidInput); }
    Ok(u32::from(c - b'0'))
}

fn sum_digits(s: &[u8]) -> Result<u32, ParseError> {
    let mut total = 0u32;
    for &c in s { total += parse_digit(c)?; }
    Ok(total)
}
```

**C++23**

```cpp
#include <cstdint>
#include <expected>
#include <span>

enum class ParseError { InvalidInput };

std::expected<std::uint32_t, ParseError> parse_digit(unsigned char c) {
    if (c < '0' || c > '9') return std::unexpected(ParseError::InvalidInput);
    return c - '0';
}

std::expected<std::uint32_t, ParseError> sum_digits(std::span<const unsigned char> s) {
    std::uint32_t total = 0;
    for (unsigned char c : s) {
        auto d = parse_digit(c);
        if (!d) return std::unexpected(d.error());
        total += *d;
    }
    return total;
}
```

**C**

```c
#include <stddef.h>
#include <stdint.h>

enum { OK = 0, ERR_INVALID_INPUT = 1 };

static int parse_digit(unsigned char c, uint32_t *out) {
    if (c < '0' || c > '9') return ERR_INVALID_INPUT;
    *out = (uint32_t)(c - '0');
    return OK;
}

static int sum_digits(const unsigned char *s, size_t n, uint32_t *out) {
    uint32_t total = 0;
    for (size_t i = 0; i < n; i++) {
        uint32_t d;
        int err = parse_digit(s[i], &d);
        if (err) return err;
        total += d;
    }
    *out = total;
    return OK;
}
```

The Phalange, Zig and Rust versions look alike: an error type in the signature and a
one-token way to pass an error up. The differences show up at the edges:

- **A very long input** makes `total` overflow. Phalange traps with code `T0002` and a
  stack trace (or saturates at `u32` max in a `--release` build, which the build states).
  Zig panics in Debug and ReleaseSafe builds, and has undefined behavior in ReleaseFast.
  Rust panics in debug builds and silently wraps in release builds. C++ and C wrap
  silently: `uint32_t` arithmetic is modular.
- **The error set.** Phalange has one program-wide set of error names, so `!u32` needs no
  error type. Zig can infer the set per function (`!u32`), or you name it, as here. Rust
  and C++ name a type, and a caller that combines several error types must convert them.
- **Precedence.** The parentheses around the comparisons are optional (comparison binds
  tighter than `or` in all five languages), but Phalange would reject `a and b or c`,
  which the other four accept: it defines only the precedences everyone agrees on (§7.4),
  so a reader never needs a precedence table.

## What happens when…

| | Phalange | Zig (ReleaseSafe / ReleaseFast) | Rust (safe code) | C++ | C |
|---|---|---|---|---|---|
| **An index is out of bounds** | Trap `T0001` | Panic / undefined | Panic | Undefined (`at()` throws; hardened libraries check) | Undefined |
| **Signed overflow** | Trap `T0002` (or saturate with `--release`) | Panic / undefined | Panic in debug, wrap in release | Undefined | Undefined |
| **Division by zero** | Trap `T0003` | Panic / undefined | Panic | Undefined | Undefined |
| **Shift by ≥ the width** | Defined: the mathematical result (`0`, or `-1` for a negative `>>`) | Rejected by the shift-amount type unless cast | Panic in debug, count masked in release | Undefined | Undefined |
| **A read of uninitialized memory** | Impossible for variables (every `let` has an initializer); allocator memory holds unspecified bytes | Allowed through `undefined`; reading it is illegal behavior, mostly unchecked | Rejected at compile time | Undefined (erroneous behavior in C++26) | Undefined |
| **A null pointer is used** | There is no null. Address 0 is ordinary memory | Only `?*T` (or `allowzero`) can be null; making a null `*T` is illegal behavior | No null in safe code | Undefined | Undefined |
| **Use after free** | Reads whatever bytes are there now: a logic bug with defined behavior (`--sanitize=address` traps on it) | Undefined | Rejected at compile time | Undefined | Undefined |
| **A data race** | Each byte reads a value some store wrote (tearing allowed) | Undefined | Rejected at compile time | Undefined | Undefined |
| **An invalid union tag** | Trap `T0007` at `match` | Panic / undefined | Cannot happen in safe code | Undefined (`std::variant` is safe) | Undefined |
| **A floating-point expression** | Evaluated as written: no FMA contraction, no fast-math | As written unless `@setFloatMode(.optimized)` | As written | Contraction is allowed and some compilers do it by default | The same as C++ (`FP_CONTRACT`) |

Rust and Phalange both remove undefined behavior, but in opposite ways. Rust **rejects**
programs whose memory use it cannot prove sound, and gives an escape hatch (`unsafe`) in
which UB is back. Phalange **defines** what every operation does, WebAssembly-style: memory
is one bounds-checked byte array, so a dangling pointer, a race or a type pun gives a wrong
answer, never a license for the optimizer. The compiler may not optimize on the assumption
that UB is absent, because there is none. Phalange catches fewer bugs at compile time than
Rust. In exchange its programs are simpler (no lifetimes, no borrow checker), and a bug
has one reproducible outcome instead of whatever the optimizer made of it.

Zig sits between them. It has UB, but its safe build modes check most of it at run time.
C and C++ leave most mistakes undefined. Sanitizers and hardened library modes help, but
they are not part of the language's meaning.

## Memory and resources

- **Phalange** has one global allocator, `std/mem`, like Rust and C++, but nothing in the
  language calls it for you: memory comes from `mem.alloc` and goes back through
  `mem.free` (usually in a `defer`), and a type that owns memory has a `deinit` you call.
  There are no destructors and no RAII: releasing memory or closing a file is a statement
  you can see. The allocator itself is ordinary Phalange code (size-class spans for small
  objects, boundary-tagged blocks for large ones, no locking until a second thread
  starts), and `free` takes the slice, so objects carry no size header.
- **Zig** is also explicit, but threads allocator values through every API instead of
  having a global one: flexible (arenas, fixed buffers, testing allocators), at the cost
  of an extra parameter on everything that allocates. Zig has `errdefer`; Phalange does
  not yet (it is an open question in Appendix D).
- **Rust** ties resources to ownership. `Drop` runs automatically at the end of a scope,
  which removes leaks and double frees but makes some calls invisible. Most standard
  containers use the global allocator (allocator parameters are still unstable).
- **C++** has the same RAII model as Rust without the borrow checker. Standard containers
  take allocator type parameters, and `std::pmr` gives runtime-polymorphic ones.
- **C** has `malloc`/`free`, and nothing else in the language.

Phalange's rule that nothing is hidden goes further than Zig's in one place: there is no
operator overloading, no method that runs without a call, and there are no closures. A
line of Phalange does what its tokens say.

## Errors

| | Phalange | Zig | Rust | C++ | C |
|---|---|---|---|---|---|
| Type | `!T`: a 16-bit code next to the value | `E!T`: an error set per function | `Result<T, E>`: any `E` | Exceptions, or `std::expected<T, E>` | Return codes, `errno` |
| Propagate | `f()!` | `try f()` | `f()?` | Automatic (exceptions) or by hand | By hand |
| Handle | `catch`, `catch \|e\|` | `catch`, `catch \|e\|` | `match`, combinators | `try`/`catch`, or by hand | By hand |
| Payload | None: the error is its name | None | Any value | Any value | None |
| Traces | Error return traces in debug builds (§16.4) | Error return traces in safe builds | Backtraces on panic; errors carry none | Stack traces through `std::stacktrace` (C++23), by hand | None |
| Unrecoverable | Trap: the program stops, `defer` does not run | Panic | Panic (unwinds or aborts) | `std::terminate` | `abort` |

Phalange's error names form **one set for the whole program**. Any function can return any
error without declaring which ones, and `@error_name(e)` turns a code back into its name.
Zig and Rust track more in the type. Phalange's choice keeps signatures short and makes
errors from different libraries compose without conversion code, but a caller cannot see
from the type which errors are possible. Errors carry no data: a function that needs to
report context returns a struct or writes to a caller's buffer.

## Generics and metaprogramming

- **Phalange** generics are monomorphized. A type parameter has at most one constraint
  from a closed, builtin set (`Eq`, `Ord`, `Num`, `Int`, `SignedInt`, `UnsignedInt`,
  `Float`), and a generic body is **type-checked once, at its definition**, against that
  constraint. An error therefore never appears inside a library generic because of a type
  argument a caller chose: it is reported at the line the caller wrote. There is no
  specialization, overloading, variadic generics, compile-time execution or dynamic
  dispatch. Behavior varies through function pointers or tagged unions.
- **Zig** uses `comptime`: types are values, and generic code is ordinary code run at
  compile time. It is very powerful and uses one language for both, but it is checked per
  instantiation. An error can surface deep inside a generic, far from the call that
  caused it.
- **Rust** traits are checked at the definition as Phalange constraints are. They are
  open (anyone can define and implement one), come with associated types and
  `dyn Trait` dispatch, and are much more expressive. Macros cover the rest.
- **C++** templates are checked at instantiation. Concepts (C++20) constrain what may
  instantiate a template, but the body is not checked against the concept.
- **C** has macros and `_Generic`.

Phalange has no macros and no compile-time code at all. Repetition that a Zig or Rust
program would generate must be written out or generated by another program. That is the
price of a grammar that an agent can parse without running anything.

## Syntax, readability and tooling

Phalange's syntax is designed so that a reader (human or agent) with one file and the
declarations it names can tell exactly what each line does:

- **LL(1) and context-free.** No symbol table is needed to parse, and there are no macros
  or preprocessor. Struct literals are `T.{ … }` and generic arguments are `T.[…]`, so
  `if x == Point { … }` and `a < b > c` are never ambiguous.
- **Precedence is a partial order.** Only relationships everyone agrees on are defined
  (`*` above `+`, arithmetic above comparison, comparison above `and`/`or`). Mixing
  anything else (`a + b & c`, `a and b or c`, `a < b < c`) is an error with a
  machine-applicable fix that adds the parentheses.
- **No shadowing** (§4.4): a local may not reuse the name of another local, a parameter,
  a top-level item or an import.
- **No discarded results**: an unused value must be assigned to `_`.
- **One canonical layout**, produced by `phalange fmt`, which also sorts imports and adds
  or removes them.
- **Diagnostics for machines.** Every diagnostic has a stable code, and
  `--format=agent-json` gives spans, expected and found types, and fixes marked as
  machine-applicable where they are.

Zig is closest here: no macros, no hidden control flow, `zig fmt`, and one binary that
builds, tests and cross-compiles. Rust's tooling is the most complete (`cargo`, `clippy`,
`rustfmt`, rust-analyzer), but the language is large and its macros make source hard to
read without expanding them. C++ has the hardest grammar to parse of the five (a
declaration and an expression can look the same) and no standard build system. C is
small, but real code relies on the preprocessor.

## Numbers and SIMD

- **Integers.** Phalange has one overflow mode per build: `trap` (the default) or
  `saturate` (`--release`). It is the only build setting that can change what a program
  means; optimization levels must not. Wrapping operators (`+%`, `-%`, `*%`) are explicit.
  Shifts are defined for every count, and division by zero always traps. Zig and Rust
  offer similar operators (`+%`, `+|`; `wrapping_add`, `saturating_add`, `checked_add`),
  but what the plain `+` does depends on the build mode. C23 adds checked arithmetic
  (`ckd_add`), but plain signed overflow stays undefined.
- **Floating point.** IEEE 754 with no contraction, no reassociation and no fast-math
  flag in Phalange. Rust behaves the same by default. C and C++ allow contraction.
- **SIMD.** Phalange's vector types are primitives of exactly 128 bits (`i8x16` …
  `f64x2`, `v128`). Their semantics are WebAssembly's on every target, so a vector program
  gives the same bits on wasm, x86-64 and AArch64. Zig's `@Vector` has any width and
  lowers to the target's best instructions. Rust has per-architecture intrinsics (stable)
  and portable `std::simd` (unstable). C and C++ use intrinsics or compiler extensions
  (C++26 adds `std::simd`). Phalange trades 256- and 512-bit AVX width for portability.

## Concurrency

Phalange treats threads as a library (`std/thread`) and adds only atomic builtins to the
language. All atomics are **sequentially consistent**: there are no memory-order
parameters. A data race is not undefined behavior: a racing read returns, for each byte,
some value that was written there. This is WebAssembly's thread model, applied everywhere.
There is no `async`/`await`, since a suspension point that looks like an ordinary
expression would be hidden control flow.

Rust prevents data races at compile time and offers async. C, C++ and Zig give
fine-grained memory orderings and treat races as undefined. Phalange's model is the
easiest to reason about, but it gives up weaker orderings, which cost performance on
weakly ordered CPUs such as ARM.

## Interoperability and targets

- **Phalange** targets WebAssembly first (`wasm32`, `wasm64`, freestanding or WASI), and
  x86-64 and AArch64 Linux natively. `extern "C"` blocks call C, `export fn` exposes
  functions, and the compiler generates a C header for them. The self-hosted compiler has
  its own backends and linkers. Programs it builds make Linux system calls directly and
  are static executables with no C library, unless they link C code (as Zig's do).
- **Zig** cross-compiles to almost anything, imports C headers directly (`@cImport`) and
  is also a C compiler (`zig cc`).
- **Rust** targets everything LLVM does (and wasm well), with C interop through `extern`
  blocks and `bindgen`/`cbindgen`.
- **C++** and **C** run everywhere and are what other languages interoperate with.

## What Phalange costs you

- **No compile-time memory safety.** Use-after-free and races are defined but not
  prevented. Rust prevents them; Phalange only makes their effects reproducible, and
  finds lifetime errors at run time with `--sanitize=address` (use after free, double
  free, leaks) when you ask. There is no race detector.
- **Bounds checks and SC atomics cost speed.** The optimizer removes many checks, but not
  all.
- **Less abstraction.** No traits or interfaces, no closures, no macros, no compile-time
  code, no operator overloading, no destructors. Code is longer and more repetitive than
  in Rust or C++, which is deliberate: it is easier to read one line at a time.
- **One error set and no error payloads.** Less type information about failures than
  Zig or Rust.
- **Only 128-bit SIMD.**
- **Young.** Phalange is a v0.1 draft with a small standard library (no time
  zones, no Unicode case folding) and no third-party ecosystem. On native targets, the linear-memory bounds model of §12.1 is not yet
  implemented: a raw pointer outside mapped memory crashes with a signal instead of a
  deterministic trap (slices, arrays and the other checks behave as on wasm). On this
  repository's benchmarks, the self-hosted compiler's own backend produces code that takes
  between 0.8 and 1.6 times as long as the LLVM-built code from the C compiler
  (`selfhost/bench.sh`).

## Choosing

- **Phalange**: code that agents generate and maintain, WebAssembly modules, programs
  where "the same answer every time, or a precise trap" matters more than peak speed or
  abstraction.
- **Zig**: C's domain with better defaults, cross-compilation and C interop, and
  `comptime` when you need generation.
- **Rust**: large programs where memory and thread safety must be proven, and where a
  rich type system pays for itself.
- **C++**: existing C++ code bases, and libraries and domains that are C++-first
  (games, HPC, finance).
- **C**: the lowest common denominator: ABIs, kernels, embedded targets, and code other
  languages call.
