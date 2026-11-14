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
| 2 | Native linker dead-symbol garbage collection | `elf.ax`, `ftab.ax`, `x64emit.ax`, `a64.ax`, `nlink.ax`, `elink.ax`, `mlink.ax`, `plink.ax` | `--units=module` and `axiom test` strip unused `std/` code, duplicate weak generics, and dead frame-table entries (done: see §2) |
| 3 | Intrusive SSA def-use lists & worklist rewrites | `ir.ax`, `opt.ax`, `combine.ax`, `slp.ax`, `lower.ax`, `isel.ax`, `wasm.ax` | `O(uses)` value replacement; eliminates `fwd` chains, `uses_stale`, and four divergent `count_uses` walks (done: see §3 for what remains) |
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

### Status

**Separate compilation: done.** `units.build_units` compiles each uncached
module on a worker thread (`std/thread`), on every host with threads
(Linux, macOS, Windows, and WebAssembly under node), instead of in a forked
child. Parts (`backend.compile_parts`) still fork: see "What remains".

How it is done (the design differs from the one below, which threaded a
context through the code generator):

- **The per-unit state is thread-local.** Every mutable module-level
  variable of the modules `codegen.ax` imports, but for the front end's
  (`ast`, `lexer`, `parser`, `sema`, `types`, `package`, `diag`, `util`,
  `target`, `c/`), is `threadlocal` (5a had gathered most of it into a few
  structs), and so are `util.ax`'s arenas and output buffer, `diag.ax`'s
  diagnostics and `target.ax`'s per-function `g_features`. A worker thread
  starts with every one of them at its initial value, as a new process
  would; the main thread is unchanged. No resets were needed: a unit gets a
  thread of its own. The driver's settings (`codegen.ax`, `backend.ax`,
  `isel.ax`, `jstub.ax`'s `g_*`, set by `main.ax`) stay shared, read only
  while workers run.
- **The front end's tables are shared, read only, with a per-worker
  overlay.** A worker finds interned strings (`u.intern`), types
  (`types.ax`) and source files (`u.g_files`) in the main thread's tables
  first, and keeps what it adds in tables of its own (new type ids continue
  from the main thread's). What the front end computes lazily is computed
  before the workers start (`u.worker_start`, `ty.worker_start`): the
  types' layouts and the files' line tables. `codegen.ax`'s caches on
  `ast.Type` (`backend`, the size check) became per-thread tables by type id
  (the fields remain in `ast.Type`, unused).
- **Unit memory.** Everything a worker allocates goes back when it ends
  (`u.worker_end`): its arenas are its own, and `util.ax`'s heap blocks are
  on a per-thread list while `tracking` (a 32-byte header instead of 16).
  The syntax tree stays in the main thread's tree arena, so nothing has to
  defer `release_tree`. Diagnostics come back as text (`dg.save`, `dg.load`,
  as from a child process); modules are waited for in order, so they come in
  the same order whatever the threads do.
- `AXIOM_JOBS` sets the number of threads (the processors by default); it
  also bounds the parts compiled at once.
- Host cleanup: `axiom cache clean` (and a failed git checkout) use
  `std/os/fs.remove_all`, not `rm -rf`; `util.self_path` searches `PATH`
  as Windows has it (`;`, `.exe`).
- `wlink.ax` joins data segments that are (nearly) adjacent: the compiler
  built for wasm32 had more than V8's 100000 and would not load under node.

Verified: the cached objects (and executables) are byte-identical to the
forked children's for the compiler itself (x86-64 at -O0, -O1, -O1 -g, -O2,
--sanitize=address; AArch64 -O0 -g; wasm32; Windows -O1 -g; macOS) and
several examples, with `AXIOM_JOBS=1` and 3 alike; diagnostics from several
failing modules come in the same order; the compiler built for wasm32 builds
with the cache under node, and the compiler built for Windows does under
Wine (the compiler itself: 188 modules, the same executable as on Linux).
A module starts as soon as any thread is free (the forked children waited
for the oldest). On Linux (4 processors, a cold cache), a `--units=module`
self-build at -O1 went from 6.2-6.8 s to 4.0-4.2 s (user + system time 12.4
to 9.0 s; on one processor 13.3 to 10.6 s), its peak memory (PSS, all
processes) from 357-379 to 379-394 MB. The -O2 self-build (parts, still
forked) is unchanged within noise (about 30 s of CPU time, 2.0 GB peak in
the main process); an -O2 build of an example executes 0.5% fewer
instructions (callgrind): thread-local access costs nothing measurable.

### What remains

