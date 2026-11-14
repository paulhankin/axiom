## C

- `__builtin_constant_p` of an expression that is not a constant expression (with no
  side effects) is 1 in code when the optimizer makes it a constant (a local initialized
  with one, a parameter of an inlined function), and stays 0 where a constant expression
  needs its value; the address of an object or function is no longer taken as a
  constant for it (a string literal's still is).
- Objects defined with a const-qualified type (nothing in them volatile or atomic) are
  read-only data, and the optimizer reads their values where they are loaded; it also
  folds comparisons that a dominating comparison of the same values decides, and an
  arithmetic right shift of -1.
- `va_arg` of an empty structure (a GNU dialect) no longer stops the compiler with an
  internal error on x86-64 Linux and AArch64 Linux; it takes nothing from the list, as
  the call passes nothing for it.
- Label addresses (`&&label`, a GNU dialect) that the code stores at run time (a local
  array initialized with them, assignments) were all the first label's on every
  target; they are now each label's.
- An asm operand with `+` evaluates its lvalue once (`"+r"(*f())` called `f` twice).
- A bit-field of a type wider than `int` promotes by its width as an `int` one does
  (a `long` bit-field of 31 bits is an `int` in expressions), as the common compilers
  have it.
