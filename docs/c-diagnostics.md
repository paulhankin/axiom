# Diagnostics of the C front end

Every code that the C front end reports, with what it means. How to compile C is in
[`docs/c.md`](c.md); the format of diagnostics (text, or JSON with
`--format=agent-json`) is the same as for Axiom code (spec §16.2).

A code names a kind of problem, not a message: the message names the construct, in
C's terms, and can differ between uses of one code. The ranges:

| Range | Kind |
|---|---|
| `C1xxx` | Preprocessing and lexing: characters, literals, directives, macros. |
| `C2xxx` | Syntax, and features of a standard other than the one in force. |
| `C3xxx` | Constraints and semantics: declarations, types, expressions, statements, linking. |
| `C4xxx` | Warnings. A warning does not fail a build. |

Other codes can appear with C sources too: `E9001` and `E0211` for a bad command line
or an unreadable file, `E9004` when the system's linker fails. They are the driver's,
and are not listed here.

An error stops the build after the unit has been analyzed: the front end goes on after
most errors to report more of them. A feature of a later standard (or one that a
standard removed) gets one diagnostic naming the standard, after which the construct is
accepted as if the mode allowed it (so that one use gives one message).

## C1xxx: preprocessing and lexing

### Characters, literals and numbers

| Code | Meaning |
|---|---|
| `C1001` | An unterminated comment. |
| `C1002` | A string literal or character constant with no closing quote on its line. |
| `C1003` | An invalid universal character name (`\u` or `\U`): too few digits, or a value that designates no character (a surrogate, above 10FFFF) or one of the basic character set. |
| `C1004` | A universal character name that is not allowed in an identifier. |
| `C1005` | An identifier that is not in Normalization Form C. |
| `C1008` | A form feed or vertical tab inside a preprocessing directive. |
| `C1010` | An empty character constant (`''`). |
| `C1011` | A character constant with more characters than fit its type, or one with a `u`, `U` or `u8` prefix that is not one code unit. |
| `C1012` | An invalid escape sequence: a lone backslash at the end, `\x` with no digits, or an unknown escape (before C99 a universal character name is one). |
| `C1013` | An octal or hexadecimal escape sequence whose value does not fit the character type. |
| `C1014` | A malformed number: a digit separator in the wrong place or before C23, an invalid digit in an octal constant, an exponent with no digits, a hexadecimal floating constant with no exponent, or any other spelling that is no number. |
| `C1015` | An invalid suffix on an integer or floating constant, or a `wb` suffix (`_BitInt`) before C23. |
| `C1016` | An integer constant too large for its type (in `#if`: for `intmax_t` and `uintmax_t`). |
| `C1017` | A feature of a later standard in a constant or a directive: a binary constant, a `long long` constant or a hexadecimal floating constant before C99 or C23, `__has_include`, `__has_embed` or `__has_c_attribute` before C23. |
| `C1018` | A floating constant out of range of its type. |
| `C1019` | A decimal floating constant (`df`, `dd`, `dl` suffixes), which this implementation does not support. |
| `C1020` | String literals that cannot be concatenated: different encoding prefixes, or (before C99) a narrow one and a wide one. |
| `C1021` | A stray character in the program (a character that is part of no token), or a `\` before `u` or `U` before C99. |
| `C1022` | A `#` or `##` outside a preprocessing directive that is no operator there. |
| `C1023` | `__VA_ARGS__` or `__VA_OPT__` outside the replacement list of a variadic macro. |
| `C1024` | A string literal or character constant with invalid UTF-8. |

### Conditional expressions (`#if`, `#elif`)

| Code | Meaning |
|---|---|
| `C1030` | A floating constant, a string literal or another token that cannot appear in the expression. |
| `C1031` | A syntax error in the expression, or no expression at all. |
| `C1032` | Division (or remainder) by zero. |
| `C1033` | An integer operation that overflows `intmax_t`. |
| `C1034` | A comma operator in the expression. |
| `C1036` | A shift count out of range, or a left shift of a negative value. |
| `C1037` | An expression nested too deeply. |

### Directives and macros

