## Compiler

- wasm64: a program built with `--sanitize=address` runs (its `main` crashed the host),
  and `axiom test --target=wasm64` runs the tests, with the sanitizer by default as on
  the other targets.
- A sanitizer report names an access at an allocation's first byte "at the start of"
  it, one at its end "just past the end of" it, and counts in bytes ("1 byte past the
  end of") rather than "0 bytes inside".
