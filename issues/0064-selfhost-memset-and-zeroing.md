# 0064: Self-hosted x86-64: small constant-size memset/memcpy as `rep stosb`/`rep movsb`, arrays zeroed eight bytes at a time

- Type: feature (performance)
- Status: fixed (163832e, 4710d95)
- Area: compiler (selfhost: x64 lowering of memset/memcpy, gvn memset idioms)

Part of the set 0053-0064 (the benchmark `bench_kernels`, selfhost against LLVM; see 0053).
This is low priority: these costs are small next to the issues above.

## Today

- `@mem_set` of a known medium size is `rep stosb`: `calculate_race_inputs` zeroes its
  856-byte output (`mov ecx,0x358; rep stos BYTE PTR es:[rdi],al`), and
  `DedupTable.clear`'s overflow path fills 2 KiB. `rep stosb` has a start-up cost of
  tens of cycles. LLVM calls the C library's vector `memset` (and inlines stores for
  small sizes).
- Small local arrays are zeroed with 8-byte stores: `dim128_pair`'s
  `both`/`only0`/`only1: [16]u32` take 8 `mov QWORD PTR [rbp-x],rdi` each, and `xs: [16]f32`
  takes 8 more. LLVM uses two `vmovups YMMWORD` stores per array, or one zmm store. A
  constant fill `[36; 11]u32` in `half_containment` is 11 `mov DWORD PTR [rbp-x],0x24`.
- A vector-store replacement for the 856-byte memset (14 `@store`s of `@splat(0.0)`)
  loaded the zero from memory before every store (issue 0056). It was no faster than
  `rep stosb` in the race kernel (±5% noise).
- `@mem_copy(out0[0..5], s[0..5])` (20 bytes, constant size, from a local array) at the end
  of `dim128_pair` is lowered as a `memmove`: an overlap test, then `std; rep movsb; cld`
  or `rep movsb`, twice per pair of positions. It should be one 16-byte and one 4-byte
  move (or an overlapping 16+16). The source is a stack slot whose address does not
  escape, so it cannot overlap the destination.

## Proposal

- Lower `memcpy`/`memmove` of a small constant size (up to about 128 bytes) as loads and
  then stores (no overlap test is needed when every load comes before every store).
- Lower `memset` of a constant size: up to about 256 bytes as vector stores (zmm/ymm/xmm,
  overlapping the tail) of a zeroed or broadcast register. Beyond that, a vector loop,
  or a call to the runtime's `memset` (which could itself use `rep stosb` only above
  about 2 KiB, where ERMSB pays off).
- Zero-initialise local arrays with the widest stores the target has (also from SROA and
  `copies.ax`, which produce these).

## Impact

Below 2% on any kernel (estimate: `race_inputs` 0-5%, `contact_net` about 1%), under
10 ns summed.

## Resolution

Fixed in 163832e and 4710d95 (`x64.ax`):

- A `memcpy` or `memmove` of up to 256 bytes is loads (all first, so the operands may
  overlap: no overlap test) and then stores: vectors up to 32 bytes (`ymm` with AVX2; no
  AVX-512 frequency licence for a copy), one more overlapping the end, and integer moves
  below 16 bytes. `dim128_pair`'s 20-byte `@mem_copy` is two 16-byte moves.
- A fill with a constant byte stores a zeroed (or loaded) vector the same way, and from 257
  bytes to 2 KiB as a loop of vector stores plus one overlapping the end; so does a
  `memcpy` of up to 2 KiB. Larger ones stay `rep stosb`/`rep movsb`. `calculate_race_inputs`
  zeroes its 856 bytes with a `ymm` loop (27 stores).
- Runs of constant stores filling 16 to 64 contiguous bytes from one base (nothing between
  them that touches memory) are vector stores of their bytes from the constant pool, or of
  a zeroed register: `[36; 11]u32` in `half_containment` is one `ymm` and one overlapping
  `xmm` store.

No kernel changed beyond the noise (0057's table, the last two columns).