| Code | Meaning |
|---|---|
| `C1040` | An `#error` directive; the message is its text. |
| `C1041` | An invalid preprocessing directive: an unknown name, or one of a later standard (`#elifdef`, `#elifndef`, `#warning`, `#embed` before C23) or of a GNU dialect (`#include_next`, `#ident`, `#warning` before C23) under an ISO standard. |
| `C1043` | A preprocessing directive inside the arguments of a macro invocation. |
| `C1044` | `defined` made by macro replacement in an `#if` expression. |
| `C1045` | `defined` with no identifier, or with no closing `)`. |
| `C1046` | `__has_include`, `__has_embed` or `__has_c_attribute` with no `(` or no `)`. |
| `C1048` | `__has_c_attribute` or `__has_attribute` with no attribute name, or `__has_builtin` with no builtin's. |
| `C1049` | `__has_include`, `__has_embed` or `__has_c_attribute` outside `#if` and `#elif`. |
| `C1050` | A conditional directive (`#if`, `#ifdef`, `#ifndef`) that is not closed by `#endif`. |
| `C1051` | A misused `_Pragma`: before C99, or without a parenthesized string literal. |
| `C1052` | A macro name that is not an identifier, after `#define`, `#undef`, `#ifdef` or `#ifndef`. |
| `C1053` | `#define` or `#undef` of a name that cannot be one (`defined`, a predefined macro, `__has_include`, ...). |
| `C1054` | A variadic macro before C99, or an invocation of one that leaves out its variable arguments before C23. |
| `C1055` | An invalid parameter list of a macro: a repeated parameter, a name that is no identifier, a missing `)`. |
| `C1056` | No white space between the name of an object-like macro and its replacement list. |
| `C1057` | A misused `__VA_OPT__`: nested, not followed by `(`, or not closed. |
| `C1058` | `##` at either end of a replacement list or of a `__VA_OPT__`. |
| `C1059` | `#` in a function-like macro not followed by a parameter. |
| `C1060` | A macro redefined in a way that is not the same definition. |
| `C1061` | Macro invocations nested too deeply in arguments (more than 400 levels). |
| `C1062` | The replacement of one macro is too large (more than 2,000,000 tokens). |
| `C1063` | An invocation of a function-like macro with no closing `)`. |
| `C1064` | The wrong number of arguments of a macro. |
| `C1065` | An empty macro argument before C99. |
| `C1066` | `#` made a string literal that is not valid; the message says why. |
| `C1067` | `##` pasted tokens into something that is not one token. |
| `C1070` | Extra tokens at the end of a directive or after a header name. |
| `C1071` | `#else`, `#elif` or `#endif` with no `#if`. |
| `C1072` | `#else` or `#elif` after `#else`. |
| `C1080` | `#include` or `#embed` with a header name in neither form, `"file"` or `<file>`. |
| `C1081` | An include file or embedded resource that cannot be found or read, or one that is a directory; also a header of the C library (`<stdlib.h>`, ...) in a program built with `--c-freestanding`. |
| `C1082` | `#include` nested too deeply (more than 200 files). |
| `C1083` | An invalid `#embed` parameter: not an identifier, a repeated one, one with unbalanced parentheses, one without its parenthesized clause, a bad `limit`. |
| `C1084` | An `#embed` parameter that no standard has. |
| `C1085` | An invalid `#line`: a line number that is not a positive decimal number or is out of range, or a file name that is not a string literal. |
| `C1086` | A use of an identifier that `#pragma GCC poison` poisoned. |

## C2xxx: syntax

| Code | Meaning |
|---|---|
| `C2001` | A syntax error: the message says what was expected and what was found. |
| `C2002` | Nested too deeply: more than 1000 levels of expressions, declarators and statements. |
| `C2003` | A `//` comment before C99. |
| `C2004` | An empty declaration (`;`) outside a function. |
| `C2005` | An attribute where none can appear. |
| `C2011` | A declaration where only a statement can be (the body of `if`, `while`, ...). |
| `C2015` | A function definition inside a function. |
| `C2020` | A storage-class or function specifier where it cannot appear. |
| `C2021` | A structure or union with no members. |
| `C2022` | The left operand of an assignment is not a unary expression. |
| `C2023` | An enumeration with no enumerators. |
| `C2024` | A declaration with no declaration specifiers. |
| `C2025` | An unknown type name: an undeclared identifier where a declaration begins. |
| `C2026` | An empty translation unit. |
| `C2027` | A cast as the operand of prefix `++` or `--`. |

The next codes are features of another standard than the one in force. The message
names the feature and the standard that has it (or removed it).

