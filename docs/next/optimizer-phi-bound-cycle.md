## Compiler

- Fixed: `-O2` could run out of stack (`trap T0008`) on a checked subtraction of
  unsigned numbers whose result, or the value its `catch` gives, is compared after a
  wrapping subtraction of a constant (`t = s -% 2; if t >= a { s - a ... }`): the proof
  of the result's lower bound asked for the same bound again, without end.
