# 0057: Self-hosted x86-64: scalar instruction selection leaves extensions, flag materialisation and copies

- Type: feature (performance)
- Status: open
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
