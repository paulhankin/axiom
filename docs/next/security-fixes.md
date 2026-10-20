## Security fixes

- At `-O2`, the compiler took the popcount of a 64-bit value with no known bound to
  be at most 63, and dropped the bounds check of an index made from it.
- A git dependency's `rev` must be a full commit hash (40 or 64 lower-case hex digits),
  and its URL cannot start with `-`. A revision such as `:/text`, which picks a commit by
  its message, could hold `..` and put the checkout outside the cache.
- The search for a file's `axiom.toml` stops at a directory anyone may write to, such
  as `/tmp`, so another user cannot plant a manifest (and its git dependencies) above
  your files.
- `axiom test` and `axiom profile` build their program in a new private directory,
  not at a predictable name in `/tmp` that another user could take over. With no
  `HOME`, the cache is a private directory for the run instead of `/tmp/.cache/axiom`.
- `std/crypto/argon2` bounds the number of passes as well as the memory: parameters
  asking for more than `MAX_WORK_KIB` (16 GiB, passes times memory) fail with
  `error.TooExpensive`, so a hash with `t=4294967295` cannot make `verify_encoded` run
  for hours.
- `std/crypto/rsa`: PKCS #1 v1.5 signatures with SHA-1 used SHA-512's DigestInfo, so
  valid SHA-1 signatures were refused and the ones made were malformed.
- `std/crypto/x509`:
  - A wildcard `*.S` under a CA with permitted DNS subtrees was judged by the one name
    `a.S`, so a CA permitted only `a.example.com` could issue `*.example.com`. A
    wildcard is now permitted only if every name it matches is.
  - A CA whose name constraints include a kind std does not enforce (directoryName,
    rfc822Name, URI and the rest, or a minimum or maximum) was treated as if they were
    absent. A path through such a CA is now refused with `error.NameConstraint`.
  - A duplicated extension after 32 others was missed, so a second basicConstraints
    could override the first. Duplicates are refused wherever they are.
  - KeyUsage honours its bit string's unused-bits count: padding bits no longer count
    as usages, and non-DER padding is refused.
- `std/crypto/chacha20poly1305.xor_stream` traps instead of letting its 32-bit block
  counter wrap to block 0, which would reuse the keystream. `seal` and `open` already
  refused messages that long.
- On Windows, `process.run` and `Command.spawn` refuse to run a batch file (`.bat` or
  `.cmd`) with `InvalidArgument`: cmd.exe, which runs it, would read an argument such
  as `x&calc` as a command. Run `cmd.exe` explicitly for a script.
- On Windows, a program named without a path that is not on the child's `PATH` gives
  `NotFound`; it was also looked for in the current directory, where a program planted
  there could run instead.
