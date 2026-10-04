# 0022: `axiom profile` (C compiler) does not pass arguments to the program

- Type: bug
- Status: open
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
