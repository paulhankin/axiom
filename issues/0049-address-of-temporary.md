# 0049: Cannot take the address of a temporary

- Type: feature
- Status: open
- Area: language

`f(&cfg.with_threads(2))` is E0703 (address of a temporary), so every builder result that is
passed by pointer must first be bound to a local (found porting the benchmark's rollouts, whose API
takes `*RolloutConfig` and `*EvalContext` built with `with_*` chains). Rust and C++ extend the
temporary's lifetime to the enclosing statement. Proposal: allow `&expr` for an rvalue,
materialising it in a hidden local that lives to the end of the enclosing statement (or the
end of the block for a `let` initializer); `&mut` of a temporary may stay an error.
