## C

- The GNU dialects accept more of what the common compilers' default modes accept: pointers
  to incompatible object types converted in an assignment, initialization, argument or
  `return` (the warning `C4146`), a cast to a union type of a value of a member's type,
  empty declarations at file scope, `int main(int argc)` (the warning `C4147`), `__complex`
  and `__complex__`, and `defined` made by macro replacement in `#if`.
- New in the GNU dialects: `__builtin_isfpclass`, `__builtin_issignaling`,
  `__builtin_issubnormal` and `__builtin_iszero`; `<alloca.h>`; `strdup`, `strndup`,
  `posix_memalign` and `alloca` declared by `<string.h>` and `<stdlib.h>` in every
  standard; a program's own definition of `NULL` before a header is replaced by the
  header's.
- `<math.h>` declares the float and long double versions of C89's functions in C89 mode
  too (C89 reserves their names), so that a call of `cosf` there is not an implicit
  declaration returning `int`.
- Fixed: `__builtin_nans`, `__builtin_nansf` and `__builtin_nansl` gave a quiet NaN; they
  give a signaling one. The GNU dialects' `__builtin_isnan`, `__builtin_isinf`,
  `__builtin_fpclassify` and their kin took the x87 format's invalid encodings
  (pseudo-infinities, pseudo-NaNs, unnormals) for infinities or numbers; they are NaNs,
  as for `<math.h>`'s macros.
