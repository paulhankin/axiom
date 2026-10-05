# 0084: Self-hosted optimizer: loads of a buffer filled by memcpy, and vector and-not without AVX-512

- Type: feature (performance)
- Status: fixed (dbca74e)
- Area: compiler (selfhost: gvn, x64)

the benchmark's half-inputs code zeroes a padded copy of the 25-point board on the
stack, copies the board into it (`@memcpy` of 100 bytes) and reads it back
as 16-byte vectors at every offset. The self-hosted compiler did all of that
through memory: the vector loads straddle the copy's stores, which the
processor cannot forward to them, so each one waited for the stores to
retire. LLVM reads the board itself (and the last, partly padded vector as
the board's last 16 bytes shifted with zeros).

Also, without AVX-512 (where and-not is a blend or ternary logic), a vector
`~y & x` was a `pxor` with an all-ones constant, a copy and a `pand`, where
SSE2 has `pandn`.

Fix:
- gvn remembers a memcpy or memmove of a constant size (`K_COPY`). A later
  load inside its destination, with nothing written to the bytes it reads or
  to the copy's source bytes since, loads from the source instead. A vector
  load that runs past the copied bytes into bytes a `memset` of 0 zeroed
  (and nothing wrote since) loads the source's last bytes and shuffles them
  down with zeros. The source bytes read must not overlap the destination
  (memmove).
- x64 selects a shuffle that moves one 16-byte source's lanes down or up by
  whole bytes, zeros shifted in (the other source zero), as `psrldq` /
  `pslldq`.
- prepare marks the `not` of a vector `and` of up to 256 bits (not AVX-512
  sizes) used only there `F_ANDN`, and the and is `pandn` (`vpandn`).

Test: `tests/run/padded_loads.ax` (copies with zeroed tails, overlapping
memmoves, writes between the copy and the load, at `--cpu=x86-64`).

Effect (the benchmark, `--cpu=x86-64 -O2`, bench_kernels min of 9):
`contact_inputs` 505 -> 496 ns (LLVM 443, Rust 561); `sigmoid_x4` loses a
`pxor` from memory and a copy per call.
