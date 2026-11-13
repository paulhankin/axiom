## C

- Enumeration constants wider than `long long`: in a GNU dialect an enumeration may have
  the fixed underlying type `__int128` or `unsigned __int128` with constants of any value
  of it, and in C23 one without a fixed type whose values `long long` and
  `unsigned long long` do not hold has the type `__int128` or `unsigned __int128`. The
  constants follow C23's typing rules in the list (the type of their expression, or a
  wider type past the previous one's) and after it (the enumerated type). Such a constant
  was the error `C3212` before.