- **Parts on threads.** `compile_parts` still forks, and so is serial (one
  piece) where there is no fork. Its children share the post-inline module
  and every table made before it (the IR's interned types and constants,
  debug-information tables, free lists, `asm/inline.ax`'s blocks, ...), and
  each changes the module (`compile_part` turns the other parts' functions
  into declarations and, but in part 0, drops the data's initializers;
  `switches.lookup_tables` and `cover.instrument` add globals) and reads the
  other parts' functions (`tailrec.prepare_sibling`, the backends' symbols).
  Threads need, per part, a view of the module (its own function and global
  lists, declarations for the others' functions without changing the shared
  ones), the main thread's IR tables copied (or overlaid, as the front
  end's are), and the cross-function reads of `passes.finish_funcs` and the
  backends confined to what is fixed before the parts start. The IR's
  tables are what item 3 is reworking (def-use lists): do this after it.
- Parts' objects still go through files (`<path>.p<k>.o`), not memory.
- A host without threads (a browser without shared memory) still compiles
  the whole program as one unit.

### Current state (before this item)

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
  `gsyms`). (Since 5a the backends' are a few structs: `x64ctx.ax`'s `O` and
  `FN`, `a64.ax`'s `O` and `FN`, and each module's `S`; see section 5.)
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

### Status

**Done.** `--units=module` builds (the default at `-O0`/`-O1`, and `axiom
test`) put each function and datum of a module's object in a section of its
own and link with `--gc-sections` (`--no-gc-sections` turns it off);
whole-program builds (`-O2`, `--units=program`) are byte-for-byte unchanged
(checked: the compiler at `-O2`, examples on x86-64, AArch64, Windows and
macOS).

What the code already had when this item started (added for C's
`-ffunction-sections`-style linking): `--function-sections`,
`--data-sections` (`isel.ax`: `func_section`, `data_section`), COMDAT groups
for `--emit=obj`, an `axiom_rets` section per code section with
`SHF_LINK_ORDER` (`ftab.ax`), per-section `.pdata`/`.xdata` (`group_unwind`),
and the mark pass in `nlink.collect_garbage` (roots: the linker's objects,
`ROOT_NAMES`, init/fini arrays, C-identifier sections, `SHF_GNU_RETAIN`,
non-code sections; `SHF_LINK_ORDER` dependents, COMDAT group members, and
the LSDA/personality references of `.eh_frame`). It was off by default and,
for Axiom code, kept almost everything:

- **`axiom_rets` kept all code.** Its name is a C identifier, which made
  every frame table a root, and its relocations kept its code. A
  `SHF_LINK_ORDER` section is now collectable whatever its name.
- **Every weak copy was kept.** `gc_mark_sym` marked both the chosen
  definition and the referring object's own copy. It now marks only the
  chosen one; `elink` applies a relocation against a dropped copy's symbol
  to the kept one (`nlink.ref_dropped`; it skipped them, which COMDAT
  copies never needed because `drop_group` unbinds their symbols first).
- **Dead FDEs stayed in `.eh_frame`** (left out of `.eh_frame_hdr` only).
  `nlink.prune_fdes` now removes the FDEs of dropped code from each
  `.eh_frame` after the mark (CIE offsets rewritten, relocations moved; a
  section with symbols referring into it, an unknown layout or the 64-bit
  form is left alone).
- **Debug information of dropped code** (with `-g`) pointed at address 0
  and overlapped the program's own. `nlink.debug_sections` now writes the
  tombstone -1 for a relocation against dropped code (1 in `.debug_ranges`
  and `.debug_loc`), as WebAssembly's DWARF has it; the suite's DWARF
  checks (`dwarf.mjs`) pass over tombstoned subprograms and sequences on
  every target. (Leaving the DIEs out altogether needs rewriting
  `.debug_info`; not done.)
- **Chains and call-site records were per object.** `ftab.ax` put every
  call's chain into the object's one `.data.rel.ro`, which every
  `axiom_rets` section referred to, so all call-site records (most of
  `.data` in a `-O0` program) stayed. With `--function-sections` each code
  section's calls now have their own `.data.rel.ro.chains` section
  (`ftab.chain_at`, built as entries are written; `ir.Chain.off` is then a
  chain number).
- **One section per datum was too many.** `--data-sections` made a
  section for each of the ~200k call-site records of the compiler (object
  sizes +75%). The compiler's own internal data (names starting with `.`:
  call-site records, constants, strings) now goes into one section per
  function that first refers to it, its calls' chains included
  (`isel.data_owners`, `owned_section`); what only data refers to keeps a
  section of its own. Output sizes were the same as one per datum.
- **The unit cache's key did not include the section flags**
  (`units.options_id`): `--units=module --function-sections` reused objects
  built without. It does now.
