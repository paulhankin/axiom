## Code generation

- Register allocation around calls: a value live across a call only on a
  seldom-taken path is kept in memory across that call instead of taking a
  callee-saved register every call of the function saves; a piece of a split
  value that would be reloaded after calls more often than it is read is read
  from its stack slot instead; a value leaving a hot block along a cold edge
  is stored on the cold side. An accumulator updated by a conditional move in
  a loop is built in its own register, and a value computed for a loop's next
  iteration waits until the old one's last use, so the loop copies nothing at
  its back edge. Measured on C programs at -O2: 5 to 9% fewer instructions in
  tree, sorting and regular-expression code.
