## Compiler

- On x86-64 with VNNI, `acc + @dot(x, y)` of `u8` by `i8` lanes is one `vpdpbusd` that
  accumulates into `acc`, instead of a `vpdpbusd` into a zeroed register and a `vpaddd`
  (as `sdot` already does on AArch64), also when `acc` comes from memory.
