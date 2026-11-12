/* A second unit: data the Axiom program reaches through pointers, laid out
   as the Axiom struct Pair is. */
#include <stdbool.h>
#include <stdint.h>

struct pair {
    int32_t a;
    double b;
    bool flag;
    uint16_t small;
};

static int counter = 1;
static const char greeting[] = "hello from C";

void c_fill(struct pair *p) {
    p->a = 7;
    p->b = 0.5;
    p->flag = true;
    p->small = 65535;
}

int *c_counter(void) { return &counter; }
const char *c_greeting(void) { return greeting; }
