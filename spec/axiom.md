# Axiom Language Specification

**Version:** 0.1-draft.9
**File extension:** `.ax`
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
14. [Targets and ABI](#14-targets-and-abi)
15. [Standard Library Core](#15-standard-library-core)
16. [Toolchain and Agent Diagnostic Interface](#16-toolchain-and-agent-diagnostic-interface)
17. [Testing](#17-testing)
18. [Complete Example](#18-complete-example)
- [Appendix A: Builtin Functions](#appendix-a-builtin-functions)
- [Appendix B: Trap Conditions](#appendix-b-trap-conditions)
- [Appendix C: Changes from the Design Notes](#appendix-c-changes-from-the-design-notes)
- [Appendix D: Open Questions](#appendix-d-open-questions)

---

## 0. Conventions

- **MUST**, **MUST NOT**, **SHOULD**, and **MAY** are used as in RFC 2119.
- A **compile error** means a conforming implementation rejects the program with a
  diagnostic. Every compile error has a stable code (§16.3).
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

Axiom is a systems language whose main target is WebAssembly. It is designed for programs
that machine agents write, read and repair. Each principle below is paired with the
concrete rules that enforce it.

| Principle | Enforced by |
|---|---|
| **Zero hidden control flow** | No exceptions, destructors, operator overloading, implicit calls, getters, or implicit allocation. The only non-local exits are `return`, `break`, `continue`, postfix `!`, `catch … return`, and traps. `defer` is written out at the point where it is registered. |
| **Context-compact grammar** | The grammar is LL(1) and context-free. Parsing never needs a symbol table. There are no macros, no preprocessor, and no user-defined operators. Each construct has exactly one canonical surface form, enforced by `axiom fmt`. |
| **Zero undefined behavior** | Arithmetic, shifts, casts, and memory access all have defined results or traps. Memory is an abstract, bounds-checked linear byte array (§12). Compilers MUST NOT optimize on assumptions that the absence of UB would otherwise justify. |
| **First-class WASM/SIMD** | 128-bit vector types are primitives. WASM semantics are the reference semantics, on every target. |
| **Explicit allocation** | No language construct allocates. Heap memory comes from one global allocator, `std/mem` (§15.1), through calls you can see (`mem.alloc`, `mem.free`); `defer` releases it. |

A secondary principle is **locality of reasoning**. The meaning of a line of code should
follow from that line and the declarations it names. This is why Axiom forbids shadowing
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
Tabs are allowed, but `axiom fmt` emits spaces.

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
enum      error     export    extern    false     fn        for       host
if        import    in        let       loop      match     mut       not
or        pub       return    Self      struct    test      true      union
while
```

`threadlocal` is not a keyword. It is an identifier that has its special meaning
only at the start of a module-level `let` (§12.8.6).

**Reserved type names.** These lex as keywords:

```
u8  u16  u32  u64  usize   i8  i16  i32  i64  isize   f32  f64   bool  void
i8x16  u8x16  i16x8  u16x8  i32x4  u32x4  i64x2  u64x2  f32x4  f64x2  v128
```

**Reserved constraint names** (§10.5): `Eq`, `Ord`, `Num`, `Int`, `SignedInt`,
`UnsignedInt`, `Float`. These may not be declared as identifiers.

**Reserved for future use:** `async`, `await`, `comptime`, `impl`, `interface`,
`null`, `trait`, `type`, `unsafe`, `yield`.

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
  stored in static read-only data. The slice does not include a terminator, but a NUL
  byte always follows the literal in memory, so `"text".ptr` is a valid C string
  (§14.5).
- A leading `-` is never part of a literal. It is the unary minus operator (§7.5).

### 2.6 Operators and punctuation

The lexer uses **maximal munch**. The complete token set is:

```
+   -   *   /   %   +%  -%  *%
&   |   ^   ~   <<  >>
==  !=  <   <=  >   >=
=   +=  -=  *=  /=  %=  +%= -%= *%= &=  |=  ^=  <<= >>=
!   .   ..  ... ,   :   ;   =>  @
(   )   {   }   [   ]
```

Maximal munch means that `f()!=x` lexes as `f()` `!=` `x`. To propagate an error and then
compare, write `(f()!) == x` or add whitespace: `f()! == x`. The formatter always emits
the spaced form.

A **builtin name** `@` IDENT is a single token (for example `@splat`).

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
                | "pub"? ExternDecl
                | "export" FnDecl

Item          ::= StructDecl | UnionDecl | EnumDecl | FnDecl | ConstDecl | GlobalDecl

ImportDecl    ::= "import" ( STRING_LIT "as" IDENT ";"
                           | "host" STRING_LIT? "{" HostFnDecl* "}" )
HostFnDecl    ::= "fn" IDENT "(" ParamList? ")" Type ";"

ExternDecl    ::= "extern" STRING_LIT STRING_LIT? "{" ExternFnDecl* "}"   (* §14.5 *)
ExternFnDecl  ::= "fn" IDENT "(" ExternParams? ")" Type ";"
ExternParams  ::= Param ( "," Param )* ( "," "..." )?

ErrorDecl     ::= "error" "{" IDENT ( "," IDENT )* ","? "}"

ConstDecl     ::= "const" IDENT ":" Type "=" Expr ";"
GlobalDecl    ::= "threadlocal"? "let" "mut"? IDENT ":" Type "=" Expr ";"
```

### 3.2 Aggregates and functions

```ebnf
StructDecl    ::= "struct" IDENT GenericParams? "{" FieldList? MethodDecl* "}"
FieldList     ::= Field ( "," Field )* ","?
Field         ::= IDENT ":" Type

UnionDecl     ::= "union" IDENT GenericParams? "{" Variant ( "," Variant )* ","? MethodDecl* "}"
Variant       ::= IDENT ( ":" Type )?

EnumDecl      ::= "enum" IDENT ( ":" PrimType )? "{" Member ( "," Member )* ","? MethodDecl* "}"
Member        ::= IDENT ( "=" "-"? ( INT_LIT | CHAR_LIT ) )?

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
                | DeferStmt | DiscardStmt
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
```

Assignment and expression statements share one production because both begin with an
expression. Whether the left-hand side is assignable is checked afterwards (§7.2). This
is the standard LL(1) factoring.

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

FieldInits    ::= ( FieldInit ( "," FieldInit )* ","? )?
FieldInit     ::= IDENT ":" Expr
ArgList       ::= Expr ( "," Expr )* ","?
```

`BinExpr` is parsed as a flat sequence, and the tree is then built by the partial-order
rules in §7.4. The flat parse is LL(1). Building the tree is a deterministic
post-pass that needs no symbol information.

**Why struct literals are `T.{ … }` and not `T { … }`.** A bare `IDENT {` is ambiguous in
positions such as `if x == Point { … }`. It is unclear whether the `{` starts a struct
literal or the `if` body. Rust resolves this with context-sensitive restrictions. Axiom
avoids the problem at the token level: a struct literal always begins with `.{`.

### 3.6 Builtin calls

```ebnf
BuiltinCall   ::= BUILTIN "(" BuiltinArgs? ")"
BuiltinArgs   ::= BuiltinArg ( "," BuiltinArg )* ","?
BuiltinArg    ::= Type | Expr        (* chosen by the builtin's signature, Appendix A *)
```

An unknown builtin name is a compile error (`E0120`) raised by the parser.

---

## 4. Modules, Names and Scopes

### 4.1 Modules

Each `.ax` file is a module. `import "path.ax" as name;` binds `name` to that module's `pub`
declarations. Paths are relative to the importing file. A path beginning with `std/`
resolves to the standard library, for example `import "std/mem" as mem;`. A path beginning
with `pkg:` names a module of another package (§4.6). The `.ax` extension may be left out.
A path that doesn't resolve to a readable file is `E0211`.

Import cycles are a compile error (`E0210`) in v0.1.

### 4.2 Visibility

- `pub` makes a top-level item, or a method, visible to importing modules.
- Struct and union **fields** are visible wherever the type is visible. v0.1 has no field
  privacy.
- `export` makes a function a WebAssembly export (§14.2). An exported function is not
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

### 4.6 Packages

A **package** is a directory with a `axiom.toml` manifest and its sources under `src/`.
Packages are the unit of distribution and of dependency management. Modules stay the unit
of naming and visibility: a package adds no new visibility level, and whatever a
package's modules mark `pub` is visible to modules that import them.

```
geometry/
  axiom.toml
  src/
    lib.ax            # entry module: import "pkg:geometry"
    shapes/circle.ax  # import "pkg:geometry/shapes/circle"
```

`axiom new <name>` creates this layout, with a manifest, an entry module and a test.

#### 4.6.1 The manifest

The manifest is written in a small subset of TOML: `[section]` headers, `key = "string"`
pairs, inline tables `{ key = "string", ... }`, and `#` comments. No other TOML syntax
is accepted.

```toml
[package]
name = "geometry"          # required: lower-case letters, digits and '_', not starting with a digit
version = "0.1.0"          # optional; informational in v0.1
entry = "src/lib.ax"       # optional; default src/lib.ax if it exists, otherwise src/main.ax

[dependencies]
vecmath = { path = "../vecmath" }
json = { git = "https://example.com/json.git", rev = "3f1c2a9d0e5b7c4a8f6e2d1b0c9a8e7f6d5c4b3a" }
```

- An unknown section, an unknown key, a malformed line, an invalid name, or a duplicate
  dependency is `E0215`.
- A dependency's key is the name it is imported by. It MUST equal the `name` in the
  dependency's own manifest (`E0215`). Packages can't be renamed on import.
- A **path** dependency is a directory, relative to the manifest's directory. A missing
  directory, or one without a manifest, is `E0217`.
- A **git** dependency MUST pin a commit with `rev` (`E0215`). Branches and version ranges
  aren't accepted, so a manifest names exactly one version of each source and builds are
  reproducible without a lock file. The toolchain clones the repository once per
  `(url, rev)` into a cache directory: `$AXIOM_CACHE`, else
  `$XDG_CACHE_HOME/axiom`, else `~/.cache/axiom`. A clone or checkout that fails is
  `E0216`. A cached checkout is never updated in place, because its `rev` can't change.

#### 4.6.2 Imports and visibility between packages

- `import "pkg:name" as x;` imports package `name`'s entry module.
  `import "pkg:name/sub/mod" as y;` imports `src/sub/mod.ax` of that package.
- A module may import `pkg:` names only for its **own package** and the package's
  **direct dependencies**. Anything else is `E0214`, including a dependency of a
  dependency, and a `pkg:` import from a file that isn't in any package. A dependency
  that's needed must be listed, so each manifest is a complete list of what the package
  uses.
- A module belongs to the package whose manifest is in the nearest directory above it.
- Relative and `std/` imports work as in §4.1, inside packages too.

#### 4.6.3 One source per name

A build uses **at most one package per name** (`E0213`). If two dependencies (directly
or through other packages) bring in different directories or commits that are both
called `util`, the build fails and names both manifests. v0.1 has no version resolution:
the author settles the conflict by pointing every manifest at the same source.

Dependency cycles between packages are allowed by the manifest rules, but an import
cycle between their modules is still `E0210`.

#### 4.6.4 Building and testing packages

- `axiom build|check|test <dir>`, or no path at all (meaning the current directory),
  works on the package in `<dir>`. `build` compiles the entry module. By default the
  output is named after the package.
- `axiom test <dir>` loads **every** `.ax` file under the package's `src/`, so tests in
  modules that the entry module doesn't import still run. Only the root package's tests
  run; its dependencies' tests don't (§17).
- Modules, not packages, are the unit of compilation, and they are compiled and cached
  separately (§16.7). Generics monomorphize across packages as they do within one.
  Distributing precompiled packages is future work (Appendix D).

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

```axiom
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

```axiom
pub union Shape {
    Circle: struct { radius: f32 },
    Rect:   struct { width: f32, height: f32 },
    Empty,
}
```

A union value holds exactly one variant. A variant without a `: Type` carries no payload.
Unions are the only form of polymorphism. Axiom has no vtables, inheritance, or interfaces.

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

A function pointer is a value that refers to a top-level function or method. Axiom has no
closures and no captured environment. A named function used as a value has function-pointer
type. On WASM, the value is a table index of type `u32` (§14.3). Calling an invalid function
pointer traps (`T0009`), which follows WASM `call_indirect` semantics.

### 5.12 Error unions: `!T`

`!T` holds either a value of type `T` or a value of type `error` (§11). Its layout is exactly
`struct { code: u16, value: T }`. `code == 0` means success. When `code ≠ 0`, the contents
of `value` are unspecified.

### 5.13 The `error` type

`error` is a 16-bit code. Code 0 is reserved and never names an error. Every name declared
in any `error { … }` block in the program is assigned a distinct non-zero code (§11.1).

### 5.14 Enums

```axiom
pub enum Align: u8 {
    START,
    END,
    LEFT = 5,
    RIGHT,

    pub fn is_side(self: Align) bool {
        return (self == Align.LEFT) or (self == Align.RIGHT);
    }
}
```

An enum is a distinct type whose values are its **members**, named constants of an integer
**representation**. `Align.START` is a member (with a module: `text.Align.START`). Enums
name one choice among several: an alignment, a mode, a kind. A set of bit flags stays a
plain integer, because the flags combine with `|`.

- **Values.** A member without `= value` is one more than the member before it, and the
  first is `0`. Values must be distinct (`E0309`), and an enum has at least one member
  (`E0304`).
- **Representation.** `enum Name: T` stores members as the integer type `T` (`E0308` for
  any other type; `E0503` if a value does not fit). Without `: T`, it is the smallest of
  `u8`, `u16`, `u32`, `u64` that holds every value, or of `i8` … `i64` if one is negative.
  Size and alignment are those of the representation, so an enum can stand in a struct
  laid out for a file format or a C API.
- **Operations.** `==` and `!=` between two values of the same enum (§7.3), and `match`
  with `.MEMBER` arms (§8.6). Nothing else: no arithmetic, no ordering, no integer
  literal in place of a member, and `@as`, `@int_cast`, `@truncate` and `@bit_cast` do not
  apply. An enum satisfies `Eq` (§10.5).
- **Conversions.** `@int_from_enum(x)` is the member's value, in the representation type.
  `@enum_from_int(E, n)` takes any integer and gives the member whose value is `n`; it
  traps (`T0007`) if there is none, and a constant `n` that names no member is `E0826`.
- **Methods** are declared inside the braces, after the members, as in structs (§9.2).
- **Constants.** `const DEFAULT: Align = Align.LEFT;` is allowed, and members are constant
  expressions in global initializers.

Only writes through raw pointers can give an enum a value that is not a member. A `match`
without `else` on such a value traps (`T0007`), as for unions (§5.10).

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
| `@int_from_enum(x)` | An enum member's value, in the representation type (§5.14) | Never fails |
| `@enum_from_int(E, n)` | The member of enum `E` whose value is `n` | Traps if there is none (`T0007`) |
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
| **C**omparison | `==` `!=` `<` `<=` `>` `>=` | int, float; `==`/`!=` also on bool, pointer, error, fn pointer, enum | result `bool` |
| Logical-**and** | `and` | bool | short-circuit |
| Logical-**or** | `or` | bool | short-circuit |

Both operands of a binary operator must have the **same type**, after literal typing
(`E0412` otherwise). Axiom has no implicit widening, and mixed `u32 + u64` is an error.

Structs, unions, arrays and slices do not support `==`. Compare them field by field or
with `@mem_eq` (Appendix A).

### 7.4 Operator precedence: a partial order

Axiom does not use a total precedence table. Precedence is a **partial order**. The
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

**Overflow mode.** Each build has an *overflow mode*, chosen with a compiler flag (§16.1):

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
not a constant known to be less than `n`, the compiler emits a compare-and-select.

**Constant evaluation.** In a `const` initializer or an array length, an overflow is a
compile error (`E0530`) in every mode.

### 7.7 Floating-point semantics

- IEEE 754 round-to-nearest-ties-to-even is used for all operations.
- Division by zero produces ±∞ or NaN, per IEEE. It does not trap.
- **No contraction.** `a * b + c` MUST NOT be fused into an FMA. Use `@fma` to request one.
- **No reassociation.** Algebraic rewrites that could change a result are forbidden.
  Axiom has no fast-math flag.
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
- `E.MEMBER` is a member of enum `E` (§5.14).
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
| `catch { … }` | If `T` is `void`, the block may complete normally, and execution continues after the expression. Otherwise the block MUST NOT complete normally, because there is no value to produce: every path ends in `return`, `break`, `continue`, or `@trap()` (`E0612`) |
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

The condition must be `bool`; Axiom has no truthiness. Braces are required. `else if`
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

The scrutinee of a `match` must be a union, an enum, an integer (character patterns are
allowed for `u8`), `bool`, or `error`.

- **Union**: each arm is `.Variant` or `.Variant(name)`. `name` binds a copy of the
  payload.
- **Enum**: each arm is `.MEMBER`.
- **Integer**, `bool`, or `error`: each arm is a literal or `error.Name`.
- Arms are tested in order. Each pattern may appear at most once (`E0631`).
- The match must be **exhaustive** (`E0630`). A union or enum match must list every variant
  (member) or end with `else`. Integer and `error` matches always need `else`.
- An arm body is a block. Axiom has no fall-through.

```axiom
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

---

## 9. Functions and Methods

### 9.1 Declarations

```axiom
fn name(p1: T1, p2: T2) R { … }
```

- The return type is **mandatory**. Use `void` when there is no value.
- Parameters are immutable bindings. To modify one, copy it into a `let mut`.
- Arguments are passed **by value**, including structs and arrays. To share, pass a
  pointer or a slice.
- Recursion is allowed. Stack exhaustion traps (`T0008`). Native stacks end in a guard
  page. On WebAssembly, frames that live in linear memory (locals whose address is
  taken, large aggregates) are checked on entry against the thread's stack limit,
  since running past the end of a linear-memory stack would otherwise go unnoticed.

### 9.2 Methods

A function declared inside a `struct`, `union` or `enum` body is an **associated function**, called
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

This table is the whole of method-call sugar. Axiom has no multi-level auto-deref, no
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
Axiom has no compile-time code execution. Instantiation is pure substitution.

```axiom
fn max[T: Ord](a: T, b: T) T {
    if a > b {
        return a;
    }
    return b;
}

pub struct List[T] {
    items: []mut T,
    len: usize,

    pub fn init(capacity: usize) !Self {
        let items = mem.alloc.[T](capacity)!;
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

fn demo() !void {
    let mut xs = List.[u32].init(16)!;
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

```axiom
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
| `Eq` | integers, floats, `bool`, pointers, `error`, function pointers, enums | `==`, `!=` |
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

Axiom generics have no specialization, overloading, variadic parameters, default
arguments, higher-kinded parameters, compile-time execution, or dynamic dispatch. There are
two ways to make code vary in behavior rather than in type:

- pass a function pointer explicitly, as in
  `fn sort[T](items: []mut T, less: fn(T, T) bool) void`, or
- dispatch over a tagged union with `match`.

---

## 11. Errors

### 11.1 Declaring errors

```axiom
error { OutOfMemory, DimensionMismatch, InvalidInput }
```

Error names form one **program-wide** set. A name may be declared in more than one module;
all declarations of the same name refer to the same error. Using `error.X` where `X` is not
declared anywhere in the program is `E0650`, which catches typos.

Codes are assigned deterministically: all declared names are sorted by byte value and
numbered starting at 1. Codes therefore depend on the whole program, so modules that
are compiled separately refer to them by name, and they are fixed at link time (§16.7). `@error_name(e)` returns the name as a `[]u8`. For an unknown code
it returns `"<unknown>"`.

### 11.2 Returning and handling errors

```axiom
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

Errors are ordinary values. Axiom has no stack unwinding and no exception tables, and it
does not capture stack traces implicitly. (Debug builds MAY record error return traces,
§16.4.)

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

- A **dangling pointer** (use after free) reads whatever bytes
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

Every local and global has an initializer. Memory obtained from the allocator has
unspecified contents (§12.1) but is never "poison".

### 12.4 Allocation

No language construct allocates heap memory: there is no implicit allocation. Heap
memory comes from the global allocator in `std/mem` (§15.1), which every part of a program
shares, like `malloc` in C or the global allocator in Rust and C++. Allocation is an
ordinary call:

```axiom
let buf = mem.alloc.[u32](1024)!;   // []mut u32
defer mem.free(buf);                 // T deduced from the argument
```

The allocator takes memory from the system with `@memory_grow` and can give it back with
`@memory_discard` (Appendix A). Code that calls `@memory_grow` itself gets pages the
allocator never touches.

### 12.5 Alignment

Alignment affects **layout only**. Misaligned loads and stores are defined, as in
WebAssembly, and may simply be slower. `@ptr_cast` and `@cast_slice` therefore never trap
on alignment. `@align_of(T)` reports the natural alignment used for layout.

### 12.6 Concurrency

Programs may run several threads over the one linear memory. §12.8 defines threads,
the memory model, and atomics.

### 12.7 Traps

A trap stops the program **immediately**, including every other thread (§12.8):

- `defer` blocks do not run.
- The host observes the trap (in JavaScript, a `WebAssembly.RuntimeError`).
- The trap site maps to a trap code, a source span and a stack trace (§16.4).

Appendix B lists every trap condition.

---

### 12.8 Threads and concurrency

This section defines shared-memory concurrency. The design follows from three facts:

- WebAssembly, the reference semantics, has shared memories, atomic instructions and
  `wait`/`notify`, but no instruction that creates a thread. Hosts create threads by
  instantiating the module again on the same memory.
- The mandate forbids hidden control flow and undefined behavior.
- The language has no borrow checker, and v0.1 does not add one.

#### 12.8.1 Decisions

| # | Decision | Reason |
|---|---|---|
| 1 | **Threads are a library, not syntax.** `std/thread` creates threads through the host ABI (§12.8.5). The language adds only atomic builtins. | Matches WebAssembly, where thread creation is a host operation. Keeps the language small. |
| 2 | **A data race is not undefined behavior.** A read that races with a write returns, for each byte, a value some store wrote to that byte. Tearing is permitted (§12.8.2). | Keeps the zero-UB rule without a borrow checker or `Send`/`Sync` traits. This is WebAssembly's memory model, applied everywhere, just as §12.1 applies WebAssembly's bounds model. A race, like a dangling pointer, is a logic bug with a defined outcome. |
| 3 | **Only sequentially consistent atomics.** There are no memory-order parameters. | Every WebAssembly atomic is sequentially consistent. SC is the model people and agents reason about correctly, and weaker orderings are a common source of subtle bugs. The cost is some performance on weakly ordered native CPUs. Relaxed orderings may come later (Appendix D). |
| 4 | **Atomics are builtins on pointers to integers** (`@atomic_add(&mut x, 1)`), not special types. | Any integer in memory can be accessed atomically, as in WebAssembly. `std/sync.Atomic[T]` wraps them for convenience. |
| 5 | **`wait`/`notify` operate on 32-bit integers only.** | Linux futexes, the native implementation, are 32-bit, and WebAssembly has `wait32`. 64-bit waiting would behave differently on different targets. |
| 6 | **No `async`/`await`, no coroutines.** | An `await` is a suspension point that looks like an ordinary expression, which is hidden control flow. Blocking calls on threads are explicit. |
| 7 | **A trap in any thread terminates the whole program.** | A trap means the program's state is no longer trustworthy. Letting other threads continue would let them observe that state. |
| 8 | **Deadlock is not an error.** A thread blocked forever is simply blocked. | Detecting it would require a runtime scheduler. |
| 9 | **Thread-local globals, mutable only** (`threadlocal let mut`, §12.8.6). | Per-thread state (an allocator cache, a random generator, a log buffer) otherwise has to be threaded through every call or locked. An immutable thread-local would just be a constant. |

#### 12.8.2 Memory model

- All threads share the whole linear memory (§12.1). Each thread has its own stack.
  Locals whose address is never taken are private to their thread.
- **Atomic operations** (§12.8.3) on the same location are *sequentially consistent*.
  All threads observe them in a single total order that is consistent with each
  thread's program order. An atomic operation, together with every access that comes
  before it in the same thread, happens-before any operation that observes its effect.
- **Non-atomic accesses.** If a non-atomic access conflicts with another access to an
  overlapping location (at least one is a write) and neither happens-before the other,
  the accesses *race*. A racing read returns, for each byte, the value written by some
  store to that byte that does not happen-after the read. Different bytes of one read
  may come from different stores (tearing). A racing write stores its bytes, and
  bytes of racing writes may interleave. Nothing else is affected, and a race is
  never undefined behavior. This is the WebAssembly threads memory model.
- **Implementation obligation.** A conforming implementation MUST NOT perform
  optimizations that are sound only if the program has no races. For example, it MUST
  NOT assume that two reads of a racing location return the same value, or turn a
  racing read into an unrelated value.
- `@fence()` is a sequentially consistent fence. Ordinary code does not need it,
  because every atomic operation is already sequentially consistent.

#### 12.8.3 Atomic builtins

`p` is a pointer to an integer type (`*T` for reads, `*mut T` for writes). `T` may be
8, 16, 32 or 64 bits, or a generic `T: Int`. Every operation is sequentially consistent.

| Builtin | Result |
|---|---|
| `@atomic_load(p)` | `T` |
| `@atomic_store(p, v)` | `void` |
| `@atomic_add`, `_sub`, `_and`, `_or`, `_xor`, `_xchg`, `_min`, `_max` `(p, v)` | the **previous** value. `add` and `sub` wrap: they never trap, in either overflow mode |
| `@atomic_cas(p, expected, desired)` | the previous value. The exchange happened iff it equals `expected` |
| `@atomic_wait(p, expected, timeout_ns: i64)` | `u32`: `0` woken by a notify, `1` the value was not `expected`, `2` timed out. A negative timeout waits forever. `p: *i32` or `*u32` only |
| `@atomic_notify(p, count: u32)` | `u32`: the number of threads woken. `p: *i32` or `*u32` only. `0xFFFF_FFFF` wakes all |
| `@fence()` | `void` |

The address must be aligned to `@size_of(T)`, otherwise the program traps (`T0010`),
as in WebAssembly. Alignment is otherwise not semantic (§12.5), so this is the one
place it matters.

`@atomic_wait` blocks only when the value at `p` equals `expected`, checked atomically
with going to sleep. `@atomic_notify` wakes up to `count` threads waiting on `p`.
Together they are the building block for every blocking primitive in `std/sync`.
Some hosts forbid blocking on the main thread (browsers do); there `@atomic_wait`
traps (`T0000`). On a single-threaded WebAssembly program, whose memory is not
shared, `@atomic_wait` also traps. Programs that never block don't notice either.

#### 12.8.4 Threads

`std/thread`:

- `spawn(alloc, f, arg) !Thread[T]` runs `f(arg)` on a new thread. `arg: T` is copied.
  To share data, pass a pointer, and synchronize access to it.
- `Thread.join()` waits for `f` to return and releases the thread's resources.
- `parallel_for(alloc, n, threads, body, ctx)` splits `0..n` into disjoint ranges,
  runs `body(ctx, lo, hi)` for each in parallel, and returns when all are done. Bodies
  that write only inside their own range cannot race.
- `cpu_count()` and `sleep_ms(ms)`.

`std/sync`: `Atomic[T: Int]`, `Mutex`, `WaitGroup`, and `Channel[T]` (bounded,
multi-producer, multi-consumer, closable).

When `main` returns, the program ends, even if other threads are still running. This
matches C and most hosts.

#### 12.8.5 Host ABI and the WebAssembly mapping

Threads use host functions from module `"axiom"`:

| Function | Meaning |
|---|---|
| `ax_thread_spawn(entry: fn(*mut void) void, arg: *mut void, stack_top: usize, exited: *mut u32) i32` | start a thread running `entry(arg)`; a negative result means failure. When the thread has returned from `entry` and no longer uses its stack, the host stores `1` to `*exited` and notifies it (so the stack can be freed) |
| `ax_thread_stack_size() usize` | the stack size the program must allocate for a new thread, or `0` if the host provides stacks (native) |
| `ax_cpu_count() u32`, `ax_sleep_ns(ns: u64)` | |

On WebAssembly, a program that uses threads is linked with a **shared, imported**
memory. The compiler also exports `axiom_thread_start(entry, arg, stack_top)`. The
host runs a new thread by instantiating the module again on the same memory, in a
Web Worker or node worker, and calling that export. The export switches the new
instance's stack pointer to `stack_top` and calls `entry(arg)`. Data segments are
initialized only once, by the first instance. Programs that never spawn threads
keep an unshared memory and need no worker support.

On native targets, threads are POSIX threads, and `wait`/`notify` are futexes (Linux).

#### 12.8.6 Thread-local globals

```axiom
threadlocal let mut calls: u64 = 0;
pub threadlocal let mut seed: [4]u64 = [1, 2, 3, 4];
```

A module-level `let mut` marked `threadlocal` has one copy per thread. Each thread's
copy starts with the initializer's value, whatever other threads have done to theirs,
and lives as long as the thread. The main thread's copy is the one it starts with.
Otherwise a thread-local is an ordinary global: it is read, assigned and passed by
address (`&mut calls`) in the same way, and a pointer to it may be given to another
thread, which then accesses the first thread's copy (a pointer is just an address,
§12.1). Such a pointer dangles once its thread has ended.

A thread-local must be `mut` (E0354). `threadlocal` is recognized only before a
module-level `let` and is otherwise an ordinary identifier.

Implementation: native code uses the platform's thread-local storage (ELF TLS, the
initial-exec model). On WebAssembly, where each thread is a separate instance, the
program's thread-locals form one block. The main thread uses a static copy. The
`axiom_thread_start` export (§12.8.5) copies the initial values to the top of the new
thread's stack and records the copy's address in a WebAssembly global, which every
access consults. With separate compilation (§16.7), this block's layout is part of
every module's cache key.

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

## 14. Targets and ABI

### 14.1 Targets

| Target | `usize` | Notes |
|---|---|---|
| `wasm32-freestanding` | 32 | Primary target. No WASI and no imports except those declared |
| `wasm32-wasi` | 32 | Standard library may use WASI imports |
| `wasm64-freestanding` | 64 | memory64 |
| native (`x86_64-*`, `aarch64-*`) | 64 | Linear memory is a region of address space reserved at the first `@memory_grow` (up to 1 TiB), whose 64 KiB pages are committed as it grows. Bounds checks against it (§12.1) are future work |

### 14.2 Exports

`export fn` produces a WebAssembly export with the same name. Parameter and return types
must be one of the following (`E0801` otherwise):

- `bool`,
- an integer type (8-bit and 16-bit types are passed as `i32`, zero- or sign-extended),
- `f32`, `f64`,
- `usize`/`isize`,
- a pointer,
- a function pointer (a table index on WebAssembly),
- a SIMD type,
- `void` as the return type.

Exported functions MUST NOT return `!T`. Handle errors before the export boundary. The
module exports its linear memory as `"memory"`.

### 14.3 Imports

```axiom
import host "env" {
    fn console_log(ptr: *u8, len: usize) void;
}
```

This declares WASM function imports from module `"env"`, which is the default if the string
is omitted. They are called like normal functions. Imported functions use the same type
restrictions as exports.

### 14.4 Function tables

Every function used as a value is placed in table 0. A function pointer is its index
(`u32`). Calling through a pointer uses `call_indirect` with a signature check, so a bad
index or wrong signature traps (`T0009`).

---

### 14.5 C foreign function interface

Axiom calls C, and C calls Axiom, with no glue code on either side.

#### 14.5.1 Calling C

```axiom
extern "C" {
    fn strlen(s: *u8) usize;
    fn snprintf(buf: *mut u8, n: usize, fmt: *u8, ...) i32;
    fn qsort(base: *mut void, n: usize, size: usize, cmp: fn(*void, *void) i32) void;
}

extern "C" "m" {          // optional second string: a library to link (-lm)
    fn cos(x: f64) f64;
}
```

- The ABI string must be `"C"` (`E0306`). The functions are ordinary C symbols,
  resolved by the linker. `pub extern "C" { … }` makes them visible to importing
  modules.
- On **native targets** they link against object files, libraries and libc as usual.
  A native program that declares no C functions of its own needs no C library: the
  self-hosted toolchain then links a static executable that makes system calls
  directly (§15.2).
- On **WebAssembly** they link against C compiled for the same wasm target. C code
  and Axiom code share the linear memory, the stack and the function table, so
  pointers mean the same thing on both sides. Freestanding wasm has no libc unless one
  is linked, for example wasi-libc.
- `...` declares a **variadic** function. It is allowed only in `extern "C"`
  (`E0307`), and only after at least one fixed parameter. The variadic arguments
  undergo C's default argument promotions, and Axiom requires them to already have
  a promoted type. They must be `i32`, `u32`, `i64`, `u64`, `isize`, `usize`, `f64`,
  a pointer or a function pointer (`E0824`, with a fix that inserts the `@as`).
  Untyped literals become `i32` (or `i64` if they don't fit) and `f64`.

#### 14.5.2 Calling Axiom from C

Every `export fn` is a C function with the same name. The one exception is `main` on
WebAssembly, which C code cannot call. The compiler
writes a C header for them with `--emit-header=<path>`. The header contains:

- a prototype for every export, using `<stdint.h>` types and `const` for immutable
  pointers;
- a definition of every struct reachable from those signatures, with a
  `_Static_assert` on its size;
- a check of the target's pointer size, because layouts are target-specific.

It compiles as C11 and as C++.

#### 14.5.3 Types at the boundary

| Axiom | C |
|---|---|
| `i8` … `i64`, `u8` … `u64` | `int8_t` … `uint64_t` (`int` is `i32` on every supported target) |
| `isize`, `usize` | `ptrdiff_t` / `long`, `size_t` / `unsigned long` |
| `f32`, `f64`, `bool` | `float`, `double`, `bool` |
| `*T`, `*mut T` | `const T *`, `T *` (`*void` is `const void *`) |
| `fn(A, B) R` | `R (*)(A, B)` |
| SIMD types | `__attribute__((vector_size(16)))` vectors |

**Structs cross the boundary by pointer only.** Axiom's struct layout rules (§5.9)
are C's natural layout rules, so a `*Point` is a valid `Point *` in C. The header's
`_Static_assert`s check this. Passing structs *by value* depends on each platform's C
calling convention, and is not supported in v0.1 (`E0801`). Slices, error unions,
tagged unions and `error` also can't appear in boundary signatures directly. Inside
structs, a slice appears in C as `struct { const T *ptr; size_t len; }`. Tagged and
error unions appear as opaque byte arrays of the right size and alignment, since C has
no equivalent.

**Small integers and `bool`** are sign- or zero-extended as C requires, on every
function and every indirect call. Any Axiom function whose signature uses only
boundary types can therefore be passed to C as a callback, with no annotation.

#### 14.5.4 NULL and C strings

- Axiom pointers are non-nullable, but C returns NULL. Check pointers that come from
  C with `std/c.is_null` / `is_null_void`. Axiom never checks for NULL implicitly.
- `std/c` provides `from_cstr` (a C string viewed as `[]u8`), `to_cstr` (a copy with a
  NUL terminator) and `null_ptr`.
- Because a NUL follows every string literal (§2.5), `"%d\n".ptr` can be passed directly
  as a C format string.

#### 14.5.5 The safety boundary

The zero-undefined-behavior guarantee covers Axiom code. **C code, and whatever it
does to memory, is outside it.** A C function that writes out of bounds or frees memory
twice can corrupt the Axiom program, just as it could corrupt any C program. The
`extern "C"` block is the visible marker of that boundary.

- On WebAssembly, the damage is confined to the module's linear memory, like
  everything else.
- On native targets, Axiom's pointers are real addresses (see the compiler README),
  which is what makes pointers interchangeable with C at no cost.

## 15. Standard Library Core

Only the parts that the language semantics depend on are specified here.

### 15.1 `std/mem`: the global allocator

```axiom
pub fn alloc[T](n: usize) ![]mut T          // contents unspecified
pub fn alloc_zeroed[T](n: usize) ![]mut T   // every byte 0
pub fn free[T](s: []mut T) void
pub fn realloc[T](s: []mut T, n: usize) ![]mut T
pub fn dup[T](s: []T) ![]mut T
pub fn create[T](v: T) !*mut T             // one value, initialized to v
pub fn destroy[T](p: *mut T) void
pub fn alloc_bytes(size: usize) usize        // 0 if there is no memory (or size is 0)
pub fn free_bytes(p: usize, size: usize) void
pub fn realloc_bytes(p: usize, old: usize, size: usize) usize
pub fn in_use() usize                        // bytes allocated and not freed
pub fn enable_threads() void
```

These are ordinary functions (§10), not builtins. The rules:

- **Sized free.** `free` takes the slice that `alloc` (or `realloc`) returned, so the
  allocator knows the size without storing it. Freeing memory that did not come from the
  allocator, or with the wrong size, traps (`T0000`) where the allocator can tell: always
  for an address outside its spans and blocks, and for a large block freed twice. Other
  double frees are logic bugs with defined behavior (§12.1).
- **Failure.** `alloc`, `realloc`, `dup` and `create` return `error.OutOfMemory` when the
  system has no more memory or the byte count overflows. `realloc` leaves the old slice
  valid when it fails.
- **Alignment.** Memory is aligned to 16 bytes, enough for every type (§12.5).
- **Zero sizes.** `alloc.[T](0)` returns an empty slice that owns no memory; freeing it
  does nothing.
- **Threads.** The allocator is safe to use from every thread. `std/thread` calls
  `enable_threads` before starting the program's second thread; code that starts threads
  another way (from C) must call it first.

The design is not normative: objects up to 8 KiB are rounded up to one of 32 size classes
and carved from 64 KiB spans, one class per span, with no per-object header; larger ones are
blocks of 4 KiB units that merge with free neighbours. A single-threaded program never
locks; with threads, each thread uses one of 32 heaps (chosen by its stack address) with
its own lock. Freed memory is reused, and past a budget, large free blocks are returned to
the system with `@memory_discard`.

A program that wants arena or pool allocation writes it on top of `mem.alloc`: an arena is
a struct that takes large blocks from `mem.alloc` and hands out pieces of them.

### 15.2 Other modules and the host ABI

The rest of the standard library, listed in `std/README.md`, is ordinary Axiom code. It adds no language rules. Operating-system
services are reached through host functions imported from module `"axiom"` (the
*Axiom host ABI*). Implementations exist for node (`runtime/host.mjs`) and POSIX
(`runtime/native_host.c` in C, `runtime/native.ax` in Axiom). A program that uses only the language and `std/mem`, `vec`,
`hash`, `map` and `matrix` needs no host at all.

The Axiom runtime uses no C library. It reaches the kernel through system calls
(`runtime/linux.ax`), starts threads with `clone` and sets up their thread-local
storage itself, starts processes with `fork` and `execve` (searching `PATH` itself), and resolves host names from `/etc/hosts` and by DNS queries to the
servers in `/etc/resolv.conf`. So a native program built by the self-hosted toolchain
is a static executable with no dynamic loader, unless it links C code or libraries,
in which case it is linked against the C library as before.

---

## 16. Toolchain and Agent Diagnostic Interface

### 16.1 Commands

```
axiom build <file.ax> --target=<t> [-O0..-O3] [--release | --overflow=trap|saturate] -o <out>
                          [--sanitize=address]    # check heap accesses (§16.8)
axiom check <file.ax>                  # parse and type-check only; no codegen
axiom fmt   <file.ax|dir>... [--check] [--stdout]   # canonical layout (§16.5)
axiom run   <file.ax> [--export=name]  # build for wasm32 and execute in the reference runtime
axiom explain <CODE>                   # long-form documentation for a diagnostic code
axiom test  <file.ax> [--filter=text] [--timeout=ms]   # run test blocks (§17)
axiom new   <name>                     # create a package skeleton (§4.6)
axiom profile <file.ax> [-- args]      # sampling profiler (§16.6)
axiom cache [path|clean]               # the compiled-module cache (§16.7)
```

`build`, `check` and `test` also accept a package directory, or no path at all for the
package in the current directory (§4.6.4).

Every command accepts `--format=human` (the default) or `--format=agent-json`.

- Compiler output is **deterministic**. The same inputs and flags produce byte-identical
  `.wasm` files and diagnostics.
- Diagnostics are sorted by file, then by start byte, then by code.

### 16.2 JSON output

With `--format=agent-json`, the command writes **one** JSON object to **stdout** and nothing
else. (Progress and log text, if any, go to stderr.) Exit codes are:

- `0`: success, possibly with warnings
- `1`: the program has errors
- `2`: invalid invocation or internal compiler error

```json
{
  "schema": "axiom-diagnostics/1",
  "status": "error",
  "diagnostics": [
    {
      "code": "E0412",
      "severity": "error",
      "message": "type mismatch in SIMD operation: expected f32x4, found f32",
      "file": "dot_product.ax",
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
          "file": "dot_product.ax",
          "span": { "start_line": 54, "start_col": 13, "end_line": 54, "end_col": 15,
                    "start_byte": 1790, "end_byte": 1792 }
        }
      ],
      "fixes": [
        {
          "description": "broadcast the scalar to all lanes",
          "applicability": "machine",
          "edits": [
            { "file": "dot_product.ax",
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

### 16.3 Diagnostic codes

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
| E0211 | Imported module not found |
| E0213 | Two different packages with the same name in one build |
| E0214 | `pkg:` import of a package that isn't a dependency (or outside any package) |
| E0215 | Invalid `axiom.toml` |
| E0216 | Git dependency can't be fetched |
| E0217 | Package directory or manifest missing |
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
| E0304 | Union or enum without variants (members) |
| E0306 | Unknown ABI in `extern` (only `"C"`) |
| E0307 | `...` outside `extern "C"`, or with no fixed parameter |
| E0308 | Enum representation is not an integer type |
| E0309 | Two enum members with the same value |
| E0311 | Unknown trap code in `expect trap(...)` |
| E0312 | Duplicate test name in a module |
| E0354 | Thread-local global without `mut` |
| E0801 | Type not allowed at the export or import boundary |
| E0823 | Invalid atomic operand (not a pointer to an integer, or not 32-bit for wait/notify) |
| E0824 | C variadic argument without a promoted type |
| E0825 | `@expect_eq` operand type cannot be compared |
| E0826 | `@enum_from_int` of a constant that is no member |

### 16.4 Runtime trap reporting and stack traces

Every trap site carries a constant **site record**: trap code, source file, byte span,
line, column, the text of the failing expression, and the name of the enclosing
function. Before trapping, the program stores the record's address in a trap-info
block, together with the compared values for `@expect_eq`.

- On WebAssembly the block is reachable through the export `axiom_trap_info`.
- On native targets it is passed to the weak hook `ax_on_trap`.

Traps raised by the engine or the OS rather than by a check are mapped to their codes:
out-of-bounds memory is `T0005`, stack exhaustion `T0008`, and a bad `call_indirect`
`T0009`.

#### 17.4.1 Stack traces

Every trap and crash is reported with a **stack trace**:

```
axiom: trap T0001 (index out of bounds): xs[i] at main.ax:4:12
stack trace (innermost first):
  at get (main.ax:4:12)
  at sum (main.ax:10:14)
  at main (main.ax:29:12)
```

The trace is built from a **shadow stack**, not by unwinding the machine stack:

- Each thread has a small record: a call depth, a ring of the innermost 256
  call-site records, and a ring of 32 error-return records.
- Each call writes its call-site record (location, calling function, called
  function) at `depth % 256` and increments `depth`. When the call returns, `depth`
  goes back to its old value. No code runs at function entry or exit, and
  inlining does not remove frames.
- After a trap, the frames are still in place, because nothing unwinds them. A host
  reads them through the export `axiom_shadow_stack()` (WebAssembly) or the third
  argument of `ax_on_trap` (native).
- Faults that aren't checks also get a trace: an engine trap on WebAssembly, or
  `SIGSEGV`, `SIGBUS` or `SIGFPE` on native targets, which the native runtime catches
  on an alternate signal stack. Stack exhaustion is reported as `T0008` on both
  targets.
- Consecutive identical frames are collapsed ("the frame above repeats N more
  times"). Frames beyond the ring's 256 entries are counted, not shown.

Why a shadow stack, when WebAssembly engines and native debuggers can already walk
stacks:

1. **Portability.** It works the same on every engine and OS and needs no debug
   information in the binary.
2. **Exact source locations.** A call site is recorded as an Axiom source span, not
   as a machine address that must be mapped back to source.
3. **It doubles as the profiler's stack sampler** (§16.6).

The cost is three stores and one load per call. On call-heavy microbenchmarks (for
example, a naive recursive Fibonacci) it roughly doubles run time. On typical code it
is within measurement noise. Stack traces are on by default, and
`--stack-traces=off` removes the instrumentation.

Traces are diagnostics only: whether they are on never changes a program's
behavior. Record contents and layouts are toolchain ABI, not language semantics.

#### 17.4.2 Error return traces

Every `return` of an error value and every `!` that propagates an error appends a
site to the thread's error-return ring. A `catch` that handles an error clears the
ring. When a test fails because it returned an error, the trace shows where the
error started and the path it took:

```
FAIL parse config (1 ms)
     config.ax:40:6: error test returned error.BadSyntax
     error return trace (oldest first):
       lexer.ax:88:13 in next_token: return error.BadSyntax;
       config.ax:21:17 in parse: let t = lex.next_token()!;
       config.ax:41:9 in test "parse config": _ = parse(text)!;
```

### 16.5 Canonical formatting

`axiom fmt` defines the single canonical layout. It rewrites each file given (every `.ax`
file under a directory) in place; `--check` changes nothing and instead lists the files that
are not canonical, exiting with status 1 if there are any; `--stdout` prints the result. A
file with a syntax error is reported (with the usual diagnostics) and left alone. Running
`fmt` twice changes nothing. Agents SHOULD run `axiom fmt` before diffing or committing.

The layout is rebuilt from the tokens. The only things kept from the input's layout are
comments, blank lines between statements and declarations, trailing commas, and the line
breaks of a statement that has comments inside it. Two programs that are equal after
formatting are the same program.

- **Blocks.** Four spaces per level, one statement per line, `{` at the end of the line
  that opens the block and `}` on a line of its own (or before `else`). An empty block is
  `{}`. Several blank lines become one; a block has none at its start or end.
- **Declarations.** One struct field or union variant per line, each followed by a comma.
  The functions of `extern` and `import host` blocks one per line. Error sets and
  anonymous struct types stay on one line: `error { NotFound, Empty }`,
  `struct { x: f32, y: f32 }`.
- **Spacing.** Binary and assignment operators, `=>`, keywords and `:` (after) are spaced;
  prefix operators (`-x`, `not x`, `&mut x`, `*T`, `!T`, `[]u8`), `.`, `..`, calls,
  indexing and postfix `!` are not. Struct literals are `.{ a: 1 }` and `.{}`; array
  literals and argument lists are `[1, 2]` and `f(a, b)`; a capture is `catch |e|`.
- **Match arms.** An arm whose block is one statement without a nested block goes on one
  line if it fits: `.Circle(c) => { return c.r; }`. The `=>` of consecutive one-line arms
  line up.
- **Long lines.** A line longer than 120 columns is broken after `and` or `or` (when one is
  no deeper in brackets than every `,`), otherwise after the last `,` that fits, otherwise
  after another binary operator. The next line starts just inside the bracket the break
  is in; outside any bracket, after the leading `if `, `while `, `return ` (and so on) or
  the first `= `, or four columns in. If lining up with the bracket would leave too little
  room, the break goes right after the bracket and the next line is indented four more.
- **Trailing commas.** A bracketed list written with a trailing comma is laid out one item
  per line, the closing bracket on a line of its own. Without one, items fill lines.
- **Comments.** A comment on a line of its own is indented like the code that follows it.
  Trailing comments on a run of consecutive lines (of about the same length, at the same
  depth) line up two spaces after the longest line. A comment-only line indented past the
  code, right after a line with a trailing comment, continues that comment and lines up
  with it.

`axiom fmt` also tidies imports. Each run of consecutive `import "…" as name;` lines is
sorted by path; a duplicate import and an unused one are removed, except an import of a
package (`pkg:`) or of a module that has tests (importing it runs them, §17); and a
standard library module that the file uses as `name.x`, where `name` is declared nowhere,
is imported: `std/name`, or `std/group/name` when that is the only module of that name one
directory down (`vec` is `std/collections/vec`).

### 16.6 Profiling

```
axiom profile <file.ax> | <package dir> [--target=wasm32|native] [-O0..-O3]
                 [--rate=<hz>] [--folded=<path>] [--format=human|agent-json]
                 [-- <program arguments>]
```

`axiom profile` builds the program, runs its `main`, and reports where the time went.
Arguments after `--` are passed to the program, and its standard input is the
profiler's (so `axiom profile selfhost/src/main.ax -- build app.ax -O2` profiles the
compiler).
It is a **sampling profiler**. About `--rate` times a second (default 999) it copies the
running thread's shadow stack (§16.4.1), so every sample is an exact source-level call
path. It is not an estimate from machine addresses.

```
profile: 340 samples at 999 Hz (wall time, 0.34 s)

  self%  total%  function
   99.4    99.4  spin
    0.6   100.0  main
    0.0    99.4  work

 total%  call site
   99.4  hot.ax:18:12  work -> spin
   99.4  hot.ax:22:8  main -> work

folded stacks: hot.folded (for flamegraph.pl, inferno or speedscope)
```

- **self%** counts the samples in which the function was the innermost frame. **total%**
  counts the samples in which it was anywhere on the stack, each sample counted once
  even under recursion.
- **call sites** attributes inclusive time to each call expression, so you can see
  which calls inside a function are expensive, not just which functions are.
- **Folded stacks** (`main;work;spin 338`, one line per distinct stack) are written to
  `--folded` (default `<name>.folded`). They are the input format of common flame
  graph tools.
- With `--format=agent-json`, stdout holds one `axiom-profile/1` object with
  `samples`, `hz`, `clock`, `seconds`, `functions` (`name`, `self`, `total`, and their
  percentages), `call_sites` (`file`, `line`, `col`, `caller`, `callee`, `total`,
  `context`) and `folded`. The program's own output goes to stderr.

How samples are taken:

| Target | Clock | Mechanism | Threads |
|---|---|---|---|
| native | process CPU time | a POSIX CPU-clock timer delivers `SIGPROF`, and the handler copies the interrupted thread's shadow stack (`runtime/profile.c`). Timer overruns weight each sample, so coarse kernel ticks don't skew totals | all threads |
| wasm32 | wall-clock time | the program runs on a shared memory, and a worker thread reads the main thread's shadow stack while it runs (`runtime/profile_worker.mjs`) | main thread only |

Profile builds are otherwise identical to ordinary builds with stack traces on (the
default), except that shadow-stack stores are volatile, so optimization can't remove
them. A program's behavior never depends on whether it is being profiled.

Limitations in v0.1:

- Time is attributed to functions and call sites, not to individual lines inside the
  innermost function.
- On WebAssembly, only the main thread is sampled.
- Stacks deeper than 256 calls lose their outermost frames. These samples are
  counted and reported as truncated.

### 16.7 Separate compilation

Each **module** is a compilation unit. A build type-checks the whole program (sema is
fast), then:

1. Compiles each module to an object file. The object holds the module's non-generic
   functions and methods, plus link-once copies of the generic instances the module
   uses.
2. Generates a small per-program **link unit**. It holds the global variables, the
   error codes and the error-name table, runtime state (the trap-info block, shadow
   stacks, and the native heap), and the host and test exports.
3. Links everything.

Module objects are cached in `<cache>/obj/`, where `<cache>` is `$AXIOM_CACHE`,
`$XDG_CACHE_HOME/axiom` or `~/.cache/axiom`. The cache key hashes everything the
object depends on:

- the compiler binary;
- the code-generation options (target, `-O`, overflow mode, stack traces, profiling,
  test mode);
- the module's source and path;
- recursively, the keys of the modules it imports;
- on WebAssembly, the layout of the program's thread-locals (§12.8.6): their order,
  sizes and alignments.

Editing a module recompiles it and every module that imports it, directly or
indirectly. Nothing else is recompiled. Missing objects are compiled in parallel. The
toolchain's C runtime (`runtime/*.c`) is compiled once per content and cached the
same way. `axiom cache clean` empties the cache, which is never evicted
automatically in v0.1. `--verbose` reports how many modules were compiled and how
many came from the cache.

What makes modules independent:

| Program-wide fact | How units agree on it |
|---|---|
| Error codes (§11.1) | Module objects load `axiom.errcode.<Name>` from the link unit. A `match` on errors becomes a chain of comparisons. Adding an error name anywhere renumbers every code without invalidating any cached object |
| `@error_name` | One function in the link unit, next to the table |
| Globals | Defined in the link unit (initializers may contain error codes) and referenced by name |
| Thread-locals on WebAssembly | One block, defined in the link unit; each module's code addresses fields of it, so its layout is in every cache key |
| Generic instances | Defined link-once by every unit that uses them, under names built from module-qualified types, so the linker keeps one copy |
| Private functions | Externally visible under their module-qualified names, because another unit's generic instance may call them |
| Source locations in site records (§16.4) | A stable hash of the file's path, not a per-build file number |

**When it is used.** Separate compilation is the default for executables at `-O0` and
`-O1`. Those are the edit–build–test builds, where only changed modules are recompiled.
At `-O2` and `-O3` the default is still one whole-program unit, so functions can be
inlined across modules. `--units=module|program` overrides the default. Emitting a
single object, IR or assembly file (`--emit=obj|llvm|asm`) always uses one unit.

A program means the same whatever the unit mode. The test suite runs in both modes.

One observable difference: in module mode every non-generic function is compiled,
not only the reachable ones. So on native targets an `extern "C"` function that is
declared but never called must still exist at link time.

---

### 16.8 The address sanitizer

Slices, arrays and integer arithmetic are checked by the language itself (Appendix B),
so the errors a sanitizer adds are those of memory *lifetime*: using memory after it is
freed, freeing it twice or with the wrong size, reaching outside an allocation through a
pointer made from an address, and never freeing it. `--sanitize=address` (accepted by
`build`, `test` and `profile`) checks for them:

- The global allocator (`std/mem`) puts a header before each allocation and a redzone
  after it, and keeps *shadow memory*: one byte per 8 bytes of heap, saying how many of
  them may be used or why they may not (redzone, freed).
- The compiler checks every load and store through a pointer or slice, `@mem_copy`,
  `@mem_set`, `@mem_eq`, `@load`, `@store`, the atomics, and `for` over a slice,
  against the shadow memory. An access to memory that is not allocated traps with
  `T0012` at the access, after a report on stderr of what was accessed and where it was
  allocated and freed:

  ```
  axiom: AddressSanitizer: heap-use-after-free: 4-byte access at 0x1100c4: 4 bytes inside a 16-byte allocation at 0x1100c0, freed
  freed:
    at main (uaf.ax:20:5)
  allocated:
    at make (uaf.ax:13:14)
    at main (uaf.ax:19:14)
  axiom: trap T0012 (memory error): xs[1] at uaf.ax:21:27
  ```

- Freed memory is kept poisoned in a quarantine (64 MiB) before the allocator reuses it,
  so late uses of it are caught for a while. `realloc` always moves.
- `free` checks its argument: a double free, a size other than the allocation's, or
  memory the allocator did not hand out traps with `T0012`.
- When `main` returns, the allocations never freed are reported as leaks, and the exit
  status becomes 23 if it was 0. Memory kept on purpose until the end can be excluded:
  allocations made between `mem.leaks_ignored(true)` and `mem.leaks_ignored(false)`
  are not reported. In a test binary (`axiom test --sanitize=address`), where each
  test runs in a process or instance of its own, a test that passes but leaves
  allocations unfreed fails with the code `leak`, and the report is its output.

The allocator and the runtime are not instrumented. Memory that is not from the heap
(globals, locals, memory from `@memory_grow`) is not checked. Allocation sites come from
the shadow stack, so they need `--stack-traces=on` (the default). Each checked access
looks its shadow byte up inline and calls into `std/mem.ax` only when that test fails, so
programs typically run three to four times slower, and use more memory. In code,
`@sanitize_address()` is a constant: whether the program is being built with the
sanitizer.

## 17. Testing

### 17.1 Test blocks and assertions

```axiom
test "parse accepts digits" {
    @expect_eq(parse_u64("42")!, 42);
    @expect(parse_u64("7")! < 10);      // any bool expression
}

test "indexing past the end traps" expect trap(T0001) {
    let xs: [2]u8 = [1, 2];
    let i: usize = 5;
    _ = xs[i];
}
```

- `test "name" Block` is a top-level declaration. Its body is checked like a function
  returning `!void`, so `!` propagates errors out of it. Tests can use the module's
  private items.
- Test names must be unique within a module (`E0312`). Tests are not names: they can't
  be called or referenced.
- Tests are **type-checked in every build** but compiled only by `axiom test`. A
  broken test fails the build, so tests never rot silently. They add nothing to
  normal outputs.
- A test **passes** if its body completes. It **fails** if it returns an error, traps,
  or doesn't finish within the timeout.
- `expect trap` inverts this: the test must trap. `expect trap(Tnnnn)` requires that
  specific trap code (`E0311` for an unknown code). Traps are defined behavior (§12.7),
  so they are testable.
- `@expect(cond)` traps with `T0011` when `cond` is false.
- `@expect_eq(a, b)` traps with `T0011` when `a != b`, and records both values. It
  compares integers, floats, `bool`, `error`, enums, pointers, and `[]u8` by content
  (`E0825` for other types). Both builtins also work outside tests, as assertions
  that stay on in every build.

### 17.2 Running tests

```
axiom test file.ax|<package dir> [--target=wasm32|native] [--filter=text] [--timeout=ms] [--format=agent-json]
```

- **Which tests run.** The tests in `file.ax`, plus those of the modules it imports that
  belong to the same package as `file.ax`, or, outside packages, that are not standard
  library modules. Given a package directory, the tests of every module in the package
  (§4.6.4). Dependencies' tests don't run. Running `axiom test std/collections/vec.ax`
  tests a standard library module.
- **Isolation.** Each test runs in isolation: a fresh WebAssembly instance in its own
  worker, or a forked process on native targets. A trap, a hang or corrupted memory
  can't affect other tests. Independent tests run in parallel. A trap in a thread the
  test started ends the test as its own trap would (`expect trap` applies to it).
- **Output.** Each test's output (stdout and stderr) is captured and reported with it.
- **Exit status.** `0` if every selected test passed, `1` if any failed, `2` if the tests
  could not be built or run. Compile errors are reported as diagnostics (§16.2).

With `--format=agent-json`, stdout holds one object:

```json
{"schema":"axiom-tests/1","status":"fail","passed":3,"failed":1,"tests":[
  {"name":"parse accepts digits","file":"parse.ax","status":"fail","duration_ms":4,
   "failure":{"code":"T0011","message":"expected equal values, left: 41, right: 42: @expect_eq(...)",
              "file":"parse.ax","span":{"start_line":2,"start_col":5,"end_line":2,"end_col":36,
              "start_byte":40,"end_byte":71},"context":"@expect_eq(parse_u64(\"42\")!, 42);"},
   "output":""}]}
```

`failure.code` is one of:

- a trap code (`T0000` to `T0012`);
- `error`: the test returned an error, and the message names it;
- `no_trap`: an `expect trap` test completed;
- `timeout`.

The span points at the failing expression, or at the test's name when there is no more
precise location.

## 18. Complete Example

`examples/dot_product.ax`, the design-notes example updated to this specification:

```axiom
// =============================================================================
// File: dot_product.ax
// Build: axiom build dot_product.ax --target=wasm32-freestanding -O3 -o engine.wasm
// =============================================================================

import "std/mem" as mem;

import host "env" {
    fn console_log(ptr: *u8, len: usize) void;
}

error { DimensionMismatch }

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
export fn compute(length: usize) f32 {
    let vec_a = mem.alloc.[f32](length) catch return -1.0;
    defer mem.free(vec_a);
    let vec_b = mem.alloc.[f32](length) catch return -1.0;
    defer mem.free(vec_b);

    for i in 0..length {
        vec_a[i] = 1.5;
        vec_b[i] = 2.0;
    }

    let result = dot_product_simd(vec_a, vec_b) catch return -2.0;

    let msg = "dot product computed";
    console_log(msg.ptr, msg.len);

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
`@float_cast`, `@bit_cast`, `@int_from_enum`, `@enum_from_int`, `@int_from_ptr`,
`@ptr_from_int`, `@ptr_cast`.

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
| `@memory_discard(addr, len)` | `usize, usize → void` | The whole 64 KiB pages in `[addr, addr + len)` may be returned to the system; their bytes become unspecified (on native targets they read as zero). WebAssembly memory cannot shrink, so there it does nothing |

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

### A.6 Atomics

See §12.8.3: `@atomic_load`, `@atomic_store`, `@atomic_add`, `@atomic_sub`,
`@atomic_and`, `@atomic_or`, `@atomic_xor`, `@atomic_xchg`, `@atomic_min`,
`@atomic_max`, `@atomic_cas`, `@atomic_wait`, `@atomic_notify`, `@fence`.

### A.7 Control

| Builtin | Notes |
|---|---|
| `@trap()` | Diverges and traps with `T0000`. Usable where any type is expected |
| `@error_name(e)` | `error → []u8` |
| `@expect(cond)` | Traps with `T0011` if `cond` is false (§17.1) |
| `@expect_eq(a, b)` | Traps with `T0011` if `a != b`, recording both values (§17.1) |
| `@memory_error()` | Diverges and traps with `T0012`: how `std/mem` reports a bad `free` under the address sanitizer (§16.8) |
| `@sanitize_address()` | `→ bool`, a constant: whether the build uses `--sanitize=address` (§16.8) |

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
| `T0007` | Invalid union tag or enum value at `match`; `@enum_from_int` of a value that is no member |
| `T0008` | Stack exhaustion |
| `T0009` | Invalid function pointer or signature mismatch (`call_indirect`) |
| `T0010` | Misaligned atomic access (§12.8.3) |
| `T0011` | Expectation failed: `@expect` / `@expect_eq` (§17.1) |
| `T0012` | Memory error found by the address sanitizer (§16.8) |

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
| C7 | The notes' inline WebAssembly (`wasm { … }` blocks) is not part of the language | Builtins (Appendix A) cover what it was used for (`@clz`, SIMD), portably and type-checked; an escape hatch into another instruction set weakens every guarantee around it, and every target but one had to translate it |
| C8 | `@load_unaligned(f32x4, &a[i])` became `@load(f32x4, a[i..i + 4])` | The original only bounds-checked element `i` and then read 16 bytes, which is a bounds-check hole. Alignment is not semantic (§12.5), so `_unaligned` is redundant |
| C9 | `arena.allocator().alloc(u32, 1024)` became `arena.allocator().alloc.[u32](1024)` | Type arguments are written `.[…]` (§10.3). A type passed as an ordinary argument would need types to be parseable as expressions |
| C10 | `&mut HEAP_BYTES` passed as a slice became `HEAP_BYTES[..]` | Array-pointer-to-slice would be an implicit conversion |
| C11 | `pub` (module visibility) and `export` (WASM export) are distinct. Top-level `let`/`let mut` globals and `error { }` declarations were added to the grammar | The notes used these without grammar support |
| C12 | Integer SIMD vectors require `+%`/`-%`/`*%`. Bare `+` is float-only | Keeps §7.6 consistent. Lane-wise overflow trapping is impractical |
| C13 | Raw pointers are fully defined through the linear-memory model (§12) | "Zero UB" and "raw pointers" conflict unless memory itself has defined semantics. WASM's model provides this, and it is adopted on all targets |
| C14 | The notes' arena-centric memory (no global allocator; every allocation names an `Allocator`; `Arena`, `FixedBuffer`, `page_allocator`) was replaced by one global allocator, `std/mem` (§12.4, §15.1) | An allocator parameter on every container and every function that allocates spread through all APIs, and choosing among allocators was a decision agents got wrong more often than they gained from it. Allocation stays explicit: a call, never a language construct |
| C14 | Error names must be declared (`error { … }`) | Prevents typos in `error.X` from silently creating new errors |
| C15 | Added `for`, `loop`, compound assignment, `_ =` discards, and a no-shadowing rule | Needed by the notes' examples, or by the agent-ergonomics goal |
| C16 | The diagnostic `suggested_fix.confidence` float became `fixes[].applicability` (an enum) plus byte-range `edits` | A float confidence is hard to act on. Byte-range edits can be applied mechanically |
| C17 | Horizontal float reductions have a fixed pairwise association order | Determinism across WASM and native |
| C18 | Shift counts are **not** clamped. `x << k` and `x >> k` have their mathematical meaning for every `k`, so `k ≥ width` gives `0` or sign fill | Design review, draft 2. `x << 40` has a clear meaning, and clamping it to `x << 31` would be surprising |
| C19 | Added monomorphized generics with builtin constraints (§10) | Design review, draft 2 |
| C20 | The language was briefly renamed Phalange (source files `.ph`); it is Axiom again, with the extension `.ax` | Design review, draft 2; the original name was preferred |
| C21 | Float literals round to the nearest value of their target type (formerly open question Q5) | The spec's own examples (`3.14159265` as `f32`) need it; found while implementing the compiler |
| C22 | For `!void`, a `catch` block may complete normally ("handle and continue") | Found while writing the standard library: without it, `f() catch \|e\| { log(e); }` could not be written |
| C23 | Shared-memory threads (library-level), a race-tolerant memory model, and sequentially consistent atomic builtins (§12.8) | The user asked for threads and concurrency. §12.8.1 gives the reasoning |
| C24 | Function pointers are allowed across the export/import boundary | Needed by `ax_thread_spawn`. On WebAssembly they are table indices |
| C25 | C FFI: `extern "C"` blocks with varargs and libraries; `export fn` is C-callable; generated headers (§14.5) | The user asked for a C FFI in both directions |
| C26 | A NUL byte follows every string literal (not part of the slice) | Found while testing the FFI: passing `"%s".ptr` to C read past the end of the literal |
| C27 | `test` blocks, `@expect`/`@expect_eq`, `axiom test`, and trap site records (§16.4, §17) | The user asked for testing support (formerly open question Q11) |
| C28 | Packages: `axiom.toml`, `pkg:` imports, path and pinned-git dependencies, one source per name (§4.6) | The user asked for packages |
| C29 | Stack traces from a per-thread shadow stack, and error return traces (§16.4) | The user asked for stack traces on crashes. Engine stack walking is neither portable nor source-exact |
| C30 | `axiom profile`: a sampling profiler over the shadow stack (§16.6) | The user asked for profiling support |
| C31 | Separate compilation of modules with a content-addressed object cache. Error codes are resolved at link time (§16.7) | The user asked for independent compilation units |
| C32 | Enums: distinct types over an integer representation, with members, methods, exhaustive `match`, `==`/`!=` and checked conversions (§5.14); `enum` is no longer reserved | The standard library passed alignments, modes and kinds as bare `u8`/`u32` constants, which any integer could be confused with |
| C33 | Thread-local globals: `threadlocal let mut` (§12.8.6). This replaces the earlier "no thread-local storage" decision | The user asked for thread-locals. WebAssembly needed a per-instance block, set up by the thread start export |

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
9. ~~**Threads and atomics.**~~ Resolved: see §12.8. New questions that follow from it:
   - **Relaxed and acquire/release orderings**, for performance on ARM. They would be
     added as extra builtins (`@atomic_load_acquire`, ...), never as defaults.
   - **Static race prevention.** A lightweight rule could forbid passing `*mut`/`[]mut`
     to `spawn` except through `std/sync` types or `parallel_for` ranges. It would be
     advisory at first, as a lint.
   - **Thread-local storage.** It needs a TLS base per WebAssembly instance.
   - **64-bit `wait`/`notify`.**
10. **FFI follow-ups** (the FFI itself is §14.5):
    - **Structs by value.** This needs each platform's C calling convention (x86-64
      SysV, AArch64, the wasm C ABI) implemented in the compiler.
    - **Native memory sandboxing vs. FFI.** A bounds-checked linear-memory region on
      native targets (§12.1) would make Axiom pointers offsets rather than
      addresses, which C can't use directly. Marshalling at the `extern` boundary
      would then be needed.
    - **Importing C headers** (`import c "stdio.h"`). This would need a C parser, which
      conflicts with "context-compact". Generating `extern` blocks with a separate
      tool is the likelier path.
11. ~~**Test blocks.**~~ Resolved: §17. Follow-ups: property-based tests (a
    `std/testing` generator module), and benchmarks (`bench "name" { … }` with
    `axiom test --bench`).
12. **Loop labels.** v0.1 has none.
13. **Relaxed SIMD.** Offer relaxed-SIMD builtins with implementation-defined results, or
    not (the current rule)?
14. **Separate compilation follow-ups** (§16.7):
    - distributing precompiled packages (objects plus an interface, instead of sources);
    - cross-module inlining at `-O2` with separate units (ThinLTO, or exporting small
      function bodies in the interface);
    - cache eviction;
    - finer invalidation, where a module's dependents are recompiled only when its
      public interface changes, not when any part of it does.
