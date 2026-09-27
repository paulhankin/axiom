#!/bin/sh
# Measures the quality of the self-hosted compiler's generated code against
# LLVM's:
#   - the compiler built by LLVM (stage 1) and the compiler built by itself
#     (stage 2) have the same source, so the ratio of their running times on
#     the same job is the speed of our code relative to LLVM's;
#   - the programs in selfhost/bench/ (loops over arrays, hashing, sorting,
#     floating point) built by the C compiler (LLVM) and by stage 2, -O2.
# Needs selfhost/bootstrap.sh.
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

for f in selfhost/bench/*.ph; do
    n=$(basename "$f" .ph)
    [ "$n" = common ] && continue
    compiler/build/phalange build "$f" --target=native -O2 -o "$T/llvm_$n"
    $B/phalange2 build "$f" --target=native -O2 -o "$T/self_$n"
    if [ "$("$T/llvm_$n")" != "$("$T/self_$n")" ]; then
        echo "$n: DIFFERENT OUTPUT" >&2
        exit 1
    fi
    a=$(best "$T/llvm_$n")
    b=$(best "$T/self_$n")
    echo "bench $n: llvm ${a} ms, self ${b} ms, ratio $(echo "scale=2; $b / $a" | bc)"
done
