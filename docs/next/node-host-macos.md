## Standard library

- Under the node host on macOS, `fs.remove_all` of a tree more than 32 levels deep
  failed with `NotFound`, and `fs.set_times` could set a time a nanosecond later than
  the one given.
- `axiom test` on WebAssembly gives each test an 8 MiB stack, as a native main thread
  has; with Node's default of 4 MiB, recursive code such as `json.parse` at its depth
  limit ran out of stack on arm64.
