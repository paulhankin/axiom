## C

- The GNU dialects (`--c-std=gnu89` ... `gnu23`) accept, each with a warning, what the
  common compilers' default modes accept and the ISO modes keep as errors: a pointer to
  a function converted to or from `void *` without a cast (`C4140`), an assignment,
  initialization, argument or `return` that discards `const`, `volatile` or `restrict`
  from the pointed-to type (`C4141`), `?:` with one `void` operand (its result is
  `void`: `C4142`), a structure or union cast to its own type (`C4143`), an
  enumeration named before its list, an incomplete type until the list (`C4144`), and in
  dialects older than their standard the keywords spelled in the reserved namespace
  (`_Bool`, `_Static_assert`, `_Generic`, ...: `C4145`).
- In a GNU dialect `__attribute__((...))` may appear in type names: at their start, inside
  an abstract declarator before `*`, among a pointer's qualifiers.
- `__LLP64__` is defined on the Windows targets (32-bit `long`, 64-bit pointers).
- Fixed: an address in an initializer of an object with static storage duration that a
  later designator replaced (`{ [0 ... 2] = &f, [1] = g }`, `{ p, [0] = q }`) was kept
  beside the new value, making the object larger and giving it the wrong elements.
