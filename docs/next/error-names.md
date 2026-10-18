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
- `error.Failed` now means only that a system call failed without saying why. The
  OS, graphics and audio modules return the cause where there is one:
  `InvalidArgument` for a bad argument or a call in the wrong state, `NoSpaceLeft`
  when a fixed table is full, `OutOfMemory`, `TimedOut` (Metal's `nextDrawable`),
  `IoError` (process pipes), `NotADirectory` (`os.chdir`), and the new `DeviceLost`
  for a graphics or audio device that was removed or reset. HRESULTs go through the
  new `com.check`, Win32 failures through GetLastError, failed Metal command buffers
  through their error code, and `std/graphics/gpu`, `std/os/window` and
  `std/audio/output` pass their backend's error on instead of replacing it.
