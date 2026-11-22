# Axiom Language Improvements Plan

This document lays out a phased plan to address eight structural weaknesses in the Axiom
language specification (`docs/spec.md`), compiler (`cmd/axiom/`), and standard library
(`std/`). Every proposal is evaluated from first principles for Axiom's design mandate:
an LL(1) context-free grammar, zero hidden control flow, locality of reasoning for machine
agents, definition-checked generics, and deterministic builds.

Several of the performance-critical IR and backend mechanisms needed here already exist in
the self-hosted compiler (built for `axiom cc` and `divconst.ax`): `ir.IN_UMULH`,
`ir.IN_SMULH`, `ir.IN_UDIV128`, `ir.F_VOLATILE`, `ir.ORD_*` atomic memory orderings, and
`ir.restrict_apart` / `ir.restrict_tagged`.

---

## Summary of Weaknesses Addressed

1. **No compile-time behavioral parameterization:** Generic containers (`Map`, `Set`,
   `TreeMap`, `Heap`, `Lru`) and algorithms (`vec.sort`) store runtime function pointers
   and make indirect calls in their hottest inner loops; `io.Writer`/`Reader` rely on
   `*mut void` type erasure; `FloatVector` exposes neither lane type nor lane count.
2. **No wide-integer arithmetic:** Stopping at 64 bits with no widening multiply or
   add-with-carry forces multi-precision math and cryptography (`bigint`, `curve25519`,
   `chacha20poly1305`, `ec`, `wideint`) into 25.5-, 26-, or 32-bit limbs (a 2x-4x
   algorithmic penalty on 64-bit targets).
3. **Aliasing blindness, §12.8.2 spec/optimizer conflict, and no `volatile`/barrier:**
   Axiom code cannot express non-aliasing even though its IR supports `restrict` for C;
   spec §12.8.2 literally forbids GVN load elimination and loop register promotion that
   `gvn.ax` and `loopopt.ax` perform; `std/crypto/subtle.zero` is plain `@mem_set` and
   can be eliminated by dead-store elimination; SC-only atomics impose a permanent barrier
   cost on AArch64.
4. **No optional type (`?T`) or empty slice literal (`[]`):** Removing `null` without `?T`
   pushed optionality into forged zero pointers (`@ptr_from_int(..., 0)` + `isnil`),
   wrapping integer sentinels (`0xffffffff`), error-channel abuse (`NotFound`/`Empty`/`Done`),
   and 190+ `@slice_from_ptr(@ptr_from_int(..., @align_of(T)), 0)` empty-slice sites.
5. **Value-only `match` on tagged unions:** Because `match` always copies payloads (§8.6),
   tagged unions cannot be mutated or inspected in place; across ~630k LOC there are 1,694
   structs vs 19 unions, and `ast.ax` resorted to raw `usize` pointer punning.
6. **Native UB in raw-pointer-free code:** Silently bitwise-copyable owning/pinned structs
   (`Vec`, `Map`, `BigInt`, `Mutex`), value-copying `for x in s`, shallow mutability
   (`*Self` returning `[]mut T`), missing field privacy (§4.2), and warning-only local
   escape checks (`W0710`, not covered by `--sanitize=address`) allow heap UAF, double-free,
   and stack use-after-return using only slices and structs.
7. **Error-set collapse:** Module-wide error sets grow to 30-80 flat names (`HttpError`
   has 80, including `DivisionByZero` and `NotADirectory`) via transitive leaf pollution
   and absence errors; generic wrappers cannot preserve error sets; errors carry no
   diagnostic context.
8. **File-level acyclic module rule (`E0210`):** Forbidding mutual references across files
   forces mutually recursive subsystems into 4,500-7,000-line monoliths (`html.ax`,
   `ir.ax`, `a64.ax`, `http.ax`, `wasm.ax`, `sema.ax`) or mutable `threadlocal` callback
   hook tables (`semabuiltin.ax`, `cgbuiltin.ax`), hurting agent context compactness and
   incremental build caching.

---

## Breaking Changes Summary

