## Standard library and libraries (breaking)

The standard library is split in two tiers, visible in the import path (spec §4.1):

- `std/`, the core, which every program and the compiler build on: `mem`, `str`, `fmt`,
  `io`, `os` (with `os/path`, `fs`, `process`, `cli`, `cpu`, `term` and `random`),
  `collections/*`, `hash` (with `hash/*`), `math` (with `math/complex`, `rand`,
  `stats`), `time` (with `time/tz`, `cron`), `thread`, `thread/pool`, `sync`, `unicode`
  (with `unicode/tables`), `diag`, `log`, `net` (sockets), `encoding/json`,
  `encoding/binary` and `c`. It imports nothing outside `std/`, every public item of it is
  documented, and its tests run with overflow trapping; the suite checks all three.
- `lib/`, the libraries released with it: `import "lib/..."` reads the directory named
  `lib` beside the standard library's (so `lib/` beside `std/` in a release; `--std` and
  `AXIOM_STD` choose both). `axiom doc lib` lists them.

Every other module moved, at the same path under `lib/`, except where said:

| Was | Now |
|---|---|
| `std/crypto/md5`, `sha1`, `sha2`, `sha3` | `std/hash/md5`, `sha1`, `sha2`, `sha3` |
| `std/crypto/random` | `std/os/random` |
| the rest of `std/crypto/` | `lib/crypto/` |
| `std/encoding/` but `json` and `binary` | `lib/encoding/` |
| `std/net/` (`http`, `http2`, `tls`, `dns`, `url`, `cookie`, `hpack`, `websocket`) | `lib/net/` (`std/net`, the sockets, stays) |
| `std/compress/`, `std/archive/`, `std/text/`, `std/graphics/` | `lib/compress/`, `lib/archive/`, `lib/text/`, `lib/graphics/` |
| `std/audio` and `std/audio/` | `lib/audio` and `lib/audio/` |
| `std/math/bigint`, `decimal`, `rational`, `matrix`, `linalg`, `geometry`, `fft` | `lib/math/...` |
| `std/unicode/collate`, `bidi`, `linebreak` | `lib/unicode/...` |
| `std/os/window`, `cocoa`, `win32`, `gamepad`, `gamecontroller`, `xinput`, `objc`, `com` | `lib/os/...` |
| `std/c/include/`, `std/c/lib/`, `std/c/rt/` (the C front end's) | `lib/c/...` (`std/c`, for calling C, stays) |
| `unicode.graphemes`, `grapheme_count`, `words`, `is_word`, `sentences`, `string_width`, `title` (and `Graphemes`, `Words`, `Sentences`) | `lib/unicode/segment`, by the same names |

To update a program, change its imports as the table says. An import of a moved
`std/` path is now `E0211` with a machine fix that imports the module where it is
(`import "std/encoding/csv"` becomes `import "lib/encoding/csv"`), so an editor or agent
that applies machine fixes does it; `axiom fmt` does not rewrite imports. Code that
segments text imports `lib/unicode/segment` (`segment.graphemes(s)` for
`unicode.graphemes(s)`); `std/unicode` keeps the break properties and gains `title_word`,
which `segment.title` uses. A module of a program's own that is imported as
`"lib/..."` relative to it now names the libraries instead: write `"./lib/..."`.
`std/internal/` may be imported from `lib/` as well as from `std/`.
