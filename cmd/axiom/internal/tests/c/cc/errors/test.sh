# Errors as a cc reports them: exit status 1, the message on standard
# error; options that cannot be honoured are errors, not ignored.
. "$CC_LIB"
: > good.h
: > prog.cpp
$CC -c bad.c -o bad.o > out 2> err; st=$?
[ $st -eq 1 ] || fail "bad.c: exit status $st"
has err "bad.c:1:"
[ -s out ] && fail "diagnostics on standard output: $(cat out)"
[ -f bad.o ] && fail "bad.o was written"
fails $CC -c missing.c
has fails.out "missing.c"
fails $CC
has fails.out "no input files"
fails $CC -S good.c
has fails.out "no assembly output"
fails $CC --no-such-option-of-axiom good.c
fails $CC -qwerty good.c
has fails.out "unknown option '-qwerty'"
fails $CC -std=c++17 good.c
has fails.out "C++"
fails $CC -std=c77 good.c
fails $CC -c prog.cpp
has fails.out "not C"
fails $CC -nostdlib good.c
fails $CC -fsanitize=address good.c
fails $CC -fshort-enums -c good.c
fails $CC -m32 -c good.c
for m in -mno-red-zone -mgeneral-regs-only -mno-sse -mno-sse2 -mcmodel=kernel; do
    fails $CC $m -c good.c
    has fails.out "$m"
done
fails $CC -Wl,-T,script.ld good.c
has fails.out "-T script.ld"
fails $CC -Wl,--wrap=malloc good.c
has fails.out "--wrap=malloc"
fails $CC -E good.c bad.c -o both.i
fails $CC -o
has fails.out "missing argument to '-o'"
fails $CC -static good.c
has fails.out "-static"
fails $CC -c good.h
has fails.out "header"
exit 0
