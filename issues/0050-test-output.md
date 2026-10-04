# 0050: `axiom test` hides what a test prints

- Type: feature
- Status: open
- Area: tools (test runner)

`axiom test` captures a test's output and shows nothing of it, even for a passing test, so a
test cannot be used to print values for comparison with another implementation (the benchmark's
ports diff Axiom against Rust row by row). Each such check needs a separate scratch program,
and with the -O2 build times of issue 0040 that cost ~5 minutes per iteration. Proposal:
`axiom test --show-output` (cargo's `--nocapture`), printing each test's output after its
result line, and showing the output of failing tests by default.