By using contextual forms (`nocopy struct`, `noalias` param, `import part`) and
dot-prefixed `.none` / `.some(x)`, **no new reserved keywords are introduced** (`none`
currently appears ~317 times as an identifier in the repo and `part` ~188 times; `.none`
and `import part` have zero collisions).

| Phase | Change | Breaking? | Scope & Migration |
|---|---|---|---|
| **0** | Spec §12.8.2 alignment, `volatile`/barrier builtins, wide-int builtins, opt-in atomic orders, `noalias` param | **No** | Purely additive |
| **1** | `?T`, `[]`, ref-binding `match`/`if let`, `for &x`/`for &mut x` | **No** *(optional follow-up: trapping `@ptr_from_int(*T, 0)` would be **Breaking**)* | Additive; if null `*T` is later disallowed, migrate `nil()`/`isnil()` to `?*T` |
| **2** | `FloatVector` replaced by `FloatVector[S]` (lane-typed) | **Yes (language)** | 0 uses in `std/` and `examples/`; updates 4 compiler tests |
| **2** | Stdlib containers (`Map`, `Set`, `TreeMap`, `Heap`, `Lru`, `vec.sort`) switched to `const` fn generic params | **Yes (stdlib API)** | Callers move `hash_fn`/`eq_fn`/`less` from `init(...)` to generic args (keep `*_dyn` variants where runtime dispatch is needed) |
| **3** | Transitive immutability on `*T` and `[]T` | **Yes (language)** | Methods like `Vec.items(*Self) []mut T` split into `items(*Self) []T` and `items_mut(*mut Self) []mut T`; machine-applicable fix via `agent-json` |
| **3** | Struct/union fields default module-private (`pub` opt-in) | **Yes (language)** | External field accesses require `pub` on data fields; mechanical migration tool can insert `pub` on plain structs |
| **3** | `nocopy` on owning/pinned stdlib structs (`Vec`, `Map`, `Mutex`, `BigInt`, `File`, ...) | **Yes (stdlib API)** | Rejects accidental bitwise copies and `for x in vecs`; callers use pointers, `.clone()`, or `for &x` / `for &mut x` |
| **3** | Promote `W0710` (dangling local ptr/slice escape) to error `E0710` + alias tracking | **Yes (language)** | Rejects programs that previously compiled with a warning or silent stack-UAR UB |
| **4** | Stdlib absence returns (`Map.get`, `Vec.pop`, `find_header`, `Iter.next`, ...) migrated from `!T` to `?T` | **Yes (stdlib API)** | Callers migrate `catch` / `!` to `??` / `if let .some(v)` |
| **4** | Tightened stdlib error sets (`VecError`, `HttpError`, `TlsError`, ...) + dead-error diagnostics | **Yes (stdlib API)** | Callers matching unreachable errors remove dead arms |
| **5** | Multi-file modules (`import part`) | **No** | Purely additive |

**Release strategy:** Land Phases 0 and 1 first (non-breaking), then ship Phases 2, 3, and
4 together in a single breaking release window (e.g., `0.3.0`) so downstream callers only
migrate once and every breaking rule in Phases 3-4 already has its idiomatic replacement
(`?T`, `[]`, ref `match`, `for &mut`) in place.

---

## Seed & Bootstrap Discipline

There is one compiler, `cmd/axiom/src/` (Axiom), built from the seed that
`cmd/axiom/seed.toml` pins (`docs/bootstrap-chain.md`). The phases landed before this
were made in a second compiler too, a bootstrap compiler in C that has since been
removed. Per `CLAUDE.md`, every phase must satisfy:
1. All front-end, type-system, and diagnostic changes land in `cmd/axiom/src/`, with
   their diagnostic codes, spans, and `agent-json` fixes in the spec and the tests.
2. **Bootstrap ordering:** Never use a new language feature inside `cmd/axiom/src/`, or
   in the standard library modules it imports, until the pinned seed accepts it: the
   feature lands first, then a new seed is made with `cmd/axiom/seed.sh` and pinned,
   then the compiler's sources may use it. A change of meaning needs sources that are
   correct under both the old and the new meaning until the new seed is pinned.
