# The C library: locales, wide characters and time

The text part of the C library ([`docs/c.md`](c.md), "The C library") is `<locale.h>`,
`<wchar.h>` (all but the input/output functions, which are `<stdio.h>`'s part's),
`<wctype.h>`, `<uchar.h>`, `<time.h>`, `<inttypes.h>` and the multibyte functions of
`<stdlib.h>` (`mblen`, `mbtowc`, `wctomb`, `mbstowcs`, `wcstombs`). Its sources are
`std/c/lib/text/` (C17, one function to a file), its system layer
`std/c/rt/text.ax` (Unicode's properties) and `std/c/rt/text_time.ax` (the clocks and the
time zone), its internal interface `std/c/include/axc/text.h`, and what the other parts
of the library ask of it `std/c/include/axc/locale.h`.

Each header declares only what its standard has: `<wchar.h>` and `<wctype.h>` are C95's
(Amendment 1), `<inttypes.h>` C99's and `<uchar.h>` C11's (an `#error`, `C1040`, in an
earlier mode); C99's names (`wcstoll`, `iswblank`, ...), C11's (`timespec_get`,
`TIME_UTC`, `struct timespec`) and C23's (`timegm`, `gmtime_r`, `localtime_r`,
`timespec_getres`, `TIME_MONOTONIC`, `TIME_ACTIVE`, `char8_t`, `mbrtoc8`, `c8rtomb`,
the `b` and `B` conversions of `<inttypes.h>`, `__STDC_VERSION_..._H__`) appear from
their standard on. Before C99, `struct lconv`'s members of C99 have reserved names. In
C23, `wcschr`, `wcspbrk`, `wcsrchr`, `wcsstr` and `wmemchr` are generic (their result
points to `const` when their argument does), and `asctime` and `ctime` are deprecated.

## Locales

`setlocale` accepts, for any category:

| Name | Encoding of `LC_CTYPE` |
|---|---|
| `"C"`, `"POSIX"` | single-byte |
| `"C.UTF-8"`, and a name whose codeset (after its last `.`, before an `@modifier`) is `UTF-8` or `utf8` in any case, such as `"en_US.UTF-8"` | UTF-8 |
| `""` | the environment's name for the category: `$LC_ALL`, else `$LC_COLLATE` (and so on), else `$LANG`, the first that is set and not empty, else `"C"` |

Any other name (`"de_DE"`, `"C.ISO-8859-1"`) returns a null pointer and changes nothing.
Every category starts as `"C"`. A name is at most 63 bytes of printable ASCII without
`;` or `=`. A query of `LC_ALL` (a null pointer) when the categories differ returns
`"LC_COLLATE=a;LC_CTYPE=b;LC_MONETARY=c;LC_NUMERIC=d;LC_TIME=e"`, which
`setlocale(LC_ALL, ...)` takes back.

Every locale has the `"C"` locale's conventions: `localeconv` gives `"."` as the
decimal point and `CHAR_MAX` and empty strings for the rest; collation (`wcscoll`,
`wcsxfrm`) is by value; `strftime`'s names, formats and `AM`/`PM` are the `"C"`
locale's. The locales differ only in `LC_CTYPE`'s encoding:

- **Single-byte** (`"C"`, `"POSIX"`): every byte is a character, whose wide value is the
  byte's (so byte `0xE9` is `L'\xE9'`, and U+00E9 for `mbrtoc32`); `MB_CUR_MAX` is 1;
  a wide character beyond `0xFF` has no multibyte form (`EILSEQ`).
- **UTF-8**: well-formed UTF-8 only, as the Unicode Standard's table 3-7 has it: no
  overlong forms, surrogates or values beyond U+10FFFF, each rejected (`EILSEQ`) at the
  first byte that cannot follow; `MB_CUR_MAX` is 4.

The other parts of the library ask with `__axc_mb_utf8()` and `__axc_mb_cur_max()`
(`axc/locale.h`). `setlocale` is not safe to call while other threads use the locale
(C17 7.11.1.1 allows the data race).

## Wide characters

`wchar_t` is UTF-32, except on Windows, where it has 16 bits and is UTF-16: a character
beyond U+FFFF is a surrogate pair. There:

- `mbrtowc` stores the high surrogate of such a character and keeps the low one in its
  `mbstate_t`; the next call stores that and returns `(size_t)-3`, reading nothing, as
  `mbrtoc16` does. `mbsrtowcs` and `mbstowcs` store both (they stop before a pair when
  only one element is left).
- `wcrtomb` and `wctomb` keep a high surrogate (no bytes, 0 returned) until its low one
  comes; `wcsrtombs` and `wcstombs` read the pair as one character. A surrogate out of a
  pair is `EILSEQ`.
- `mbtowc` returns -1 (`EILSEQ`) for such a character, which one `wchar_t` cannot hold;
  `mblen` counts its bytes.

`char16_t` and `char32_t` are UTF-16 and UTF-32 in every locale (`__STDC_UTF_16__`,
`__STDC_UTF_32__`), and in C23 `char8_t` UTF-8: `mbrtoc8` gives a character's first code
unit and returns `(size_t)-3` for each of the others, and `c8rtomb` keeps the units of a
character until its last. UTF-8 and the single-byte encoding have no shift states:
`mblen`, `mbtowc` and `wctomb` return 0 for a null pointer. After an `EILSEQ` the
state is set to the initial one.

