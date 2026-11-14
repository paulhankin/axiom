## C

- With `--c-lib=system`, the system's `<math.h>` works under every standard: its
  `HUGE_VAL`, `HUGE_VALF`, `HUGE_VALL` and `INFINITY` (written as out-of-range constants
  for compilers other than its own) are infinities, its comparison macros (`isgreater`
  ...) are the compiler's built-ins under the ISO standards too, and in C23 its `INFINITY`
  and `NAN` no longer clash with `<float.h>`'s. `<limits.h>` adds C23's
  `BITINT_MAXWIDTH`, `BOOL_MAX`, `BOOL_WIDTH` and `__STDC_VERSION_LIMITS_H__` to the
  system's.
- With `--c-lib=system` in a GNU dialect, the system's headers no longer define
  `__attribute__` away: a program's attributes (`packed`, `aligned`, `constructor`, ...)
  after them were ignored.
- With `--c-lib=system` on AArch64 Linux, `<signal.h>` (which reaches the kernel's headers)
  compiles under the ISO standards and the GNU dialects. The GNU dialects have the type
  names `__int128_t` and `__uint128_t`.
