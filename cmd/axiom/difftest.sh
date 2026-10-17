#!/bin/sh
# Differential test of the self-hosted compiler against the C compiler:
#   - for every .ax file under tests/, std/, examples/ and cmd/axiom/src/, both
#     must print the same diagnostics (agent-json) with the same exit status;
#   - the test suite (tests/run.mjs, which checks the programs' output) must
#     pass for WebAssembly and native code in several configurations;
#   - the backends must agree with the C compiler's LLVM-generated code on
#     programs exercising every operation on edge values (fuzz/run.sh).
#
#   cmd/axiom/difftest.sh [compiler]     (default: cmd/axiom/build/axiom)
set -u
cd "$(dirname "$0")/../.."
NEW=${1:-selfhost/build/axiom}
OLD=cmd/axiom/bootstrap/build/axiom
pass=0
fail=0

# Each file's verdict ("same f" or "DIFF check f"), several files at a time.
export OLD NEW
out=$(find tests std examples cmd/axiom/src -name '*.ax' | sort |
    xargs -P "$(nproc 2>/dev/null || echo 4)" -n 16 sh -c '
        for f; do
            a=$($OLD check "$f" --format=agent-json 2>&1; echo "status $?")
            b=$($NEW check "$f" --format=agent-json 2>&1; echo "status $?")
            if [ "$a" = "$b" ]; then echo "same $f"; else echo "DIFF check $f"; fi
        done' sh)
pass=$(printf '%s\n' "$out" | grep -c '^same ')
fail=$(printf '%s\n' "$out" | grep -c '^DIFF ')
printf '%s\n' "$out" | grep '^DIFF ' | sort -k3
echo "diagnostics: $pass identical, $fail different"

# The sections that do not depend on the configuration run with the first.
skip_platform=
suite() {
    echo "suite: $*"
    if ! AXIOM=$NEW AXIOM_TEST_SKIP_PLATFORM=$skip_platform node tests/run.mjs "$@" > /tmp/axiom-difftest.$$ 2>&1; then
        grep -v '^ok' /tmp/axiom-difftest.$$
        fail=$((fail + 1))
    else
        tail -1 /tmp/axiom-difftest.$$
    fi
    rm -f /tmp/axiom-difftest.$$
    skip_platform=1
}
suite
suite -O2
suite --units=program
suite --native
suite --native -O2
if command -v qemu-aarch64 >/dev/null && [ -d /usr/aarch64-linux-gnu ] && command -v aarch64-linux-gnu-gcc >/dev/null &&
   [ "$(uname -m)" = x86_64 ]; then
    AXIOM_NATIVE_TARGET=aarch64 AXIOM_NATIVE_EXEC="qemu-aarch64 -L /usr/aarch64-linux-gnu" suite --native
fi
NEW=$NEW cmd/axiom/fuzz/run.sh 1 3 || fail=$((fail + 1))
[ $fail -eq 0 ]
