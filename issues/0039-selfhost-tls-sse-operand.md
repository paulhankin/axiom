# 0039: Self-hosted x86-64 backend: "TLS operand in an SSE instruction"

- Type: bug
- Status: open
- Area: compiler (selfhost, x86-64 backend)

## Today

Building the benchmark's Axiom port natively with the self-hosted compiler fails:

```
$ selfhost/build/axiom build src/bin/bench_kernels.ax --target=native -O2 -o bk
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