| Code | Meaning |
|---|---|
| `C2100` | A keyword of a later standard (`_Bool`, `inline`, `restrict`, `_Generic`, `typeof`, `constexpr`, ...). |
| `C2101` | `long long` (C99). |
| `C2106` | Declarations mixed with code (C99). |
| `C2107` | A declaration in the first clause of `for` (C99). |
| `C2108` | Designated initializers (C99). |
| `C2109` | Compound literals (C99). |
| `C2110` | `[*]` array declarators (C99). |
| `C2111` | `static` and type qualifiers in array parameter declarators (C99). |
| `C2112` | A trailing comma in an enumerator list (C99). |
| `C2113` | Implicit `int`: a declaration with no type specifier (removed in C99). |
| `C2127` | Anonymous structures and unions (C11). |
| `C2130` | Attributes (`[[...]]`; C23). |
| `C2135` | An enumeration with a fixed underlying type (C23). |
| `C2136` | An empty initializer `{}` (C23). |
| `C2137` | An unnamed parameter in a function definition (C23). |
| `C2138` | A label before a declaration or at the end of a block (C23). |
| `C2139` | `...` as the only parameter (C23). |
| `C2140` | A storage-class specifier in a compound literal (C23). |
| `C2141` | A static assertion without a message (C23). |
| `C2142` | An identifier list (a K&R parameter list), removed in C23. |

## C3xxx: constraints and semantics

### Declarations and types

| Code | Meaning |
|---|---|
| `C3001` | An undeclared identifier, or a name that is not a type name where one is needed. |
| `C3002` | A redefinition of an object, function, tag, label, member, parameter or enumeration constant. |
| `C3003` | Conflicting types, or kinds of declaration, for one identifier. |
| `C3004` | Conflicting linkage (declared `static` after a declaration with external linkage, ...). |
| `C3005` | An invalid combination of type specifiers (`_Complex` too: only `float`, `double` and `long double _Complex`, in any order). |
| `C3006` | An invalid storage-class or function specifier, or type qualifier: repeated where that is not allowed, or on a type that cannot take it. |
| `C3007` | An incomplete type where a complete one is needed. |
| `C3008` | An invalid declarator or member: arrays of functions, functions returning arrays, a flexible array member that is not last, a member of function type, and so on. |
| `C3009` | An invalid array size: not an integer, not positive, or too large (more than 2^47 bytes). |
| `C3010` | A declaration that declares nothing, a member declaration that declares no member, or a name where none can be. |
| `C3011` | The wrong kind of tag (`union S` for a structure), or an enumeration that is not defined. |
| `C3013` | An enumeration constant out of range (of `int`, before C23; of the fixed underlying type). |
| `C3033` | An invalid initializer: of a typedef, a function or a parameter, an `extern` object in a block, or too many braces around a scalar. |
| `C3045` | A function with internal linkage that is used and not defined (6.9p3). |
| `C3050` | A static assertion that fails. |
| `C3070` | An invalid bit-field (its type, an atomic type included, or width), or an operator applied to one (`&`, `sizeof`, `typeof`). |
| `C3071` | An invalid alignment specifier (an incomplete type, a negative value, a value that is not a power of two or is larger than 4096), one where none may be, or objects declared with different alignments. |
| `C3073` | An invalid `constexpr` declaration or initializer (C23). |
| `C3074` | An invalid declaration whose type is inferred (`auto`, C23). |
| `C3075` | An invalid fixed underlying type of an enumeration, or one that disagrees with an earlier declaration (C23). |
| `C3076` | `thread_local` where it cannot be, or declarations that disagree about it. |
| `C3077` | An invalid argument clause of a standard attribute, or an attribute where it cannot be. |
| `C3078` | An invalid compound literal: of variable length array or incomplete type, or with a storage-class specifier it cannot have. |
| `C3092` | An invalid atomic type: the `_Atomic` qualifier on an array or function type, or `_Atomic ( type-name )` of an array, function, atomic or qualified type. |
| `C3200` | A generic selection with more than one `default` association. |
| `C3201` | An association of a generic selection whose type is not a complete object type, or is variably modified. |
| `C3202` | Two associations of a generic selection with compatible types. |
| `C3203` | No association of a generic selection matches the controlling expression's type, and there is no `default`. |
| `C3210` | A `_BitInt` width out of range (at least 2, or 1 for `unsigned _BitInt`; at most `BITINT_MAXWIDTH`, 65535). |
| `C3211` | An integer constant with a `wb` or `uwb` suffix that is too large for any bit-precise type. |
| `C3212` | An integer constant expression whose value is wider than 64 bits (of `__int128` or a `_BitInt`) where a value of at most 64 bits is needed: an alignment, a bit-field's width, an enumeration constant, a designator's index. |
| `C3220` | `main` declared `inline` or `_Noreturn`. |
| `C3221` | An inline definition that defines a modifiable object with static storage duration or refers to an identifier with internal linkage (6.7.4p3). |
| `C3222` | The address of an object declared `register` taken, or a `register` array used as a pointer. |

### Expressions and initializers

