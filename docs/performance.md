# Performance of the standard library

This is a record of a review of `std/` for speed: how it was measured, what
was slow, and what changed.

## How to measure

    axiom build --target=native -O2 cmd/axiom/internal/tests/bench/std.ax -o build/bench/std
    build/bench/std [filter]      # Axiom; also writes the inputs to build/bench/
    cmd/axiom/internal/tests/tools/bench_std.py [filter]   # the same work in zlib, OpenSSL, libjpeg, Pillow, ...

`cmd/axiom/internal/tests/bench/std.ax` times the standard library's heavy operations on
generated inputs (4 MB of English-like text, a 4 MB JSON document, a
1024×768 image, a 2 MB Markdown document, and so on). It saves those inputs so
that `cmd/axiom/internal/tests/tools/bench_std.py` can run the same work through Python's bindings
to the usual C libraries. Those libraries do the work, so Python's own
overhead hardly counts for these bulk operations.

To find where the time goes, use the sampling profiler:

    axiom profile prog.ax --target=native -O2

## Results

| Operation | Before | After | Reference |
|---|---:|---:|---:|
| AES-128-GCM seal | 17.8 MB/s | 111 MB/s | 749 MB/s (OpenSSL, AES-NI) |
| ChaCha20-Poly1305 seal | 120 MB/s | 423 MB/s | 751 MB/s (OpenSSL, AVX2) |
| SHA-256 | 182 MB/s | 227 MB/s | 1241 MB/s (OpenSSL, SHA-NI) |
| bigint mod_pow, 2048 bits | 55 ms | 17.6 ms | 21 ms (Python `pow`) |
| deflate compress, level 6 | 11 MB/s | 15 MB/s | 21 MB/s (zlib) |
| deflate decompress | 184 MB/s | 200 MB/s | 258 MB/s (zlib) |
| base64 decode | 222 MB/s | 1091 MB/s | 622 MB/s (CPython) |
| JSON parse | 168 MB/s | 179 MB/s | 44 MB/s (CPython `json`) |
| JSON stringify | 78 MB/s | 104 MB/s | 59 MB/s (CPython `json`) |
| regex find_all (words) | 150 MB/s | 157 MB/s | 11 MB/s (CPython `re`) |
| Markdown to HTML (GFM) | 14.5 MB/s | 16.1 MB/s | 7.2 MB/s (cmark-gfm) |
| PNG encode | 6.4 MB/s | 7.8 MB/s | 7.0 MB/s (Pillow, zlib) |
| PNG decode | 56 MB/s | 123 MB/s | 142 MB/s (Pillow) |
| JPEG encode, quality 90 | 51 MB/s | 191 MB/s | 855 MB/s (libjpeg-turbo, SIMD) |
| JPEG decode | 240 MB/s | 304 MB/s | 685 MB/s (libjpeg-turbo, SIMD) |
| image resize (upscale 3×) | 16.7 MB/s | 83 MB/s | 232 MB/s (Pillow, Lanczos, SIMD) |
| draw: fill circles | 0.25 ms each | 0.06 ms each | |
| draw: stroke curves | 1.2 ms each | 0.75 ms each | |
| text layout | 1.7 MB/s | 5.2 MB/s | |
| PDF write (laid-out text) | 1.0 MB/s | 2.6 MB/s | |
| Unicode graphemes (ASCII) | 74 MB/s | 327 MB/s | |
| Unicode upper (ASCII) | 141 MB/s | 186 MB/s | |

