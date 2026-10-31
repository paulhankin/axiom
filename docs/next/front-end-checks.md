## Language

These programs were accepted and now are compile errors; the fixes are mechanical:

- A method or associated function without `pub` is private to its module (spec §4.2):
  calling `s.m()` or `T.f()`, or taking `T.m` as a value, from another module is
  `E0203`. Mark the function `pub`.
- The part of a `std/` or `pkg:name/` import path after the prefix may not contain `\`
  or a `..` part (`E0218`), so that it stays inside the library or the package.
- An empty character literal `''` is `E0004`; a `_` in a number literal that is not
  between two digits (`1__0`, `10_`, `0x_ff`, `1_.5`) is `E0005`.
- Source text that is not well-formed UTF-8, or that has a `\r` not followed by `\n`,
  is `E0007`.
- Expressions, types and blocks nested more than 1000 levels deep are `E0110`, where
  the compiler used to run out of stack.

## Fixes

- The `E0219` message no longer crashes (or prints garbage) when the working
  directory is inside the `internal` tree it names.
- `@features` checks each extension's whole name: the bootstrap compiler cut names at
  63 characters, so a long unknown name could be taken for a known one.
