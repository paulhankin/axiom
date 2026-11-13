## C front end

- The x86-64 assembler now encodes the AVX (VEX) forms of the SSE3, SSSE3,
  SSE4.1 and SSE4.2 instructions whose legacy forms it already had: the
  horizontal arithmetic and `vaddsub*`, `vrcpps`/`vrcpss`/`vrsqrtps`/`vrsqrtss`,
  `vmovddup`/`vmovshdup`/`vmovsldup`/`vlddqu`, the `vmovhps`/`vmovlps`/`vmovhpd`/
  `vmovlpd`/`vmovhlps`/`vmovlhps` moves, `vphadd*`/`vphsub*`/`vpsign*`/`vpabs*`/
  `vpmaddubsw`/`vpmulhrsw`, `vround*`, `vpalignr`, `vptest`, `vphminposuw`,
  `vpackusdw`, `vpmovsxbw`/`vpmovzxbw`, `vpminsb`/`vpminuw`/`vpmaxsb`/`vpmaxuw`,
  `vmpsadbw`, the AVX conditional moves `vmaskmovps`/`vmaskmovpd`/`vpmaskmovd`/
  `vpmaskmovq`, the mask-unpack instructions `kunpckbw`/`kunpckwd`/`kunpckdq`, and
  the size-suffixed scalar conversions `vcvtsi2ssl`/`vcvtsi2ssq`/`vcvtsi2sdl`/
  `vcvtsi2sdq`; and the AVX-512BW/F (EVEX) forms, with masking, zeroing and
  broadcast, of those that the architecture extends there (`vpabs[bwdq]`,
  `vpmaddubsw`, `vpmulhrsw`, `vpalignr`, `vpackusdw`, `vpmovsxbw`/`vpmovzxbw`,
  `vpminsb`/`vpminuw`/`vpmaxsb`/`vpmaxuw`).
