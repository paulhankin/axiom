# 0037: Stack traces cost 15% in call-heavy native code

- Type: feature (performance)
- Status: fixed (bf2e70d)
- Area: compiler (C), runtime

## Today

Every call records its call site on the shadow stack (§16.4.1): a load of
the ring index from thread-local storage, a store of the site pointer and a
store of the incremented index, and a restore after the call. With issue
0032, `inline` calls no longer pay for this. Recursive and other genuinely
out-of-line hot calls still do, on every target.

Measured on the benchmark's benchmarks (the benchmark `axiom/BENCHMARKS.md`, native
AVX-512, ns per position, best of 5):

| Kernel | traces on | `--stack-traces=off` | Rust |
|---|---|---|---|
| movegen (recursive `generate_moves_sub`) | 739 | 639 | 571 |
| classify | 5.6 | 4.5 | 4.6 |
| contact_net | 241 | 227 | 202 |

Traces are on by default and are part of what makes Axiom pleasant to debug,
so turning them off is not the answer for most programs.

## Proposal

Natively, get the same traces with nothing on the hot path:

- Keep frame pointers (`frame-pointer=all`, cheap on x86-64 and AArch64),
  and emit a table mapping each call's return address to its site record
  (the same records the shadow stack stores today), sorted, in a section
  of the executable.
- At a trap (and in `axiom profile`'s sampler, which can read the frame
  chain from the signal context), walk the frame-pointer chain and look up
  each return address. Inline calls (issue 0032) are recovered from their
  hidden site arguments or from a second table of inlined ranges.
- Keep the shadow stack on wasm, where the stack cannot be walked.

If the full change is too big, cheaper steps that help:
- Don't record a site for calls from a function that has no trap sites and
  no further calls, or hoist the record out of loops when the site is the
  same each iteration.
- Measure the cost of the current sequence and make sure it is one TLS
  base computation per function, not per call.

## Resolution

The proposal's design, for the C compiler on x86-64 Linux (spec §16.4.1, "Frame tables"; Appendix C, C42). Calls no longer touch the shadow stack:

- Every function keeps a frame pointer (`frame-pointer=all`; the runtime's C sources are compiled with `-fno-omit-frame-pointer`).
- Each call that the shadow stack recorded gets a `DILocation` in a lexical block of its own, and is `nomerge` (so are trap calls), so that optimization keeps one call per site and inline context. After optimization, the `inlinedAt` chain of each remaining call's location names the calls inlined around it: calls to `inline` functions and whatever LLVM inlined. That is the "second table of inlined ranges", without a second table: every call is followed by an inline-assembly label and a 16-byte entry in the section `axiom_rets`, `{label, chain}` (relative), where the chain is the call's own site record (none for a trap) and then those of the inline calls that led to it. The records are the same ones the shadow stack stores. The debug information is stripped afterwards unless `-g` asked for it. `inline` functions take no hidden site arguments in this mode (issue 0032's scheme stays for the shadow stack).
- `axiom.trap` calls `ax_stack_walk` (`runtime/native_host.c`) with its own frame, and the crash handlers (`native_host.c`, `test_main.c`) call it with the signal context's. It makes the table absolute and heap-sorts it on first use (no allocation, so it is safe in the signal handler), walks the frame-pointer chain within the thread's stack, looks each return address up as the first label at or after it (at most 64 bytes on: copies of the call's result may come between), and fills the thread's shadow-stack record (calls and depth) exactly as the stores would have. Every report, `ax_on_trap`'s third argument and the test runner's frames are therefore produced by unchanged code, with the same frames, spans and inline frames: `tests/traces` passes natively at `-O0` to `-O3`, with `-g`, and with `--units=module`. Every call has an entry (with an empty chain if it records nothing), so a return address never finds a later call's label.
- `axiom profile` builds and `--sanitize=address` builds keep the shadow stack: the sampler and the allocator read it while the program runs, and keeping it there costs nothing that matters (profile builds already differ, with volatile stores). So does WebAssembly, whose stack cannot be walked, and every other target. Profiles are therefore exactly as before.

Measured (native, `-O2`):

| | shadow stack | frame tables | `--stack-traces=off` |
|---|---|---|---|
| recursive fib(40) | 1152 ms | 407 ms | 462 ms |
| fib(40), `-O0` | 1144 ms | 625 ms | 665 ms |
| the benchmark movegen | 795 ns | 645 ns | 697 ns |
| the benchmark classify | 5.1 ns | 4.6 ns | 5.0 ns |

(`bench_kernels` best of 7 per kernel, built by the old compiler (`/home/user/axiom`) and by this one; the machine was loaded (load average about 4 on 4 cores), so differences of a few percent are noise; that the frame tables come out ahead of `--stack-traces=off` is layout luck. Checksums are unchanged.) Compile time does not go up (the self-hosted compiler at `-O2`: 42 s -> 33 s, at `-O0` 5.4 s -> 5.0 s, on the same loaded machine); executables grow by 10-17% (the table, the chains and the frame pointers).

Not identical to the shadow stack: a fault that is not a check (a raw-pointer access, say) inside a function that LLVM inlined is reported in the function it was inlined into, since only call sites are in the table; checks, stack overflow in recursion and faults in functions that were not inlined give the same trace as before. Frames outside a callback from C code compiled without frame pointers can be missing (C passed with `--link=file.c` is compiled with them).
