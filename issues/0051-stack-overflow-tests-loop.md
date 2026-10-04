# 0051: Stack-overflow tests turn into infinite loops at -O2 with wrapping arithmetic

- Type: bug (tests)
- Status: open
- Area: tests

Since issue 0047 (overflow wraps), LLVM may turn the tests' runaway recursion
(`forever(n + 1) + 1` and the traces recursion) into loops at `--native -O2`, so nothing
overflows the stack: `tests/traces/traces.ax` "recursion" and `tests/testing/stack.ax` "main
thread overflow" fail, and `tests/traces/crash.ax` loops forever and hangs the test runner
(found by the issue 0040 work, also on main without it).

Fix: make the recursion impossible to turn into a loop (e.g. keep a live local across the
call, pass the address of a local to an opaque function, or recurse through a function
pointer), and give the runner a per-test timeout so a hang fails instead of stalling.
