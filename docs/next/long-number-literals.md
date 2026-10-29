## Compiler

- A number literal longer than 120 characters (its `_` separators and base prefix aside)
  is an error, `E0005`. The digits past the 120th used to be dropped without a word, so
  such a float literal had a different value from the one written.
