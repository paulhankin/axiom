# 0023: Aligned allocation in std/mem

- Type: feature
- Status: fixed (04f5fc3)
- Area: std/mem

## Today

`mem.alloc` returns 16-byte-aligned memory (`alloc_bytes`: "aligned to 16"),
and there is no way to ask for more. A 64-byte vector load (`f32x16`) from
such memory crosses a cache-line boundary three times in four, and a 32-byte
one (`f32x8`) half the time.

Rust and C code that wants full-width loads keeps its tables on 64-byte
boundaries (`#[repr(align(64))]`, `aligned_alloc`); the benchmark's `AlignedVec`
holds every net's weights that way for its AVX-512 kernels.

## Measured

the benchmark's AVX-512 kernels (`axiom/src/eval/simd_avx512.ax`) with the weights
copied to 64-byte-aligned memory, against `mem.alloc`'s alignment (best of 12
alternating runs, ns per position; bit-identical results):

| Kernel | 16-byte aligned | 64-byte aligned |
|---|---|---|
| race net (214 × 128) | 243.6 | 220.2 |
| pruned contact (200 × 16) | 111.7 | 102.7 |
| pruned race (200 × 8) | 142.3 | 131.5 |
| pruned boards | 66.0 | 57.4 |

The only way to get there today is to over-allocate and keep both the
allocation (to free) and an aligned view, in every type that owns such data.

## Proposal

- `mem.alloc_aligned.[T](n, align) ![]mut T` and `mem.alloc_zeroed_aligned`,
  `align` a power of two up to 4096; freed with `mem.free` like any slice
  (the allocator finds the block from the pointer, as for its large
  allocations).
- Or, cheaper to specify: every allocation of 64 bytes or more is 64-byte
  aligned. Slab classes of 64 bytes and up can be laid out that way, and
  large allocations already start near a span boundary.
- `mem.dup_aligned` for copies.

## Resolution

`mem.alloc_aligned.[T](n, align)`, `alloc_zeroed_aligned`, `dup_aligned` and `alloc_aligned_bytes(size, align)` (spec §15.1): `align` a power of two up to 4096, freed with `mem.free`. Up to 128, the first size class whose objects start at multiples of `align` (spans and their headers are 128-aligned; a smaller free size than the class's is accepted); large objects are already at 128. Beyond 128, a large block holds the object at the boundary with an alias header in front of it naming the block, which `free` recognizes. Under `--sanitize=address` the header is padded so the allocation stays aligned.
