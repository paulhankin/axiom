## Language

- Named error sets (spec §11.3): `error FileError { NotFound, PermissionDenied }`
  declares a set, `error ConfigError = FileError | { SyntaxError };` combines sets, and
  `FileError!T` returns only the errors of the set. `return error.X`, `!` and `match`
  arms are checked against it (`E0652`, `E0653`), a `match` that lists every error of a
  set needs no `else`, and `catch |e|` binds `e` with the set's type. Sets have the
  representation of `!T`, but error union types do not convert: `S!T` is not `!T`, and
  `fn() S!T` is not `fn() !T`. `!T` is unchanged and still means any error.

## Standard library

- Most of std now returns named error sets: each module that can fail declares one
  named for it (`mem.AllocError`, `fs.FsError`, `json.JsonError`, `png.PngError`, …),
  and about two thirds of its fallible functions return `Set!T`. Functions that can pass
  on errors of no set (readers, writers, callbacks) still return `!T`. Programs that
  return such a call directly from a function returning `!T` now write `return f()!;`,
  and helpers that take a `!T` need the set (`fs.FsError!T`) or a `!` on the way in.