- The http router refuses `\` and `:` in route parameters, as it does `/`, `..` and NUL,
  so `/files/{name}` cannot reach outside its directory on Windows.
- `os.unsetenv` removes every entry for a variable when the environment the program
  started with held several (it removed only the first, so a child could still get the
  variable), and `os.setenv` leaves exactly one.
- `term.set_title` also leaves out C1 control characters (U+0080 to U+009F) and invalid
  UTF-8, so a title cannot end its escape sequence early and start others.
- `fs.copy_file` gives a new file the source's permission bits (less the umask) instead
  of 0644, so a copy of a private file is private too.
- `fs.map` takes the size and kind of the file from the file it opened, not from an
  earlier `stat` of the path.
- On macOS, a child process inherits only its standard streams, not other descriptors
  open in the parent (such as a pipe another thread had just made).
- The HTTP/2 server accepted malformed header fields: names that are not lower-case
  tokens, values holding NUL, CR or LF, connection-specific headers (`Connection`,
  `Transfer-Encoding`, `Upgrade` and the like), unknown, repeated or misplaced
  pseudo-headers, requests without `:method`, `:scheme` or `:path`, and bodies whose
  length did not match `content-length`. It now resets such a stream (RFC 9113
  §8.2–8.3), applies HTTP/1's checks to the method, path and header values, and drops
  trailers as HTTP/1 does.
- An HTTP/2 connection had no request deadline, so PINGs alone kept it, and a server
  thread, busy for ever. HTTP/2 follows `Options.request_timeout_ms` like HTTP/1, which
  also bounds how long a response waits for flow-control window. A request that
  completed while a response waited for window was never answered; it now is.
- An HTTP/2 connection could hold a body of `Options.max_body_bytes` on each of its
  100 streams at once; a connection's bodies together are now limited to the maximum
  body size plus 1 MiB.
- Chunked HTTP/1 bodies accepted a bare LF as a line ending, control bytes in chunk
  extensions and any trailer lines. Chunk lines must end in CRLF, extensions must be
  well-formed, and trailers are checked like headers.
- The HPACK encoder kept every `SETTINGS_HEADER_TABLE_SIZE` change a peer sent; it now
  keeps only the smallest and the last (RFC 7541 §4.2).
- The TLS server took any bytes, cut at 255, as the client's server name. A name over
  255 bytes, or holding anything but letters, digits, `-`, `.` and `_`, is now refused
  with an `illegal_parameter` alert, and `tls.client` refuses to send one.
- DNS replies could give names whose labels held `.`, `\`, CR, LF, NUL or other bytes
  outside printable ASCII, so a name could read as a different one. Such labels are
  refused (`BadResponse`).
- The TLS 1.3 client accepted a TLS 1.2 cipher suite in a TLS 1.3 ServerHello or
  HelloRetryRequest, and a HelloRetryRequest for a group it had already sent a share
  for. Both are refused with `illegal_parameter`.
- TIFF: a file whose directories loop back on themselves is refused at once, instead of
  being walked 65536 times by `page_count` and `decode_page`.
- GIF and WebP: an animation is refused (`TooLarge`) once its frames hold more than
  `image.MAX_PIXELS` pixels together, since each frame is a full copy of the canvas: a
  small file could use many gigabytes.
- QOI, BMP, PNM and HDR: a file too small for the image it declares is refused before
  memory is allocated for it, so a few bytes no longer make a 16384 × 16384 image. A
  truncated uncompressed BMP still decodes if at least half its rows are there.
- PDF: an object that fails to parse is parsed only once, however often it is referred
  to, and parsing is limited to a few times the file's size. Each stream is decoded
  once, and each use of a form XObject counts against the text extractor's step limit,
  so a form that draws itself many times no longer takes seconds and hundreds of
  megabytes.
- `std/compress/zstd.compress` could take hours on repetitive input longer than one
  block (128 KiB) at level 3 or above: near a block's end, each position measured its
  match past the block. It is now linear.
- `std/archive/zip`:
  - `Archive.open` refuses, with `error.Corrupt`, archives whose entries share bytes
    (several central directory entries pointing at one local file), which let an 11 KB
    archive extract to 200 MB.
  - `extract_all` and `extract_all_password` take a `max_size` and fail with
    `error.TooLarge`, before writing anything, if the entries' sizes add up to more.
    Callers need to pass one.
  - A ZIP64 locator in an archive shorter than 56 bytes made `Archive.open` trap; it
    fails with `error.Corrupt`.
- `std/text/markdown`: a GFM table with a wide header and many short rows padded every
  row to the header's width, so 18 KB of input took 10 s and 1 GB. Padding has a
  budget proportional to the input; past it the table ends.
- `std/text/html`: the table that finds repeated attribute names used an unkeyed hash,
  so a tag with many chosen names took quadratic time. It uses std's keyed hash.
- `std/text/diff.apply`: a hunk removing lines from line 0 (`@@ -0,1 ...`) trapped in an
  `--overflow=trap` build; it fails with `error.InvalidPatch`.
- `std/encoding/xml`: attribute defaults from the internal DTD subset took cubic time
  and quadratic memory (a 39 KB document ran for 17 s in 250 MB), and a long default on
  many elements could multiply a document's size a thousandfold. Defaults are looked up
  by element, and what they add counts against a limit (`MAX_EXPANSION` or four times
  the document's size); past it, parsing fails with `error.TooLarge`. Elements with
  tens of thousands of attributes or namespace declarations, and thousands of
  namespace bindings in scope, no longer take quadratic time to parse or canonicalize.
- `std/encoding/yaml`: the alias limit counted a scalar as one node whatever its length,
  so a 100 KB document aliasing a 50 KB scalar 10000 times expanded to 500 MB in code
  that walks the tree. A scalar counts its length too, and such documents fail with
  `error.TooLarge`.
- `std/text/template` was not safe for untrusted templates, as it claimed:
  - Deeply nested sections overflowed the stack. Sections nest at most `MAX_DEPTH`
    (100) deep, together with partials when rendering (`error.TooDeep`).
  - Nested sections, and partials that include others twice, could write gigabytes or
    run for ever. A render does at most `MAX_WORK` steps, so it writes at most 64 MiB,
    else it fails with the new `error.TooLarge`; each partial is compiled once per
    render.
  - Many tags on one line took quadratic time to compile. Delimiters set with
    `{{=...=}}` are at most `MAX_DELIMITER` (16) bytes.
- The specification and `SECURITY.md` now say that a data race on a slice, union or
  error union can tear it into a value no store wrote, and so reach outside an object
  (§12.8.2). They had said that no race is undefined behavior; that holds for plain
  data. Share such values between threads only with synchronization.
- `std/text/math`: commands whose arguments have no braces (`\sqrt\sqrt…x`,
  `\frac1\frac1…`, `\hat\hat…`) now count toward the 64-level nesting limit, so deeply
  nested input gives `InvalidMath` instead of overflowing the stack, through
  `std/text/markdown` with `Options.math` too.
- `std/text/regex`: `captures` (and `replace_all`) took time proportional to the text
  times the pattern times its number of groups, without limit. Patterns whose program
  size times (groups + 1) exceeds `4 × MAX_PROGRAM`, or with more than 1000 groups, now
  fail with `TooBig`. A `replace_all` template with many `${` and no `}` no longer takes
  quadratic time.
- `std/text/wrap`: wrapping a very long word, with or without a hyphenator, no longer
  takes quadratic time.
- `std/unicode/collate`: sort keys took quadratic time on long runs of combining marks.
  A discontiguous contraction looks at most 30 non-starters past its start (the
  stream-safe limit of UAX #15), and the marks it takes are no longer removed from the
  middle of the text.
- `std/math/decimal`: `normalize` and exact quotients and roots removed trailing zeros
  one at a time (8 s for `1` and 100000 zeros); they are found by halves. `MAX_DIGITS`,
  which bounds every number and precision, is now 100000 (it was 1000000, where
  parsing, formatting and division took seconds to minutes); past it operations fail
  with `Overflow`.
- `std/math/bigint`: `parse` and `format` convert by halves, `sqrt` uses Newton's method
  at doubling precision, and `gcd` uses Lehmer's algorithm. Division is still quadratic.
- `std/math/rational`: `Rational.parse` accepts at most `MAX_DIGITS` (100000) digits;
  more fail with `Overflow`.
- `std/time`: `Time.add`, `at_offset`, `from_unix_millis` and the `Duration`
  constructors and arithmetic overflowed (and trapped in an `--overflow=trap` build)
  near the ends of an i64; they saturate. `std/time/tz` rules no longer overflow for
  far-away instants.
- `std/graphics/image/jpeg`: progressive scans made of end-of-band runs took time far
  beyond the file's size (a 512 KB file of 255 such scans took over a minute). A
  refinement run visits only the coefficients already nonzero, and a file's scans
  together may visit each block at most 32 times, else it is `Corrupt`. The minimum
  file size for a frame counts the frame padded to whole MCUs over all components.
- `std/graphics/image/avif`:
  - A small file could claim a huge image and have it allocated and filled before any
    AV1 data was read (356 bytes took 2 GB). The image is allocated once its frame is
    decoded, and the frame must cover the size claimed.
  - Extents that all covered the same data were copied once each (180 KB took 900 MB).
    An item's extents may not add up to more than the file, and all the item data read
    for a file is limited to twice its size.
  - A grid could repeat one tile thousands of times, with a buffer sized by the tiles
    rather than the grid. A grid's tiles must just cover it, each tile may appear once,
    and the grid is at most `MAX_PIXELS`.
  - Items are indexed rather than searched (60,000 items took 11 s), an AV1 frame may
    be at most `image.MAX_PIXELS` pixels (`Unsupported`), and only the first rotation,
    mirroring and clean aperture of an item is applied.
- `std/graphics/image/exr`: each PIZ block cleared and filled a 16384-entry Huffman
  table and an 8 KiB bitmap whatever its size, so a 4.8 MB file of tiny blocks took
  5 s; a block now uses a table as large as its size pays for, and long codes are found
  by their length rather than by a scan that could take quadratic time. Huffman output
  past a block's lookup table was mapped through stale entries; it decodes as 0.
