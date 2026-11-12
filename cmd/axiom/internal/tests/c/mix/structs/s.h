/* The structures of structs/ and their functions. */
#include <stdint.h>

struct small { int32_t a, b; };
struct large { int64_t v[5]; int32_t tag; };
struct fl { float x, y; };
struct dd { double x, y; };
struct mixed { double d; int32_t i; char c; };

struct small make_small(int a, int b);
struct large make_large(int base);
int64_t sum_large(struct large l);
struct fl scale_fl(struct fl f, float k);
struct dd swap_dd(struct dd d);
struct mixed make_mixed(int i);

/* rt.ax */
int64_t ax_small_sum(const struct small *s);
int64_t ax_large_sum(const struct large *l);
double ax_mixed_sum(const struct mixed *m);
void ax_mixed_set(struct mixed *m);
