# 0022: `axiom profile` (C compiler) does not pass arguments to the program

- Type: bug
- Status: fixed (04f5fc3)
- Area: tools, compiler (C)

## What happens

Spec §16.6 documents `axiom profile <file.ax> [-- args]`, but the C
compiler rejects both forms:

```
$ axiom profile bench.ax --target=native -O2 -- contact_net
axiom: error[E9001]: unknown option '--'
$ axiom profile bench.ax --target=native -O2 contact_net
axiom: error[E9001]: unexpected argument 'contact_net'
```

so a benchmark that selects its kernels by argument has to read an
environment variable instead when profiled.

## Fix

Accept `--` and pass what follows to the program, natively and on wasm.

## Resolution

The C compiler takes `--` and passes the rest to the program, natively and on wasm (after the `--` that runtime/host.mjs expects, which the self-hosted compiler did not pass either). tests/profile/hot.ax now does its work only when given two arguments.
