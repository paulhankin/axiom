include config.mk

prog: prog.c config.h
	$(CC) $(CFLAGS) -o prog prog.c $(LIBS)

check: prog
	test "$$(./prog)" = "config 1.5"
