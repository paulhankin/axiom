# 0039: Self-hosted x86-64 backend: "TLS operand in an SSE instruction"

- Type: bug
- Status: fixed (8d1375a)
- Area: compiler (selfhost, x86-64 backend)

## Today

Building the benchmark's Axiom port natively with the self-hosted compiler fails:

```
$ cmd/axiom/build/axiom build src/bin/bench_kernels.ax --target=native -O2 -o bk
axiom: x86-64 code generation: TLS operand in an SSE instruction (in src/eval/net.collect_nonzero)
```

(in the benchmark's `axiom/` directory, at the benchmark 73229e8, axiom 5bc31bc). The function
stores vectors into a thread-local scratch buffer (`threadlocal let mut` in
`src/eval/net.ax`, reached through `nz.idx[n..]` / `nz.val[n..]` with `@store` of
`u32x4`/`f32x4`), and the instruction selector folds the thread-local address into an SSE
store's memory operand, which the encoder then rejects. The wasm build of the same program
works.

## Expected

Thread-local addresses used by vector loads and stores are materialised into a register
first (`mov %fs:0, reg` + `lea`, or the TLS sequence the backend uses for scalars) and the
SSE/AVX instruction addresses memory through it; or the encoder accepts `%fs:`-segment
operands on SSE instructions (they are legal on x86-64: an `fs` override prefix works with
`movups`/`movdqu`).

A regression test: a `threadlocal let mut buf: [16]f32` written with `@store(buf[4..], v)`
and read back with `@load`, natively at -O0 and -O2.

## Resolution

The fs segment prefix was already emitted ahead of every instruction with a
thread-local operand; only the ModRM encoding was missing. `x64enc.modrm_mem`
now encodes all thread-local forms: `[index*scale + sym@tpoff]` (SIB with no
base, as before) and, with a base register (Windows' thread block, or a
folded pointer), mod 10 with the displacement relocated as `TPOFF32`. It serves
the SSE/VEX/EVEX encoder (whose check is gone) and the integer encoder's
`enc_mem`, which had its own copy of the base form. Legacy SSE (`64 f3 0f 6f`),
VEX and EVEX encodings were checked with objdump for Linux and Windows objects.

tests/run/threadlocal_simd.ax stores and loads `f32x4`, `u32x4`, `f32x8` and
`f32x16` in thread-local arrays at constant and run-time offsets, through a
pointer to a thread-local struct (inlined at -O2, as in the benchmark), and on a second
thread; it passes natively and on wasm at -O0 and -O2 with both compilers.

the benchmark's `bench_kernels` (axiom/src/bin) now builds natively with the
self-hosted compiler at -O0 and -O2, and its twelve checksums equal the C
compiler's build natively (the AVX-512 kernels, e.g. `contact_net`
16852.3878) and on wasm (`contact_net` 16852.3868). The self-hosted native
build is correct but slow on the AVX-512 kernels (`contact_net` 3.5 us
against the C compiler's 0.34 us, slower than its own wasm build's 1.2 us): 512-bit
operations it does not select whole are split into 128-bit pieces
(`vextracti32x4`/`vpshufb`/`vinserti32x4`), and some conversions go lane by lane.
