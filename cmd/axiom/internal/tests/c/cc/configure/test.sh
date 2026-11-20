# A configure script in the style of the generated ones (its probes of the
# compiler, the preprocessor, headers, types and libraries), then make.
. "$CC_LIB"
command -v make > /dev/null || { echo "(no make: skipped)"; exit 0; }
CFLAGS="-g $OPT" sh ./configure > conf.out 2>&1 || fail "configure: $(cat conf.out) $(cat config.log)"
has conf.out "checking whether the compiler supports GNU C... yes"
has conf.out "checking for no_such_header.h... no"
has conf.out "checking for strndup... yes"
has conf.out "checking dependency style of $CC... make rules"
run make > make.out 2>&1
run make check > check.out 2>&1
# (the GNU compilers' macros, as a GNU-compatible cc's; __VERSION__ names
# this compiler; not under -std=c95, the strict standard)
echo __GNUC__ __GNUC_MINOR__ __VERSION__ > v.c
run $CC -E -P v.c -o v.i
has v.i "4 2 \"4.2.1 Compatible Axiom "
run $CC -std=iso9899:199409 --c-lib=axiom -E -P v.c -o v95.i
has v95.i "__GNUC__ __GNUC_MINOR__ __VERSION__"
exit 0
