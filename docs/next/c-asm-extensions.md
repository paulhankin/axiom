## C front end

- The inline and file-scope assemblers (`cmd/axiom/src/asm/`) cover much more of
  each architecture and of the conventional assembler's own language. On x86-64 they now
  encode the x87 FPU, MMX, SSE2 through SSE4.2, AVX and AVX2, FMA, F16C, the
  AVX-512 F/VL/BW/DQ instructions with masking (`{%k1}{z}`), broadcast
  (`{1to16}`) and embedded rounding (`{rn-sae}`), and the system instruction set
  (`cli`, `wrmsr`, `mov` to and from control and debug registers, ...). On
  AArch64 the system registers are now named from a generated table (the generic
  `s3_0_c15_c0_0` form still works), with the `msr <pstatefield>, #imm` forms.
  The instruction tables are generated from the architecture manuals by
  `cmd/internal/tools/gen_x64_table.py` and `gen_a64_sysregs.py`.
- Assembly text — in a template or a file-scope `asm` — now supports `.macro`
  with arguments, defaults, `:req`, `:vararg` and `\@`; `.rept`, `.irp` and
  `.irpc`; `.if`/`.ifdef`/`.ifc`/`.else`/`.elseif`/`.endif`; `.equiv`, `.set`
  and `name = value`; the conventional assembler's full expression precedence (the
  comparison and logical operators, symbol differences); `.comm`, `.lcomm`,
  `.incbin` (bounded to the include directories), `.octa`, `.uleb128`,
  `.sleb128` and the `0f`/`0d` floating forms; `.section` with flags and types
  and `.subsection`. `.balign`/`.p2align` in code pad with the multi-byte NOP
  sequences.
