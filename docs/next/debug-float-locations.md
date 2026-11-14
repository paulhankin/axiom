## Compiler

- Debug information at `-O1` and above on x86-64 Linux: a floating-point variable
  whose value nothing holds any more is computed from the registers, spill slots and
  constants it is made from, with DWARF 5's typed operations (arithmetic in its
  format, conversions from integers and between the formats, bit casts); the system's
  debugger there shows it. Not on AArch64, whose system debugger evaluates none of
  these operations.
- Optimized variables have locations in more places: a value reached first through a
  long chain of the values computed from it is followed again where it is reached
  more directly (a merge's inputs, a base plus a constant), as it would have been
  from there. Of the instructions stepped in randomly generated programs, the
  debugger now shows a value at 92.9% on x86-64 for C (89.0% before).
