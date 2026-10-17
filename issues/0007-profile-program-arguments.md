# 0007: `axiom profile` cannot pass arguments to the program

- Type: feature
- Status: fixed (commit "Faster optimizer, part 1")
- Area: compiler (selfhost), `axiom profile`

`axiom profile <file.ax> [--rate=<hz>] [--folded=<path>]` builds and runs a
program under the sampling profiler, but has no way to give the program
arguments (or stdin), so a program like the compiler itself
(`axiom build ...`) cannot be profiled with it. Linux `perf` was not
available either, so investigating the compiler's own speed needed ad-hoc
timers.

Suggested: everything after `--` goes to the program
(`axiom profile cmd/axiom/src/main.ax -- build x.ax -O2`), and stdin is
passed through.

Fixed: arguments after `--` go to the program; stdin is inherited.
