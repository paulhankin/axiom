#!/bin/sh
# Bootstraps the Axiom compiler (cmd/axiom/) from a seed: an Axiom compiler
# that already exists (docs/bootstrap-chain.md).
#
#   stage 0  the seed, the first of these that there is:
#              - $AXIOM_SEED, a compiler on this machine;
#              - the release that cmd/axiom/seed.toml pins, downloaded once
#                into the cache and checked against the hash pinned there;
#              - the bootstrap compiler (cmd/axiom/bootstrap/), in C, built
#                with make (it uses LLVM): the way until a release that
#                builds this tree is pinned
#   stage 1  cmd/axiom/src compiled by stage 0
#   stage 2  cmd/axiom/src compiled by stage 1 (its own backends and linker)
#   stage 3  cmd/axiom/src compiled by stage 2: must be bit-identical to stage 2
#
# The result is cmd/axiom/build/axiom (stage 2), which needs nothing but
# the C library: no LLVM, no C compiler, no linker. It does not depend on
# the seed: a seed that compiles the sources correctly gives the same stage 2
# as any other. Environment:
#   AXIOM_SEED  a compiler to use as stage 0 (it must accept the language the
#           compiler's sources are written in: a build of a recent commit, or
#           the pinned release)
#   SEED_TOML   the file that pins the seed release (default
#           cmd/axiom/seed.toml; a file that does not exist pins none)
#   AXIOM_UPDATE_URL  where releases are downloaded from (default
#           https://github.com/paulhankin/axiom; `axiom update` reads it too)
#   OPT     optimization level for stages 2 and 3 (default: -O2)
#   OPT1    optimization level for stage 1 (default: -O0 with the bootstrap
#           compiler, where LLVM takes a minute at -O2, and OPT with a seed;
#           stage 2 comes out the same either way)
#   TARGET  the compiler's target (default: the generic processor of this
#           machine's architecture, so the binary runs on any machine of its
#           kind; TARGET=native tunes it to this processor's extensions, and
#           it then runs only where they exist)
#   CROSS   1: also check that stage 2 built for the other architecture (run
#           with qemu) generates the same code as stage 2 itself
set -eu
cd "$(dirname "$0")/../.."

OPT=${OPT:--O2}
CROSS=${CROSS:-0}
B=cmd/axiom/build
if [ -z "${TARGET:-}" ]; then
    case "$(uname -s)-$(uname -m)" in
        Darwin-*) TARGET=aarch64-macos ;;
        *-x86_64) TARGET=x86_64 ;;
        *-aarch64 | *-arm64) TARGET=aarch64 ;;
        *) TARGET=native ;;
    esac
fi

# The release that $SEED_TOML pins, for this platform: downloads its archive
# (once: it is kept in the cache), checks it against the pinned hash and
# unpacks it. Sets SEED to its compiler.
pinned_seed() {
    toml=$1
    version=$(sed -n 's/^version *= *"\([0-9][0-9.]*\)" *$/\1/p' "$toml")
    case "$(uname -s)-$(uname -m)" in
        Linux-x86_64) plat=linux-x86_64 ;;
        Linux-aarch64 | Linux-arm64) plat=linux-aarch64 ;;
        Darwin-arm64) plat=macos-aarch64 ;;
        *) echo "bootstrap: no release for $(uname -s) $(uname -m): set AXIOM_SEED to a compiler" >&2; exit 1 ;;
    esac
    want=$(sed -n "s/^$plat *= *\"\([0-9a-f]\{64\}\)\" *\$/\1/p" "$toml")
    if [ -z "$version" ] || [ -z "$want" ]; then
        echo "bootstrap: $toml pins no version, or no hash for $plat" >&2
        exit 1
    fi
    cache=${AXIOM_CACHE:-${XDG_CACHE_HOME:-$HOME/.cache}/axiom}/seed
    name=axiom-$version-$plat
    SEED=$cache/$want/$name/bin/axiom
    [ -x "$SEED" ] && return 0
    url=${AXIOM_UPDATE_URL:-https://github.com/paulhankin/axiom}/releases/download/v$version/$name.tar.gz
    echo "seed: $url"
    tmp=$cache/$want.tmp
    rm -rf "$tmp"
    mkdir -p "$tmp"
    curl -fsSL --proto '=https,file' --proto-redir '=https' -o "$tmp/$name.tar.gz" "$url" ||
        { echo "bootstrap: cannot download the seed (set AXIOM_SEED to a compiler to do without)" >&2; exit 1; }
    if command -v sha256sum >/dev/null; then
        got=$(sha256sum "$tmp/$name.tar.gz" | cut -d' ' -f1)
    else
        got=$(shasum -a 256 "$tmp/$name.tar.gz" | cut -d' ' -f1)
    fi
    if [ "$got" != "$want" ]; then
        echo "bootstrap: the seed's hash is $got, and $toml pins $want" >&2
        rm -rf "$tmp"
        exit 1
    fi
    tar -xzf "$tmp/$name.tar.gz" -C "$tmp"
    rm -f "$tmp/$name.tar.gz"
    [ -x "$tmp/$name/bin/axiom" ] || { echo "bootstrap: the seed's archive has no $name/bin/axiom" >&2; exit 1; }
    rm -rf "${cache:?}/$want"
    mv "$tmp" "$cache/$want"
}

