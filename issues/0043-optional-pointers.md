# 0043: No optional pointers or optional values

- Type: feature (design)
- Status: open
- Area: language

Rust code passes `Option<&T>`, `Option<Arc<...>>` and returns `Option<T>` everywhere
(the benchmark's evaluator: an optional endgame-table database, optional caches, optional cube info,
lookups that may miss). Axiom has neither nullable pointers nor an option type, so the port
(the benchmark `axiom/src/eval/`) uses a sentinel object, a `usize` address that is 0 for
none, or a value plus a `bool`, depending on the case: each one an ad-hoc convention that
the compiler cannot check.

Proposal: `?*T` (a pointer that may be null, which must be tested before use, with the
`*T` representation and no cost) and `?T` for values (a tagged union with `T`, `orelse`,
`if (x) |v|` unwrapping as Zig does), or a standard-library `Option[T]` union if the
language prefers no new syntax.
