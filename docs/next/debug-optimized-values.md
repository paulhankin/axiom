## Compiler

- Optimized code (`-O1` and up, `-g`) shows more of its variables in a debugger, never a
  wrong value: a value no register or slot holds any more is computed from the values it
  was made from where they are held (DWARF expressions: additions, subtractions, bitwise
  operations, shifts, extensions), a parameter is its entry value (`DW_OP_entry_value`)
  once nothing holds it, a spill slot an instruction only reads keeps its value, every
  frame slot the code spills to is followed, and code that jump threading copies keeps
  its variables' assignments. Stepping randomly generated C functions at `-O2`, a
  debugger shows a value at 87.6% of the instructions where a variable has one (x86-64,
  from 58.1%) and 83.5% (AArch64, from 75.9%).
- Every call is a call site in the debug information (`DW_TAG_call_site`: where it
  returns, or where it is in tail position, its callee, the arguments whose values the
  caller keeps), what debuggers need to find parameters' entry values and to show
  frames of calls in tail position.
- Vectors, 16-byte `long double` values and 128-bit values have locations: in their
  vector register, or in pieces (two general registers; a vector wider than the target's
  registers, computed in 128-bit pieces).
- C's macros are in the debug information (`.debug_macro`), with the lines that define
  and undefine them and the files as they are included, so debuggers expand them;
  `--debug-macros=off` leaves them out.
- WebAssembly: optimized code's variables have locations in the function's locals
  (`DW_OP_WASM_location`), and a trap's stack trace shows the C functions that were
  running, also at `-O2` (C's calls now write their call-site records on the shadow
  stack, as Axiom's do; `--stack-traces=off` leaves them out).
