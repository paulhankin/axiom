## Standard library

- `std/compress/deflate` reports corrupt input as `error.Corrupt`, like the other
  compressors and archives (it was `CorruptData`), and `std/io` and `std/os/fs` report
  permission problems as `error.PermissionDenied`, like `std/os` and `std/os/process`
  (it was `AccessDenied`). Programs that match on the old names change the name.
  std/README's Conventions now state the error vocabulary.
