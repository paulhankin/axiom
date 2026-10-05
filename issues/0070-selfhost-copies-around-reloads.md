# 0070: Self-hosted register allocator: copies after reloads and rematerialized arguments, call arguments shuffled

- Type: feature (performance)
- Status: fixed (d5130b0, 30d8bc2, 106658d)
- Area: compiler (selfhost: mach, x64)

Follow-up to 0066 and 0067. After 0067, the benchmark's `MoveGenerator.generate_moves_sub`
(`src/core/moves.ax`) executes 619k instructions per 100 `generate_moves_into` calls against
LLVM's 539k. The booleans are branches now; most of the rest is register copies (101k
`mov r,r` against LLVM's 75k) and stack loads (66k against 46k):

- **A reload or rematerialized stack argument, then a copy.** The incoming stack arguments
  `hash` and `occ` are reloaded from their slots into one register and copied to another
  (`mov rdi,[rbp+0x10]; mov r13,rdi`, `mov r15,[rbp+0x18]; ...; mov rdx,r15`) in the hot
  path; the load could go straight to the copy's destination (0054 item 3, which 0066
  did for spill slots).
- **Call arguments shuffled through temporaries.** The `save_moves` call moves four
  arguments with six copies and three reloads, through registers that are themselves
  arguments: `mov rax,rdi; ...; mov rdx,rax; mov rax,[rbp-0x48]; mov rdi,rdx; mov rdx,rcx;
  mov rcx,rax` (LLVM: two copies and two reloads). The pieces of `self` reloaded after the
  recursive call take `rax` (the base of a compare) rather than `rdi`, where the call wants
  it.
- **Stores repeated at joins.** A value stored to its slot on one path into a join is
  stored again after the join before the call (`mov [rbp-0x68],r10`), because another path
  does not store it.
- **Loop invariants reloaded per block** after the recursive call (the 0066 leftover): `r`,
  `self`, `b`, `depth`, `occ`.

Also left from 0067: `@any_true`/`@all_true` of 512-bit vectors go through `kmovq`, a
compare and `setcc` where `kortest` and a jump would do.

## Resolution

d5130b0, 30d8bc2 and 106658d: a load whose value is only copied loads straight into
the copy's register (`fold_load_copies`, for stack arguments and spill slots), reloads of
a slot whose register still holds it are dropped across blocks as well as within one
(`drop_reloads` as a dataflow over the function), and a test after a flag-setting
instruction uses its flags. With the rest of these commits `generate_moves_sub` went from
614k to 595k instructions (LLVM: 546k) and the 100 `generate_moves_into` calls from
1049k to 1017k (901k). Call-argument shuffles, stores at joins, splits at loop edges,
mask registers and the rest moved to 0077; `kortest` to 0078.
