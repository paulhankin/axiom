# Options of the conventional cc command line: the optimization and debug
# levels, -W and -f options (the meaningful ones mapped, the others
# accepted), -std=, -m options, and the questions configure scripts ask.
. "$CC_LIB"
for o in -O -O0 -O1 -O2 -O3 -Os -Oz -Og -Ofast; do
    run $CC $o -c chars.c -o o.o
done
for g in -g -g0 -g1 -g2 -g3 -ggdb -gdwarf-4 -gsplit-dwarf; do
    run $CC $g -c chars.c -o o.o
done
run $CC -Wall -Wextra -Wno-unused-parameter -Wshadow -Wstrict-prototypes -Werror=implicit-function-declaration \
    -pedantic -fno-common -fvisibility=hidden -fno-strict-aliasing -fstack-protector-strong -fomit-frame-pointer \
    -fdiagnostics-color=never -fno-builtin -fexceptions -fsome-unknown-option -pipe -m64 -mtune=generic \
    -march=x86-64 -pthread -D_FORTIFY_SOURCE=2 -Wp,-DFROM_WP -Wa,--noexecstack -c chars.c -o o.o
run $CC $OPT -fsigned-char chars.c -o signed_
./signed_ > got
same got "0 -128"
run $CC $OPT -funsigned-char chars.c -o unsigned_
./unsigned_ > got
same got "1 0"
# -fwrapv: signed + is not marked as not overflowing (the IR's nsw)
run $CC --emit=ir chars.c -o plain.ir
grep -q "add nsw i32" plain.ir || fail "no nsw add without -fwrapv"
run $CC -fwrapv --emit=ir chars.c -o wrap.ir
grep -q "add nsw" wrap.ir && fail "-fwrapv: an nsw add"
# warnings: shown, errors with -Werror, none with -w
run $CC -c warn.c -o w.o 2> err
has err "warning"
fails $CC -Werror -c warn.c -o w.o
has fails.out "error"
run $CC -w -Werror -c warn.c -o w.o 2> err
[ -s err ] && fail "-w: $(cat err)"
run $CC -Werror -Wno-error -c warn.c -o w.o 2> err
# the questions
run $CC --version > got
has got "axiom"
run $CC -dumpversion > got
[ -s got ] || fail "-dumpversion printed nothing"
run $CC -dumpmachine > got
grep -q -- '-linux-gnu$' got || fail "-dumpmachine: $(cat got)"
run $CC -print-prog-name=ld > got
same got "ld"
run $CC -v 2> got
has got "Target:"
run $CC -print-file-name=no-such-file.a > got
same got "no-such-file.a"
# the compiler under the name axcc is the cc command
ln -sf "$AXIOM" axcc
run ./axcc $OPT chars.c -o viaaxcc
./viaaxcc > got
[ -s got ] || fail "axcc's program printed nothing"
# -march=native and the processor's extensions
run $CC -march=native -c chars.c -o o.o
if [ "$(uname -m)" = x86_64 ]; then
    run $CC -msse4.2 -mno-avx -c chars.c -o o.o
fi
exit 0
