# Compiler Architectural Roadmap

This document lays out six architectural improvements to the self-hosted Axiom
compiler (`cmd/axiom/src/`). Each item addresses a structural constraint left
over from the initial port from `cmd/axiom/bootstrap/` or from how features
grew around single-shot process execution and acyclic module imports.

Scope: all six items live in `cmd/axiom/src/` (and, where noted, host helpers
in `std/`). None changes the Axiom language (`docs/spec.md`) or front-end
diagnostics, so `cmd/axiom/bootstrap/` needs no changes except where its `-g`
parity check in the test suite can be widened once item 6 lands.

---

## Summary & Sequencing

| # | Area | Primary modules | Core payoff |
|---|---|---|---|
| 1 | Process-free separate compilation & threaded codegen | `util.ax`, `units.ax`, `codegen.ax`, `ir.ax`, `backend.ax`, `x64*.ax`, `a64.ax`, `wasm.ax`, `nlink.ax` | Object cache and parallel builds work natively on Windows and WebAssembly, without `fork()` |
| 2 | Native linker dead-symbol garbage collection | `elf.ax`, `ftab.ax`, `x64emit.ax`, `a64.ax`, `nlink.ax`, `elink.ax`, `mlink.ax`, `plink.ax` | `--units=module` and `axiom test` strip unused `std/` code, duplicate weak generics, and dead frame-table entries |
| 3 | Intrusive SSA def-use lists & worklist rewrites | `ir.ax`, `opt.ax`, `combine.ax`, `slp.ax`, `lower.ax`, `isel.ax`, `wasm.ax` | `O(uses)` value replacement; eliminates `fwd` chains, `uses_stale`, and four divergent `count_uses` walks |
| 4 | Two-tier arena incremental session in `axiom lsp` | `util.ax`, `ast.ax`, `types.ax`, `sema.ax`, `units.ax`, `ide.ax`, `lsp.ax` | Millisecond editor queries without spawning `axiom ide` or re-checking `std/` on every keystroke |
| 5 | Decomposing large modules & C-era `i32` tags | `x64*.ax`, `sema.ax`, `codegen.ax`, `gvn.ax`, `ast.ax`, `ir.ax`, `mach.ax` | Leaf context modules (`x64ctx.ax`), smaller pass files under acyclic imports, and exhaustive `enum` tags |
| 6 | DWARF `.debug_line` (and `.eh_frame`) on ELF & Mach-O | `main.ax`, `x64emit.ax`, `a64.ax`, `elink.ax`, `mlink.ax` | `-g` produces source-level line tables (and optional unwind info) on Linux and macOS, matching Windows `.pdb` |

### Recommended order

```
  [5a: x64ctx + backend state structs]
                  │
                  ▼
  [1: Explicit compilation context + unit_arena + thread pool]
          │                                       │
          ▼                                       ▼
  [4: Two-tier arena in-process LSP]      [2: nlink dead-symbol GC]
                                                  │
                                                  ▼
  [3: Intrusive IR def-use lists]         [6: ELF/Mach-O DWARF .debug_line]
          │
          ▼
  [5b: sema / codegen / gvn splits + typed enums]
```

- **5a before 1**: extracting backend state structs (`x64ctx.ax`, `a64` state,
  `ir`/`codegen` module state) gives item 1 clean per-unit state boundaries
  instead of ad-hoc global resets.
- **1 before 4**: once front-end and type-table state are explicitly owned
  rather than stored in process-lifetime globals, `lsp.ax` can hold a persistent
  session in-process.
- **2 before 6**: dead-symbol GC in `nlink.ax` ensures DWARF `.debug_line` and
  `.eh_frame` emitters only emit entries for live functions, avoiding tombstones
  for discarded weak generic copies and unused `std/` functions.
- **3 is independent** of linker and driver work and can proceed in parallel,
  ideally before splitting optimizer passes in **5b**.

---

## 1. Decouple Separate Compilation and Parallelism from POSIX `fork()`

### Current state

The compiler relies on single-shot process isolation in two places:

