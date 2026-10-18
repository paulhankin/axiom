#!/bin/sh
# Differential fuzzing of the self-hosted compiler's backends against the C
# compiler (stage 0, LLVM): gen.py writes a program that applies every
# integer, float and SIMD operation to edge values and prints the results;
# each backend's output must match the reference native build exactly.
#
#   cmd/axiom/fuzz/run.sh [first seed] [last seed]    (default 1 5)
#
# Environment: NEW (default cmd/axiom/build/axiom), OLD (default
# cmd/axiom/bootstrap/build/axiom). AArch64 runs need qemu-aarch64, the AArch64 C
# library in /usr/aarch64-linux-gnu and aarch64-linux-gnu-gcc (for the test's
# C host functions); they are skipped otherwise.
set -u
cd "$(dirname "$0")/../../.."
NEW=${NEW:-cmd/axiom/build/axiom}
OLD=${OLD:-cmd/axiom/bootstrap/build/axiom}
T=$(mktemp -d)
trap 'rm -rf "$T"' EXIT
RT=cmd/axiom/internal/tests/native_rt.c
fail=0
arm=0
if command -v qemu-aarch64 >/dev/null && [ -d /usr/aarch64-linux-gnu ] && command -v aarch64-linux-gnu-gcc >/dev/null; then
    arm=1
fi

check() {  # name, output file
    if cmp -s "$T/ref.txt" "$2"; then
        echo "  $1: ok"
    else
        echo "  $1: DIFFERENT ($(cmp "$T/ref.txt" "$2" 2>&1 | head -1))"
        fail=$((fail + 1))
    fi
}

seed=${1:-1}
last=${2:-5}
while [ "$seed" -le "$last" ]; do
    echo "seed $seed"
    f=$T/f$seed.ax
    python3 cmd/axiom/fuzz/gen.py "$seed" > "$f"
    $OLD build "$f" --target=native --link=$RT -o "$T/ref" >/dev/null 2>&1 || { echo "  reference build failed"; exit 1; }
    "$T/ref" > "$T/ref.txt" 2>&1
    if $NEW build "$f" -o "$T/w.wasm" > "$T/log" 2>&1; then
        node cmd/axiom/fuzz/runw.mjs "$T/w.wasm" > "$T/w.txt" 2>&1
        check wasm32 "$T/w.txt"
    else
        echo "  wasm32: build failed"; fail=$((fail + 1))
    fi
    for cfg in "x86-64:--cpu=x86-64" "x86-64-v2:--cpu=x86-64-v2" "x86-64-v3:--cpu=x86-64-v3" "x86-64-O2:--cpu=x86-64-v3 -O2" \
               "x86-64-v4:--cpu=x86-64-v4" "x86-64-v4-O2:--cpu=x86-64-v4 -O2" \
               "avx512bw:--cpu=x86-64-v3 --features=+avx512bw"; do
        name=${cfg%%:*}
        # only what this processor runs: the x86-64 configurations on an x86-64
        # host, and the AVX-512 ones where it has AVX-512BW
        [ "$(uname -m)" = x86_64 ] || continue
        case $name in
            x86-64-v4* | avx512bw) grep -q avx512bw /proc/cpuinfo 2>/dev/null || { echo "  $name: skipped (no AVX-512 here)"; continue; } ;;
        esac
        # shellcheck disable=SC2086
        if $NEW build "$f" --target=x86_64 ${cfg#*:} --link=$RT -o "$T/x" > "$T/log" 2>&1; then
            "$T/x" > "$T/x.txt" 2>&1
            check "$name" "$T/x.txt"
        else
            echo "  $name: build failed"; fail=$((fail + 1))
        fi
    done
    if [ $arm -eq 1 ]; then
        for cfg in "aarch64:--cpu=generic" "aarch64-lse:--features=+lse" "aarch64-O2:-O2"; do
            name=${cfg%%:*}
            # shellcheck disable=SC2086
            if $NEW build "$f" --target=aarch64 ${cfg#*:} --link=$RT -o "$T/a" > "$T/log" 2>&1; then
                qemu-aarch64 -L /usr/aarch64-linux-gnu "$T/a" > "$T/a.txt" 2>&1
                check "$name" "$T/a.txt"
            else
                echo "  $name: build failed"; fail=$((fail + 1))
            fi
        done
    fi
    seed=$((seed + 1))
done
[ $fail -eq 0 ] && echo "all outputs identical" || { echo "$fail differences"; exit 1; }
