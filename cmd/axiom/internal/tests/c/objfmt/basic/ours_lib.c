// An object of this compiler: data, a function pointer in data, string
// literals, a jump table, internal and external functions.
#include <stdio.h>

int counter = 5;
static int calls;
int ext_val = 3;

int ext_fn(int x) { return x * 2; }

int get(void) { return counter + ext_val + calls++; }

static const char *msg(void) { return "hello"; }

int (*fp)(int) = ext_fn;

int sw(int x) {
    switch (x) {
    case 0: return 11;
    case 1: return 22;
    case 2: return 7;
    case 3: return 99;
    case 4: return 5;
    case 5: return 77;
    default: return -1;
    }
}

void say(void) { printf("hi %d %s %d %d\n", counter, msg(), sw(counter), fp(4)); }
