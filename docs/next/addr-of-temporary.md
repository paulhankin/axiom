## Language

- `&expr` works on any value, not only places: a temporary (a call's result, say) is
  materialized in a hidden local that lives until the function returns, so
  `f(&cfg.with_threads(2))` passes a builder's result by pointer, and a `*Self` method
  can be called on a temporary. `&mut` of a temporary is still an error (E0703).
