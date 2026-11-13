## C

- The optimizer acts on what functions promise: `const` and `pure` (GNU dialects) and
  C23's `[[unsequenced]]` and `[[reproducible]]` (calls merged, moved out of loops,
  removed when unused); `malloc` (a new object, apart from all others) and `alloc_size`
  (`__builtin_object_size`, and `__builtin_dynamic_object_size` with run-time sizes and
  variable length arrays); `nonnull`, `returns_nonnull` and `[static n]` parameters
  (tests against null folded; `--c-keep-null-checks` keeps them).
- A path to `unreachable()` or `__builtin_unreachable()` is a fact below its test: later
  tests of the value fold, switches lose their range checks, and the test itself is not
  made. The inliner keeps cold paths small and gives `hot` functions more room.
