#!/bin/sh
# Releases of the compiler (docs/releasing.md).
#
#   cmd/axiom/release.sh release <version> [options]      a release 0.N.0, from main
#   cmd/axiom/release.sh fix <version> [<commit>...] [options]
#                                                          a fix release 0.N.M (M > 0) from
#                                                          release-0.N, with the commits (from
#                                                          main) cherry-picked onto it first
#   cmd/axiom/release.sh notes <version>   gather docs/next/*.md into docs/releases/<version>.md
#   cmd/axiom/release.sh build <version>   build and check build/release/axiom-<version>-*.{tar.gz,zip}
#                                          and build/release/SHA256SUMS
#
# release and fix do every step of docs/releasing.md: set the version, gather
# the notes (and open them in $EDITOR, when there is a terminal), commit,
# build and test, then, once asked, tag the commit, push the branch and the
# tag, and create the GitHub release with the notes and the archives. After a
# release 0.N.0 they create release-0.N and set main to the next version;
# after a fix release they add its notes to main's docs/releases. Each step
# that is done already is skipped, so after a failure (a cherry-pick that
# conflicts, a test that fails) fix it, commit, and run the same command
# again. They need git with push access, gh (logged in, `gh auth login`) and
# what build needs. Options:
#   --yes          do not ask before publishing
#   --dry-run      do the local steps (the commit, the build and tests), then
#                  print the rest instead of doing it; run again without
#                  --dry-run to publish what was built
#   --draft        create the GitHub release as a draft
#   --advisory <file.json>
#                  also create a draft security advisory (the JSON body of
#                  GitHub's "create a repository security advisory" API)
#
# build needs a clean tree whose compilers' version (cmd/axiom/src/version.ax,
# cmd/axiom/bootstrap/src/version.h) is <version>, and docs/releases/<version>.md.
# It bootstraps, runs the test suite with both compilers, builds the compiler
# for every release target, and packs each with the standard library (its
# sources, without test data), the licenses and the release notes. The
# archives are deterministic: the same commit gives the same bytes. Environment:
#   SKIP_TESTS   1: do not run the test suite (for trying the script out)
set -eu
cd "${RELEASE_SH_ROOT:-$(dirname "$0")/../..}"

