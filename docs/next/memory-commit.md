## Standard library

- On Linux and macOS, the memory the allocator draws on is no longer one read-write
  mapping of 1 TiB made at the first allocation: the 1 TiB of address space is reserved
  with no access, and `@memory_grow` makes pages read-write as the heap grows (as on
  Windows). A program's data size (`VmData`) is now what it uses, so it runs under a
  data size limit (`ulimit -d`, where it used to fail to allocate at all) and under
  strict overcommit (`vm.overcommit_memory=2`, where the mapping took the system's
  whole commit budget), and an allocation the system cannot commit fails with
  `OutOfMemory`.
