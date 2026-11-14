## Compiler

- A long function of straight-line code compiles much faster at -O2: the register
  allocator's splitting of spilled values, its sets of assigned ranges, the copy hoisting
  before allocation, the stack slots' colouring and the end of C's block scopes cost time
  quadratic in its length (each walked the whole function for every value, slot or
  scope). One C function of 1,600 intrinsic calls took 9.8 s on x86-64 and takes 1.4 s;
  the output is the same.
