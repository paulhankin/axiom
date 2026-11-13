## Compiler

- Two builds of a C program with `-g` are the same also when they are made at different
  times: the line tables' MD5 of the predefined macros' `<built-in>` no longer changes
  with `__DATE__` and `__TIME__`.
- An enumeration constant that a signed 64-bit integer does not hold (an unsigned
  64-bit one with its top bit set, a 128-bit one) has its exact value in the debug
  information: DWARF's `DW_FORM_udata` or `DW_FORM_data16`; CodeView's unsigned 64-bit
  numeric leaf (a 128-bit one is left out there).
- WebAssembly: after a C `longjmp`, a stack trace no longer shows the frames the jump
  left (`setjmp` keeps the shadow stack's depth).
