## Compiler

- At `-O2`, a C call in tail position becomes a jump also when a local whose address
  escaped earlier belongs to a block that has ended by then (C's lifetimes: the object
  is no longer alive), as in `if (...) { struct node n = {...}; if (f(&n)) return 1; }
  return g(x);`. A call while such an object is still alive (in its block, or an object
  of the function's body) keeps the caller's frame, as before.
