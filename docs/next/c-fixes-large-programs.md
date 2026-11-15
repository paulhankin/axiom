## C

- A fallthrough declaration (`[[fallthrough]];`, `__attribute__((fallthrough));`) that
  ends a block is accepted when the label after the block is a `case` or `default`
  label of the switch (`case 1: { ...; [[fallthrough]]; } case 2:`), as C23 6.7.13.5p2
  asks of the next block item reached; one that ends a loop's or an inner switch's body
  stays `C3077`.
- The GNU dialects accept a translation unit without declarations (one that conditional
  inclusion leaves empty); under an ISO standard it stays `C2026`.
- `--c-lib=system`: a system header's redefinition of a macro that another system header
  defined replaces it (as the system's `<limits.h>` does after a library's own header
  supplied `CHAR_MAX`), instead of the error `C1060`; and `atexit`, `at_quick_exit` and
  `pthread_atfork`, which the system's C library keeps in its static part
  (`libc_nonshared.a`) rather than in its shared object, link.
- A C program links from objects and libraries alone (`axiom build main.o util.o libx.a`,
  outside a package): the C start-up and the C library (`--c-lib`) are added as when a
  C source is given (issue #63).
- `main` may be an object's or an archive member's when no translation unit defines one
  (`axiom build util.c main.o`), with the start-up a unit's `main` has (arguments,
  constructors, `exit` with its value); `C3063` is reported only when neither the sources
  nor the objects and archives linked define `main`, and a source's `main` beside an
  object's is `C3064` (issue #64).
