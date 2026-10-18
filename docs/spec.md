# Axiom Language Specification

**Version:** 0.1-draft.9
**File extension:** `.ax`
**Status:** Working draft. Everything here may change. Open questions are
[GitHub issues](https://github.com/paulhankin/axiom/issues).

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

Axiom is a systems language that compiles to native code (x86-64 and AArch64) and to
WebAssembly. It is designed for programs that machine agents write, read and repair.
Each principle below is paired with the concrete rules that enforce it.

| Principle | Enforced by |
|---|---|
| **Zero hidden control flow** | No exceptions, destructors, operator overloading, implicit calls, getters, or implicit allocation. The only non-local exits are `return`, `break`, `continue`, postfix `!`, `catch … return`, and traps. `defer` is written out at the point where it is registered. |
| **Context-compact grammar** | The grammar is LL(1) and context-free. Parsing never needs a symbol table. There are no macros, no preprocessor, and no user-defined operators. Each construct has exactly one canonical surface form, enforced by `axiom fmt`. |
| **No undefined behavior outside raw pointers** | Arithmetic, shifts, casts, slices, arrays and every other checked operation have defined results or trap. The one rule a program can break unchecked is that an access through a raw pointer stays inside the object the pointer came from (§12.1): on WebAssembly the damage stays inside the module's memory, and on native targets it is undefined behavior. Compilers MUST NOT optimize on any other assumption. |
| **First-class SIMD** | Vector types of 128, 256 and 512 bits are primitives. WebAssembly's semantics are the reference semantics, on every target. |
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
only at the start of a module-level `let` (§12.8.6). Likewise `inline` has its meaning
only directly before `fn` (§9.5).

**Reserved type names.** These lex as keywords:

```
u8  u16  u32  u64  usize   i8  i16  i32  i64  isize   f32  f64   bool  void
i8x16  u8x16  i16x8   u16x8   i32x4   u32x4   i64x2  u64x2  f32x4   f64x2  v128
i8x32  u8x32  i16x16  u16x16  i32x8   u32x8   i64x4  u64x4  f32x8   f64x4  v256
i8x64  u8x64  i16x32  u16x32  i32x16  u32x16  i64x8  u64x8  f32x16  f64x8  v512
```

**Reserved constraint names** (§10.5): `Eq`, `Ord`, `Num`, `Int`, `SignedInt`,
`UnsignedInt`, `Float`, `FloatVector`. These may not be declared as identifiers.

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

FnDecl        ::= "inline"? "fn" IDENT GenericParams? "(" ParamList? ")" Type FnAttr? Block   (* §9.5 *)
FnAttr        ::= ( "@features" | "@multiversion" ) "(" STRING_LIT ( "," STRING_LIT )* ","? ")"   (* §13.4 *)
ParamList     ::= Param ( "," Param )* ","?
Param         ::= IDENT ":" Type

GenericParams ::= "[" GenericParam ( "," GenericParam )* ","? "]"
GenericParam  ::= IDENT ( ":" Constraint )? | "const" IDENT ":" Type
Constraint    ::= "Eq" | "Ord" | "Num" | "Int" | "SignedInt" | "UnsignedInt" | "Float" | "FloatVector"
```

The trailing-comma forms are LL(1). After a `,`, an `IDENT` begins another element, and
anything else ends the list. The FIRST sets of `Field` (`IDENT`) and `MethodDecl`
(`pub`, `fn`) are disjoint, except for `inline`, an identifier: `inline` followed by
`fn` begins a method, which takes one more token of lookahead (as `threadlocal let`
does).

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

IfStmt        ::= "if" ( Expr | "let" Patterns "=" Expr ) Block ( "else" ( IfStmt | Block ) )?
WhileStmt     ::= "while" Expr Block
LoopStmt      ::= "loop" Block
ForStmt       ::= "for" IDENT "in" Expr ( ".." Expr )? Block
MatchStmt     ::= "match" Expr "{" MatchArm* "}"
MatchArm      ::= ( Patterns | "else" ) "=>" Block
Patterns      ::= Pattern ( "," Pattern )*
Pattern       ::= "." IDENT ( "(" IDENT ")" )?      (* union variant *)
                | "error" "." IDENT                 (* error value *)
                | "-"? INT_LIT | CHAR_LIT | "true" | "false"

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
declarations. Paths are relative to the importing file, with `/` between their parts on
every host: an absolute path (`/a/b.ax`, `C:/a/b.ax`) or one containing `\` is `E0218`.
A path beginning with `std/` resolves to the standard library, for example
`import "std/mem" as mem;`. A path beginning with `pkg:` names a module of another package
(§4.6). The `.ax` extension may be left out. A path that doesn't resolve to a readable file
is `E0211`.

A module in a directory named `internal`, or below one, may be imported only by modules in
the tree rooted at that directory's parent, as in Go: `a/b/internal/c.ax` by `a/b/x.ax` or
`a/b/d/y.ax`, not by `a/z.ax`. Any other import of it is `E0219`. The rule applies to the
standard library too, so `std/internal/...` (the runtime among it) can be imported only from
`std`.

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
version = "0.1.0"          # optional; informational
axiom = "0.1"              # optional: the oldest compiler that builds the package
entry = "src/lib.ax"       # optional; default src/lib.ax if it exists, otherwise src/main.ax

[dependencies]
vecmath = { path = "../vecmath" }
json = { git = "https://example.com/json.git", rev = "3f1c2a9d0e5b7c4a8f6e2d1b0c9a8e7f6d5c4b3a" }
```

- An unknown section, an unknown key, a malformed line, an invalid name, or a duplicate
  dependency is `E0215`.
- `axiom` names the oldest compiler version that builds the package (`a`, `a.b` or
  `a.b.c`; a value that is not a version is `E0215`). A compiler older than that
  (`axiom version`) refuses the package with `E0220`.
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
  Distributing precompiled packages is future work ([#36](https://github.com/paulhankin/axiom/issues/36)).

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

A raw pointer is the address of a byte in memory (§12.1). It has the same size as `usize`.
Pointers are **non-nullable in the type system**: the language has no `null` literal. The
only operations on pointers are:

- dereference: `p.*`, which is a place expression and is writable only for `*mut T`
- field access with one level of auto-deref: `p.field` means `p.*.field` (§7.9)
- method calls (§9.3)
- `==` and `!=`
- the conversion builtins `@int_from_ptr`, `@ptr_from_int`, `@ptr_cast`, and
  `@slice_from_ptr`
- the offset builtins `@ptr_add` and `@ptr_sub`
- the cache hints `@prefetch` and `@prefetch_write`

There is no pointer arithmetic operator. `@ptr_add(p, n)`, with `p` a `*T` or `*mut T`
and `n` a `usize`, is the pointer `n` elements of `T` past `p`, of the same type as `p`:
the address `@int_from_ptr(p) + n * @size_of(T)` (C's `p + n`, Rust's `p.add(n)`).
`@ptr_sub(p, n)` is the pointer `n` elements before `p` (Rust's `p.sub(n)`). `T` must
not be `void`. Neither ever traps: each is the program's promise that the result stays
inside the object `p` points into (a global, a local, an array, or one block from the
allocator), or just past its end. Compilers rely on the promise to fold constant offsets
into addressing (on WebAssembly, `v128.load offset=16` rather than an add and an overflow
check). The promise is not checked. A result outside the object is a logic bug, like a
wrong `@ptr_from_int` address, but with a weaker guarantee: code compiled on the promise
may have moved or merged the accesses, so loads and stores through it break the access
rule (§12.1) and comparisons with it give unspecified results.

```axiom
let row = @ptr_add(base, r * 8);          // base: *f32x4, after checking the matrix's length
acc = acc + @ptr_add(row, 1).* * x;       // the second f32x4 of the row
```

For an offset in bytes, or one that may leave the object, use
`@ptr_from_int(*T, @int_from_ptr(p) + k)`, which is checked arithmetic on the address.

`@prefetch(p)`, with `p` any pointer, is a hint that the program will soon read the
memory at `p`: native compilers start loading its cache line into every level of the
cache (x86 `prefetcht0`, Arm `prfm pldl1keep`). It has no other effect. `p` is evaluated,
but nothing is read: whatever `p` is (past the end of memory, freed, misaligned), it never
traps, and it may compile to nothing (WebAssembly has no prefetch instruction).

```axiom
for i in @as(usize, 0)..n {
    @prefetch(&table[bucket(keys[i])]);   // start the loads, then look the keys up
}
```

`@prefetch_write(p)` is the same hint for memory the program will soon write: native
compilers fetch the line ready to be modified, so that the store that follows does not
have to ask the other cores for it again (x86 `prefetchw` where the function's extensions
include `prfchw` (§13.4), else `prefetcht0`; Arm `prfm pstl1keep`). It reads and writes
nothing, never traps, and may compile to nothing, exactly as `@prefetch` does.

### 5.8 SIMD vectors

| 128 bits | 256 bits | 512 bits | Lane type |
|---|---|---|---|
| `i8x16`, `u8x16` | `i8x32`, `u8x32` | `i8x64`, `u8x64` | 8-bit integer |
| `i16x8`, `u16x8` | `i16x16`, `u16x16` | `i16x32`, `u16x32` | 16-bit integer |
| `i32x4`, `u32x4` | `i32x8`, `u32x8` | `i32x16`, `u32x16` | 32-bit integer |
| `i64x2`, `u64x2` | `i64x4`, `u64x4` | `i64x8`, `u64x8` | 64-bit integer |
| `f32x4` | `f32x8` | `f32x16` | f32 |
| `f64x2` | `f64x4` | `f64x8` | f64 |
| `v128` | `v256` | `v512` | raw bits (no lane interpretation) |

A vector type's size and alignment are its width in bytes: 16, 32 or 64. Every operator and
builtin of §13 applies to every width, lane by lane, with the same results. Where a target's
registers are narrower (WebAssembly, AArch64's Advanced SIMD, x86-64 without AVX), a wider
vector is computed in 128-bit pieces. See §13.

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
| `@as(T, x)` | Lossless numeric widening (for example `u8 → u32`, `i16 → i64`, `u32 → i64`, `f32 → f64`), and `bool` to any integer type (`false` is 0, `true` is 1) | A conversion that can lose information is a compile error (`E0510`) |
| `@int_cast(T, x)` | Integer-to-integer conversion, checked at runtime | Traps if the value is out of range (`T0004`) |
| `@truncate(T, x)` | Integer-to-integer conversion keeping the low bits | Never fails |
| `@int_from_float(T, x)` | Truncates toward zero and **saturates** | Never fails. NaN becomes 0 (matches WASM `trunc_sat`) |
| `@float_from_int(T, x)` | Rounds to nearest, ties to even | Never fails |
| `@float_cast(T, x)` | `f64 → f32` with round-to-nearest-even | Never fails |
| `@bit_cast(T, x)` | Reinterprets bits. Sizes must match | Size mismatch is a compile error (`E0512`) |
| `@int_from_enum(x)` | An enum member's value, in the representation type (§5.14) | Never fails |
| `@enum_from_int(E, n)` | The member of enum `E` whose value is `n` | Traps if there is none (`T0007`) |
| `@int_from_ptr(p)` | Address as `usize` | Never fails |
| `@ptr_from_int(P, n)` | `usize` to pointer type `P` | Never fails. Any address is valid to form; what it may access is §12.1's rule |
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
| **M**ultiplicative | `*` `/` `%` `*%` | int, float (`%` int only) | `/` and `%` trap on divisor 0. Integer `*` wraps (§7.6) |
| **A**dditive | `+` `-` `+%` `-%` | int, float | Integer `+` and `-` wrap (§7.6). The `%`-suffixed forms always wrap, int only |
| **S**hift | `<<` `>>` | int; the count may be any unsigned int | §7.6 |
| **B**itwise | `&` `\|` `^` | int, SIMD int, `v128` | each operator is its own family: `&`, `\|`, `^` |
| **C**omparison | `==` `!=` `<` `<=` `>` `>=` | int, float; `==`/`!=` also on bool, pointer, error, fn pointer, enum | result `bool` |
| Logical-**and** | `and` | bool | short-circuit |
| Logical-**or** | `or` | bool | short-circuit |

The logical operators are the keywords `and`, `or` and `not`. That leaves `!` to error
propagation (`e!`) and error-union types (`!T`), with no lexical ambiguity between them.

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
| `a and b or c` | **E0150**: `and` and `or` are unordered ([#22](https://github.com/paulhankin/axiom/issues/22)) |
| `a < b == c` | **E0150** |

Unary operators bind tighter than all binary operators. Postfix operators bind tighter
than unary operators: `-a.b` is `-(a.b)`, and `&a[i]` is `&(a[i])`.

### 7.5 Unary operators

| Operator | Operand | Meaning |
|---|---|---|
| `-x` | signed int, float, SIMD (float or signed int lanes) | Negation. For integers, `-MIN` overflows: it is `MIN` by default (§7.6) |
| `-%x` | signed or unsigned int, SIMD int | Wrapping negation, in every overflow mode |
| `~x` | int, SIMD int, `v128` | Bitwise NOT |
| `not x` | bool | Logical NOT |
| `&x` | place | Address, of type `*T` |
| `&mut x` | mutable place | Address, of type `*mut T` |

Applying `-` to an unsigned type, or a vector of unsigned lanes, is a compile error (`E0520`).
Use `-%` or subtract from 0.

### 7.6 Integer arithmetic semantics

**Overflow wraps.** `+`, `-`, `*` and unary `-` on integers, the compound assignments
built from them (`+=`, `-=`, `*=`), and `MIN / -1` compute their result **modulo 2ⁿ**,
where `n` is the bit width: the result is the one value of the type congruent to the
mathematical result. For example, with `x: u8 = 250`, `x + 10` is `4`; with
`m: i32 = @min_value(i32)`, `-m`, `m * -1`, `m / -1` and `@abs(m)` are all `m`, and
`m - 1` is `@max_value(i32)`. This is the meaning in every build, at every optimization
level, on every target (as in Go and Java, and Rust's release builds).

`+%`, `-%`, `*%` and unary `-%` are explicit spellings of the same wrapping arithmetic.
They are still valid, and they keep wrapping in every overflow mode, so code that relies on
wrapping (hashes, checksums, random number generators) can say so and stays correct
under `--overflow=trap`. Use `@add_checked`, `@sub_checked` and `@mul_checked` (which
return `error.Overflow`) where an overflow must be detected, for example in sizes computed
from untrusted input, and `@add_sat`, `@sub_sat` and `@mul_sat` to clamp (Appendix A).

**Overflow modes.** A build may ask for another overflow mode with a compiler flag
(§16.1). They are debugging aids, not the language's meaning:

- `wrap` (the default): as above.
- `trap` (`--overflow=trap`): if the mathematical result of `+`, `-`, `*`, unary `-`,
  `MIN / -1` or `@abs(MIN)` is not representable, the program traps (`T0002`). A program
  that wraps nowhere behaves the same in both modes, so the mode finds unintended
  overflows (in tests, say) at the cost of a check on every operation.
- `saturate` (`--overflow=saturate`): the result is clamped to the type's MIN or MAX.

In every mode, the `%`-suffixed operators, the `@atomic_*` arithmetic and integer vector
arithmetic (§13.1) wrap, and the `_sat` and `_checked` builtins behave as their names say.
The overflow mode is the **only** way build configuration can change a program's meaning,
and only for a program whose arithmetic overflows. Optimization level (`-O0`…`-O3`) MUST
NOT change observable behavior.

Why wrap by default rather than trap: a check on every operation was the largest
remaining cost of compute-heavy code against the same code built without checks, and it
kept the optimizer from vectorizing reductions (WebAssembly has no overflow flag, so each
check is an extra compare and branch, and a checked multiply needs a wider one). Trapping
in debug builds and saturating in release builds was rejected because tested and shipped
programs would differ, and a saturated length silently gives wrong answers. Overflow
bugs are still caught by bounds checks where they would matter for memory, by
`--overflow=trap` in tests, and by `@*_checked` where a program must handle them.

**Division.** `/` truncates toward zero. `%` gives a result with the sign of the dividend,
so `a == (a / b) * b + (a % b)` (with wrapping arithmetic, this holds for `MIN / -1` too).
A divisor of 0 traps (`T0003`) in **every** mode. `MIN % -1` is `0`.

**Shifts.** A shift has its mathematical meaning for **every** count, including counts at or
above the bit width. Let `x` have `n` bits and let the count be `k`:

- `x << k` is `x · 2ᵏ` reduced modulo `2ⁿ`: bits shifted out of the top are discarded.
  When `k ≥ n` the result is `0`. `<<` never traps, in any overflow mode.
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

**Constant evaluation.** An integer operation whose operands are all constants is
evaluated by the compiler: in a `const` initializer or an array length, and anywhere else
(`A + 100` for a `const A: u8 = 200`). There, an overflow of `+`, `-`, `*`, unary `-` or
a conversion to the expression's type is a compile error (`E0530`) in every mode: an
expression that always overflows is almost certainly a mistake, and the check costs
nothing. Write the `%`-suffixed operators (`A +% 100`) to wrap a constant on purpose.

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
initializer. The type annotation may be omitted when the initializer's type can be
determined (§6.1).

A local's initializer may instead be `undefined`, which leaves its bytes unspecified until
they are written, as memory from the allocator (§12.3): a buffer filled before it is read
need not be cleared first.

```axiom
let mut rows: [64]usize = undefined;   // only rows[0..n] are written, and read
```

`undefined` is recognised only as the whole initializer of a `let` (elsewhere it is an
ordinary identifier). The binding must be `let mut` (`E0413`: it could never be written)
and must have a type annotation (`E0414`). Reading its bytes before they are written gives
unspecified values, never poison (§12.3); an implementation may give them any values,
zeroes included (it may zero a variable small enough to be kept in registers).

### 8.2 Expression statements and discards

An expression statement's value must be `void`. Discarding any other value is a compile
error (`E0620`). To throw a value away deliberately, write `_ = f();`.

Discarding an error union with `_ =` is also rejected (`E0621`). Handle it with `!` or
`catch` first. This makes it impossible to ignore an error silently.

### 8.3 `if`

The condition must be `bool`; Axiom has no truthiness. Braces are required. `else if`
chains are written as `else if …`.

`if let P = e { A } else { B }` tests `e` against the patterns `P` of a `match` arm
(§8.6): it is exactly `match e { P => { A } else => { B } }`, so the scrutinee and the
patterns follow the rules of `match`, and a binding `.Variant(x)` is in scope in `A` only.
The `else` part is optional, and may be another `if` or `if let`.

```axiom
if let .Circle(c) = shape {
    total += c.radius;
}
```

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
- An arm may list several patterns, separated by commas: `.Sat, .Sun => { … }`. It is
  taken when any of them matches. Such an arm cannot bind a payload (`E0635`), and
  `else` stands alone.
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

### 9.5 Inline functions

```axiom
inline fn bit(x: u64, k: u32) u64 { return (x >> k) & 1; }
```

A function (or method) declared `inline` is **always inlined**: every direct call is
replaced by the function's body (at every optimization level), so a small helper called
with constant arguments costs what the same code written out would, the way macros or
`#[inline(always)]` are used elsewhere. It changes no behavior: traps, overflow checks
and stack traces are the same.

- An inline function cannot be recursive, directly or through other inline functions
  (`E0314`, reported at each call on the cycle). Recursion through a function that is not
  inline is allowed.
- It cannot be `export`ed, and cannot have `@features` or `@multiversion` (`E0313`).
- A direct call compiles it with its caller's processor extensions (§13.4): called from a
  function with `@features`, or from a version of a `@multiversion` function, its body is
  compiled for those extensions too, and `@target_has` inside it answers for them. Used as
  a value, it has the build's extensions.
- It may be generic, and it may be used as a value (§9.4); a call through a function
  pointer is an ordinary call.
- Stack traces (§16.4.1) keep its frames. A direct call to it does not record its call
  site; the site is passed to the inlined body, which records it only on the paths that
  need it, before a trap or before calling a function that is not inline. On every other
  path the call costs nothing. (With frame tables, §16.4.1, nothing is recorded at run
  time at all: the inlined code's frames come from the table.) A fault that is not a check (an out-of-bounds raw pointer
  access, say) inside an inlined body is reported as if it were in the caller.
- With separate compilation (§16.7), every module that calls an inline function compiles
  its own copy.
- The self-hosted compiler inlines such calls wherever its inliner can (at `-O1` and
  above), and keeps ordinary call-site records.

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
Constraint    ::= "Eq" | "Ord" | "Num" | "Int" | "SignedInt" | "UnsignedInt" | "Float" | "FloatVector"
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
([#25](https://github.com/paulhankin/axiom/issues/25)).

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
| `FloatVector` | the float vector types (`f32x4` … `f32x16`, `f64x2` … `f64x8`) | binary `+` `-` `*` `/`; unary `-`; `@min`, `@max`, `@abs`, `@sqrt`, `@fma`; `@load(T, s)` and `@store(s, v)`; `@splat(lit)` of a float literal (see below) |

**Literals in generic code.** An untyped literal is accepted as a value of type `T` only if
it is representable in **every** type that satisfies `T`'s constraint:

| Constraint | Literals accepted |
|---|---|
| `Num`, `Int` | integers `0` … `127` |
| `SignedInt` | integers `-128` … `127` |
| `UnsignedInt` | integers `0` … `255` |
| `Float` | any float literal, and integers exactly representable in `f32` |
| `FloatVector` | none; `@splat` takes a float or integer literal, converted to the lane type when instantiated |

A `FloatVector` parameter's lane count and lane type are not known in the body, so
operations that depend on them (lane access, shuffles, comparisons, reductions) are not
permitted; `@size_of(T)` gives the width. For example, the same code at any width:

```axiom
fn scale[V: FloatVector](xs: []mut V, k: V) void {
    let half: V = @splat(0.5);
    for i in 0..xs.len {
        xs[i] = xs[i] * k * half;
    }
}
```

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

### 12.1 Objects, pointers and the access rule

Memory holds **objects**: globals and string literals, locals whose address is taken,
blocks from the allocator (§12.4), pages from `@memory_grow`, and memory that C code or the
host hands over. Within an object, memory is plain bytes:

- A load of `k` bytes at address `a` reads the bytes from `a` up to `a + k` as
  little-endian, and a store writes them. Any type may be read from any bytes: **type punning** through
  `@ptr_cast` is defined. Loads of `bool` normalize (§5.3), and invalid union tags trap at
  `match` (§5.10).
- Misaligned accesses are defined (§12.5).
- There is no strict-aliasing rule: compilers MUST NOT assume that two pointers do not
  alias because of their types, and two pointers into one object may alias freely.

A pointer is the address of a byte, the size of `usize`. It **comes from** an object
when it is made from that object: by `&` (or a slice of it), as a slice's `.ptr`, from
the allocator, or from such a pointer by `@ptr_add`, `@ptr_sub` or `@ptr_cast`. A
pointer made from an integer (`@ptr_from_int`) comes from any object whose address the
program has **exposed**: stored in memory, passed to or returned from a function, or
converted to an integer with `@int_from_ptr`.

**The access rule.** A load or store through a pointer touches only the object the
pointer comes from, and only while that object exists: a global or string literal
always, a local until its function returns, an allocator block until it is freed. Nothing
writes a `let` global (one without `mut`) or a string literal.

Compilers rely on the rule, and on nothing else. They may assume that accesses through
pointers from different objects do not overlap, that a local whose address is never
exposed is reached only through the function's own pointers to it, that what the
allocator returns is a new object no earlier pointer points into, and that constant data
does not change.

The checked operations never break the rule: indexing and slicing (§7.8), `@load` and
`@store` on slices, `@mem_copy`, `@mem_set` and `@mem_eq`, and `for` over a slice all trap
first. Only a raw pointer can break it: one that dangles (its object freed, or its
function returned), one moved outside its object by `@ptr_add` (§5.7) or made from a
wrong address, or a slice made by `@slice_from_ptr` with the wrong length. Code in C
(§14.5.5) can break it too. What happens then depends on the target:

- **WebAssembly.** All memory is the module's one linear memory, `M[0 .. size)`. An
  access that reaches past `size` traps (`T0005`). Any other access reads or writes
  whatever bytes are at that address, which may belong to another object, the stack or
  the allocator: the results are unspecified from then on, and since the compiler relied
  on the rule they need not match a naive reading of the code. But the damage stays in
  linear memory. Code, return addresses and the host are outside it, and calls through
  function pointers are checked, so control flow is never hijacked.
- **Native targets.** Pointers are machine addresses (§14.5.5), and accesses through
  them are not checked. Breaking the rule is **undefined behavior**: an access may reach
  another object, the allocator's own data, or the native stack with its return
  addresses, and so may change anything the process does. An access to unmapped memory
  (address 0 among it) stops the program with a crash report and a stack trace (§16.4);
  in a test binary it is reported as `T0005`.

There is no null: address 0 is not an object on any target. A pointer from C that may be
NULL is checked with `std/c.is_null` (§14.5.4). `--sanitize=address` (§16.8) finds the
commonest breaches of the rule, use after free and accesses outside a heap block, and
traps on them with `T0012`.

### 12.2 Where values live

| | WebAssembly | Native targets |
|---|---|---|
| Globals and string literals | data segments of linear memory, all of them writable | the executable's data sections. `let` globals and string literals are read-only (a write faults), except, on Linux and macOS, constants that hold addresses, which the loader relocates |
| Locals whose address is never taken | WebAssembly locals or registers; no pointer reaches them | registers or the stack frame; no pointer reaches them |
| Locals whose address is taken (`&x`, `&mut x`, a slice of an array local, an implicit `&` in a method call) | the thread's stack in linear memory (the main thread's is the lowest 1 MiB) | the native stack frame |
| Allocator blocks and `@memory_grow` pages | linear memory | one region of address space, reserved at the first `@memory_grow` (up to 1 TiB) |
| Memory from C or the host | linear memory, which C code linked into the module shares | anywhere in the process |

After a function returns, its locals' storage is reused, and a pointer to one of them
dangles. The compiler SHOULD warn when such a pointer escapes (`W0710`).

### 12.3 No uninitialized values

Every local and global has an initializer. Memory obtained from the allocator, and a
local initialized with `undefined` (§8.1), has unspecified contents but is never
"poison".

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

There is one allocator rather than an allocator parameter on every container and every
function that allocates. Such a parameter spreads through every API, and choosing among
allocators is a decision agents get wrong more often than they gain from it. Allocation
still stays explicit: it is always a call, never a language construct.

### 12.5 Alignment

Alignment affects **layout only**. Misaligned loads and stores are defined, as in
WebAssembly, and may simply be slower. `@ptr_cast` and `@cast_slice` therefore never trap
on alignment. `@align_of(T)` reports the natural alignment used for layout.

### 12.6 Concurrency

Programs may run several threads over the same memory. §12.8 defines threads,
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
| 3 | **Only sequentially consistent atomics.** There are no memory-order parameters. | Every WebAssembly atomic is sequentially consistent. SC is the model people and agents reason about correctly, and weaker orderings are a common source of subtle bugs. The cost is some performance on weakly ordered native CPUs. Relaxed orderings may come later ([#27](https://github.com/paulhankin/axiom/issues/27)). |
| 4 | **Atomics are builtins on pointers to integers** (`@atomic_add(&mut x, 1)`), not special types. | Any integer in memory can be accessed atomically, as in WebAssembly. `std/sync.Atomic[T]` wraps them for convenience. |
| 5 | **`wait`/`notify` operate on 32-bit integers only.** | Linux futexes, the native implementation, are 32-bit, and WebAssembly has `wait32`. 64-bit waiting would behave differently on different targets. |
| 6 | **No `async`/`await`, no coroutines.** | An `await` is a suspension point that looks like an ordinary expression, which is hidden control flow. Blocking calls on threads are explicit. |
| 7 | **A trap in any thread terminates the whole program.** | A trap means the program's state is no longer trustworthy. Letting other threads continue would let them observe that state. |
| 8 | **Deadlock is not an error.** A thread blocked forever is simply blocked. | Detecting it would require a runtime scheduler. |
| 9 | **Thread-local globals, mutable only** (`threadlocal let mut`, §12.8.6). | Per-thread state (an allocator cache, a random generator, a log buffer) otherwise has to be threaded through every call or locked. An immutable thread-local would just be a constant. |

#### 12.8.2 Memory model

- All threads share all memory (§12.1, §12.2). Each thread has its own stack.
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
| `@atomic_add`, `_sub`, `_and`, `_or`, `_xor`, `_xchg`, `_min`, `_max` `(p, v)` | the **previous** value. `add` and `sub` wrap: they never trap, in any overflow mode |
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
thread, which then accesses the first thread's copy. Such a pointer dangles once its
thread has ended (§12.1).

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
| `+` `-` `*` | lane-wise IEEE | lane-wise, wrapping in every overflow mode (`*` not for 8-bit lanes; WASM has no `i8x16.mul`). `@add_sat`/`@sub_sat` saturate | no |
| `/` | lane-wise IEEE | no | no |
| `+%` `-%` `*%` | no | the same as `+` `-` `*` | no |
| `&` `\|` `^` `~` | no | yes | yes |
| `<<` `>>` | no | an unsigned count: a scalar for every lane, or a vector of the same shape with one count per lane (`v << w`: lane i shifted by `w[i]`). The §7.6 semantics apply per lane (`k ≥` lane width gives `0` or sign fill) | no |
| unary `-` | yes | signed lanes, wrapping; `-%` on any lanes | no |
| `==` `<` … | no. Use `@lanes_eq`, `@lanes_lt`, … | no | no |

Integer vector arithmetic wraps, like scalar arithmetic by default (§7.6). It wraps under
`--overflow=trap` and `--overflow=saturate` too: checking every lane would be expensive,
and WebAssembly and most processors have no lane-wise overflow flags. `+%`, `-%` and
`*%` on integer vectors mean the same as `+`, `-` and `*`.

Scalars are **never** broadcast implicitly. Write `@splat(x)`.

### 13.2 Vector builtins

Appendix A has full signatures.

- Construction: `@splat`, `@vec` (from lane values).
- Lanes: `@extract_lane`, `@replace_lane`. The lane index must be a constant expression.
- Memory: `@load(V, s)` / `@store(s, v)`. `s` is a slice, and the program traps if
  `s.len * size_of(elem)` is less than the vector's size (`T0006`).
  `@load_partial(V, s)` reads lanes `0..min(s.len, n)` of an `n`-lane vector from a slice
  of its lane type, the others zero, and `@store_partial(s, v)` writes those lanes: neither
  touches memory past `s`, so a loop over vectors needs no scalar loop for its tail. x86
  does them with masked moves.
- Rearrangement: `@shuffle(a, b, [i0, …])` takes constant indices into the lanes of `a`
  then `b`; their count sets the result's lanes (any count making a 128-, 256- or 512-bit
  vector), so `@shuffle(a, zero, [0, …, 7, 8, …, 8])` widens an `f32x8` to an `f32x16`
  with zeros and `@shuffle(a, a, [8, …, 15])` takes the high half of an `f32x16`.
  `@swizzle(a, idx)`
  takes runtime indices: `a` is a vector of any lanes and `idx` an integer vector with as
  many lanes of the same width, taken as unsigned; lane i of the result is `a[idx[i]]`, or
  0 when `idx[i]` is not a lane of `a`. `@swizzle2(a, b, idx)` is the same over the `2n`
  lanes of `a` then `b`, so a table of 32 `f32` is two `f32x16` (x86: `vpermps`,
  `vpermt2ps`, and their kin for other lane widths; elsewhere byte lookups such as
  `i8x16.swizzle`, `pshufb` and `tbl`).
- Comparison: `@lanes_eq`, `@lanes_ne`, `@lanes_lt`, `@lanes_le`, `@lanes_gt`,
  `@lanes_ge`. Each returns an all-ones or all-zeros integer vector of the same shape.
- Selection and tests: `@select(mask, a, b)`, `@any_true`, `@all_true`, `@bitmask`.
- Reductions: `@reduce_add(v)` and `@dot(a, b)`. For floats, the association order is
  fixed as **folding the halves**: the high half is added to the low half, lane `i` plus
  lane `i + n/2`, and so on until one lane is left. For 4 lanes that is
  `(l0 + l2) + (l1 + l3)`, for 2 lanes `l0 + l1`, and for 16 lanes lanes `8..16` onto
  `0..8`, then `4..8` onto `0..4`, `2..4` onto `0..2` and lane 1 onto lane 0 (the order
  of x86's `_mm512_reduce_add_ps`). The result is therefore deterministic across targets,
  and it is the shortest sequence on every machine (no horizontal adds).
- Lane-wise math: `@min`, `@max`, `@abs`, `@sqrt`, `@floor`, `@ceil`, `@trunc`,
  `@nearest`, `@fma`; on integer vectors `@clz`, `@ctz` and `@popcount`.
- Conversions: `@float_from_int(F, v)` and `@int_from_float(I, v)` convert lane by lane
  between vectors of the same lane count, with the scalar rules (round to nearest;
  truncate and saturate, NaN to 0).
- Lane width: `@widen_low(v)` and `@widen_high(v)` take the low or high half of the lanes
  of an integer vector with 8-, 16- or 32-bit lanes and extend each to twice its width,
  sign- or zero-extended by the lane type, keeping the vector's size: `u8x16 → u16x8`,
  `i32x8 → i64x4` (WebAssembly's `extend_low`/`extend_high`, x86's `pmovsx`/`pmovzx`).
  `@narrow(T, a, b)` is the reverse: `a` and `b` are integer vectors of one type with
  16-, 32- or 64-bit lanes, and `T` an integer vector with twice as many lanes of half the
  width; the result holds the lanes of `a` then those of `b`, each saturated to `T`'s
  lane range, the source lanes read by their own type. So `@narrow(u8x16, a, b)` of two
  `i16x8` clamps each lane to `0..255` (WebAssembly's `i8x16.narrow_i16x8_u`, x86's
  `packuswb`), and of two `u16x8` clamps it to `..255`.

### 13.3 Native targets

On native targets, vector types lower to the processor's vector registers (SSE2/AVX/AVX-512
or NEON), in pieces where a type is wider than they are. The semantics are the WASM SIMD
semantics, including saturation behavior and NaN rules, except that shift counts follow
§7.6 rather than WASM's modulo rule. The compiler inserts fix-ups wherever host
instructions differ.

A 256- or 512-bit vector is passed to and returned from a function in memory, like a
struct larger than 32 bytes: the caller passes the address of a copy, and a result is
written through an address the caller passes. So functions compiled for different
processor extensions (§13.4) agree on how such vectors arrive. Only 128-bit vectors may
cross the export and import boundary (§14.2).

### 13.4 Processor extensions

A build targets an architecture and a set of **processor extensions** the code may use,
chosen with `--cpu` and `--features` (§16.1). An extension changes how operations are
lowered, never what a program computes: every operator and builtin of §13.1-13.2 has the
same result with or without one. Extensions have the names `--features` takes:

| Architecture | Extensions |
|---|---|
| `x86_64` (SSE2 is the baseline) | `sse3`, `ssse3`, `sse4.1`, `sse4.2`, `popcnt`, `lzcnt`, `bmi`, `bmi2`, `avx`, `avx2`, `fma`, `cx16`, `aes`, `pclmul`, `sha`, `f16c`, `movbe`, `adx`, `avx512f`, `avx512cd`, `avx512bw`, `avx512dq`, `avx512vl`, `avx512ifma`, `avx512vbmi`, `avx512vbmi2`, `avx512vnni`, `avx512bitalg`, `avx512vpopcntdq`, `avx512bf16`, `avx512fp16`, `gfni`, `vaes`, `vpclmulqdq`, `avxvnni`, `sha512`, `sm3`, `sm4`, `avx10.1`, `prfchw` |
| `aarch64` (FP and Advanced SIMD are the baseline) | `lse`, `rcpc`, `dotprod`, `cssc`, `aes` (with PMULL), `sha2` (SHA-1 and SHA-256), `sha3`, `sha512`, `sm4` (SM3 and SM4), `crc`, `rdm`, `fp16`, `fhm`, `bf16`, `i8mm`, `sve`, `sve2`, `dit`, `sme` |
| `wasm32`, `wasm64` | `simd128`, `bulk-memory`, `nontrapping-fptoint`, `sign-ext`, `mutable-globals`, `atomics`, `multivalue`, `relaxed-simd`, `tail-call` |

An extension implies those it builds on (`avx2` implies `avx`, which implies `sse4.2`, and
so on; `avx512bw` implies `avx512f`, which implies `avx2`, `fma` and `f16c`), and removing
one with `--features=-x` removes the extensions that need it.

**Target conditions.** These builtins are constant `bool`s:

- `@target_has("ext")`: whether the code being compiled may use extension `ext`, that is,
  the build's extensions plus those of the enclosing function (`@features`, or the version
  of a `@multiversion` function being compiled, and for an `inline` function, those of the
  function it is inlined into, §9.5). A function that is not inline has the build's
  extensions whichever function calls it. `ext` is a string literal naming an
  extension of any architecture (`E0827` otherwise); another architecture's extension is
  simply absent.
- `@target_arch("x86_64")`: the architecture, one of `wasm32`, `wasm64`, `x86_64`, `aarch64`.
- `@target_os("linux")`: the operating system of a native target, one of `linux`, `macos`,
  `windows`, or `none` for WebAssembly (`os.system()` in std/os is the run-time answer:
  under WebAssembly, the host's).

A **target condition** is one of these, `true` or `false`, or `not`, `and` and `or` of target
conditions (with parentheses). An `if` whose condition is a target condition compiles only
the branch the condition selects. The other branch is still parsed and type-checked, but no
code is generated for it, and it may use builtins that the code is not compiled for (below).
So a portable fallback costs nothing where the faster path applies:

```axiom
fn crc32c(crc: u32, data: []u8) u32 {
    if @target_arch("x86_64") and @target_has("sse4.2") {
        return crc32c_hw(crc, data);    // uses @x86_crc32c
    }
    return crc32c_table(crc, data);
}
```

**Run-time detection.** `@cpu_has("ext")` is a `bool`: whether the processor running the
program has extension `ext`. It is the constant `true` where `@target_has("ext")` is (and
then also counts as a target condition), and always `false` under WebAssembly, where
programs cannot ask. Otherwise it asks std/os/cpu (`cpu.features()`, detected once:
`cpuid` on x86-64, the system on AArch64). Every build includes std/os/cpu for this.

**Functions compiled for more extensions.** A function declared with an attribute after
its return type is compiled for extensions the build does not assume:

- `fn f(...) T @features("ext,ext") { ... }` compiles the body with these extensions added.
  A call checks the processor first and traps with `T0013` if it lacks one (the check
  disappears where the build already has them). Call it after `@cpu_has`, or from a
  function with the same extensions.
- `fn f(...) T @multiversion("ext,ext", "ext", ...) { ... }` compiles the body once for
  each listed set of extensions, and once for the build's own. A call runs the first
  version, in the order listed, whose extensions the processor has, and the build's own
  version otherwise. Within each version, `@target_has` answers for that version.

Each string is a comma-separated list of extensions (a leading `+` is allowed). A set
naming an extension of another architecture describes no version on this target: such a
`@multiversion` version is left out, and an `@features` function always traps. A function
has at most one attribute (`E0829`, as for an unknown attribute, no sets or an empty set).
Every version is shown under the function's name in stack traces and profiles. The
versions share one calling convention: a value passes between functions compiled for
different extensions as it does between any two functions.

**Builtins that need an extension.** Builtins that stand for one processor's instructions
(`@x86_*`, `@arm_*`) exist only where the architecture and extensions they need are
available: using one in code that is compiled (not in a branch a target condition rules
out) for a target or function without them is `E0828`.

| Builtin | Needs | Result |
|---|---|---|
| `@x86_cpuid(leaf, subleaf)` | `x86_64` | `u32x4`: `eax`, `ebx`, `ecx`, `edx` of the `cpuid` instruction |
| `@x86_xgetbv(xcr)` | `x86_64` | `u64`: the extended control register (`xgetbv`; only when `cpuid` reports OSXSAVE) |

The cryptographic instructions of §13.5 are builtins of this kind.

**Start-up check.** A native program built for more than the architecture's baseline (any
`--cpu` but the baseline, including the default `--target=native`, which means this
machine's processor) checks the processor before `main` and exits with status 1 and a
message naming the missing extensions, instead of failing on an unsupported instruction:

```
axiom: this program was built for processors with extensions this one lacks (--cpu=x86-64-v4); missing: avx512f avx512cd avx512bw avx512dq avx512vl
```

### 13.5 Processor instructions for cryptography

These builtins are single instructions of one architecture: AES rounds, carry-less
multiplication, SHA rounds and message schedules, and CRC steps. Each needs its
architecture and extension (`E0828`, §13.4), so they appear under a target condition or
in a function with `@features` or `@multiversion`, next to a portable fallback. Their
results are exactly those of the instruction (Intel's and Arm's manuals define them).
Operand types are checked (`E0812`); an immediate must be a constant in range (`E0820`).

In the table, `B` is `u8x16`, `D` is `u32x4`, `Q` is `u64x2`, and `V` is any 128-bit
integer vector (the result has its type).

| Builtin | Needs | Signature | Instruction |
|---|---|---|---|
| `@x86_aesenc(s, k)`, `@x86_aesenclast`, `@x86_aesdec`, `@x86_aesdeclast` | `x86_64`, `aes` | `B, B → B` (also `u8x32`, `u8x64`: each 128-bit part) | one AES round (`aesenc`, ...) |
| `@x86_aesimc(k)` | `x86_64`, `aes` | `B → B` | InvMixColumns of a round key |
| `@x86_aeskeygenassist(k, rcon)` | `x86_64`, `aes` | `B, const u8 → B` | key expansion assist |
| `@x86_clmul(a, b, imm)` | `x86_64`, `pclmul` | `Q, Q, const u8 → Q` (also `u64x4`, `u64x8`) | 64×64 → 128-bit carry-less product of the halves `imm` selects: bit 0 for `a`, bit 4 for `b` (`pclmulqdq`) |
| `@x86_sha1rnds4(abcd, e, f)` | `x86_64`, `sha` | `D, D, const 0..3 → D` | four SHA-1 rounds with function `f` |
| `@x86_sha1nexte(a, b)`, `@x86_sha1msg1`, `@x86_sha1msg2` | `x86_64`, `sha` | `D, D → D` | SHA-1 E and message schedule |
| `@x86_sha256rnds2(cdgh, abef, wk)` | `x86_64`, `sha` | `D, D, D → D` | two SHA-256 rounds (the words in the low half of `wk`) |
| `@x86_sha256msg1(a, b)`, `@x86_sha256msg2` | `x86_64`, `sha` | `D, D → D` | SHA-256 message schedule |
| `@x86_crc32c(crc, x)` | `x86_64`, `sse4.2` | `u32, u8/u16/u32/u64 → u32` | CRC-32C of `x`'s bytes (reflected, not inverted: `crc32`) |
| `@arm_aese(s, k)`, `@arm_aesd` | `aarch64`, `aes` | `B, B → B` | AddRoundKey, SubBytes, ShiftRows (or their inverses) |
| `@arm_aesmc(s)`, `@arm_aesimc` | `aarch64`, `aes` | `B → B` | MixColumns (or its inverse) |
| `@arm_pmull(a, b)`, `@arm_pmull2` | `aarch64`, `aes` | `Q, Q → Q` | carry-less product of the low (`pmull2`: high) halves |
| `@arm_sha1c(abcd, e, wk)`, `@arm_sha1p`, `@arm_sha1m` | `aarch64`, `sha2` | `D, u32, D → D` | four SHA-1 rounds: choose, parity, majority |
| `@arm_sha1h(e)` | `aarch64`, `sha2` | `u32 → u32` | rotate left by 30 |
| `@arm_sha1su0(a, b, c)`, `@arm_sha1su1(a, b)` | `aarch64`, `sha2` | `D, D, D → D`; `D, D → D` | SHA-1 message schedule |
| `@arm_sha256h(abcd, efgh, wk)`, `@arm_sha256h2(efgh, abcd, wk)` | `aarch64`, `sha2` | `D, D, D → D` | four SHA-256 rounds |
| `@arm_sha256su0(a, b)`, `@arm_sha256su1(a, b, c)` | `aarch64`, `sha2` | `D, D → D`; `D, D, D → D` | SHA-256 message schedule |
| `@arm_sha512h(a, b, c)`, `@arm_sha512h2` | `aarch64`, `sha512` | `Q, Q, Q → Q` | two SHA-512 rounds |
| `@arm_sha512su0(a, b)`, `@arm_sha512su1(a, b, c)` | `aarch64`, `sha512` | `Q, Q → Q`; `Q, Q, Q → Q` | SHA-512 message schedule |
| `@arm_eor3(a, b, c)` | `aarch64`, `sha3` | `V, V, V → V` | `a ^ b ^ c` |
| `@arm_bcax(a, b, c)` | `aarch64`, `sha3` | `V, V, V → V` | `a ^ (b & ~c)` |
| `@arm_rax1(a, b)` | `aarch64`, `sha3` | `Q, Q → Q` | `a ^ @rotl(b, 1)` in each lane |
| `@arm_xar(a, b, k)` | `aarch64`, `sha3` | `Q, Q, const 0..63 → Q` | `@rotr(a ^ b, k)` in each lane |
| `@arm_crc32(crc, x)`, `@arm_crc32c` | `aarch64`, `crc` | `u32, u8/u16/u32/u64 → u32` | CRC-32 (or CRC-32C) of `x`'s bytes, reflected, not inverted |

The 256- and 512-bit forms of `@x86_aesenc` and its kin and of `@x86_clmul` act on each
128-bit part. They are single instructions where the build has `vaes` (or `vpclmulqdq`),
and one instruction per part otherwise. On AArch64 `@x86_*` builtins do not exist, and on
x86-64 `@arm_*` ones do not, so code for both architectures keeps each under its own
target condition:

```axiom
fn crc32c_step(crc: u32, x: u64) u32 @multiversion("sse4.2", "crc") {
    if @target_arch("x86_64") and @target_has("sse4.2") {
        return @x86_crc32c(crc, x);
    }
    if @target_arch("aarch64") and @target_has("crc") {
        return @arm_crc32c(crc, x);
    }
    return crc32c_table(crc, x);
}
```

The standard library uses them in std/crypto (`aes_gcm`, `sha1`, `sha2`, `sha3`) and
std/hash/crc, with the portable code where they are absent.

---

## 14. Targets and ABI

### 14.1 Targets

| Target | `usize` | Notes |
|---|---|---|
| `wasm32-freestanding` | 32 | No WASI and no imports except those declared |
| `wasm64-freestanding` | 64 | memory64 |
| native (`x86_64-*`, `aarch64-*`) | 64 | Allocator memory is a region of address space reserved at the first `@memory_grow` (up to 1 TiB), whose 64 KiB pages are committed as it grows. Pointers are machine addresses, and accesses through them are not checked (§12.1) |
| `aarch64-macos` (also `aarch64-apple-darwin`, `macos`) | 64 | Apple silicon, macOS 11 or later; self-hosted compiler only. A position-independent Mach-O executable linked against `libSystem` and given an ad-hoc code signature, so it runs without Xcode. Memory is as for native. Not yet supported: `axiom test` and `axiom profile` for this target, linking C code or `-l` libraries |
| `x86_64-windows` (also `x86_64-pc-windows-msvc`, `windows`) | 64 | Windows 10 or later on x86-64; self-hosted compiler only. A position-independent PE32+ console executable importing from the system's DLLs (`kernel32`, `ntdll`, `ws2_32`, `bcrypt`); no C runtime is needed. Memory is as for native. `axiom test` runs each test in a process of its own (Windows cannot fork), under Wine on other systems. Debug symbols (function names and line tables) go in `<out>.pdb` (`--pdb=off`: none). `--icon=<file.ico>` builds an icon into the executable's resources. Not yet supported: linking C code or `-l` libraries |

Linux is the native operating system. `--target=native` on a Mac means
`aarch64-macos`, and on Windows `x86_64-windows`; elsewhere they are cross targets,
so a Linux host can build Mac and Windows executables. On macOS, variadic C arguments
are passed on the stack (Apple's arm64 ABI), and a thread-local global (§12.8.6)
lives in a per-thread block whose address is kept in a pthread key. On Windows,
every function uses the Windows x64 calling convention (four register arguments,
shadow space, `rsi`, `rdi` and `xmm6`-`xmm15` preserved), and thread-locals are the
image's TLS directory. An `extern "C" "name"` block (§14.5.1) names the DLL its
functions come from (`name.dll`) rather than a library to link. A program can ask which system it runs on with `os.system()`
(std/os); paths on Windows may begin with a drive (`C:/`), which `path.is_abs`
recognizes there.

### 14.2 Exports

`export fn` produces a WebAssembly export with the same name. Parameter and return types
must be one of the following (`E0801` otherwise):

- `bool`,
- an integer type (8-bit and 16-bit types are passed as `i32`, zero- or sign-extended),
- `f32`, `f64`,
- `usize`/`isize`,
- a pointer,
- a function pointer (a table index on WebAssembly) whose own parameter and return types
  are allowed here (C calls it with its calling convention),
- a 128-bit SIMD type,
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
- On native targets, Axiom's pointers are machine addresses, which is what makes them
  interchangeable with C at no cost, and C code can damage anything in the process, as
  an Axiom access that breaks §12.1's rule can.

## 15. Standard Library Core

Only the parts that the language semantics depend on are specified here.

### 15.1 `std/mem`: the global allocator

```axiom
pub fn alloc[T](n: usize) ![]mut T          // contents unspecified
pub fn alloc_zeroed[T](n: usize) ![]mut T   // every byte 0
pub fn free[T](s: []mut T) void
pub fn realloc[T](s: []mut T, n: usize) ![]mut T
pub fn dup[T](s: []T) ![]mut T
pub fn alloc_aligned[T](n: usize, align: usize) ![]mut T          // at a multiple of align
pub fn alloc_zeroed_aligned[T](n: usize, align: usize) ![]mut T
pub fn dup_aligned[T](s: []T, align: usize) ![]mut T
pub fn new[T]() !*mut T                    // one value, contents unspecified
pub fn new_zeroed[T]() !*mut T             // one value, every byte 0
pub fn create[T](v: T) !*mut T             // one value, initialized to v
pub fn destroy[T](p: *mut T) void          // frees a value from new, new_zeroed or create
pub fn alloc_bytes(size: usize) usize        // 0 if there is no memory (or size is 0)
pub fn alloc_aligned_bytes(size: usize, align: usize) usize
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
  double frees corrupt its state, so that later blocks may overlap, and break the access
  rule (§12.1).
- **Failure.** `alloc`, `realloc`, `dup`, `new` and `create` return `error.OutOfMemory` when the
  system has no more memory or the byte count overflows. `realloc` leaves the old slice
  valid when it fails.
- **Alignment.** Memory is aligned to 16 bytes, enough for every type (§12.5). The
  `_aligned` functions ask for more: `align` is a power of two up to 4096 (anything else
  traps), and the memory starts at a multiple of it, for data read with wide vector loads
  (64 bytes for `f32x16`) or by the page. It is freed with `free` like any other;
  `realloc` keeps only the usual 16.
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
*Axiom host ABI*). Implementations exist for node (`std/internal/runtime/host.mjs`) and POSIX
(`std/internal/runtime/native_host.c` in C, `std/internal/runtime/native.ax` in Axiom). A program that uses only the language and `std/mem`, `vec`,
`hash`, `map` and `matrix` needs no host at all.

One host function has no counterpart on WebAssembly: `ax_fork() i32` copies the
process, returning 0 in the copy and the copy's handle (for `ax_proc_wait`) in the
original, or a negative error where the host cannot copy a process (node returns
`-38`). The self-hosted compiler uses it to compile modules in parallel (§16.7), and
compiles the program as one unit where it is missing. std does not use it.

The self-hosted compiler itself uses only the host ABI and std, so it builds for every
target, WebAssembly included: `node std/internal/runtime/host.mjs axiom.wasm -- build app.ax`
produces the same bytes as the native compiler.

The Axiom runtime uses no C library. It reaches the kernel through system calls
(`std/internal/runtime/linux.ax`), starts threads with `clone` and sets up their thread-local
storage itself, starts processes with `fork` and `execve` (searching `PATH` itself), and resolves host names from `/etc/hosts` and by DNS queries to the
servers in `/etc/resolv.conf`. So a native program built by the self-hosted toolchain
is a static executable with no dynamic loader, unless it links C code or libraries,
in which case it is linked against the C library as before.

---

## 16. Toolchain and Agent Diagnostic Interface

### 16.1 Commands

```
axiom build <file.ax> --target=<t> [-O0..-O3] [--overflow=wrap|trap|saturate] -o <out>
                          [--sanitize=address]    # check heap accesses (§16.8)
                          [--cpu=<preset>] [--features=+ext,-ext]   # processor extensions (§13.4)
                          [-g | --debug-info=lines|none]            # DWARF line tables (below)
axiom check <file.ax>                  # parse and type-check only; no codegen
axiom fmt   <file.ax|dir>... [--check] [--stdout]   # canonical layout (§16.5)
axiom test  <file.ax> [--filter=text] [--timeout=ms]   # run test blocks (§17)
axiom new   <name>                     # create a package skeleton (§4.6)
axiom profile <file.ax> [-- args]      # sampling profiler (§16.6)
axiom cache [path|clean]               # the compiled-module cache (§16.7)
axiom lsp                              # the language server, over standard input and output
axiom version                          # the compiler's version
```

`build`, `check` and `test` also accept a package directory, or no path at all for the
package in the current directory (§4.6.4).

Every command accepts `--format=human` (the default) or `--format=agent-json`.

`--overflow=wrap|trap|saturate` selects the overflow mode (§7.6). `wrap`, the default, is
the language's meaning; `trap` checks every `+`, `-`, `*`, unary `-`, `MIN / -1` and
`@abs` on integers and traps (`T0002`) on overflow, to find unintended overflows; `saturate`
clamps. `build`, `test` and `profile` accept it. `--release` is accepted for
compatibility and changes nothing.

`-g` (or `--debug-info=lines`) adds DWARF line tables to the output: a compile unit per
object, a subprogram per function and a source line and column for each instruction,
so that native profilers (`perf annotate`, `perf report --sort srcline`), debuggers and
`addr2line -i` map machine code back to Axiom source, inlined functions included
(`DW_TAG_inlined_subroutine`). It is off by default (`--debug-info=none`), at every
optimization level: stack traces (§16.4) need no debug information, and outputs
stay small and independent of the directory they were built in. It does not change
the generated instructions. `build`, `test` and `profile` accept it. Today only the
bootstrap compiler emits line tables; the compiler accepts the option and emits none.

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
| E0218 | Import path absolute or containing `\` |
| E0219 | Import of an `internal` module from outside its parent's tree (§4.1) |
| E0220 | A package needs a newer compiler (`axiom` in its manifest, §4.6) |
| E0410 | Struct literal fields missing or out of order |
| E0413 | `undefined` initializer on an immutable `let` |
| E0414 | `undefined` initializer without a type annotation |
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
| E0635 | A `match` arm with several patterns binds a payload |
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
| E0313 | `inline` on an exported function, or with `@features` / `@multiversion` (§9.5) |
| E0314 | Recursion through inline functions (§9.5) |
| E0354 | Thread-local global without `mut` |
| E0801 | Type not allowed at the export or import boundary |
| E0823 | Invalid atomic operand (not a pointer to an integer, or not 32-bit for wait/notify) |
| E0824 | C variadic argument without a promoted type |
| E0825 | `@expect_eq` operand type cannot be compared |
| E0826 | `@enum_from_int` of a constant that is no member |
| E0827 | Unknown processor extension, architecture or operating system name (§13.4) |
| E0828 | Builtin used in code not compiled for the architecture or extensions it needs (§13.4) |
| E0829 | Invalid function attribute (`@features`, `@multiversion`) |

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

#### 16.4.1 Stack traces

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
  inlining does not remove frames. A call to an `inline` function writes its record
  only when a trap or another call inside it needs it (§9.5).
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

The cost is three stores and one load per call (none for a call to an `inline`
function). On call-heavy microbenchmarks (for
example, a naive recursive Fibonacci) it roughly doubles run time. On typical code it
is within measurement noise. Stack traces are on by default, and
`--stack-traces=off` removes the instrumentation.

**Frame tables.** Native code can be walked, so on x86-64 Linux the C compiler, and on
every native target (x86-64 and AArch64 Linux, macOS, Windows) the self-hosted one,
build the same records without a shadow stack, and calls cost nothing:

- Every function keeps a frame pointer, and the executable has a table (the section
  `axiom_rets`) that maps the return address of each call to its call-site record,
  followed by the records of the calls inlined around it: calls to `inline` functions
  and the ones the optimizer inlined. The C compiler takes these from the inlining
  information LLVM keeps in its debug locations, and drops the debug information
  afterwards unless `-g` asks for it; the self-hosted one keeps them with each call
  as it inlines. (Its x86-64 code gives a leaf function, whose only calls are traps,
  no frame: the table marks such functions, and the walk finds their return
  addresses on the stack.)
- A trap walks the frame-pointer chain from the trap's own frame, a crash from the
  interrupted one, and looks each return address up. The frames fill the thread's
  shadow-stack record (its calls and depth) as the stores would have, so the report,
  `ax_on_trap`'s third argument and the test runner's frames are the same. Error-return
  traces (§16.4.2) still use the record's ring.
- The differences: a fault that is not a check (a raw pointer access, say) inside a
  function the optimizer inlined is reported in the function it was inlined into, and
  frames outside a callback from C code compiled without frame pointers can be missing
  (C code built with the program, `--link=file.c`, keeps them).
- Keeping frame pointers costs a register and a push and pop per function, and the
  table 16 bytes per call. A recursive Fibonacci runs in about a third of the time it
  takes with the shadow stack, the same as with `--stack-traces=off` to within noise.
- `axiom profile` (§16.6) and `--sanitize=address` (§16.8) builds, which read the
  records while the program runs, keep the shadow stack, as do WebAssembly and, with
  the C compiler, other native targets.

Traces are diagnostics only: whether they are on never changes a program's
behavior. Record contents and layouts are toolchain ABI, not language semantics.

#### 16.4.2 Error return traces

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
profiler's (so `axiom profile cmd/axiom/src/main.ax -- build app.ax -O2` profiles the
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
| native | process CPU time | a POSIX CPU-clock timer delivers `SIGPROF`, and the handler copies the interrupted thread's shadow stack (`std/internal/runtime/profile.c`). Timer overruns weight each sample, so coarse kernel ticks don't skew totals | all threads |
| x86_64-windows | process CPU time | a sampler thread credits each thread the cycles it ran (`QueryThreadCycleTime`) at every tick; a thread with a period's worth is suspended and its shadow stack copied (`std/internal/runtime/profile_windows.ax`). Each sample is weighted by the periods it stands for, and waiting threads are not sampled | all threads |
| wasm32 | wall-clock time | the program runs on a shared memory, and a worker thread reads the main thread's shadow stack while it runs (`std/internal/runtime/profile_worker.mjs`) | main thread only |

Profile builds are otherwise identical to ordinary builds with stack traces on (the
default), except that they always use the shadow stack (also where ordinary builds use
frame tables, §16.4.1), that shadow-stack stores are volatile, so optimization can't remove
them, and that calls to `inline` functions (§9.5) record their call sites like any other,
so time spent in them is attributed to them. A program's behavior never depends on
whether it is being profiled.

Limitations in v0.1:

- Time is attributed to functions and call sites, not to individual lines inside the
  innermost function. For that, build with `-g` (§16.1) and use a native profiler.
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
toolchain's C runtime (`std/internal/runtime/*.c`) is compiled once per content and cached the
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
single object or IR file (`--emit=obj|ir`) always uses one unit.

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
the shadow stack (which sanitizer builds keep also where other builds use frame tables,
§16.4.1), so they need `--stack-traces=on` (the default). Each checked access
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

- a trap code (`T0000` to `T0013`);
- `error`: the test returned an error, and the message names it;
- `no_trap`: an `expect trap` test completed;
- `timeout`.

The span points at the failing expression, or at the test's name when there is no more
precise location.

## 18. Complete Example

`examples/dot_product/dot_product.ax`:

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

Builtins are how Axiom reaches what other languages leave to inline assembly. There is no
inline assembly, nor inline WebAssembly: an escape hatch into one instruction set weakens
every guarantee around it and has to be translated on every other target, while a builtin
is portable and type-checked.

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
| `@ptr_add(p, n)` / `@ptr_sub(p, n)` | `*T, usize → *T` (`*mut T → *mut T`) | `n` elements of `T` past (before) `p`. Never traps; the result must stay in `p`'s object (§5.7) |
| `@slice_from_ptr(p, n)` | `*T, usize → []T` (`*mut T → []mut T`) | No check at creation. The slice's accesses are bounds-checked against `n`, so `p` and `n` must describe memory of one object (§12.1) |
| `@prefetch(p)` | `*T → void` (or `*mut T`) | A hint that `p`'s memory will be read soon; no other effect, never traps (§5.7). Nothing on WebAssembly |
| `@prefetch_write(p)` | `*T → void` (or `*mut T`) | A hint that `p`'s memory will be written soon; no other effect, never traps (§5.7). Nothing on WebAssembly |
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
| `@clz(x)`, `@ctz(x)`, `@popcount(x)` | Return the same type as `x`; lane-wise on integer vectors |
| `@rotl(x, k)`, `@rotr(x, k)` | `k` is taken modulo the width |
| `@add_sat(a, b)`, `@sub_sat(a, b)`, `@mul_sat(a, b)` | Saturating, in every overflow mode. Also lane-wise on 8/16-bit integer vectors (add/sub only) |
| `@add_checked`, `@sub_checked`, `@mul_checked` | Returns `!T`, with `error.Overflow`, in every mode |
| `@min(a, b)`, `@max(a, b)` | Floats: a NaN operand gives NaN, and `-0.0 < +0.0` (WASM semantics) |
| `@abs(x)` | Integer `@abs(MIN)` overflows (§7.6): it is `MIN` by default, traps under `--overflow=trap`. Lane-wise on vectors, where it always wraps |
| `@sqrt`, `@floor`, `@ceil`, `@trunc`, `@nearest`, `@fma(a, b, c)`, `@rem(a, b)`, `@copysign(a, b)` | Floats |

### A.5 SIMD

| Builtin | Signature |
|---|---|
| `@splat(x)` | Scalar to `Vec`, with the type taken from context |
| `@vec(V, l0, l1, …)` | Construct a vector from exactly `lanes(V)` lane values |
| `@extract_lane(v, const i)` | `Vec → lane` |
| `@replace_lane(v, const i, x)` | `Vec → Vec` |
| `@load(V, s)` / `@store(s, v)` | Slice-checked load/store of the vector's size (§13.2) |
| `@load_partial(V, s)` / `@store_partial(s, v)` | The first `min(s.len, lanes)` lanes, from or to a slice of the lane type; the rest of a load is zero (§13.2) |
| `@shuffle(a, b, const [i0, …])` | Two-input lane shuffle; the result has as many lanes as indices (128, 256 or 512 bits). Indices must be in range (`E0820`) |
| `@swizzle(a, idx)` | Runtime indices (an integer vector of the same shape, unsigned) into the whole vector, of any lanes. Out-of-range indices give 0 (WASM semantics) |
| `@swizzle2(a, b, idx)` | `@swizzle` over the lanes of `a` then `b` |
| `@lanes_eq/ne/lt/le/gt/ge(a, b)` | Produce a mask vector |
| `@select(mask, a, b)` | Bitwise select |
| `@any_true(v)`, `@all_true(v)` | `→ bool` |
| `@bitmask(v)` | `→ u32`, one bit per lane (`u64` for 64 lanes) |
| `@reduce_add(v)` | Halves folded (§13.2). Integer lanes wrap |
| `@dot(a, b)` | Float vectors: `@reduce_add(a * b)`. `i16` lanes: WASM `i32x4.dot_i16x8_s`, giving half as many `i32` lanes; lane `i` is `a[2i] * b[2i] + a[2i+1] * b[2i+1]` in `i32`, which wraps only when both products are `-32768 * -32768`. One instruction per register on x86 (`pmaddwd`) and WebAssembly |
| `@widen_low(v)`, `@widen_high(v)` | Integer `Vec` of 8-, 16- or 32-bit lanes → half its lanes, twice as wide, extended by the lane type (§13.2) |
| `@narrow(T, a, b)` | `Vec, Vec → T`: the lanes of `a` then `b`, saturated to `T`'s narrower lanes (§13.2) |

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

### A.8 Processor extensions

See §13.4 and §13.5.

| Builtin | Notes |
|---|---|
| `@target_has("ext")` | `→ bool`, a constant: the code may use extension `ext` |
| `@target_arch("arch")` | `→ bool`, a constant: `wasm32`, `wasm64`, `x86_64` or `aarch64` |
| `@target_os("os")` | `→ bool`, a constant: `linux`, `macos`, `windows` or `none` (WebAssembly) |
| `@cpu_has("ext")` | `→ bool`: the running processor has extension `ext` |
| `@x86_cpuid(leaf, subleaf)` | `u32, u32 → u32x4`; needs `x86_64` |
| `@x86_xgetbv(xcr)` | `u32 → u64`; needs `x86_64` |
| `@x86_aesenc(s, k)`, `@x86_aesenclast`, `@x86_aesdec`, `@x86_aesdeclast`, `@x86_aesimc`, `@x86_aeskeygenassist` | AES rounds and key expansion; need `x86_64`, `aes` (§13.5) |
| `@x86_clmul(a, b, imm)` | carry-less multiplication; needs `x86_64`, `pclmul` (§13.5) |
| `@x86_sha1rnds4`, `@x86_sha1nexte`, `@x86_sha1msg1`, `@x86_sha1msg2`, `@x86_sha256rnds2`, `@x86_sha256msg1`, `@x86_sha256msg2` | SHA-1 and SHA-256; need `x86_64`, `sha` (§13.5) |
| `@x86_crc32c(crc, x)` | CRC-32C; needs `x86_64`, `sse4.2` (§13.5) |
| `@arm_aese`, `@arm_aesd`, `@arm_aesmc`, `@arm_aesimc`, `@arm_pmull`, `@arm_pmull2` | AES and carry-less multiplication; need `aarch64`, `aes` (§13.5) |
| `@arm_sha1c`, `@arm_sha1p`, `@arm_sha1m`, `@arm_sha1h`, `@arm_sha1su0`, `@arm_sha1su1`, `@arm_sha256h`, `@arm_sha256h2`, `@arm_sha256su0`, `@arm_sha256su1` | SHA-1 and SHA-256; need `aarch64`, `sha2` (§13.5) |
| `@arm_sha512h`, `@arm_sha512h2`, `@arm_sha512su0`, `@arm_sha512su1` | SHA-512; need `aarch64`, `sha512` (§13.5) |
| `@arm_eor3`, `@arm_bcax`, `@arm_rax1`, `@arm_xar` | SHA-3; need `aarch64`, `sha3` (§13.5) |
| `@arm_crc32(crc, x)`, `@arm_crc32c(crc, x)` | CRC-32 and CRC-32C; need `aarch64`, `crc` (§13.5) |

---

## Appendix B: Trap Conditions

This is the complete list of runtime traps. Each has a stable code.

| Code | Condition |
|---|---|
| `T0000` | `@trap()` or WASM `unreachable` |
| `T0001` | Index or slice bounds violation |
| `T0002` | Integer overflow, only in the opt-in `trap` overflow mode (`--overflow=trap`, §7.6); by default overflow wraps |
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
| `T0013` | A function compiled for processor extensions the running processor lacks (`@features`, §13.4) |

Nothing else traps. The only undefined behavior is an access that breaks the rule of §12.1
on a native target.
