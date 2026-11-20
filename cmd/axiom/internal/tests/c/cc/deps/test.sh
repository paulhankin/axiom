# The make rules of the headers read: -M, -MM, -MD, -MMD, -MF, -MT, -MQ, -MP
# (-isystem directories' headers are system headers).
. "$CC_LIB"
run $CC -MM -Iinc -isystem sys m.c > got
same got "m.o: m.c a.h inc/b.h inc/c.h"
run $CC -M -Iinc -isystem sys m.c > got
has got "m.o: m.c a.h inc/b.h inc/c.h sys/s.h"
has got "stdio.h"
run $CC -MM -MT 'x$y.o' -MQ 'q$z.o' -MP -Iinc -isystem sys m.c -MF rule
same rule 'x$y.o q$$z.o: m.c a.h inc/b.h inc/c.h

a.h:

inc/b.h:

inc/c.h:'
# -MMD with -c: the object and its rule beside it (-o's name)
mkdir -p out
run $CC $OPT -c -MMD -MP -Iinc -isystem sys m.c -o out/m.o
[ -f out/m.o ] || fail "no out/m.o"
same out/m.d "out/m.o: m.c a.h inc/b.h inc/c.h

a.h:

inc/b.h:

inc/c.h:"
# -MD -MF, without -o
run $CC $OPT -c -MD -MF m.dep -Iinc -isystem sys m.c
[ -f m.o ] || fail "no m.o"
has m.dep "m.o: m.c a.h inc/b.h inc/c.h sys/s.h"
# -Wp,-MD,file (as some makefiles write it)
run $CC $OPT -c -Wp,-MMD,wp.d -Iinc -isystem sys m.c -o wp.o
same wp.d "wp.o: m.c a.h inc/b.h inc/c.h"
# compiled and linked: the rule of the program's source
run $CC $OPT -MMD -Iinc -isystem sys m.c -o prog
./prog > got
same got "3"
same prog.d "m.o: m.c a.h inc/b.h inc/c.h"
exit 0
