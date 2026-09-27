#!/bin/sh
# Differential test of the self-hosted compiler against the C compiler: for
# every .ph file under tests/, std/, examples/ and selfhost/src/, both must
# print the same diagnostics (agent-json) with the same exit status, and
# emit byte-identical LLVM IR in several configurations. The IR is compared
# before any LLVM pass runs (PHALANGE_O0_PIPELINE=""), where it is most
# sensitive to differences.
#
#   selfhost/difftest.sh [compiler]     (default: selfhost/build/phalange)
set -u
cd "$(dirname "$0")/.."
NEW=${1:-selfhost/build/phalange}
OLD=compiler/build/phalange
T=$(mktemp -d)
trap 'rm -rf "$T"' EXIT
export PHALANGE_O0_PIPELINE=
pass=0
fail=0

same() {
    if [ "$1" = "$2" ]; then
        pass=$((pass + 1))
    else
        fail=$((fail + 1))
        echo "DIFF $3"
    fi
}

files=$(find tests std examples selfhost/src -name '*.ph' | sort)
for f in $files; do
    a=$($OLD check "$f" --format=agent-json 2>&1; echo "status $?")
    b=$($NEW check "$f" --format=agent-json 2>&1; echo "status $?")
    same "$a" "$b" "check $f"
done
for flags in "" "--target=native" "-O2" "--target=native -O2" "--release -O1" "--stack-traces=off" "--target=wasm64"; do
    for f in $files; do
        case $f in selfhost/*) continue ;; esac
        rm -f "$T/a.ll" "$T/b.ll"
        # shellcheck disable=SC2086
        a=$($OLD build "$f" --emit=llvm -o "$T/a.ll" $flags 2>&1; echo "status $?"; cat "$T/a.ll" 2>/dev/null)
        # shellcheck disable=SC2086
        b=$($NEW build "$f" --emit=llvm -o "$T/b.ll" $flags 2>&1; echo "status $?"; cat "$T/b.ll" 2>/dev/null)
        same "$a" "$b" "build $f $flags"
    done
done
echo "$pass identical, $fail different"
[ $fail -eq 0 ]
