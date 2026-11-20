# -E (to standard output, -o, line markers, -P), -D, -U, -I, -include,
# -std=, -dM, standard input as the source (-), -x c; the preprocessed
# text compiled again (its line markers keep the diagnostics' lines).
. "$CC_LIB"
run $CC -E -P -Iinc -include pre.h -DDEF=3 -DGONE -UGONE pp.c > got
same got "int a = 5;
int b = 9;
int c = 3;
long v = 201710L;
int arch = 1;"
run $CC -E -P -std=c99 -Iinc -Ino/such/dir -include pre.h pp.c -o got2
has got2 "long v = 199901L;"
# (C89 with the compiler's C library: the system's headers need C99)
run $CC -E -P -ansi --c-lib=axiom -Iinc -include pre.h pp.c -o got3
has got3 "long v = __STDC_VERSION__;"
run $CC -E -P -std=gnu2x -Iinc -include pre.h pp.c -o got4
has got4 "long v = 202311L;"
# line markers
run $CC -E lines.c > lines.i
has lines.i '# 1 "lines.c"'
has lines.i '# 100 "elsewhere.c"'
# read again: the error is at the line the markers give
cp lines.i again.c
$CC -c again.c -o again.o 2> err.txt && fail "again.c compiled"
has err.txt "elsewhere.c:101"
# -dM: the macros at the end
printf '#define MINE (1 + 2)\n' | $CC -dM -E - > macros || fail "-dM -E -"
has macros "#define MINE (1 + 2)"
has macros "#define __STDC__ 1"
# standard input, -x c with another name
printf 'int main(void) { return 4; }\n' | $CC $OPT -x c - -o fromstdin || fail "-x c -"
./fromstdin; [ $? -eq 4 ] || fail "fromstdin's exit status"
printf 'int main(void) { return 5; }\n' > prog.txt
run $CC $OPT -x c prog.txt -x none -o fromtxt
./fromtxt; [ $? -eq 5 ] || fail "fromtxt's exit status"
# a header does not exist: an error (as configure's checks of headers expect)
printf '#include <no_such_header.h>\n' > nohdr.c
fails $CC -E nohdr.c
exit 0
