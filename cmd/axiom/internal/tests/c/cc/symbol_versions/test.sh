# Symbol versions: a shared library's version definitions from its version
# script (named nodes, inheritance, extern "C++" blocks, .symver's versions
# of a name), the versions a program needs of it and of the system's C
# library (.gnu.version_r), which the dynamic linker checks and binds to.
. "$CC_LIB"
command -v readelf > /dev/null || exit 0
mkdir -p lib other
# defined: the dynamic symbols a file defines, with their versions
defined() {
    readelf -W --dyn-syms "$1" | awk '$7 != "UND" && $8 != "" && NR > 3 { print $8 }' | sort | paste -sd ' ' -
}
verdefs() {
    readelf -V "$1" | sed -n '/version_d/,/^$/p' | grep -E 'Rev:|Parent' | sed 's/^ *[0-9a-fx]*: *//'
}
verneeds() {
    readelf -V "$1" | sed -n '/version_r/,/^$/p' | grep -E 'File:|Name:' | sed 's/^ *[0-9a-fx]*: *//; s/  Flags.*//'
}
build() {  # build <target flags...>: the library, the programs, and runs them
    run $CC "$@" $OPT -shared -fPIC -Wl,-soname,libsv.so.1 -Wl,--version-script=lib.map lib.c value.s -o lib/libsv.so.1
    ln -sf libsv.so.1 lib/libsv.so
    run $CC "$@" $OPT main.c -Llib -lsv -Wl,-rpath,"$PWD/lib" -o prog
    run $CC "$@" $OPT old.c -Llib -lsv -Wl,-rpath,"$PWD/lib" -o old
    run $CC "$@" $OPT math.c -lm -o math
}
check() {  # check <runner...>: the versions, and the programs' output
    verdefs lib/libsv.so.1 > got
    same got "Rev: 1  Flags: BASE  Index: 1  Cnt: 1  Name: libsv.so.1
Rev: 1  Flags: none  Index: 2  Cnt: 1  Name: LIB_1.0
Rev: 1  Flags: none  Index: 3  Cnt: 2  Name: LIB_2.0
Parent 1: LIB_1.0"
    defined lib/libsv.so.1 > got
    same got "_ZN1AC1Ev@@LIB_1.0 _ZN2ns1fEPKc@@LIB_2.0 _ZN2ns1fEi@@LIB_1.0 _ZNK1A4sizeEv@@LIB_1.0 _ZTV1A@@LIB_1.0 call_foo@@LIB_1.0 f@@LIB_1.0 foo@@LIB_2.0 foo@LIB_1.0 g@@LIB_2.0 value@@LIB_2.0"
    verneeds prog | grep -A2 'libsv' > got
    same got "Version: 1  File: libsv.so.1  Cnt: 2
Name: LIB_1.0
Name: LIB_2.0"
    verneeds prog | grep -q 'File: libc.so.6' || fail "prog needs no version of libc.so.6: $(readelf -V prog)"
    verneeds old | grep -A1 'libsv' > got
    same got "Version: 1  File: libsv.so.1  Cnt: 1
Name: LIB_1.0"
    verneeds math | grep -A1 'libm.so.6' | grep -q 'Name: GLIBC_' || fail "math needs no version of libm.so.6: $(readelf -V math)"
    "$@" ./prog > got 2>&1 || fail "prog: $(cat got)"
    same got "1 2 20 21 42 65 7"
    "$@" ./old > got 2>&1 || fail "old: $(cat got)"
    same got "10"
    "$@" ./math x > got 2>&1 || fail "math: $(cat got)"
    same got "0.841"
}
build
check
# the versions bind: a library without LIB_2.0 does not do for prog
run $CC $OPT -shared -fPIC -Wl,-soname,libsv.so.1 -Wl,--version-script=one.map -DONE_VERSION lib.c -o other/libsv.so.1
LD_LIBRARY_PATH="$PWD/other" ./prog > got 2>&1 && fail "prog ran with a library without LIB_2.0: $(cat got)"
has got "LIB_2.0"
# a node of no name gives no versions
printf '{ global: f; local: *; };\n' > v.map
run $CC -shared -DONE_VERSION lib.c -Wl,--version-script=v.map -o v.so
readelf -V v.so | grep -q version_d && fail "an anonymous node's library has version definitions"
defined v.so > got
same got "f"
# errors: a version no node defines, a parent not defined before, an
# anonymous node with others, another language's block
printf 'int h(void) { return 0; }\n__asm__(".symver h, h@@NONE");\n' > bad.c
printf 'LIB_1.0 { h; };\n' > v.map
fails $CC -shared bad.c -Wl,--version-script=v.map -o v.so
has fails.out "NONE"
for bad in 'V2 { f; } V1; V1 { g; };' '{ f; }; V1 { g; };' 'V1 { extern "Java" { x; }; };' 'V1 { f; }; V1 { g; };'; do
    printf '%s\n' "$bad" > v.map
    fails $CC -shared -DONE_VERSION lib.c -Wl,--version-script=v.map -o v.so
    has fails.out "version script"
done
# (.symver is file-scope asm's)
printf 'void f(void) { __asm__ volatile(".symver f, f@V1"); }\n' > inl.c
fails $CC -c inl.c -o inl.o
has fails.out ".symver"
# AArch64, under the emulator
if command -v qemu-aarch64-static > /dev/null && [ -d /usr/aarch64-linux-gnu/lib ]; then
    rm -rf lib/* prog old math
    build --target=aarch64
    check env QEMU_LD_PREFIX=/usr/aarch64-linux-gnu qemu-aarch64-static
fi
exit 0
