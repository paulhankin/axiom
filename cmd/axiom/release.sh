#!/bin/sh
# Release archives of the compiler (docs/releasing.md).
#
#   cmd/axiom/release.sh notes <version>   gather docs/next/*.md into docs/releases/<version>.md
#   cmd/axiom/release.sh build <version>   build and check build/release/axiom-<version>-*.{tar.gz,zip}
#                                          and build/release/SHA256SUMS
#
# build needs a clean tree whose compilers' version (cmd/axiom/src/version.ax,
# cmd/axiom/bootstrap/src/version.h) is <version>, and docs/releases/<version>.md.
# It bootstraps, runs the test suite with both compilers, builds the compiler
# for every release target, and packs each with the standard library (its
# sources, without test data), the licenses and the release notes. The
# archives are deterministic: the same commit gives the same bytes. Environment:
#   SKIP_TESTS   1: do not run the test suite (for trying the script out)
set -eu
cd "$(dirname "$0")/../.."

usage() {
    echo "usage: cmd/axiom/release.sh notes|build <version>" >&2
    exit 2
}
[ $# -eq 2 ] || usage
cmd=$1
v=$2
echo "$v" | grep -Eq '^[0-9]+\.[0-9]+\.[0-9]+$' || { echo "release: version '$v' is not a.b.c" >&2; exit 2; }

if [ "$cmd" = notes ]; then
    out=docs/releases/$v.md
    [ -e "$out" ] && { echo "release: $out exists" >&2; exit 1; }
    mkdir -p docs/releases
    {
        echo "# Axiom $v"
        for f in docs/next/*.md; do
            [ "$(basename "$f")" = README.md ] && continue
            echo
            cat "$f"
        done
    } > "$out"
    for f in docs/next/*.md; do
        [ "$(basename "$f")" = README.md ] || git rm -q "$f"
    done
    git add "$out"
    echo "wrote $out; edit it, then commit"
    exit 0
fi
[ "$cmd" = build ] || usage

# The version, the tree and the notes.
grep -q "pub let VERSION: \[\]u8 = \"$v\";" cmd/axiom/src/version.ax ||
    { echo "release: cmd/axiom/src/version.ax is not version $v" >&2; exit 1; }
grep -q "#define AXIOM_VERSION \"$v\"" cmd/axiom/bootstrap/src/version.h ||
    { echo "release: cmd/axiom/bootstrap/src/version.h is not version $v" >&2; exit 1; }
[ -f "docs/releases/$v.md" ] || { echo "release: no docs/releases/$v.md (release.sh notes $v)" >&2; exit 1; }
if [ -n "$(git status --porcelain)" ]; then
    echo "release: the tree has uncommitted changes" >&2
    exit 1
fi
commit=$(git rev-parse HEAD)
mtime=$(git log -1 --format=%ct)

cmd/axiom/bootstrap.sh
A=cmd/axiom/build/axiom
if [ "${SKIP_TESTS:-0}" != 1 ]; then
    node cmd/axiom/internal/tests/run.mjs
    AXIOM=$A node cmd/axiom/internal/tests/run.mjs
fi

R=build/release
rm -rf "$R"
mkdir -p "$R"
# os-arch, the compiler's --target, and the executable's name
for t in "linux-x86_64 x86_64 axiom" "linux-aarch64 aarch64 axiom" "macos-aarch64 aarch64-macos axiom" \
         "windows-x86_64 x86_64-windows axiom.exe"; do
    # shellcheck disable=SC2086
    set -- $t
    name=axiom-$v-$1
    d=$R/stage/$name
    mkdir -p "$d/bin"
    echo "build $name"
    $A build cmd/axiom/src/main.ax --target="$2" -O2 --pdb=off -o "$d/bin/$3"
    git ls-files std | grep -v '/testdata/' | while read -r f; do
        mkdir -p "$d/$(dirname "$f")"
        cp "$f" "$d/$f"
    done
    cp LICENSE-MIT LICENSE-APACHE THIRD-PARTY-NOTICES.md "$d/"
    cp "docs/releases/$v.md" "$d/RELEASE-NOTES.md"
    printf 'Axiom %s (%s), built from commit %s.\n\nPut bin/ on the PATH; the compiler finds std/ next to it. See RELEASE-NOTES.md\nand https://github.com/paulhankin/axiom.\n' \
        "$v" "$1" "$commit" > "$d/README.txt"
    python3 - "$R" "$R/stage" "$name" "$mtime" <<'EOF'
import gzip, io, os, sys, tarfile, zipfile, time
out, stage, name, mtime = sys.argv[1], sys.argv[2], sys.argv[3], int(sys.argv[4])
files = []
for dp, dn, fn in os.walk(os.path.join(stage, name)):
    dn.sort()
    for f in sorted(fn):
        files.append(os.path.relpath(os.path.join(dp, f), stage))
def mode(rel):
    return 0o755 if rel.split(os.sep)[1] == 'bin' else 0o644
if name.startswith('axiom-') and 'windows' in name:
    with zipfile.ZipFile(os.path.join(out, name + '.zip'), 'w', zipfile.ZIP_DEFLATED) as z:
        for rel in files:
            zi = zipfile.ZipInfo(rel.replace(os.sep, '/'), time.gmtime(max(mtime, 315532800))[:6])
            zi.external_attr = mode(rel) << 16
            zi.compress_type = zipfile.ZIP_DEFLATED
            z.writestr(zi, open(os.path.join(stage, rel), 'rb').read())
else:
    buf = io.BytesIO()
    with tarfile.open(fileobj=buf, mode='w', format=tarfile.PAX_FORMAT) as t:
        for rel in files:
            ti = t.gettarinfo(os.path.join(stage, rel), rel.replace(os.sep, '/'))
            ti.mtime, ti.uid, ti.gid, ti.uname, ti.gname, ti.mode = mtime, 0, 0, '', '', mode(rel)
            t.addfile(ti, open(os.path.join(stage, rel), 'rb'))
    with open(os.path.join(out, name + '.tar.gz'), 'wb') as f:
        with gzip.GzipFile(filename='', mode='wb', fileobj=f, mtime=0) as g:
            g.write(buf.getvalue())
EOF
done

# The Linux x86-64 archive, unpacked, builds and runs a program.
if [ "$(uname -s)-$(uname -m)" = Linux-x86_64 ]; then
    T=$R/stage/check
    mkdir -p "$T"
    tar -xzf "$R/axiom-$v-linux-x86_64.tar.gz" -C "$T"
    printf 'import "std/io" as io;\n\nexport fn main() i32 {\n    io.stdout().write_all("ok\\n") catch return 1;\n    return 0;\n}\n' > "$T/hello.ax"
    out=$(cd "$T" && "./axiom-$v-linux-x86_64/bin/axiom" build hello.ax --target=native -o hello && ./hello)
    [ "$out" = ok ] || { echo "release: the unpacked compiler does not work: $out" >&2; exit 1; }
    [ "$("$T/axiom-$v-linux-x86_64/bin/axiom" version)" = "axiom $v" ] || { echo "release: wrong version" >&2; exit 1; }
fi
rm -rf "$R/stage"
(cd "$R" && sha256sum axiom-"$v"-* > SHA256SUMS)
cat "$R/SHA256SUMS"
echo "built $R for $v (commit $commit)"