usage() {
    echo "usage: cmd/axiom/release.sh release|fix|notes|build <version> (see the comment at its top)" >&2
    exit 2
}
[ $# -ge 2 ] || usage
cmd=$1
v=$2
echo "$v" | grep -Eq '^[0-9]+\.[0-9]+\.[0-9]+$' || { echo "release: version '$v' is not a.b.c" >&2; exit 2; }

if [ "$cmd" = release ] || [ "$cmd" = fix ]; then
    # (this checks out other branches, whose copy of this script differs: run
    # from a copy of it, and run their own notes and build)
    if [ -z "${RELEASE_SH_COPY:-}" ]; then
        copy=$(mktemp "${TMPDIR:-/tmp}/release.XXXXXX")
        cp "$0" "$copy"
        RELEASE_SH_COPY=$copy RELEASE_SH_ROOT=$(pwd) exec sh "$copy" "$@"
    fi
    trap 'rm -f "$RELEASE_SH_COPY"' EXIT
    shift 2
    yes=0
    dry=0
    draft=""
    advisory=""
    picks=""
    while [ $# -gt 0 ]; do
        case $1 in
            --yes) yes=1 ;;
            --dry-run) dry=1 ;;
            --draft) draft=--draft ;;
            --advisory)
                [ $# -ge 2 ] || usage
                advisory=$(cd "$(dirname "$2")" && pwd)/$(basename "$2")
                [ -f "$advisory" ] || { echo "release: no advisory file $2" >&2; exit 2; }
                shift
                ;;
            -*) usage ;;
            *)
                [ "$cmd" = fix ] || usage
                picks="$picks $1"
                ;;
        esac
        shift
    done
    say() { echo "release: $*"; }
    die() { echo "release: $*" >&2; exit 1; }
    commit_all() {
        git commit -q -m "$1" || die "nothing to commit for '$1'"
    }
    set_version() {
        sed -i.bak "s/pub let VERSION: \[\]u8 = \"[0-9.]*\";/pub let VERSION: []u8 = \"$1\";/" cmd/axiom/src/version.ax
        sed -i.bak "s/#define AXIOM_VERSION \"[0-9.]*\"/#define AXIOM_VERSION \"$1\"/" cmd/axiom/bootstrap/src/version.h
        rm -f cmd/axiom/src/version.ax.bak cmd/axiom/bootstrap/src/version.h.bak
        grep -q "\"$1\"" cmd/axiom/src/version.ax && grep -q "\"$1\"" cmd/axiom/bootstrap/src/version.h ||
            die "could not set the version to $1"
        git add cmd/axiom/src/version.ax cmd/axiom/bootstrap/src/version.h
    }
    current_version() {
        sed -n 's/^pub let VERSION: \[\]u8 = "\([0-9.]*\)";/\1/p' cmd/axiom/src/version.ax
    }
    major=${v%%.*}
    rest=${v#*.}
    minor=${rest%%.*}
    patch=${rest#*.}
    series=$major.$minor
    branch=release-$series
    tag=v$v

    # Before anything: the tools, the tree, and a release not made already.
    command -v gh >/dev/null || die "gh is not installed (https://cli.github.com)"
    if [ "$dry" = 0 ]; then
        gh auth status >/dev/null 2>&1 || die "gh is not logged in (gh auth login)"
        if gh release view "$tag" >/dev/null 2>&1; then
            die "the GitHub release $tag exists already"
        fi
    fi
    [ -z "$(git status --porcelain)" ] || die "the tree has uncommitted changes"
    git fetch -q origin main
    start=$(git rev-parse --abbrev-ref HEAD)

    # The branch and the version.
    if [ "$cmd" = release ]; then
        [ "$patch" = 0 ] || die "a release is a.b.0; use fix for $v"
        [ "$start" = main ] || die "a release is made from main (this is $start)"
        [ "$(git rev-list --count HEAD..origin/main)" = 0 ] || die "main is behind origin/main (git pull)"
        [ "$(current_version)" = "$v" ] || [ "$(git tag --points-at HEAD)" = "$tag" ] ||
            die "main is version $(current_version), not $v"
    else
        [ "$patch" != 0 ] || die "a fix release is a.b.M with M > 0; use release for $v"
        if git rev-parse -q --verify "refs/heads/$branch" >/dev/null; then
            git checkout -q "$branch"
        elif git ls-remote --exit-code --heads origin "$branch" >/dev/null; then
            git fetch -q origin "$branch"
            git checkout -q -b "$branch" "origin/$branch"
        else
            prev=v$series.$((patch - 1))
            git rev-parse -q --verify "refs/tags/$prev" >/dev/null || git fetch -q origin "refs/tags/$prev:refs/tags/$prev" ||
                die "no $branch and no tag $prev to make it from"
            say "making $branch from $prev"
            git checkout -q -b "$branch" "$prev"
        fi
        for c in $picks; do
            if git log --format=%B origin/main..HEAD | grep -q "cherry picked from commit $(git rev-parse "$c")"; then
                say "$c is on $branch already"
                continue
            fi
            say "cherry-picking $c"
            git cherry-pick -x "$c" ||
                die "the cherry-pick of $c conflicts: resolve it, git cherry-pick --continue, then run this again (without the commits already picked)"
        done
    fi

    # The version and the notes, committed as "Axiom <version>".
    if ! git rev-parse -q --verify "refs/tags/$tag" >/dev/null; then
        if [ "$(current_version)" != "$v" ]; then
            set_version "$v"
        fi
        if [ ! -f "docs/releases/$v.md" ]; then
            ls docs/next/*.md 2>/dev/null | grep -qv README.md || die "docs/next has no notes for $v"
            sh cmd/axiom/release.sh notes "$v"
            if [ -t 0 ] && [ -t 1 ] && [ -n "${EDITOR:-}" ]; then
                $EDITOR "docs/releases/$v.md"
            fi
            git add "docs/releases/$v.md"
        fi
        if [ -n "$(git status --porcelain)" ]; then
            commit_all "Axiom $v"
        fi
    fi
    commit=$(git rev-parse HEAD)

    # Build and test, unless this commit's archives are built already.
    if [ "$(cat build/release.commit 2>/dev/null)" != "$commit" ] || [ ! -f build/release/SHA256SUMS ]; then
        sh cmd/axiom/release.sh build "$v"
        echo "$commit" > build/release.commit
    fi
    grep -q "axiom-$v-" build/release/SHA256SUMS || die "build/release holds no archives for $v"

    # Publish.
    if [ "$dry" = 1 ]; then
        say "dry run: built $v at $(git rev-parse --short HEAD) on $(git rev-parse --abbrev-ref HEAD); still to do:"
        say "  tag $tag, push $(git rev-parse --abbrev-ref HEAD) and $tag, create the GitHub release$([ -n "$draft" ] && echo ' (a draft)')"
        [ -n "$advisory" ] && say "  create the draft security advisory from $advisory"
        if [ "$cmd" = release ]; then
            say "  push $branch, set main to $major.$((minor + 1)).0 and push it"
        else
            say "  add docs/releases/$v.md to main and push it"
        fi
        say "run the same command without --dry-run to do them"
        git checkout -q "$start"
        exit 0
    fi
    if [ "$yes" = 0 ]; then
        printf 'release: publish %s (commit %s): push %s and the tag %s, and create the GitHub release? [y/N] ' \
            "$v" "$(git rev-parse --short HEAD)" "$(git rev-parse --abbrev-ref HEAD)" "$tag"
        read -r answer
        [ "$answer" = y ] || [ "$answer" = yes ] || die "not published"
    fi
    git rev-parse -q --verify "refs/tags/$tag" >/dev/null || git tag -a "$tag" -m "Axiom $v" "$commit"
    git push origin "HEAD:refs/heads/$(git rev-parse --abbrev-ref HEAD)"
    git push origin "refs/tags/$tag"
    gh release create "$tag" build/release/axiom-"$v"-* build/release/SHA256SUMS --verify-tag \
        --title "Axiom $v" --notes-file "docs/releases/$v.md" $draft
    if [ -n "$advisory" ]; then
        gh api --method POST "repos/{owner}/{repo}/security-advisories" --input "$advisory"
    fi

    # After it: the branch for fix releases and main's next version, or the
    # fix release's notes on main.
    notes=$(cat "docs/releases/$v.md")
    if [ "$cmd" = release ]; then
        if ! git ls-remote --exit-code --heads origin "$branch" >/dev/null; then
            git branch -f "$branch" "$tag"
            git push origin "refs/heads/$branch"
        fi
        set_version "$major.$((minor + 1)).0"
        commit_all "Version $major.$((minor + 1)).0 on main, after the release of $v"
        git push origin HEAD:refs/heads/main
    else
        git checkout -q main
        git pull -q --rebase origin main
        if [ ! -f "docs/releases/$v.md" ]; then
            printf '%s\n' "$notes" > "docs/releases/$v.md"
            git add "docs/releases/$v.md"
            commit_all "docs/releases: $v, released from $branch"
            git push origin HEAD:refs/heads/main
        fi
        git checkout -q "$start"
    fi
    say "released $v: $(gh release view "$tag" --json url --jq .url)"
    exit 0
fi
[ $# -eq 2 ] || usage

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
    # the suite with both compilers, and with the compiler as the archives
    # have it: native code at -O2
    node cmd/axiom/internal/tests/run.mjs
    AXIOM=$A node cmd/axiom/internal/tests/run.mjs
    # (on a Mac, the native suite links cmd/axiom/internal/tests/native_rt.ax, not C)
    native_target=""
    [ "$(uname -s)" = Darwin ] && native_target=aarch64-macos
    AXIOM=$A AXIOM_TEST_SKIP_PLATFORM=1 AXIOM_NATIVE_TARGET=${AXIOM_NATIVE_TARGET:-$native_target} \
        node cmd/axiom/internal/tests/run.mjs --native -O2
    # the standard library's parsers against mutated inputs, with a seed of
    # this run's own
    python3 cmd/axiom/internal/tests/tools/fuzz.py --minutes 5 --seed 0 --compiler $A
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
    printf 'Axiom %s (%s), built from commit %s.\n\nPut bin/ on the PATH; the compiler finds std/ next to it. See RELEASE-NOTES.md\nand https://github.com/paulhankin/axiom.\n\nAxiom is experimental software, provided "as is", without warranty of any kind.\nUse it at your own risk; see LICENSE-MIT and LICENSE-APACHE for the full terms.\n' \
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

# Every archive holds the compiler, the library and the runtime, and no test data.
for f in "$R"/axiom-"$v"-*; do
    case $f in
        *.zip) list=$(unzip -Z1 "$f") ;;
        *) list=$(tar -tzf "$f") ;;
    esac
    for want in bin/axiom std/mem.ax std/internal/runtime/host.mjs std/internal/runtime/rt.ax LICENSE-MIT RELEASE-NOTES.md; do
        echo "$list" | grep -q "/$want" || echo "$list" | grep -q "/${want}.exe" ||
            { echo "release: $(basename "$f") lacks $want" >&2; exit 1; }
    done
    echo "$list" | grep -q testdata && { echo "release: $(basename "$f") holds test data" >&2; exit 1; }
done
# The AArch64 compiler reports its version under qemu, where qemu is.
if command -v qemu-aarch64 >/dev/null && [ "$(uname -m)" = x86_64 ]; then
    mkdir -p "$R/stage/qa"
    tar -xzf "$R/axiom-$v-linux-aarch64.tar.gz" -C "$R/stage/qa"
    [ "$(qemu-aarch64 "$R/stage/qa/axiom-$v-linux-aarch64/bin/axiom" version)" = "axiom $v" ] ||
        { echo "release: the AArch64 compiler does not run under qemu" >&2; exit 1; }
fi
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
echo "$commit" > build/release.commit
echo "built $R for $v (commit $commit)"
