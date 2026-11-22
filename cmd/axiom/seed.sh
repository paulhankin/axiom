#!/bin/sh
# Makes a seed for bootstrap.sh from this commit (docs/bootstrap-chain.md):
# the compiler alone, for each platform the compiler is built on, packed as a
# release's archives are but without the standard library (bootstrap.sh
# compiles the tree's own). The archives are deterministic, as releases are.
#
#   cmd/axiom/seed.sh <0.N.M-seed.K> [--publish]
#
# It bootstraps, runs the test suite with the compiler, builds
# build/seed/axiom-<name>-<os>-<arch>.tar.gz and build/seed/SHA256SUMS, and
# prints the cmd/axiom/seed.toml that pins them. --publish then makes the
# GitHub pre-release v<name> of this commit with them (it needs gh, logged
# in); a pre-release is not one `axiom update` offers. Environment:
#   SKIP_TESTS   1: do not run the test suite
set -eu
cd "$(dirname "$0")/../.."

[ $# -ge 1 ] || { echo "usage: cmd/axiom/seed.sh <0.N.M-seed.K> [--publish]" >&2; exit 2; }
v=$1
echo "$v" | grep -Eq '^[0-9]+\.[0-9]+\.[0-9]+-seed\.[0-9]+$' ||
    { echo "seed: a seed is named 0.N.M-seed.K, not '$v'" >&2; exit 2; }
if [ -n "$(git status --porcelain)" ]; then
    echo "seed: the tree has uncommitted changes" >&2
    exit 1
fi
commit=$(git rev-parse HEAD)
mtime=$(git log -1 --format=%ct)

cmd/axiom/bootstrap.sh
A=cmd/axiom/build/axiom
[ "${SKIP_TESTS:-0}" = 1 ] || AXIOM=$A node cmd/axiom/internal/tests/run.mjs

R=build/seed
rm -rf "$R"
mkdir -p "$R"
# os-arch and the compiler's --target
for t in "linux-x86_64 x86_64" "linux-aarch64 aarch64" "macos-aarch64 aarch64-macos"; do
    # shellcheck disable=SC2086
    set -- $t
    name=axiom-$v-$1
    d=$R/stage/$name
    mkdir -p "$d/bin"
    echo "build $name"
    $A build cmd/axiom/src/main.ax --target="$2" -O2 -o "$d/bin/axiom"
    cp LICENSE-MIT LICENSE-APACHE THIRD-PARTY-NOTICES.md "$d/"
    printf 'A seed for building Axiom from source (%s), built from commit %s:\nthe compiler without the standard library. See docs/bootstrap-chain.md in\nhttps://github.com/paulhankin/axiom. It is not a release.\n\nAxiom is experimental software, provided "as is", without warranty of any kind.\nUse it at your own risk; see LICENSE-MIT and LICENSE-APACHE for the full terms.\n' \
        "$1" "$commit" > "$d/README.txt"
    python3 - "$R" "$R/stage" "$name" "$mtime" <<'EOF'
import gzip, io, os, sys, tarfile
out, stage, name, mtime = sys.argv[1], sys.argv[2], sys.argv[3], int(sys.argv[4])
files = []
for dp, dn, fn in os.walk(os.path.join(stage, name)):
    dn.sort()
    for f in sorted(fn):
        files.append(os.path.relpath(os.path.join(dp, f), stage))
buf = io.BytesIO()
with tarfile.open(fileobj=buf, mode='w', format=tarfile.PAX_FORMAT) as t:
    for rel in files:
        ti = t.gettarinfo(os.path.join(stage, rel), rel.replace(os.sep, '/'))
        ti.mtime, ti.uid, ti.gid, ti.uname, ti.gname = mtime, 0, 0, '', ''
        ti.mode = 0o755 if rel.split(os.sep)[1] == 'bin' else 0o644
        t.addfile(ti, open(os.path.join(stage, rel), 'rb'))
with open(os.path.join(out, name + '.tar.gz'), 'wb') as f:
    with gzip.GzipFile(filename='', mode='wb', fileobj=f, mtime=0) as g:
        g.write(buf.getvalue())
EOF
done
rm -rf "$R/stage"
if command -v sha256sum >/dev/null; then
    (cd "$R" && sha256sum axiom-"$v"-*.tar.gz > SHA256SUMS)
else
    (cd "$R" && shasum -a 256 axiom-"$v"-*.tar.gz > SHA256SUMS)
fi

echo "seed $v of commit $commit, in $R; cmd/axiom/seed.toml for it:"
echo
echo "[seed]"
echo "version = \"$v\""
sed "s/^\([0-9a-f]*\)  axiom-$v-\(.*\)\.tar\.gz\$/\2 = \"\1\"/" "$R/SHA256SUMS"

if [ "${2:-}" = --publish ]; then
    gh release create "v$v" --target "$commit" --prerelease --title "Seed $v" \
        --notes "A seed for building Axiom from source (docs/bootstrap-chain.md): the compiler of commit $commit, without the standard library. It is not a release." \
        "$R"/axiom-"$v"-*.tar.gz "$R/SHA256SUMS"
fi
