# -c, -o, several sources, objects and archives linked, -l and -L, the
# program's exit status.
. "$CC_LIB"
run $CC $OPT -c util.c
[ -f util.o ] || fail "-c util.c made no util.o"
run $CC $OPT -c main.c sub/three.c
[ -f main.o ] && [ -f three.o ] || fail "-c main.c sub/three.c: no main.o and three.o in the current directory"
mkdir -p out
run $CC $OPT -c util.c -o out/u.o
[ -f out/u.o ] || fail "-c -o out/u.o"
fails $CC -c util.c main.c -o x.o
has fails.out "more than one source"
run $CC main.o out/u.o three.o -o prog -lm
./prog > got; st=$?
[ $st -eq 7 ] || fail "prog: exit status $st"
same got "40 3 7"
# sources and objects together, a.out by default
run $CC $OPT main.c util.o three.o -lm
./a.out x > got
same got "40 6 7"
# an archive the system's ar makes of the objects, -L and -l (also apart)
rm -f libu.a
ar rcs libu.a util.o three.o || fail "ar"
run $CC main.o -L. -lu -lm -o p2
./p2 > got
same got "40 3 7"
run $CC main.o -L . -l u -l m -Wl,--as-needed -Wl,-O1 -o p3
./p3 > got
same got "40 3 7"
# objects of other names (x.lo), alone
cp util.o util.lo
run $CC main.o util.lo three.o -lm -o p5
./p5 > got
same got "40 3 7"
# objects alone, linked in a package's directory: a C program all the same
mkdir -p pkg
printf '[package]\nname = "x"\n' > pkg/axiom.toml
(cd pkg && $CC ../main.o ../util.o ../three.o -lm -o p6) || fail "a link next to axiom.toml"
./pkg/p6 > got
same got "40 3 7"
# one build of the three sources
run $CC $OPT -o p4 main.c util.c sub/three.c -lm
./p4 > got
same got "40 3 7"
exit 0
