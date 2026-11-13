/* The C side of ffi_values.ax: functions of structures by value, and calls
   of the Axiom program's exports that take and return them. */
#include <stdint.h>

typedef struct { int32_t a, b; } Small;
typedef struct { int64_t v[5]; int32_t tag; } Big;
typedef struct { float x, y; } Fl;
typedef struct { double x, y; } Dd;
typedef struct { double d; int32_t i; uint8_t c; } Mix;
typedef struct { uint8_t c; } Tiny;

Small c_make_small(int32_t a, int32_t b) { Small s = {a - 1, b + 10}; return s; }
int64_t c_sum_big(Big l) { int64_t s = 0; for (int i = 0; i < 5; i++) s += l.v[i]; return s + l.tag; }
Big c_make_big(int32_t base) { Big b; for (int i = 0; i < 5; i++) b.v[i] = base * (i + 1); b.tag = 1; return b; }
Fl c_scale(Fl f, float k) { Fl r = {f.x * k, f.y * k}; return r; }
Dd c_swap(Dd d) { Dd r = {d.y, d.x}; return r; }
Mix c_mix(Mix m, int32_t n) { m.d += n; m.i *= n; m.c += 1; return m; }
Tiny c_tiny(Tiny t, Tiny u) { Tiny r = {(uint8_t)(t.c + u.c)}; return r; }
double c_many(Small a, Small b, Small c, Small d, Dd e, Dd f, Dd g, Dd h, Dd i) {
    return a.a + b.b + c.a + d.b + e.x + f.y + g.x + h.y + i.x + i.y + 10.0;
}

Small ax_twice_small(Small s);
Big ax_big(Big b, int32_t k);
double ax_mix(Mix m);
int64_t c_call_back(void) {
    Small s = ax_twice_small((Small){3, 4});
    Big b = ax_big((Big){{1, 2, 3, 4, 5}, 9}, 2);
    double m = ax_mix((Mix){0.5, 2, 3});
    return s.a * 1000 + s.b * 100 + b.v[4] + b.tag + (int64_t)(m * 4.0);
}
