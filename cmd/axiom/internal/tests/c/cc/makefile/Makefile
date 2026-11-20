# The conventional makefile of a small C program: objects with their
# dependencies (-MMD -MP), a static library made with ar, the program.
CFLAGS = -O2 -g -Wall -Wextra -fPIC
CPPFLAGS = -Iinclude -DNDEBUG
AR = ar
OBJS = src/add.o src/scale.o

all: calc

%.o: %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -MMD -MP -c $< -o $@

libcalc.a: $(OBJS)
	rm -f $@
	$(AR) rcs $@ $(OBJS)

calc: src/main.o libcalc.a
	$(CC) $(LDFLAGS) -o $@ src/main.o -L. -lcalc

check: calc
	test "$$(./calc 3)" = 50

clean:
	rm -f calc libcalc.a src/*.o src/*.d

-include $(OBJS:.o=.d) src/main.d
