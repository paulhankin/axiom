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
  A block's restricted pointer is also apart from everything not based on it within
  the block (parameters, globals, locals, other pointers), through copies, arithmetic,
  casts and loops' pointers, inlined calls included; a local structure with
  restrict-qualified members is still taken apart into registers. A copy of a restricted
  pointer used after its block, or a pointer to restricted members taken with `&`, was
  taken as apart from another such copy before.
- `#pragma GCC optimize`, `target`, `push_options`, `pop_options` and `reset_options`
  give the functions defined after them their optimization level and extensions (as the
  attributes `optimize` and `target`); `#pragma GCC unroll n`, `ivdep` and `novector`
  are honoured by the loop unroller and the vectorizer; `#pragma GCC dependency` warns
  (C4115) when the named file is newer.
- The GNU dialects have `#import` and, before C23, the decimal floating types and their
  constants.
- docs/c.md no longer says that signed overflow wraps: the optimizer takes signed `+`,
  `-` and `*` as not overflowing (as it has since C's arithmetic was marked so).
- A GNU designator range whose initializer has effects (`[0 ... 9] = f()`) evaluates it
  once and gives every element its value; it was the error `C3901` before.
- Nested functions may be variadic (called directly and through pointers, on every
  target), and may use their enclosing functions' variable length arrays and variably
  modified types (typedefs, pointers to variable length arrays, their own parameters'
  types whose lengths name the enclosing function's objects). A pointer to a variable
  length array or a variably modified parameter in a nested function was miscompiled
  before; the other cases were the error `C3901`.
- `goto *` and `&&label` where variable length arrays or objects with cleanups are in
  scope: the jump frees the arrays and calls the cleanups of the scopes it leaves, and
  traps when it would jump into such a scope; it was the error `C3901` before.
- A bit-field of a packed structure that no unit of 1, 2, 4 or 8 bytes holds (one that
  spans 9 bytes, an `__int128` or `_BitInt` one across 8-byte boundaries, one in a
  structure smaller than its unit) is reached through the bytes it spans; it was the
  error `C3901`. Packed bit-fields wider than 64 bits no longer read or write the bytes
  after them.
- `__bf16` in the GNU dialects: bfloat16, with conversions rounded once from every
  type, arithmetic in float rounded after each operation, the platforms' ABI on x86-64 and
  AArch64, and `__BFLT16_*__` characteristic macros.
- `#assert`, `#unassert` and `#if #pred(answer)` in the GNU dialects, with the target's
  `cpu`, `machine` and (on Linux) `system` predicates.
- `__declspec(...)` in the GNU dialects, on every target: `align`, `thread`, `selectany`,
  `noinline`, `noreturn`, `naked`, `deprecated`, `restrict`, `allocate` ... as the GNU
  attributes that do the same; `dllexport` and `dllimport` are accepted.
- `-Apred=answer`, `-Apred(answer)`, `-A-pred=answer` and `-A-`: a GNU dialect's
  assertions on the command line.
- `__declspec` under an ISO standard when the target is Windows (whose headers use it),
  with its modifiers on objects, functions, structure tags and members.
- `_Complex __bf16` and vectors of `__bf16`, computed as `_Float16`'s are.
- The format checks take `_Decimal64x` with `%DD` (the library's `printf` and `scanf`
  read and write it so).
- `restrict` on block-scope `static` pointers, on file-scope pointers (in `main`, whose
  block is theirs), and across structure assignments and fills; the rules for
  restrict-qualified members now hold in `main` only (the block of a file-scope
  structure type's members is `main`'s, which code outside `main` may not be in).
- An asm constraint that is not one of the target's, an operand wider than 64 bits in a
  register and an explicit register variable on WebAssembly are the rule violation
  `C3096`, no longer `C3901`.
