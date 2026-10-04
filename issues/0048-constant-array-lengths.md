# 0048: Array lengths cannot be constant expressions

- Type: feature
- Status: open
- Area: language, compiler (C and selfhost)

`[MAX_PREFIX_DEPTH + 1]u64` is a parse error: an array length must be a literal or a single
constant name, so ports write `[3]u64` with a comment saying where 3 comes from (found
porting the benchmark's rollouts, the benchmark `axiom/src/eval/rollout.ax`). Rust, C and Zig take any
constant expression. Proposal: accept a constant expression of an unsigned type in array
lengths (the constant evaluator exists already: E0530 even mentions array lengths).