3. **Verification gate for every phase:**
   - `cmd/axiom/bootstrap.sh` (stage 2 and stage 3 identical)
   - `node cmd/axiom/internal/tests/run.mjs` (includes `axiom fmt --check`)
   - `cmd/axiom/difftest.sh`

---

## Phase 0: Memory Model Alignment & Performance Builtins (Non-Breaking)

Addresses weaknesses #2, #3a, #3b, #3c, and #3d.

### 0.1 Align Spec §12.8.2 with WebAssembly's Benign-Race Model
- **Spec (`docs/spec.md` §12.8.2):** Replace the literal ban on assuming two reads of a
  racing location return the same value with the actual WebAssembly/Java rule:
  - A racing byte read returns some non-happens-after written byte, so coalescing redundant
    loads and delaying non-atomic stores between synchronization operations is valid.
  - Rematerializing a single SSA load into multiple machine loads (causing one variable to
    observe two different values) and inventing speculative stores are forbidden.
- **Compiler audit:** Audit `cmd/axiom/src/gvn.ax`, `cmd/axiom/src/loopopt.ax`, and
  register-allocator rematerialization in `x64.ax` / `a64.ax` to verify compliance.

### 0.2 Volatile & Compiler-Barrier Builtins
- **New builtins (Appendix A):**
  - `@volatile_load(p: *T) T`
  - `@volatile_store(p: *mut T, v: T) void`
  - `@volatile_zero(s: []mut u8) void`
  - `@black_box(x: T) T` (opaque value barrier preventing constant folding / branch conversion)
- **Front end & IR:** Add to `ast.ax`, `semabuiltin.ax`, `cgbuiltin.ax`, lowering directly to
  `ir.F_VOLATILE` and an opaque barrier.
- **Optimizer:** Ensure `cmd/axiom/src/dse.ax` never deletes a volatile memset/store.
- **Stdlib:** Update `std/crypto/subtle.ax` (`subtle.zero` calls `@volatile_zero`;
  `subtle.equal` and `subtle.is_zero32` shield masks with `@black_box`).

### 0.3 Wide-Integer Arithmetic Builtins
- **New builtins (Appendix A):** For `u32`, `i32`, `u64`, `i64` (and generic `T: Int`):
  - `@mul_wide(a: T, b: T)` -> `struct { lo: T, hi: T }`
  - `@mul_high(a: T, b: T)` -> `T`
  - `@add_carry(a: T, b: T, cin: u8)` -> `struct { value: T, carry: u8 }`
  - `@sub_borrow(a: T, b: T, bin: u8)` -> `struct { value: T, borrow: u8 }`
  - `@div_wide(hi: T, lo: T, d: T)` -> `struct { quo: T, rem: T }` (traps `T0003` if
    `d == 0` or quotient overflows `T`)
- **Lowering:**
  - Native x86-64 / AArch64: wire to existing `ir.IN_UMULH`, `ir.IN_SMULH`, `ir.IN_UDIV128`
    and add `adc`/`sbb`/`adds`/`adcs`/`subs`/`sbcs` lowering in `x64isel.ax` and `a64.ax`.
  - WebAssembly (`wasm.ax`): lower to portable 32-bit split sequences.
- **Stdlib upgrades:**
  - `std/math/bigint.ax`: move from 32-bit limbs to 64-bit limbs.
  - `std/crypto/curve25519.ax`: move from 10 x 25.5-bit limbs to 5 x 51-bit limbs.
  - `std/crypto/chacha20poly1305.ax`: move Poly1305 from 5 x 26-bit limbs to 3 x 44-bit limbs.
  - `std/crypto/ec.ax`: move P-256 and P-384 Montgomery arithmetic to 64-bit limbs.
  - `std/c/rt/wideint.ax`: replace software 4-multiply `mul64` with `@mul_wide`.

### 0.4 Opt-In Atomic Memory Orders
- **Spec (§12.8.3):** Keep all existing `@atomic_*` calls sequentially consistent by default.
  Allow an optional trailing constant string order (`"relaxed"`, `"acquire"`, `"release"`,
  `"acq_rel"`, `"seq_cst"`), checked at compile time for validity per operation (new error
  code `E0830` for invalid order or invalid operation/order pair such as `"release"` on load).
