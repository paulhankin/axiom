#!/bin/sh
# Bootstraps the self-hosted Phalange compiler.
#
#   stage 0  the C compiler (compiler/), built with make
#   stage 1  selfhost/src compiled by stage 0
#   stage 2  selfhost/src compiled by stage 1
#   stage 3  selfhost/src compiled by stage 2: must be bit-identical to stage 2
#
# The result is selfhost/build/phalange (stage 2). Environment:
#   LLVM_CONFIG   llvm-config to use (default: llvm-config)
#   OPT           optimization level for the stages (default: -O2)
set -eu
cd "$(dirname "$0")/.."

LLVM_CONFIG=${LLVM_CONFIG:-llvm-config}
OPT=${OPT:--O2}
LLVM_LIBS="-L$($LLVM_CONFIG --libdir) $($LLVM_CONFIG --libs)"
B=selfhost/build

stage() {
    echo "stage $3: $1 -> $2"
    start=$(date +%s)
    # shellcheck disable=SC2086
    "$1" build selfhost/src/main.ph --target=native $OPT -o "$2" $LLVM_LIBS
    echo "         $(($(date +%s) - start)) s, $(wc -c < "$2") bytes"
}

make -C compiler --no-print-directory
mkdir -p $B
stage compiler/build/phalange $B/phalange1 1
stage $B/phalange1 $B/phalange2 2
stage $B/phalange2 $B/phalange3 3

if ! cmp -s $B/phalange2 $B/phalange3; then
    echo "bootstrap FAILED: stage 2 and stage 3 differ (the self-hosted compiler is not at a fixed point)" >&2
    exit 1
fi
echo "fixed point: stage 2 and stage 3 are bit-identical"
if cmp -s $B/phalange1 $B/phalange2; then
    echo "stage 1 and stage 2 are bit-identical too: the C and Phalange compilers generate the same code"
fi
cp $B/phalange2 $B/phalange

# The compiler's own unit tests.
$B/phalange test selfhost/src/util.ph --target=native
echo "built $B/phalange"
