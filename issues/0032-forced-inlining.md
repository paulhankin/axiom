# 0032: Forced inlining

- Type: feature
- Status: fixed (7f31aba)
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

## Resolution

`inline fn` (spec §9.5; `inline` is a keyword only before `fn`, also for methods and generic functions). Recursion through inline functions is `E0314`, reported at each call on the cycle (recursion through a function that is not inline is fine); `inline` with `export`, `@features` or `@multiversion` is `E0313`. Both compilers parse and check it, and `axiom fmt` lays out inline methods.

C compiler: the function gets `alwaysinline` and is inlined at every level (`-O0` runs `always-inline` too); with separate compilation every unit that calls it defines its own internal copy. A direct call records no shadow-stack site. Instead the call site's record is passed as a trailing argument, together with those of the inline calls that led to the caller (four in all, innermost first; a fifth level pushes the outermost around the call). The body pushes them only before a trap or before a call to a function that is not inline. Once inlined the arguments are constants, so the pushes fold away on every other path, and traces keep every frame (`tests/traces/traces.ax` covers six levels of nesting, an inline function calling an ordinary one, and one used as a value). Used as a value, an inline function is called through a thunk with the ordinary signature. Profile builds record inline calls like any other, so `axiom profile` still attributes time to them. A fault that is not a check (a raw-pointer access out of bounds, say) inside an inlined body is reported as if it were in the caller. On wasm the stack check of an inline function's own frame counts only the slots that do not become registers.

Measured on the benchmark's `bench_kernels` (wasm, `-O2`, node): marking `int_of`, `prep_direct`, `indirect_double`, `double_shots` and `nondouble_shots` `inline` takes contact_inputs from about 690 to 600 ns per position, with the same checksum. Natively LLVM was already inlining them, and the time does not change.

Self-hosted compiler: its inliner always inlines an `inline` function where it can (`-O1` and above). It keeps ordinary call-site records.
