## Building from source

- `cmd/axiom/bootstrap.sh` now builds a compiler for the generic processor of the
  machine's architecture (`x86_64`, `aarch64` or `aarch64-macos`), so the binary runs on
  any machine of that kind; `TARGET=native` restores the build tuned to the machine's own
  extensions. The compiler speed is the same within measurement noise.
