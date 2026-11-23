## C

- `axiom cc -fvisibility=hidden` (and `default`, `protected`, `internal`) works: it is
  `axiom build --c-visibility=`, the visibility of the definitions that neither the
  attribute `visibility` nor `#pragma GCC visibility` gives one, so a shared library built
  with it exports only what is marked. It was accepted and did nothing.
- `#pragma GCC visibility push(default)` inside `push(hidden)` (or under
  `-fvisibility=hidden`) gives default visibility; the pragma's "default" and "none in
  force" were one value.
- Declarations carry their visibility: an undefined symbol of an object has its
  declaration's, the linker takes the most constraining of a symbol's, and a hidden
  declaration's references bind to the module's own definition (directly, not through the
  GOT), so a shared library made of objects does not export what its objects declare
  hidden.
