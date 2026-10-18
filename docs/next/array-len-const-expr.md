## Language

- An array length can be any constant expression of an integer type, `[MAX_DEPTH + 1]u64`
  or `[@size_of(u64) * 2]u8`, not only a literal or a single `const` (spec §5.5).
