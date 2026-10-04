# 0037: Stack traces cost 15% in call-heavy native code

- Type: feature (performance)
- Status: open
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
