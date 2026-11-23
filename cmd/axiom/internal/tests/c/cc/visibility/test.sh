# -fvisibility= (axiom build's --c-visibility=), #pragma GCC visibility
# over it and the attribute visibility over both: a shared library's
# dynamic symbols for each, its declarations' visibility binding its
# references to its own definitions (one build, and objects linked), and a
# program linked with it.
. "$CC_LIB"
command -v readelf > /dev/null || exit 0
# the library's definitions in .dynsym, as name:visibility
exports() {
    readelf --dyn-syms -W "$1" | awk '$7 != "UND" && $8 != "" && NR > 3 { print $8 ":" $6 }' | sort | tr '\n' ' ' > got
    [ "$(cat got)" = "$2 " ] || fail "$1: exports '$(cat got)', expected '$2'"
}
# (sorted by name; the attribute's and push(default)'s are in every one)
all_of() {
    printf '%s\n' after_pop:DEFAULT attr_default:DEFAULT attr_over_pragma:DEFAULT data_default:DEFAULT entry:DEFAULT \
        prot:PROTECTED pushed_default:DEFAULT "$@" | sort | tr '\n' ' ' | sed 's/ $//'
}
run $CC $OPT -shared -fPIC lib.c impl.c -o libnone.so
exports libnone.so "$(all_of api:DEFAULT data_plain:DEFAULT internal_:DEFAULT)"
run $CC $OPT -shared -fPIC -fvisibility=default lib.c impl.c -o libdefault.so
exports libdefault.so "$(all_of api:DEFAULT data_plain:DEFAULT internal_:DEFAULT)"
run $CC $OPT -shared -fPIC -fvisibility=hidden lib.c impl.c -o libhidden.so
exports libhidden.so "$(all_of)"
run $CC $OPT -shared -fPIC -fvisibility=protected lib.c impl.c -o libprotected.so
exports libprotected.so "$(all_of api:PROTECTED data_plain:PROTECTED internal_:PROTECTED)"
run $CC $OPT -shared -fPIC -fvisibility=internal lib.c impl.c -o libinternal.so
exports libinternal.so "$(all_of)"
# objects compiled apart: the hidden declarations are in lib.o's symbols
run $CC $OPT -fPIC -fvisibility=hidden -c lib.c -o lib.o
run $CC $OPT -fPIC -c impl.c -o impl.o
readelf -s -W lib.o | grep -q 'HIDDEN .*UND impl_func$' || fail "lib.o: impl_func is not a hidden reference"
run $CC -shared lib.o impl.o -o libobjs.so
exports libobjs.so "$(all_of)"
# axiom build's option
run "$AXIOM" build lib.c impl.c --target=native --c-lib=system --c-std=gnu17 --pic --emit=shared --c-visibility=hidden -o libbuild.so
exports libbuild.so "$(all_of)"
# a program linked with each runs (the hidden own() is the library's)
for v in none hidden protected objs; do
    run $CC $OPT main.c -L. -l$v -Wl,-rpath,"$PWD" -o prog_$v
    ./prog_$v > got || fail "prog_$v: $(cat got)"
    same got "3 4 6 7 8 9 1220"
done
# nested pragmas: push(default) inside push(hidden), under -fvisibility=hidden;
# and the preprocessed text keeps them
run $CC -shared -fvisibility=hidden nested.c -o libnested.so
exports libnested.so "inner:DEFAULT"
run $CC -E nested.c -o nested.i
run $CC -shared -fvisibility=hidden nested.i -o libnested2.so
exports libnested2.so "inner:DEFAULT"
run $CC -shared nested.i -o libnested3.so
exports libnested3.so "inner:DEFAULT last:DEFAULT"
# a hidden declaration is the library's own: what no object defines is an error
fails $CC -shared -fPIC undef.c -o libundef.so
has fails.out "declares 'nowhere' hidden"
fails $CC -fvisibility=sideways -c impl.c -o x.o
has fails.out "unknown visibility"
exit 0
