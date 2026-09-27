# Phalange standard library

Import with `import "std/<module>" as <name>;`. Everything that allocates takes a
`mem.Allocator`, and nothing allocates behind your back.

| Module | Contents |
|---|---|
| [`mem`](mem.ph) | `Allocator` (generic `alloc.[T](n)` / `free(s)`), `page_allocator`, `Arena`, `FixedBuffer` |
| [`vec`](vec.ph) | `Vec[T]`: growable array. `sort`, `reverse`, `index_of`, `binary_search` on slices |
| [`hash`](hash.ph) | Hash and equality functions for integers and `[]u8` (`hash_u64`, `hash_bytes`, `eq_bytes`, …) |
| [`map`](map.ph) | `Map[K, V]`: open-addressing hash map, and `Iter[K, V]` |
| [`matrix`](matrix.ph) | `Matrix[T: Num]` (heap, row-major); `Mat4` (4×4 `f32` on `f32x4` columns); SIMD `dot`, `axpy` |
| [`io`](io.ph) | `Writer` / `Reader` interfaces; `stdout`/`stderr`/`stdin`; files; `BufferWriter`, `SliceReader`, `BufWriter`; integer/float formatting and parsing |
| [`thread`](thread.ph) | `spawn(alloc, f, arg)` → `Thread[T]` with `join()`; `parallel_for`; `cpu_count`, `sleep_ms` |
| [`sync`](sync.ph) | `Atomic[T: Int]`, `Mutex`, `WaitGroup`, `Channel[T]` (bounded MPMC, closable) |
| [`c`](c.ph) | C interop helpers: `from_cstr`, `to_cstr`, `cstr_len`, `null_ptr`, `is_null`; the C↔Phalange type table |
| [`net`](net.ph) | TCP: `connect`, `listen` → `TcpStream` / `TcpListener` (with `io.Reader`/`io.Writer` adapters); IPv4 parsing/formatting |

Each module carries its own tests: `phalange test std/<module>.ph`.

## Conventions

- **Interfaces are structs of function pointers.** `Allocator`, `Writer` and `Reader`
  each hold a context pointer plus the functions that operate on it. This is how the
  language expresses polymorphism without traits or vtables.
- **Maps take their hash and equality functions at construction.** For example:
  `Map.[u64, V].init(al, hash.hash_u64, hash.eq_u64)`.
- **Errors, not sentinels.** Lookups that can fail return `!T` (for example
  `error.NotFound`, `error.Empty`). The idiom for "value or default" is
  `m.get(k) catch 0`.
- **Threads share memory; synchronize it.** `spawn` copies its argument, so pass a
  pointer to shared data and protect it with `std/sync`, or use `parallel_for`, which
  hands each thread a disjoint range. `page_allocator` is thread-safe. `Arena`,
  `Vec` and `Map` are not.
- **Adapters point at their source.** `Arena.allocator()`, `BufferWriter.writer()`
  and similar return values that point at the object they came from, so don't move
  that object while the adapter is in use.

## Example

```phalange
import "std/mem" as mem;
import "std/io" as io;
import "std/map" as map;
import "std/hash" as hash;

export fn main() i32 {
    let mut arena = mem.Arena.init(mem.page_allocator);
    defer arena.deinit();
    let mut ages = map.Map.[[]u8, u32].init(arena.allocator(), hash.hash_bytes, hash.eq_bytes);
    ages.put("ada", 36) catch return 1;
    io.stdout().print_u64(@as(u64, ages.get("ada") catch 0)) catch return 1;
    return 0;
}
```

See also [`examples/wordcount.ph`](../examples/wordcount.ph).

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
| `ph_thread_spawn(entry, arg, stack_top) i32`, `ph_thread_stack_size() usize` | threads (spec §12.8.5) |
| `ph_cpu_count() u32`, `ph_sleep_ns(ns)` | |

A negative result means failure. The codes are `-1` general failure, `-2` connection
refused / not found, `-3` address in use, `-4` invalid address, and `-13` access denied.

Two implementations are provided:

- **[`runtime/host.mjs`](../runtime/host.mjs)** for node, usable as a runner:
  `node runtime/host.mjs prog.wasm`. Node sockets are asynchronous, so they live in
  a worker thread, and the wasm thread blocks on `Atomics.wait` until each
  operation completes.
- **[`runtime/native_host.c`](../runtime/native_host.c)** for POSIX. The compiler
  links it automatically when a native program uses the host ABI.
