# Security review of the standard library

This is a record of a review of `std/` for problems with untrusted input,
what was found, and what was changed. It also says what is still open and
how to go on checking.

## What "safe" means here

Axiom checks bounds, so bad input can't corrupt memory. Integer overflow
wraps (spec §7.6; until issue 0047 it trapped): sizes computed from input
use `@mul_checked`/`@add_checked`, and a wrapped index still meets a
bounds check. Input can still cause harm in four ways:

- **Traps.** An out-of-bounds index (or an overflow, in an
  `--overflow=trap` build) that input can reach kills the process. For a
  server, that is a denial of service.
- **Resource exhaustion.** Input can make the program allocate without
  limit, or do work that grows faster than linearly: nested structures,
  backtracking, quadratic loops, decompression bombs.
- **Protocol bugs.** Some inputs are accepted that should be refused:
  request smuggling, certificate-checking gaps, TLS state-machine holes.
- **Environment problems.** Files, paths, symbolic links, permissions and
  terminal output are all places where input can do damage.

The rule the library now follows is: **for any input, a parser returns an
error, and work and memory are bounded by a small multiple of the input's
size** (or by a documented limit).

## How the review was done

1. Each module was read with the four questions above in mind. Five review
   passes covered: media codecs, network/TLS/X.509, memory and runtime,
   text formats, and the filesystem.
2. Mutation fuzzing: `tools/fuzz.py`, which drives `tests/fuzz/fuzz.ax`.
   Seeds come from std's own encoders, plus ffmpeg/sox sounds, system
   fonts and CA certificates. Mutated inputs run through every parser
   target, and a trap, a signal or a hang (10 s) is saved as a failure.
3. Every fix has a regression test beside the code, usually a hostile
   input that has to fail quickly or produce a bounded result.

To run the fuzzer:

    tools/fuzz.py --runs 5000            # all targets
    tools/fuzz.py --runs 20000 pdf font  # some of them

## Findings and fixes

### Media (images, audio, fonts, compression)

| Where | Problem | Fix |
|---|---|---|
| JPEG | fractional chroma sampling indexed past the MCU; unbounded progressive scans; a small file could claim huge dimensions | reject fractional sampling; at most 256 scans; stop early on fake bytes; require `pixels / 512 <= data.len` |
| EXR | 32-bit size products overflowed; tile/line sizes unchecked; duplicate blocks; a huge alpha fill (found by fuzzing) | 64-bit checked sizes, clamped tiles, a seen-block bitmap, `raw / 2048 <= data.len`, shared PIZ scratch space |
| FLAC | block size and preallocation taken from the header | `MAX_BLOCK = 65536`; preallocation capped at 1 MiB |
| Vorbis | codebook sort was quadratic | heapsort on packed keys |
| font | GPOS kerning rescanned every subtable per pair (found by fuzzing); cmap 12 overflowed and could return glyphs past `numGlyphs`; CFF INDEX offSize unchecked | kerning subtables precomputed (at most 32); wrapping cmap arithmetic with a `gid < num_glyphs` check; offSize must be 1..4, with saturating offsets |

### Network, TLS and X.509

