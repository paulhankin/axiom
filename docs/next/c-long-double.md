## C

- `long double` has the format of the target's C ABI: the x87's 80-bit extended format
  on `x86_64` Linux and IEEE 754 binary128 on `aarch64` Linux and WebAssembly (16 bytes,
  aligned to 16), so that C compiled here passes `long double` values to and from the
  platform's libraries; it stays `double` on Windows and macOS. Its arithmetic and
  conversions are correctly rounded software (`std/c/rt/ldouble.ax`), constants are
  read and folded exactly in that format, and `<float.h>`'s `LDBL_*` and `DECIMAL_DIG`
  describe it. `--c-long-double=64|80|128` chooses another format (`64` restores the
  former `long double`, the same as `double`; `__AXIOM_LDBL_WIDTH__` says which).
  Programs that relied on `sizeof(long double) == 8` or on `long double` being `double`
  on those targets now get the wider type.