| Code | Meaning |
|---|---|
| `C3020` | Invalid operands to an operator (or a type name used as a value). |
| `C3021` | An assignment, `++` or `--` on something that is not a modifiable lvalue: an array, a `const` object, a structure with a `const` member. |
| `C3022` | Incompatible types in an assignment, initialization, argument or `return`, or one that discards qualifiers. |
| `C3023` | The wrong number of arguments in a call. |
| `C3024` | A call of something that is not a function or a pointer to one. |
| `C3025` | A member access on something without that member, or on a type that is no structure or union (also `offsetof`). |
| `C3026` | The operand of `&` is not an lvalue. |
| `C3028` | An invalid cast. |
| `C3029` | An invalid operand of `sizeof` or `_Alignof`: a function type or an incomplete type. |
| `C3030` | A void value used, or a controlling expression or operand that must be a scalar or integer and is not. |
| `C3031` | An initializer of an object with static storage duration that is not constant. |
| `C3032` | Excess elements in an initializer, or a string too long for its array. |
| `C3034` | An integer constant too large for any integer type. |
| `C3035` | A constant expression is needed and the expression is not an integer constant expression. |
| `C3036` | A constant expression whose evaluation overflows (a signed integer operation, 6.6p4). |
| `C3060` | A call of an undeclared function (C99 and later; before C99 it is warning `C4001`). |
| `C3072` | An invalid designator in an initializer. |

### Statements and functions

| Code | Meaning |
|---|---|
| `C3040` | `case` or `default` outside a `switch` statement. |
| `C3041` | A duplicate `case` value, or a second `default` label. |
| `C3042` | `break` outside a loop or `switch`, or `continue` outside a loop. |
| `C3043` | A label that is used and not defined. |
| `C3044` | A `return` with a value in a function that returns `void`, or without one in a function that returns a value. |
| `C3061` | A definition of `main` of a type that is not supported: it must be `int (void)` or `int (int, char **)`. |

### Builtins, variable length arrays and linking

The codes `C3080`, `C3081` and `C3082` have two meanings each, as the variable length
array checks and the builtin checks were numbered apart. The message says which.