- `elf.write` shares section names in `.shstrtab` (each once; a section
  with relocations named by the end of its `.rela` section's name), which
  matters with tens of thousands of sections per object. Executables are
  unchanged; `--emit=obj` objects are smaller.

Measured (x86-64 Linux, this machine):

| build | before | after |
|---|---|---|
| the compiler, `-O1 --units=module`: executable | 16.15 MB | 14.46 MB (text 3.79 MB -> 3.42, data 7.04 -> 6.06, `.eh_frame` 817 KB -> 668) |
| the same: cold build / relink with everything cached | 5.0 s / 1.1 s | 4.6-5.5 s / 1.2 s |
| the same: object cache | 33.6 MB | 44.6 MB |
| `examples/` at `-O0`: wordcount, md2pdf, texmath, http_server, pi, swarm, charts | 0.45, 7.98, 6.12, 2.83, 6.29, 2.52, 6.88 MB | 0.19, 4.76, 2.10, 1.46, 2.85, 1.44, 2.95 MB |
| the compiler building itself at `-O2` | 30-32 s, 2.23 GB | the same (output identical) |

Link times of the examples' cached rebuilds are unchanged (within noise).
A test in the suite (`sections: --units=module leaves out what the program
does not use`) checks that an unused function goes, `--no-gc-sections`
keeps it, the program runs, and no FDE describes left-out code.

### What remains

- The objects of `--units=module` are ~33% larger (section headers and
  `.rela` sections: ~140k sections for the compiler). The plan's compact
  atom table inside `elf.Obj` would avoid that; not needed so far.
- Each module's site-string blob (`.site_strs`, in `codegen.ax`) is one
  datum, kept whole if any of its records is: about a third of a small
  program's `.rodata`. Splitting it per function was tried; it saved 3-5%
  on small programs and cost as much on large ones, so it is not done.
- DWARF DIEs and line sequences of dropped code stay (tombstoned), as with
  the system linkers; item 6's emitters could leave them out.
- Mach-O `__compact_unwind` and Windows `.pdata` already go with their code
  (link-order sections); the PDB lists only kept functions (`plink.ax`
  `pdb_input`: kept code sections only).

### Original analysis (before this item)


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

**Status: done, but for the worklist rewrite of `fold_combine` and the
removal of `Value.fwd` (see "What remains").**

### What was there

`ir.Value` held its operands (`a`, `b`, `c`, `xs`) and only a count of
its uses and a user (`uses`, `user`), set by whole-function walks. Replacing
a value set `fwd` (`ir.forward`), and `ir.apply_forwarding` rewrote every
operand of the function through `ir.resolve` later (skipped when nothing had
been forwarded since: `fwd_epoch`). `ir.count_uses` was called by
`combine.ax` (`uses_of`, recounting once per `fold_combine` round, with
`uses_stale`; `sink_phi_ops` recounting after each rewrite), `slp.ax` (six
recounts), `vectorize.ax` and `idioms.ax`; `lower.ax` had a walk of its own.
`isel.count_uses` and `wasm.count_uses` count the same references as
`ir.count_uses` did (not distinct users), and set the backends' scratch
fields in the same walk.

### What was done

1. **Lists of uses** (`ir.Use`, `ir.XUse`): each operand has a place on
   the list of the instruction it reads: `ua`, `ub`, `uc` inside the value
   (16 bytes each: the next place, and the address of what points here with
   the operand's slot in its low two bits, from which the user follows) and
   an array `xu` for `xs` (each place also names its user). Lists exist for
   instructions only (constants, globals and parameters are shared or have
   no use for them); `nuses` is the count. An instruction is *registered*
   (on its operands' lists) while it is in a block that is in its function:
   `insert_before`/`append_inst` register it, `remove_inst`/`remove_block`
   take it off (and `move_block_after` puts a removed block's instructions
   back), so the counts are what a walk of the function would count.
   Operands of a registered instruction change only through `ir.set_a`,
   `set_b`, `set_c`, `set_x`, `push_x`, `trunc_x`, `clear_x`, `set_xs` and
   `set_operand`; every pass and the C front end use them (an instruction out
   of a block may be written directly, and is read through `resolve()` when
   it goes in).
2. **`O(uses)` replacement**: `ir.replace_all(old, new)` rewrites each use
   and splices the list onto `new`'s; `ir.forward(v, r)` of an instruction
   does it at once (and sets `fwd` for the passes that kept `v` in their own
   tables); `ir.replace_uses` uses it too. `ir.single_user` and
   `ir.one_user` read the list. `apply_forwarding` is left only for values
   without lists (the inliner's constant parameters, `merge_same_named`'s
   globals and functions); the passes' 40 calls of it are gone.
3. **Counts**: `ir.count_uses`, `combine.uses_stale` and the recounts in
   `combine.ax`, `slp.ax`, `vectorize.ax` and `idioms.ax` are gone (they read
   `nuses`); `lower.ax` takes a copy of the counts as they were before it
   began (its in-place construction of aggregates needs the count before its
   own removals). `Value.uses`/`user` are now only the backends' (set by
   `isel.count_uses` and `wasm.count_uses`, which also initialize their
   scratch: they were left as they are, because item 5a moves that state).
4. **Checks**: with `AXIOM_CHECK_IR=1`, `ir.check_uses` checks after every
   cleanup sub-pass, at the end of `optimize_func` and before the backends
   that each registered instruction's operands are on their lists, that every
   place on a list is an operand of a registered instruction of the function
   that reads that value, the counts, and that no operand reads a replaced
   value.

The output changed where passes read operands they had not resolved after
replacing values in the same walk: `thread.merge_phis` merges phis whose
inputs were merged before, and `fold_combine`'s single-use rewrites see the
uses already moved (and counts kept current within a round). `divconst.ax`,
whose rewrites read the divisions before theirs as they were, makes its
replacements at the end of its walk, so its output is unchanged.

Compile time (the compiler building itself at -O2, one process, user+sys)
went down about 3.5% (29.8 s to 28.7 s); the peak memory went up by 130 MB
(2502 to 2631 MB, 5%: a value is 72 bytes larger).

### What remains

- **Worklist `fold_combine`**: it still sweeps the function in rounds; the
  lists make a worklist of the users of each rewritten value possible (it
  changes the order of rewrites, so the output).
- **`Value.fwd` and the passes' `resolve()` calls**: operands no longer
  need `resolve()` (the check above holds), but about 400 calls remain, many
  on values the passes keep in their own tables (gvn's, thread's, unroll's,
  slp's), for which `fwd` is the replacement. Each table could be kept by the
  lists instead (and `thread.outside_uses`, `gvn.sink_uses` built from them),
  then `fwd` removed.
- **The backends' counts**: `isel.count_uses`/`wasm.count_uses` could read
  `nuses` and `single_user` once the backends do not change the IR after
  counting (and `Value.user` and `uses` would go, 16 bytes).

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

### Status

**5a (state structs): done.** The backends' and the lowering's mutable
module-level state is now in a few structs, one global each, so item 1 can
own them per unit (or make each one `threadlocal`) instead of finding and
resetting dozens of globals:

- `x64ctx.ax` (new, a leaf): `Obj` (`xc.O`: the object, its sections, DWARF,
  frame tables, unwind tables, Windows/macOS flags, symbol lists, the constant
  pool) and `Func` (`xc.FN`: `ir.Func`, `MFunc`, AVX/upper-half state,
  outgoing area, shrink-wrapping), shared by `x64.ax` and `x64emit.ax`. Each
  of those keeps the rest of its state in a private `State` (`S`): instruction
  selection's block caches, the emitter's encoder, frame, branch relaxation,
  call-frame and jump-table state.
- `a64.ax`: `Obj` (`O`) and `Func` (`FN`), private (nothing outside reads them).
- `ir.ax`: the interned types are one `TypeTab` (`TT`). `CINTS` was already one
  struct.
- `codegen.ax`: the per-unit tables (instances to generate, the runtime's trap,
  call-site, shadow-stack and thread-local globals, string and global values,
  ...) are one `State` (`S`).

The resets stay where they were (e.g. `x64.compile` sets each `xc.O` field);
replacing them with one assignment of a fresh value is item 1's, which has to
decide what outlives a unit and where each list's storage comes from
(functions are compiled in scratch scopes). Every step was
checked to leave the compiler's output byte-identical (objects for nine target
and option sets, and the compiler itself on x86-64 and AArch64), with no change
in self-build time or memory.

Left at module level, deliberately:

- read-only tables, and process-wide settings read from the environment once
  (`x64emit.ax`'s placement knobs, `AXIOM_DEBUG_RA`, `ir.ptr_bytes`), plus
  `x64enc.ax`'s lazily built SSE table: for threads, initialize these before
  starting workers;
- `codegen.ax`'s unit handles (`O`, `M`, `B`, the `T_*` IR types, `F`) and the
  driver's settings and results (`g_*`, read and set by `main.ax`/`units.ax`),
  left as they are to keep this step's edits there small; item 1 moves them
  into its `CodegenCtx` along with `S`;
- `ir.ax`'s value and block free lists, graveyard, `fwd_epoch` and the
  debug-info side tables (`dbg_*`), and `wasm.ax`'s state, which item 3
  (def-use lists) is reworking: group them once it lands.

Other modules with process-lifetime state that item 1 had to account for:
`nlink.ax`, `asm/obj.ax`, `codeview.ax`, `dloc.ax`, `mach.ax` and `target.ax`.
(Item 1 made all of this thread-local, but the linkers' (`nlink.ax`), which
run on the main thread: see section 1, "Status".)

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
