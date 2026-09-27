#!/bin/sh
# Measures the quality of the self-hosted compiler's generated code: the
# compiler built by LLVM (stage 1) and the compiler built by itself (stage 2)
# have the same source, so the ratio of their running times on the same job
# is the speed of our code relative to LLVM's. Needs selfhost/bootstrap.sh.
#
#   selfhost/bench.sh [runs]      (default 3; the best run counts)
set -eu
cd "$(dirname "$0")/.."
RUNS=${1:-3}
B=selfhost/build
T=$(mktemp -d)
trap 'rm -rf "$T"' EXIT

best() {
    b=
    i=0
    while [ $i -lt "$RUNS" ]; do
        s=$(date +%s%N)
        "$@" > /dev/null
        e=$(( ($(date +%s%N) - s) / 1000000 ))
        if [ -z "$b" ] || [ $e -lt $b ]; then b=$e; fi
        i=$((i + 1))
    done
    echo $b
}

for job in "-O0 --units=program" "-O2"; do
    # shellcheck disable=SC2086
    a=$(best $B/phalange1 build selfhost/src/main.ph --target=native $job -o $T/x)
    # shellcheck disable=SC2086
    b=$(best $B/phalange2 build selfhost/src/main.ph --target=native $job -o $T/y)
    echo "compile selfhost $job: llvm-built ${a} ms, self-built ${b} ms, ratio $(echo "scale=2; $b / $a" | bc)"
done
echo "size: llvm-built $(wc -c < $B/phalange1) bytes, self-built $(wc -c < $B/phalange2) bytes"
