# 0006: The self-hosted compiler does not build for wasm32

- Type: bug
- Status: open
- Area: compiler (selfhost)

```
$ selfhost/build/axiom check selfhost/src/main.ax --target=wasm32
selfhost/src/gvn.ax:2701:17: error[E0512]: @bit_cast size mismatch: i64 is 8 bytes, usize is 4 bytes
    uses.at(@bit_cast(usize, o.aux)).push(x);
selfhost/src/gvn.ax:2751:43: error[E0512]: ...
selfhost/src/wlink.ax:213:24: error[E0512]: ...
```

The compiler assumes 64-bit `usize` in a few places (`@bit_cast` between
`i64` and `usize`). `axiom check` defaults to wasm32, so checking the
compiler's own sources needs `--target=native`. Fixing the three casts
(`@int_cast` / `@truncate` instead of `@bit_cast`) would let the compiler
build for wasm32 (and wasm64), i.e. run in a browser; whether everything
else it needs (files, processes for the linker and module workers) is
available there is a separate question.