SEED=${AXIOM_SEED:-}
SEED_TOML=${SEED_TOML:-cmd/axiom/seed.toml}
if [ -n "$SEED" ]; then
    [ -x "$SEED" ] || { echo "bootstrap: AXIOM_SEED=$SEED is not a program" >&2; exit 1; }
elif [ -f "$SEED_TOML" ]; then
    pinned_seed "$SEED_TOML"
fi
if [ -n "$SEED" ]; then
    OPT1=${OPT1:-$OPT}
else
    OPT1=${OPT1:--O0}
fi

stage() {
    echo "stage $3: $1 -> $2"
    start=$(date +%s)
    target=$TARGET
    std=
    if [ "$3" = 1 ]; then
        if [ -n "$SEED" ]; then
            # (a release looks for the standard library beside itself: this
            # tree's is the one to compile)
            std=--std=std
        else
            # (the bootstrap compiler knows only --target=native)
            target=native
        fi
    fi
    # shellcheck disable=SC2086
    if ! "$1" build cmd/axiom/src/main.ax --target="$target" ${4:-$OPT} $std -o "$2"; then
        [ "$3" = 1 ] && [ -n "$SEED" ] &&
            echo "bootstrap: the seed ($("$SEED" version)) does not build these sources: it must accept the language they are written in" >&2
        exit 1
    fi
    echo "         $(($(date +%s) - start)) s, $(wc -c < "$2") bytes"
}

mkdir -p $B
if [ -n "$SEED" ]; then
    echo "seed: $SEED ($("$SEED" version))"
    stage "$SEED" $B/axiom1 1 "$OPT1"
elif [ "$(uname -s)" = Darwin ]; then
    make -C cmd/axiom/bootstrap --no-print-directory
    # The bootstrap compiler does not build native Mac programs, but it
    # builds WebAssembly: stage 1 is the compiler for wasm32,
    # run under node, building itself for the Mac (by name: under node it
    # cannot tell that it is on one).
    echo "stage 1: cmd/axiom/bootstrap/build/axiom -> $B/axiom1.wasm (node) -> $B/axiom1"
    start=$(date +%s)
    # shellcheck disable=SC2086
    cmd/axiom/bootstrap/build/axiom build cmd/axiom/src/main.ax --target=wasm32 $OPT1 -o $B/axiom1.wasm
    # shellcheck disable=SC2086
    node std/internal/runtime/host.mjs $B/axiom1.wasm -- build cmd/axiom/src/main.ax --target=aarch64-macos $OPT1 -o $B/axiom1
    echo "         $(($(date +%s) - start)) s, $(wc -c < "$B/axiom1") bytes"
else
    make -C cmd/axiom/bootstrap --no-print-directory
    stage cmd/axiom/bootstrap/build/axiom $B/axiom1 1 "$OPT1"
fi
# (stages 2 and 3 have the same file name: a Mac executable's code
# signature holds it)
mkdir -p $B/stage2 $B/stage3
stage $B/axiom1 $B/stage2/axiom 2
stage $B/stage2/axiom $B/stage3/axiom 3

if ! cmp -s $B/stage2/axiom $B/stage3/axiom; then
    echo "bootstrap FAILED: stage 2 and stage 3 differ (the compiler is not at a fixed point)" >&2
    exit 1
fi
echo "fixed point: stage 2 and stage 3 are bit-identical"
cp $B/stage2/axiom $B/axiom2
rm -f $B/axiom
cp $B/stage2/axiom $B/axiom

if [ "$CROSS" = 1 ]; then
    # The compiler built for the other architecture must generate the same
    # code as the native one.
    host=$(uname -m)
    other=aarch64
    [ "$host" = aarch64 ] && other=x86_64
    echo "cross: stage 2 for $other"
    $B/axiom build cmd/axiom/src/main.ax --target=$other $OPT -o $B/axiom-$other
    # (explicit targets: --target=native would use this machine's extensions)
    $B/axiom build cmd/axiom/src/main.ax --target=$host $OPT -o $B/axiom-generic
    qemu-$other -L /usr/$other-linux-gnu $B/axiom-$other build cmd/axiom/src/main.ax --target=$host $OPT -o $B/axiom-cross
    if ! cmp -s $B/axiom-generic $B/axiom-cross; then
        echo "bootstrap FAILED: the $other-hosted compiler generates different code" >&2
        exit 1
    fi
    echo "cross: the $other-hosted compiler generates the same code, bit for bit"
fi

# The compiler's own unit tests.
$B/axiom test cmd/axiom/src/util.ax --target=native
echo "built $B/axiom (--target=$TARGET)"
