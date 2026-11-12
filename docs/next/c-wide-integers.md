## C

- Integers wider than 64 bits: `__int128` and `unsigned __int128` in the GNU dialects
  (and the attribute `mode(TI)`), and C23's `_BitInt(N)` up to `BITINT_MAXWIDTH`, now
  65535 (it was 64), with `wb` and `uwb` constants of any width. Every operator,
  conversion (to and from every integer and floating type, `long double` included,
  correctly rounded), `switch`, `_Atomic` objects, `<stdbit.h>`, `<stdckdint.h>` and the
  overflow builtins work on them; constant expressions are folded in the type's
  precision. They are laid out and passed as each platform's C ABI says (x86-64 System V,
  AAPCS64, Windows x64, WebAssembly), checked against the system's C compiler on x86-64
  and AArch64 Linux; bit-fields of these types too. Values of up to 128 bits are
  computed inline (add/adc, sub/sbb on x86-64; adds/adc, subs/sbc on AArch64), their
  division on x86-64 with the 128-by-64-bit `div`; wider ones call `std/c/rt/wideint.ax`.