1. **Per-module separate compilation (`units.ax`, lines 262–353)**:
   `build_units` parses and checks the whole program once in the parent process,
   then calls `spawn_unit` -> `u.fork()` so each uncached module lowers to IR,
   optimizes, emits `.o` into `<cache>/obj/<key>.o`, and `_exit`s without
   polluting the parent's module-level state.
2. **Parallel whole-program codegen (`backend.ax`, lines 215–336)**:
   at `-O2`/`-O3`, `compile_parts` runs inlining once on the whole `ir.Module`,
   partitions surviving functions deterministically by IR instruction count,
   and `u.fork()`s one child per part to run post-inline passes and machine
   code emission.

Two architectural issues follow from this:

- **Module-level mutable globals prevent in-process reuse.** Per-compilation
  state sits in `let mut` globals across `types.ax` (`ttab`), `sema.ax`
  (`g_prog`), `codegen.ax` (`M`, `inst_map`, `fn_map`, `vtables`, ...),
  `ir.ax` (`CINTS`, `derived`, `fn_arena`), `x64emit.ax` (`OBJ`, `MF`, `F`,
  `FT`, `fsym`), `a64.ax` (`OBJ`, `MF`, `F`, `consts`), and `nlink.ax` (`objs`,
  `gsyms`).
- **Non-POSIX hosts silently lose separate compilation and parallelism.** On
  Windows (`std/internal/runtime/windows.ax:1189`, `ax_fork` returns `AX_ERR`)
  and WebAssembly (`host.mjs`), `u.fork()` returns `-1`. Whenever `todo.len > 0`
  in `units.build_units`, `spawn_unit` fails on the first miss, `build_units`
  returns `-1`, and `main.ax:806-808` sets `separate = false` and falls back to
  whole-program compilation (`CG_WHOLE`). As a result, a Windows-hosted or
  Wasm-hosted compiler never populates the object cache and compiles all parts
  serially.
- **Residual POSIX path/tool assumptions** remain in `main.ax:208` (`axiom cache
  clean` spawns `["rm", "-rf", dir]`) and `util.ax:207-224` (`self_path` splits
  `PATH` on `':'` and joins with `'/'` when `ax_self_path` falls back).

### Target design

1. **Explicit per-unit compilation context (`UnitCtx` / `BackendCtx`)**:
   - Group all `codegen.ax` state into a `CodegenCtx` struct, `ir.ax` module-wide
     tables (`CINTS`, `derived`, interned IR types) into `ir.Module` (or an
     `ir.Ctx` owned by `ir.Module`), and backend per-object/per-function state
     into `x64ctx.Ctx`, `a64.Ctx`, and `wasm.Ctx`.
   - Keep `u.intern` (the global string intern table) shared: interned `[]u8`
     slices are immutable and content-addressed. Make `u.intern` lock-protected
     (or give worker threads read-only access to pre-interned front-end symbols
     plus a thread-local symbol table for backend-synthesized symbol names such
     as `$pN` part suffixes and constant-pool labels).
2. **Tiered arenas in `util.ax`**:
   - Split the current permanent arena (`perm`) from a resettable `unit_arena`
     alongside the existing per-function `fn_arena` (`u.scratch_on` /
     `u.scratch_reset`).
   - During `--units=module`, the checked AST and `TypeTable` stay in `perm`
     (deferring `u.release_tree` in `codegen.ax` until all units have been
     lowered, rather than mutating AST pointers destructively mid-loop). Each
     module's `CG_MODULE` IR, optimization, and object serialization run in
     `unit_arena`, which resets between modules in single-threaded mode (e.g.
     WebAssembly), bringing full `<cache>/obj/` caching to Wasm hosts with
     bounded peak memory.
