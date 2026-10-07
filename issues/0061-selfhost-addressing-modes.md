# 0061: Self-hosted x86-64: addressing modes not folded, address arithmetic recomputed in loops

- Type: feature (performance)
- Status: fixed (0cbcfb6, 4b24f5d)
- Area: compiler (selfhost: x64 address selection, gvn LICM/reassociation)

Part of the set 0053-0064 (the benchmark `bench_kernels`, selfhost against LLVM; see 0053).

## Today

**`base + disp + index*scale` not folded.** `b.raw[ON_ROLL][i]` (`raw: [2][25]u8`) in
`generate_moves_sub`:

```
selfhost                                   LLVM
lea    rdi,[rbx+0x19]                      movzx r10d,BYTE PTR [r9+r12*1+0x19]
movzx  ecx,BYTE PTR [rdi+r12*1]
lea    rcx,[rbx+0x19]
mov    BYTE PTR [rcx+r12*1],al
```

The `lea` is repeated for each access, even next to an earlier one. Stack arrays are
the same: `lea rsi,[rbp-0x78]; mov DWORD PTR [rsi+rdx*4],r8d`, and
`lea r8,[rbp-0x68]; mov r8d,DWORD PTR [r8+rdx*4]` in `dim128_pair`'s mask loops.

**Constant offsets not moved into the displacement.** `ZOBRIST[kd + old_dest + 1]` is
`add rsi,0x190; ...; mov rcx,rsi; add rcx,0x1; lea rdi,[rip+ZOBRIST]; mov
rcx,QWORD PTR [rdi+rcx*8]`. LLVM: `xor r10,QWORD PTR [rcx+r11*8+0xc88]`.

**Global addresses are recomputed per use.** There are 57 RIP-relative `lea`s in
`generate_moves_sub` (LLVM: 19), and `lea r8,[rip+INPVEC]` inside the `base_inputs`
loop. Each is cheap, but the code is twice as long. Hoist one per function or loop, as
LLVM does, or use `[rip+sym+disp]` directly when there is no index.

**Loop-invariant address sums are recomputed in the loop.** `accumulate16` inlined into
the pruned-boards kernel, `sr[i]` where `sr = &boards.at(b + j).raw[side]`:

```
lea    r13,[rbx+r12*1]       ; per iteration
lea    r13,[r13+rcx*1+0x0]
lea    r13,[r13+r8*1+0x0]
movzx  r13d,BYTE PTR [r13+r11*1+0x0]
```

LLVM computes `r10 = &sr[0]` before the loop and uses `movzx r8d,BYTE PTR [r10+rbx*1]`.
The `ptradd` chain `((base + a) + b) + c) + i` is not reassociated so that its invariant
part can be hoisted. The redundant `+0x0` displacement bytes are an encoder detail.

**Strength reduction makes four induction variables.** `calculate_race_inputs`' 23-point
loop keeps `rsi`, `rdi`, `r8` and `r9` as four pointers (`lea rsi,[rsi+0x10]` x4 per
iteration) where one base with displacements 0/4/8/12 would do.

## Proposal

- `x64.ax` address selection: match `ptradd(ptradd(p, c1), x*s)` and
  `ptradd(ptradd(p, x*s), c)` (and `add` forms) as one `[base + index*scale + disp]`,
  through one level of a single-use `lea` into its users, and through stack-slot bases
  (`[rbp - off + index*scale]`). Fold `index + c` into `disp + c*scale`.
- Keep the address of a global in a register when it is used several times in a loop or
  function (hoisted with `hoist_constants`). Otherwise use the RIP-relative operand
  directly.
- `gvn.ax`: reassociate `ptradd` chains to put loop-invariant terms first, so LICM hoists
  their sum. When strength-reducing several addresses with the same stride, share one
  induction variable and differ by displacement.

## Impact

Estimate: 3-5% of movegen and the input kernels, and some of the pruned kernels' loop
length (about 20-40 ns summed). The loop bodies get shorter and use fewer registers, which
also helps 0054.

## Resolution

Fixed in 0cbcfb6 (`x64.ax`) and 4b24f5d (`gvn.ax`):

- **`base + disp + index*scale`.** An indexed memory operand takes a constant offset of its
  base (a register, a stack slot or a folded `ptradd` with a constant) and of its index
  (`(x + c) * s`) into the displacement: `b.raw[ON_ROLL][i]` is `movzx esi,BYTE PTR
  [rdi+rbx*1+0x19]`, and stack arrays `[rbp-0x78+rdx*4]`.
- **Address sums in loops.** An address-only `ptradd` with a variable offset that is the
  base of another in a different block is computed once, where it is defined, instead of
  again at each use: the pruned-boards kernel's inner loop reads `movzx ebx,BYTE PTR
  [r8+r11*1]` instead of three `lea`s per iteration.
- **Globals.** A global's address is computed once per machine block (until a label).
  Hoisting it out of loops (`hoist_constants`) was tried: with the global in a register
  across the loop, the allocator spilled hot values instead (more instructions in most
  kernels), so it is left for 0054.
- **Induction pointers.** Strength-reduced addresses with the same step whose starts are
  a constant apart (each a linear combination of the same values plus constants) share one
  pointer, the others a displacement from it: `calculate_race_inputs`' 23-point loop has
  one pointer (`vmovss [rdx-0xc]`, `[rdx-0x8]`, `[rdx-0x4]`, `[rdx]`) instead of four.
  Not for wasm, whose offsets cannot be negative.

RIP-relative `lea`s in the the benchmark functions: 1,148 to 1,112; most of those left are the
shadow stack's site records (0053). Measured together with 0057 (its table).
