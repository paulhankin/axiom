#!/bin/sh
# Measures the code of the compiler just built (cmd/axiom/build/axiom2,
# stage 2 of bootstrap.sh) against that of another build of the compiler,
# BASE: an earlier commit's, a release's or the seed's.
#   - Each compiles the compiler's sources into a compiler of its own; the
#     two results do the same job (compiling the sources again), so the ratio
#     of their running times is the speed of this build's code relative to
#     BASE's.
#   - The programs in cmd/axiom/bench/ (loops over arrays, hashing, sorting,
#     floating point, the allocator: alloc_*, some with threads) are built by
#     both at -O2; their outputs must be the same.
# Needs cmd/axiom/bootstrap.sh, and BASE to accept the language the sources
# are written in. cmd/axiom/bench/layouts.py compares the code of two builds
# too, on these programs and the C ones in cmd/axiom/bench/c/, over several
# code layouts.
#
#   BASE=<compiler> cmd/axiom/bench.sh [runs]      (default 3; the best run counts)
set -eu
cd "$(dirname "$0")/../.."
RUNS=${1:-3}
B=cmd/axiom/build
[ -n "${BASE:-}" ] && [ -x "$BASE" ] || { echo "bench: set BASE to the compiler to compare with" >&2; exit 2; }
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

echo "building the compiler with each"
"$BASE" build cmd/axiom/src/main.ax --std=std --target=native -O2 -o $T/base
$B/axiom2 build cmd/axiom/src/main.ax --target=native -O2 -o $T/self
for job in "-O0 --units=program" "-O2"; do
    # shellcheck disable=SC2086
    a=$(best $T/base build cmd/axiom/src/main.ax --target=native $job -o $T/x)
    # shellcheck disable=SC2086
    b=$(best $T/self build cmd/axiom/src/main.ax --target=native $job -o $T/y)
    echo "compile the compiler $job: base-built ${a} ms, self-built ${b} ms, ratio $(ratio $a $b)"
done
echo "size: base-built $(wc -c < $T/base) bytes, self-built $(wc -c < $T/self) bytes"

for f in cmd/axiom/bench/*.ax; do
    n=$(basename "$f" .ax)
    [ "$n" = common ] && continue
    "$BASE" build "$f" --std=std --target=native -O2 -o "$T/base_$n"
    $B/axiom2 build "$f" --target=native -O2 -o "$T/self_$n"
    if [ "$("$T/base_$n")" != "$("$T/self_$n")" ]; then
        echo "$n: DIFFERENT OUTPUT" >&2
        exit 1
    fi
    a=$(best "$T/base_$n")
    b=$(best "$T/self_$n")
    echo "bench $n: base ${a} ms, self ${b} ms, ratio $(ratio $a $b)"
done
