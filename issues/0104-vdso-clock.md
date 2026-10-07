# 0104: Native Linux programs read the clock with a system call, not the vDSO

- Type: feature (performance)
- Status: fixed (651c7cc)
- Area: runtime (linux.ax)

Programs built by the self-hosted compiler for Linux link no C library: `runtime/linux.ax`
makes every system call itself. `clock_gettime` (behind `std/time`'s `Instant.now` and
`now`) was the `clock_gettime` system call, where the C library calls the kernel's vDSO,
code the kernel maps into every process that reads the clock from shared memory without
entering the kernel. On this machine's Xeon (a VM, TSC clock source) a system call takes
131 ns and the vDSO 20 ns.

the benchmark's `bench_search` times each position between two `Instant.now()` calls, as Rust's
`bench_matrix` does, so each position's time included about one system call more than
Rust's and the LLVM-built port's (both through the C library). At 0-ply a position takes
about 1.3 µs: the self-hosted build measured 1.04x Rust's time, 0.91x with the vDSO (min of
21 interleaved runs, 2000 positions; LLVM-built 0.85-0.89x). `strace -c` showed 400
`clock_gettime` calls for 200 positions, now none. Every program that reads the clock often
(profilers, timeouts, rate limits) pays the same.

## Resolution

`clock_gettime` in `runtime/linux.ax` looks up the vDSO's `__vdso_clock_gettime`
(`__kernel_clock_gettime` on AArch64) once, through the ELF image at the auxiliary
vector's `AT_SYSINFO_EHDR`: its first `PT_LOAD` (for the load bias) and `PT_DYNAMIC`
(`DT_SYMTAB`, `DT_STRTAB`, and the symbol count from `DT_HASH`, else from `DT_GNU_HASH`'s
chains), and calls it through a function pointer (the C calling convention). With no vDSO, no
such symbol, or a non-zero result from it (a clock it cannot read), it makes the system call
as before. `std/time`'s new test "clocks read often" reads the monotonic clock 100000
times (3 ms natively, against 13 ms with system calls) and checks it never goes backwards
and agrees with a sleep and with the real-time clock. AArch64 was not run (no qemu here).
