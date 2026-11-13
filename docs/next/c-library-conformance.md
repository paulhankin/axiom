## C

- `docs/c-conformance.md` accounts for every clause of C89 to C23 and every
  implementation-defined choice of Annex J.3; the suite checks that every constraint of
  clause 6 is diagnosed, that every translation limit is met, and that each standard
  header declares only its standard's names (parameter names in prototypes are now
  reserved ones, so a program's macros cannot break them).
- `printf`, `scanf`, `strtod` and `strtof` follow the current rounding direction.
  `getc`, `putc`, `getchar` and `putchar` are also macros with in-line fast paths;
  `fgets`, `fscanf`, `wcslen`, `wmemchr` and the wide classification functions are
  faster (the classes are generated tables).
- `TIME_THREAD_ACTIVE` (C23) is provided, the local time zone follows changes of
  `$TZ`, `system` passes its command to `cmd.exe` as it is, and `wcscoll` orders by
  code point on Windows too. `qsort` stays inside the array whatever the comparison
  function does.
