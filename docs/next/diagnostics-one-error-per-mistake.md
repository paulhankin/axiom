## Diagnostics

- A syntax error costs one error. A declaration with one is *poisoned*: the parser reports
  no further syntax error in it, its body is not checked (its signature is, so callers
  check as usual), and no diagnostic in it has a fix. After an error it cannot recover
  from, the parser resumes at the next top-level declaration (spec §16.2). Before, a
  mistake such as `fn f() -> i32` cost five errors, one of them a wrong `machine` fix.
- Forms from other languages have a diagnostic of their own, `E0160` to `E0180`, that
  names the Axiom form: `!x`, `&&` and `||`, `T{ ... }`, `->` or `:` before a return
  type, `i++`, `c ? a : b`, `x as T`, `try f()` and `f()?`, `null`, `None`, `Some(x)`,
  `Ok(x)` and `Err(e)`, `C::R`, `var` and a leading `mut`, `s.len()`, `if` and `match`
  as expressions, a C-style `for`, `lo..=hi`, `*p`, `fn id<T>` and `Vec<T>`, `Vec[T]`,
  `'abc'`, `+` on slices, and `const Color = enum { ... }` (or `struct`, `union`). Most
  have a `machine` fix, and the code is checked as if the Axiom form had been written;
  each costs one error.
- An unknown name (`E0202`) or a field or method a type does not have (`E0404`) comes
  with notes naming the nearest names and their signatures, and a `suggestion` fix when
  one alone is within two edits (`fmt.println` suggests `fmt.print`).
- The test suite applies every `machine` fix of its errors tests and checks that the
  diagnostic is gone and no error was added (spec §16.2).
