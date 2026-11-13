## C

- The C library has Annex K, the bounds-checking interfaces: every `_s` function of
  `<stdio.h>`, `<stdlib.h>`, `<string.h>`, `<time.h>` and `<wchar.h>`, `errno_t`,
  `rsize_t`, `RSIZE_MAX` and the runtime-constraint handlers, declared when a program
  defines `__STDC_WANT_LIB_EXT1__` as 1; `__STDC_LIB_EXT1__` is defined (`201112L`).
  The default handler is `abort_handler_s` (docs/c.md, "Annex K").
- `tmpfile`'s files are now private to their owner (permissions 0600; on Windows an
  owner-only access control list and exclusive access), and `tmpnam` no longer takes
  an existing file it cannot read for a free name.
