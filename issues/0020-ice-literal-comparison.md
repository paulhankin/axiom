# 0020: Internal compiler error comparing two integer literals

- Type: bug
- Status: fixed (04f5fc3)
- Area: compiler (C)

## What happens

A comparison of two untyped integer literals type-checks, then crashes code
generation:

```axiom
export fn main() i32 {
    let mut x: usize = 0;
    if (x == 1) or (0 == 0) {
        x = 2;
    }
    return @truncate(i32, @bit_cast(isize, x));
}
```

```
$ axiom build lit.ax --target=native -O2 -o lit
axiom: internal compiler error: no LLVM type for integer literal
```

It turned up in generated code (the benchmark's AVX-512 kernels, where a generator
wrote `(p == n - 1)` with both sides constant). `axiom check` accepts it.

## Possible fixes

Give the comparison of two literals a type (fold it to a `bool` constant in
sema, as `true`/`false`), or report a diagnostic in sema; either way not an
ICE.

## Resolution

Sema folds a comparison of two untyped constants to a `bool` constant (both compilers); tests/run/control.ax has the example.
