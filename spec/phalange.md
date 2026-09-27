# Phalange Language Specification

**Version:** 0.1-draft.3
**File extension:** `.ph`
**Status:** Working draft. Everything here may change. Decisions that differ from the
original design notes are listed in [Appendix C](#appendix-c-changes-from-the-design-notes),
and unresolved questions are in [Appendix D](#appendix-d-open-questions).

---

## Table of Contents

0. [Conventions](#0-conventions)
1. [Design Mandate](#1-design-mandate)
2. [Lexical Structure](#2-lexical-structure)
3. [Grammar](#3-grammar)
4. [Modules, Names and Scopes](#4-modules-names-and-scopes)
5. [Types](#5-types)
6. [Conversions](#6-conversions)
7. [Expressions](#7-expressions)
8. [Statements](#8-statements)
9. [Functions and Methods](#9-functions-and-methods)
10. [Generics](#10-generics)
11. [Errors](#11-errors)
12. [Memory Model](#12-memory-model)
13. [SIMD](#13-simd)
14. [Inline WebAssembly](#14-inline-webassembly)
15. [Targets and ABI](#15-targets-and-abi)
16. [Standard Library Core](#16-standard-library-core)
17. [Toolchain and Agent Diagnostic Interface](#17-toolchain-and-agent-diagnostic-interface)
18. [Complete Example](#18-complete-example)
- [Appendix A: Builtin Functions](#appendix-a-builtin-functions)
- [Appendix B: Trap Conditions](#appendix-b-trap-conditions)
- [Appendix C: Changes from the Design Notes](#appendix-c-changes-from-the-design-notes)
- [Appendix D: Open Questions](#appendix-d-open-questions)

---

## 0. Conventions

- **MUST**, **MUST NOT**, **SHOULD**, and **MAY** are used as in RFC 2119.
- A **compile error** means a conforming implementation rejects the program with a
  diagnostic. Every compile error has a stable code (§17.3).
- A **trap** ends execution immediately and deterministically. In WebAssembly it is the
  `unreachable` instruction or a native WASM trap. A trap is *defined behavior*: its
  cause and location are part of the program's meaning (§12.7, Appendix B).
- An **unspecified value** is some bit pattern of the correct type, chosen by the
  implementation and possibly different between runs. Reading one is **not** undefined
  behavior, and a compiler MUST NOT assume it never happens. (This is the difference from
  C's "indeterminate value" and LLVM's `poison`.)
- **Implementation-defined** behavior is documented by each implementation and fixed
  for a given target (for example, the width of `usize`).
- There is **no undefined behavior**. Every well-formed program has one meaning for each
  input, apart from the explicitly listed unspecified values and implementation-defined
  choices.

Grammar uses EBNF: `::=` is a definition, `|` is alternation, `?` means optional, `*` means
zero or more, `+` means one or more, `( )` groups, and `"x"` is a literal token.

---

## 1. Design Mandate

Phalange is a systems language whose main target is WebAssembly. It is designed for programs
that machine agents write, read and repair. Each principle below is paired with the
concrete rules that enforce it.

| Principle | Enforced by |
|---|---|
| **Zero hidden control flow** | No exceptions, destructors, operator overloading, implicit calls, getters, or implicit allocation. The only non-local exits are `return`, `break`, `continue`, postfix `!`, `catch … return`, and traps. `defer` is written out at the point where it is registered. |
| **Context-compact grammar** | The grammar is LL(1) and context-free. Parsing never needs a symbol table. There are no macros, no preprocessor, and no user-defined operators. Each construct has exactly one canonical surface form, enforced by `phalange fmt`. |
| **Zero undefined behavior** | Arithmetic, shifts, casts, and memory access all have defined results or traps. Memory is an abstract, bounds-checked linear byte array (§12). Compilers MUST NOT optimize on assumptions that the absence of UB would otherwise justify. |
| **First-class WASM/SIMD** | 128-bit vector types are primitives. Inline WebAssembly is structured, typed, and validated (§14). WASM semantics are the reference semantics. |
| **Arena-centric memory** | There is no global allocator. Every allocation names an `Allocator`. The standard library is built around arenas and `defer`. |

A secondary principle is **locality of reasoning**. The meaning of a line of code should
follow from that line and the declarations it names. This is why Phalange forbids shadowing
(§4.4), forbids silently discarding results (§8.2), and allows only the short, closed list
of implicit conversions in §6.1.

---

## 2. Lexical Structure

### 2.1 Source text

Source files are UTF-8. A byte-order mark is a compile error (`E0001`). Non-ASCII
characters may appear only inside comments and string literals. Line terminators are
`\n` or `\r\n`.

### 2.2 Whitespace and comments

Whitespace (space, tab, line terminators) separates tokens and is otherwise ignored.
Tabs are allowed, but `phalange fmt` emits spaces.

- `// …` runs to the end of the line.
- `/// …` is a *doc comment* and attaches to the declaration that follows it.
- There are no block comments. (One comment form makes source easy to scan and to strip.)

### 2.3 Identifiers

```
IDENT ::= [A-Za-z_][A-Za-z0-9_]*      (but not a keyword or reserved type name, and not "_")
```

A lone `_` is the **discard token** (§8.2). It is not an identifier.

### 2.4 Keywords

```
and       as        break     catch     const     continue  defer     else
error     export    false     fn        for       host      if        import
in        let       loop      match     mut       not       or        pub
return    Self      struct    true      union     wasm      while
```

**Reserved type names.** These lex as keywords:

```
u8  u16  u32  u64  usize   i8  i16  i32  i64  isize   f32  f64   bool  void
i8x16  u8x16  i16x8  u16x8  i32x4  u32x4  i64x2  u64x2  f32x4  f64x2  v128
```

**Reserved constraint names** (§10.5): `Eq`, `Ord`, `Num`, `Int`, `SignedInt`,
`UnsignedInt`, `Float`. These may not be declared as identifiers.

**Reserved for future use:** `async`, `await`, `comptime`, `enum`, `extern`, `impl`,
`interface`, `null`, `test`, `trait`, `type`, `unsafe`, `yield`.

### 2.5 Literals

```
INT_LIT    ::= DEC | "0x" HEX+ | "0b" BIN+ | "0o" OCT+     (digits may be separated by "_")
DEC        ::= [0-9] ( "_"? [0-9] )*
FLOAT_LIT  ::= DEC "." DEC ( [eE] [+-]? DEC )?  |  DEC [eE] [+-]? DEC
STRING_LIT ::= '"' ( CHAR | ESCAPE )* '"'
CHAR_LIT   ::= "'" ( ASCII_CHAR | ESCAPE ) "'"
ESCAPE     ::= "\n" | "\r" | "\t" | "\0" | "\\" | "\"" | "\'" | "\x" HEX HEX | "\u{" HEX+ "}"
```

- A float literal MUST have digits on both sides of the `.`. This means `0..n` lexes as
  `0` `..` `n`, with no special case needed.
- A character literal has type `u8` and must be ASCII (a `\u{…}` escape above `0x7F` is
  `E0004`).
- A string literal has type `[]u8`. Its bytes are the UTF-8 encoding of its contents,
  stored in static read-only data. It has **no** NUL terminator.
- A leading `-` is never part of a literal. It is the unary minus operator (§7.5).

### 2.6 Operators and punctuation

The lexer uses **maximal munch**. The complete token set is:

```
+   -   *   /   %   +%  -%  *%
&   |   ^   ~   <<  >>
==  !=  <   <=  >   >=
=   +=  -=  *=  /=  %=  +%= -%= *%= &=  |=  ^=  <<= >>=
!   .   ..  ,   :   ;   =>  ->  @
(   )   {   }   [   ]
```

Maximal munch means that `f()!=x` lexes as `f()` `!=` `x`. To propagate an error and then
compare, write `(f()!) == x` or add whitespace: `f()! == x`. The formatter always emits
the spaced form.

A **builtin name** `@` IDENT is a single token (for example `@splat`).

### 2.7 Inline-WASM lexical mode

After `wasm {` or `wasm -> Type {`, the lexer switches to *WASM mode* until the matching
`}` (§14.2). In WASM mode, tokens follow the WebAssembly text format: instruction names
like `i32.clz` and `v128.load`, `$`-prefixed names, integer and float immediates, and
`offset=`/`align=` immediates. Mode switching is purely lexical and needs no parser
feedback.

---

## 3. Grammar

The grammar is LL(1). Each alternative can be chosen from the next token alone. There is
one principled exception, which still needs no symbol table: builtin argument lists
(§3.6), where the builtin's fixed signature decides whether an argument position holds a
type or an expression. Builtins are a closed, language-defined set, so their signatures
are effectively part of the grammar.

### 3.1 Module structure

```ebnf
Module        ::= TopDecl* EOF

TopDecl       ::= ImportDecl
                | ErrorDecl
                | "pub"? Item
                | "export" FnDecl

Item          ::= StructDecl | UnionDecl | FnDecl | ConstDecl | GlobalDecl

ImportDecl    ::= "import" ( STRING_LIT "as" IDENT ";"
                           | "host" STRING_LIT? "{" HostFnDecl* "}" )
HostFnDecl    ::= "fn" IDENT "(" ParamList? ")" Type ";"

ErrorDecl     ::= "error" "{" IDENT ( "," IDENT )* ","? "}"

ConstDecl     ::= "const" IDENT ":" Type "=" Expr ";"
GlobalDecl    ::= "let" "mut"? IDENT ":" Type "=" Expr ";"
```

### 3.2 Aggregates and functions

```ebnf
StructDecl    ::= "struct" IDENT GenericParams? "{" FieldList? MethodDecl* "}"
FieldList     ::= Field ( "," Field )* ","?
Field         ::= IDENT ":" Type

UnionDecl     ::= "union" IDENT GenericParams? "{" Variant ( "," Variant )* ","? MethodDecl* "}"
Variant       ::= IDENT ( ":" Type )?

MethodDecl    ::= "pub"? FnDecl

FnDecl        ::= "fn" IDENT GenericParams? "(" ParamList? ")" Type Block
ParamList     ::= Param ( "," Param )* ","?
Param         ::= IDENT ":" Type

GenericParams ::= "[" GenericParam ( "," GenericParam )* ","? "]"
GenericParam  ::= IDENT ( ":" Constraint )? | "const" IDENT ":" Type
Constraint    ::= "Eq" | "Ord" | "Num" | "Int" | "SignedInt" | "UnsignedInt" | "Float"
```

The trailing-comma forms are LL(1). After a `,`, an `IDENT` begins another element, and
anything else ends the list. The FIRST sets of `Field` (`IDENT`) and `MethodDecl`
(`pub`, `fn`) are disjoint.

### 3.3 Types

```ebnf
Type          ::= "!" Type                          (* error union *)
                | "*" "mut"? Type                   (* raw pointer *)
                | "[" "]" "mut"? Type               (* slice *)
                | "[" ArrayLen "]" Type             (* array *)
                | "fn" "(" TypeList? ")" Type       (* function pointer *)
                | "struct" "{" FieldList? "}"       (* anonymous struct *)
                | "error"
                | "Self"
                | PrimType
                | NamedType

NamedType     ::= IDENT ( "." ( IDENT GenericArgs? | "[" GenericArgList "]" ) )?
GenericArgs   ::= "." "[" GenericArgList "]"
GenericArgList::= GenericArg ( "," GenericArg )* ","?
GenericArg    ::= Type | INT_LIT

ArrayLen      ::= INT_LIT | IDENT ( "." IDENT )?    (* must name a const *)
TypeList      ::= Type ( "," Type )* ","?
PrimType      ::= (any reserved type name, §2.4)
```

`NamedType` accepts `Name`, `module.Name`, `Name.[Args]` and `module.Name.[Args]`. After
the `.`, the next token (`IDENT` or `[`) decides which form it is.

### 3.4 Statements

```ebnf
Block         ::= "{" Stmt* "}"

Stmt          ::= LetStmt
                | IfStmt | WhileStmt | LoopStmt | ForStmt | MatchStmt
                | ReturnStmt | BreakStmt | ContinueStmt
                | DeferStmt | WasmStmt | DiscardStmt
                | Block
                | ExprStmt

LetStmt       ::= "let" "mut"? IDENT ( ":" Type )? "=" Expr ";"
ExprStmt      ::= Expr ( AssignOp Expr )? ";"
DiscardStmt   ::= "_" "=" Expr ";"
AssignOp      ::= "=" | "+=" | "-=" | "*=" | "/=" | "%=" | "+%=" | "-%=" | "*%="
                | "&=" | "|=" | "^=" | "<<=" | ">>="

IfStmt        ::= "if" Expr Block ( "else" ( IfStmt | Block ) )?
WhileStmt     ::= "while" Expr Block
LoopStmt      ::= "loop" Block
ForStmt       ::= "for" IDENT "in" Expr ( ".." Expr )? Block
MatchStmt     ::= "match" Expr "{" MatchArm* "}"
MatchArm      ::= Pattern "=>" Block
Pattern       ::= "." IDENT ( "(" IDENT ")" )?      (* union variant *)
                | "error" "." IDENT                 (* error value *)
                | "-"? INT_LIT | CHAR_LIT | "true" | "false"
                | "else"

ReturnStmt    ::= "return" Expr? ";"
BreakStmt     ::= "break" ";"
ContinueStmt  ::= "continue" ";"
DeferStmt     ::= "defer" ( Block | ExprStmt )
WasmStmt      ::= "wasm" WasmBody
```

Assignment and expression statements share one production because both begin with an
expression. Whether the left-hand side is assignable is checked afterwards (§7.2). This
is the standard LL(1) factoring.

A statement that starts with `wasm` is always a `WasmStmt`. The value-producing form
`wasm -> T { … }` (§3.5) may appear only where an expression is expected in a non-initial
position, for example after `return` or `=`.

### 3.5 Expressions

```ebnf
Expr          ::= BinExpr ( "catch" Handler )?
Handler       ::= ( "|" IDENT "|" )?
                  ( "return" Expr? | "break" | "continue" | Block | BinExpr )

BinExpr       ::= UnaryExpr ( BinOp UnaryExpr )*
BinOp         ::= "*" | "/" | "%" | "*%"
                | "+" | "-" | "+%" | "-%"
                | "<<" | ">>" | "&" | "|" | "^"
                | "==" | "!=" | "<" | "<=" | ">" | ">="
                | "and" | "or"

UnaryExpr     ::= ( "-" | "-%" | "~" | "not" | "&" "mut"? ) UnaryExpr
                | PostfixExpr

PostfixExpr   ::= PrimaryExpr PostfixOp*
PostfixOp     ::= "." ( IDENT | "*" | "{" FieldInits "}" | "[" GenericArgList "]" )
                | "(" ArgList? ")"
                | "[" IndexOrSlice "]"
                | "!"
IndexOrSlice  ::= Expr ( ".." Expr? )?
                | ".." Expr?

PrimaryExpr   ::= IDENT | "Self"
                | INT_LIT | FLOAT_LIT | STRING_LIT | CHAR_LIT | "true" | "false"
                | "(" Expr ")"
                | "." "{" FieldInits "}"           (* struct literal, type from context *)
                | "[" Expr ( ";" Expr | ( "," Expr )* ","? ) "]"   (* array literal *)
                | "error" "." IDENT
                | BuiltinCall
                | "wasm" "->" Type WasmBody

FieldInits    ::= ( FieldInit ( "," FieldInit )* ","? )?
FieldInit     ::= IDENT ":" Expr
ArgList       ::= Expr ( "," Expr )* ","?
```

`BinExpr` is parsed as a flat sequence, and the tree is then built by the partial-order
rules in §7.4. The flat parse is LL(1). Building the tree is a deterministic
post-pass that needs no symbol information.

**Why struct literals are `T.{ … }` and not `T { … }`.** A bare `IDENT {` is ambiguous in
positions such as `if x == Point { … }`. It is unclear whether the `{` starts a struct
literal or the `if` body. Rust resolves this with context-sensitive restrictions. Phalange
avoids the problem at the token level: a struct literal always begins with `.{`.

### 3.6 Builtin calls

```ebnf
BuiltinCall   ::= BUILTIN "(" BuiltinArgs? ")"
BuiltinArgs   ::= BuiltinArg ( "," BuiltinArg )* ","?
BuiltinArg    ::= Type | Expr        (* chosen by the builtin's signature, Appendix A *)
```

An unknown builtin name is a compile error (`E0120`) raised by the parser.

### 3.7 Inline WASM body

```ebnf
WasmBody      ::= "{" WasmInstr* "}"
WasmInstr     ::= WASM_OPCODE WasmImmediate*
WasmImmediate ::= "$" IDENT | INT_LIT | FLOAT_LIT | ("offset" | "align") "=" INT_LIT
                | "(" "result" WasmValType ")"
```

See §14 for the permitted instruction set and typing.

---

## 4. Modules, Names and Scopes

### 4.1 Modules

Each `.ph` file is a module. `import "path.ph" as name;` binds `name` to that module's `pub`
declarations. Paths are relative to the importing file. A path beginning with `std/`
resolves to the standard library, for example `import "std/mem" as mem;`.

Import cycles are a compile error (`E0210`) in v0.1.

### 4.2 Visibility

- `pub` makes a top-level item, or a method, visible to importing modules.
- Struct and union **fields** are visible wherever the type is visible. v0.1 has no field
  privacy.
- `export` makes a function a WebAssembly export (§15.2). An exported function is not
  automatically `pub`.

### 4.3 Declaration order

Top-level declarations may appear in any order. Name resolution has two steps: first
collect the names of all top-level items in all modules, then check function bodies in a
single pass. No forward declarations or header files are needed.

Inside a block, a local is visible from the statement after its `let` to the end of the
block.

### 4.4 No shadowing

A local variable or parameter MUST NOT have the same name as any of the following
(`E0204`):

- another local that is in scope,
- a parameter of the enclosing function,
- a top-level item of the current module,
- an import alias.

This rule removes a whole class of "which `x` is this?" misreadings. The diagnostic
suggests a fresh name.

### 4.5 `Self`

Inside a `struct` or `union` body, `Self` refers to the enclosing type, both as a type and
as an expression-level namespace (for example `Self.init(…)` or `Self.{ … }`).

---

## 5. Types

### 5.1 Integers

| Type | Size (bytes) | Range |
|---|---|---|
| `u8`, `i8` | 1 | 0…2⁸−1, −2⁷…2⁷−1 |
| `u16`, `i16` | 2 | 0…2¹⁶−1, −2¹⁵…2¹⁵−1 |
| `u32`, `i32` | 4 | 0…2³²−1, −2³¹…2³¹−1 |
| `u64`, `i64` | 8 | 0…2⁶⁴−1, −2⁶³…2⁶³−1 |
| `usize`, `isize` | target | 4 bytes on `wasm32`, 8 bytes on `wasm64` and 64-bit native |

Signed integers are two's complement. All integers are little-endian in memory.
Alignment equals size.

### 5.2 Floating point

`f32` and `f64` are IEEE 754-2019 binary32 and binary64 (§7.7 gives the arithmetic
rules). Alignment equals size.

### 5.3 `bool`

`bool` occupies one byte: `0` is `false` and `1` is `true`. Loading a `bool` from memory
whose byte is any other value yields `true` (a **normalizing load**), so a corrupted byte
cannot become UB.

### 5.4 `void`

`void` is the zero-size type. A function returning `void` produces no value. `*void` and
`*mut void` are opaque pointers that MUST be converted with `@ptr_cast` before they can be
dereferenced. `void` cannot be the type of a local, field or array element.

### 5.5 Arrays: `[N]T`

An array holds `N` contiguous `T` values. `N` is an integer literal or a `const` of an
integer type, and `N` ≥ 0. Arrays are **value types**: assignment and argument passing copy
all elements. Size is `N * size_of(T)` and alignment is `align_of(T)`. `a.len` is the
constant `N` (type `usize`).

### 5.6 Slices: `[]T` and `[]mut T`

A slice is a fat pointer `{ ptr: *T, len: usize }` with that exact layout (pointer first).
The fields `.ptr` and `.len` are readable but not assignable. Every index `s[i]` is
bounds-checked and traps when `i >= s.len` (§7.8). Through a `[]T`, elements are read-only.
Through a `[]mut T`, they are readable and writable.

### 5.7 Raw pointers: `*T` and `*mut T`

A raw pointer is an address in linear memory (§12). It has the same size as `usize`.
Pointers are **non-nullable in the type system**: the language has no `null` literal. The
only operations on pointers are:

- dereference: `p.*`, which is a place expression and is writable only for `*mut T`
- field access with one level of auto-deref: `p.field` means `p.*.field` (§7.9)
- method calls (§9.3)
- `==` and `!=`
- the conversion builtins `@int_from_ptr`, `@ptr_from_int`, `@ptr_cast`, and
  `@slice_from_ptr`

There is no pointer arithmetic operator. Write
`@ptr_from_int(*T, @int_from_ptr(p) + k)`, which states the byte offset explicitly.

### 5.8 SIMD vectors

| Type | Lanes × lane type |
|---|---|
| `i8x16`, `u8x16` | 16 × 8-bit |
| `i16x8`, `u16x8` | 8 × 16-bit |
| `i32x4`, `u32x4` | 4 × 32-bit |
| `i64x2`, `u64x2` | 2 × 64-bit |
| `f32x4` | 4 × f32 |
| `f64x2` | 2 × f64 |
| `v128` | 128 raw bits (no lane interpretation) |

All vector types are 16 bytes with 16-byte alignment. See §13.

### 5.9 Structs

```phalange
struct Vec2 {
    x: f32,
    y: f32,

    pub fn length_sq(self: Vec2) f32 {
        return (self.x * self.x) + (self.y * self.y);
    }
}
```

**Layout (deterministic).** Fields are laid out **in declaration order**. Each field's
offset is the previous field's end, rounded up to that field's alignment. The struct's
alignment is the largest field alignment, or 1 if there are no fields. Its size is the
end of the last field rounded up to the struct's alignment. The compiler MUST NOT reorder
fields or add padding beyond this rule. The contents of padding bytes are unspecified.

Anonymous struct types `struct { … }` follow the same rules. Two anonymous struct types are
the same type when their field names and field types match exactly, in the same order.

### 5.10 Tagged unions

```phalange
pub union Shape {
    Circle: struct { radius: f32 },
    Rect:   struct { width: f32, height: f32 },
    Empty,
}
```

A union value holds exactly one variant. A variant without a `: Type` carries no payload.
Unions are the only form of polymorphism. Phalange has no vtables, inheritance, or interfaces.

**Layout.**

1. Tag type: `u8` if the union has ≤ 256 variants, otherwise `u16`. Unions with more than
   65 536 variants are rejected.
2. Tag values are assigned `0, 1, 2, …` in declaration order.
3. The tag is at offset 0.
4. The payload is at offset `round_up(size_of(tag), A)`, where `A` is the largest payload
   alignment.
5. Size is `round_up(payload_offset + largest_payload_size, max(A, align_of(tag)))`.

If a union is inspected by `match` and its tag holds an out-of-range value, which can only
happen after writing through raw pointers, the program traps (`T0007`).

### 5.11 Function pointer types: `fn(T1, T2) R`

A function pointer is a value that refers to a top-level function or method. Phalange has no
closures and no captured environment. A named function used as a value has function-pointer
type. On WASM, the value is a table index of type `u32` (§15.3). Calling an invalid function
pointer traps (`T0009`), which follows WASM `call_indirect` semantics.

### 5.12 Error unions: `!T`

`!T` holds either a value of type `T` or a value of type `error` (§11). Its layout is exactly
`struct { code: u16, value: T }`. `code == 0` means success. When `code ≠ 0`, the contents
of `value` are unspecified.

### 5.13 The `error` type

`error` is a 16-bit code. Code 0 is reserved and never names an error. Every name declared
in any `error { … }` block in the program is assigned a distinct non-zero code (§11.1).

---

## 6. Conversions

### 6.1 Implicit conversions (the complete list)

The following are the **only** conversions the compiler performs without being asked:

1. **Untyped literal typing.** An integer, float or character literal takes the type the
   context expects. An integer literal's value must be exactly representable (`E0503`
   otherwise). A float literal is rounded to the nearest value of the target float type
   (ties to even), so `let x: f32 = 0.1;` is valid.
2. **Mutability weakening.** `[]mut T → []T` and `*mut T → *T`.
3. **Error-union wrapping.** A `T` or an `error` converts to `!T` in a `return` statement,
   an assignment, or an argument passed to a `!T` parameter.

If an untyped literal has no contextual type, the compiler reports an error (`E0502`,
"cannot infer type of literal") and suggests an annotation. For example, `let x = 0;` is an
error, while `let x: u32 = 0;` and `let x = n + 1;` (where `n: u32`) are fine.

### 6.2 Explicit conversions

All other conversions use builtins (Appendix A):

| Builtin | Meaning | Failure |
|---|---|---|
| `@as(T, x)` | Lossless numeric widening (for example `u8 → u32`, `i16 → i64`, `u32 → i64`, `f32 → f64`) | A conversion that can lose information is a compile error (`E0510`) |
| `@int_cast(T, x)` | Integer-to-integer conversion, checked at runtime | Traps if the value is out of range (`T0004`) |
| `@truncate(T, x)` | Integer-to-integer conversion keeping the low bits | Never fails |
| `@int_from_float(T, x)` | Truncates toward zero and **saturates** | Never fails. NaN becomes 0 (matches WASM `trunc_sat`) |
| `@float_from_int(T, x)` | Rounds to nearest, ties to even | Never fails |
| `@float_cast(T, x)` | `f64 → f32` with round-to-nearest-even | Never fails |
| `@bit_cast(T, x)` | Reinterprets bits. Sizes must match | Size mismatch is a compile error (`E0512`) |
| `@int_from_ptr(p)` | Address as `usize` | Never fails |
| `@ptr_from_int(P, n)` | `usize` to pointer type `P` | Never fails. Any address is valid to form |
| `@ptr_cast(P, p)` | Changes the pointee type | Never fails. Alignment is not semantic (§12.5) |

---

## 7. Expressions

### 7.1 Evaluation order

Evaluation is strictly **left to right**. This applies to:

- the operands of a binary operator, where the left is fully evaluated before the right,
- function and builtin arguments,
- field initializers, which are evaluated in written order (and must be written in
  declaration order, §7.10),
- array literal elements,
- the index expression, which is evaluated after the indexed expression.

For an assignment `lhs op= rhs`, the place `lhs` is evaluated first (including any index
bounds checks), then `rhs` is evaluated, and then the store happens.

### 7.2 Places

A **place** is an expression that denotes a memory location:

- a local or global variable,
- `p.*` where `p` is a pointer,
- `e.f` where `e` is a place or a pointer,
- `e[i]` where `e` is an array place, a slice, or a pointer to an array.

A place is **mutable** if it is any of the following:

- a `let mut` variable,
- a field or element of a mutable place,
- `p.*` or `p.f` where `p: *mut T`,
- `s[i]` where `s: []mut T`.

The left-hand side of an assignment must be a mutable place (`E0701`).

### 7.3 Operators

| Family | Operators | Operand types | Notes |
|---|---|---|---|
| **M**ultiplicative | `*` `/` `%` `*%` | int, float (`%` int only) | `/` and `%` trap on divisor 0 |
| **A**dditive | `+` `-` `+%` `-%` | int, float | `%`-suffixed forms wrap, int only |
| **S**hift | `<<` `>>` | int; the count may be any unsigned int | §7.6 |
| **B**itwise | `&` `\|` `^` | int, SIMD int, `v128` | each operator is its own family: `&`, `\|`, `^` |
| **C**omparison | `==` `!=` `<` `<=` `>` `>=` | int, float; `==`/`!=` also on bool, pointer, error, fn pointer | result `bool` |
| Logical-**and** | `and` | bool | short-circuit |
| Logical-**or** | `or` | bool | short-circuit |

Both operands of a binary operator must have the **same type**, after literal typing
(`E0412` otherwise). Phalange has no implicit widening, and mixed `u32 + u64` is an error.

Structs, unions, arrays and slices do not support `==`. Compare them field by field or
with `@mem_eq` (Appendix A).

### 7.4 Operator precedence: a partial order

Phalange does not use a total precedence table. Precedence is a **partial order**. The
relationships everyone agrees on are defined. Any other combination of operators must be
**parenthesized**, and otherwise it is a compile error (`E0150`, with a machine-applicable
fix that inserts the parentheses).

**Ordering relation** (read `X > Y` as "X binds tighter than Y"):

```
M > A > C
S > C,   & > C,   | > C,   ^ > C
C > and, C > or
```

The relation is transitive, so every arithmetic, shift, and bitwise operator binds
tighter than `and` and `or`. `and` and `or` are **unordered** with respect to each other.

**Associativity.**

- **M**, **A**, `&`, `|`, `^`, `and`, and `or` are left-associative within their own
  family. For example, `a - b + c` is `(a - b) + c`.
- **S** does not chain. `a << b << c` is `E0150`.
- **C** is non-associative. `a < b < c` is `E0150`.

**The rule.** Build the tree from the flat operator sequence by precedence climbing with
these levels: M = 5; A, S, `&`, `|`, `^` = 4; C = 3; `and` = 2; `or` = 1. Operators on the
same level are left-associative. Then, for every binary node whose unparenthesized operand
is itself a binary node, one of the following must hold:

- the operand's operator binds tighter than the node's operator in the partial order, or
- both operators are in the same chainable family.

If neither holds, the program is rejected with `E0150`.

| Expression | Result |
|---|---|
| `a + b * c` | `a + (b * c)` |
| `i < chunks * 4` | `i < (chunks * 4)` |
| `x > 0 and y > 0` | `(x > 0) and (y > 0)` |
| `a + b & c` | **E0150**: `+` and `&` are unordered |
| `x & mask == 0` | `(x & mask) == 0`. Unlike C, `&` binds tighter than `==`, so the classic C bug cannot occur |
| `a << 2 + 1` | **E0150**: `<<` and `+` are unordered |
| `a or b == c` | `a or (b == c)` |
| `a and b or c` | **E0150**: `and` and `or` are unordered (Appendix D, Q2) |
| `a < b == c` | **E0150** |

Unary operators bind tighter than all binary operators. Postfix operators bind tighter
than unary operators: `-a.b` is `-(a.b)`, and `&a[i]` is `&(a[i])`.

### 7.5 Unary operators

| Operator | Operand | Meaning |
|---|---|---|
| `-x` | signed int, float, SIMD | Negation. For integers, `-MIN` overflows (§7.6) |
| `-%x` | signed or unsigned int | Wrapping negation |
| `~x` | int, SIMD int, `v128` | Bitwise NOT |
| `not x` | bool | Logical NOT |
| `&x` | place | Address, of type `*T` |
| `&mut x` | mutable place | Address, of type `*mut T` |

Applying `-` to an unsigned type is a compile error (`E0520`). Use `-%` or subtract from 0.

### 7.6 Integer arithmetic semantics

**Overflow mode.** Each build has an *overflow mode*, chosen with a compiler flag (§17.1):

- `trap` (the default): if the mathematical result of `+`, `-`, `*`, unary `-`, or
  `MIN / -1` is not representable, the program traps (`T0002`).
- `saturate` (selected by `--release`): the result is clamped to the type's MIN or MAX.

The overflow mode is the **only** way build configuration can change a program's meaning.
Optimization level (`-O0`…`-O3`) MUST NOT change observable behavior. (See Appendix D, Q1.)

**Wrapping operators.** `+%`, `-%`, `*%` and unary `-%` compute the result modulo 2ⁿ in
every mode.

**Division.** `/` truncates toward zero. `%` gives a result with the sign of the dividend,
so `a == (a / b) * b + (a % b)` whenever `a / b` does not overflow. A divisor of 0 traps
(`T0003`) in **every** mode. `MIN % -1` is `0`.

**Shifts.** A shift has its mathematical meaning for **every** count, including counts at or
above the bit width. Let `x` have `n` bits and let the count be `k`:

- `x << k` is `x · 2ᵏ` reduced modulo `2ⁿ`: bits shifted out of the top are discarded.
  When `k ≥ n` the result is `0`. `<<` never traps in either overflow mode.
- For unsigned `x`, `x >> k` is `⌊x / 2ᵏ⌋`, so `k ≥ n` gives `0`.
- For signed `x`, `x >> k` is the arithmetic shift `⌊x / 2ᵏ⌋`, which rounds toward −∞. When
  `k ≥ n` the result is `0` if `x ≥ 0` and `-1` if `x < 0`.

For example, with `x: u32 = 0xFFFF_FFFF`, `x << 40` is `0` and `x >> 40` is `0`.

The count may be any **unsigned** integer type, independently of `x`'s type. This is the
one exception to the same-type rule in §7.3. An untyped literal count is typed as `u32`. A
signed count is a compile error (`E0525`); converting it explicitly makes the negative case
visible.

*Codegen note:* WebAssembly's `i32.shl` family takes the count modulo the width. When `k` is
not a constant known to be less than `n`, the compiler emits a compare-and-select. Raw shift
instructions inside inline WASM (§14) keep WebAssembly semantics.

**Constant evaluation.** In a `const` initializer or an array length, an overflow is a
compile error (`E0530`) in every mode.

### 7.7 Floating-point semantics

- IEEE 754 round-to-nearest-ties-to-even is used for all operations.
- Division by zero produces ±∞ or NaN, per IEEE. It does not trap.
- **No contraction.** `a * b + c` MUST NOT be fused into an FMA. Use `@fma` to request one.
- **No reassociation.** Algebraic rewrites that could change a result are forbidden.
  Phalange has no fast-math flag.
- **NaN payloads** of results are unspecified, as in WebAssembly §4.3.3. NaN-ness itself
  is deterministic.
- Comparisons follow IEEE: any comparison with NaN is `false`, except `!=`, which is `true`.
- `%` is not defined for floats. Use `@rem`.

### 7.8 Indexing and slicing

- `a[i]`: `a` is an array, a slice, or a pointer to an array. `i` must be `usize`
  (`E0540`). The program traps if `i >= len` (`T0001`).
- `a[lo..hi]`, `a[lo..]`, `a[..hi]`, `a[..]` produce a slice. The bounds are `usize`.
  An omitted `lo` is 0 and an omitted `hi` is `len`. The program traps unless
  `lo <= hi <= len` (`T0001`).
- The slice of an array must be taken from a place, and it points into that place. The
  slice is `[]mut T` if the place is mutable and `[]T` otherwise.
- Slicing a slice keeps its mutability.

### 7.9 Field access

`e.f` reads field `f`. If `e` has type `*T` or `*mut T` with `T` a struct, `e.f` means
`e.*.f`. This is the only automatic dereference in the language, and it goes exactly one
level deep. `e.len` and `e.ptr` on slices and `e.len` on arrays are built-in fields.

### 7.10 Struct literals

- `T.{ f1: e1, f2: e2 }` constructs a struct of type `T`.
- `.{ … }` constructs a struct whose type comes from the context (a declared `let` type, a
  parameter type, a return type, a field type, or a union payload). Without such a
  context it is `E0503`.
- **Every** field must be given, **in declaration order** (`E0410`, fixable). There are no
  default field values.

### 7.11 Union construction

- `U.Variant(payload)` constructs a variant that has a payload.
- `U.Variant` constructs a variant without one.
- The payload expression is typed by the variant, so `.{ … }` works:
  `Shape.Circle(.{ radius: 1.0 })`.

### 7.12 Error propagation `e!` and `catch`

`e!` requires `e: !T` and produces a `T`. If `e` holds an error, the enclosing function
runs its pending `defer`s (§8.8) and returns that error. The enclosing function must itself
return some `!U` (`E0610`).

`e catch H` requires `e: !T` and handles the error with `H`:

| Handler form | Result |
|---|---|
| `catch fallback` | If `e` holds an error, the result is `fallback`, which must have type `T` |
| `catch return x` / `catch break` / `catch continue` | Leaves the function or loop on error |
| `catch { … }` | The block MUST NOT complete normally: every path ends in `return`, `break`, `continue`, or `@trap()` (`E0612`) |
| `catch \|err\| …` | Binds the `error` value to `err` in the handler |

`catch` applies to the whole `BinExpr` on its left, so `a + f() catch 0` is `E0150`.
Write `a + (f() catch 0)`.

### 7.13 Calls

- `f(a, b)` calls a function or function pointer.
- `Module.f(…)` and `Type.f(…)` call qualified functions.
- `x.m(…)` is a method call (§9.3).
- The argument count and types must match exactly (§6.1 for the allowed conversions).

---

## 8. Statements

### 8.1 `let`

`let` introduces an immutable binding and `let mut` a mutable one. Every binding has an
initializer, so no variable is ever uninitialized. The type annotation may be omitted when
the initializer's type can be determined (§6.1).

### 8.2 Expression statements and discards

An expression statement's value must be `void`. Discarding any other value is a compile
error (`E0620`). To throw a value away deliberately, write `_ = f();`.

Discarding an error union with `_ =` is also rejected (`E0621`). Handle it with `!` or
`catch` first. This makes it impossible to ignore an error silently.

### 8.3 `if`

The condition must be `bool`; Phalange has no truthiness. Braces are required. `else if`
chains are written as `else if …`.

### 8.4 `while` and `loop`

`while cond { … }` evaluates `cond` before each iteration. `loop { … }` repeats until it
reaches a `break` or `return`.

### 8.5 `for`

`for` has two forms:

1. `for i in lo..hi { … }` evaluates `lo` then `hi` **once** before the loop. Both must have
   the same integer type, which becomes the type of `i`. The body runs for
   `i = lo, lo+1, …, hi−1`. If `lo >= hi`, the body never runs. Inside the body, `i` is
   immutable.
2. `for x in s { … }` evaluates `s` (an array place or a slice) once. The body runs with
   `x` bound to a copy of each element in order. No bounds check is emitted.

`break` and `continue` apply to the innermost loop. v0.1 has no loop labels.

### 8.6 `match`

The scrutinee of a `match` must be a union, an integer (character patterns are allowed
for `u8`), `bool`, or `error`.

- **Union**: each arm is `.Variant` or `.Variant(name)`. `name` binds a copy of the
  payload.
- **Integer**, `bool`, or `error`: each arm is a literal or `error.Name`.
- Arms are tested in order. Each pattern may appear at most once (`E0631`).
- The match must be **exhaustive** (`E0630`). A union match must list every variant or
  end with `else`. Integer and `error` matches always need `else`.
- An arm body is a block. Phalange has no fall-through.

```phalange
fn area(s: Shape) f32 {
    match s {
        .Circle(c) => { return 3.14159265 * c.radius * c.radius; }
        .Rect(r)   => { return r.width * r.height; }
        .Empty     => { return 0.0; }
    }
}
```

### 8.7 `return`, `break`, `continue`

- `return` with no value is allowed only in a `void` or `!void` function. Such functions
  may also finish by reaching the end of the body; for `!void`, that means success.
- A non-`void` function must `return` on every path (`E0601`). Control-flow analysis
  treats `match`, `if`/`else`, `loop` without `break`, and `@trap()` precisely.

### 8.8 `defer`

`defer S` registers `S` to run when control leaves the **enclosing block**. This includes
normal completion, `return`, `break`, `continue`, `e!` propagation, and `catch return`.

- Deferred statements run in **reverse** order of registration.
- `S` runs with the variable values as they are at exit time. Variables are referenced,
  not captured by value.
- `S` MUST NOT contain `return`, `break`, `continue` that exits `S`, `!`, or a `catch` with
  a leaving handler (`E0640`). A deferred statement cannot change where control goes.
- A `defer` inside a loop body runs at the end of **each** iteration.
- On a **trap**, deferred statements do **not** run.

### 8.9 `wasm`

See §14.

---

## 9. Functions and Methods

### 9.1 Declarations

```phalange
fn name(p1: T1, p2: T2) R { … }
```

- The return type is **mandatory**. Use `void` when there is no value.
- Parameters are immutable bindings. To modify one, copy it into a `let mut`.
- Arguments are passed **by value**, including structs and arrays. To share, pass a
  pointer or a slice.
- Recursion is allowed. Stack exhaustion traps (`T0008`).

### 9.2 Methods

A function declared inside a `struct` or `union` body is an **associated function**, called
as `Type.name(…)`. If its first parameter is named `self` and has type `Self`, `*Self`, or
`*mut Self`, it is also a **method**.

### 9.3 Method call desugaring

For `x.m(args)`, where `x` has type `T` or is a pointer to `T`, and `T.m` has a `self`
parameter of type `P`:

| `x` is | `P` is `Self` | `P` is `*Self` | `P` is `*mut Self` |
|---|---|---|---|
| a value `T` | `T.m(x, args)` | `T.m(&x, args)`: `x` must be a place | `T.m(&mut x, args)`: `x` must be a mutable place |
| `*T` | `T.m(x.*, args)` | `T.m(x, args)` | `E0702` |
| `*mut T` | `T.m(x.*, args)` | `T.m(x, args)` | `T.m(x, args)` |

This table is the whole of method-call sugar. Phalange has no multi-level auto-deref, no
auto-ref of temporaries, and no trait lookup.

### 9.4 Function pointers

Using a function name as a value (without a call) produces a function-pointer value.
Calling through it is written like any other call: `a.alloc_fn(a.ctx, n, 8)`. This is a
field access followed by a call, not a method call, because `alloc_fn` is a field.

---

## 10. Generics

### 10.1 Overview

Functions, structs and unions may take **compile-time parameters**. These are either type
parameters or integer constant parameters. Generics are **monomorphized**: each distinct
set of arguments produces its own specialized copy, as if the programmer had written that
copy out by hand. At run time there is no type information, boxing, vtable or dispatch.
Phalange has no compile-time code execution. Instantiation is pure substitution.

```phalange
fn max[T: Ord](a: T, b: T) T {
    if a > b {
        return a;
    }
    return b;
}

pub struct List[T] {
    items: []mut T,
    len: usize,

    pub fn init(alloc: Allocator, capacity: usize) !Self {
        let items = alloc.alloc.[T](capacity)!;
        return Self.{ items: items, len: 0 };
    }

    pub fn push(self: *mut Self, x: T) !void {
        if self.len == self.items.len {
            return error.OutOfMemory;
        }
        self.items[self.len] = x;
        self.len += 1;
    }

    pub fn get(self: *Self, i: usize) T {
        return self.items[..self.len][i];
    }
}

fn demo(alloc: Allocator) !void {
    let mut xs = List.[u32].init(alloc, 16)!;
    xs.push(7)!;
    let a = max(xs.get(0), 3);       // T deduced as u32 from the first argument
    let b = max.[f64](1.5, 2.0);     // explicit type argument
}
```

### 10.2 Generic parameters

```ebnf
GenericParams ::= "[" GenericParam ( "," GenericParam )* ","? "]"
GenericParam  ::= IDENT ( ":" Constraint )?        (* type parameter *)
                | "const" IDENT ":" Type           (* integer constant parameter *)
Constraint    ::= "Eq" | "Ord" | "Num" | "Int" | "SignedInt" | "UnsignedInt" | "Float"
```

A parameter list may follow the name of a `fn`, a `struct` or a `union`. The parameters
are in scope in the signature and the body, and also in every field, variant and method of
a generic type. A method of a generic type may declare its own additional parameters, as
in `fn map[U](…)`.

A `const` parameter must have an integer type. Inside the body it is a constant of that
type, so it can be used both as a value and as an array length. For example,
`fn sum[const N: usize](a: [N]f32) f32` has a parameter of type `[N]f32`.

### 10.3 Applying generic arguments: `Name.[Args]`

```ebnf
GenericArgs   ::= "." "[" GenericArg ( "," GenericArg )* ","? "]"
GenericArg    ::= Type | INT_LIT
```

Generic arguments are written the same way in **type and expression position**:

```phalange
let xs: List.[u32] = List.[u32].init(alloc, 16)!;
let p = max.[i64](a, b);
let v: Grid.[f32, 4] = …;
```

**Why `.[`.** In expression position, `Name[` already means indexing and `Name<` already
means less-than. The sequence `.[` is unused, so it keeps the grammar LL(1). It also joins
the existing "dot introduces a special postfix form" family (`.{`, `.*`). Using the same
spelling everywhere means there is one canonical form.

A constant argument is an integer literal or the name of a `const`, including an enclosing
`const` parameter. A name parses as a `Type` and is resolved to a constant during name
resolution, so the parser still needs no symbol table. Arbitrary expressions are not
allowed as arguments; declare a `const` for them.

Other rules:

- A generic type must always be fully applied. Writing a bare `List` as a type is
  `E0460`.
- Inside a generic type's body, `Self` is the instantiated type.
- A generic function used as a value must be instantiated first: `max.[u32]` is a
  function pointer, but `max` alone is `E0466`.
- `export fn` and host imports cannot be generic (`E0466`).

### 10.4 Argument deduction

When calling a generic **function**, the argument list may be omitted if every parameter can
be deduced from the argument types:

- Each argument type is matched **structurally** against its parameter type. No implicit
  conversions are applied during matching, with one exception: mutability weakening. For
  example, `[]T` matches `[]mut u32` with `T = u32`.
- Untyped literals do not contribute to deduction. If a parameter could only be deduced
  from literals, the program is rejected with `E0463`. For example, `max(1, 2)` is
  rejected; write `max.[u32](1, 2)`.
- Conflicting deductions are `E0464`. For example, `max(x_u32, y_u64)` deduces two
  different `T`s, and there is no widening to reconcile them.
- The return type and the surrounding context **never** influence deduction.
- Deduction is all-or-nothing: either every argument is given explicitly, or none is.
- For methods, the enclosing type's parameters come from the receiver's type. The method's
  own parameters are deduced from its arguments, or given explicitly:
  `alloc.alloc.[T](n)`.

Types (as opposed to functions) are never deduced. `List.[u32]` must always be written in
full.

### 10.5 Constraints

A generic body is **type-checked once, at its definition**, against the constraints of its
parameters. Any operation on a `T` that the constraint does not permit is a compile error in
the generic itself (`E0461`), even if no instantiation ever uses it. At an instantiation site,
the compiler checks only that the arguments satisfy the constraints (`E0462`).

This means that an error can never surface *inside* a library generic because of the type
argument a caller chose. This is a key property for agents: the error is always reported at
the line the agent wrote.

v0.1 has a closed set of builtin constraints. User-defined constraints are future work
(Appendix D).

| Constraint | Satisfied by | Additionally permits on values of `T` |
|---|---|---|
| *(none)* | any type except `void` | copy, assign, pass, return; `&`/`&mut`; using `T` in fields, arrays, slices and pointers; `@size_of(T)`, `@align_of(T)` |
| `Eq` | integers, floats, `bool`, pointers, `error`, function pointers | `==`, `!=` |
| `Ord` | integers, floats | everything `Eq` permits; `<` `<=` `>` `>=`; `@min`, `@max` |
| `Num` | integers, floats | everything `Ord` permits; binary `+` `-` `*` `/` |
| `Int` | all integer types | everything `Num` permits; `%`; `+%` `-%` `*%`; shifts; bitwise; `@clz` and similar; `@int_cast`/`@truncate` to or from `T`; `@min_value(T)`/`@max_value(T)` |
| `SignedInt` | `i8` … `i64`, `isize` | everything `Int` permits; unary `-`; `@abs` |
| `UnsignedInt` | `u8` … `u64`, `usize` | everything `Int` permits |
| `Float` | `f32`, `f64` | everything `Num` permits; unary `-`; `@sqrt`, `@floor` and the other float builtins |

**Literals in generic code.** An untyped literal is accepted as a value of type `T` only if
it is representable in **every** type that satisfies `T`'s constraint:

| Constraint | Literals accepted |
|---|---|
| `Num`, `Int` | integers `0` … `127` |
| `SignedInt` | integers `-128` … `127` |
| `UnsignedInt` | integers `0` … `255` |
| `Float` | any float literal, and integers exactly representable in `f32` |

For other values, convert explicitly, for example `@int_cast(T, @as(u32, 1000))`. This
conversion traps at run time if `T` cannot hold the value.

Semantics such as overflow behavior, NaN rules and shift behavior are those of the concrete
instantiated type.

### 10.6 Monomorphization

- **Identity.** Two instantiations are the same when all their arguments are identical:
  types by structural identity (§5), constants by value. `List.[u32]` written in two
  different modules is a single type.
- **Layout.** A generic type's layout is determined after substitution, by the rules in §5.
- **Code.** Each distinct instantiation of a function is emitted exactly once per program.
  Its debug and symbol name is its canonical spelling, for example `max.[u32]`.
- **Termination.** Instantiation nesting deeper than 64 levels is `E0465`. An example that
  would otherwise never terminate is `f.[T]` calling `f.[List.[T]]`.

### 10.7 Deliberately absent

Phalange generics have no specialization, overloading, variadic parameters, default
arguments, higher-kinded parameters, compile-time execution, or dynamic dispatch. There are
two ways to make code vary in behavior rather than in type:

- pass a function pointer explicitly, as in
  `fn sort[T](items: []mut T, less: fn(T, T) bool) void`, or
- dispatch over a tagged union with `match`.

---

## 11. Errors

### 11.1 Declaring errors

```phalange
error { OutOfMemory, DimensionMismatch, InvalidInput }
```

Error names form one **program-wide** set. A name may be declared in more than one module;
all declarations of the same name refer to the same error. Using `error.X` where `X` is not
declared anywhere in the program is `E0650`, which catches typos.

Codes are assigned deterministically: all declared names are sorted by byte value and
numbered starting at 1. `@error_name(e)` returns the name as a `[]u8`. For an unknown code
it returns `"<unknown>"`.

### 11.2 Returning and handling errors

```phalange
fn parse_digit(c: u8) !u32 {
    if (c < '0') or (c > '9') {
        return error.InvalidInput;
    }
    return @as(u32, c - '0');
}

fn sum_digits(s: []u8) !u32 {
    let mut total: u32 = 0;
    for c in s {
        total += parse_digit(c)!;
    }
    return total;
}
```

Errors are ordinary values. Phalange has no stack unwinding and no exception tables, and it
does not capture stack traces implicitly. (Debug builds MAY record error return traces,
§17.4.)

---

## 12. Memory Model

### 12.1 Abstract linear memory

All addressable data lives in a single **linear memory**: a byte array `M[0 .. size)` whose
size may grow at run time (`@memory_grow`). A pointer is an index into `M`. This is the
WebAssembly memory model, adopted as the semantics on **every** target.

- A **load** of `k` bytes at address `a` traps (`T0005`) if `a + k > size`. Otherwise it
  reads `M[a .. a+k)` as little-endian.
- A **store** is defined in the same way.
- Every address in bounds is readable and writable. Nothing is "invalid" beyond the bounds
  check.

Because of this:

- A **dangling pointer** (use after free, or after an arena reset) reads whatever bytes
  are currently at that address. The value is unspecified, but the behavior is defined.
  It is a logic bug, not UB.
- **Type punning** through `@ptr_cast` is defined: it reads the bytes as the new type.
  Loads of `bool` normalize (§5.3), and invalid union tags trap at `match` (§5.10).
- **Aliasing**: compilers MUST NOT assume that two pointers do not alias, whatever their
  types. There is no strict-aliasing rule and no implicit `restrict`.

### 12.2 Where values live

- **Globals** (`let`, `let mut` at module level) and **string literals** are in static
  regions of linear memory.
- **Locals whose address is never taken** have no address. They may live in WASM locals or
  registers, and no pointer can reach them.
- **Locals whose address is taken** (`&x`, `&mut x`, a slice of an array local, or an
  implicit `&` in a method call) live in a **shadow stack** in linear memory. After the
  function returns, their bytes may be reused. A pointer to them then reads unspecified
  bytes (defined, per §12.1). The compiler SHOULD warn when such a pointer escapes
  (`W0710`).

### 12.3 No uninitialized values

Every local and global has an initializer. Memory obtained from an allocator has
unspecified contents (§12.1) but is never "poison".

### 12.4 Allocation

Phalange has **no global allocator** and no implicit allocation. No language construct
allocates heap memory. Allocation always names an `Allocator` (§16.1):

```phalange
let buf = a.alloc.[u32](1024)!;   // []mut u32; a: Allocator
defer a.free(buf);                 // T deduced from the argument
```

### 12.5 Alignment

Alignment affects **layout only**. Misaligned loads and stores are defined, as in
WebAssembly, and may simply be slower. `@ptr_cast` and `@cast_slice` therefore never trap
on alignment. `@align_of(T)` reports the natural alignment used for layout.

### 12.6 Concurrency

v0.1 is single-threaded. Shared-memory threads and atomics are future work (Appendix D).

### 12.7 Traps

A trap stops the current WebAssembly instance, or the native process's Phalange context,
**immediately**:

- `defer` blocks do not run.
- The host observes the trap (in JavaScript, a `WebAssembly.RuntimeError`).
- In debug builds, the trap site maps to a trap code and source span (§17.4).

Appendix B lists every trap condition.

---

## 13. SIMD

### 13.1 Operators on vectors

| Operator | Float vectors | Integer vectors | `v128` |
|---|---|---|---|
| `+` `-` `*` | lane-wise IEEE | **not allowed**. Use `+%` `-%` `*%` (wrapping) or `@add_sat`/`@sub_sat` | no |
| `/` | lane-wise IEEE | no | no |
| `+%` `-%` `*%` | no | lane-wise wrapping (`*%` not for 8-bit lanes; WASM has no `i8x16.mul`) | no |
| `&` `\|` `^` `~` | no | yes | yes |
| `<<` `>>` | no | scalar unsigned count, with the §7.6 semantics applied per lane (`k ≥` lane width gives `0` or sign fill) | no |
| unary `-` | yes | `-%` only | no |
| `==` `<` … | no. Use `@lanes_eq`, `@lanes_lt`, … | no | no |

Requiring `+%` on integer vectors keeps §7.6 honest. Checking overflow on every lane would
be expensive, and silently wrapping under the `+` symbol would contradict the scalar rules.

Scalars are **never** broadcast implicitly. Write `@splat(x)`.

### 13.2 Vector builtins

Appendix A has full signatures.

- Construction: `@splat`, `@vec` (from lane values).
- Lanes: `@extract_lane`, `@replace_lane`. The lane index must be a constant expression.
- Memory: `@load(V, s)` / `@store(s, v)`. `s` is a slice, and the program traps if
  `s.len * size_of(elem) < 16` (`T0006`).
- Rearrangement: `@shuffle(a, b, [i0, …])` takes constant indices, and
  `@swizzle(a, idx)` takes runtime indices.
- Comparison: `@lanes_eq`, `@lanes_ne`, `@lanes_lt`, `@lanes_le`, `@lanes_gt`,
  `@lanes_ge`. Each returns an all-ones or all-zeros integer vector of the same shape.
- Selection and tests: `@select(mask, a, b)`, `@any_true`, `@all_true`, `@bitmask`.
- Reductions: `@reduce_add(v)` and `@dot(a, b)`. For floats, the association order is
  fixed as **pairwise**: `(l0 + l1) + (l2 + l3)` for 4 lanes and `l0 + l1` for 2 lanes. The
  result is therefore deterministic across targets.
- Lane-wise math: `@min`, `@max`, `@abs`, `@sqrt`, `@floor`, `@ceil`, `@trunc`,
  `@nearest`, `@fma`.

### 13.3 Native targets

On native targets, vector types lower to 128-bit registers (SSE2/AVX or NEON). The
semantics are the WASM SIMD semantics, including saturation behavior and NaN rules, except
that shift counts follow §7.6 rather than WASM's modulo rule. The compiler inserts fix-ups
wherever host instructions differ.

---

## 14. Inline WebAssembly

### 14.1 Forms

```phalange
// Statement form: stack effect [] -> []
wasm {
    nop
}

// Expression form: stack effect [] -> [T]
fn count_leading_zeros(val: u32) u32 {
    return wasm -> u32 {
        local.get $val
        i32.clz
    };
}
```

### 14.2 Semantics

An inline block is validated as a WebAssembly `block` with type `[] -> []` (statement form)
or `[] -> [W(T)]` (expression form). `W(T)` is the WASM value type of `T`, defined in the
table below. Validation errors are compile errors (`E09xx`) that point to the offending
instruction.

**Accessing Phalange variables.** `$name` names an in-scope Phalange local or parameter:

| Phalange type | WASM type | `local.get` | `local.set` / `local.tee` |
|---|---|---|---|
| `i32` `u32` `usize`/`isize` (wasm32) | `i32` | yes | yes, if `let mut` |
| `i64` `u64` `usize`/`isize` (wasm64) | `i64` | yes | yes, if `let mut` |
| `f32`, `f64` | `f32`, `f64` | yes | yes, if `let mut` |
| SIMD types | `v128` | yes | yes, if `let mut` |
| `*T`, `*mut T` | `i32`/`i64` | yes | yes, if `let mut` |
| `bool`, 8-bit and 16-bit ints | `i32` | yes | **no**, because writing an out-of-range value would break the type's invariant |
| structs, arrays, slices, unions | none | use `.field` in Phalange code before the block | no |

**Permitted instructions (v0.1).**

- all WASM 2.0 numeric instructions,
- fixed-width SIMD instructions (not relaxed SIMD),
- `nop`, `unreachable`, `drop`, `select`,
- `local.get`/`set`/`tee` on captured variables,
- loads and stores on memory 0 (these are bounds-trapped as usual),
- `memory.size`, `memory.fill`, `memory.copy`,
- structured `block`/`loop`/`if`/`else`/`end`, `br`, `br_if`, `br_table`, whose targets
  must be labels **inside** the inline block.

**Forbidden instructions** (`E0901`): `return`, `call`, `call_indirect`, `global.*`,
`table.*`, `memory.grow`, `memory.init`, `data.drop`, and any branch that targets a label
outside the block. These would create control flow or state changes that are invisible to
the surrounding Phalange code.

### 14.3 Portability

Inline WASM is **not** target-specific assembly. On native targets, the compiler translates
the block using WebAssembly semantics. An inline block therefore means the same thing on
every target, and is really a portable low-level IR escape hatch.

---

## 15. Targets and ABI

### 15.1 Targets

| Target | `usize` | Notes |
|---|---|---|
| `wasm32-freestanding` | 32 | Primary target. No WASI and no imports except those declared |
| `wasm32-wasi` | 32 | Standard library may use WASI imports |
| `wasm64-freestanding` | 64 | memory64 |
| native (`x86_64-*`, `aarch64-*`) | 64 | Linear memory is a reserved virtual region with guard pages and bounds checks (§12.1). Future work |

### 15.2 Exports

`export fn` produces a WebAssembly export with the same name. Parameter and return types
must be one of the following (`E0801` otherwise):

- `bool`,
- an integer type (8-bit and 16-bit types are passed as `i32`, zero- or sign-extended),
- `f32`, `f64`,
- `usize`/`isize`,
- a pointer,
- a SIMD type,
- `void` as the return type.

Exported functions MUST NOT return `!T`. Handle errors before the export boundary. The
module exports its linear memory as `"memory"`.

### 15.3 Imports

```phalange
import host "env" {
    fn console_log(ptr: *u8, len: usize) void;
}
```

This declares WASM function imports from module `"env"`, which is the default if the string
is omitted. They are called like normal functions. Imported functions use the same type
restrictions as exports.

### 15.4 Function tables

Every function used as a value is placed in table 0. A function pointer is its index
(`u32`). Calling through a pointer uses `call_indirect` with a signature check, so a bad
index or wrong signature traps (`T0009`).

---

## 16. Standard Library Core

Only the parts that the language semantics depend on are specified here.

### 16.1 `std/mem.Allocator`

```phalange
pub struct Allocator {
    ctx: *mut void,
    alloc_fn: fn(*mut void, usize, usize) !*mut u8,   // (ctx, size, align)
    free_fn:  fn(*mut void, *mut u8, usize) void,     // (ctx, ptr, size)

    pub fn alloc[T](self: Allocator, n: usize) ![]mut T {
        let bytes = @mul_checked(n, @size_of(T)) catch return error.OutOfMemory;
        let p = self.alloc_fn(self.ctx, bytes, @align_of(T))!;
        return @slice_from_ptr(@ptr_cast(*mut T, p), n);
    }

    pub fn free[T](self: Allocator, s: []mut T) void {
        self.free_fn(self.ctx, @ptr_cast(*mut u8, s.ptr), s.len * @size_of(T));
    }
}
```

These are ordinary generic methods (§10), not builtins. If the byte count overflows,
`alloc` returns `error.OutOfMemory` without calling the hook.

### 16.2 `std/mem.Arena`

```phalange
pub struct Arena {
    backing: Allocator,
    // … implementation fields …

    pub fn init(backing: Allocator) Arena;
    pub fn allocator(self: *mut Arena) Allocator;  // ctx points at *this* Arena
    pub fn reset(self: *mut Arena) void;           // O(1), keeps pages for reuse
    pub fn deinit(self: *mut Arena) void;          // returns all pages to `backing`
}
```

Typical use:

```phalange
import "std/mem" as mem;

fn work() !void {
    let mut arena = mem.Arena.init(mem.page_allocator);
    defer arena.deinit();

    let list = arena.allocator().alloc.[u32](1024)!;
    // … use list …
}
```

`arena.allocator()` stores a pointer to `arena`. If `arena` is copied, allocators obtained
from the old copy still point at the old location. The compiler SHOULD warn when an
`Arena` whose `allocator()` has been called is later moved (`W0711`).

### 16.3 `std/mem.page_allocator`

`page_allocator` is a global `Allocator` backed by `memory.grow` on WASM and by the linear
region reservation on native targets. On `wasm32-freestanding`, `free` is a no-op because
WASM memory cannot shrink.

### 16.4 `std/mem.FixedBuffer`

`FixedBuffer` is a bump allocator over a caller-provided `[]mut u8`. It never grows and
returns `error.OutOfMemory` when the buffer is full. It is intended for `static` arena
patterns like the example in §18.

---

## 17. Toolchain and Agent Diagnostic Interface

### 17.1 Commands

```
phalange build <file.ph> --target=<t> [-O0..-O3] [--release | --overflow=trap|saturate] -o <out>
phalange check <file.ph>                  # parse and type-check only; no codegen
phalange fmt   <file.ph> [--check]        # canonical formatting; --check exits 1 if not canonical
phalange run   <file.ph> [--export=name]  # build for wasm32 and execute in the reference runtime
phalange explain <CODE>                   # long-form documentation for a diagnostic code
```

Every command accepts `--format=human` (the default) or `--format=agent-json`.

- Compiler output is **deterministic**. The same inputs and flags produce byte-identical
  `.wasm` files and diagnostics.
- Diagnostics are sorted by file, then by start byte, then by code.

### 17.2 JSON output

With `--format=agent-json`, the command writes **one** JSON object to **stdout** and nothing
else. (Progress and log text, if any, go to stderr.) Exit codes are:

- `0`: success, possibly with warnings
- `1`: the program has errors
- `2`: invalid invocation or internal compiler error

```json
{
  "schema": "phalange-diagnostics/1",
  "status": "error",
  "diagnostics": [
    {
      "code": "E0412",
      "severity": "error",
      "message": "type mismatch in SIMD operation: expected f32x4, found f32",
      "file": "dot_product.ph",
      "span": {
        "start_line": 56, "start_col": 15, "end_line": 56, "end_col": 24,
        "start_byte": 1873, "end_byte": 1882
      },
      "context": "acc = acc + va;",
      "expected": "f32x4",
      "found": "f32",
      "notes": [
        {
          "message": "`va` declared here with type f32",
          "file": "dot_product.ph",
          "span": { "start_line": 54, "start_col": 13, "end_line": 54, "end_col": 15,
                    "start_byte": 1790, "end_byte": 1792 }
        }
      ],
      "fixes": [
        {
          "description": "broadcast the scalar to all lanes",
          "applicability": "machine",
          "edits": [
            { "file": "dot_product.ph",
              "start_byte": 1880, "end_byte": 1882,
              "replacement": "@splat(va)" }
          ]
        }
      ]
    }
  ]
}
```

**Field rules:**

| Field | Rule |
|---|---|
| `schema` | Versioned. Adding fields is non-breaking. Removing or renaming a field bumps the version |
| `status` | `"ok"` or `"error"` |
| `severity` | `"error"`, `"warning"`, or `"note"` |
| `span` | Lines and columns are **1-based**. Columns count Unicode scalar values. Byte offsets are **0-based** and `end_byte` is exclusive. Tools should prefer byte offsets |
| `expected` / `found` | Present for type errors. Types are printed in canonical source syntax |
| `fixes[].applicability` | `"machine"`: applying every edit yields a program without this diagnostic, and the fix does not change intended semantics. `"suggestion"`: plausible, needs review. `"placeholder"`: the edit contains `<…>` holes that must be filled in |
| `fixes[].edits` | Non-overlapping byte-range replacements, applied together atomically |

### 17.3 Diagnostic codes

Codes are stable. A code's meaning is never reused for something else.

| Range | Category |
|---|---|
| `E00xx` | Lexical |
| `E01xx` | Syntax (including `E0150` operator mixing) |
| `E02xx` | Names, modules, visibility, shadowing |
| `E03xx` | Declarations and items |
| `E04xx` | Types |
| `E05xx` | Literals, operators, conversions |
| `E06xx` | Control flow, returns, errors, discards |
| `E07xx` | Places, mutability, pointers |
| `E08xx` | ABI, exports, imports, SIMD, builtins |
| `E09xx` | Inline WASM |
| `Wxxxx` | Warnings, with ranges that mirror the `E` ranges |
| `Txxxx` | Runtime traps (Appendix B) |

Initial registry, not exhaustive:

| Code | Meaning |
|---|---|
| E0001 | Byte-order mark in source |
| E0004 | Invalid character literal |
| E0120 | Unknown builtin |
| E0150 | Operators require parentheses (§7.4) |
| E0204 | Shadowing |
| E0210 | Import cycle |
| E0410 | Struct literal fields missing or out of order |
| E0412 | Type mismatch |
| E0460 | Generic type used without arguments |
| E0461 | Operation not permitted by the parameter's constraint |
| E0462 | Generic argument does not satisfy the constraint |
| E0463 | Cannot deduce generic argument (only literals, or not in any parameter type) |
| E0464 | Conflicting deductions for a generic parameter |
| E0465 | Instantiation depth limit exceeded |
| E0466 | Generic used where a concrete function is required (value, `export`, import) |
| E0502 | Cannot infer type of literal |
| E0503 | Literal out of range for type, or untyped `.{}` with no context |
| E0510 | `@as` conversion is lossy |
| E0512 | `@bit_cast` size mismatch |
| E0520 | Unary `-` on unsigned type |
| E0525 | Signed shift count |
| E0530 | Overflow in constant evaluation |
| E0540 | Index is not `usize` |
| E0601 | Missing return |
| E0610 | `!` used in a function that does not return `!T` |
| E0612 | `catch` block completes normally |
| E0620 | Non-`void` result discarded |
| E0621 | Error union discarded with `_ =` |
| E0630 | Non-exhaustive `match` |
| E0631 | Duplicate `match` pattern |
| E0640 | Control flow leaves a `defer` |
| E0650 | Undeclared error name |
| E0701 | Assignment to an immutable place |
| E0702 | `*mut Self` method called through `*T` |
| E0801 | Type not allowed at the export or import boundary |
| E0901 | Forbidden instruction in inline WASM |

### 17.4 Runtime trap reporting

Debug builds (`--overflow=trap`, no `--strip`) SHOULD emit a custom WASM section,
`phalange.traps`, that maps each trap site's code offset to a trap code and source span.
`phalange run --format=agent-json` then reports a trap as a diagnostic with `severity:
"error"`, a `Txxxx` code, and the span of the failing expression. Debug builds MAY also
record an **error return trace**: the list of `!` and `return error.X` sites an error
passed through.

### 17.5 Canonical formatting

`phalange fmt` defines the single canonical layout. Its output is the same no matter how the
input was laid out, and running it twice changes nothing. Agents SHOULD run `phalange fmt`
before diffing or committing. Two programs that are equal after formatting are the same
program.

---

## 18. Complete Example

`examples/dot_product.ph`, the design-notes example updated to this specification:

```phalange
// =============================================================================
// File: dot_product.ph
// Build: phalange build dot_product.ph --target=wasm32-freestanding -O3 -o engine.wasm
// =============================================================================

import host "env" {
    fn console_log(ptr: *u8, len: usize) void;
}

error { OutOfMemory, DimensionMismatch }

// -----------------------------------------------------------------------------
// Minimal bump arena over a caller-provided buffer
// -----------------------------------------------------------------------------
pub struct Arena {
    memory: []mut u8,
    offset: usize,

    pub fn init(buffer: []mut u8) Arena {
        return Arena.{ memory: buffer, offset: 0 };
    }

    /// `align` must be a power of two.
    pub fn alloc(self: *mut Arena, size: usize, align: usize) ![]mut u8 {
        let mask = align - 1;
        let aligned = (self.offset + mask) & ~mask;

        if (aligned + size) > self.memory.len {
            return error.OutOfMemory;
        }

        let slice = self.memory[aligned..aligned + size];
        self.offset = aligned + size;
        return slice;
    }

    pub fn reset(self: *mut Arena) void {
        self.offset = 0;
    }
}

// -----------------------------------------------------------------------------
// SIMD dot product
// -----------------------------------------------------------------------------
pub fn dot_product_simd(a: []f32, b: []f32) !f32 {
    if a.len != b.len {
        return error.DimensionMismatch;
    }

    let n = a.len;
    let chunks = n / 4;
    let mut acc: f32x4 = @splat(0.0);

    let mut i: usize = 0;
    while i < chunks * 4 {
        let va = @load(f32x4, a[i..i + 4]);
        let vb = @load(f32x4, b[i..i + 4]);
        acc = acc + va * vb;
        i += 4;
    }

    // Pairwise horizontal sum: (l0 + l1) + (l2 + l3)
    let mut total = @reduce_add(acc);

    // Scalar tail
    while i < n {
        total += a[i] * b[i];
        i += 1;
    }

    return total;
}

// -----------------------------------------------------------------------------
// Exports
// -----------------------------------------------------------------------------
let mut HEAP_BYTES: [65536]u8 = [0; 65536];

export fn compute(length: usize) f32 {
    let mut arena = Arena.init(HEAP_BYTES[..]);
    defer arena.reset();

    let byte_size = length * @size_of(f32);
    let raw_a = arena.alloc(byte_size, 16) catch return -1.0;
    let raw_b = arena.alloc(byte_size, 16) catch return -1.0;

    let vec_a = @cast_slice(f32, raw_a);
    let vec_b = @cast_slice(f32, raw_b);

    for i in 0..length {
        vec_a[i] = 1.5;
        vec_b[i] = 2.0;
    }

    let result = dot_product_simd(vec_a, vec_b) catch return -2.0;

    let msg = "dot product computed";
    console_log(msg.ptr, msg.len);

    wasm {
        nop
    }

    return result;
}
```

---

## Appendix A: Builtin Functions

In the table, `T` and `V` denote **type** arguments. `S` denotes any integer type and `F`
any float type. `Vec` denotes any SIMD type. A **const** argument must be a constant
expression.

### A.1 Types and layout

| Builtin | Signature | Notes |
|---|---|---|
| `@size_of(T)` | `usize` | |
| `@align_of(T)` | `usize` | |
| `@offset_of(T, field)` | `usize` | `field` is an identifier |
| `@min_value(T)` / `@max_value(T)` | `T` | integer T |

### A.2 Conversions

See §6.2: `@as`, `@int_cast`, `@truncate`, `@int_from_float`, `@float_from_int`,
`@float_cast`, `@bit_cast`, `@int_from_ptr`, `@ptr_from_int`, `@ptr_cast`.

### A.3 Slices and memory

| Builtin | Signature | Notes |
|---|---|---|
| `@slice_from_ptr(p, n)` | `*T, usize → []T` (`*mut T → []mut T`) | No check at creation. Accesses are bounds-checked against linear memory |
| `@cast_slice(T, s)` | `[]U → []T` (keeps mutability) | Traps `T0006` if `s.len * size_of(U)` is not a multiple of `size_of(T)` |
| `@mem_copy(dst, src)` | `[]mut T, []T → void` | Traps `T0006` if lengths differ. Overlap is allowed (memmove semantics) |
| `@mem_set(dst, v)` | `[]mut T, T → void` | |
| `@mem_eq(a, b)` | `[]T, []T → bool` | Bytewise. Lengths may differ, in which case the result is `false` |
| `@memory_size()` | `→ usize` | In 64 KiB pages |
| `@memory_grow(pages)` | `usize → !usize` | Returns the previous size, or `error.OutOfMemory` |

### A.4 Scalar integer and float

| Builtin | Notes |
|---|---|
| `@clz(x)`, `@ctz(x)`, `@popcount(x)` | Return the same type as `x` |
| `@rotl(x, k)`, `@rotr(x, k)` | `k` is taken modulo the width |
| `@add_sat(a, b)`, `@sub_sat(a, b)`, `@mul_sat(a, b)` | Saturating, in every overflow mode. Also lane-wise on 8/16-bit integer vectors (add/sub only) |
| `@add_checked`, `@sub_checked`, `@mul_checked` | Returns `!T`, with `error.Overflow`, in every mode |
| `@min(a, b)`, `@max(a, b)` | Floats: a NaN operand gives NaN, and `-0.0 < +0.0` (WASM semantics) |
| `@abs(x)` | Integer `@abs(MIN)` overflows (§7.6) |
| `@sqrt`, `@floor`, `@ceil`, `@trunc`, `@nearest`, `@fma(a, b, c)`, `@rem(a, b)`, `@copysign(a, b)` | Floats |

### A.5 SIMD

| Builtin | Signature |
|---|---|
| `@splat(x)` | Scalar to `Vec`, with the type taken from context |
| `@vec(V, l0, l1, …)` | Construct a vector from exactly `lanes(V)` lane values |
| `@extract_lane(v, const i)` | `Vec → lane` |
| `@replace_lane(v, const i, x)` | `Vec → Vec` |
| `@load(V, s)` / `@store(s, v)` | Slice-checked 16-byte load/store (§13.2) |
| `@shuffle(a, b, const [i0, …])` | Two-input byte-granular or lane-granular shuffle. Indices must be in range (`E0820`) |
| `@swizzle(a, idx)` | `u8x16` runtime indices. Out-of-range indices give 0 (WASM semantics) |
| `@lanes_eq/ne/lt/le/gt/ge(a, b)` | Produce a mask vector |
| `@select(mask, a, b)` | Bitwise select |
| `@any_true(v)`, `@all_true(v)` | `→ bool` |
| `@bitmask(v)` | `→ u32`, one bit per lane |
| `@reduce_add(v)` | Pairwise order (§13.2). Integer lanes wrap |
| `@dot(a, b)` | Float vectors: `@reduce_add(a * b)`. `i16x8`: WASM `i32x4.dot_i16x8_s` |
| `@narrow`, `@widen_low`, `@widen_high` | Lane-width changes per WASM |

### A.6 Control

| Builtin | Notes |
|---|---|
| `@trap()` | Diverges and traps with `T0000`. Usable where any type is expected |
| `@error_name(e)` | `error → []u8` |

---

## Appendix B: Trap Conditions

This is the complete list of runtime traps. Each has a stable code.

| Code | Condition |
|---|---|
| `T0000` | `@trap()` or WASM `unreachable` |
| `T0001` | Index or slice bounds violation |
| `T0002` | Integer overflow (in `trap` overflow mode) |
| `T0003` | Integer division or remainder by zero |
| `T0004` | `@int_cast` value out of range |
| `T0005` | Linear-memory access out of bounds |
| `T0006` | Length precondition failed (`@load`, `@store`, `@cast_slice`, `@mem_copy`) |
| `T0007` | Invalid union tag at `match` |
| `T0008` | Stack exhaustion |
| `T0009` | Invalid function pointer or signature mismatch (`call_indirect`) |

Nothing else traps, and nothing is undefined.

---

## Appendix C: Changes from the Design Notes

These are the places where this draft departs from, or tightens, the original design notes.
Each change was made to resolve an inconsistency or to make a principle enforceable.

| # | Change | Reason |
|---|---|---|
| C1 | Struct literals are written `T.{ … }` and `.{ … }`, not `T { … }` | `IDENT {` is not LL(1) after `if`/`while`/`match` conditions |
| C2 | Operator precedence is a **partial order**. Mixing unordered operators needs parentheses (§7.4) | The notes say both "no precedence ambiguity" and use a flat `PrimaryExpr (BinaryOp PrimaryExpr)*`. The notes' own code (`aligned + size > len`, `i < chunks * 4`) needs *some* precedence. The partial order gives the unambiguous cases and rejects the rest |
| C3 | Logical operators are the keywords `and`/`or`/`not` | Frees `!` for error propagation (`e!`) and error-union types (`!T`) without lexical ambiguity |
| C4 | Assignment and expression statements are merged into one production | `AssignStmt` and `ExprStmt` share a FIRST set, so they are not LL(1) as written |
| C5 | Return types are mandatory (`void` for none) | One canonical form. The notes' examples already wrote `void` |
| C6 | Function-pointer types list parameter types only: `fn(*mut void, usize) T` | `fn(ctx: …)` needs two-token lookahead to tell `name:` from a type name |
| C7 | Inline WASM has typed forms: `wasm { }` (`[]→[]`) and `wasm -> T { }` (`[]→[T]`) used as an expression | The notes' `count_leading_zeros` relied on an implicit fall-through return value, which is hidden control flow |
| C8 | `@load_unaligned(f32x4, &a[i])` became `@load(f32x4, a[i..i + 4])` | The original only bounds-checked element `i` and then read 16 bytes, which is a bounds-check hole. Alignment is not semantic (§12.5), so `_unaligned` is redundant |
| C9 | `arena.allocator().alloc(u32, 1024)` became `arena.allocator().alloc.[u32](1024)` | Type arguments are written `.[…]` (§10.3). A type passed as an ordinary argument would need types to be parseable as expressions |
| C10 | `&mut HEAP_BYTES` passed as a slice became `HEAP_BYTES[..]` | Array-pointer-to-slice would be an implicit conversion |
| C11 | `pub` (module visibility) and `export` (WASM export) are distinct. Top-level `let`/`let mut` globals and `error { }` declarations were added to the grammar | The notes used these without grammar support |
| C12 | Integer SIMD vectors require `+%`/`-%`/`*%`. Bare `+` is float-only | Keeps §7.6 consistent. Lane-wise overflow trapping is impractical |
| C13 | Raw pointers are fully defined through the linear-memory model (§12) | "Zero UB" and "raw pointers" conflict unless memory itself has defined semantics. WASM's model provides this, and it is adopted on all targets |
| C14 | Error names must be declared (`error { … }`) | Prevents typos in `error.X` from silently creating new errors |
| C15 | Added `for`, `loop`, compound assignment, `_ =` discards, and a no-shadowing rule | Needed by the notes' examples, or by the agent-ergonomics goal |
| C16 | The diagnostic `suggested_fix.confidence` float became `fixes[].applicability` (an enum) plus byte-range `edits` | A float confidence is hard to act on. Byte-range edits can be applied mechanically |
| C17 | Horizontal float reductions have a fixed pairwise association order | Determinism across WASM and native |
| C18 | Shift counts are **not** clamped. `x << k` and `x >> k` have their mathematical meaning for every `k`, so `k ≥ width` gives `0` or sign fill | Design review, draft 2. `x << 40` has a clear meaning, and clamping it to `x << 31` would be surprising |
| C19 | Added monomorphized generics with builtin constraints (§10) | Design review, draft 2 |
| C20 | The language was renamed from Axiom to Phalange, and the extension from `.ax` to `.ph` | Design review, draft 2 |
| C21 | Float literals round to the nearest value of their target type (formerly open question Q5) | The spec's own examples (`3.14159265` as `f32`) need it; found while implementing the compiler |

---

## Appendix D: Open Questions

Each question is followed by the options and the current leaning.

1. **Integer overflow.** This is undecided. The current text (§7.6) traps in the default
   mode and saturates under `--release`. The options are:

   | Option | Pros | Cons |
   |---|---|---|
   | **(a)** Trap in debug, saturate in release (current) | Release builds never trap on overflow | Tested and shipped programs can differ. Saturation silently gives wrong answers (for example, a length stuck at `MAX`) |
   | **(b)** Trap in every mode, plus explicit `+\|` `-\|` `*\|` saturating operators next to `+%` | One meaning everywhere. Bugs are loud. The chosen behavior is visible at each use | A check on every operation. WASM has no overflow flag, so a check costs an extra compare and branch, and multiplication needs a wider multiply |
   | **(c)** Wrap in every mode (Go, Java) | Cheapest. Deterministic | Overflow bugs are silent. `+%` becomes redundant |
   | **(d)** Trap in every mode, with a per-module or per-function attribute to opt into wrapping | Keeps (b)'s semantics with a performance escape hatch | Adds an attribute syntax that v0.1 otherwise does not have |

   *Leaning:* (b). Measuring the cost of the checks on real WASM engines should settle it.
2. **`and`/`or` mixing.** Should `a and b or c` be allowed as `(a and b) or c`? *Leaning:*
   no, keep requiring parentheses.
3. **Optionals.** This is undecided. Generics make an optional type possible as a library
   type, so the question is how much language support it gets:

   | Option | Notes |
   |---|---|
   | **(a)** Library only: `union Option[T] { Some: T, None }` | No new syntax. Unwrapping is a `match`. `Option.[*T]` is 8 bytes on wasm32 (tag plus pointer), where a nullable pointer would be 4 |
   | **(b)** `?T` sugar for `Option.[T]` with an `orelse` operator | Shorter to write. Could use a niche layout for `?*T` (address 0 means "none"), but that needs address 0 to be reserved and `@ptr_from_int(0)` to be banned for `*T` |
   | **(c)** No optionals; use sentinel values or `!T` | Simplest, but makes "absent" and "failed" look alike |

   *Leaning:* (a) now, then revisit (b) once there is real code to measure against.
4. **Error sets.** A single program-wide error set is simple, but it does not let a function
   say *which* errors it can return. Consider `fn f() FileError!T` with an inferred or
   declared set, and exhaustive `match` over it.
5. ~~**Float literal exactness.**~~ Resolved: float literals round to nearest (§6.1, C21).
6. **`errdefer`.** Useful for cleaning up partial allocations when a function fails.
   *Leaning:* add it.
7. **User-defined constraints.** v0.1 constraints are a closed builtin set (§10.5). Generic
   containers such as a hash map currently need their hash and equality functions passed
   as function pointers, which costs an indirect call. Options include static interfaces,
   meaning named sets of required functions resolved at monomorphization time (not
   vtables), or passing functions as `const` generic parameters.
8. **Generics over SIMD.** Should there be constraints like `FloatVec` or `IntVec`, or a
   `Vec[T, const N]` family, so that kernels can be written once for `f32x4` and `f64x2`?
9. **Threads and atomics.** *Leaning:* sequentially consistent atomics only. Data races on
   non-atomic locations produce unspecified values (not UB), matching the WASM threads
   proposal.
10. **Native-target pointers and FFI.** The linear-memory model makes calling C code awkward.
    An `extern` boundary with explicit marshalling may be needed.
11. **Test blocks.** A `test "name" { … }` construct with `phalange test --format=agent-json`.
12. **Loop labels.** v0.1 has none.
13. **Relaxed SIMD.** Allow it in inline WASM with implementation-defined results, or forbid
    it (the current rule)?
