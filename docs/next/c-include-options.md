## C

- `axiom cc -iquote dir` (`axiom build --c-iquote=dir`) searches `dir` for
  `#include "..."` only, after the including file's directory and before the `-I`
  directories, as a conventional cc does; `#include <...>` no longer looks there.
- `axiom cc` takes `-imacros file` (`--c-imacros=`: the file's macros, not its text),
  `-nostdinc` (`--c-nostdinc`: neither the compiler's nor the system's headers are
  searched), `-undef` (`--c-undef`: no target macros, nor the GNU compilers'), `-H`
  (`--c-show-includes`: the headers read, on standard error) and `-dD` (with `-E`,
  `--c-pp-defines`: the unit's `#define` and `#undef` directives kept in the text), which
  were errors.
