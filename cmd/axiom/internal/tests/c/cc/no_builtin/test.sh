# -fno-builtin: calls of memcpy, memmove, memset and memcmp go to the
# program's own definitions in another unit (by default they are the
# compiler's memory operations); -fbuiltin after it undoes it.
. "$CC_LIB"
run $CC $OPT -fno-builtin main.c mem.c -o prog
run ./prog > out.txt
same out.txt "hello 0 1111"
run $CC $OPT -fno-builtin-memcpy main.c mem.c -o prog
run ./prog > out.txt
same out.txt "hello 0 1111"
exit 0
