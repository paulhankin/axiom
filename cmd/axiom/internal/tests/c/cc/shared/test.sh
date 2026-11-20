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
# -Wl,-z,defs: the library's undefined reference is an error then
fails $CC -shared -Wl,-z,defs lib.o -o lib/strict.so
exit 0
