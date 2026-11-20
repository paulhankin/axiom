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
- The GNU dialects give an enumeration whose constants do not all fit `int` C23's types
  before C23 too (the first of `unsigned int`, `long`, ... that holds them, and the
  constants that type), as the system's `<sys/epoll.h>` needs (`EPOLLET = 1u << 31`);
  under an ISO standard before C23 such a constant stays `C3013`.
- `<stddef.h>` replaces a definition of `offsetof` the program made before including it
  in a GNU dialect, as it replaces an earlier `NULL`; and with `--c-lib=system` a system
  header's definition of a macro the program defined first (a disabled `assert` of its
  own, then `<assert.h>`) replaces the program's instead of `C1060`.
- main's argument strings are one block, one after another (as the system's start-up
  gives them), so a program that retitles its process by writing over the space from
  `argv[0]` to the end of the last argument no longer overwrites the allocator's memory.
- `_mm_malloc`'s memory is `posix_memalign`'s outside Windows and `_mm_free` is `free`,
  as with other compilers' headers: a program may free either's memory with the other.
- `<immintrin.h>` has `_mm256_set_epi64x`, `_mm256_setr_epi64x` and `_mm256_set1_epi64x`
  (it had them without the `x`, which are not Intel's names), and the x86-64 assembler
  takes `movd` between an `xmm` register and a 64-bit general register (the GNU
  assembler's spelling of `movq`).
