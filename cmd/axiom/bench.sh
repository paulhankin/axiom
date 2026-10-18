#!/bin/sh
# Measures the quality of the compiler's generated code against
# LLVM's:
#   - the compiler built by the bootstrap compiler (LLVM, -O2) and the compiler
#     built by itself (stage 2) have the same source, so the ratio of their
#     running times on the same job is the speed of our code relative to
#     LLVM's. (bootstrap.sh builds stage 1 at -O0 to save a minute, so this
#     script builds its own LLVM reference at -O2.)
#   - the programs in cmd/axiom/bench/ (loops over arrays, hashing, sorting,
#     floating point) built by the bootstrap compiler (LLVM) and by stage 2, -O2.
# Needs cmd/axiom/bootstrap.sh, and a Linux x86-64 or AArch64 machine (the
# bootstrap compiler builds native code there only).
#
#   cmd/axiom/bench.sh [runs]      (default 3; the best run counts)
set -eu
cd "$(dirname "$0")/../.."
RUNS=${1:-3}
B=cmd/axiom/build
C=cmd/axiom/bootstrap/build/axiom
T=$(mktemp -d)
trap 'rm -rf "$T"' EXIT

now_ms() { node -e 'console.log(Date.now())'; }
ratio() { awk "BEGIN { printf \"%.2f\", $2 / $1 }"; }
best() {
    b=
    i=0
    while [ $i -lt "$RUNS" ]; do
        s=$(now_ms)
        "$@" > /dev/null
        e=$(( $(now_ms) - s ))
        if [ -z "$b" ] || [ $e -lt $b ]; then b=$e; fi
        i=$((i + 1))
    done
    echo $b
}

echo "building the LLVM reference compiler at -O2 (about a minute)"
$C build cmd/axiom/src/main.ax --target=native -O2 -o $T/llvm
for job in "-O0 --units=program" "-O2"; do
    # shellcheck disable=SC2086
    a=$(best $T/llvm build cmd/axiom/src/main.ax --target=native $job -o $T/x)
    # shellcheck disable=SC2086
    b=$(best $B/axiom2 build cmd/axiom/src/main.ax --target=native $job -o $T/y)
    echo "compile the compiler $job: llvm-built ${a} ms, self-built ${b} ms, ratio $(ratio $a $b)"
done
echo "size: llvm-built $(wc -c < $T/llvm) bytes, self-built $(wc -c < $B/axiom2) bytes"

for f in cmd/axiom/bench/*.ax; do
    n=$(basename "$f" .ax)
    [ "$n" = common ] && continue
    $C build "$f" --target=native -O2 -o "$T/llvm_$n"
    $B/axiom2 build "$f" --target=native -O2 -o "$T/self_$n"
    if [ "$("$T/llvm_$n")" != "$("$T/self_$n")" ]; then
        echo "$n: DIFFERENT OUTPUT" >&2
        exit 1
    fi
    a=$(best "$T/llvm_$n")
    b=$(best "$T/self_$n")
    echo "bench $n: llvm ${a} ms, self ${b} ms, ratio $(ratio $a $b)"
done
