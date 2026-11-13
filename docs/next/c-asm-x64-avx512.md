## C front end

- The x86-64 assembler (inline asm, file-scope asm, `.s` sources) now encodes the rest
  of AVX-512 — the F/DQ/BW/CD forms that were missing, VBMI2, BITALG, VNNI,
  VP2INTERSECT, BF16, the whole of FP16, ER, PF, 4FMAPS and 4VNNIW — with masking,
  zeroing, broadcast, rounding and `{sae}`, and the VEX instructions of AVX-VNNI-INT8,
  AVX-VNNI-INT16, AVX-IFMA, AVX-NE-CONVERT, SHA512, SM3, SM4, CMPccXADD and FMA4, in
  both syntaxes. The rounding operand follows the conventional placement
  (`vcvtusi2ss %eax, {rn-sae}, %xmm0, %xmm1`), and the Intel syntax's `WORD BCST` /
  `DWORD BCST` / `QWORD BCST` must name the instruction's element size.
