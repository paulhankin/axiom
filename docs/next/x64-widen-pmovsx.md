## The compiler

- `@widen_low` and `@widen_high` compile to `pmovsx`/`pmovzx` on x86-64 with SSE4.1 (one
  instruction per result register, after a `pshufd`, `vextracti128` or `vextracti64x4`
  for the high half), on 128-, 256- and 512-bit vectors as the target's extensions allow,
  instead of lane by lane.