| Code | Meaning |
|---|---|
| `C3080` | A `va_list` builtin used wrongly: outside a variadic function, with an operand that is not a `va_list`, or `va_arg` of a type that cannot be fetched. Also: a variable length array before C99. |
| `C3081` | `offsetof` of a bit-field, or with a member designator that does not designate a member. Also: a variably modified type where none can be (file scope, a member, static storage duration, a function's result, an `extern` object). |
| `C3082` | An operand of a `<stdbit.h>`, `<stdckdint.h>`, `<complex.h>`, `<math.h>`, `<tgmath.h>` or `<fenv.h>` builtin (or of a GNU dialect's builtin) of a type it does not take (`__builtin_complex` takes two operands of one real floating type; `__builtin_creal`, `__builtin_cimag` and `__builtin_conj` a complex one; the classification macros a real one; a type-generic macro arithmetic ones, and complex ones only where it has a complex function). Also: an initializer of a variable length array (from C23, other than `{}`). |
| `C3093` | An argument of a `<stdatomic.h>` generic function of a type it does not take: an object that is not a pointer to a complete atomic type (or one to a `const` type, where the function stores), `fetch_` operations on `atomic_bool`, on a floating or structure type, or (other than `fetch_add` and `fetch_sub`) on a pointer, an `expected` argument that is not a pointer to the non-atomic type, a memory order that is not an integer. |
| `C3083` | A `goto` into the scope of an identifier with a variably modified type (or, in a GNU dialect, of an object with a cleanup). |
| `C3084` | A `case` or `default` label in the scope of such an identifier declared in its `switch` statement. |
| `C3085` | `[*]` outside the parameters of a function prototype that is not a definition. |
| `C3062` | An external name that is the name of one of the runtime's functions (`ax_write`, ...). Reported when code is generated, so `axiom check` does not report it. |
| `C3063` | The program has no function `main` (reported for an executable, not by `axiom check`). |
| `C3064` | An external name defined in two translation units; a note points to the other definition. A library's unit gives way to the program's, and a weak definition to one that is not weak, without this error. |
| `C3065` | An external name defined by a C translation unit and by an Axiom `export fn` of the same program; a note points to the Axiom definition. |
| `C3066` | A C translation unit defines `main` in a program whose root module is Axiom's (`axiom build main.ax x.c`), which has the program's `main`. |
| `C3067` | On Windows, the address of an object that a DLL defines (an object declared and defined by no unit) in the initializer of an object with static storage duration: it is not an address constant, since the program finds it when it runs. Reported by `build`. |
| `C3300` | A call of `setjmp` (`<setjmp.h>`) in a program for a WebAssembly target without its exception-handling extension (`--features=+exception-handling`), which it needs to return to a frame. Reported by `build`. |
| `C3301` | The address of `setjmp`'s function (`__axc_setjmp`) taken: it can only be called, so that the compiler knows the function calling it. Reported by `build`. |
| `C3901` | A feature that this implementation does not support: `_Imaginary` (the imaginary types), and (C23) the decimal floating types; in a GNU dialect inline assembly on WebAssembly targets, constraints it does not know (x87, MMX, mask registers), `__int128` (and `mode(TI)`), `_FloatN`, `_FloatNx` and `__float128`, vector types (`vector_size`), `aligned` on a typedef for another alignment than its type's, a packed bit-field no unit in its structure holds, a designator range whose value has effects, `goto *` or the address of a label in the scope of a variable length array or an object with a cleanup. |

### The GNU dialects

| Code | Meaning |
|---|---|
| `C3090` | An invalid use of a GNU extension: a statement expression or a label's address outside a function, an asm label on an automatic object or after a use, two asm labels for one entity, weak or alias without linkage, an alias whose target the unit does not define, a section for an automatic object, a constructor or destructor with parameters, a cleanup on an object that is not automatic or with a function of more than one parameter, `__builtin_frame_address` or `__builtin_return_address` of a level other than 0. |
| `C3091` | A call of a function declared with the attribute `error("message")`. |
| `C3096` | An asm statement or file-scope asm the rules reject: a constraint (an output's without `=` or `+`, a matching constraint that names no output in a register, a constant out of its letter's range or not a constant), an operand (of a type no register holds, two of one name, a memory operand of a bit-field or a `register` object), a clobber that is not a register or is the stack or frame pointer, an operand in a clobbered register or two in one register, a template reference to no operand or label, file-scope asm with operands, an explicit register variable of no register of the target, a naked function's body that is not asm statements without operands. |
| `C3097` | The assembler rejects an asm statement's template (with stand-in operands) or file-scope asm: an instruction, operand form or directive that is not supported or does not exist, an immediate out of range; the message names it, and its line in the text. |

## C4xxx: warnings

| Code | Meaning |
|---|---|
| `C4001` | An implicit function declaration (C89 and C95; from C99 it is error `C3060`). |
| `C4040` | A label that is defined and not used. |
| `C4050` | A use of a name or entity declared `[[deprecated]]`, with its message if it has one. |
| `C4051` | The result of a call of a `[[nodiscard]]` function is discarded, with its message if it has one. |
| `C4052` | An attribute with no prefix that is not a standard attribute; in a GNU dialect, a GNU attribute this compiler does not know, or one where it cannot appertain: it is ignored. |
| `C4053` | A function declared `_Noreturn` or `[[noreturn]]` that can return. Reported when code is generated. |
| `C4090` | A member of an object of atomic structure or union type accessed (undefined, 6.5.2.3p5): it is accessed as an ordinary object. |
| `C4091` | A pointer to an atomic type converted to a pointer to `void` that does not have the `_Atomic` qualifier (6.5.16.1p1 asks for it): the conversion is made. |
| `C4080` | The second argument of `va_start` is not the function's last parameter. |
| `C4081` | `va_arg` of a type that the default argument promotions change. |
| `C4101` | A backslash-newline at the end of a file. |
| `C4102` | A multi-character character constant. (Its value is implementation-defined; see [`docs/c.md`](c.md).) |
| `C4103` | A `#warning` directive (C23, and the GNU dialects), or `#pragma GCC warning`; the message is its text. |
| `C4110` | `#pragma message`; the message is its text. |
| `C4111` | An unknown pragma (only with `--c-warn-unknown-pragmas`); it is ignored. |
| `C4112` | A known pragma that is malformed (`#pragma pack(3)`, ...); it is ignored. |
| `C4113` | A pragma that is not honoured: `#pragma STDC FENV_ROUND FE_TONEARESTFROMZERO` (no target has the direction), and `FENV_ROUND` of a directed rounding on WebAssembly (which rounds to nearest only). |
| `C4114` | `#pragma pack(show)`: the value in force. |
| `C4120` | An empty case range (`case 5 ... 3:`); it matches nothing. |
| `C4121` | A null pointer constant passed for a parameter declared `nonnull`. |
| `C4123` | A call of a function declared with the attribute `warning("message")`. |

`#pragma GCC diagnostic` can hide a warning, or make it an error (with its code)
([`docs/c.md`](c.md), "Pragmas").
