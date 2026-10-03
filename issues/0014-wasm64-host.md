# 0014: wasm64 programs do not run under runtime/host.mjs

- Type: bug
- Status: open
- Area: runtime: `runtime/host.mjs` (and the workers)

Programs build for `--target=wasm64` (memory64), and the compiler itself
does since 0006, but none of them run under the node host:

```
$ axiom build hello.ax --target=wasm64 -o h.wasm && node runtime/host.mjs h.wasm
TypeError: Cannot convert -1 to a BigInt
    at std/io.fd_write ...
```

and a program that uses threads fails to instantiate with "cannot import
memory32 as memory64".

With memory64, `usize`/`isize` host-function parameters and results are
i64, which JavaScript sees as BigInt: the host functions take and return
Numbers. The shared memory a threaded program imports must be created with
`address: 'i64'` (`index: 'i64'` in older V8). The thread, test, network
and profiler workers create memories and read pointers the same way.

Fix: have the host detect a module's memory type (`WebAssembly.Module.imports`
or the exported memory) and wrap each import to convert i64 arguments to
Numbers and results back to BigInt where the ABI's types are 64-bit (a
table of which parameters are pointers/sizes, from the host ABI). Then run
the test suite with `--target=wasm64` in CI. (Which node versions support
memory64 without a flag needs checking; node 22 here instantiates wasm64
modules, as the failures above happen inside them.)
