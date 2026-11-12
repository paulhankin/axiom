/* The other unit of structs/: functions of structures by value. */
#include "s.h"

struct small make_small(int a, int b) { return (struct small){a - 1, b + 10}; }

struct large make_large(int base) {
    struct large l;
    for (int i = 0; i < 5; i++)
        l.v[i] = base * (i + 1);
    l.tag = 99;
    return l;
}

int64_t sum_large(struct large l) {
    int64_t s = 0;
    for (int i = 0; i < 5; i++)
        s += l.v[i];
    return l.tag == 99 ? s : -1;
}

struct fl scale_fl(struct fl f, float k) { return (struct fl){f.x * k, f.y * k}; }
struct dd swap_dd(struct dd d) { return (struct dd){d.y, d.x}; }
struct mixed make_mixed(int i) { return (struct mixed){i + 0.5, i * 2, (char)(i + 1)}; }
