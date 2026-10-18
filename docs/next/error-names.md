## Standard library

- `std/compress/deflate` reports corrupt input as `error.Corrupt`, like the other
  compressors and archives (it was `CorruptData`), and `std/io` and `std/os/fs` report
  permission problems as `error.PermissionDenied`, like `std/os` and `std/os/process`
  (it was `AccessDenied`). Programs that match on the old names change the name.
  std/README's Conventions now state the error vocabulary.
- More of the error vocabulary (`std/README.md`, Conventions): corrupt input is now
  `Corrupt` in `graphics/image` and its formats, `audio` and its formats, `graphics/font`,
  `graphics/pdf` and `graphics/pdf/read`, `text/markdown/pdf`, `crypto/der`, `time/tz`,
  `encoding/msgpack`, `encoding/cbor` and `encoding/protobuf` (formerly `InvalidImage`,
  `InvalidAudio`, `InvalidFont`, `Malformed`, `InvalidDer`, `InvalidZone`, `Invalid`), and
  `SyntaxError` in `graphics/svg` (formerly `InvalidSvg`). Ownership ends with `deinit`
  everywhere: the `release` methods of the `graphics/gpu`, `metal`, `d3d11`, `d3d12`, `d2d`,
  `quartz` and `hlsl` wrappers and the `destroy` of `thread/pool` and the `audio/output`,
  `coreaudio` and `xaudio2` voices are `deinit` (COM's and Objective-C's `release` and the
  semaphore's keep their names: they are not ownership).
