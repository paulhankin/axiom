# Security of the standard library

How `std/` is meant to behave on untrusted input, the defences it has, and how to
check them. To report a vulnerability, see [`SECURITY.md`](../SECURITY.md).

## What "safe" means here

Checked code cannot corrupt memory: indexing and slicing are bounds-checked, and
integer overflow wraps (spec §7.6), so sizes computed from input use
`@mul_checked`/`@add_checked` and a wrapped index still meets a bounds check. Where
std uses raw pointers (the allocator, the runtime, codecs' inner loops), it keeps to
the access rule of spec §12.1. Input can still cause harm in four ways:

- **Traps.** An out-of-bounds index (or an overflow, in an `--overflow=trap` build) that
  input can reach kills the process. For a server, that is a denial of service.
- **Resource exhaustion.** Input can make the program allocate without limit, or do
  work that grows faster than linearly: nested structures, backtracking, quadratic
  loops, decompression bombs.
- **Protocol bugs.** Some inputs are accepted that should be refused: request
  smuggling, certificate-checking gaps, TLS state-machine holes.
- **Environment problems.** Files, paths, symbolic links, permissions and terminal
  output are all places where input can do damage.

The rule every parser follows: **for any input, it returns an error, and its work and
memory are bounded by a small multiple of the input's size** (or by a documented
limit).

## Checking

- **Regression tests.** Every defence below has a test beside the code, usually a
  hostile input that has to fail quickly or produce a bounded result.
- **Fuzzing.** `cmd/axiom/internal/tests/tools/fuzz.py` mutates seed inputs (made by
  std's own encoders, plus sounds from ffmpeg and sox, system fonts and CA
  certificates) and runs them through every parser target of
  `cmd/axiom/internal/tests/fuzz/fuzz.ax`, the network parsers among them (TLS hello
  messages, HTTP/2 frames, HPACK blocks, WebSocket frames, DNS replies, fed as bytes
  without a socket); a trap, a signal or a hang (10 s) is saved as a failure:

      cmd/axiom/internal/tests/tools/fuzz.py --runs 5000            # all targets
      cmd/axiom/internal/tests/tools/fuzz.py --runs 20000 pdf font  # some of them
      cmd/axiom/internal/tests/tools/fuzz.py --minutes 30 --seed 0  # half an hour, a fresh seed

  The fuzzer runs for a few minutes in the release steps (`docs/releasing.md`); a
  longer run with a new seed is worth it after a change to a parser.

- **The address sanitizer.** `axiom test --sanitize=address` (spec §16.8) traps on use
  after free, double frees and out-of-block accesses in the heap, and reports leaks.

Known gaps are GitHub issues, among them
[#41](https://github.com/paulhankin/axiom/issues/41) (fuzzing for longer, and on a
schedule).

## Defences

### Media: images, audio, fonts, compression

| Where | What std does |
|---|---|
| JPEG | refuses fractional chroma sampling; at most 256 progressive scans; stops early on filler bytes; a file must be at least `pixels / 512` bytes for the size it claims |
| EXR | 64-bit checked size products; tiles clamped to the image; each block decoded once (a seen-block bitmap); `raw / 2048 <= data.len`; shared PIZ scratch space |
| FLAC | block size at most `MAX_BLOCK` (65536); preallocation from the header capped at 1 MiB |
| Vorbis | codebooks sorted in O(n log n) (heapsort on packed keys) |
| fonts | GPOS kerning subtables precomputed (at most 32), never rescanned per pair; cmap format 12 arithmetic wraps and every glyph id is checked against `numGlyphs`; CFF INDEX `offSize` must be 1 to 4, with saturating offsets |

### Network, TLS and X.509

| Where | What std does |
|---|---|
| http server | strict `Content-Length` and `Transfer-Encoding` parsing (no duplicates, no `Transfer-Encoding` on HTTP/1.0, strict chunk sizes) against request smuggling; exactly one `Host`; control characters refused in headers; at most 100 trailers; a request deadline (`Options.request_timeout_ms`) against slowloris |
| http client | methods, headers and URLs validated (no CR, LF or spaces); host names limited to letters, digits, `-` and `.` |
| http router | `..`, `/` and NUL refused in path parameters (except `*`), so `/files/:name` cannot escape |
| tls | post-handshake messages capped at 64 KiB; ChangeCipherSpec accepted only as a single `0x01` before the handshake completes; a 20 s handshake deadline; ALPN must be one that was offered; `RecordOverflow` on oversized inner plaintext; no handshake bytes carried across a key change; KeyUpdate validated; NewSessionTicket accepted by clients only |
| tls client | the server's session id echo must match; the ServerHello after a HelloRetryRequest must keep its suite and group |
| x509 | at most 64 signature checks while building a path; a CA's name constraints checked against every certificate below it in the path (the leaf and the intermediates, not self-issued ones), names trimmed of a trailing dot and wildcards checked as a representative name; trailing data and duplicate extensions refused |
| rsa, der, base64, hmac, aes-gcm, chacha20-poly1305 | huge public exponents, non-minimal integers, loose padding, HKDF output past `255 * n` and messages past the AEAD limits are refused (or trap, where they are programming errors) |
| hash maps | SipHash-1-3 with a per-process random key, against hash flooding |
| dns | a random query id, a connected socket, and answers matched to the question |

### Memory and runtime

| Where | What std does |
|---|---|
| mem | requests over 1 TiB fail (`too_big`) before any size arithmetic; a small free checks that the pointer starts an object (a modular-inverse test) and traps on an immediate double free |
| json | arena chunks at least as large as the allocation |
| vec, str | `v.extend(v.items())` and `s.push_str(s.bytes())` work when the source aliases the destination |
| c | `to_cstr` refuses an interior NUL |
| fs | NUL refused in paths by every runtime; saturating times; temporary directories created 0700; `remove_all` and `walk` open directories relative to their parents with `O_NOFOLLOW` (`ax_dir_*`), so a symbolic link swapped in for a directory is not followed (from 32 levels down an ancestor's handle is closed and, when reopened through `..`, checked by device and inode, so a tree deeper than the descriptor limit cannot stop a cleanup); the node host pins each directory with a descriptor and resolves names through it (Linux), or checks device and inode before each use |
| threads | signal alternate stacks and thread stacks freed when a thread exits |
| log | CR, LF, C1 controls, U+2028/U+2029 and invalid UTF-8 escaped in messages and keys, so input cannot forge log lines; a failed rotation reopens the file in append mode |

### Text formats

| Where | What std does |
|---|---|
| regex | linear-time matching; a compile work budget (4 × `MAX_PROGRAM`); nesting depth below 1000; a bounded DFA cache (`DFA_MAX_PCS`). `find_all` and `replace_all` search again after each match, so a pattern whose higher-priority alternative outlives its match (`a*b\|a` on runs of `a`) takes quadratic time over many matches, as in RE2; an untrusted pattern over a large text needs a bound on one of them |
| markdown | linear-time structures for the known CommonMark pathological inputs (nested brackets, emphasis, backticks, list tightness); a budget on link reference expansion; safe HTML by default |
| unicode | linear-time reordering (a counting sort), word boundaries (a capped regional-indicator count) and sentence boundaries (a memoized suffix scan) |
| bidi | isolates matched with an explicit stack, not recursion |
| path | glob `**` matched with single backtracking, not exponentially |
| draw | edges sorted in O(n log n) above 32 crossings; bad or tiny dash patterns fall back to a solid line |
| text | `fit` measures widths incrementally |
| time | saturating arithmetic for far-away times |
