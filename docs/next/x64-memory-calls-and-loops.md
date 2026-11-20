## Compiler

- C programs' calls of the library's `memcpy`, `memmove` and `memset` are the
  compiler's own memory operations (as `__builtin_memcpy` already was): a constant size
  is moved in registers (`memcpy(&w, p, 8)` is a load), and on x86-64 the others are
  copied inline. Before, each was a call of the library's C loop, which went a byte at
  a time when the two addresses were aligned differently. `memcmp` of 1, 2, 4 or 8
  bytes is two loads and a compare (natively), and the library's `memcmp` compares
  words at any alignment (8 times faster on 1000 bytes aligned differently). A
  memcpy/memset-heavy benchmark runs in a fifth of the time.
  A program that defines these functions itself in another unit keeps its calls of
  them with `--c-no-builtin` (`axiom cc -fno-builtin`).
- On x86-64, copies and fills of a size known only at run time of 33 to 256 bytes are
  loops of vectors (up to 2.7 times faster than the `rep movsb`/`stosb` they were),
  and a `memmove` whose destination overlaps the end of its source copies backwards
  in vectors: 15 times faster at 100 bytes than the byte-at-a-time backward string
  copy.
- x86-64 code has fewer jumps in loops: a short loop latch (`i++; if i < n goto top`)
  is copied into the jumps to it, so a switch's cases in a loop branch back to its
  head themselves, and jumps through empty blocks go straight to their end; `pc + 1`
  computed before `pc`'s last read no longer costs a copy on every way back. On every
  target, a jump table over a value whose possible values its cases cover (a state
  machine's state) has no range check. A CSV state machine runs 12-15% faster, a register-based
  virtual machine 5-8%, a bytecode interpreter 2%.
- A struct with padding, built field by field and copied as words, no longer reads a
  word over a narrower store just made (a store-forwarding stall): 25% faster in a
  loop building and storing such structs.
