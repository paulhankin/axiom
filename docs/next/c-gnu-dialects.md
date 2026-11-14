## C

- `--c-std=gnu89` ... `gnu23` select the GNU dialects: the standard and the widely used
  GNU extensions: `__attribute__((...))` (aligned, packed, section, weak, alias,
  constructor, destructor, cleanup, transparent_union, mode, ...), `[[gnu::...]]`,
  statement expressions, labels as values and `goto *`, case ranges, `?:` without its
  middle operand, `__auto_type`, `__label__`, designator ranges, zero-length arrays, asm
  labels, the alternative keywords (`__inline__`, `__typeof__`, ...), the `__builtin_`,
  `__sync_` and `__atomic_` builtins, `#include_next`, `__COUNTER__` and named variadic
  macro arguments. `__GNUC__` is not defined: test `__AXIOM_GNU__`, `__has_attribute`,
  `__has_builtin`. `__int128`, inline assembly and vector types are in their own notes.
- Pragmas work in every mode: `#pragma once`, `pack`, `push_macro`/`pop_macro`, `weak`,
  `message`, `STDC`, `GCC diagnostic` (which can now silence a warning), `GCC poison`,
  `GCC system_header`, and `_Pragma`. `#pragma pack` was ignored before: structures
  under it now have its layout.
- `__has_include`, `__has_attribute` and `__has_builtin` are available in every mode
  (before, `__has_include` was C23's).
