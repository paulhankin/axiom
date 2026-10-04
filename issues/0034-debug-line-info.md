# 0034: Line tables for native profilers

- Type: feature
- Status: open
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
