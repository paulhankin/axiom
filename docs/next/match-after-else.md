## Compiler

- A `match` arm written after `else` (warning `W0631`) is never taken, as spec §8.6
  says. Before, the arms became the cases of a switch whatever their order, so such an
  arm was taken for its value.
