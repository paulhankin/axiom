# make with the conventional rules and CC set to the cc command: a build,
# its check, and a rebuild after a header changes (through the -MMD rules).
. "$CC_LIB"
command -v make > /dev/null || { echo "(no make: skipped)"; exit 0; }
run make CC="$CC" > log1 2>&1
run make CC="$CC" check > log2 2>&1
has src/scale.d "include/calc.h"
make CC="$CC" -q calc || fail "make -q: not up to date after the build"
sleep 1
sed 's/CALC_BASE 10/CALC_BASE 100/' include/calc.h > h.tmp && mv h.tmp include/calc.h
run make CC="$CC" > log3 2>&1
has log3 "src/scale.c"
[ "$(./calc 3)" = 500 ] || fail "after the header changed: $(./calc 3)"
run make CC="$CC" clean > /dev/null
exit 0
