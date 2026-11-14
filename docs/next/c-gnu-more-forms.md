## C

- The GNU dialects accept more of what the common compilers' default modes accept: a
  `void *` compared with a function pointer (the warning `C4149`), a string literal
  longer than the character array it initializes, the rest dropped (`C4150`), a flexible
  array member of a union (`C4151`), the initializer of a flexible array member of a
  structure that is the first member of a static union, and, without a diagnostic,
  unnamed parameters in a function definition, labels before declarations and at the
  end of a block and `_Static_assert` without a message in every standard.
- GNU vector types: `vector_size` in a type name (a compound literal's, a cast's) makes
  the vector type (it was ignored, with a warning); the result of a vector comparison
  converts to the unsigned vector of its shape in an assignment, initialization,
  argument or `return`.
- The attribute `aligned(n)` on a function aligns its code to n bytes, and `__alignof__`
  of the function is n.
- New builtins: `__builtin_isnanf`, `__builtin_isnanl`, `__builtin_isinff`,
  `__builtin_isinfl`, `__builtin_signbitf`, `__builtin_signbitl`, `__builtin_finite`,
  `__builtin_finitef` and `__builtin_finitel`.
