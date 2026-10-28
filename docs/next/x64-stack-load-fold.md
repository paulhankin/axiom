## Compiler

- Fixed: native x86-64 code (at `-O1` and above) could read a local array's element
  too late, after a store through a pointer to it, or at the wrong index. A load from a
  local array was moved into the instruction that used its value, as only a spill
  slot's reload may be, past an instruction overwriting the register of its index (the
  0 of a comparison's result). A loop comparing a name against each of a local array of
  names (`for k in known { if str.eq(name, k) ... }`) could compare it against the
  first's length every time.