- **Lowering:** Map directly to existing `ir.ORD_*` flags (`cmd/axiom/src/ir.ax`);
  WebAssembly keeps SC instructions (which validly
  implement weaker orders).
- **Stdlib:** Use `acquire`/`release`/`relaxed` on hot uncontended paths in `std/sync.ax`
  and `std/mem.ax`, verified with `fuzz_alloc.py` and AArch64 benchmarks.

### 0.5 `noalias` Parameter Qualifier
- **Grammar (§3.2):** `Param ::= "noalias"? IDENT ":" Type` (`noalias` contextual like
  `inline`/`threadlocal`; valid only on `*T`, `*mut T`, `[]T`, `[]mut T`, else `E0315`).
- **Lowering:** Set the existing `noalias` parameter attribute already consumed by
  `ir.restrict_apart` in `gvn.ax` and `vectorize.ax`.
- **Safety & Sanitizer:** Document in §12.1 that passing overlapping memory to `noalias`
  parameters violates the pointer-access rule. Under `--sanitize=address`, emit slice
  range-overlap checks at function entry that trap `T0012`.

---

## Phase 1: Optional Type (`?T`), Empty Slice Literal (`[]`), and Reference `match`/`for` (Non-Breaking)

Addresses weaknesses #4, #5, and #6b.

### 1.1 Optional Type `?T` with Niche Optimization
- **Grammar (LL(1), zero new keywords):**
  - `Type ::= "?" Type | ...`
  - `PrimaryExpr ::= ... | "." "none" | "." "some" "(" Expr ")"`
  - `Expr ::= BinExpr ( "catch" Handler | "??" OptHandler )?` where `OptHandler` allows
    `return Expr? | break IDENT? | continue IDENT? | Block | BinExpr` (mirrors `catch`
    without `|err|`).
  - `Pattern ::= ... | "." "none" | "." "some" "(" IDENT ")"` (usable in `match` and `if let`).
- **Typing & Conversions:**
  - Implicit conversion `T -> ?T` (§6.1), matching `T -> !T`.
  - `.none` takes its `?T` type from context (`E0502` without context).
  - `??` requires `?T` (`E0614` otherwise); mixing `??` unparenthesized in a `BinExpr` is
    `E0150`, matching `catch`.
- **Deterministic Layout (§5):**
  - `?*T`, `?*mut T`, `?fn(...)`: pointer-sized; bit pattern `0` is `.none`.
  - `?bool`: 1 byte (`0` = `false`, `1` = `true`, `2` = `.none`).
  - `?[]T`, `?[]mut T`: two words; `.ptr == 0` is `.none` (valid empty slices use non-zero
    aligned pointers).
  - All other `?T`: `struct { has: bool, value: T }`.
- **C FFI (§14.5):** Allow `?*T`, `?*mut T`, and `?fn(...)` at the C boundary, mapping
  directly to nullable C pointers (`const T *`, `T *`) without `std/c.is_null`.
- **Optional follow-up (BREAKING if enabled):** After migrating `cmd/axiom/src/` off
  `nil()`/`isnil()`, trap `T0005` on `@ptr_from_int(*T, 0)` so non-optional `*T` is
  strictly non-null at runtime as well as compile time.

### 1.2 Empty Slice Literal `[]`
- **Grammar (§3.5):** In `PrimaryExpr`, `[]` (empty brackets) parses as an empty slice
  literal (LL(1): `]` cannot begin an expression). Requires contextual type `[]T` or
  `[]mut T` (`E0502` otherwise).
- **Codegen:** Produces `{ ptr: @align_of(T), len: 0 }` (non-zero aligned address,
  preserving the `?[]T` niche).
- **Cleanup:** Replace the ~190 `@slice_from_ptr(@ptr_from_int(..., @align_of(T)), 0)`
  expressions in `std/` and fix the latent double-free in `std/math/matrix.ax` (`deinit`
  sets `self.data = [];`).

