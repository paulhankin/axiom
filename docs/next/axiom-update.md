## Tools

- `axiom update [<version> | latest | head]` replaces the toolchain with another
  version. An install from a release archive updates to a release (the latest by
  default): it downloads the archive for its platform, checks it against the release's
  `SHA256SUMS`, and swaps it in where the old one was. A source checkout moves to the
  release's tag, or to the head of `main` (`head`, in any case, and the default), and
  rebuilds the compiler. See `docs/install.md`.
- On Windows, the compiler finds its own file, and so the standard library next to
  it, when it is run by name from the `PATH` (as `axiom`, without `.exe`).
