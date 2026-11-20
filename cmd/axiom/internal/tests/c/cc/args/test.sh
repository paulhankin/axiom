# main's arguments: one block of strings (argv[0] to the last), which a
# program may overwrite whole, as a program that retitles its process does.
. "$CC_LIB"
run $CC $OPT main.c -o prog
./prog alpha "" "a longer argument" z > got || fail "prog: exit status $?"
same got "5 1  retitled"
./prog one two > got || fail "prog: exit status $?"
same got "3 1 two retitled"
