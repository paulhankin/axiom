## Assembler

- The AArch64 assembler of inline assembly, file-scope asm and assembly sources encodes
  the base and system instruction sets in full (pointer authentication and PAuth_LR, the
  unprivileged and RCpc loads and stores, `casp`, memory tagging, the memory-copy and
  memory-set instructions, the 128-bit atomics, read-check-write, CSSC, checked pointer
  arithmetic, `bc.cond`, the system-level-cache and range prefetches, the system
  instruction aliases `at`, `dc`, `ic`, `tlbi` (with its range and nXS forms) and `tlbip`,
  `sys`, `sysl`, `sysp`, flag manipulation, `wfet`/`wfit`, transactional memory, the
  guarded control stack, the 64-byte loads and stores, the fixed-point conversions,
  literal pools: `ldr x0, =value`, `.ltorg`); Advanced SIMD in full, each instruction with
  exactly the arrangements the architecture allows (the dot-product, matrix-multiply,
  complex-number, BF16, `fmlal`-class, FAMINMAX, FP8 and LUT extensions, `pmull` and the
  cryptographic instructions, every structure load and store); SVE and SVE2 (every
  addressing form of their loads, stores and prefetches); and SME, SME2, SME2.1, SVE2.1
  and B16B16 (the ZA array, tiles and slices, `zt0`, the predicate-as-counter registers
  `pn0`-`pn15`, strided register lists, the multi-vector instructions). The tables are
  generated from the architecture's encodings (`cmd/internal/tools/gen_a64_table.py`,
  `gen_a64_sysregs.py`, `gen_a64_sysops.py`) and checked by round-trip fuzzing.
- `mrs`/`msr` know the architecture's 1210 named system registers, each in the
  directions it may be accessed (`msr midr_el1, x0` is an error), with the generic
  `s3_0_c15_c0_0` form and `msr <pstatefield>, #imm`; `mrrs`/`msrr` take only the 128-bit
  ones. `msr dit`, `msr ssbs`, `msr tco` and the `svcr` fields have their right
  encodings.
- SME, SME2, SME2.1, SVE2.1 and their kin need their extension, known to `--features`
  and `--cpu` (`sme`, `sme2`, `sme2p1`, `sve2p1`, `sme-f64f64`, `sme-i16i64`,
  `sme-f16f16`, `b16b16`); `smstart`, `smstop`, `rdsvl`, `addsvl`, `addspl`, `revd`,
  `sclamp`, `uclamp` and `fclamp` need theirs too. `.arch_extension name` allows an
  extension's instructions in the rest of an assembly without changing what the program
  needs at run time.
- The ABI's relocation operators: `:pg_hi21:`, `:pg_hi21_nc:`, the move-wide groups
  (`:abs_g0:`-`:abs_g3:`, `_nc`, `_s`, `:prel_g*:`), the thread-local ones of every code
  model (`:tprel_*:`, `:dtprel_*:`, `:gottprel:`, `:gottprel_lo12:`, `:gottprel_g1:`,
  `:tlsdesc:`, `:tlsdesc_lo12:`, `:tlsdesc_off_g1:`, `:tlsldm:`, `.tlsdesccall`,
  `.tlsdescldr`, `.tlsdescadd`), the Mach-O spellings (`sym@PAGE`, `@PAGEOFF`,
  `@GOTPAGE`, `@GOTPAGEOFF`, and the thread-local descriptors' `@TLVPPAGE` and
  `@TLVPPAGEOFF`) and COFF's `:secrel_lo12:`/`:secrel_hi12:`; and `adr`, `b.cond`, `cbz`,
  `cbnz`, `tbz`, `tbnz`, `prfm` and literal loads to symbols of other sections and units.
  The linkers apply them with the ABI's range checks and relax the thread-local
  sequences of an executable's own variables as the ABI describes. On macOS and Windows
  an absolute address in the complete move-wide sequence assembles (as `adrp` and `add`);
  in a `.word` or `.hword`, which such an image cannot hold, it is an error.
- Inline assembly takes the rest of the dialect's machine constraints (`Ump`, `Umq`, the
  SVE memory forms, `Usa`, `Ush`, `Y`, `Dz`, `Dn`, `Upa`, `Upl`, and `k`: the stack
  pointer), objects wider than a scalar in vector registers (up to 16 bytes in a q
  register, up to 256 in an SVE z register), and the clobbers `z`, `p`, `pn`, `ffr`; `%z`
  writes a vector register as `z<n>`, and a `z<n>` clobber protects `v<n>`.
- Encodings the architecture makes CONSTRAINED UNPREDICTABLE are errors (writeback to a
  transferred register, a pair loaded into one register, a store exclusive whose status
  register is its data or base register), as are shift amounts beyond the operation's
  width, register lists of mixed arrangements, a 32-bit offset register without its
  extend and a 32-bit logical immediate that does not fit 32 bits: they were assembled as
  something else.
- Fixed: an asm statement whose `=&r` output and memory operand had the same stand-in
  register failed its early check.
