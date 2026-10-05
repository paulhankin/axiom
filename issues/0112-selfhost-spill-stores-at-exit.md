# 0112: Self-hosted register allocator: a region that only defines a spilled value stores it at its exit

- Type: feature (performance)
- Status: fixed (1fb5609)
- Area: compiler (selfhost: mach)

`split` (mach.ax) stores a spilled value where a region leaves it when that is no more
frequent than after each definition. In the benchmark's `hitting_shots_not_on_bar` the twelve
shifted masks (`blocked_opp_own >> k`, 0086 item 2) are defined in a row and then only
read in later blocks: each region in the defining block is a definition and nothing else,
and its exit (into the rarer path that goes on) is cheaper than the definition by
frequency. All twelve were therefore held in registers to the end of the block and stored
there, and those twelve short intervals of high weight evicted the values used all over
the function (`e` and `bo`, the hitters and blots), which were then split at every block
and reloaded at nearly every use (67 and 57 reloads of their slots in a standalone copy of
the function).

Fix: a region that does not read the value stores it after the definition, whatever the
exit costs. In the standalone copy the frame accesses go from 595 to 474 (LLVM 341) and it
runs about as fast as LLVM's.

What is left (0086): long-lived values used everywhere still lose to short intervals in
`calculate_half_inputs_with_masks` (a reload piece at a join of the bar-one loop evicts
`e`, which is then reloaded 68 times; 545 frame loads against LLVM's 285): evicting a
value only around the interfering stretch (region splitting) would be needed.
