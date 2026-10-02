# 0001: Every value gets its own wasm local; deep recursion exhausts the stack at -O2

- Type: bug
- Status: open
- Area: compiler (selfhost), wasm backend (`selfhost/src/wasm.ax`)

## What happens

`std/text/template.ax`'s "examples" test fails on wasm at -O2 (it passes at
-O1, and natively at every level):

```
$ selfhost/build/axiom test --target=wasm32 -O2 std/text/template.ax
FAIL examples
     std/text/template.ax:310:8: T0008 trap T0008 (stack exhausted): run
       at Renderer.run (std/text/template.ax:310:8)
       at Renderer.partial (std/text/template.ax:423:17)
       ... (alternating, about 200 frames)
```

The test checks that a partial including itself stops at `MAX_DEPTH` (100);
the stack runs out first.

## Cause

`gen_function` gives every instruction that needs a local (`FL_LOCAL`, with
uses) a wasm local of its own (`new_local`, around line 2836); nothing reuses
a local whose value is dead. After -O2 inlining, `Renderer.run` and
`Renderer.partial` have very many values, so very many locals, and V8 keeps
all of a function's locals in its native stack frame. 100 levels of
`run` + `partial` then exceed V8's native stack (about 1 MB), although the
linear-memory stack (1 MB, `wlink` `stack_size`) is nowhere near full.

## Fix

Allocate wasm locals like registers: compute liveness over the IR, then
colour values per wasm value type (i32, i64, f32, f64, v128) so that values
whose lifetimes do not overlap share a local. Phis are written by
`phi_copies` at the end of each predecessor (all sources are pushed before
any local.set), so a phi's local is live from the end of each predecessor;
it may share with a source that dies there. Besides the bug, this shrinks
wasm code (fewer local declarations, smaller indices) and V8 frames for all
programs.

To check: the test above, the full suite on wasm at -O0..-O2, and byte
counts of `examples/*.ax` built for wasm before and after.
