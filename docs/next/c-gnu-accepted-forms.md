## C

- The GNU dialects accept more of what the common compilers accept in their default
  modes: an empty declaration at file scope; `__complex__`, `__complex`, and `_Complex`
  alone for `double _Complex`; `__real` and `__imag`; `~` of a complex value (its
  conjugate) and `__builtin_conj`, `creal`, `cimag`; `__builtin_classify_type`; a
  folded builtin such as `__builtin_constant_p` in an integer constant expression
  (`__builtin_choose_expr`'s condition, a case label); bit-fields of any integer type
  and (with the warning `C4146`) a `return` with a value in a `void` function in
  gnu89; `return` of a void expression; `register` objects as x87, MMX or mask asm
  operands; a nested function's `&&label` of its enclosing function's local label;
  and, when hosted, `alloca` as `__builtin_alloca`.
- `__real__` and `__imag__` of a `long double _Complex` value that is not an lvalue
  (where `long double` is wider than `double`) no longer stop the compiler.
