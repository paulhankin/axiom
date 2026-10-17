# Security policy

## Supported versions

Fixes land on `main`, and the latest release (`0.N.M`) gets them in a fix release
from its `release-0.N` branch (see [`docs/releasing.md`](docs/releasing.md)). Older
releases are not supported.

## Reporting a vulnerability

Please report vulnerabilities privately, through GitHub's private vulnerability
reporting: on the repository's **Security** tab, choose **Report a vulnerability**
(or go to https://github.com/paulhankin/axiom/security/advisories/new). Do not open a
public issue for a vulnerability.

Include what is affected (the compiler, a `std` module, the runtime), the target
(WebAssembly, or native on Linux, macOS or Windows), the compiler and flags, and an
input or program that shows the problem. Reports are handled on a best-effort basis;
expect an acknowledgement within a week. A fix is made on `main` and published with a
GitHub security advisory, crediting the reporter unless they prefer otherwise.

## What counts

Axiom promises that checked code (everything except raw pointers) has defined
results or traps, and the standard library promises that for any input, a parser
returns an error and its work and memory stay bounded by a small multiple of the
input's size (see [`docs/security.md`](docs/security.md)). Breaking one of those
promises is a vulnerability. For example:

- **The compiler** generating code that skips a bounds check, an overflow check in an
  `--overflow=trap` build, or any other check the spec requires; or code that is wrong
  in a way an attacker could exploit.
- **The standard library**: memory corruption, an input that a parser should refuse
  but accepts (request smuggling, certificate-validation gaps, TLS state-machine
  errors), work or memory unbounded in the input (decompression bombs, quadratic
  time), path or symbolic-link escapes, terminal-escape injection, or a cryptographic
  weakness (timing leaks in code meant to be constant time, weak randomness).
- **The runtime** (`std/internal/runtime/`): anything that lets a program's input
  escape the guarantees above.

A trap that untrusted input can reach is a denial of service, and worth reporting
privately if it affects a network-facing module (`std/net/...`); elsewhere, a public
issue is fine.

## What does not count

- Programs that break the access rule of spec §12.1 with raw pointers: on native
  targets that is undefined behavior, as in C, by design.
- Bugs in C code linked into a program (spec §14.5.5).
- The bootstrap compiler (`cmd/axiom/bootstrap/`), which exists only to build the
  compiler, except where it miscompiles the compiler itself.
- The examples (`examples/`) and the test suite, which are not hardened.