### 1.3 Reference-Binding `match` and `if let` on `*U` and `*mut U`
- **Semantics (§8.3, §8.6):** When the scrutinee of `match` or `if let` has type `*U` or
  `*mut U` (where `U` is a tagged union or `?T`), a payload binding `.Variant(p)` or
  `.some(p)` binds `p: *Payload` or `p: *mut Payload` pointing directly to the payload in
  place, with zero copying.
- **Implementation:** Update `check_match` / `gen_match` in `sema.ax` and `codegen.ax`.

### 1.4 By-Reference `for` Iteration
- **Grammar (§8.5):** `ForStmt ::= Label? "for" ( "&" "mut"? )? IDENT "in" Expr ( ".." Expr )? Block`.
- **Semantics:**
  - `for &x in s` binds `x: *T` (for `[]T`, `[]mut T`, or `[N]T` place).
  - `for &mut x in s` binds `x: *mut T` (requires `[]mut T` or mutable `[N]T` place, else `E0701`).
  - Range loops `for &i in lo..hi` are rejected (`E0706`).
- **Codegen:** Pointer-stepping loop with zero bounds checks and zero element copies.

---

## Phase 2: Compile-Time Function Parameters & Vector Generics (Breaking)

Addresses weakness #1.

### 2.1 `const` Function Generic Parameters (Non-Breaking)
- **Grammar (§10.2):** Allow `const IDENT ":" FnType` in `GenericParam`, where `FnType` may
  reference earlier generic type parameters in the same list:
  ```axiom
  pub struct Map[K, V, const HASH: fn(K) u64, const EQ: fn(K, K) bool] { ... }
  pub fn sort[T, const LESS: fn(T, T) bool](xs: []mut T) void { ... }
  ```
- **Arguments (§10.3):** `GenericArg` accepts module-qualified functions, methods, and
  instantiated generic functions (already parsed by `NamedType`, preserving LL(1)).
- **Type Checking (§10.5):** Definition-site checked strictly against the parameter's
  function signature.
- **Monomorphization (§10.6):** Include the resolved function symbol in the instantiation
  key. Calls to `HASH(...)` or `LESS(...)` lower to direct calls, enabling full inlining
  by `inline.ax` and LLVM.
- **Separate Compilation (§16.7):** If a `const` function argument is marked `inline`,
  include its body in the calling unit's cache key (same rule as inline generic callees).

### 2.2 Lane-Parameterized SIMD Constraints — BREAKING (`FloatVector`)
- **Breaking change:** Replace `V: FloatVector` (0 uses in `std/` and `examples/`; 4 test
  files) with lane-parameterized constraints:
  - `V: FloatVector[S]`
  - `V: IntVector[S]`
  - `V: Vector[S]`
  where `S` is a concrete lane type or earlier generic parameter (e.g.,
  `fn dot[S: Float, V: FloatVector[S]](a: []S, b: []S) S`).
- **New builtin:** `@lanes(V) usize` (compile-time constant lane count).
- **Permitted operations in body:** `@splat(x: S)`, `@load_partial(V, s: []S)`,
  `@store_partial(s: []mut S, v: V)`, `@reduce_add(v: V) -> S`, `@dot`.

### 2.3 Stdlib Container Migration — BREAKING (Stdlib API)
- Migrate `std/collections/map.ax`, `set.ax`, `treemap.ax`, `heap.ax`, `lru.ax`, and
  `vec.ax` (`sort`, `sort_stable`, `binary_search`, `lower_bound`, `upper_bound`) to
  `const` function generic parameters.
- Mark core hash/equality/order helpers in `std/hash.ax` as `inline`.
- Keep `*_dyn` variants (`sort_dyn`, `MapDyn`) where runtime-chosen comparators are needed.
- Add static `fmt.write_to[W, const WRITE: fn(*mut W, []u8) !usize]` alongside dynamic
  `io.Writer`.
- Verify benchmark speedups on `cmd/axiom/bench/hashmap.ax` and `cmd/axiom/bench/sort.ax`.

---

## Phase 3: Closing the "No UB Outside Raw Pointers" Safety Gap (Breaking)