3. **Worker-pool parallelism via `std/thread`**:
   - On native hosts (Linux, macOS, Windows), drive `units.build_units` and
     `backend.compile_parts` with `std/thread/pool.ax` (each worker owning its
     own `unit_arena` and `fn_arena` backed by `std/mem`'s per-thread heaps).
   - Keep `compile_parts`'s deterministic size-balanced partitioning unchanged so
     the resulting object list and linked binary remain bit-identical regardless
     of thread scheduling or job count (`-j`).
   - Pass compiled in-memory `elf.Obj` / Wasm object buffers directly from
     `compile_parts` workers to `nlink` / `wlink` without temporary `/tmp` object
     files (matching how `wlink` already links single-part builds from memory).
4. **Host cleanup**:
   - Replace `["rm", "-rf", dir]` in `main.ax` with recursive directory removal
     via `std/os/fs.ax`.
   - Handle `';'` and `'\\'` (and `.exe` suffix probing) in `util.self_path` on
     Windows.

### Invariants & verification

- Bit-identical outputs for `--units=module` and `--units=program` across `-j 1`
  and `-j N`.
- `AXIOM_PARTS=4` produces byte-identical binaries whether run with 1 worker or
  4 workers.
- Cache hit/miss tests (`cmd/axiom/internal/tests/units/`) pass when the
  compiler itself runs under Node (`axiom.wasm`) and natively on Windows.

---

## 2. Dead-Symbol and Section Garbage Collection in `nlink.ax`

### Current state

The WebAssembly linker (`wlink.ax`, lines 113–140) builds a reachability graph
from exported functions and globals (`flive`, `slive`), dropping unreferenced
functions and data segments from the final `.wasm` module.

The native linker (`nlink.ax`) does not:

- Both native backends (`x64emit.ax:20`, `a64.ax:80`) emit every function of an
  object into a single `.text` section (`TEXT`) and every read-only constant
  into a single `.rodata` section (`RODATA`).
- `nlink.place` (`nlink.ax:356-381`) and `nlink.copy_class` (`nlink.ax:398`)
  concatenate every input section of each class (`C_TEXT`, `C_CONST`, `C_DATA`,
  `C_BSS`, `C_TLS`) unconditionally.
- In `--units=program` (`-O2` default), `codegen.ax` (`mark_reachable`) and
  `inline.ax` (`dead_funcs`) already prune dead functions before emission.
  In `--units=module` (`-O0`/`-O1` default, and `axiom test`), each cached `.o`
  is compiled with `CG_MODULE` (`codegen.ax:5687-5703`), which emits:
  1. Every non-generic function in that module (so importing one constant or
     helper from a large `std/` module links the entire module's machine code).
  2. `LINK_LINKONCE` (`elf.B_WEAK`) copies of every generic instantiation used
     by that module. `nlink.ax` resolves symbol references to the first object's
     copy (and `plink.ax:1031-1037` skips losing copies when writing `.pdb`),
     but the losing copies' machine code, read-only pools, `.pdata`/`.xdata`
     records, and `axiom_rets` frame-table entries remain in the linked binary
     (and get hashed page-by-page by `mlink.ax` on macOS).

### Target design

1. **Atomized sections (or per-symbol subsections) in `elf.Obj`**:
   - Emit each function into its own `C_TEXT` section (or record a table of
     *atoms* `[sec, off, size, reloc_range, align]` inside `elf.Obj` so the
     cached `.o` file format does not pay 64-byte ELF section-header overhead
     per tiny function if atom metadata is stored compactly, or use standard
     `.text.<name>` / `.rodata.<name>` sections).
   - Standard per-function/per-datum sections (`.text.<sym>`, `.rodata.<sym>`,
     `.data.<sym>`, `.bss.<sym>`) are the cleanest fit because `elf.Obj` already
     serializes sections, relocations (`Rela` per section), and symbols cleanly,
     and external `.o` files compiled with `-ffunction-sections -fdata-sections`
     gain GC automatically.
   - Associate auxiliary metadata sections with their owning text section:
     - `axiom_rets` entries (`ftab.ax`) and `axiom_chains` records.
     - Windows `.pdata` and `.xdata` unwind records (`x64emit.ax:22-23`).
     - `.axiom.lines` per-function line chunks (`x64emit.ax:21`).
2. **Reachability mark pass in `nlink.ax`**:
   - Before `classify` / `place` / GOT sizing, seed a worklist with:
     - The entry symbol (`_start` / `main` / runtime entry).
     - All `export` symbols (for shared/C-callable builds).
     - Linker-referenced runtime hooks (`axiom_trap`, `ax_unavailable`, TLS init,
       asan init constructors if present).
     - Any non-GC-able `.init_array` / Foreign `.o` sections lacking section
       granularity.
   - Pop a live section `(oi, sec_idx)`; mark it live, and for every `elf.Rela`
     in that section resolve `(oi, rela.sym)`:
     - If global (`gi >= 0`), resolve to the **winning** `(g.obj, g.sym)` only
       (automatically leaving all losing `B_WEAK` duplicate generic sections
       unmarked!).
     - If local (`B_LOCAL`), resolve to `(oi, sym.sec)`.
     - Push the target section if not yet marked.
   - Allocate GOT/PLT slots (`build_got`), assign virtual addresses (`place`),
     copy bytes (`copy_class`), emit symbol tables (`elink`, `mlink`, `plink`),
     and emit `axiom_rets` / `.pdata` / debug line tables **only for marked
     sections**.

### Invariants & verification

- `axiom test` and `-O0`/`-O1` `--units=module` binaries shrink substantially
  on programs importing `std/` modules and contain zero duplicate weak generic
  bodies in `.text` or stale entries in `axiom_rets`.
- Stack traces (`cmd/axiom/internal/tests/traces/`), crash reports, Windows
  SEH unwinding (`windows/`), and `axiom profile` resolve every live return
  address identically before and after GC.

---

## 3. Intrusive SSA Def-Use Lists in `ir.ax`

### Current state

`ir.Value` (`ir.ax:860-891`) stores operands directly (`a`, `b`, `c: *mut Value`
plus `xs: List[*mut Value]` for calls and phis) and tracks consumers with only:

- `uses: i32` — a reference count, and
- `user: *mut Value` — valid **only** when `uses == 1`,
- `fwd: *mut Value` — a forwarding pointer set by `ir.forward(old, new)`.

Because a value with two or more uses does not know where those uses live,
replacing a value across a function requires setting `old.fwd = new` and later
walking every instruction and operand of the entire function in
`ir.apply_forwarding(f)` (`ir.ax:2089-2133`), often paired with a second full
walk in `ir.count_uses(f)` (`ir.ax:2510-2548`).

Consequences across the middle-end and backends:

1. **Four subtly different `count_uses` walks** exist:
   - `ir.count_uses` (`ir.ax:2510`): counts references (an instruction `add x, x`
     adds 2 to `x.uses`) and tracks `user` when `uses == 1`.
   - `lower.count_uses` (`lower.ax:282`): resets and recounts `uses` without
     maintaining `user`.
   - `isel.count_uses` (`isel.ax:334`): counts **distinct user instructions** per
     value (using `mark`) so `x + x` counts as 1 user for address-mode/load
     folding!
   - `wasm.count_uses` (`wasm.ax:2319`): counts references plus implicit trap/ret
     users for WebAssembly expression-tree stackification.
2. **Stale-count hazards (`uses_stale`)**: `combine.ax` maintains a manual
   `uses_stale` flag whenever a transform mutates operands without running a
   full recount, disabling single-use combines until the next sweep.
3. **Repeated whole-function sweeps inside local transforms**: passes such as
   `sink_phi_ops` (`combine.ax:2451-2485`) and straight-line vectorization
   (`slp.ax:295-309, 1320, 1703`) call `apply_forwarding(f)` + `count_uses(f)`
   after local rewrites, turning local pattern matches into `O(|F|)` scans per
   firing.

### Target design

1. **Explicit `ir.Use` edges**:
   ```axiom
   pub struct Use {
       val: *mut Value,      // the operand value being read
       user: *mut Value,     // the instruction (or synthetic Terminator/Ret node) reading it
       prev: *mut Use,       // doubly-linked ring/list on `val.first_use`
       next: *mut Use,
       slot: u32,            // operand index in `user` (0=a, 1=b, 2=c, 3+k = xs[k])
   }
   ```
   - Embed `in_uses: [3]Use` inline inside `ir.Value` for the fixed `a`, `b`, `c`
     slots (zero extra allocations for 95%+ of IR instructions, which have <= 3
     operands), and allocate `Use` arrays alongside `xs` only for `Call` and
     `Phi` nodes.
   - Represent block branch conditions (`Block.cond`) and `Block.ret` either as
     explicit terminator `Value`s (`VK_BR`, `VK_RET`, `VK_SWITCH`) at the tail
     of `Block`, or give `Block` two inline `Use` slots (`cond_use`, `ret_use`)
     tagged so `Use.user` covers block-level uses uniformly without special
     cases in every walk.
2. **`O(1)` operand mutation and `O(uses)` RAUW**:
   - Provide `ir.set_op(inst, slot, new_val)` and `ir.rauw(old_val, new_val)`:
     `rauw` walks `old_val.first_use`, rewrites each consumer slot in `O(1)`,
     splices the `Use` list onto `new_val.first_use`, and updates `old_val.uses`
     and `new_val.uses` in place.
   - Provide `ir.has_one_use(v)` (`v.uses == 1`), `ir.single_user(v)`
     (`v.first_use.user`), and `ir.has_single_user_inst(v)` (true even for
     `add x, x`, which checks whether all `Use` nodes on `v.first_use` point to
     the same `user` — replacing `isel.count_uses`'s custom walk in `O(1)` for
     1–2 uses!).
   - Remove `Value.fwd`, `fwd_epoch`, `ir.forward`, `ir.resolve`,
     `ir.apply_forwarding`, and `uses_stale`.
3. **Worklist-driven `combine.ax`**:
   - When `rauw(old, new)` splices uses onto `new`, push `new`'s users onto
     `combine`'s instruction worklist directly instead of re-sweeping all blocks
     of `f` up to 4 iterations.
4. **Debug verification**:
   - Extend `AXIOM_CHECK_IR=1` to verify after every pass that every `Value`'s
     `first_use` list length equals `v.uses` and every `Use` points back to a
     live instruction in `f` whose operand slot equals `v` (catching dangling
     operands immediately, especially combined with `AXIOM_POISON=1`).

### Invariants & verification

- `AXIOM_CHECK_IR=1`, `AXIOM_CHECK_CINTS=1`, and `AXIOM_POISON=1` pass across
  `std_opt.mjs`, `run.mjs`, and `fuzz/run.sh`.
- Generated machine code on `cmd/axiom/bench/` is at least as good as before
  (and compile time at `-O2` on functions with thousands of blocks drops).

---

## 4. Two-Tier Arena Incremental Session for `axiom lsp`

### Current state

`lsp.ax` (`cmd/axiom/src/lsp.ax:354-460`) speaks JSON-RPC over stdio and keeps
open editor buffers in memory, but delegates every semantic action (`check`,
`hover`, `definition`, `references`, `rename`, `complete`, `signature`,
`symbols`) by spawning a fresh `axiom ide` child process (`ide.ax:70-109`),
streaming all open buffers over `stdin`, reading JSON from `stdout`, and letting
the child exit.

Every hover, completion, signature-help popup, or debounced edit therefore pays:
- Process creation (`fork`/`exec` on Unix, `CreateProcessW` on Windows),
- Re-reading, lexing, parsing, and type-checking every transitive `std/` and
  package module from scratch.

The current design was chosen deliberately because the front end allocates into
a single bump arena without destructors. We can preserve bump-arena simplicity
while eliminating both subprocess spawning and redundant `std/` checking.

### Target design

1. **Two-tier arena lifetime (`session_arena` + `deps_arena` + `query_arena`)**:
   - **`session_arena`**: lives for the lifetime of `axiom lsp` (open document
     table, workspace configuration, `axiom.toml` package resolution).
   - **`deps_arena`**: holds the lexed, parsed, and type-checked `Module`s and
     `TypeTable` entries of **clean** dependency modules (`std/*` and external
     `pkg:` dependencies, plus workspace files without open unsaved edits whose
     on-disk mtime/hash has not changed).
   - **`query_arena`**: reset at the end of every LSP request. Holds the ASTs,
     local scopes, expression types, instantiated generic signatures, and
     diagnostics of the active/dirty module(s) being edited or queried.
2. **Interface-hash invalidation (reusing `units.ax`'s `interface_key`)**:
   - `units.ax` (`blank_bodies` / `interface_key`) already computes a hash of a
     module's public/exported surface with function bodies stripped.
   - Cache each module's `interface_key` in `lsp.ax`:
     - When an edit inside an open buffer only changes function bodies (the vast
       majority of keystrokes during editing), its `interface_key` does not
       change: **only that single module** needs to be re-parsed and body-checked
       in `query_arena` against the frozen `deps_arena` symbol/type tables!
     - When an edit changes a top-level signature, `struct`, `enum`, `shape`, or
       `import`, rebuild the affected downstream workspace modules; rebuild
       `deps_arena` only when a clean dependency file changes on disk or when
       `deps_arena` fragmentation crosses a threshold (e.g. 128 MiB).
3. **Scoped `TypeTable` & `Diagnostics` rollback**:
   - Partition `types.TypeTable` with a watermark (`base_len` frozen at the end
     of `deps_arena` check): types interned during a `query_arena` check live
     above the watermark in `query_arena` buckets and are discarded in `O(1)` by
     restoring bucket heads to `base_len` when `query_arena` resets.
   - Collect diagnostics into a `diag.Sink` value inside `CodegenCtx`/`SemaCtx`
     instead of printing directly to `fd 2`, so `lsp.ax` reads structured
     `diag.Item` slices in-memory without JSON serialization/deserialization
     between two processes.
   - Keep `axiom ide` CLI subcommand working as a thin wrapper around the exact
     same in-process `ide.Session` API so existing CLI/editor tests continue to
     exercise it directly.

### Invariants & verification

- Every existing `axiom ide` and `axiom lsp` test returns identical spans, hover
  types, completions, and diagnostics whether answered on a cold session or
  after 100 incremental body/signature edits.
- Add an `ide`/`lsp` replay test editing function bodies and top-level
  declarations across multi-module packages with `AXIOM_POISON=1` enabled on
  `query_arena` resets to guarantee no pointer in `deps_arena` ever references
  `query_arena` memory.

---

## 5. Decompose Acyclic-Import "God Modules" and C-Port Tag Constants

### Current state

Axiom forbids cyclic module imports (`sema.ax:243-246`). When the compiler was
ported from C (where `.c` files shared header declarations freely) and grew new
optimizations and targets, mutually coupled data structures or inverted state
ownership caused several files to grow very large:

- `x64.ax` (8,885 lines) + `x64emit.ax` (2,731 lines): `x64.ax` imports
  `x64emit.ax`, which forced per-object and per-function state (`OBJ`, `TEXT`,
  `MF`, `F`, `AVX`, `UPPER`, `WIN` in `x64emit.ax:18-36`) down into the emitter
  so both modules could touch it.
- `sema.ax` (6,368 lines): combines module graph loading and package visibility
  rules, top-level declaration resolution, mutually recursive type/stmt/expr/
  builtin checking, and post-check validation passes (`gate_fn`,
  `check_inline_cycles`, `check_sizes`).
- `codegen.ax` (5,770 lines): combines monomorphization, expression/statement IR
  lowering, runtime/entry synthesis, ASan instrumentation, shadow-stack/frame-
  table plumbing, and `@...` builtin lowering (with x86-64 capability queries).
- `gvn.ax` (5,594 lines): houses dominator-tree GVN + value-range facts alongside
  five distinct passes: dead store elimination (`dse`), shadow-stack optimization
  (`ss_opt`), loop-idiom recognition (`loop_idioms`), LICM (`licm`), loop-range
  check splitting (`split_loops`), and code sinking (`sink`).

In addition, while `ExprKind`, `StmtKind`, `TyKind`, and `ValueKind` have been
migrated to native Axiom `enum`s, several core discriminators remain raw `i32`
constants from C (`ast.OP_*`, `ast.BI_*`, `ast.Decl.state`, `ir.IN_*`, `ir.CMP_*`,
`nlink.C_*`, `mach.K_*`), losing type-safety and `match` exhaustiveness.

### Target design

Apply the **leaf context + pure pass** pattern (the same pattern `ast.ax`,
`ir.ax`, and `mach.ax` already use successfully):

1. **Split the x86-64 backend (`x64*.ax`)**:
   - Introduce `x64ctx.ax` holding `Ctx` (`obj`, `mf`, `f`, target feature flags,
     frame/unwind metadata, ABI convention) plus x86-64 register/condition constants.
   - Decompose `x64.ax` into:
     - `x64abi.ax`: System V and Windows x64 parameter/return classification,
       call lowering, prologue/epilogue frame sizing.
     - `x64simd.ax`: SSE2 / SSSE3 / SSE4.1 / AVX2 / AVX-512 vector and mask
       (`k1`) selection, shuffle/permute synthesis, and `codegen.ax`'s target
       capability queries (`perm_ok`, etc., removing `codegen.ax`'s need to
       import all of `x64.ax`!).
     - `x64isel.ax`: scalar/memory instruction selection and address folding.
     - `x64emit.ax` & `x64enc.ax`: post-RA peepholes, jump relaxation, placement,
       unwind/line emission, and binary encoding.
2. **Split `gvn.ax` into focused middle-end modules**:
   - Keep value numbering, memory SSA/alias queries, and branch/range facts in
     `gvn.ax` (exposing a read-only `Facts` / alias query helper if needed).
   - Extract `dse.ax` (dead store elimination + `ss_opt` shadow-stack elision)
     and `loopopt.ax` (`licm`, `loop_idioms`, `split_loops`, `sink`), orchestrated
     from `passes.ax`.
3. **Trim `sema.ax` and `codegen.ax` at phase boundaries**:
   - From `sema.ax`, extract `modload.ax` (file discovery, import path/visibility
     checks, cycle detection, topological module ordering, producing an unchecked
     `Program`) and `semapost.ax` (post-typecheck structural checks: `gate_fn`,
     `check_inline_cycles`, `check_sizes`, unused-import/symbol warnings). Keep
     mutually recursive `check_decl` / `check_stmt` / `check_expr` /
     `check_builtin` together in `sema.ax`.
   - From `codegen.ax`, extract `cg_builtin.ax` (`@...` builtin lowering) and
     `cg_runtime.ax` (`main` / `test` / `profile` entry wrappers, ASan globals,
     CPU feature guard synthesis).
4. **Finish migrating `i32` tag constants to `enum`s**:
   - Convert `BinOp` / `UnOp` (`OP_*`), `BuiltinId` (`BI_*`), `DeclState`,
     `ir.Intrinsic` (`IN_*`), `ir.CmpPred` (`CMP_*`), `mach.OpKind` (`K_*`), and
     `nlink.SecClass` (`C_*`) to `enum` types, replacing `if`/`else if` chains
     with exhaustive `match` expressions.

### Invariants & verification

- Pure refactoring: `cmd/axiom/bootstrap.sh` reaches an identical stage 2 ==
  stage 3 fixed point, `axiom fmt --check` passes on every new file, and
  compiled test outputs are byte-for-byte unchanged.

---

## 6. DWARF `.debug_line` (and `.eh_frame`) on ELF and Mach-O

### Current state

- On `x86_64-windows`, the compiler emits full symbol and source-line debug
  information into `<out>.pdb` (`pdb.ax`, `plink.ax:1017-1050`), driven by
  `.axiom.lines` sections recorded during x86-64 encoding (`x64emit.ax:21`) from
  `ir.Value.src` -> `mc.MInst.src`.
- On Linux (`elink.ax`) and macOS (`mlink.ax`), `-g` / `--debug-info=lines` is
  parsed and accepted by `main.ax` (`main.ax:70, 333-335`) for CLI compatibility
  with the bootstrap compiler, **but ignored**: ELF and Mach-O outputs get a
  symbol table (`symtab`/`strtab`), yet no DWARF `.debug_line` or `.eh_frame`.
- `a64.ax` propagates `MInst.src` through `mach.ax`, but does not yet record
  `.axiom.lines` during encoding.

This is the one remaining functional capability where the C/LLVM bootstrap
compiler produces richer output than the self-hosted compiler.

### Target design

1. **Unified `.axiom.lines` emission across both native backends**:
   - Factor the compact `.axiom.lines` section writer out of `x64emit.ax` into a
     small shared helper (e.g. in `ftab.ax` or `dwarf.ax`), enabled whenever
     `WIN` (with PDB on) **or** `-g` / `--debug-info=lines` is passed.
   - Call it from both `x64emit.ax` and `a64.ax` as instructions are encoded,
     keyed by function section/symbol so item 2's linker GC drops line tables of
     dead functions automatically.
2. **DWARF v4 `.debug_line` encoder (`dwarf.ax`)**:
   - Add a self-contained `dwarf.ax` module (~350 lines, analogous to `pdb.ax`)
     that consumes the live functions' resolved addresses and `.axiom.lines`
     tables after `nlink.place` and emits a DWARF v4 line-number program
     (DWARF v4 is universally understood by `gdb`, `lldb`, `addr2line`, `perf`,
     `objdump -d -S`, and macOS `atos` / `dsymutil` without needing `.debug_str_offsets`):
     - Directory table + deduplicated file-name table (relative to build root
       when under it, for reproducible builds).
     - Compact line-number state machine opcodes (`DW_LNS_copy`,
       `DW_LNS_advance_pc`, `DW_LNS_advance_line`, `DW_LNS_set_file`, special
       opcodes, and `DW_LNE_set_address` / `DW_LNE_end_sequence` per contiguous
       live function).
   - Emit minimal `.debug_info` + `.debug_abbrev` compile-unit header
     (`DW_TAG_compile_unit` with `DW_AT_stmt_list`, `DW_AT_low_pc`,
     `DW_AT_high_pc`, `DW_AT_comp_dir`) because `gdb` and `lldb` look up
     `.debug_line` via a compile unit's `DW_AT_stmt_list`.
3. **Wire into `elink.ax` and `mlink.ax`**:
   - **`elink.ax`**: append non-`SHF_ALLOC` sections `.debug_abbrev`,
     `.debug_info`, and `.debug_line` after the loadable segments (zero runtime
     memory footprint; loaded only by debuggers/profilers).
   - **`mlink.ax`**: for `aarch64-macos`, either emit `__DWARF` segment sections
     (`__debug_abbrev`, `__debug_info`, `__debug_line`) in a companion
     `<out>.dSYM/Contents/Resources/DWARF/<out>` bundle (or directly in an
     unmapped `__DWARF` segment / `N_OSO` stab, matching how `lldb` and `atos`
     resolve Mach-O line info without inflating signed `__TEXT` pages).
4. **Optional `.eh_frame` unwind tables**:
   - Every Axiom function that makes a call already maintains a standard frame
     pointer (`rbp` on x86-64, `x29`/`x30` on AArch64), and leaf frame layouts
     are known in `x64emit.ax` and `a64.ax`. Emitting compact `.eh_frame` (CIE +
     per-function FDEs) lets external tools (`perf record --call-graph dwarf`,
     `gdb`/`lldb` across leaf functions, and linked C code throwing/unwinding)
     walk Axiom frames seamlessly, complementing Windows `.pdata`/`.xdata`.

### Invariants & verification

- Without `-g`, ELF and Mach-O binaries remain byte-for-byte unchanged (save any
  dead-code reductions from item 2).
- With `-g`, builds remain deterministic (same source tree path -> identical
  bytes).
- Add tests in `cmd/axiom/internal/tests/` verifying that `addr2line` / DWARF
  line parsing maps trap and call instruction addresses back to exact `.ax`
  file and line numbers at `-O0` and `-O2` on both x86-64 and AArch64, and
  enable the existing `-g` suite configuration against `cmd/axiom/build/axiom`.

---

## Cross-Cutting Verification Checklist

Every step of this plan must preserve the repository's existing invariants:

1. **Bootstrap fixed point**: `cmd/axiom/bootstrap.sh` succeeds with stage 2 and
   stage 3 bit-identical (`CROSS=1` included when touching backends or linkers).
2. **Canonical formatting**: `cmd/axiom/build/axiom fmt --check` passes across
   the entire repository.
3. **Differential parity**: `cmd/axiom/difftest.sh` passes (diagnostics, exit
   codes, `-O0`/`-O2`, `--units=module|program`, backend fuzzer vs LLVM).
4. **Optimizer & memory hygiene**: `node cmd/axiom/internal/tests/std_opt.mjs`
   passes with `AXIOM_CHECK_IR=1`, `AXIOM_CHECK_CINTS=1`, `AXIOM_POISON=1`, and
   `--sanitize=address`.
5. **Cross-target suites**: AArch64 (`qemu-aarch64`), Windows (`wine64`), macOS
   Mach-O structural checks (`macho.mjs`), and WebAssembly (`axiom.wasm` on
   Node) all pass.
