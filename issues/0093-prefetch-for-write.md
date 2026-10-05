# 0093: `@prefetch` cannot ask for a line to be written

- Type: feature
- Status: open
- Area: language (§5.7), compiler (C and selfhost)

`@prefetch(p)` (issue 0041) is a hint that the program will soon read the line at `p`
(x86 `prefetcht0`, Arm `prfm pldl1keep`). the benchmark's evaluation cache prefetches each bucket of
a batch before it looks the bucket up, then locks it and may update it. Rust's engine asks for
those lines in a state ready for writing:

- on x86-64, `_mm_prefetch::<_MM_HINT_ET0>`, which is `prefetchw` where the CPU has PRFCHW;
- on AArch64, `prfm pstl1keep`.

(crates/the benchmark-eval/src/cache.rs `prefetch_for_write`.) Without this, a line fetched for
reading is shared, and the write that follows needs a second coherence transaction for
ownership. The port (the benchmark axiom/src/eval/cache.ax, `prefetch_board`) can only
prefetch for reading.

Proposal: an optional second argument, `@prefetch(p, .write)` (default `.read`), or a separate
builtin `@prefetch_write(p)`, whichever fits the builtin conventions better. Lowering:

- x86-64: `prefetchw` when the function's extensions include `prfchw` (add it to std/os/cpu's
  list if it is missing; Broadwell and later, and every AMD64 CPU, have it), else `prefetcht0`;
- AArch64: `prfm pstl1keep`;
- WebAssembly: nothing.

As for `@prefetch`, the hint never traps and has no other effect.

## Resolution

`@prefetch_write(p)` (spec §5.7, Appendix A, C45): the same hint as `@prefetch` for memory the
program will soon write, with the same rules (any `*T` or `*mut T`, `p` evaluated once, nothing
read or written, never traps, `void`; `E0822` for a non-pointer).

**Why a separate builtin and not `@prefetch(p, .write)`.** Every Axiom builtin has a fixed
arity (the signature strings of `builtins.h` / `ast.ax`), none takes an optional argument, and
none takes an enum-literal selector: variants that are different instructions are different
names (`@atomic_add`/`@atomic_sub`, `@widen_low`/`@widen_high`, `@x86_aesenc`/`@x86_aesenclast`,
`@load`/`@load_partial`). A `.read`/`.write` operand would also need a rule that it be a
constant, and an error code for when it is not, for no gain in expressiveness. `@prefetch(p)`
is unchanged.

**x86-64 extension `prfchw`** (std/os/cpu `X_PRFCHW`, bit 39 in both compilers' tables; CPUID
0x80000001 ECX bit 8, so `--cpu=native` has it on every AMD64 processor and Intel from
Broadwell). The `skylake` preset gained it (so `icelake-server`, `sapphirerapids`, `alderlake`,
`arrowlake` and the `znver*` presets, built on it, have it too); `haswell` and the
`x86-64-v*` levels do not, as in LLVM. Listed in §13.4 and selfhost/README.md.

**C compiler**: `llvm.prefetch(p, 1, 3, 1)` (write, locality 3, data); with `+prfchw` in the
function's target features LLVM selects `prefetchw`, otherwise `prefetcht0`; Arm `prfm pstl1keep`;
nothing on WebAssembly.

**Self-hosted compiler**: `@prefetch` was evaluated and dropped there; both hints are now IR
intrinsics, `ir.IN_PREFETCH` and `ir.IN_PREFETCHW` (codegen.ax, native targets only). They are
kept by dead-code elimination (`has_side_effects`) and never moved or merged, but GVN, dead store
elimination and the shadow-stack analysis treat them as touching no memory, so a prefetch in a
loop does not block forwarding or hoisting of the loads around it. x86-64: a new machine
instruction `PREFETCH` with the address folded into its memory operand (an address-only
`ptradd` stays unmaterialized, as for loads): `prefetcht0` (0F 18 /1), or `prefetchw`
(0F 0D /1) for `IN_PREFETCHW` when the function's extensions include `prfchw`. AArch64: a
`PRFM` machine instruction encoded through the loads' addressing (`prfm` with an unsigned
offset or a register index, `prfum` for small negative or unaligned offsets): `pldl1keep`,
`pstl1keep`.

Tests: tests/run/prefetch_write.ax (a batch of buckets prefetched for writing and then
updated, in plain code and in an `@features("prfchw")` function, odd addresses, evaluated once),
tests/errors/prefetch.ax (the new builtin's errors), tests/lowering/prefetch.ax (both
compilers: `prefetchw` with skylake and with `x86-64-v3 --features=+prfchw`, `prefetcht0` with
haswell, the indexed address folded, nothing on wasm32). The AArch64 encodings were checked with
llvm-objdump on the self-hosted compiler's objects.

the benchmark's port now uses it in `PositionCache.prefetch_board` (axiom/src/eval/cache.ax), as
Rust's `prefetch_for_write`: results stay bit-identical to Rust's bench_matrix. Single-threaded
bench_search times did not move beyond the machine's noise with either compiler (Cascade Lake,
min of 15/15/9 runs; prefetcht0 -> prefetchw): LLVM 1ply 28.9 -> 29.2 ms, 2ply 116.8 -> 118.0 ms,
3ply 755.7 -> 772.6 ms; self-hosted 1ply 30.7 -> 30.7 ms, 2ply 119.8 -> 118.5 ms, 3ply
779.5 -> 751.4 ms (Rust 27.7, 105.6, 670.5 ms). With one thread no other core holds the lines, so
a read prefetch already brings them in exclusive state; the write hint matters when threads
share the cache.
