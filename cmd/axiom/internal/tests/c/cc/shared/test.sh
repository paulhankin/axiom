# -shared (-fPIC, -Wl,-soname, a version script, references left to the
# loader), a program linked with it (-L -l, -Wl,-rpath) that exports its
# own definitions (-rdynamic) and loads the library again (dlopen).
. "$CC_LIB"
mkdir -p lib
run $CC $OPT -fPIC -c lib.c -o lib.o
run $CC -shared -Wl,-soname,libdemo.so.1 -Wl,--version-script=lib.map lib.o -o lib/libdemo.so.1
ln -sf libdemo.so.1 lib/libdemo.so
run $CC $OPT -rdynamic main.c -Llib -ldemo -ldl -Wl,-rpath,"$PWD/lib" -o prog
run $CC $OPT -shared -fPIC plugin.c -o plugin.so
./prog "$PWD/lib/libdemo.so.1" "$PWD/plugin.so" > got || fail "prog: $(cat got)"
same got "50
20 inner hidden
201"
# without -rdynamic: the program keeps host_value, which only the library calls
run $CC $OPT main.c -Llib -ldemo -ldl -Wl,-rpath,"$PWD/lib" -o prog2
./prog2 > got || fail "prog2: $(cat got)"
same got "50"
# version scripts: words split at ':' too, a name itself decides before a
# pattern, malformed ones are errors
exports() {
    run $CC -shared vs.c -Wl,--version-script=v.map -o v.so
    readelf --dyn-syms v.so | awk '$7 != "UND" && $8 != "" && NR > 3 { print $8 }' | sort | tr '\n' ' ' > got
    [ "$(cat got)" = "$1 " ] || fail "$(cat v.map): exports '$(cat got)', expected '$1'"
}
if command -v readelf > /dev/null; then
    # (the library's undefined symbols are what it uses: no main, no
    # ax_unavailable)
    readelf --dyn-syms lib/libdemo.so.1 | grep -E ' (main|ax_unavailable)$' > got && fail "libdemo.so.1 lists: $(cat got)"
    printf '{ global:g; local: *; };\n' > v.map; exports "g"
    printf '{global:g;h;local:*;};\n' > v.map; exports "g h"
    printf 'V1 { global: f*; local: fp; *; };\n' > v.map; exports "fq@@V1"
    run $CC $OPT ex.c -Wl,--export-dynamic-symbol,exported_one -o ex
    readelf --dyn-syms ex | grep -q exported_one || fail "--export-dynamic-symbol: not exported"
fi
for bad in '{ global: g' '{ global: g; ' 'V1 { g; } ' '{ extern "Java" { x; }; };' 'g;'; do
    printf '%s\n' "$bad" > v.map
    fails $CC -shared vs.c -Wl,--version-script=v.map -o v.so
    has fails.out "version script"
done
# -Wl,-z,defs: the library's undefined reference is an error then
fails $CC -shared -Wl,-z,defs lib.o -o lib/strict.so
exit 0
