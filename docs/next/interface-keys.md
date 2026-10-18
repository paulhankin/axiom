## Compiler

- Separate compilation recompiles a module's importers only when its interface
  changes: a change inside the body of a non-generic, non-inline function or
  method (or a test) now recompiles that module alone, where before it
  recompiled every module importing it (spec §16.7).
