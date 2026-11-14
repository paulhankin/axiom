## C front end

- The AArch64 assembler (asm statements, file-scope asm, `.s` and `.S` sources) has
  `bc.cond`, the system-level-cache prefetch operations (`prfm pldslckeep, [x0]`) and
  the range prefetch `rprfm`, the checked pointer arithmetic (`addpt`, `subpt`,
  `maddpt`, `msubpt`), PAuth_LR (`paciasppc`, `autiasppc`, `retaasppc`, `pacm`, the
  `171615` forms ...), and in Advanced SIMD FAMINMAX (`famax`, `famin`), FP8 (the
  `f1cvtl` family, `fcvtn` to FP8, `fscale`, `fdot`, `fmlalb`, `fmlallbb` ...) and LUT
  (`luti2`, `luti4`); `brb iall`/`brb inj` and the `tlbi vmallws2e1*` operations.
- `mrs`/`msr` know the architecture's 1210 named system registers (before: 126), each
  in the directions it may be accessed (`msr midr_el1, x0` is now an error), and
  `mrrs`/`msrr` take only the 128-bit ones.
- Encodings the architecture makes CONSTRAINED UNPREDICTABLE are now errors instead of
  being assembled: writeback to a register the instruction transfers (`ldr x1, [x1],
  #8`), a pair loaded into one register (`ldp x0, x0, [x1]`), a store exclusive whose
  status register is its data or base register. A shift amount beyond the operation's
  width (`eor w0, w1, w2, asr #44`) and a negative one are errors; they were encoded as
  some other amount. A register list must have one arrangement (`{v0.8h, v1.4h}` is an
  error), a 32-bit offset register its extend (`[x0, w1, uxtw]`; `[x0, w1]` was taken
  as uxtw), and a 32-bit logical instruction's immediate must fit in 32 bits (it was
  truncated).
- An asm statement whose `=&r` output and memory operand happened to have the same
  stand-in register no longer fails its early check.
