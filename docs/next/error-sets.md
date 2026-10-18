## Language

- Named error sets (spec §11.3): `error FileError { NotFound, PermissionDenied }`
  declares a set, `error ConfigError = FileError | { SyntaxError };` combines sets, and
  `FileError!T` returns only the errors of the set. `return error.X`, `!` and `match`
  arms are checked against it (`E0652`, `E0653`), a `match` that lists every error of a
  set needs no `else`, and `catch |e|` binds `e` with the set's type. Sets have the
  representation of `!T`, but error union types do not convert: `S!T` is not `!T`, and
  `fn() S!T` is not `fn() !T`. `!T` is unchanged and still means any error.
