# 0076: Self-hosted outliner: hot loop bodies outlined as cold functions

- Type: bug (performance)
- Status: open
- Area: compiler (selfhost: outline)

`outline.ax` takes a conditional region that makes calls to be a function's uncommon path,
and moves it into a `.cold` function. A branch inside a loop is taken each time around,
though, and nothing says either side is the uncommon one. the benchmark's
`rsort.insertion_sort` (`src/eval/rsort.ax`) has its whole shifting loop under
`if less(&v[i], &v[i - 1])` (an indirect call before constant propagation): it became
`insertion_sort.[IdxScore].cold1`, run for most elements, with the element being inserted
kept in the caller's frame and passed by address. The same happened to `heapsort`,
`encode_pruning_inputs`, `push_swapped_moves` and `fill_race_inputs`. Sorting the candidate
moves of a 2-ply search took about 35% longer than LLVM's code.