| Where | Problem | Fix |
|---|---|---|
| http | request smuggling (duplicate or odd `Content-Length`, `Transfer-Encoding` on HTTP/1.0, loose chunk sizes), header injection, slowloris | strict Content-Length and Transfer-Encoding parsing; exactly one Host; control characters rejected; strict chunk sizes; at most 100 trailers; a request deadline (`Options.request_timeout_ms`) |
| http client | CR/LF and spaces in methods, headers and URLs | validated; hosts limited to letters, digits, `-` and `.` |
| http router | `..`, `/` and NUL in parameters | rejected (except for `*`), so `/files/:name` can't escape |
| tls | unbounded post-handshake messages; ChangeCipherSpec accepted at any time; no handshake timeout; ALPN not checked against the offer; leftover handshake bytes across a key change | 64 KiB cap; ChangeCipherSpec only as a single `0x01` before the handshake completes; a 20 s handshake deadline; ALPN must be one that was offered; `RecordOverflow` on inner plaintext; KeyUpdate validated; NewSessionTicket only accepted by clients |
| tls client | the server's session id echo and the HelloRetryRequest's choices were not checked | the echo must match; the ServerHello after a retry must keep its suite and group |
| x509 | unbounded signature checks while building a path; name constraints bypassed by a trailing dot or a wildcard; trailing data and duplicate extensions accepted | at most 64 signature checks; names are trimmed and a wildcard is checked as a representative name; both are rejected |
| rsa, der, base64, hmac, aes-gcm, chacha | huge public exponents; non-minimal integers; loose padding; HKDF output past `255 * n`; messages past the AEAD limits | each rejected, or a trap for a programming error |
| hash maps | the hash was unkeyed, so collisions could be precomputed (hash flooding) | SipHash-1-3 with a per-process random key |
| DNS (native) | fixed query id, unconnected socket, answers not matched to the question | random id, connected socket, question checked |

### Memory and runtime

| Where | Problem | Fix |
|---|---|---|
| mem | enormous size requests overflowed size arithmetic; freeing an interior pointer or freeing twice went undetected | a 1 TiB cap (`too_big`); small frees check that the pointer starts an object (a modular-inverse test) and trap on an immediate double free |
| json | the arena chunk size could be smaller than the allocation | `size = max(chunk, n)` |
| vec, str | `v.extend(v.items())` and `s.push_str(s.bytes())` read freed memory after growing | the source is handled when it aliases |
| c | `to_cstr` silently truncated at an interior NUL | rejected; the size is stored in a header |
| fs | NUL in paths; mtime overflow; temporary directories created 0755; `remove_all` and `walk` followed a symbolic link swapped in for a directory | NUL rejected by every runtime; saturating times; 0700; directories opened relative to their parents with `O_NOFOLLOW` (`ax_dir_*`) |
| threads | signal alternate stacks and thread stacks leaked | freed on exit |
| log | CR, LF, C1 controls, U+2028/U+2029 and invalid UTF-8 in messages or keys could forge log lines; failed rotation lost the file | all escaped; the sink reopens in append mode |

### Text formats

| Where | Problem | Fix |
|---|---|---|
| regex | patterns like `((a{100}){100}){100}` compiled for a long time and nested deeply; the DFA cache could grow without limit | a compile work budget (4 × `MAX_PROGRAM`), depth < 1000, and a cap on the DFA cache (`DFA_MAX_PCS`) |
| markdown | the usual CommonMark pathological inputs (nested brackets, emphasis, backticks, list tightness, link reference expansion) | linear-time structures and an expansion budget; safe HTML by default |
| unicode | reordering, word boundaries after long runs of regional indicators, and sentence boundaries were quadratic | a counting sort, a capped RI count, and a memoized sentence suffix scan |
| bidi | isolate matching recursed | an explicit stack |
| path | glob `**` backtracked exponentially | single-backtrack matching |
| draw | the edge sort was quadratic; hostile dash arrays created millions of segments | an O(n log n) sort for more than 32 crossings; bad or tiny dash patterns fall back to a solid line |
| text | `fit` measured the whole prefix for every character | incremental width |
| time | far-away times overflowed | saturating arithmetic |

## What is still open

- **The JavaScript host and symbolic links.** Node has no `openat`, so
  `remove_all` and `walk` under `runtime/host.mjs` still follow a link
  that is swapped in between checking and opening. Native programs are
  safe.
- **Regex `find_all` and `replace_all`** can take quadratic time for some
  patterns, the same as RE2's. Each search is linear, but they search again
  after every match.
- **X.509 name constraints** are checked against the leaf certificate's
  names only, not the names of intermediate certificates.
- **Deep directory trees:** `remove_all` and `walk` keep one descriptor
  open per level, so a tree deeper than the descriptor limit fails with an
  error.
- **Fuzzing** has been run for thousands, not millions, of inputs per
  target. Longer runs are worthwhile after changes to a parser.
