#!/bin/sh
# The compiler's whole check, in every configuration:
#   - the test suite (cmd/axiom/internal/tests/run.mjs, which checks the programs' output) must
#     pass for WebAssembly and native code in several configurations;
#   - the backends must agree with each other on programs exercising every
#     operation on edge values (fuzz/run.sh).
#
#   cmd/axiom/difftest.sh [compiler]     (default: cmd/axiom/build/axiom)
set -u
cd "$(dirname "$0")/../.."
NEW=${1:-cmd/axiom/build/axiom}
T=$(mktemp -d)
trap 'rm -rf "$T"' EXIT
fail=0

# The sections that do not depend on the configuration run with the first.
skip_platform=
suite() {
    echo "suite: $*"
    if ! AXIOM=$NEW AXIOM_TEST_SKIP_PLATFORM=$skip_platform node cmd/axiom/internal/tests/run.mjs "$@" > "$T/suite.log" 2>&1; then
        grep -v '^ok' "$T/suite.log"
        fail=$((fail + 1))
    else
        tail -1 "$T/suite.log"
    fi
    skip_platform=1
}
suite
suite -O2
suite --units=program
suite --native
suite --native -O2
# (qemu-aarch64 by that name: `axiom test --target=aarch64` looks for it too)
if command -v qemu-aarch64 >/dev/null && [ -d /usr/aarch64-linux-gnu ] && command -v aarch64-linux-gnu-gcc >/dev/null &&
   [ "$(uname -m)" = x86_64 ]; then
    AXIOM_NATIVE_TARGET=aarch64 AXIOM_NATIVE_EXEC="qemu-aarch64 -L /usr/aarch64-linux-gnu" suite --native
fi
NEW=$NEW cmd/axiom/fuzz/run.sh 1 3 || fail=$((fail + 1))
[ $fail -eq 0 ]
