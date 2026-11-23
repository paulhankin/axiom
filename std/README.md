# Axiom standard library

Import with `import "std/<path>" as <name>;`, for example
`import "std/collections/vec" as vec;`. This is the core: what every program and the
compiler itself build on. It imports nothing outside `std/`, and the test suite holds it
to rules the rest of the project adopts module by module: every public item is
documented (`W0320`), iterators follow one protocol (spec §15.3), and its tests pass
under the address sanitizer and with integer overflow trapping. The libraries released
with it, from network protocols, data formats, compression and cryptography to text
processing, graphics and sound, are in [`lib/`](../lib/README.md) (`import "lib/<path>"`,
spec §4.1).

The core modules are at the top of `std/` (with a few of their parts, such as `hash/*`
and `unicode/*`); the rest are grouped in subdirectories. Memory comes from the global
allocator in `mem`; nothing allocates behind your back (a function that allocates returns
`!T`, and a type that owns memory has `deinit`).

## Core

| Module | Contents |
|---|---|
| [`mem`](mem.ax) | The global allocator: `alloc.[T](n)`, `free(s)`, `realloc(s, n)`, `alloc_zeroed`, `dup`, `alloc_aligned.[T](n, align)` (and `_zeroed`, `dup_aligned`), `new.[T]()`, `new_zeroed`, `create(v)` / `destroy(p)`, `in_use()`, `stats()`; under `--sanitize=address`, shadow memory, quarantine and leak reports (`leaks_ignored`) |
| [`str`](str.ax) | Strings as `[]u8`: comparison, search, `trim`, `cut`, `split` / `lines` / `fields` iterators, ASCII case, `join` / `replace`, UTF-8 (`decode_rune`, `encode_rune`, `valid_utf8`, `runes`); `String`, a growable string with a `Writer` |
| [`fmt`](fmt.ax) | Templates: `fmt.print("{} is {:>5.2}\n", [fmt.str(s), fmt.float(x)])`, also `write`, `alloc`, `bufprint`; `parse_int[T]`, `parse_float` (correctly rounded), `format_float` (shortest round trip), `format_int` |
| [`io`](io.ax) | `Writer` / `Reader` interfaces; `stdout`/`stderr`/`stdin`; files (`open`, `create`, `append`, `open_update`; `seek` from a `Whence`, `size`); `BufferWriter`, `SliceReader`, `BufWriter`, `CountingWriter`, `discard`; integer formatting and parsing (`fmt_u64`, `parse_i64`, ...) and `Writer.print_f64` (float parsing is in `fmt`) |
| [`os`](os.ax) | The process: `args`/`arg`/`arg_count`, `getenv`/`has_env`/`setenv`/`unsetenv`/`environ`, `pid`, `cwd`/`chdir`, signals (`signal_catch`/`ignore`/`default`, `signal_poll`, `signal_wait`; not on the node host), `exit` |
| [`c`](c.ax) | C interop helpers: `from_cstr`, `to_cstr`, `cstr_len`, `null_ptr`, `is_null`; the C↔Axiom type table |
| [`log`](log.ax) | Structured logging: levels (`Level`: `DEBUG`, `INFO`, `WARN`, `ERROR`), formats (`Format`: `TEXT`, `JSON`), typed key-value fields, text (logfmt) or JSON lines, `Logger` with its own fields (`with`), a default logger (`log.info(...)`, `set_level`, `level_from_env`), size-rotated log files (`open_file`); whole lines, safe across threads |
| [`hash`](hash.ax) | Hash and equality functions for integers and `[]u8` (`hash_u64`, `hash_bytes`, `eq_bytes`, …), keyed per process against chosen collisions |
| [`hash/xxhash`](hash/xxhash.ax) | XXH32 and XXH64, one-shot and streaming (the checksums of LZ4 and Zstandard) |
| [`hash/siphash`](hash/siphash.ax) | SipHash-2-4 (64- and 128-bit results) and SipHash-1-3 under a 16-byte `Key`: one-shot (`sip24`, `sip13`, `sip24_128`) and streaming (`Sip64`, `Sip128`). Passes the reference implementation's vectors. (`hash.hash_bytes` is already SipHash-1-3 under a random per-process key, so maps of untrusted keys are safe as they are) |
| [`hash/crc`](hash/crc.ax) | CRCs of any width from 1 to 64 bits in the Rocksoft model (`Params`: width, poly, init, refin, refout, xorout), table-driven (`Crc`: `checksum`, `update`, streaming `Digest`); presets CRC-8/SMBUS, CRC-16/ARC, MODBUS, IBM-3740 (CCITT-FALSE), XMODEM, KERMIT, CRC-24/OPENPGP, CRC-32 (ISO-HDLC), CRC-32C, CRC-32/BZIP2, CRC-32/MPEG-2, CRC-64/ECMA-182, CRC-64/XZ. `crc32` and `crc32c` (also used by `Crc` for those presets) run eight bytes at a time, or with the processor's CRC or carry-less multiplication instructions. Check values from the CRC catalogue, longer inputs against zlib and binascii |
| [`hash/murmur3`](hash/murmur3.ax), [`hash/fnv`](hash/fnv.ax) | MurmurHash3 `x86_32` and `x64_128` (checked against the mmh3 package); FNV-1 and FNV-1a in 32 and 64 bits (`fnv1a_64`, ..., with `_update` forms for streaming; the specification's test strings) |
| [`unicode`](unicode.ax) | Unicode 15.1: general categories (`category` → `Category`) and other properties (`east_asian_width` → `EastAsianWidth`, `is_letter`, `is_space`, `combining_class`, bidi and line-break classes, ...), terminal display width of a code point (`width`), simple and full case mapping (`upper`, `lower`, `case_fold`, `equal_fold`; `title_word`, for lib/unicode/segment's `title`), normalization (`normalize` with a `Form`: NFC, NFD, NFKC, NFKD; single steps `decompose_step`, `compose_step`), the segmentation properties (`grapheme_break`, `word_break`, `sentence_break`; the segmenters are lib/unicode/segment's), UTF-16. Passes the UCD's conformance tests. The tables ([`unicode/tables`](unicode/tables.ax)) are generated by [`cmd/internal/tools/gen_unicode.py`](../cmd/internal/tools/gen_unicode.py) |
| [`hash/sha2`](hash/sha2.ax) | SHA-256, SHA-384, SHA-512; a `Hasher` over any of them (and SHA-1), chosen by `Algorithm`. SHA-NI on x86-64, the SHA256 and SHA512 instructions on AArch64, where present |
| [`hash/sha1`](hash/sha1.ax) | SHA-1, for protocols that require it (WebSocket, HMAC-SHA1 in TOTP); not for new signatures. SHA-NI or the AArch64 SHA1 instructions where present |
| [`hash/sha3`](hash/sha3.ax) | SHA3-224/256/384/512 and SHAKE128/256 (Keccak); the AArch64 SHA3 instructions where present |
| [`hash/md5`](hash/md5.ax) | MD5, for formats that use it as a checksum (not for security) |

## Collections

| Module | Contents |
|---|---|
| [`collections/vec`](collections/vec.ax) | `Vec[T]`: growable array (`sort.[LESS]`, `sort_stable.[LESS]`, and `*_dyn`). On slices: `sort.[T, LESS]` (introsort, in place), `sort_stable.[T, LESS]` (merge sort), `sort_with.[T, C, LESS]` / `sort_stable_with.[T, C, LESS]` (a context value passed to each comparison, e.g. sorting indices by keys), `is_sorted.[T, LESS]`, `binary_search.[T, LESS]`, `lower_bound.[T, LESS]`, `upper_bound.[T, LESS]` (and `*_dyn` for runtime comparators), `reverse`, `index_of` |
| [`collections/arena`](collections/arena.ax) | `Arena`: bump allocation freed all at once (`bytes`, `dup`, `copy[T]`), for parsed documents |
| [`collections/map`](collections/map.ax) | `Map[K, V, HASH, EQ]`: open-addressing hash map (and `MapDyn[K, V]`, `Iter[K, V]`) |
| [`collections/set`](collections/set.ax) | `Set[K, HASH, EQ]`: hash set with union, intersection and difference in place (and `SetDyn[K]`) |
| [`collections/treemap`](collections/treemap.ax) | `TreeMap[K, V, LESS]`: ordered map (AVL tree, and `TreeMapDyn[K, V]`): `floor`, `ceil`, `min`, `max`, in-order and range iteration |
| [`collections/deque`](collections/deque.ax) | `Deque[T]`: double-ended queue on a ring buffer |
| [`collections/heap`](collections/heap.ax) | `Heap[T, LESS]`: priority queue (binary heap, and `HeapDyn[T]`), built from a slice in linear time |
| [`collections/lru`](collections/lru.ax) | `Lru[K, V, HASH, EQ]`: least-recently-used cache of a fixed capacity (a `Map` and a doubly linked list in a `Vec`, O(1) operations; also `LruDyn[K, V]`): `get` (marks recent), `put` (gives back the evicted entry), `peek`, `remove`, `pop_lru`, iteration from most to least recent |
| [`collections/bitset`](collections/bitset.ax) | `BitSet`: sets of bits of a given length (`resize`), `next_set`/`next_clear`, union, intersection and difference (`union_with`, `intersect_with`, `difference_with`) |

## Operating system

`os` is the process itself; `std/os/` has the file system and the rest.

| Module | Contents |
|---|---|
| [`os/path`](os/path.ax) | File paths as text: `join`, `clean`, `dir`, `base`, `ext`, `stem`, `with_ext`, `rel`, element iteration; glob matching (`*`, `?`, `[a-z]`, `**`) |
| [`os/fs`](os/fs.ax) | The file system: `read_file`, `write_file`, `append_file`, `copy_file`; `stat`/`lstat` (`Info`: `Kind`, size, modification time, mode), `exists`; `mkdir`, `mkdir_all`, `remove`, `remove_all`, `rename`; `read_dir` (sorted), `walk`, `glob`; `cwd`, `absolute`, `temp_dir`, `make_temp_dir`; `chmod`, `symlink`, `read_link`, `hard_link`, `set_times`, `truncate`, `sync`; advisory locks (`lock`, `try_lock` with a `Lock` kind, `unlock`; not on node); `map` (memory-mapped files, or a copy written back where mapping is impossible); `Watcher` (`Change` events; inotify, or `fs.watch` on node) |
| [`os/process`](os/process.ax) | Child processes: `Command` (arguments, environment changes, directory, each stream inherited, piped or `/dev/null`: `Stdio`), `spawn` → `Child` (`Pipe`s with readers and writers, `wait`, `try_wait`, `kill`), `output` (captured without deadlock), `status`, `run` |
| [`os/cli`](os/cli.ax) | Command-line parsing, GNU style: flags and options (bundled short ones, `--name=value`, unique prefixes, `--`), positionals (`Arity`: optional, required, many), subcommands, typed getters, generated `help`, precise error messages |
| [`os/cpu`](os/cpu.ax) | Processor extensions: `features` (the running processor's, detected once: `cpuid` on x86-64, the system on AArch64), `has` by name, the table of names and bits the compiler shares (`--features`, `@target_has`, `@cpu_has`, spec §13.4) |
| [`os/term`](os/term.ax) | Terminals: `is_tty`, `size`, raw mode, `color_enabled` (`NO_COLOR`, `FORCE_COLOR`, `TERM`), `Style` (attributes; `BasicColor`, 256 and RGB colours), cursor and screen control, `decode_key` (`KeyCode`: arrows, function keys; modifiers, UTF-8) |
| [`os/random`](os/random.ax) | Secure random bytes from the operating system: `fill`, `uint64`, `below` |

## Concurrency

| Module | Contents |
|---|---|
| [`thread`](thread.ax) | `spawn(f, arg)` → `Thread[T]` with `join()` and `is_finished()`; `exit` (of the calling thread); `parallel_for`; `cpu_count`, `sleep_ms` |
| [`sync`](sync.ax) | `Atomic[T: Int]`, `Mutex`, `Condvar` (with timeouts), `RwLock` (writer-preferring), `Semaphore`, `Once`, `Barrier`, `WaitGroup`, `Channel[T]` (bounded MPMC, closable) |
| [`thread/pool`](thread/pool.ax) | A thread pool: `submit(f, arg)` → `Future[A, R]` (`join`, `done`), fire-and-forget `run`, `wait` for all |

## Time

| Module | Contents |
|---|---|
| [`time`](time.ax) | `Duration`, the monotonic `Instant`, wall-clock `Time`, calendar fields (`DateTime`, `Weekday`), RFC 3339 and HTTP dates, `sleep` |
| [`time/tz`](time/tz.ax) | Time zones: TZif files (RFC 8536, v1–v4) from the system's database (`Zone.load`, safe names) or bytes, POSIX TZ rules, `Zone.local` (`$TZ`, `/etc/localtime`); `lookup` (offset, DST, abbreviation), `to_local`, and `from_local` with PEP 495 folds for repeated and skipped times. Checked against Python's `zoneinfo` ([`time/tz/testdata`](time/tz/testdata)) |
| [`time/cron`](time/cron.ax) | Cron expressions (Vixie cron's syntax): five fields or six with seconds, lists, ranges, steps, month and day names, `@daily` and the other macros, `L` and `5L`, day-of-month/day-of-week OR rule; `Schedule.next_utc` / `prev_utc` in UTC and `next_in` / `prev_in` a `tz.Zone` (skipped local times fire at the change, repeated ones once unless hourly); hand-checked cases incl. leap years and DST changes |

## Networking

| Module | Contents |
|---|---|
| [`net`](net.ax) | Sockets: TCP (`connect`, `listen` by name; `connect_addr`, `listen_addr`; local and peer addresses), UDP (`UdpSocket`), Unix sockets; `Addr` (IPv4 and IPv6 by `Family`, parsing and RFC 5952 formatting), `resolve`; `Poller` (readiness of many sockets, for event loops); timeouts |

## Encodings

| Module | Contents |
|---|---|
| [`encoding/json`](encoding/json.ax) | `parse` → `Document` (a `Value` tree: `get`, `at`, `as_str`, `as_i64`, …); `Writer` (streaming, compact or pretty); `stringify`/`stringify_pretty`, or `stringify_to`/`stringify_pretty_to` an `io.Writer` |
| [`encoding/binary`](encoding/binary.ax) | Fixed-width integers and floats in either byte order (`u32_le`, `put_u64_be`, `get_u16(b, Endian)`, ...), LEB128 varints and their zigzag form (`uvarint`, `put_uvarint`, `svarint`, over an `io.Reader`/`Writer` too); `Cursor`, checked reads from a slice (`read_u32`, `read_uvarint`, `read_bytes`; `Truncated` past the end), and `Encoder`, values to an `io.Writer`. Varints checked against the protobuf documentation |

## Mathematics

| Module | Contents |
|---|---|
| [`math`](math.ax) | Elementary functions on `f64` (and `f32` forms of `exp`, `log`, `log2`, `pow`, `cbrt`, `hypot`, `sin`, `cos`, `tan`, `atan`, `atan2` and `tanh`, suffixed `f`): `exp`, `exp2`, `expm1`, `log`, `log1p`, `log2`, `log10`, `pow`, `cbrt`, `hypot`, `sin`, `cos`, `tan`, `sincos`, `asin`, `acos`, `atan`, `atan2`, `sinh`, `cosh`, `tanh`, `ldexp`, `frexp`; the special functions `erf`, `erfc`, `gamma`, `lgamma`; constants. Within 1 ulp (2 for `tan`, `expm1`, `sinh`, `tanh`; 2–8 for the special functions), trigonometry of huge arguments included |
| [`math/complex`](math/complex.ax) | `Complex` numbers on `f64`: arithmetic (Smith's division), `abs`, `arg`, `polar`, `exp`, `log`, `sqrt`, `pow`, trigonometric and hyperbolic functions and their inverses on the principal branches of C99 (Kahan's formulas, signed zeros respected); `format`. Checked against Python's `cmath` |
| [`math/stats`](math/stats.ax) | Statistics on `[]f64`: compensated `sum`, `mean`, sample and population variance and deviation, `median`, `quantile` (R/NumPy's default), geometric and harmonic means, `covariance`, Pearson and Spearman correlation, `ranks`, `linear_regression`, `histogram`; one-pass `Running` statistics (Welford, mergeable); the normal distribution (`normal_pdf`, `normal_cdf`, `normal_sf`, `normal_quantile`) |
| [`math/rand`](math/rand.ax) | Seedable pseudo-random numbers (xoshiro256\*\*): integers in ranges without bias, floats, normal, exponential, Poisson and weighted choices, `shuffle`, `choose`, `jump` for parallel streams; not for secrets (see `crypto/random`) |

## Conventions

- **Interfaces are structs of function pointers.** `Writer` and `Reader`
  each hold a context pointer plus the functions that operate on it. This is how the
  language expresses polymorphism without traits or vtables.
- **Maps take their hash and equality functions as type arguments.** For example:
  `Map.[u64, V, hash.hash_u64, hash.eq_u64].init()` (or
  `MapDyn.[u64, V].init(hash_fn, eq_fn)` when chosen at run time).
- **Errors, not sentinels.** Lookups that can fail return `!T` (for example
  `error.NotFound`, `error.Empty`). The idiom for "value or default" is
  `m.get(k) catch 0`.
- **One name for one thing.** Error names are program-wide, so modules share them:
  input a binary format or archive cannot parse is `Corrupt`, a text format's is
  `SyntaxError`, a network protocol's is `DecodeError`; a more specific name
  (`ChecksumMismatch`, `Unsupported`, `TooLarge`) only where a program would act
  differently. Permission problems are `PermissionDenied`. The end of a sequence is
  `Done` from an iterator's `next`, `Closed` from a connection or channel, and
  `EndOfStream` from a reader. A call the caller made wrongly (a bad argument, or an
  object not in the state the call needs) is `InvalidArgument`; a fixed table that is
  full (windows, voices, descriptor slots) is `NoSpaceLeft`; a graphics or audio
  device that was removed or reset is `DeviceLost`. A generic `Failed` is a last
  resort, for a system call that failed without saying why: name the cause wherever
  the system reports one (`com.check` classifies an HRESULT this way).
- **Error sets.** A module that can fail declares one named error set (spec §11.3),
  named for the module (`FsError`, `JsonError`, `PngError`; the allocator's is
  `mem.AllocError`), holding its own errors and those it can pass on, and its functions
  return `FsError!T`, so callers can `match` every case. A function that can pass on an
  error of no set (from an `io.Reader` or `io.Writer` or a callback whose type returns
  `!T`, or a function that itself returns `!T`) returns `!T`. A `Set!T` widens to `!T`
  wherever one is expected, so callers that do not care which error keep working.
- **Ownership is explicit.** A type that owns memory has `deinit`; new code does not
  add `release` or `destroy` for the same thing. A function whose result the caller
  frees says so in its doc comment ("free with mem.free"); an `_alloc` suffix marks the
  allocating form only where a non-allocating one exists beside it. Options structs are
  passed by value, named `opts`.
- **Threads share memory; synchronize it.** `spawn` copies its argument, so pass a
  pointer to shared data and protect it with `std/sync`, or use `parallel_for`, which
  hands each thread a disjoint range. The allocator is thread-safe; `Vec` and `Map`
  are not.
- **Byte order comes from `std/encoding/binary`.** A module that reads or writes
  little- or big-endian integers uses `binary.u32_le(b[i..])`, `binary.put_u16_be(...)`
  and the rest (inline functions, so they cost nothing) rather than its own helpers;
  only the runtime, which `binary` sits above, keeps its own.
- **Implementation modules live under `internal/`.** A module's parts that are not
  for programs (`graphics/internal/shape/`, `audio/internal/opus/`,
  `audio/internal/aac/`, `graphics/image/internal/av1/`, `text/internal/html/`, …) sit
  in an `internal/` directory, which only modules in the tree above it may import
  (spec §4.1). What a program may import is in the tables above.
- **Adapters point at their source.** `BufferWriter.writer()`
  and similar return values that point at the object they came from, so don't move
  that object while the adapter is in use.

## What the test suite covers

Every module has `test` blocks that the suite runs on WebAssembly and natively, under the
address sanitizer and with integer overflow trapping (`--overflow=trap`). The suite also
checks that the core imports only itself and that the compiler imports only the core,
that every public item has a `///` comment, and that every public `next` is an
iterator's (spec §15.3).

## Example

```axiom
import "std/collections/map" as map;
import "std/hash" as hash;
import "std/io" as io;

export fn main() i32 {
    let mut ages = map.Map.[[]u8, u32, hash.hash_bytes, hash.eq_bytes].init();
    defer ages.deinit();
    ages.put("ada", 36) catch return 1;
    io.stdout().print_u64(@as(u64, ages.get("ada") catch 0)) catch return 1;
    return 0;
}
```

See also [`examples/wordcount/wordcount.ax`](../examples/wordcount/wordcount.ax) and
[`examples/http_server/http_server.ax`](../examples/http_server/http_server.ax), a JSON API over HTTP.

Iterators (`str.split`, `regex.find_all`, `Map.iter`, …) have one protocol (spec
§15.3): `next(self: *mut Self) ?T`, or `E!?T` when a step can fail, and the end is
always `.none`. A `while let` loop drives one; the collections give entries of
pointers into themselves (`e.key: *K`, `e.value: *mut V`):

```axiom
let mut it = str.split(line, ",");
while let .some(field) = it.next() {
    ...
}
let mut es = ages.iter();
while let .some(e) = es.next() {
    e.value.* += 1;
}
let mut r = tar.Reader.init(bytes);
while let .some(entry) = r.next()! {
    ...
}
```

A cursor read through its fields, such as the HTML and Markdown tree walkers, moves
with `advance()` instead.

## Host ABI

Modules that use the operating system (`io`, `os` and its parts, `net`, `thread`,
`time`, `log`, `os/random`, and in `lib/` windows, audio and the GPU) call host functions imported
from module `"axiom"`, which `wasm32-freestanding` does not have. The core of that ABI
is below; the runtime files define the rest (more file operations, processes and pipes,
signals, terminals, UDP, Unix-domain sockets, polling, name resolution, file watching):

| Function | Meaning |
|---|---|
| `ax_write(fd, p, n) isize`, `ax_read(fd, p, n) isize` | read and write file descriptors; `0` from read means end of file |
| `ax_open(path, len, flags) i32`, `ax_close(fd)` | `flags`: `0` read, `1` create/truncate, `2` append, `3` read and write, `4` read and write, created or truncated, `5` read and append, created; plus `8` with `1` or `4`: the file must not exist (`-17`); `16`: a file created is its owner's alone (permissions 0600; on Windows an access control list of the owner's); `32`: exclusive access (on Windows no other handle may open it meanwhile; elsewhere no effect) |
| `ax_seek(fd, off, whence) i64` | moves a file's position (`whence`: `0` start, `1` current, `2` end); returns the new position |
| `ax_stat(path, len, follow, out) i32` | `out[0..4]` (`i64`): kind (`1` file, `2` directory, `3` symbolic link, `4` other), size, modification time in ns, permission bits; `follow = 0` does not follow a final symbolic link |
| `ax_fstat(fd, out) i32` | `ax_stat` for an open file |
| `ax_create(path, len, mode) i32` | opens a file for writing, created or truncated; a new file gets the permission bits `mode` (less the umask) |
| `ax_mkdir(path, len, mode) i32`, `ax_remove(path, len, dir) i32`, `ax_rename(from, flen, to, tlen) i32` | `mode`: permission bits for the new directory; `dir = 1` removes an (empty) directory |
| `ax_readdir(path, len, buf, cap) isize` | the entries (not `.`, `..`) as a kind byte (`0` unknown), the name and a `0` byte each; returns the bytes needed, which may exceed `cap` |
| `ax_dir_open(dir, name, len) i32`, `ax_dir_read(dir, buf, cap) isize`, `ax_dir_remove(dir, name, len, isdir) i32`, `ax_dir_close(dir)`, `ax_dir_stat(dir, out) i32` | directory handles (`dir = -1`: the working directory) for walking trees: opening never follows a symbolic link (`-40`) and fails with `-20` on a non-directory; the entries are as `ax_readdir`'s; `ax_dir_stat` gives the identity (device and inode) of an open directory, so that a tree walk can close an ancestor and check it when reopened through `..`. The JavaScript host keeps paths, so it cannot stop a link swapped in meanwhile |
| `ax_getcwd(buf, cap) isize` | the working directory; returns its length |
| `ax_argc() i32`, `ax_arg(i, buf, cap) isize` | the program's arguments (argument 0 is its name); returns the length |
| `ax_getenv(name, len, buf, cap) isize` | an environment variable; `-2` if unset |
| `ax_exit(code)` | ends the process, from any thread (spec §12.8.5) |
| `ax_os() i32` | the operating system: `1` Linux, `2` macOS, `3` Windows, `0` another (or none) |
| `ax_tcp_connect(name, len, port) i32`, `ax_tcp_listen(name, len, port) i32` | return a socket handle |
| `ax_tcp_accept(h) i32`, `ax_tcp_local_port(h) i32` | |
| `ax_sock_read(h, p, n) isize`, `ax_sock_write(h, p, n) isize`, `ax_sock_close(h)` | |
| `ax_sock_shutdown(h) i32` | sends the end of the stream; reading goes on |
| `ax_sock_timeout(h, ms) i32` | later reads and writes on `h` fail with `-5` after `ms` milliseconds (`0`: never) |
| `ax_thread_spawn(entry, arg, stack_top, exited) i32`, `ax_thread_stack_size() usize`, `ax_thread_exit()` | threads (spec §12.8.5) |
| `ax_cpu_count() u32`, `ax_sleep_ns(ns)` | |
| `ax_random(p, n) i32` | fills `p[0..n]` with cryptographically secure random bytes |
| `ax_clock_ns(clock) i64` | nanoseconds on clock `0` (real time since 1970), `1` (monotonic), `2` (the processor time of the process) or `3` (the processor time of the calling thread; on WebAssembly the process's where node has no per-thread time) |
| `ax_tz_local(buf, cap) isize` | the host's local time zone as the bytes of a TZif file (RFC 8536, version 2 or later, with a footer rule for later times); `-2` if the host has none (native targets, whose zone is the system's files). `std/time/tz`'s `Zone.local` asks for it when `$TZ` and `/etc/localtime` give no zone: the node host builds one from what the JavaScript engine knows of its zone (`Date`, `Intl`), so that no time zone database is needed |

A negative result means failure. The codes are `-1` general failure, `-2` connection
refused / not found, `-3` address in use, `-4` invalid address, `-5` timed out, `-13`
access denied, `-11` would block, `-17` already exists, `-20` not a directory, `-21` is a
directory, `-39` directory not empty, `-40` a symbolic link where none may be, and `-95`
not supported. Functions that fill a buffer return the length the whole result
needs; when that is more than `cap`, the buffer was not written and the caller retries
with more room.

The implementations:

- **[`std/internal/runtime/host.mjs`](internal/runtime/host.mjs)** for node, usable as a runner:
  `node std/internal/runtime/host.mjs prog.wasm`. Node sockets are asynchronous, so they live in
  a worker thread, and the wasm thread blocks on `Atomics.wait` until each
  operation completes. The program's files are node's; with `--root=dir`, node's under
  `dir`, which the program sees as `/` (its `tmp` the program's `/tmp`; a path that
  leads out of it, through `..` or a symbolic link, does not); with `--files=memory`, a
  file system in memory ([`memfs.mjs`](internal/runtime/memfs.mjs), on the same
  worker, so that every thread sees the same files), which a host without files of
  its own (a browser) can use. A threaded program runs all its threads on workers,
  under a supervisor on node's main thread (spec §12.8.5, "Ending the program").
- **[`std/internal/runtime/native.ax`](internal/runtime/native.ax)** for Linux: the host ABI
  written in Axiom, which the compiler builds into every Linux program. It makes
  Linux system calls directly ([`std/internal/runtime/linux.ax`](internal/runtime/linux.ax)) and needs no
  C library, so those programs are static executables (unless they link C code).
- **[`std/internal/runtime/darwin.ax`](internal/runtime/darwin.ax)** for macOS, through libSystem, and
  **[`std/internal/runtime/windows.ax`](internal/runtime/windows.ax)** for Windows, through the Win32
  API, take native.ax's place for those targets.