Addresses weaknesses #6a, #6c, and #6d. Scheduled after Phase 1 so `for &mut` and ref
`match` are already available as replacements.

### 3.1 Transitive Immutability on `*T` and `[]T` — BREAKING (Language)
- **Spec (§7.2):** Projecting a field of type `*mut U` or `[]mut U` through an immutable
  place (`*S`, `[]S`, or non-`mut` local `S`) weakens the resulting type to `*U` or `[]U`.
  Obtaining `*mut U` or `[]mut U` requires a mutable base (`*mut S`, `[]mut S`, or `let mut`).
- **Diagnostic:** Emit `E0707` with a machine-applicable `agent-json` fix suggesting
  `*mut Self` on the receiver or immutable `[]T` on the return type.
- **Stdlib & compiler migration:** Split `Vec.items` (`*Self -> []T`) / `Vec.items_mut`
  (`*mut Self -> []mut T`), `Matrix.row` / `row_mut`, `List.at` / `at_mut`, etc.

### 3.2 Module-Private Fields by Default — BREAKING (Language)
- **Grammar (§3.2):** `Field ::= "pub"? IDENT ":" Type` (LL(1)).
- **Visibility (§4.2):** Fields without `pub` are accessible only inside their declaring
  module (`E0203` from outside, with note pointing to the field).
- **Migration:** Add a temporary `axiom fmt --pub-fields` pass (or migration script) to
  mark plain-data struct fields `pub` in `std/`, `examples/`, and `cmd/axiom/src/`, while
  leaving container internals (`Vec.buf`, `Vec.len`, `Map.ctrl`, `Mutex.state`,
  `BigInt.limbs`) private.

### 3.3 Non-Copyable Structs (`nocopy`) — BREAKING (Stdlib API)
- **Grammar (§3.2):** `StructDecl ::= "nocopy"? "struct" ...`,
  `UnionDecl ::= "nocopy"? "union" ...` (contextual identifier; 0 collisions in repo).
  Any aggregate containing a `nocopy` field/element is implicitly `nocopy`.
- **Semantics (strictly local, no lifetimes):**
  - Copying a `nocopy` value from a place (`let b = a;`, `f(a)`, `return p.*;`,
    `for x in s`) is compile error `E0708` (with fixes suggesting `&a`, `&mut a`,
    `for &mut x in s`, or `.clone()`).
  - Allowed moves: direct construction from rvalues (`let mut v = Vec.[u32].init();`,
    `Self.{ v: v }` from a local), `return v;` of a local, and `@swap(&mut a, &mut b)`
    (new builtin).
  - Definite-use check within the function body: using a `nocopy` local after it has been
    moved is `E0709`.
- **Stdlib:** Mark owning and address-pinned types `nocopy`: `Vec`, `Map`, `Set`,
  `TreeMap`, `Heap`, `Lru`, `Deque`, `Arena`, `String`, `BigInt`, `File`, `Mutex`,
  `Condvar`, `WaitGroup`, `Channel`, `Thread`, and `json`/`toml`/`yaml` `Document`.

### 3.4 Hard Local-Escape Error (`E0710`) + Stack ASan — BREAKING (Language)
- **Compile-time check:** Promote `W0710` to compile error `E0710` and extend
  `check_escape` (`cmd/axiom/src/sema.ax`) with
  intra-procedural local-origin taint tracking across `let` bindings, assignments, slicing
  (`a[lo..hi]`), field projection, and struct literals. Reject returning a local-derived
  pointer/slice or storing one into a global or parameter-reachable place.
- **Runtime stack ASan (`--sanitize=address`):** Extend `codegen.ax` and `std/mem.ax` so
  address-taken local arrays and structs are allocated in an ASan stack quarantine frame
  and poisoned upon function exit, trapping indirect stack use-after-return (e.g., via
  `return io.fmt_u64(buf[..], n);`) with `T0012`.

---

## Phase 4: Error-Set Reform (Breaking)

Addresses weakness #7.

### 4.1 Inline Anonymous Error Sets in Types (Non-Breaking)
- **Grammar (§3.3):** `Type ::= "{" ErrorNames "}" "!" Type | ...` (LL(1): `{` cannot
  otherwise start a `Type`).
