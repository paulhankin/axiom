# 0002: The test suite never builds `std` at -O2

- Type: bug (test coverage)
- Status: open
- Area: tests (`tests/run.mjs`)

## What happens

`node tests/run.mjs` runs each std module's tests at the default
optimization level only. Building every std module's tests at -O2 (native
and wasm32) turned up three compiler bugs that the suite had never seen:

- an exponential blow-up in the wasm backend (`std/unicode.ax` ran out of
  memory past 12 GB) — fixed in fafd682;
- an integer overflow in GVN's bounds analysis that crashed the compiler on
  `std/graphics/svg.ax` and `examples/texmath.ax` — fixed in fafd682;
- [0001](0001-wasm-one-local-per-value.md), still open.

## Fix

Add an -O2 pass over std to the suite: build and run each module's tests
with `-O2` on native and wasm32 (as the sweep did:
`axiom test --target=$t -O2 <module>`). The sweep took about 20 minutes
with 4 jobs, so it may belong in a `--slow` / `--opt` mode of `run.mjs`, or
a separate script, rather than every run. `AXIOM_CHECK_CINTS=1` (shared
constants left unchanged) and `AXIOM_POISON=1` (released memory never
reused) are worth running in the same pass.
