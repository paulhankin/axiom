# _FORTIFY_SOURCE with the system's C library: the headers' wrappers of the
# functions with variable arguments (sprintf, printf, open ...) forward them
# with __builtin_va_arg_pack, and each call inlines its wrapper; a call
# behaves as without _FORTIFY_SOURCE, and an overflow of an array the
# compiler knows the size of ends the program as the library's check does.
. "$CC_LIB"
cat > probe.c <<'P'
#include <stdio.h>
#if defined __va_arg_pack && defined __USE_FORTIFY_LEVEL && __USE_FORTIFY_LEVEL >= 2
fortified_wrappers
#endif
P
run $CC -D_FORTIFY_SOURCE=2 -O2 -E probe.c -o probe.i
grep -q fortified_wrappers probe.i || exit 0  # (a C library without the wrappers)
run $CC -O2 fort.c -o plain
./plain > want || fail "plain: exit status $?"
cp out.txt want.txt
for f in "-D_FORTIFY_SOURCE=2 -O2" "-D_FORTIFY_SOURCE=2 -O1" "-D_FORTIFY_SOURCE=3 -O2" "-D_FORTIFY_SOURCE=2 -O3"; do
    rm -f out.txt
    run $CC $f fort.c -o fort
    ./fort > got || fail "$f: exit status $?"
    cmp -s got want || fail "$f: the output differs: $(cat got)"
    cmp -s out.txt want.txt || fail "$f: out.txt differs: $(cat out.txt)"
    ./fort abcdefghijklmnop > got 2> err
    st=$?
    [ $st -ne 0 ] || fail "$f: the overflow is not caught: $(cat got)"
    has err "buffer overflow detected"
done
# (the headers' check of open's arguments, which the GNU compilers report
# when compiling: here the link fails, naming it)
fails $CC -D_FORTIFY_SOURCE=2 -O2 mode.c -o mode
has fails.out "__open_missing_mode"
exit 0
