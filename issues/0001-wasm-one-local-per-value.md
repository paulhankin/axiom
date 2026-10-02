# 0001: Every value gets its own wasm local; deep recursion exhausts the stack at -O2

- Type: bug
- Status: fixed (commit "wasm: shared locals; no big frames inlined into recursion")
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

## Resolution

The diagnosis above was only part of it. With locals shared, `Renderer.run`
went from 191 to 49 locals and the test still failed: the stack that ran out
was the linear-memory one. At -O2 `Renderer.run` had a 37 KB frame (at -O1,
128 bytes): the inliner had copied in callees with `io.BufWriter`s (4 KB
each) and other buffers, and 100 levels of recursion need 3.7 MB of a 1 MB
stack. Three changes:

- **Inlining** (`selfhost/src/inline.ax`): a callee whose stack slots take
  more than 1 KB is not inlined into a recursive function (one in a call
  graph cycle), where every level would pay for them; LLVM has the same
  limit. `Renderer.run`'s frame is now 64 bytes.
- **Locals** (`selfhost/src/wasm.ax`, `alloc_locals`): values share wasm
  locals when their lifetimes do not overlap: liveness over the blocks (reads
  of inlined and recomputed values counted where they are pushed, phis
  defined at the start of their block), then a first-fit colouring per value
  type in dominator tree order, which is optimal for SSA. About 4x fewer
  locals in large functions, so smaller V8 frames; wasm output 1-2% smaller
  (`examples/`, -O0 and -O2).
- **Phi folding** (`selfhost/src/opt.ax`): `phi(x, undef)` became `x` even
  where `x` is not defined on every path into the phi (a store sunk into a
  merge block from one arm), leaving a use that its definition does not
  dominate. The old allocator hid it (every value had its own zeroed
  local); the new one reports such IR as an internal error instead of
  reusing a local. The fold now needs `x` to dominate the phi.
