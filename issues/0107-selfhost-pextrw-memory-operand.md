# 0107: Self-hosted x86-64: a constant vector folded into `pextrw` as a memory operand (SIGILL)

- Type: bug
- Status: fixed (see below)
- Area: compiler (selfhost: x64)

`rm_mem_ok` (x64.ax) lets the r/m operand of an SSE instruction be a constant from the
pool. `pextrw r32, xmm, imm8` (`0f c5`, VEX `vpextrw`) has no memory form there: with a
memory operand the bytes decode as an invalid instruction. A lane extracted from a constant
vector of 16-bit lanes (tests/run/overflow_wrap.ax, once 0109 folded `u + v` of two
constant vectors) crashed with SIGILL in the native (AVX-512) build.

Fix: `pextrw` is excluded like `pmovmskb`; and an extract from a constant vector is now
folded to its lane (opt.ax `fold`), so it rarely gets that far.
