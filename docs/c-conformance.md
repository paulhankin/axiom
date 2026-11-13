# C conformance

This is the clause-by-clause account of the C front end and library ([`c.md`](c.md),
[`c-text.md`](c-text.md)) against the standards it implements: C89 (ANSI X3.159-1989 /
ISO/IEC 9899:1990, with Amendment 1: `--c-std=c95`), C99, C11, C17 and C23. It was
written by walking each standard's table of contents and its Annex J. The clauses are
C17's (C11's numbering is the same; C99's library clauses are the table below; C23's
differ where shown); C89's are those of the same topics in its clause 3 (language) and
4 (library).

A row's status:

- **yes**: implemented, in every standard that has the clause (with each standard's
  differences: a later feature is diagnosed in an earlier mode, `C2100`-`C2142` and
  `C1017`), and tested by the tests named (paths under `cmd/axiom/internal/tests/c/`:
  `run/` programs built and run, `pp/` preprocessor outputs, `parse/` syntax trees,
  `lib/` hosted library programs; `*` patterns name families).
- **deviation**: implemented but for what is described (the list of [`c.md`](c.md),
  "Known deviations").
- **n.a.**: not applicable here (an optional feature not provided, as its feature macro
  says), or informative text.

Four checks cover whole clauses at once and are named below by these names:

