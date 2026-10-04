# 0057: Self-hosted x86-64: scalar instruction selection leaves extensions, flag materialisation and copies

- Type: feature (performance)
- Status: fixed (0cbcfb6, 9e2bb29)
- Area: compiler (selfhost: x64 instruction selection, x64enc, opt layout)

Part of the set 0053-0064 (the benchmark `bench_kernels`, selfhost against LLVM; see 0053).
These are small per instruction but they are everywhere in the scalar kernels (movegen,
`*_inputs`). Across the the benchmark functions the selfhost emits 23,400 instructions and LLVM
16,700.

## Today

**Redundant extensions; `@min`/`@max` without immediates.** `base_inputs_with_masks`
(`pruning_inputs`, 24 iterations x 2):
`@ptr_add(inp, @as(usize, @min(b0_side[i], 15))).*`

```
selfhost                                   LLVM
movzx  edx,BYTE PTR [rdi+rax*1]            movzx  r9d,BYTE PTR [rdi+rcx*1+0x18]
movzx  edx,dl              ; again         cmp    r9b,0xf
mov    r8d,0xf             ; constant      cmovae r9d,edx      ; edx = 15, hoisted
movzx  r8d,r8b             ; zext of it    movzx  r9d,r9b
mov    r9,rdx              ; 2-address     shl    r9d,0x4
cmp    edx,r8d                             vmovups xmm0,[r9+r8*1]
cmova  r9d,r8d
movzx  edx,r9b
shl    rdx,0x4
lea    r8,[rip+INPVEC]     ; per use (0061)
vmovdqu xmm0,XMMWORD PTR [r8+rdx*1]
```

`x64.ax` `gen_int_intr`'s `IN_SMIN`/`IN_UMIN`/... case extends both operands with
`reg_ext`, even when one is a constant (compare with an immediate instead) or already
zero-extended by its `movzx` load. Elsewhere: `mov r12d,r12d` after a 32-bit op (11 in
`calculate_half_inputs_with_masks`), `and esi,0xffff; mov esi,esi`, and `tzcnt
r11d,r9d; mov r11d,r11d` in the FMA loops. The selector should know which values
already have zero upper bits (results of 32-bit ops, `movzx` loads, `setcc+movzx`,
`tzcnt`/`lzcnt`/`popcnt`) and drop the extension.

**Booleans materialised before branching.** There are 169 `setcc` instructions in the the benchmark
code, against 19 for LLVM. Typical (`save_moves`, the dedup probe loop):

```
vpmovmskb edi,xmm0
cmp    edi,0xffff
setne  dil
movzx  edi,dil
test   edi,edi
je     ...
```