- Allows leaf functions to declare exact minimal sets inline (e.g.,
  `pub fn push(self: *mut Self, x: T) { OutOfMemory }!void`) without polluting callers
  with unrelated module-wide errors.

### 4.2 Error-Set Generic Parameters (Non-Breaking)
- **Grammar (§10.2):** `GenericParam ::= ... | "error" IDENT` (LL(1)).
- Allows higher-order utilities (`fn retry[error E, T](f: fn() E!T) E!T`) to preserve the
  caller's exact error set rather than widening to `!T`.

### 4.3 Dead-Error Diagnostics & Private Inference (Non-Breaking Warning)
- Keep `pub fn` error sets explicit so separate compilation (§16.7) interfaces never depend
  on non-generic function bodies.
- Allow private (`fn`, non-`pub`) functions to write `infer!T` for body-inferred closed
  error sets.
- Emit `W0655` ("declared error set contains errors this function can never return") with
  an `agent-json` machine-applicable fix that narrows the set.

### 4.4 Stdlib Migration & Diagnostic Context Convention — BREAKING (Stdlib API)
- Migrate absence operations from `!T` (`NotFound`/`Empty`/`Done`) to `?T`: `Map.get`,
  `TreeMap.get`, `Vec.pop`, `Vec.last`, `Iter.next`, `http.find_header`,
  `json.Value.get/at`, `toml.Value.get/at`, `str.index_of_*`.
- Narrow stdlib error sets (`Vec.push` -> `{ OutOfMemory }!void`, `Arena.bytes` ->
  `{ OutOfMemory }![]mut u8`, decompose `HttpError` into layered sets via `error A = B | C`).
- Standardize a small `std/diag.ErrorInfo` struct (`offset: usize, line: u32, col: u32,
  detail: []u8`) for parser/validator out-params, keeping `error` itself a zero-overhead
  16-bit code on the fast path.

---

## Phase 5: Multi-File Modules & Compiler Dogfooding (Non-Breaking)

Addresses weakness #8 and exercises Phases 1-4 inside the self-hosted compiler.

### 5.1 Multi-File Modules via `import part`
- **Grammar (§3.1, §4.1):**
  `ImportDecl ::= "import" ( STRING_LIT "as" IDENT ";" | "part" STRING_LIT ";" | "host" ... )`
  (`part` is contextual directly after `import`, matching `import host`; zero new keywords).
- **Semantics:**
  - Only a module's entry file may declare `import part "sub.ax";`.
  - A part file cannot be imported directly by other modules (`E0221`) and cannot belong
    to more than one module (`E0222`).
  - All parts of a module share one top-level namespace (including module-private items and
    fields) and may mutually refer to each other freely without `E0210`.
  - Each part declares its own `import "..." as alias;` lines so every file remains
    locally self-contained for agents.
  - Inter-module imports remain strictly acyclic (`E0210`).
- **Toolchain updates:**
  - `modload.ax`, `sema.ax`, `units.ax`: load part files into the owning `ast.Module`, hash the combined module
    interface for separate-compilation caching (§16.7), and optionally emit one cached
    object per part sharing that interface key.
  - `fmt.ax` and `lsp.ax`: tidy `import part` lines and resolve cross-part symbols.

### 5.2 Compiler & Stdlib Refactoring
- Remove the `threadlocal` `set_hooks` / `unset_*` callback tables in
  `cmd/axiom/src/semabuiltin.ax` and `cmd/axiom/src/cgbuiltin.ax` by making them parts of
  `sema.ax` and `codegen.ax`.
- Split 5k-7k LOC monoliths (`ir.ax`, `sema.ax`, `a64.ax`, `wasm.ax`, `std/text/html.ax`,
  `std/net/http.ax`) into cohesive 400-800 LOC parts.
- Migrate `cmd/axiom/src/` from `nil()`/`isnil()` to `?*T` and replace the manual
  `x: usize` type-punning in `cmd/axiom/src/ast.ax` with a tagged `union` using Phase 1's
  reference-binding `match`.