The machine was a 4-core 2.1 GHz Xeon VM, and the numbers vary by 10–20%
from run to run. "Before" is the library at the start of the review. The
reference column is a C library with hand-written SIMD, or hardware
instructions such as AES-NI and SHA-NI where it has them. At the time of
this table Axiom had no way to use those instructions; it has now (see
[Hardware instructions](#hardware-instructions) below).

## What changed

**The compiler.** `@swizzle` became `pshufb` on x86-64 CPUs that have
SSSE3, and `i8x16.swizzle` on WebAssembly. Before, it was lowered to 16
scalar lane operations. The self-hosted compiler already did this.

**Checked arithmetic in inner loops.** Overflow checks are cheap one at a
time, but in the innermost loops of Poly1305, Montgomery multiplication and
YCbCr conversion they took most of the time. Those sums provably cannot
overflow; wrapping operators (`+%`, `*%`) now say so, with a comment on why.
(Since issue 0047 plain `+` and `*` wrap too, and checks exist only in
`--overflow=trap` builds.)

| Module | Change | Speed-up |
|---|---|---|
| `crypto/aes_gcm` | `pshufb` S-box lookups without masks; four counter blocks per pass through the S-box rows; GHASH as BearSSL's `ctmul64` (6 bit reversals per block instead of 15). Still constant-time. | 6.3× |
| `crypto/chacha20poly1305` | ChaCha20 on four-word rows with lane rotations (SIMD); Poly1305 without overflow checks | 3.5× |
| `bigint` | Montgomery multiplication and schoolbook limb loops without overflow or repeated bounds checks | 3.1× (`mod_pow`), 4.6× (`mul`) |
| `image/jpeg` encode | colour conversion once into padded planes (not per sample, with clamped coordinates); a 2×2 box filter for chroma; the DCT on `f32x4`, bit-identical to before | 3.7× |
| `image/jpeg` decode | wrapping YCbCr arithmetic, branchless clamping, upsampling loops without per-pixel bounds tests | 1.3× |
| `image` resize | a separable filter over a ring of horizontally filtered rows (it was a 2-D loop reading every tap with `get`); `srgb8` starts its search from a 4096-entry bucket table | 5× |
| `image/png` decode | 8-bit rows that need no expansion are copied whole | 2.2× |
| `deflate` compress | match lengths compared 16 bytes at a time | 1.4× |
| `draw` | a fast path writing opaque solid colour straight into 8-bit pixels; the rasterizer works only on the columns a row touches, keeps each row's crossings in order from one sample line to the next (insertion sort), merges spans while inside the shape, and sweeps edges sorted by top | 4× fills, 1.6× strokes |
| `text` | kerning pairs and ASCII glyph lookups cached for each layout | 3× |
| `pdf` write | (text layout, above) | 2.6× |
| `base64` decode | four characters at a time through a table | 4.9× |
| `unicode` | ASCII fast paths for grapheme iteration and case mapping | 4.4× graphemes, 1.3× upper |
| `tls` | a record header that isn't TLS (plain HTTP to a TLS port) is refused at once instead of waiting for the rest of a record that never comes: the `https` test went from 5.5 s to 0.35 s | |

## Still slower than it could be

- **AES-GCM** with hardware instructions is still several times slower
  than OpenSSL, which interleaves eight AES blocks with GHASH in 256- and
  512-bit registers (VAES, VPCLMULQDQ) and reduces once per eight blocks.
- **SHA-512 and SHA-3 on x86-64** have no hardware path: only the newest
  processors have SHA512 instructions, and none have SHA-3 ones. SHA-3 is
  particularly slow under the self-hosted compiler, which keeps the 25-word
  state in memory.
- **Strokes** are built as one small polygon per segment and join, then
  rasterized with 16 sample lines per row. An outline stroker, or analytic
  coverage, would be faster.
- **Deflate compression** is about 30% slower than zlib at the same level.
- **PNG encoding** spends its time in deflate, as zlib-based encoders do.

## Hardware instructions

Issue 0016 added builtins for the processors' cryptographic instructions
(spec §13.5): AES rounds, carry-less multiplication, SHA-1/SHA-256/SHA-512
rounds, SHA-3's three-way XOR and rotates, and CRC steps. The standard
library picks them at run time with `@multiversion` and falls back to the
portable code (still used under WebAssembly, and on processors without
them):

| Module | x86-64 | AArch64 |
|---|---|---|
| `crypto/aes_gcm` | AES-NI rounds; GHASH by PCLMULQDQ (Karatsuba, four blocks per reduction) | AESE/AESMC; GHASH by PMULL |
| `crypto/sha1`, `crypto/sha2` (SHA-256) | SHA-NI | SHA1 and SHA256 instructions |
| `crypto/sha2` (SHA-384, SHA-512) | (portable) | SHA512 instructions |
| `crypto/sha3` | (portable) | EOR3, RAX1, XAR, BCAX |
| `hash/crc` (CRC-32, also `compress/deflate.crc32`) | folding by PCLMULQDQ, 64 bytes a step | CRC32 instructions |
| `hash/crc` (CRC-32C) | SSE4.2 `crc32` | CRC32C instructions |

Each has a test that compares the hardware path with the portable one on
random inputs; the AArch64 paths are tested under qemu.

Measured on a 32-thread x86-64 machine with AVX-512, SHA-NI and VAES
(Linux under WSL 2), `-O2`, with the self-hosted compiler; the C compiler's
numbers are in parentheses where they differ by more than 10%. "Portable"
is the code before the change on the same machine. The reference is
`cmd/axiom/internal/tests/tools/bench_std.py` (Python's `hashlib`, `zlib` and `cryptography`, which
use OpenSSL and zlib).

| Operation | Portable | Hardware | Reference |
|---|---:|---:|---:|
| AES-128-GCM seal | 155 MB/s (219) | 1773 MB/s (3472) | 1813 MB/s (OpenSSL) |
| SHA-256 | 382 MB/s | 2859 MB/s | 2933 MB/s (OpenSSL) |
| SHA-1 | 442 MB/s | 3121 MB/s | 3120 MB/s (OpenSSL) |
| CRC-32 | 2334 MB/s (3125) | 22451 MB/s | 6908 MB/s (zlib) |
| CRC-32C | 743 MB/s | 15785 MB/s | |
| SHA-512 (no hardware path here) | 581 MB/s (756) | | 1398 MB/s (OpenSSL) |
| SHA3-256 (no hardware path here) | 67 MB/s (285) | | 801 MB/s (OpenSSL) |

(`openssl speed -evp aes-128-gcm` reaches 17.7 GB/s on
this machine: Python's `cryptography` allocates and copies for each call.)
