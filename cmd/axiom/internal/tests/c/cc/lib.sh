# Helpers of the cc command's tests (c/cc/*/test.sh, which source this):
# CC is the compiler's cc command, OPT the suite's -O level.
set -u
fail() {
    echo "FAIL: $*"
    exit 1
}
# run <command...>: it must succeed
run() {
    "$@" || fail "exit status $?: $*"
}
# fails <command...>: it must fail (with status 1, as a cc's)
fails() {
    "$@" > fails.out 2>&1
    st=$?
    [ $st -eq 1 ] || fail "exit status $st (expected 1): $*: $(cat fails.out)"
}
# same <file> <text>: the file's text is exactly <text> (and a newline)
same() {
    printf '%s\n' "$2" > same.want
    cmp -s "$1" same.want || fail "$1 is not as expected:
--- expected
$2
--- got
$(cat "$1")"
}
# has <file> <text>: the file has a line with <text>
has() {
    grep -qF -- "$2" "$1" || fail "no '$2' in $1:
$(cat "$1")"
}
