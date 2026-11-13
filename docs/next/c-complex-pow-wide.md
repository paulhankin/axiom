## C library: complex power and logarithm fixes

- `cpow` (and `cpowf`, `cpowf16`, `cpowl` of the x87 format) of a large `w` or a `z`
  near the unit circle, where the angle `Im(w log z)` needs more than its type's pairs
  hold, computes `w log z` in triple-double with the angle reduced exactly: each part
  within 2 ulps while `(|Re w| + |Im w|)(|log |z|| + |arg z|)` is below 2^90 for
  double (2^120 for float, 2^70 for the x87 format). A double `cpow` was off by
  hundreds of ulps from about 2^20 (`cpow(0.6 + 0.8i, 1e12)`), and the x87 one beyond
  2^60 returned unrelated values.
- `clogf128` (and `clogl` where `long double` is binary128, `catanhf128` and the other
  functions that take its real part) near the unit circle lost all its digits (`log |z|`
  for `|z| = 1 + 2^-110`): the squares' sum is now exact there.