`wint_t` is the platform's (the compiler's `__AXIOM_WINT_TYPE__`); `WEOF` is
`(wint_t)-1`.

The wide numeric conversions (`wcstol`, `wcstod`, ...) skip white space as `iswspace`
says, then give the run of ASCII characters that may be in a number to the narrow
function (`strtol`, `strtod`, ...), so that both read numbers alike.

## Wide character classes

In a single-byte locale only ASCII characters have classes, those of `<ctype.h>` in the
`"C"` locale, and only `A`-`Z` and `a`-`z` have case mappings. In a UTF-8 locale every
code point has these, from Unicode 15.1's properties (the standard library's tables,
`std/unicode`):

| Class | Code points |
|---|---|
| `cntrl` | general category Cc, Cf, Zl or Zp |
| `print` | not `cntrl`, and not unassigned (Cn), a surrogate (Cs) or private use (Co) |
| `space` | White_Space |
| `blank` | `space`, and Zs or U+0009 |
| `digit` | `0` to `9` only (C17 7.30.2.1.5) |
| `xdigit` | `0`-`9`, `a`-`f`, `A`-`F` only (7.30.2.1.12) |
| `upper`, `lower` | Uppercase, Lowercase; and `print`, not `space` |
| `alpha` | Alphabetic, `upper` or `lower`; and `print`, not `space` or `digit` |
| `alnum` | `alpha` or `digit` |
| `graph` | `print` and not `space` |
| `punct` | `graph` and not `alnum` (so other scripts' digits, marks and symbols too) |

The classes relate as C17 7.30.2.1 requires. `towupper` and `towlower` are Unicode's
simple case mappings. `wctype` and `wctrans` take the standard's names in every locale.

## Time

`time_t` and `clock_t` are the 64-bit type of `int64_t`; `time_t` counts seconds since
1970-01-01 00:00:00 UTC, leap seconds ignored. The calendar is the proleptic Gregorian
one, for every year an `int` `tm_year` holds; beyond that `gmtime` and `localtime` return
a null pointer (`EOVERFLOW`) and `mktime` and `timegm` `(time_t)-1`.

- **Clocks.** `time` and `TIME_UTC` are the real-time clock, `TIME_MONOTONIC` a
  monotonic one and `clock` and `TIME_ACTIVE` the processor time of the process (in
  microseconds for `clock`: `CLOCKS_PER_SEC` is 1000000): the runtime's clocks 0, 1 and
  2 (`ax_clock_ns`, `std/README.md`). `TIME_THREAD_ACTIVE` is not provided.
  `timespec_getres` gives 100 ns on Windows, 1 µs for the real time and processor time
  on macOS and WebAssembly, and 1 ns otherwise.
- **The local time zone** is the standard library's (`std/time/tz`, `Zone.local`):
  `$TZ` (a zone name, a TZif file or a POSIX rule such as `"EST5EDT,M3.2.0,M11.1.0"`),
  else `/etc/localtime`, else UTC. It is read the first time a program needs it and kept:
  a later change of `$TZ` has no effect (there is no `tzset`). On Windows and on
  WebAssembly, which have neither, local time is UTC.
- **`mktime`** takes fields out of their ranges (each excess carried into the next field,
  as C17 7.27.2.3p2 says) and reads `tm_isdst` as POSIX does: positive presumes daylight
  saving time, 0 standard time, negative whichever is in force. A local time that occurs
  twice is the instant of the kind asked for (the first one when not asked); one in the
  gap of a change to daylight saving time is read with the offset before it (moved
  forward by the gap); a time given with the kind not in force is read with the offset
  of that kind nearest to it within a year, else an hour from the one in force.
- **`strftime`** has every conversion of C23 in the `"C"` locale; the `E` and `O`
  modifiers give the plain forms, and on other specifiers they and an unknown specifier
  are copied as they are. `%Y` is the year in full (`-5`, `12345`); `%C` the year
  divided by 100 and truncated, with at least two digits; `%y` the last two digits of
  the year's absolute value; `%G` and `%g` likewise for the ISO 8601 week-based year.
  `%z` and `%Z` are the local zone's offset and abbreviation at the time the fields name
  read as a local time with their `tm_isdst` (nothing if `tm_isdst` is negative): `struct
  tm` has no member for the zone, so `gmtime`'s result gives the local zone's standard
  time, not UTC, unless the zone is UTC.
- **`asctime`** has the standard's format, and its buffer is large enough for any `int`
  fields (a name out of range is `???`); `ctime` returns a null pointer when `localtime`
  does.

## Not provided, or limits

- The `"C"` conventions only: no locale has other names, grouping or collation.
- Unicode 15.1: the classes of characters assigned later are those of unassigned ones.
- `TIME_THREAD_ACTIVE`; `tzset` and a change of `$TZ` while the program runs.
- Every hosted C program's build analyzes `std/c/rt/text.ax` and `text_time.ax` and the
  modules they import (`std/unicode`'s tables, `std/time/tz`), whether it uses them or
  not: about 50 ms of each build.
