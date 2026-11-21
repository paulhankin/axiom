# The warnings for a later standard's features in an older GNU dialect
# (C4145: _Bool, _Atomic, _Static_assert in gnu89) appear only with -pedantic
# or -Wpedantic (--c-pedantic), as the GNU compilers have them; axiom build's
# plain --c-std=gnu89, without --c-gnu-compat, has them always.
. "$CC_LIB"
run $CC -std=gnu89 -c later.c -o later.o 2> err
[ -s err ] && fail "without -pedantic: $(cat err)"
for p in -pedantic -Wpedantic -pedantic-errors; do
    run $CC -std=gnu89 $p -c later.c -o later.o 2> err
    has err "C4145"
done
run $CC -std=gnu89 -Wpedantic -Wno-pedantic -c later.c -o later.o 2> err
[ -s err ] && fail "-Wno-pedantic: $(cat err)"
run "$AXIOM" build later.c --target=native --c-std=gnu89 --emit=obj -o later.o 2> err
has err "C4145"
run "$AXIOM" build later.c --target=native --c-std=gnu89 --c-gnu-compat --emit=obj -o later.o 2> err
[ -s err ] && fail "--c-gnu-compat: $(cat err)"
run "$AXIOM" build later.c --target=native --c-std=gnu89 --c-gnu-compat --c-pedantic --emit=obj -o later.o 2> err
has err "C4145"
exit 0
