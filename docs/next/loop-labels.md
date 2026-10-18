## Language

- Loop labels: `rows: for r in 0..n { … }` can be left or restarted from a nested loop
  with `break rows;` or `continue rows;` (also after `catch`), running the `defer`s of
  the loops left on the way (spec §8.5).