| Check | What it verifies |
|---|---|
| **constraints** | `tools/c_constraints.mjs` over `c/constraints.txt`: every Constraints paragraph of clause 6 (and 5.1.2.2.1's, 6.10.8's, C23's new ones), 113 paragraphs, 229 programs, each violation rejected with its diagnostic (5.1.1.3), programs at a constraint's edge accepted. |
| **limits** | `tools/c_limits.mjs`: every translation limit of 5.2.4.1 reached by a program built and run in C89, C17 and C23, and the limit exceeded four or eight times. |
| **names** | `tools/c_header_names.mjs` over `c/headers/names.txt`: every header in every standard declares and defines only that standard's names and those it reserves (7.1.3, future library directions), and all it requires. 231 header and standard pairs. |
| **abi** | `run/abi_gen_*`, `tools/gen_c_abi.py`: the calling conventions and layouts against the platforms' C compilers. |

## Library clause numbers

| Header | C89 | C99 | C11, C17 | C23 |
|---|---|---|---|---|
| `<assert.h>` | 4.2 | 7.2 | 7.2 | 7.2 |
| `<complex.h>` | - | 7.3 | 7.3 | 7.3 |
| `<ctype.h>` | 4.3 | 7.4 | 7.4 | 7.4 |
| `<errno.h>` | 4.1.3 | 7.5 | 7.5 | 7.5 |
| `<fenv.h>` | - | 7.6 | 7.6 | 7.6 |
| `<float.h>` | 4.1.4 | 7.7 | 7.7 | 7.7 |
| `<inttypes.h>` | - | 7.8 | 7.8 | 7.8 |
| `<iso646.h>` | (AMD1) | 7.9 | 7.9 | 7.9 |
| `<limits.h>` | 4.1.4 | 7.10 | 7.10 | 7.10 |
| `<locale.h>` | 4.4 | 7.11 | 7.11 | 7.11 |
| `<math.h>` | 4.5 | 7.12 | 7.12 | 7.12 |
| `<setjmp.h>` | 4.6 | 7.13 | 7.13 | 7.13 |
| `<signal.h>` | 4.7 | 7.14 | 7.14 | 7.14 |
| `<stdalign.h>` | - | - | 7.15 | 7.15 |
| `<stdarg.h>` | 4.8 | 7.15 | 7.16 | 7.16 |
| `<stdatomic.h>` | - | - | 7.17 | 7.17 |
| `<stdbit.h>` | - | - | - | 7.18 |
| `<stdbool.h>` | - | 7.16 | 7.18 | 7.19 |
| `<stdckdint.h>` | - | - | - | 7.20 |
| `<stddef.h>` | 4.1.5 | 7.17 | 7.19 | 7.21 |
| `<stdint.h>` | - | 7.18 | 7.20 | 7.22 |
| `<stdio.h>` | 4.9 | 7.19 | 7.21 | 7.23 |
| `<stdlib.h>` | 4.10 | 7.20 | 7.22 | 7.24 |
| `<stdnoreturn.h>` | - | - | 7.23 | 7.25 |
| `<string.h>` | 4.11 | 7.21 | 7.24 | 7.26 |
| `<tgmath.h>` | - | 7.22 | 7.25 | 7.27 |
| `<threads.h>` | - | - | 7.26 | 7.28 |
| `<time.h>` | 4.12 | 7.23 | 7.27 | 7.29 |
| `<uchar.h>` | - | - | 7.28 | 7.30 |
| `<wchar.h>` | (AMD1) | 7.24 | 7.29 | 7.31 |
| `<wctype.h>` | (AMD1) | 7.25 | 7.30 | 7.32 |
| Future directions | 4.13 | 7.26 | 7.31 | 7.33 |

## Clause 4: conformance

| Clause | Status | Tests |
|---|---|---|
| 4p5 strictly conforming programs | yes: every mode accepts its standard's language and diagnoses extensions (but `#pragma`s) | `run/c89`, `run/c99_features`, `run/c23`, `run/err_*_c89` ... |
| 4p6 hosted and freestanding implementations | yes: hosted by default, `--c-freestanding` (the freestanding headers, C23 4p7's `<string.h>`) | `lib/core/hosted`, `lib/core/freestanding`, `lib/ext1/freestanding` |
| 4p8 implementation-defined behavior documented | yes: J.3 below | |
| 4p9 extensions only if no strictly conforming program changes | yes: GNU extensions only in `gnuNN` modes | `run/err_gnu_*_c17` |

## Clause 5: environment

| Clause | Status | Tests |
|---|---|---|
| 5.1.1.1 program structure, translation units | yes | `C programs` section (several units), `mix/*` |
| 5.1.1.2 translation phases 1-8 | yes (phase 1: UTF-8 source; trigraphs C89-C17) | `pp/*` (`trigraphs*`, `splice*`, `concat*`), `parse/*` |
| 5.1.1.3 diagnostics | yes: every constraint and syntax rule violation is an error | **constraints**, `run/err_*`, `pp/err_*` |
| 5.1.2.1 freestanding environment | yes | `lib/core/freestanding` |
| 5.1.2.2.1 program startup (`main`'s forms) | yes; others `C3061` | **constraints**, `run/err_main_type` |
| 5.1.2.2.2 program execution | yes | `lib/*` |
| 5.1.2.2.3 program termination (`main`'s return is `exit`'s) | yes | `lib/core/exit_reenter`, `lib/stdio/exit_flush` |
| 5.1.2.3 program execution, side effects, sequence points | yes | `run/control`, `run/err_constexpr_volatile` |
| 5.1.2.4 multi-threaded executions and data races | yes: sequentially consistent atomics, threads | `run/atomic_*`, `mix/stdio_threads`, `lib/threads/*` |
| 5.2.1 character sets | yes: UTF-8 source and execution sets | `pp/*utf8*`, `run/char_wide_multi` |
| 5.2.1.1 trigraph sequences (removed in C23) | yes | `pp/lex_trigraph_*` |
| 5.2.1.2 multibyte characters | yes | `lib/text/mb_strings`, `lib/text/utf8_invalid` |
| 5.2.2 character display semantics (escapes) | yes | `pp/err_escape_*`, `pp/err_escape_*` |
| 5.2.3 signals and interrupts | yes: the library is reentrant where 7.1.4p4 asks | `lib/core/signal*` |
| 5.2.4.1 translation limits | yes, every one, far beyond | **limits** |
| 5.2.4.2.1 sizes of integer types `<limits.h>` | yes | `pp/headers_*`, `run/conv_*` |
| 5.2.4.2.2 characteristics of floating types `<float.h>` | yes (`FLT_ROUNDS` is 1 always: deviation, below) | `pp/headers_c23`, `lib/math/*` |

## Clause 6: language

Every Constraints paragraph of this clause is in **constraints**, besides the tests named.

| Clause | Status | Tests |
|---|---|---|
| 6.2.1 scopes of identifiers | yes | `run/flow_scopes`, `run/scope_stmt_*`, `run/decl_misc` |
| 6.2.2 linkages of identifiers | yes | `run/err_linkage`, `C programs` |
| 6.2.3 name spaces of identifiers | yes | `run/flow_scopes`, `run/scope_stmt_*` |
| 6.2.4 storage durations of objects (thread storage C11) | yes | `run/thread_local`, `run/err_tls_*` |
| 6.2.5 types (`_Bool`, complex, `_BitInt` C23, decimal floating C23) | yes; imaginary types (optional, Annex G) too | `run/bitint*`, `run/complex_*`, `run/imaginary`, `run/dec_*` |
| 6.2.6 representations of types | yes: two's complement, J.3 | `run/conv_*` |
| 6.2.7 compatible type and composite type (C23 tags across scopes) | yes | `run/enum_c23_compat`, `run/err_untagged_compat_c23` |
| 6.2.8 alignment of objects | yes | `run/alignas*`, `run/err_alignas_*` |
| 6.3.1 arithmetic operands (conversions, promotions) | yes | `run/conv_*`, `run/conv_float_int` |
| 6.3.2 other operands (lvalues, pointers, null pointers) | yes | `run/casts`, `run/err_npc_not_ice` |
| 6.4.1 keywords | yes, per standard | `run/err_true_c17`, `run/err_*_c99` |
| 6.4.2 identifiers (UCNs, NFC in C23, `__func__`) | yes | `pp/err_ucn_*`, `pp/err_ident_nfc_c23` |
| 6.4.3 universal character names | yes | `pp/err_ucn_*` |
| 6.4.4 constants (integer, floating, character, C23 `nullptr`, `wb`, separators, decimal floating) | yes | `pp/err_int_*`, `pp/err_float_*`, `run/const_exprs`, `run/err_dec_const_*` |
| 6.4.5 string literals | yes | `pp/err_string_mix_*` |
| 6.4.6 punctuators (digraphs C95) | yes | `pp/lex_digraph_*`, `run/err_digraph_c89` |
| 6.4.7 header names | yes | `pp/include*` |
| 6.4.8 preprocessing numbers | yes | `pp/lex_ppnumber*` |
| 6.4.9 comments (`//` C99) | yes | `pp/lex_line_comment_*`, `run/err_line_comment_c89` |
| 6.5.1 primary expressions, 6.5.1.1 generic selection (C11) | yes | `run/err_generic_*`, `run/generic*` |
| 6.5.2 postfix operators (subscripts, calls, members, `++`, compound literals) | yes | `run/complit*`, `run/err_arg_*` |
| 6.5.3 unary operators (`sizeof`, `_Alignof`, C23 `alignof`) | yes | `run/err_sizeof_*` |
| 6.5.4-6.5.17 casts, binary operators, conditional, assignment, comma | yes | `run/err_operands*`, `run/err_assign_*`, `run/err_cond_*` |
| 6.6 constant expressions (C23 `constexpr` objects) | yes | `run/const_exprs`, `run/constexpr*`, `run/err_const_overflow_*` |
| 6.7 declarations | yes | `run/err_conflict_*`, `run/err_redef_*` |
| 6.7.1 storage-class specifiers (C23 `constexpr`, `auto`) | yes | `run/err_storage_repeat`, `run/err_two_storage` |
| 6.7.2 type specifiers (structures, enumerations, atomic, C23 fixed underlying types, `typeof`) | yes | `run/bitfields*`, `run/enum_c23*`, `run/atomic_*` |
| 6.7.3 type qualifiers | yes; `restrict` accepted, not used by the code generator | `run/err_qual_*`, `run/array_quals_c23` |
| 6.7.4 function specifiers | yes | `run/err_inline_*`, `run/err_noreturn_main` |
| 6.7.5 alignment specifier | yes (up to 4096) | `run/err_alignas_*` |
| 6.7.6 declarators (arrays, VLAs, functions, K&R until C23) | yes | `run/err_vla_*`, `run/err_kr_*` |
| 6.7.7 type names, 6.7.8 type definitions | yes | `run/err_typedef_redef_*` |
| 6.7.9 initialization (designators C99, `{}` C23) | yes | `run/desig*`, `run/empty_init`, `run/err_init_*` |
| 6.7.10 static assertions | yes | `run/err_static_assert` |
| C23 6.7.13 attributes | yes: the standard ones; others ignored with `C4052` | `run/attrs_c23`, `run/err_attr_*` |
| 6.8 statements and blocks (labels before declarations, C23) | yes | `run/control`, `run/err_label_*` |
| 6.8.4 selection (`switch`'s cases) | yes | `run/err_case_*`, `run/err_switch_float` |
| 6.8.5 iteration | yes | `run/err_for_decl_*` |
| 6.8.6 jumps | yes | `run/err_vla_goto*`, `run/err_break_outside` |
| 6.9 external definitions, 6.9.1 functions, 6.9.2 objects (tentative) | yes | `run/err_internal_undefined`, `run/err_tentative_internal` |
| 6.10 preprocessing directives | yes | `pp/*` |
| 6.10.1 conditional inclusion (`__has_include` C23, `#elifdef`) | yes | `pp/err_if_*`, `pp/if_*` |
| 6.10.2 source file inclusion | yes (200 deep) | `pp/include*`, **limits** |
| C23 6.10.3 binary resource inclusion `#embed`, `__has_embed` | yes (`limit`, `prefix`, `suffix`, `if_empty`) | `pp/embed*`, `pp/err_embed_*` |
| 6.10.3 (C23 6.10.5) macro replacement, `#`, `##`, `__VA_ARGS__`, `__VA_OPT__` | yes | `pp/macro*`, `pp/err_paste*`, `pp/err_va_*` |
| 6.10.4 line control | yes | `pp/err_line_*`, `run/err_line_presumed` |
| 6.10.5 error, C23 `#warning` | yes | `pp/err_error`, `pp/err_warning_c17` |
| 6.10.6 pragma directive (`STDC` pragmas) | yes; `FENV_ACCESS ON` not honoured: deviation | `pp/pragma*` |
| 6.10.7 null directive | yes | `pp/*` |
| 6.10.8 predefined macro names (with `__STDC_LIB_EXT1__`, `__STDC_HOSTED__`) | yes | `pp/err_define_predef`, `lib/ext1/c99`, **constraints** |
| 6.10.9 pragma operator | yes | `pp/err_pragma_op*` |
| 6.11 future language directions | n.a. (informative) | |

## Clause 7: library

Every header is in **names**; each row's tests are `cmd/axiom/internal/tests/c/lib/`'s.

| Clause | Status | Tests |
|---|---|---|
| 7.1.1-7.1.3 definitions, headers, reserved identifiers | yes: parameter names are reserved (`__s`), each standard's names only | **names**, `lib/core/headers_c89` |
| 7.1.4 use of library functions (macros, `#undef`, thread safety) | yes: data races only where the standard allows (`c.md`, "Threads") | `mix/stdio_threads`, `lib/text/time_bases` |
| 7.2 diagnostics `<assert.h>` (C23 variadic) | yes | `core/assert`, `core/assert_c23` |
| 7.3 complex arithmetic `<complex.h>` | yes, with the imaginary types | `math/math_complex`, `run/complex_*`, `run/imaginary` |
| 7.4 character handling `<ctype.h>` | yes | `core/ctype` |
| 7.5 errors `<errno.h>` | yes (thread-local `errno`) | `core/errno`, `core/strerror_all` |
| 7.6 floating-point environment `<fenv.h>` | yes on x86-64 and AArch64; WebAssembly has none | `math/math_fenv_*` |
| 7.7 characteristics of floating types `<float.h>` | yes; `*_SNAN` not defined | **names** |
| 7.8 format conversion of integer types `<inttypes.h>` | yes | `text/inttypes` |
| 7.9 alternative spellings `<iso646.h>` | yes | `pp/err_header_iso646_c89` |
| 7.10 sizes of integer types `<limits.h>` | yes | **names** |
| 7.11 localization `<locale.h>` | yes: "C", "POSIX", UTF-8 names, "" | `text/locale`, `text/collation` |
| 7.12 mathematics `<math.h>` | yes but C23's `exp2m1` family, `iseqsig`, narrowing functions (the mathematics part's list in `c.md`) | `math/*` |
| 7.13 nonlocal jumps `<setjmp.h>` | yes natively; WebAssembly `C3300` | `core/setjmp*` |
| 7.14 signal handling `<signal.h>` | yes | `core/signal*` |
| 7.15 alignment `<stdalign.h>` | yes | `run/alignas` |
| 7.16 variable arguments `<stdarg.h>` (C23 one-argument `va_start`) | yes | `run/err_va_*`, `run/variadic*` |
| 7.17 atomics `<stdatomic.h>` | yes (all orders sequentially consistent) | `run/atomic_*`, `run/err_stdatomic_*` |
| C23 7.18 bit and byte utilities `<stdbit.h>` | yes | `run/bi_stdbit` |
| 7.18 (C23 7.19) boolean type `<stdbool.h>` | yes | `run/err_hdr_stdbool_c89` |
| C23 7.20 checked integer arithmetic `<stdckdint.h>` | yes | `run/bi_ckdint` |
| 7.19 (C23 7.21) common definitions `<stddef.h>` (`unreachable`, `nullptr_t`) | yes | `run/bi_offsetof`, **names** |
| 7.20 (C23 7.22) integer types `<stdint.h>` | yes | `pp/headers_*` |
| 7.21 (C23 7.23) input/output `<stdio.h>` | yes: every function, the formatted engines exact in every rounding direction | `stdio/*` |
| 7.22 (C23 7.24) general utilities `<stdlib.h>` | yes | `core/*` |
| 7.23 (C23 7.25) `_Noreturn` `<stdnoreturn.h>` | yes | **names** |
| 7.24 (C23 7.26) string handling `<string.h>` | yes | `core/string_hosted`, `run/string*` |
| 7.25 (C23 7.27) type-generic math `<tgmath.h>` | yes | `math/math_tgmath` |
| 7.26 (C23 7.28) threads `<threads.h>` | yes | `threads/*` |
| 7.27 (C23 7.29) date and time `<time.h>` (C23 `TIME_THREAD_ACTIVE`, `timegm`, `_r` forms) | yes | `text/time_*` |
| 7.28 (C23 7.30) Unicode utilities `<uchar.h>` | yes | `text/mb_strings` |
| 7.29 (C23 7.31) wide character utilities `<wchar.h>` | yes | `text/wcs*`, `stdio/wide*` |
| 7.30 (C23 7.32) wide character classification `<wctype.h>` | yes (Unicode 15.1) | `text/wctype_all` |
| 7.31 (C23 7.33) future library directions | yes: no header uses a reserved name it is not given | **names** |

## Annexes

| Annex | Status | |
|---|---|---|
| A language syntax summary | yes (the grammar of clause 6) | `parse/*` |
| B library summary | yes (clause 7) | **names** |
| C sequence points | yes | |
| D universal character names for identifiers (C23: UAX #31) | yes | `pp/err_ucn_ident*`, `pp/err_emoji_ident` |
| E implementation limits (`<limits.h>`, `<float.h>` minima) | yes | **names**, `pp/headers_*` |
| F IEC 60559 floating-point arithmetic | deviation: not claimed (`__STDC_IEC_559__` undefined); IEC formats, the rounding modes and flags on x86-64 and AArch64, conversions in the current rounding direction (F.5), library functions to Annex F's special cases | `math/*`, `stdio/rounding` |
| G IEC 60559-compatible complex arithmetic | deviation: not claimed (needs Annex F); G.5 and G.6's special values followed | `math/math_complex`, `run/complex_special` |
| H language independent arithmetic | n.a. (informative) | |
| I common warnings | n.a. (informative) | |
| J portability issues | below | |
| K bounds-checking interfaces | yes, all of it (`__STDC_LIB_EXT1__` 201112L) | `lib/ext1/*` |
| L analyzability | n.a. (`__STDC_ANALYZABLE__` not defined) | |
| M (C23) change history | n.a. (informative) | |

## J.3 implementation-defined behavior

The choices this implementation makes for each item of Annex J.3 (C17's numbering;
C23 J.3 has the same items, its own additions marked). Where a choice depends on the
target, the table of `c.md` "Per target" gives it.

### J.3.1 translation

| Item | Choice |
|---|---|
| How a diagnostic is identified (3.10, 5.1.1.3) | `file:line:col: error[Cnnnn]: message` with the source line, or JSON (`--format=agent-json`); warnings `Cnnnn` from 4000. |
| Whether each nonempty sequence of white space other than new-line is kept in phase 3 | It is replaced by one space. |

### J.3.2 environment

| Item | Choice |
|---|---|
| The mapping of physical source characters to the source character set | UTF-8; any other bytes are `C1024` in literals and stray elsewhere. |
| The name and type of the function called at startup in a freestanding environment | `main`, as in a hosted one. |
| Termination of a freestanding program | As a hosted one's (the start-up returns `main`'s value to the system). |
| An alternative manner of defining `main` | None (`C3061`). |
| The values given to `main`'s strings | The command line's arguments, as the system gives them (UTF-8; on Windows converted from UTF-16). |
| What an interactive device is | A terminal (`isatty`; a console on Windows). |
| Signals beyond the six of `<signal.h>`, and their numbers | Only those six, with Linux's numbers on every target. |
| Signal values other than SIGFPE, SIGILL and SIGSEGV that correspond to a computational exception | None. |
| Signals for which `signal(sig, SIG_IGN)` is the equivalent at startup | None (all are `SIG_DFL`). |
| The set of environment names and the method for altering the environment list (`getenv`) | The process's environment; `std/os.setenv` (or POSIX's `setenv`) changes it. |
| The manner of execution of the string by `system` | `/bin/sh -c` (Linux, macOS), `%ComSpec% /s /c "string"` (Windows); none on WebAssembly. |

### J.3.3 identifiers

| Item | Choice |
|---|---|
| Multibyte characters in identifiers | Those of UAX #31's XID classes (C23 Annex D), as UTF-8 or UCNs; C23: NFC required. |
| Significant initial characters in an identifier | All of them. |

### J.3.4 characters

| Item | Choice |
|---|---|
| Bits in a byte | 8. |
| Values of the members of the execution character set | Their code points (UTF-8). |
| Unique values of the escape sequences | Their ASCII codes (`\a` 7, `\b` 8, `\f` 12, `\n` 10, `\r` 13, `\t` 9, `\v` 11). |
| The value of a `char` object holding a value outside the basic set | Its byte, signed or not as `char` is (per target; `--c-char`). |
| Which of `signed char` and `unsigned char` has `char`'s range | Signed, but on AArch64 Linux unsigned. |
| The mapping of source characters in character constants and string literals | UTF-8 bytes; `L` UTF-32 (UTF-16 on Windows), `u` UTF-16, `U` UTF-32, `u8` UTF-8. |
| An integer character constant with more than one character, or a character or escape not in the basic set | `'ab'` is `'a' * 256 + 'b'` (`C4102`); a multibyte character is its UTF-8 bytes so combined. |
| A wide character constant with more than one multibyte character | The value of its last character. |
| The current locale's mapping for a wide character constant | UTF-32 (UTF-16) of its characters, in every locale. |
| A string literal with characters not in the execution set | Not possible: every code point is one. |
| The encoding of narrow, wide, `u8`, `u`, `U` literals (C23) | As above. |

### J.3.5 integers

| Item | Choice |
|---|---|
| Extended integer types | `__int128` (GNU dialects), `_BitInt(N)` for any N up to 65535 (bit-precise, C23); `intmax_t` stays 64 bits (C23 7.22.1.5). |
| Signed integer representation | Two's complement (C23 requires it). |
| Whether the extraordinary value of a signed integer is a trap representation | It is not. |
| The rank of an extended integer type relative to another of the same precision | `__int128` above `_BitInt(128)`. |
| Conversion to a signed type of a value out of range | Wraps modulo 2^N. |
| Bitwise operations on signed integers | On the two's complement bits; `>>` of a negative value is arithmetic. |

### J.3.6 floating point

| Item | Choice |
|---|---|
| The accuracy of floating operations and of the library's results | Operations correctly rounded (IEC 60559); library: `c.md`'s table (within 1 ulp, most correctly rounded). |
| The accuracy of conversions between floating representations and character strings | Exact, correctly rounded in the current direction (`<stdio.h>`, `<stdlib.h>`). |
| The rounding behaviors of `FLT_ROUNDS` values other than those defined | None; `FLT_ROUNDS` is 1 (deviation: it does not follow `fesetround`). |
| The evaluation methods of `FLT_EVAL_METHOD` | 0. |
| The direction of rounding of an integer converted to a floating type that cannot represent it exactly | The current direction (hardware conversions); `long double` wider than `double`: to nearest. |
| The direction of rounding of a floating value converted to a narrower one | The same. |
| How the nearest representable value is chosen for a floating constant | Correctly rounded to nearest, ties to even. |
| Whether expressions are contracted (`FP_CONTRACT`) | Never. |
| The default state of `FENV_ACCESS` | Off (`--c-fenv-access` sets it on for every function); on is honoured. |
| Additional floating-point exceptions, rounding modes, environments and classifications | None. |
| The default state of `FP_CONTRACT` | Off. |
| Whether the "inexact" exception may be raised when the rounded result equals the mathematical one | Not by the library's exact functions. |
| Whether the "underflow" (and "inexact") exception may be raised when the result is tiny but not inexact, or inexact but not tiny | Raised for a result that is subnormal or zero and inexact only. |
| Whether the functions honor the rounding direction mode (C23 F.10) | The exact ones and the conversions do; the elementary functions round to nearest. |
| The decimal floating types (C23) | Provided: IEEE 754's decimal formats in the BID encoding (docs/c.md). |

### J.3.7 arrays and pointers

| Item | Choice |
|---|---|
| The result of converting a pointer to an integer or back | The address's bits, truncated or zero-extended to the integer's width. |
| The size of the result of subtracting two pointers to elements of the same array | `ptrdiff_t` (per target). |

### J.3.8 hints

| Item | Choice |
|---|---|
| The extent to which `register` is honored | Not beyond its constraints (no address). |
| The extent to which `inline` suggestions are effective | The optimizer decides, at `-O2` and above, whatever the keyword. |

### J.3.9 structures, unions, enumerations and bit-fields

| Item | Choice |
|---|---|
| Whether a plain `int` bit-field is signed | Signed. |
| Allowable bit-field types other than `_Bool`, `signed int`, `unsigned int` (and `_BitInt`) | All integer types, enumerations (`char`, `short`, `long`, `long long`, `__int128`); not atomic ones (`C3070`). |
| Whether atomic types are permitted for bit-fields | No (`C3070`). |
| Whether a bit-field can straddle a storage-unit boundary | No (System V, AAPCS64 and Windows rules: `c.md`, "Bit-fields"). |
| The order of allocation of bit-fields within a unit | From the least significant bit. |
| The alignment of non-bit-field members | Their type's (natural), or `#pragma pack`'s. |
| The integer type compatible with each enumerated type | `unsigned int` with no negative enumerator, else `int` (C23: wider when needed, or the fixed type). |

### J.3.10 qualifiers

| Item | Choice |
|---|---|
| What constitutes an access to a volatile object | Each load or store of it in the abstract machine, none merged or removed. |

### J.3.11 preprocessing directives

| Item | Choice |
|---|---|
| The locations searched for `<...>` and `"..."` headers, and how the name is mapped | `"file"`: the includer's directory, then `-I`, then `std/c/include`; `<file>`: the last two. The name is a path as written. |
| The mapping of a header name's characters to a file name | Byte for byte (UTF-8). |
| How `#include` with macro-replaced tokens is combined into a header name | The tokens' spellings joined, spaces where the tokens had them. |
| The nesting limit for `#include` | 200. |
| Whether `#` in a string literal of a stringized argument is escaped before `\u` | Yes, as any `\`. |
| The behavior on each recognized non-STDC `#pragma` | `c.md`, "Pragmas". |
| `__DATE__` and `__TIME__` when not available | Always available (the build's time, or `SOURCE_DATE_EPOCH`). |
| `#embed`'s resources and their element width (C23) | Files, by the include rules; 8-bit elements (`CHAR_BIT`). |

### J.3.12 library functions

| Item | Choice |
|---|---|
| Macros and functions in a freestanding implementation | The freestanding headers and C23's `<string.h>` subset. |
| The format of `assert`'s diagnostic | `file:line: function: assertion failed: expression`. |
| The representation of the floating-point status flags (`fegetexceptflag`) | The flags' bits (MXCSR's or FPSR's layout, as `FE_*`). |
| Whether `feraiseexcept` also raises "inexact" with "overflow" or "underflow" | No. |
| Strings other than "C" and "" as `setlocale`'s second argument | "POSIX", "C.UTF-8", and names whose codeset is UTF-8. |
| The types of `float_t` and `double_t` | `float` and `double`. |
| Domain errors of the mathematics functions beyond those required | None. |
| The values returned on a domain error | NaN (`EDOM`, "invalid"). |
| Whether a mathematics function sets `errno` to `ERANGE` on underflow | Yes (a subnormal or zero inexact result). |
| Whether `fmod` with a zero second argument is a domain error or returns zero | A domain error (NaN). |
| The base-2 logarithm of the modulus used by `remquo` | 31. |
| Whether a signal handler is reset to `SIG_DFL` before it is called | No (7.14.1.1p3's second choice: the signal is blocked meanwhile). |
| The null pointer constant `NULL` | `((void *)0)`. |
| Whether the last line of a text stream needs a new-line | No. |
| Whether spaces before a new-line written to a text stream appear when read | Yes. |
| The number of null characters that may be appended to a binary stream | None. |
| Whether the file position of an append stream is first at the beginning or end | At the end (and every write at the end). |
| Whether a write on a text stream truncates the file beyond that point | No. |
| The characteristics of file buffering | `c.md`, "Streams": `BUFSIZ` 4096; `stdin`/`stdout` line buffered on a terminal, else full; `stderr` unbuffered. |
| Whether a zero-length file exists | Yes. |
| The rules for valid file names | The system's (UTF-8; on Windows any name `CreateFileW` takes). |
| Whether the same file can be opened several times | Yes (but by `fopen_s` for writing on Windows: exclusive). |
| The nature and choice of encodings for multibyte characters in files | The locale's: single-byte in "C", UTF-8 otherwise. |
| The effect of `remove` on an open file | Linux and macOS: removed, the open stream keeps it; Windows: the system's (fails while open, but `tmpfile`'s). |
| The effect if a file of `rename`'s new name exists | Replaced, on every target. |
| Whether an open temporary file is removed on abnormal termination | Linux and macOS: it was removed at once; Windows: no. |
| Which changes of mode `freopen` with a null file name permits | Any whose reading and writing the file was opened for; text and binary; appending. |
| The style of infinity or NaN printed | `inf`, `nan`, `-nan` (upper case for `F`, `E`, `G`, `A`); no n-char-sequence. |
| The output of `%p` | `0x` and lower-case hexadecimal digits. |
| The interpretation of `-` in a `%[` scanset that is not first, nor last, nor second after `^` | A range. |
| The set of sequences `%p` of `fscanf` matches | What `%p` of `fprintf` writes (hexadecimal, `0x` optional). |
| `errno` after a failed `fgetpos`, `fsetpos` or `ftell` | `EOVERFLOW`, `ESPIPE`, `EBADF` or `EINVAL`, as the cause is. |
| The meaning of any n-char or n-wchar sequence in a NaN for `strtod` | Read and ignored (a quiet NaN); `nan("...")` takes a number as the payload. |
| Whether `strtod` sets `errno` to `ERANGE` when the result underflows | Yes. |
| Whether `calloc`, `malloc`, `realloc` and `aligned_alloc` return a null pointer or a unique pointer for a size of 0 | A unique pointer. |
| Whether open streams with unwritten data are flushed, removed, closed by `abort` or `_Exit` | Not flushed, not removed (nor by `quick_exit`). |
| The termination status returned by `abort`, `exit`, `_Exit` or `quick_exit` | `abort`: the signal SIGABRT (status 3 on Windows and WebAssembly); the others: the argument (`EXIT_SUCCESS` 0, `EXIT_FAILURE` 1). |
| The value `system` returns when its argument is not null | POSIX's encoding of the status; Windows' exit status; -1 on WebAssembly. |
| The range and precision of times representable in `clock_t` and `time_t` | 64-bit integers: seconds since 1970 (`time_t`), microseconds of processor time (`clock_t`). |
| The local time zone and daylight saving time | `$TZ`, `/etc/localtime`, else UTC (`c-text.md`, "Time"). |
| The era for `clock` | The process's start. |
| The `TIME_UTC` epoch | 1970-01-01 00:00:00 UTC. |
| The replacement string for `%Z` with no time zone known | Empty (a negative `tm_isdst`). |
| Whether the mathematics functions raise "inexact" for exact results | No for the exact functions; the elementary ones may. |
| Whether `feholdexcept` with traps | No traps exist. |
| The bounds-checking functions' default constraint handler (K) | `abort_handler_s`. |
| Whether `__STDC_WANT_LIB_EXT1__` undefined declares Annex K's names (K.3.1.1) | No. |
| `abort_handler_s`'s message (K) | `runtime-constraint violation: function: what` on standard error. |

### J.3.13 architecture

| Item | Choice |
|---|---|
| The values or expressions of `<float.h>`, `<limits.h>`, `<stdint.h>` macros | `c.md`, "Implementation-defined behavior", per target; checked by **names** and `pp/headers_*`. |
| The result of `sizeof` and `_Alignof` | `c.md`'s tables, per target (the platforms' ABIs, **abi**). |
| The number, order and encoding of bytes in an object | Little-endian, 8-bit bytes. |
| Whether `_Alignas` of an extended alignment is supported | Up to 4096. |
| The valid alignments | Every power of two up to 4096. |
| The value of `__STDC_ENDIAN_NATIVE__` (C23) | `__STDC_ENDIAN_LITTLE__`. |

## J.4 locale-specific behavior

| Item | Choice |
|---|---|
| Additional members of the source and execution sets | Every Unicode code point (UTF-8). |
| The presence, meaning and representation of additional multibyte characters | UTF-8 in every locale but "C" and "POSIX" (single-byte). |
| The shift states of multibyte characters | None. |
| The direction of writing of printed characters | Left to right. |
| The decimal-point character | `.` in every locale. |
| The set of printing and control characters, and the classes of `<ctype.h>` and `<wctype.h>` | `<ctype.h>`: the "C" locale's in every locale; `<wctype.h>`: ASCII's in "C", Unicode 15.1's in UTF-8 locales (`c-text.md`). |
| The native environment (`setlocale(..., "")`) | `$LC_ALL`, `$LC_<category>`, `$LANG`, else "C". |
| Collation (`strcoll`, `wcscoll`) | By code point in every locale. |
| The formats and names of `strftime` | The "C" locale's in every locale. |
| `localeconv`'s values | The "C" locale's in every locale. |

## Unspecified behavior this implementation fixes

| Item | What it does |
|---|---|
| The order of evaluation of operands and arguments (6.5p3) | Left to right (not to be relied on: the optimizer may reorder operations without side effects). |
| The values of padding bytes and bits | Zero in objects the program initializes as a whole; else unchanged. |
| Whether `setjmp` is a macro | A macro over `__axc_setjmp`. |
| Whether `errno` is a macro | A macro over `__axc_errno()`, each thread's. |
| `qsort`'s order of equal elements, and with an inconsistent comparison function | Not stable; any order, never an access outside the array (`core/qsort_inconsistent`). |
| The order of `bsearch`'s comparisons | By halving. |
| The resolution of `clock` and `timespec_get` | `timespec_getres`'s: 1 ns (Linux), 100 ns (Windows), 1 µs (macOS, WebAssembly). |
| The order and contiguity of storage that `malloc` gives | The Axiom allocator's (or the system C library's). |
| Whether `getc` and `putc` are macros (and evaluate the stream more than once) | Macros, each argument evaluated once. |
| The value of the file position indicator after a successful `ungetc` on a text stream | The position before the byte read. |
| The details of the value stored by `fgetpos` | The offset and the conversion state. |
| `strtok`'s and `rand`'s state | One for the program. |
| The `tmpnam` names | `axc-` and 12 random letters and digits, in the temporary directory. |

## Counts

| | |
|---|---|
| Clauses walked | 4 (conformance) + 22 of clause 5 + 64 rows of clause 6 + 36 of clause 7 + 13 annexes |
| Constraints paragraphs tested | 113, with 229 programs (**constraints**) |
| Translation limits tested | 21 (all of 5.2.4.1), 72 programs (**limits**) |
| Header and standard pairs checked | 231 (**names**) |
| J.3 items documented | 108 |
| Defects found and fixed in this sweep | `qsort`'s partition walked off the array with an inconsistent comparison function; parameter names in every header's prototypes (a program's macros broke them); `wchar_t` declared by `<inttypes.h>`; `strerror`, `strcoll` and `strxfrm` missing from C89 hosted `<string.h>`; `FE_DYNAMIC` missing from C23 `<fenv.h>`; `TIME_THREAD_ACTIVE` missing; `wcscoll`/`wcsxfrm` disagreeing with `strcoll` on UTF-16; `system`'s command misquoted for `cmd.exe`; the wide `%s` and `fgetwc` losing a surrogate pair's second half; `freopen(NULL, ...)` refusing modes the file allowed; printf/strtod not honoring the rounding direction (F.5); `tmpnam` taking an unreadable existing file for a free name; `C3082`'s description |