LLVM emits `cmp; jne`. A compare in the same block whose only uses are a branch (possibly
through `and`/`or`/`xor 1` of compares, or a 0/1 value kept in a stack slot) should
become the branch's flags. Short-circuit `and`/`or` of compares should become two
branches, or `setcc; and; jcc`, but not three steps per operand. The same holds for
`generate_moves_sub`'s `legal`/`off`/`hit` flags (`setb sil; movzx esi,sil; ...; test
sil,sil; je`).

**Read-modify-write memory.** `b.raw[ON_ROLL][BAR] += 1` is
`movzx eax,BYTE PTR [rbx+0x31]; add eax,0x1; mov BYTE PTR [rbx+0x31],al`. LLVM emits
`inc BYTE PTR [r9+0x31]` (also `dec`, `add [m],imm`, `or [m],r`). `generate_moves_sub`
does six of these per step.

**Constants and copies.** There are 336 `mov r32,0x0`, against 39 for LLVM. Many of them
are for `cmovae r,zero`, and many are in loops, as in `cmp r8,0x20; mov eax,0x0; mov
rdx,rcx; cmp rcx,0x20; cmovae rdx,rax` (the same compare twice around a constant
load). Then there is `mov r8d,0x18; mov r12,r8; ...; sub r12,r8` (24 - r through a copy),
and `add eax,0xffffffff` (69, against 1 for LLVM) for `sub`/`dec`/`lea -1`. Hoist
constants that a loop uses into registers, emit `xor r,r` where the flags are dead,
re-use flags from an identical earlier compare, and use `lea` for 3-address
add/sub/shift-add forms (LLVM: `lea eax,[r8+rax*1-0x1]`).

**Encoding and layout.** 48 `rex cmp`/`rex test` with a REX prefix that carries no bits
(`x64enc.ax`: byte-register operands that do not need REX). 13 `jmp` to the
next instruction (`opt.ax` layout should drop them, e.g. `generate_moves_sub+0x6bc: jmp
459ef1` followed by `459ef1`).

## Proposal

In `x64.ax`: track known-zero-extended virtual registers (a bit per vreg set by
32-bit defs, `movzx`, `setcc`+`movzx`, bit counts) and skip `ext_reg`/`EXT` for them.
Give `min`/`max`/`cmp`/`test` immediate forms. Fuse compares into their branches when
the boolean's only use is the branch (including through `and`/`or`/`not`). Fold
`load; op; store` to the same address into one memory-destination instruction (the
selector already folds loads into operands). Use `lea` for `a + b + c` and `a - k`.
Emit `xor` for zero where the flags are free. In `x64enc.ax`: only emit REX when a bit
is set or for `spl/bpl/sil/dil`. In `opt.ax` layout: remove unconditional jumps to the
next block.

## Impact

Estimate: 5-10% on movegen and the input kernels (about 50-80 ns summed). For example,
the `base_inputs` loop body is 24 instructions per iteration where 12 would do.

## Resolution

Fixed in 0cbcfb6 (with 0061's addressing modes) and 9e2bb29, in `x64.ax` and `x64enc.ax`:

- **Branches on compares.** A branch on an `and`/`or`/`not` of comparisons used only there
  jumps on each comparison's flags (`branch_tree`: `cmp; jcc; cmp; jcc`). A comparison whose
  every use is the condition of a branch or select in its block is compared again at each
  (no `setcc`, and no dead `cmp` left behind). `setcc` in the the benchmark functions: 169 to 63.
  Those left are booleans used across blocks or as values, and `@any`/`@all` of vectors
  (`vpmovmskb; cmp; setne; movzx; test; je` in `save_moves`), whose fusion into the branch
  belongs with the vector selection (0058/0062).
- **Extensions.** `zext_known` knows which values have zero upper bits (32-bit operations,
  loads, logical shifts, unsigned division, bit counts, masks, `umin`/`umax`, selects and
  phis of such, masked truncations, float bitcasts); their zero extensions are copies.
  `@min`/`@max` compare at the operands' own width, with an immediate for a constant, and
  copy after the compare: the `base_inputs` loop body is `movzx; mov r,15; cmp dl,15;
  cmova; shl` (LLVM's shape). `movzx`: 323 to 179; `mov r32,r32`: 39 to 6.
- **Read-modify-write.** `load; add/sub/and/or/xor; store` to the same address is one
  instruction with a memory destination (`add BYTE PTR [rax+0x31],0x1`).
- **Constants and copies, after allocation.** `mov r, 0` is `xor r32, r32` where the flags
  are dead (336 to 4 `mov r,0`); `mov d, a; add d, b|k` (and `sub k`) is `lea` where the
  flags are dead (`add r,-1`: 106 to 47); a compare identical to an earlier one in the block,
  with nothing between that sets the flags or writes its registers, is dropped; `k - x` puts
  `k` straight into the result.
- **Encoding and layout.** No REX for byte operations whose only byte register is a
  `/digit` or the wider operand of `movzx` (48 to 0); no jumps to the next instruction,
  past blocks that encode nothing and to local labels (13 to 0).

Not done: constants used by `cmov` in loops are still built in the loop (`mov r11d,0x3`);
hoisting them, like hoisting globals' addresses (tried for 0061), raises register pressure
that the allocator (0054) answers with spills.

Instructions in the the benchmark functions of `bench_kernels`: 23,362 before, 22,320 after
(LLVM: 16,700). Tests: `tests/lowering/x64_scalar.ax`.

Measured with 0059, 0061 and 0064 (`bench_kernels`, ns per position, min of 15 interleaved
runs, before 0053's frame tables; the machine was shared, load 6-12 on 4 cores, so about
±10%: the net kernels, which are vector code, moved within that noise):

| kernel | before | 0059 (68528df) | + 0057, 0061 (0cbcfb6) | all (4710d95) | LLVM |
|---|---|---|---|---|---|
| movegen | 1136 | 1126 | 1114 | 1087 | 724 |
| contact_inputs | 337 | 335 | 282 | 307 | 186 |
| race_inputs | 216 | 202 | 193 | 208 | 108 |
| pruning_inputs | 38.3 | 39.3 | 25.5 | 27.6 | 19.1 |
| contact_net | 409 | 364 | 385 | 424 | 291 |
| race_net | 263 | 283 | 253 | 266 | 203 |
| pruned_contact | 111 | 112 | 112 | 109 | 91 |
| pruned_race | 158 | 142 | 151 | 147 | 130 |
| contact_single | 385 | 379 | 400 | 398 | 310 |
| pruned_single | 163 | 158 | 156 | 157 | 123 |
| pruned_boards | 83.5 | 83.1 | 80.6 | 77.4 | 53.1 |

Repeated runs of single kernels (25 each) put contact_inputs at 297-307 for every build
from 0cbcfb6 on, contact_net at 348-352 for before, 0059 and all. `bench_search --mode 2ply
--limit 50` gives identical results (and every checksum is unchanged).
