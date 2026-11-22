## Tools

- `axiom doc` prints signatures and documentation from the command line, compactly, for
  a reader such as an agent: `axiom doc std` lists the standard library's modules with
  their summaries, `axiom doc std/str` a module's public items (signature and first
  sentence each), `axiom doc std/str.split` one item with its whole documentation, the
  names of the error set it returns and its location, `axiom doc
  std/collections/vec.Vec` a type with its fields and methods. It also takes
  `pkg:<name>`, a file, a directory, and `--search=<text>` over the public items under a
  root. It only parses, so it answers in milliseconds and for a module that does not
  compile. `--format=agent-json` gives the same under the schema `axiom-doc/1` (spec
  §16.10, `docs/cli.md`).
- The warning `W0320`, off by default and turned on with `--warn=W0320`, reports a
  public item (or a public method of a public type) of the program's own modules that
  has no `///` comment.
- The diagnostic code registry (spec §16.3) lists the warnings too, and the test suite
  checks them as it checks the errors.
