# 0067: Self-hosted optimizer: booleans materialised as 0/1 and tested again

- Type: feature (performance)
- Status: in progress
- Area: compiler (selfhost: combine, gvn, thread, x64)

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
