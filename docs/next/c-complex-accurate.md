## C library

- `<complex.h>`'s functions (every type: `float`, `double`, `long double` in each of its
  formats, `_Float16`, `_Float128`) compute each part in pairs of the real type from real
  functions that give about twice its precision, and round once: each part of `cexp`,
  `clog`, `csqrt`, `csin` ... `ctanh`, `casin` ... `catanh` and `cpow` is within half an
  ulp and a small fraction where it is normal (were up to 2.9 ulps; measured against an
  arbitrary-precision reference near the branch cuts, the unit circle, the poles of
  `ctan` and the overflow and underflow thresholds too), with Annex G's special values as
  before. Against the former, less accurate functions a double function takes 1.1 to 2.1
  times their time (`cexp` and `clog` 1.1, `csqrt` 1.35, `casin` and `cacos` 2.1).
- `cpow` (and `cpowf`, `cpowf16`, `cpowl` of the x87 format) of a large `w` or a `z` near
  the unit circle computes `w log z` in triple-double with the angle reduced exactly: each
  part within 2 ulps while `(|Re w| + |Im w|)(|log |z|| + |arg z|)` is below 2^90 for
  double (2^120 for float, 2^70 for the x87 format); beyond, the error grows (docs/c.md,
  "Known gaps"). A double `cpow` was off by hundreds of ulps from about 2^20
  (`cpow(0.6 + 0.8i, 1e12)`), and the x87 one beyond 2^60 returned unrelated values.
- Fixed: `clog` (and `casin`, `cacos`, `casinh`, `cacosh`, `cpow`) of an argument beyond
  about 1e154 in magnitude returned an infinite or NaN real part; `cacos` and `cacosh` of
  a large argument near the real axis lost the low-order digits of their small part;
  `clogf128` (and `clogl` where `long double` is binary128, and the functions that take
  its real part) near the unit circle lost all its digits; on a processor without fused
  multiply-add, `catanh` and `catan` of an argument near 1e150 with a very small other
  part returned a NaN part, and `casin` and `casinh` of an argument with a subnormal part
  could differ in its last bit from the result with fused multiply-add.
