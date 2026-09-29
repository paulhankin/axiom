# Phalange standard library

Import with `import "std/<module>" as <name>;`. Memory comes from the global allocator
in `mem`; nothing allocates behind your back (a function that allocates returns `!T`,
and a type that owns memory has `deinit`).

| Module | Contents |
|---|---|
| [`mem`](mem.ph) | The global allocator: `alloc.[T](n)`, `free(s)`, `realloc(s, n)`, `alloc_zeroed`, `dup`, `create(v)` / `destroy(p)`, `in_use()` |
| [`vec`](vec.ph) | `Vec[T]`: growable array. `sort`, `reverse`, `index_of`, `binary_search` on slices |
| [`hash`](hash.ph) | Hash and equality functions for integers and `[]u8` (`hash_u64`, `hash_bytes`, `eq_bytes`, …) |
| [`map`](map.ph) | `Map[K, V]`: open-addressing hash map, and `Iter[K, V]` |
| [`matrix`](matrix.ph) | `Matrix[T: Num]` (heap, row-major); `Mat4` (4×4 `f32` on `f32x4` columns); SIMD `dot`, `axpy` |
| [`io`](io.ph) | `Writer` / `Reader` interfaces; `stdout`/`stderr`/`stdin`; files; `BufferWriter`, `SliceReader`, `BufWriter`; integer/float formatting and parsing |
| [`thread`](thread.ph) | `spawn(f, arg)` → `Thread[T]` with `join()`; `parallel_for`; `cpu_count`, `sleep_ms` |
| [`sync`](sync.ph) | `Atomic[T: Int]`, `Mutex`, `WaitGroup`, `Channel[T]` (bounded MPMC, closable) |
| [`c`](c.ph) | C interop helpers: `from_cstr`, `to_cstr`, `cstr_len`, `null_ptr`, `is_null`; the C↔Phalange type table |
| [`net`](net.ph) | TCP: `connect`, `listen` → `TcpStream` / `TcpListener` (with `io.Reader`/`io.Writer` adapters, timeouts); IPv4 parsing/formatting |
| [`str`](str.ph) | Strings as `[]u8`: comparison, search, `trim`, `cut`, `split` / `lines` / `fields` iterators, ASCII case, `join` / `replace`, UTF-8 (`decode_rune`, `encode_rune`, `valid_utf8`, `runes`); `String`, a growable string with a `Writer` |
| [`fmt`](fmt.ph) | Templates: `fmt.print("{} is {:>5.2}\n", [fmt.str(s), fmt.float(x)])`, also `write`, `alloc`, `bufprint`; `parse_int[T]`, `parse_float` (correctly rounded), `format_float` (shortest round trip), `format_int` |
| [`time`](time.ph) | `Duration`, the monotonic `Instant`, wall-clock `Time`, calendar fields (`DateTime`), RFC 3339 and HTTP dates, `sleep` |
| [`json`](json.ph) | `parse` → `Document` (a `Value` tree: `get`, `at`, `as_str`, `as_i64`, …); `Writer` (streaming, compact or pretty); `stringify` |
| [`http`](http.ph) | HTTP/1.1: a threaded `Server` with keep-alive and chunked bodies (HTTPS with `server_tls` / `Router.serve_tls`), `Router` with `{param}` and `*` patterns, `Request` / `Response`; a client (`get`, `post`, `request`) for `http://` and `https://` URLs; percent-encoding |
| [`tls`](tls.ph) | TLS 1.3 client and server (`connect`, `client`, `server` → `Conn`, a `Reader`/`Writer`): AES-GCM and ChaCha20-Poly1305, X25519 and P-256 key exchange, Ed25519 / ECDSA / RSA certificates, chain and host-name verification against the system's roots, ALPN, SNI, key updates |
| [`deflate`](deflate.ph) | Compression and decompression: DEFLATE in the raw, zlib and gzip formats (levels 0–9, output within a fraction of a percent of zlib's size), whole-buffer (`compress`, `decompress`) or streaming (`Compressor` as an `io.Writer`, `Decompressor` as an `io.Reader`); `crc32`, `adler32` |
| [`base64`](base64.ph) | Base64 encoding and decoding, standard and URL-safe alphabets |
| [`math`](math.ph) | Elementary functions on `f64` (and `f32`, suffixed `f`): `exp`, `expm1`, `log`, `log1p`, `log2`, `log10`, `pow`, `cbrt`, `hypot`, `sin`, `cos`, `tan`, `sincos`, `asin`, `acos`, `atan`, `atan2`, `sinh`, `cosh`, `tanh`, `ldexp`, `frexp`; constants. Within 1 ulp (2 for `tan`, `expm1`, `sinh`, `tanh`), trigonometry of huge arguments included |
| [`color`](color.ph) | `Color` (linear-light RGB + alpha, HDR-capable), `Rgba8`/`Rgba16` (sRGB-encoded); sRGB encoding (exact 8-bit tables), HSV, HSL, CIE XYZ, L\*a\*b\*, LCh, Oklab, Oklch, YCbCr; Display P3 and Rec. 2020 gamuts; PQ and HLG transfer functions, tone mapping; CIE76 / CIEDE2000 differences, WCAG contrast; `mix`, `over`, premultiplied alpha; CSS-style parsing and hex formatting |
| [`image`](image.ph) | `Image`: 1–4 channels of U8, U16 (sRGB-encoded) or F32 (linear, HDR) samples; `get`/`set` as `color.Color`, `convert`, `crop`, `resize` (linear-light filtering), `flip_vertical`; half-float conversion |
| [`bigint`](bigint.ph) | `BigInt`: arbitrary-precision integers — arithmetic (Karatsuba multiplication), truncated and Euclidean division, shifts and bitwise operations, `pow`, `mod_pow` (Montgomery; `mod_pow_secret` in constant time), `gcd`, `mod_inverse`, `sqrt`, conversion to and from any base and bytes |
| [`regex`](regex.ph) | Linear-time regular expressions (RE2 syntax): `find`, `captures` (named groups), `find_all`, `replace_all`; a lazy DFA with a Pike VM for captures |

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

Cryptography lives in `std/crypto/` (the primitives are written to run in constant
time where secrets are involved):

| Module | Contents |
|---|---|
| [`crypto/random`](crypto/random.ph) | Secure random bytes from the operating system: `fill`, `uint64`, `below` |
| [`crypto/sha2`](crypto/sha2.ph) | SHA-256, SHA-384, SHA-512; a `Hasher` over any of them |
| [`crypto/hmac`](crypto/hmac.ph) | HMAC; HKDF (`extract`, `expand`) |
| [`crypto/subtle`](crypto/subtle.ph) | Constant-time comparison |
| [`crypto/aes_gcm`](crypto/aes_gcm.ph) | AES-128/256 and AES-GCM (`seal`, `open`) |
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
| `ph_tcp_connect(name, len, port) i32`, `ph_tcp_listen(name, len, port) i32` | return a socket handle |
| `ph_tcp_accept(h) i32`, `ph_tcp_local_port(h) i32` | |
| `ph_sock_read(h, p, n) isize`, `ph_sock_write(h, p, n) isize`, `ph_sock_close(h)` | |
| `ph_sock_timeout(h, ms) i32` | later reads and writes on `h` fail with `-5` after `ms` milliseconds (`0`: never) |
| `ph_thread_spawn(entry, arg, stack_top, exited) i32`, `ph_thread_stack_size() usize` | threads (spec §12.8.5) |
| `ph_cpu_count() u32`, `ph_sleep_ns(ns)` | |
| `ph_random(p, n) i32` | fills `p[0..n]` with cryptographically secure random bytes |
| `ph_clock_ns(clock) i64` | nanoseconds on clock `0` (real time since 1970) or `1` (monotonic) |

A negative result means failure. The codes are `-1` general failure, `-2` connection
refused / not found, `-3` address in use, `-4` invalid address, `-5` timed out, and `-13`
access denied.

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
