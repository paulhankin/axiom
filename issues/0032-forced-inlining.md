# 0032: Forced inlining

- Type: feature
- Status: open
- Area: language, compiler (C and selfhost)

## Today

Axiom has no way to ask for a function to be inlined, and no macros. Whether
a call is inlined is LLVM's choice, and the stack-trace instrumentation
(three stores and a load per call) makes small functions look bigger to its
inliner. Rust code written for speed leans on `#[inline(always)]` and on
`macro_rules!` for exactly this: the benchmark's `features.rs` expands its
hitting-shot steps with macros (`prep_direct!`, `eval_indirect_double!`,
`eval_nondouble!`) so every die is a constant and every shift is by a
constant.

The Axiom port writes those macros as small functions called with constant
dice (the benchmark `axiom/src/eval/features.ax`, `double_shots`,
`nondouble_shots`, `indirect_double`). On wasm32 LLVM keeps them as calls:
`hitting_shots_not_on_bar` makes 21 calls per side, each with a stack check,
a shadow-stack frame and variable shifts, and the function takes about half
of the contact inputs' time (an `axiom profile` of the wasm build). Rust's
wasm build runs the same code at about 0.6x the port's time.

## Proposal

An `inline` modifier on function declarations, `inline fn f(...)`, meaning
"always inline": the C compiler adds LLVM's `alwaysinline` attribute (and
drops the call site's shadow-stack record, since the body becomes part of
the caller's frame; a trap inside reports the caller's call site, or better,
the inlined call's span as an extra frame if the trap machinery can carry
it). Recursion through `inline` functions is an error. The self-hosted
compiler inlines them itself or ignores the hint.
