## C

- Enumeration constants wider than `long long`: in a GNU dialect an enumeration may have
  the fixed underlying type `__int128` or `unsigned __int128` with constants of any value
  of it, and in C23 one without a fixed type whose values `long long` and
  `unsigned long long` do not hold has the type `__int128` or `unsigned __int128`. The
  constants follow C23's typing rules in the list (the type of their expression, or a
  wider type past the previous one's) and after it (the enumerated type). Such a constant
  was the error `C3212` before.
- Non-local `goto` in the GNU dialects: a nested function may jump to a local label
  (`__label__`) of its enclosing function, on every target (WebAssembly with its
  exception-handling extension). The cleanups and variable length arrays of the scopes
  the jump leaves, in the nested function and in the enclosing one, are unwound.
- Conversions between the decimal floating types and `_Float16` round once, directly to
  binary16, in every binary rounding direction (the environment's under strict floating
  point) with that rounding's exceptions; they went through binary128 before.
- `restrict` on block-scope pointer objects and on structure members is used by the
  optimizer (accesses through two of a block's restricted pointers, or two
  restrict-qualified members of one structure object, are apart: loads after stores are
  forwarded, loops vectorized without an overlap check), and a function's restrict
  parameters keep telling it once the function is inlined (the call becomes a scope).
