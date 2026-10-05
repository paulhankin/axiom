# 0099: Self-hosted optimizer: atomics on globals check their alignment at run time

- Type: feature (performance)
- Status: fixed (0753e8d)
- Area: compiler (selfhost: combine)

An atomic access traps when misaligned (spec §12.8), so codegen tests
`ptrtoint(p) & (size - 1) == 0` first. For a global the address is aligned by the linker
(`Global.align`), but nothing folded the test: `lea r, [rip + G]; test r, 3; jne trap`
before every atomic on a global, e.g. the benchmark's `bo.default_db()` (`DEFAULT_ONCE.done()`),
which every 0-ply sanity check runs, and std/mem's allocator.

Fix (combine, 0753e8d): `ptrtoint(@g) & k` is 0 when `k` has no bit at or above `g`'s alignment
(not for WebAssembly globals, which have no address). Covered by
`tests/run/float_selects.ax` (an atomic global) and `tests/run/atomics.ax`.
