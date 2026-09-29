# Phalange standard library

Import with `import "std/<module>" as <name>;`. Memory comes from the global allocator
in `mem`; nothing allocates behind your back (a function that allocates returns `!T`,
and a type that owns memory has `deinit`).

| Module | Contents |
|---|---|
| [`mem`](mem.ph) | The global allocator: `alloc.[T](n)`, `free(s)`, `realloc(s, n)`, `alloc_zeroed`, `dup`, `create(v)` / `destroy(p)`, `in_use()`; under `--sanitize=address`, shadow memory, quarantine and leak reports (`leaks_ignored`) |
| [`vec`](vec.ph) | `Vec[T]`: growable array. `sort`, `reverse`, `index_of`, `binary_search` on slices |
| [`arena`](arena.ph) | `Arena`: bump allocation freed all at once (`bytes`, `dup`, `copy[T]`), for parsed documents |
| [`hash`](hash.ph) | Hash and equality functions for integers and `[]u8` (`hash_u64`, `hash_bytes`, `eq_bytes`, …) |
| [`xxhash`](xxhash.ph) | XXH32 and XXH64, one-shot and streaming (the checksums of LZ4 and Zstandard) |
| [`map`](map.ph) | `Map[K, V]`: open-addressing hash map, and `Iter[K, V]` |
| [`set`](set.ph) | `Set[K]`: hash set with union, intersection and difference in place |
| [`treemap`](treemap.ph) | `TreeMap[K, V]`: ordered map (AVL tree): `floor`, `ceil`, `min`, `max`, in-order and range iteration |
| [`deque`](deque.ph) | `Deque[T]`: double-ended queue on a ring buffer |
| [`heap`](heap.ph) | `Heap[T]`: priority queue (binary heap), built from a slice in linear time |
| [`bitset`](bitset.ph) | `BitSet`: fixed-size sets of bits, `next_set`/`next_clear`, union/intersection |
| [`rand`](rand.ph) | Seedable pseudo-random numbers (xoshiro256\*\*): integers in ranges without bias, floats, normal, exponential, Poisson and weighted choices, `shuffle`, `choose`, `jump` for parallel streams; not for secrets (see `crypto/random`) |
| [`matrix`](matrix.ph) | `Matrix[T: Num]` (heap, row-major); `Mat4` (4×4 `f32` on `f32x4` columns); SIMD `dot`, `axpy` |
| [`linalg`](linalg.ph) | Linear algebra on `Matrix[f64]`: LU with partial pivoting (`solve`, `det`, `inverse`), Cholesky, Householder QR and least squares (`lstsq`), symmetric eigenproblems (Jacobi), the SVD (one-sided Jacobi), `rank`, `cond`, `norm`, `trace`. Checked against NumPy |
| [`io`](io.ph) | `Writer` / `Reader` interfaces; `stdout`/`stderr`/`stdin`; files (with `seek`, `size`); `BufferWriter`, `SliceReader`, `BufWriter`; integer/float formatting and parsing |
| [`path`](path.ph) | File paths as text: `join`, `clean`, `dir`, `base`, `ext`, `stem`, `with_ext`, `rel`, element iteration; glob matching (`*`, `?`, `[a-z]`, `**`) |
| [`fs`](fs.ph) | The file system: `read_file`, `write_file`, `append_file`, `copy_file`; `stat`/`lstat` (`Info`: kind, size, modification time, mode), `exists`; `mkdir`, `mkdir_all`, `remove`, `remove_all`, `rename`; `read_dir` (sorted), `walk`, `glob`; `cwd`, `absolute`, `temp_dir`, `make_temp_dir` |
| [`os`](os.ph) | The process: `args`/`arg`/`arg_count`, `getenv`/`has_env`/`setenv`/`unsetenv`/`environ`, `pid`, `cwd`/`chdir`, signals (`signal_catch`/`ignore`/`default`, `signal_poll`, `signal_wait`; not on the node host), `exit` |
| [`process`](process.ph) | Child processes: `Command` (arguments, environment changes, directory, each stream inherited, piped or `/dev/null`), `spawn` → `Child` (`Pipe`s with readers and writers, `wait`, `try_wait`, `kill`), `output` (captured without deadlock), `status`, `run` |
| [`log`](log.ph) | Structured logging: levels (`DEBUG`, `INFO`, `WARN`, `ERROR`), typed key-value fields, text (logfmt) or JSON lines, `Logger` with its own fields (`with`), a default logger (`log.info(...)`, `set_level`, `level_from_env`), size-rotated log files (`open_file`); whole lines, safe across threads |
| [`thread`](thread.ph) | `spawn(f, arg)` → `Thread[T]` with `join()`; `parallel_for`; `cpu_count`, `sleep_ms` |
| [`sync`](sync.ph) | `Atomic[T: Int]`, `Mutex`, `Condvar` (with timeouts), `RwLock` (writer-preferring), `Semaphore`, `Once`, `Barrier`, `WaitGroup`, `Channel[T]` (bounded MPMC, closable) |
| [`pool`](pool.ph) | A thread pool: `submit(f, arg)` → `Future[A, R]` (`join`, `done`), fire-and-forget `run`, `wait` for all |
| [`c`](c.ph) | C interop helpers: `from_cstr`, `to_cstr`, `cstr_len`, `null_ptr`, `is_null`; the C↔Phalange type table |
| [`net`](net.ph) | TCP: `connect`, `listen` → `TcpStream` / `TcpListener` (with `io.Reader`/`io.Writer` adapters, timeouts); IPv4 parsing/formatting |
| [`str`](str.ph) | Strings as `[]u8`: comparison, search, `trim`, `cut`, `split` / `lines` / `fields` iterators, ASCII case, `join` / `replace`, UTF-8 (`decode_rune`, `encode_rune`, `valid_utf8`, `runes`); `String`, a growable string with a `Writer` |
| [`unicode`](unicode.ph) | Unicode 15.1: general categories and other properties (`is_letter`, `is_space`, `combining_class`, bidi and line-break classes, ...), terminal display width (`width`, `string_width`), simple and full case mapping (`upper`, `lower`, `title`, `case_fold`, `equal_fold`), normalization (NFC, NFD, NFKC, NFKD), grapheme, word and sentence segmentation (UAX #29), UTF-16. Passes the UCD's conformance tests. The tables ([`unicode/tables`](unicode/tables.ph)) are generated by [`tools/gen_unicode.py`](../tools/gen_unicode.py) |
| [`unicode/linebreak`](unicode/linebreak.ph), [`unicode/bidi`](unicode/bidi.ph) | Line breaking (UAX #14: break opportunities and mandatory breaks) and the bidirectional algorithm (UAX #9: embedding levels, line levels, visual order, mirroring); both pass the UCD conformance tests |
| [`fmt`](fmt.ph) | Templates: `fmt.print("{} is {:>5.2}\n", [fmt.str(s), fmt.float(x)])`, also `write`, `alloc`, `bufprint`; `parse_int[T]`, `parse_float` (correctly rounded), `format_float` (shortest round trip), `format_int` |
| [`time`](time.ph) | `Duration`, the monotonic `Instant`, wall-clock `Time`, calendar fields (`DateTime`), RFC 3339 and HTTP dates, `sleep` |
| [`tz`](tz.ph) | Time zones: TZif files (RFC 8536, v1–v4) from the system's database (`Zone.load`, safe names) or bytes, POSIX TZ rules, `Zone.local` (`$TZ`, `/etc/localtime`); `lookup` (offset, DST, abbreviation), `to_local`, and `from_local` with PEP 495 folds for repeated and skipped times. Checked against Python's `zoneinfo` ([`tz/testdata`](tz/testdata)) |
| [`json`](json.ph) | `parse` → `Document` (a `Value` tree: `get`, `at`, `as_str`, `as_i64`, …); `Writer` (streaming, compact or pretty); `stringify` |
| [`msgpack`](msgpack.ph) | MessagePack: `Value` (nil, bool, int64/uint64, float, str, bin, array, map with any keys, ext), `Timestamp` (extension -1), streaming `Writer` (smallest encodings; `json` writes a JSON value), `encode`, `decode`/`decode_prefix` (checked lengths, UTF-8 and depth, no recursion), `to_json`. Passes the msgpack-test-suite ([`msgpack/testdata`](msgpack/testdata), from [`tools/gen_binary_tests.py`](../tools/gen_binary_tests.py)) |
| [`cbor`](cbor.ph) | CBOR (RFC 8949): `Value` (the whole data model: 65-bit integers, bytes, text, arrays, maps, tags, simple values, half/single/double floats), `Writer` (preferred serialization; indefinite lengths), `encode`, `encode_canonical` (sorted keys), strict `decode` (well-formedness, UTF-8, depth; CBOR sequences via `decode_prefix`), `diagnostic` notation, `to_json`. Passes RFC 8949 Appendix A ([`cbor/testdata`](cbor/testdata)) |
| [`protobuf`](protobuf.ph) | Protocol Buffers wire format: `Encoder` (every scalar type, nested messages, packed fields), `Reader`/`Field` (typed accessors, `last`, `count`, packed `varints`/`fixed`, groups skipped), `decode_raw` (as `protoc --decode_raw`). Checked against the protobuf library's encoder and a real `FileDescriptorProto` ([`protobuf/testdata`](protobuf/testdata), from [`tools/gen_protobuf_tests.py`](../tools/gen_protobuf_tests.py)) |
| [`toml`](toml.ph) | TOML 1.0.0: `parse` → `Document` (`Value`: strings, integers, floats, booleans, `DateTime` of four kinds, arrays, tables; `get`, `path("a.b")`, `at`, typed accessors), `stringify`. Passes toml-test's 709 TOML 1.0 cases ([`toml/testdata`](toml/testdata), from [`tools/gen_toml_tests.py`](../tools/gen_toml_tests.py)) |
| [`yaml`](yaml.ph) | YAML 1.2: `parse` / `parse_all` → `json.Value` trees (core schema: null, booleans, integers, floats, strings; anchors and aliases shared, with a size limit against alias bombs), `Events` (the libyaml-style event stream with tags, anchors and styles), `stringify` (block style). Passes all 402 cases of the yaml-test-suite ([`yaml/testdata`](yaml/testdata), from [`tools/gen_yaml_tests.py`](../tools/gen_yaml_tests.py)) |
| [`xml`](xml.ph) | XML 1.0 with namespaces: `Reader` (pull tokens; UTF-8/UTF-16/Latin-1 input, internal DTD subset with entities and attribute defaults, expansion limits against billion laughs, no external entities), `parse` → `Document`/`Element` DOM (`attr`, `attr_ns`, `elements()`, `text_alloc`), `Writer` (pretty printing), `escape_text`/`escape_attr`, `canonicalize` (C14N). Checked against libxml2 on an 88-case corpus ([`xml/testdata`](xml/testdata), from [`tools/gen_xml_tests.py`](../tools/gen_xml_tests.py)) |
| [`html`](html.ph) | HTML per the WHATWG Living Standard: `parse` / `parse_fragment` / `parse_bytes` (UTF-16 and windows-1252 sniffing) → `Document` (numbered nodes with parent/child/sibling links; `root`, `head`, `body`, `title`, `find`, `children`, `walk`, `attr`, `text_content`; editing with `create_element`, `set_attr`, `append_child`, `insert_before`, `remove`, `clone_tree`), CSS selectors (`query`, `query_all`, `matches`, `compile`), serialization (`to_html`, `inner_html`, `escape`), and the `Tokenizer` alone. Passes all 1,792 html5lib-tests tree construction cases and all 7,028 tokenizer cases ([`html/testdata`](html/testdata), from [`tools/gen_html_tests.py`](../tools/gen_html_tests.py)); linear time on hostile input. [`html/entities.ph`](html/entities.ph) holds the named character references (from [`tools/gen_entities.py`](../tools/gen_entities.py)) |
| [`csv`](csv.ph) | CSV (RFC 4180): a `Reader` of records (quoted fields with delimiters, quotes and line breaks; other delimiters, comments, trimming, lazy quotes, a field limit) and a `Writer` that quotes only when needed |
| [`ini`](ini.ph) | INI files: sections, `key = value` / `key: value`, comments, continuation lines; case-insensitive lookups with typed getters; a `Writer` |
| [`markdown`](markdown.ph) | Markdown: CommonMark 0.31.2 (passes all 652 specification examples) with the GitHub extensions (tables, strikethrough, extended autolinks, task lists, tag filter); `parse` → `Document` (a tree of nodes with `walk`, `text`, `url`, `plain_text`), `to_html`. Safe for untrusted input by default (raw HTML omitted, dangerous URLs dropped), linear time on hostile input. Entities from [`tools/gen_entities.py`](../tools/gen_entities.py) |
| [`http`](http.ph) | HTTP/1.1: a threaded `Server` with keep-alive and chunked bodies (HTTPS with `server_tls` / `Router.serve_tls`), `Router` with `{param}` and `*` patterns, `Request` / `Response`; a client (`get`, `post`, `request`) for `http://` and `https://` URLs; percent-encoding |
| [`tls`](tls.ph) | TLS 1.3 client and server (`connect`, `client`, `server` → `Conn`, a `Reader`/`Writer`): AES-GCM and ChaCha20-Poly1305, X25519 and P-256 key exchange, Ed25519 / ECDSA / RSA certificates, chain and host-name verification against the system's roots, ALPN, SNI, key updates |
| [`deflate`](deflate.ph) | Compression and decompression: DEFLATE in the raw, zlib and gzip formats (levels 0–9, output within a fraction of a percent of zlib's size), whole-buffer (`compress`, `decompress`) or streaming (`Compressor` as an `io.Writer`, `Decompressor` as an `io.Reader`); `crc32`, `adler32` |
| [`lz4`](lz4.ph) | LZ4: `decompress` (frames with any block size, linked blocks, checksums, content sizes; skippable and legacy frames; size-limited), `compress` (frames; levels 1–12 with hash chains), raw `compress_block`/`decompress_block`. Checked against the lz4 library ([`lz4/testdata`](lz4/testdata), from [`tools/gen_compress_tests.py`](../tools/gen_compress_tests.py)) |
| [`bzip2`](bzip2.ph) | bzip2: `decompress` (concatenated streams, block and stream CRCs, size-limited), `compress` (block sizes 1–9: RLE, BWT by prefix doubling, MTF, multi-table Huffman; output as small as bzip2's). Checked against the bz2 library and the bzip2 tool ([`bzip2/testdata`](bzip2/testdata)) |
| [`xz`](xz.ph) | xz and LZMA: `decompress` (.xz streams, concatenated and padded; every block and index record checked; CRC-32/CRC-64/SHA-256; LZMA2 with the delta and BCJ filters for x86, ARM, ARM-Thumb, ARM64, PowerPC and SPARC), `decompress_lzma` (.lzma), `decompress_lzma2`; `compress`/`compress_lzma` (an LZMA encoder with hash chains, repeat matches and lazy parsing, levels 1–9); `crc64`. Checked against liblzma and the xz tool ([`xz/testdata`](xz/testdata)) |
| [`zstd`](zstd.ph) | Zstandard (RFC 8878): `decompress` (frames and skippable frames; raw, RLE and compressed blocks; Huffman literals with direct or FSE-coded trees; FSE sequences in all table modes; repeat offsets; XXH64 checksums; size-limited), `compress` (levels 1–19: hash chains with repeat offsets and lazy matching, Huffman literals, FSE tables fitted per block; smaller than the reference at levels 1–3). Checked against the zstd library ([`zstd/testdata`](zstd/testdata)) |
| [`brotli`](brotli.ph) | Brotli (RFC 7932): `decompress` (all block, context and distance modes; the static dictionary with its 121 transforms; size-limited), `compress` (qualities 1–11: hash chains with the last distance tried first, per-meta-block prefix codes, stored when that is smaller). Checked against the brotli library ([`brotli/testdata`](brotli/testdata)) |
| [`zip`](zip.ph) | ZIP archives: `Archive.open` (central directory, ZIP64, comments, UTF-8 names), `extract` (stored and deflated; CRC and declared sizes enforced, so no zip bombs), `extract_all` (refuses zip-slip names and links), `safe_name`; `Writer` (deflate or store, Unix modes and times, ZIP64 when needed). Checked against Python's zipfile ([`zip/testdata`](zip/testdata), from [`tools/gen_archive_tests.py`](../tools/gen_archive_tests.py)) |
| [`tar`](tar.ph) | tar archives: `Reader` (ustar with prefixes, pax extended and global headers, GNU long names and base-256 numbers, checksums verified), `extract_all` (safe names only), `Writer` (ustar, with pax records for long or non-ASCII names, big sizes and negative times). Checked against Python's tarfile in all three formats ([`tar/testdata`](tar/testdata)) |
| [`base64`](base64.ph) | Base64 encoding and decoding, standard and URL-safe alphabets |
| [`base32`](base32.ph) | Base32 (RFC 4648), standard and extended-hex alphabets, padded or not |
| [`hex`](hex.ph) | Hexadecimal encoding and decoding; `xxd`-style hex dumps |
| [`mime`](mime.ph) | Media types by extension, `Content-Type` parsing, header parameters (quoted strings, RFC 2231 `name*`), quoted-printable, multipart bodies (`Multipart` reader, `MultipartWriter` for form-data) |
| [`math`](math.ph) | Elementary functions on `f64` (and `f32`, suffixed `f`): `exp`, `expm1`, `log`, `log1p`, `log2`, `log10`, `pow`, `cbrt`, `hypot`, `sin`, `cos`, `tan`, `sincos`, `asin`, `acos`, `atan`, `atan2`, `sinh`, `cosh`, `tanh`, `ldexp`, `frexp`; the special functions `erf`, `erfc`, `gamma`, `lgamma`; constants. Within 1 ulp (2 for `tan`, `expm1`, `sinh`, `tanh`; 2–8 for the special functions), trigonometry of huge arguments included |
| [`complex`](complex.ph) | `Complex` numbers on `f64`: arithmetic (Smith's division), `abs`, `arg`, `polar`, `exp`, `log`, `sqrt`, `pow`, trigonometric and hyperbolic functions and their inverses on the principal branches of C99 (Kahan's formulas, signed zeros respected); `format`. Checked against Python's `cmath` |
| [`fft`](fft.ph) | Fast Fourier transforms of any length in O(n log n): radix-2, and Bluestein's algorithm otherwise; `forward`, `inverse`, reusable `Plan`s, real `rfft`/`irfft`, `convolve` |
| [`stats`](stats.ph) | Statistics on `[]f64`: compensated `sum`, `mean`, sample and population variance and deviation, `median`, `quantile` (R/NumPy's default), geometric and harmonic means, `covariance`, Pearson and Spearman correlation, `ranks`, `linear_regression`, `histogram`; one-pass `Running` statistics (Welford, mergeable); the normal `pdf`, `cdf`, `sf` and `quantile` |
| [`color`](color.ph) | `Color` (linear-light RGB + alpha, HDR-capable), `Rgba8`/`Rgba16` (sRGB-encoded); sRGB encoding (exact 8-bit tables), HSV, HSL, CIE XYZ, L\*a\*b\*, LCh, Oklab, Oklch, YCbCr; Display P3 and Rec. 2020 gamuts; PQ and HLG transfer functions, tone mapping; CIE76 / CIEDE2000 differences, WCAG contrast; `mix`, `over`, premultiplied alpha; CSS-style parsing and hex formatting |
| [`image`](image.ph) | `Image`: 1–4 channels of U8, U16 (sRGB-encoded) or F32 (linear, HDR) samples; `get`/`set` as `color.Color`, `convert`, `crop`, `resize` (linear-light filtering), `flip_vertical`; half-float conversion |
| [`draw`](draw.ph) | 2D vector drawing onto an `Image`: `Path` (lines, quadratic and cubic Béziers, arcs, rectangles, rounded rectangles, ellipses), `Canvas` with affine transforms, fills (nonzero, even-odd), strokes (width, miter/round/bevel joins, butt/round/square caps, dashes), paints (solid, linear and radial gradients, images), Porter-Duff operators and blend modes in linear light, global alpha, clipping, save/restore; exact anti-aliasing |
| [`font`](font.ph) | TrueType and OpenType (CFF) fonts and collections: `Font` (cmap formats 0/4/6/12, advances, GPOS and `kern` kerning, simple, composite and Type 2 charstring outlines as `draw.Path`s), text `layout` with word wrapping, `measure`, `draw_text`, `draw_text_box`; unhinted, no shaping beyond kerning |
| [`text`](text.ph) | Rich text layout: styled spans (font, fallback fonts, size, colour, underline, strike-through, letter spacing), UAX #14 line breaking with grapheme-safe emergency breaks, bidirectional text, kerning, tabs, alignment (start, end, left, right, centre, justify), line spacing; `Layout` with glyph and line positions, hit testing (`offset_at`), `draw` onto a canvas, `measure` |
| [`pdf`](pdf.ph) | Writing PDF: pages of vector graphics (`draw.Path`s filled and stroked with solid colours, alpha and linear/radial gradients; clipping; transforms), text in embedded TrueType (subset) and OpenType CFF fonts with ToUnicode maps, `draw_layout` for `text.Layout`s, images (Flate with soft-mask alpha, or JPEG as is), links, bookmarks, metadata; compressed |
| [`pdf/read`](pdf/read.ph) | Reading PDF: cross-reference tables and streams, object streams, incremental updates, repair of damaged files; objects and decoded streams (Flate with predictors, LZW, ASCIIHex, ASCII85, RunLength); pages, metadata, and text extraction (ToUnicode maps, simple-font encodings and glyph names, layout into lines and words). Hardened against hostile files. Tables from [`tools/gen_pdf_tables.py`](../tools/gen_pdf_tables.py) |
| [`audio`](audio.ph) | `Sound`: interleaved frames of U8, S16, S24, S32, F32 or F64 samples; `get`/`set` as floats, `convert` (exact integer ↔ float), `remix` (channel counts), `slice`, `gain`, `peak`, `resample` (Kaiser-windowed sinc, > 90 dB rejection) |
| [`bigint`](bigint.ph) | `BigInt`: arbitrary-precision integers — arithmetic (Karatsuba multiplication), truncated and Euclidean division, shifts and bitwise operations, `pow`, `mod_pow` (Montgomery; `mod_pow_secret` in constant time), `gcd`, `mod_inverse`, `sqrt`, conversion to and from any base and bytes |
| [`decimal`](decimal.ph) | `Decimal`: arbitrary-precision decimal numbers (a `BigInt` times a power of ten) — exact `add`, `sub`, `mul`; `div` and `sqrt` to a precision; `quantize`, `round`, `round_sig` with seven rounding modes; `cmp`, `normalize`; parsing and Python's string format; bounded against hostile exponents. Checked against Python's `decimal` ([`decimal/testdata`](decimal/testdata)) |
| [`regex`](regex.ph) | Linear-time regular expressions (RE2 syntax): `find`, `captures` (named groups), `find_all`, `replace_all`; a lazy DFA with a Pike VM for captures |
| [`diff`](diff.ph) | Differences between texts (or token sequences) by Myers's algorithm in linear space, minimal but bounded on pathological input as in GNU diff; `lines`, `tokens`, `unified` (byte-for-byte `diff -u` format, `\ No newline at end of file` included), `apply` (strict patching). Patches applied by GNU `patch` |
| [`template`](template.ph) | Mustache templates over `json.Value` data: escaped and raw interpolation, dotted names, sections, inverted sections, comments, partials, set-delimiter tags, standalone-line rules; compile once and render often. Passes the 136 tests of the Mustache specification's required modules ([`template/testdata`](template/testdata)); safe for untrusted templates |
| [`hyphen`](hyphen.ph) | Hyphenation by Liang's algorithm (TeX's): `Hyphenator` from patterns and exceptions in TeX's format (UTF-8), `breaks`, `hyphenate`; American English built in ([`hyphen/en_us.ph`](hyphen/en_us.ph), TeX's ushyphmax patterns). Checked against the TeXbook and a reference implementation ([`tools/gen_hyphen.py`](../tools/gen_hyphen.py)) |

Image files are read and written by `std/image/`:

| Module | Contents |
|---|---|
| [`image/formats`](image/formats.ph) | Any format: `detect`, `decode`, `encode`, `load(path)`, `save(path)` (by extension) |
| [`image/png`](image/png.ph) | PNG: every colour type and bit depth, interlacing, transparency; writes 8/16-bit with per-row filters |
| [`image/jpeg`](image/jpeg.ph) | JPEG: baseline and progressive, any sampling, restart markers, gray/YCbCr/RGB/CMYK/YCCK, output identical to libjpeg's; writes baseline with optimized Huffman tables |
| [`image/bmp`](image/bmp.ph) | BMP: 1–32 bits, palettes, bit fields, RLE4/RLE8, all header versions |
| [`image/qoi`](image/qoi.ph) | QOI |
| [`image/pnm`](image/pnm.ph) | PBM, PGM, PPM (ASCII and binary), PAM, PFM (float) |
| [`image/hdr`](image/hdr.ph) | Radiance RGBE (HDR) |
| [`image/exr`](image/exr.ph) | OpenEXR (HDR): scanline and tiled, NONE/RLE/ZIPS/ZIP/PIZ/PXR24, half/float/uint; writes ZIP, ZIPS, RLE or NONE |

Sound files are read and written by `std/audio/`:

| Module | Contents |
|---|---|
| [`audio/formats`](audio/formats.ph) | Any format: `detect`, `decode`, `encode`, `load(path)`, `save(path)` (by extension); WAV, AIFF and FLAC are written, all five read |
| [`audio/wav`](audio/wav.ph) | WAV: PCM 8–32 bits, float, A-law, µ-law, WAVE_FORMAT_EXTENSIBLE, RF64 |
| [`audio/aiff`](audio/aiff.ph) | AIFF and AIFF-C: big- and little-endian PCM, float, A-law, µ-law |
| [`audio/flac`](audio/flac.ph) | FLAC: decodes every stream (4–32 bits, 1–8 channels; CRCs and MD5 checked); encodes at levels 0–8, about as small as the reference encoder |
| [`audio/vorbis`](audio/vorbis.ph) | Ogg Vorbis decoding: any channel count and rate, floor 1, residues 0–2, coupling; trimmed to the granule positions; output within 3·10⁻⁷ of libvorbis |
| [`audio/mp3`](audio/mp3.ph) | MP3 decoding (MPEG-1/2/2.5 layer III): CBR and VBR, joint stereo (mid/side, intensity), short and mixed blocks, the bit reservoir, LAME gapless trimming; output within 10⁻⁶ of libmpg123 |
| [`audio/ogg`](audio/ogg.ph) | The Ogg container: pages (CRC-checked) and packets |

Cryptography lives in `std/crypto/` (the primitives are written to run in constant
time where secrets are involved):

| Module | Contents |
|---|---|
| [`crypto/random`](crypto/random.ph) | Secure random bytes from the operating system: `fill`, `uint64`, `below` |
| [`crypto/sha2`](crypto/sha2.ph) | SHA-256, SHA-384, SHA-512; a `Hasher` over any of them (and SHA-1) |
| [`crypto/sha1`](crypto/sha1.ph) | SHA-1, for protocols that require it (WebSocket, HMAC-SHA1 in TOTP); not for new signatures |
| [`crypto/sha3`](crypto/sha3.ph) | SHA3-224/256/384/512 and SHAKE128/256 (Keccak) |
| [`crypto/blake2`](crypto/blake2.ph) | BLAKE2b and BLAKE2s, keyed or not, any digest size |
| [`crypto/kdf`](crypto/kdf.ph) | Password-based key derivation: PBKDF2 (over any `Hasher`), scrypt |
| [`crypto/argon2`](crypto/argon2.ph) | Argon2id, Argon2i, Argon2d (RFC 9106); PHC strings (`hash_encoded`, `verify_encoded`) |
| [`crypto/md5`](crypto/md5.ph) | MD5, for formats that use it as a checksum (not for security) |
| [`crypto/hmac`](crypto/hmac.ph) | HMAC; HKDF (`extract`, `expand`) |
| [`crypto/subtle`](crypto/subtle.ph) | Constant-time comparison; `zero` for secrets |
| [`crypto/aes_gcm`](crypto/aes_gcm.ph) | AES-128/192/256 (encryption and decryption) and AES-GCM (`seal`, `open`) |
| [`crypto/aes`](crypto/aes.ph) | AES in CBC (PKCS#7 padding, checked in constant time) and CTR modes, for formats that need them |
| [`crypto/chacha20poly1305`](crypto/chacha20poly1305.ph) | ChaCha20, Poly1305 and their AEAD |
| [`crypto/curve25519`](crypto/curve25519.ph) | X25519 key agreement and Ed25519 signatures |
| [`crypto/ec`](crypto/ec.ph) | P-256 and P-384: ECDH and ECDSA |
| [`crypto/rsa`](crypto/rsa.ph) | RSA signatures, PKCS #1 v1.5 and PSS (over `bigint`) |
| [`crypto/der`](crypto/der.ph) | Reading ASN.1 DER |
| [`crypto/x509`](crypto/x509.ph) | X.509 certificates, PEM, private keys, `CertPool` (with the system's roots), chain verification |

Each module carries its own tests: `phalange test std/<module>.ph`. The cryptography
has not been audited: review it before relying on it.

## Conventions

- **Interfaces are structs of function pointers.** `Writer` and `Reader`
  each hold a context pointer plus the functions that operate on it. This is how the
  language expresses polymorphism without traits or vtables.
- **Maps take their hash and equality functions at construction.** For example:
  `Map.[u64, V].init(hash.hash_u64, hash.eq_u64)`.
- **Errors, not sentinels.** Lookups that can fail return `!T` (for example
  `error.NotFound`, `error.Empty`). The idiom for "value or default" is
  `m.get(k) catch 0`.
- **Threads share memory; synchronize it.** `spawn` copies its argument, so pass a
  pointer to shared data and protect it with `std/sync`, or use `parallel_for`, which
  hands each thread a disjoint range. The allocator is thread-safe; `Vec` and `Map`
  are not.
- **Adapters point at their source.** `BufferWriter.writer()`
  and similar return values that point at the object they came from, so don't move
  that object while the adapter is in use.

## Example

```phalange
import "std/hash" as hash;
import "std/io" as io;
import "std/map" as map;

export fn main() i32 {
    let mut ages = map.Map.[[]u8, u32].init(hash.hash_bytes, hash.eq_bytes);
    defer ages.deinit();
    ages.put("ada", 36) catch return 1;
    io.stdout().print_u64(@as(u64, ages.get("ada") catch 0)) catch return 1;
    return 0;
}
```

See also [`examples/wordcount.ph`](../examples/wordcount.ph) and
[`examples/http_server.ph`](../examples/http_server.ph), a JSON API over HTTP.

Iterators (`str.split`, `regex.find_all`, …) have a `next()` that fails with
`error.Done` after the last item:

```phalange
let mut it = str.split(line, ",");
loop {
    let field = it.next() catch break;
    ...
}
```

## Host ABI

`io` and `net` need an operating system, which `wasm32-freestanding` does not have.
They call a small set of host functions imported from module `"phalange"`:

| Function | Meaning |
|---|---|
| `ph_write(fd, p, n) isize`, `ph_read(fd, p, n) isize` | read and write file descriptors; `0` from read means end of file |
| `ph_open(path, len, flags) i32`, `ph_close(fd)` | `flags`: `0` read, `1` create/truncate, `2` append |
| `ph_seek(fd, off, whence) i64` | moves a file's position (`whence`: `0` start, `1` current, `2` end); returns the new position |
| `ph_stat(path, len, follow, out) i32` | `out[0..4]` (`i64`): kind (`1` file, `2` directory, `3` symbolic link, `4` other), size, modification time in ns, permission bits; `follow = 0` does not follow a final symbolic link |
| `ph_mkdir(path, len, mode) i32`, `ph_remove(path, len, dir) i32`, `ph_rename(from, flen, to, tlen) i32` | `mode`: permission bits for the new directory; `dir = 1` removes an (empty) directory |
| `ph_readdir(path, len, buf, cap) isize` | the entries (not `.`, `..`) as a kind byte (`0` unknown), the name and a `0` byte each; returns the bytes needed, which may exceed `cap` |
| `ph_dir_open(dir, name, len) i32`, `ph_dir_read(dir, buf, cap) isize`, `ph_dir_remove(dir, name, len, isdir) i32`, `ph_dir_close(dir)` | directory handles (`dir = -1`: the working directory) for walking trees: opening never follows a symbolic link (`-40`) and fails with `-20` on a non-directory; the entries are as `ph_readdir`'s. The JavaScript host keeps paths, so it cannot stop a link swapped in meanwhile |
| `ph_getcwd(buf, cap) isize` | the working directory; returns its length |
| `ph_argc() i32`, `ph_arg(i, buf, cap) isize` | the program's arguments (argument 0 is its name); returns the length |
| `ph_getenv(name, len, buf, cap) isize` | an environment variable; `-2` if unset |
| `ph_exit(code)` | ends the process |
| `ph_tcp_connect(name, len, port) i32`, `ph_tcp_listen(name, len, port) i32` | return a socket handle |
| `ph_tcp_accept(h) i32`, `ph_tcp_local_port(h) i32` | |
| `ph_sock_read(h, p, n) isize`, `ph_sock_write(h, p, n) isize`, `ph_sock_close(h)` | |
| `ph_sock_timeout(h, ms) i32` | later reads and writes on `h` fail with `-5` after `ms` milliseconds (`0`: never) |
| `ph_thread_spawn(entry, arg, stack_top, exited) i32`, `ph_thread_stack_size() usize` | threads (spec §12.8.5) |
| `ph_cpu_count() u32`, `ph_sleep_ns(ns)` | |
| `ph_random(p, n) i32` | fills `p[0..n]` with cryptographically secure random bytes |
| `ph_clock_ns(clock) i64` | nanoseconds on clock `0` (real time since 1970) or `1` (monotonic) |

A negative result means failure. The codes are `-1` general failure, `-2` connection
refused / not found, `-3` address in use, `-4` invalid address, `-5` timed out, `-13`
access denied, `-17` already exists, `-20` not a directory, `-21` is a directory and `-39`
directory not empty. Functions that fill a buffer return the length the whole result
needs; when that is more than `cap`, the buffer was not written and the caller retries
with more room.

Two implementations are provided:

- **[`runtime/host.mjs`](../runtime/host.mjs)** for node, usable as a runner:
  `node runtime/host.mjs prog.wasm`. Node sockets are asynchronous, so they live in
  a worker thread, and the wasm thread blocks on `Atomics.wait` until each
  operation completes.
- **[`runtime/native_host.c`](../runtime/native_host.c)** for POSIX. The C compiler
  links it automatically when a native program uses the host ABI. The self-hosted
  compiler builds the same host ABI written in Phalange,
  [`runtime/native.ph`](../runtime/native.ph), into every native program. It makes
  Linux system calls directly ([`runtime/linux.ph`](../runtime/linux.ph)) and needs no
  C library, so those programs are static executables.
