# 0116: Self-hosted optimizer: a slot that is only ever copied whole is split into its fields

- Type: feature (performance)
- Status: fixed (99ffec5)
- Area: compiler (selfhost: sroa); compiler (C) noted

Found in the benchmark's `axiom/` port: `rsort`'s insertion sort and Lomuto partition move
`IdxScore` (`{u32, f32}`) elements through a temporary (`let tmp = v[i]; ... v[j] = tmp`),
and several other hot loops copy small structs from one place to another the same way.

`sroa` split such a temporary into its fields, so each copy into or out of it became one
load and one store per field: two of each for an 8-byte `{u32, f32}` element where one
`i64` move does. Nothing reads the temporary's fields; it is only ever the source or the
destination of whole copies (and fills).

Fix (`sroa.ax`): a slot whose every use is a `memcpy`, `memmove` or `memset` of all of it
(`whole_copies_only`) is split into integers as wide as fit, 8 bytes and then 4, 2, 1
(`int_leaves`), instead of into its fields. Instructions at 1-ply (the benchmark `bench_search`,
five positions, after the inlining of the benchmark 9543112 and 93dde1a): 4,812,849 ->
4,788,981 (`partition_lt` 49,953 -> 39,324, `eval_batch_cross_roll_into_with_scratch`
199,015 -> 192,335).

The C compiler (LLVM) has the same shape of problem: a `let t = p.*` or `a[i] = a[j]` of a
struct is a load and a store of the struct value, which LLVM splits per field. Copying such
places with a `memmove` of the bytes (as clang copies C structs) gives one `i64` move; it
was tried (`copy_place` in `codegen.c`) and not kept, as it did not change the measured
times of the LLVM-built port.
