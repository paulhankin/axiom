## Compiler

- `axiom test` and `axiom profile` for another architecture find `qemu-aarch64-static`
  (or `qemu-x86_64-static`) as well as `qemu-aarch64`, and say which emulator they need
  when none is installed (`E9003`), instead of "the test runner failed".
