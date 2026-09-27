#!/bin/sh
# Differential test of the self-hosted compiler against the C compiler:
#   - for every .ph file under tests/, std/, examples/ and selfhost/src/, both
#     must print the same diagnostics (agent-json) with the same exit status;
#   - the test suite (tests/run.mjs, which checks the programs' output) must
#     pass for WebAssembly and native code in several configurations;
#   - the backends must agree with the C compiler's LLVM-generated code on
#     programs exercising every operation on edge values (fuzz/run.sh).
#
#   selfhost/difftest.sh [compiler]     (default: selfhost/build/phalange)
set -u
cd "$(dirname "$0")/.."
NEW=${1:-selfhost/build/phalange}
OLD=compiler/build/phalange
pass=0
fail=0

files=$(find tests std examples selfhost/src -name '*.ph' | sort)
for f in $files; do
    a=$($OLD check "$f" --format=agent-json 2>&1; echo "status $?")
    b=$($NEW check "$f" --format=agent-json 2>&1; echo "status $?")
    if [ "$a" = "$b" ]; then
        pass=$((pass + 1))
    else
        fail=$((fail + 1))
        echo "DIFF check $f"
    fi
done
echo "diagnostics: $pass identical, $fail different"

suite() {
    echo "suite: $*"
    if ! PHALANGE=$NEW node tests/run.mjs "$@" > /tmp/phalange-difftest.$$ 2>&1; then
        grep -v '^ok' /tmp/phalange-difftest.$$
        fail=$((fail + 1))
    else
        tail -1 /tmp/phalange-difftest.$$
    fi
    rm -f /tmp/phalange-difftest.$$
}
suite
suite -O2
suite --units=program
suite --native
suite --native -O2
if command -v qemu-aarch64 >/dev/null && [ -d /usr/aarch64-linux-gnu ] && command -v aarch64-linux-gnu-gcc >/dev/null &&
   [ "$(uname -m)" = x86_64 ]; then
    PHALANGE_NATIVE_TARGET=aarch64 PHALANGE_NATIVE_EXEC="qemu-aarch64 -L /usr/aarch64-linux-gnu" suite --native
fi
NEW=$NEW selfhost/fuzz/run.sh 1 3 || fail=$((fail + 1))
[ $fail -eq 0 ]
