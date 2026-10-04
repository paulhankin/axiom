# 0067: Self-hosted optimizer: booleans materialised as 0/1 and tested again

- Type: feature (performance)
- Status: fixed (b5484e8, c35afe7, c77857d)
- Area: compiler (selfhost: combine, thread, opt, x64, mach)

Follow-up to 0057 and 0066. After 0066, the benchmark's movegen kernel is about 695 ns per
position with the self-hosted compiler against 600-670 with the C compiler (LLVM). In
`MoveGenerator.generate_moves_sub` (`src/core/moves.ax`) the selfhost executes 721k
instructions per 100 `generate_moves_into` calls against LLVM's 539k (deterministic
counts, ptrace single-stepping). Most of the difference is not register allocation but
how the loop's booleans are kept:

```
let mut legal = false;
let mut off = false;
if i >= r { dest = i - r; legal = b.raw[OPP][23 - dest] < 2; }
else { off = true; legal = can_bear_off and ((i == n_back) or (i + 1 == r)); }
if legal { ... if not off { hit = ...; } ... call ... if not off { ... if hit { ... } } used = true; }
```

- **Boolean locals become `i8` phis.** A `bool` local lives in a byte slot, so after
  mem2reg `legal`, `off`, `hit` and `used` are phis of `i8` (`zext` of each comparison,
  constants 0 and 1), tested with `icmp ne 0`. In code: `setb r13b; movzx r13d,r13b; ...
  test r13b,r13b; je`. The `i1` would do.
- **A branch on a phi of conditions is not threaded.** The join after `if i >= r` only
  merges `legal` and branches on it. LLVM jumps on each arm's own comparison
  (`cmp cl,0x2; jae continue`, `test r13d,0xffffc0; jne continue`) and never forms
  `legal`. Jump threading (`thread.ax`) only threads edges where the condition is a
  constant.
- **A phi of constants that a dominating branch decides is not that branch's
  condition.** `off` is `false` on every path through `i >= r` and `true` on every path
  through the other arm, so `not off` is `i >= r`. LLVM tests `cmp r12,r11` again after the
  recursive call; the selfhost keeps the 0/1 `off` (and `not off`) in registers and on the
  stack across it, and threading cannot use what the earlier branch decided.
- **Comparisons used in other blocks are materialised** (`setcc; movzx`, kept in a
  register or spilled) and tested there, where LLVM compares again in the using block
  (`can_bear_off`, `doubles`).
- **`@any`/`@all` feeding a branch** (`save_moves`' 16-byte compare of a board against the
  hash table entry) are `vpcmpeqb; vpmovmskb; cmp 0xffff; setne; movzx; test; je`;
  LLVM: `vptest; jne`.
- **`setcc` then `movzx`** where LLVM zeroes the register before the compare
  (`xor eax,eax; cmp; sete al`).

## Proposal

- combine: phis of `i8` whose inputs are all `zext` of `i1` or 0/1 become `i1` phis.
- gvn/combine: an `i1` phi whose inputs are constants that the immediate dominator's
  branch decides (each predecessor reached only through one of its edges) is that
  condition, or its negation.
- thread: a block that only merges values and branches on one of its phis is copied into
  each predecessor whose input is a condition computed there (tail duplication), so the
  predecessor branches on it directly; a condition decided by a dominating branch on the
  same value is known on the edge.
- x64: comparisons used by branches and selects in other blocks are compared again there
  when their operands are available; `@any`/`@all`/`@bitmask != 0` feeding a branch as
  `vptest`/`kortest` and `jcc`.

## Fixed (b5484e8, c35afe7, c77857d)

- **Boolean phis** (`combine.ax` `narrow_bool_phis`): a web of `i8` phis whose inputs are
  `zext` of `i1` values or 0/1 becomes `i1` phis (other uses read a `zext`); `!= 0` of them
  folds away.
- **Phis of decided constants** (`thread.ax` `phi_conditions`): an `i1` phi of constants at
  M whose every input comes along one edge of the branch of M's immediate dominator, true
  on one edge and false on the other, is that branch's condition or its negation. `off` is
  `i < r` again.
