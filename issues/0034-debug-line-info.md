# 0034: Line tables for native profilers

- Type: feature
- Status: fixed (7f31aba)
- Area: compiler (C), tools

## Today

Native executables carry no DWARF (`readelf -S` shows no `.debug_*`
sections), so `perf annotate`, `perf report --sort srcline` and debuggers
cannot map instructions back to Axiom source. After inlining, a hot
function is one symbol of a thousand instructions (the benchmark's
`calculate_half_inputs_with_masks` is 1,580 lines of disassembly holding 20
inlined Axiom functions), and only `axiom profile`'s shadow-stack sampler
splits it, at function granularity and with the instrumentation's own cost.
Rust builds with `debug = line-tables-only` for this.

## Proposal

`-g` (or `--debug-info=lines|full|none`) emitting DWARF line tables through
LLVM's DIBuilder: a compile unit per module, a `DISubprogram` per function and
a `DILocation` per instruction from the spans the compiler already has (it
records them for stack traces). Inlined code then shows as `DW_TAG_inlined_
subroutine`, which `perf`, `addr2line -i` and `gdb` understand. Default off
for `--release`, on otherwise, or off everywhere; the size cost is small.

## Resolution

`-g` / `--debug-info=lines` (and `--debug-info=none`, the default at every level, including `--release`) in the C compiler, for `build`, `test` and `profile`. The output then carries DWARF 4 line tables made through LLVM's DIBuilder: a line-tables-only compile unit per object, a `DISubprogram` per function (named as in stack traces, file and line of its declaration) and a `DILocation` for each instruction, from the expression and statement spans that trap sites already use. Inlined code appears as `DW_TAG_inlined_subroutine`. Default off: the shadow stack needs no debug information, and outputs stay small and do not depend on the directory they were built in. `full` (variables and types) is not implemented.

Checked on a native `-O2 -g` build: `readelf -S` shows `.debug_info`, `.debug_line` and the rest. `llvm-addr2line-18 -i -f` on an instruction of an inlined `inline fn` gives the whole chain (`get` at line 6, in `pair` at 10, in `sum` at 17, in `run` at 27). `perf report --sort srcline` and `perf annotate` (`/usr/lib/linux-tools/6.8.0-142-generic/perf`) split the time by Axiom line and interleave the source. The whole suite passes with `-g` (`node tests/run.mjs -g`, which `run.mjs` now accepts; also `--native -g -O2`), so the verifier accepts the debug info everywhere. wasm builds carry the same `.debug_*` sections (wasm-ld keeps them). The self-hosted compiler accepts the option and emits nothing for it.
