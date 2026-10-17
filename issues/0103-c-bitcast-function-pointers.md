# 0103: C compiler: `@bit_cast` between a function pointer and an integer is invalid LLVM IR

- Type: bug
- Status: fixed (b3bf084)
- Area: compiler (C: codegen)

```axiom
fn add1(x: i32) i32 { return x + 1; }
export fn main() i32 {
    let f: fn(i32) i32 = add1;
    let a = @bit_cast(usize, f);
    let g = @bit_cast(fn(i32) i32, a);
    return g(41) - 42;
}
```

`cmd/axiom/bootstrap/build/axiom build t.ax --target=native -O2` stops with
`internal compiler error: invalid LLVM IR generated: Invalid bitcast %1 = bitcast ptr %0 to
i64` (and `i64 to ptr`). On native targets a function pointer is a `ptr`, so the conversion
to or from an integer must be `ptrtoint`/`inttoptr`, not `bitcast`. The self-hosted
compiler builds and runs it (`runtime/linux.ax` uses it to call the vDSO's clock_gettime).

## Fix

`@bit_cast` picks `ptrtoint`/`inttoptr` by the LLVM types of the operand and the result
rather than by Axiom pointer types; `tests/run/bitcast_fn_ptr.ax`.
