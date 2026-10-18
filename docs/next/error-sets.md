## Language

- Named error sets (spec §11.3): `error FileError { NotFound, PermissionDenied }`
  declares a set, `error ConfigError = FileError | { SyntaxError };` combines sets, and
  `FileError!T` returns only the errors of the set. `return error.X`, `!` and `match`
  arms are checked against it (`E0652`, `E0653`), a `match` that lists every error of a
  set needs no `else`, and `catch |e|` binds `e` with the set's type. Sets have the
  representation of `!T`, and an `S!T` widens to `!T` (or to a set that holds all of S),
  as does a function pointer returning one; a `!T` does not narrow to `S!T`. `!T` is
  unchanged and still means any error.

## Standard library

- Most of std now returns named error sets: each module that can fail declares one
  named for it (`mem.AllocError`, `fs.FsError`, `json.JsonError`, `png.PngError`, …),
  and about two thirds of its fallible functions return `Set!T`. Functions that can pass
  on errors of no set (readers, writers, callbacks) still return `!T`. A `Set!T` widens
  to `!T`, so existing callers compile unchanged; a `match` on a set's errors needs no
  `else` when it lists them all.