- **Branches on phis of conditions** (`thread.ax` `phi_branch`): a small block that
  branches on its own phi, entered from a block that computes the phi's input and only goes
  there, is copied into it ending in the branch (tail duplication). Each arm of
  `if i >= r` now jumps on its own comparison.
- **Known conditions**: a branch on a value that a branch above (on the same value, through
  blocks with one predecessor) decided is threaded, also when the value is computed in
  another block.
- **SSA repair in threading**: a value of the threaded block used beyond it gets its value
  or the copy's, whichever reaches the use, and phis where both do. Threading used to give
  up on any use outside the successor the edge goes to.
- **Comparisons in other blocks** (`opt.ax` `sink_compares`, native backends): a branch or
  select on a comparison computed elsewhere (operands constants, parameters or used
  elsewhere too) compares again where it is: `test r8d,0xffffc0` for `can_bear_off` in the
  loop, `cmp r12,rcx` for `not off` after the recursive call.
- **x64**: `@any_true`/`@all_true` of 128- and 256-bit vectors as `ptest` (fused into the
  branch or select: `vptest xmm0,xmm0; je` in `save_moves`); `setcc` into a register zeroed
  before the compare (`xor; cmp; sete`, no `movzx`); `x & ~y` as `andn`.
- **Coalescing** (`mach.ax`): a copy whose source is defined in several places (a phi) is
  coalesced when no definition of the source lies where the destination lives (only
  single-definition sources were). A constant that is rematerialized is not joined to a
  register defined again (a two-address destination): in `dim128_pair` the sigmoid's
  tables (`vpermt2ps` destinations) lost their rematerialization that way and kept 15
  registers through the output layer, whose activations were spilled and reloaded: the
  contact_net regression of 0066.

Deterministic counts (as 0066), main (95c7657) against after:

| | instructions | stack loads | stack stores |
|---|---|---|---|
| `generate_moves_sub` (100 calls) | 721k -> 619k (LLVM 539k) | 75.2k -> 65.9k (46.2k) | 23.5k -> 20.3k (24.0k) |
| movegen in all | 1168k -> 1059k | 87.3k -> 77.9k | 36.8k -> 33.6k |
| contact_inputs (300 calls) | 1077k -> 1067k | 4.9k -> 4.9k | 9.8k -> 10.4k |
| contact_net (16 batches) | 1081k -> 1060k | 8.1k -> 3.6k | 8.6k -> 5.5k |
| pruned_single (400 evals) | 303.1k -> 303.1k | 3.6k -> 3.6k | 6.0k -> 6.0k |

In `generate_moves_sub` per 100 calls: `setcc` 21.8k -> 5.6k (LLVM 4.5k), `movzx` of a byte
register 21.8k -> 0, register copies 117k -> 101k (LLVM 75k).

Timings (`bench_kernels`, ns per position, min of 11 and 15 interleaved runs; the machine
was shared, so about ±5%):

| kernel | main | after | LLVM | Rust |
|---|---|---|---|---|
| movegen | 742-760 | 643-646 | 586-618 | 653 |
| contact_inputs | 183-194 | 176-179 | 148-154 | 155 |
| contact_net | 246-250 | 245-248 | 229-243 | 228 |
| race_net | 181-184 | 166-177 | 164-172 | 163 |
| contact_single | 298-304 | 263-288 | 261-280 | 253 |
| pruned_single | 129-132 | 116-129 | 107-110 | 105 |

`bench_search --mode 2ply --limit 50` gives identical results. Tests:
`tests/run/booleans.ax`, `tests/lowering/x64_booleans.ax` (ptest), `x64_scalar.ax` (andn).

Left (0070): what remains of movegen's gap is register copies and reloads around
rematerialized stack arguments and calls (`mov rdi,[rbp+0x10]; mov r13,rdi`; the
arguments of `save_moves` shuffled through temporaries), not booleans. `@any_true` of a
512-bit vector still goes through `kmovq`, a compare and `setcc` (no `kortest`).
