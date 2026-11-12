## Compiler

- Loops run faster at `-O1` and above (most for C code):
  - A loop counter narrower than a pointer (C's `int i`, an `i32` or `u32` range loop)
    is counted in a register as wide as a pointer where its step cannot overflow: C's
    signed arithmetic, an Axiom range loop's step, or a step checked with
    `--overflow=trap`. Indexing with it no longer sign-extends it in every iteration,
    and `t[i + j]` with `i` fixed in the loop takes one addressing mode, `t + i` computed
    before the loop. (Unsigned C counters, whose arithmetic wraps, are not widened.)
  - A global the loop increments (`steps++`) is kept in a register across the loop and
    stored back on its way out, where nothing in the loop may read or write it otherwise
    (no call, no access through a pointer that may point to it, nothing volatile or
    atomic).
  - Loads the loop does not write are moved before loops whose values are used after
    them too.
  - `-O2`: a small loop whose trip count is not known (`for (i = 0; i < n; i++)`) runs
    two iterations per test of its bound, the one left over on its own.
  - x86-64 and AArch64: the address of a global indexed in a loop (`table[i]`) is built
    once before the loop, not in every iteration.
