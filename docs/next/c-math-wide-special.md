## C

- `erfl`, `erfcl`, `lgammal` and `tgammal` (and `erff128` ... `tgammaf128`) compute in the
  format's precision where `long double` is wider than `double`: within 0.55 ulp of
  binary128. The `_Float16` elementary functions are correctly rounded.
