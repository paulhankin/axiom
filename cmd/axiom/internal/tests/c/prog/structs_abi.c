/* flags: --c-std=c11 */
/* Structures and unions as values: passed and returned by value at every
   size from one byte to hundreds (including the sizes the calling
   conventions split between registers, mixed integer and floating
   members, and odd sizes), through function pointers, as variadic
   arguments, assigned, nested, in arrays, with bit-fields of every width,
   unions of them, nested designated initializers, compound literals, a
   flexible array member, and a tagged-union expression tree evaluated
   by value. */
#include "host.h"
#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>
#include <stdbool.h>

static uint64_t h = 1469598103934665603ull;
static void mix(uint64_t v) { h = (h ^ v) * 1099511628211ull; }
static void mixd(double d) { mix((uint64_t)(int64_t)(d * 1000)); }

/* ---- every small size ---- */

typedef struct { uint8_t a; } s1;
typedef struct { uint8_t a, b, c; } s3;
typedef struct { uint16_t a; uint8_t b; } s4p;
typedef struct { uint8_t a[5]; } s5;
typedef struct { uint8_t a[7]; } s7;
typedef struct { int32_t a; float b; } sif;
typedef struct { float a, b, c; } sf3;
typedef struct { double a; } sd1;
typedef struct { double a, b; } sd2;
typedef struct { double a; int64_t b; } sdl;
typedef struct { int64_t a; double b; } sld;
typedef struct { float a; double b; } sfd;
typedef struct { uint8_t a; double b; uint8_t c; } spad;
typedef struct { int32_t a[3]; } si3;
typedef struct { int64_t a[2]; int32_t b; } s20;
typedef struct { double a[4]; } sd4;
typedef struct { char name[13]; } s13;
typedef struct { uint8_t a[100]; } s100;

#define MAKER(T, body) static T make_##T(int k) { T r; body; return r; }
MAKER(s1, r.a = (uint8_t)k)
MAKER(s3, r.a = (uint8_t)k; r.b = (uint8_t)(k + 1); r.c = (uint8_t)(k * 3))
MAKER(s4p, r.a = (uint16_t)(k * 1000); r.b = (uint8_t)k)
MAKER(s5, for (int i = 0; i < 5; i++) r.a[i] = (uint8_t)(k + i))
MAKER(s7, for (int i = 0; i < 7; i++) r.a[i] = (uint8_t)(k * i))
MAKER(sif, r.a = -k; r.b = (float)k / 4)
MAKER(sf3, r.a = (float)k; r.b = (float)k * 0.5f; r.c = -(float)k)
MAKER(sd1, r.a = k * 1.25)
MAKER(sd2, r.a = k * 1.5; r.b = -k * 2.5)
MAKER(sdl, r.a = k / 8.0; r.b = (int64_t)k << 40)
MAKER(sld, r.a = -((int64_t)k << 33); r.b = k * 0.125)
MAKER(sfd, r.a = (float)k + 0.5f; r.b = k * 3.0)
MAKER(spad, r.a = (uint8_t)k; r.b = k * 0.75; r.c = (uint8_t)~k)
MAKER(si3, r.a[0] = k; r.a[1] = -k; r.a[2] = k * k)
MAKER(s20, r.a[0] = k; r.a[1] = (int64_t)k * 1000000007; r.b = k ^ 0x5a5a)
MAKER(sd4, for (int i = 0; i < 4; i++) r.a[i] = k + i * 0.25)
MAKER(s13, for (int i = 0; i < 12; i++) r.name[i] = (char)('a' + (k + i) % 26); r.name[12] = 0)
MAKER(s100, for (int i = 0; i < 100; i++) r.a[i] = (uint8_t)(k + i * 7))

static uint64_t sum_bytes(const void *p, size_t n) {
    const unsigned char *b = p;
    uint64_t s = 0;
    for (size_t i = 0; i < n; i++) s = s * 131 + b[i];
    return s;
}

/* a function taking all of them, interleaved with scalars */
static double take_all(s1 a, int x, s3 b, double y, s4p c, s5 d, float z, s7 e, sif f, sf3 g, sd1 i, sd2 j,
                       char w, sdl k, sld l, sfd m, spad n, si3 o, s20 p, sd4 q, s13 r, s100 s) {
    double t = a.a + x + b.a + b.b + b.c + y + c.a + c.b + z + w;
    for (int u = 0; u < 5; u++) t += d.a[u];
    for (int u = 0; u < 7; u++) t += e.a[u];
    t += f.a + f.b + g.a + g.b + g.c + i.a + j.a + j.b + k.a + (double)(k.b >> 40) + (double)(l.a >> 33) + l.b;
    t += m.a + m.b + n.a + n.b + n.c + o.a[0] + o.a[1] + o.a[2] + (double)p.a[0] + (double)(p.a[1] % 1000) + p.b;
    for (int u = 0; u < 4; u++) t += q.a[u];
    t += r.name[0] + r.name[11];
    for (int u = 0; u < 100; u += 9) t += s.a[u];
    return t;
}

/* returning each through a pointer to a function returning it */
typedef sd2 (*make_sd2_fn)(int);
typedef s20 (*make_s20_fn)(int);

static sd2 twice_sd2(make_sd2_fn f, int k) {
    sd2 a = f(k), b = f(k + 1);
    return (sd2){ a.a + b.a, a.b + b.b };
}

static s20 add_s20(s20 x, s20 y) {
    x.a[0] += y.a[0];
    x.a[1] += y.a[1];
    x.b += y.b;
    return x;   /* the caller's copy is unchanged */
}

/* variadic structures */
static double va_structs(int n, ...) {
    va_list ap;
    va_start(ap, n);
    double t = 0;
    for (int i = 0; i < n; i++) {
        switch (i % 4) {
        case 0: { sd2 v = va_arg(ap, sd2); t += v.a - v.b; break; }
        case 1: { s3 v = va_arg(ap, s3); t += v.a * 100 + v.c; break; }
        case 2: { sif v = va_arg(ap, sif); t += v.a + v.b; break; }
        case 3: { s100 v = va_arg(ap, s100); t += v.a[99]; break; }
        }
    }
    va_end(ap);
    return t;
}

/* ---- bit-fields ---- */

struct flags {
    unsigned a : 1;
    unsigned b : 3;
    signed c : 5;
    unsigned : 0;
    unsigned d : 7;
    _Bool e : 1;
    unsigned f : 20;
    signed g : 12;
    unsigned long long big : 40;
    unsigned char ch : 4;
};

static struct flags flip(struct flags f) {
    f.a = !f.a;
    f.b = f.b + 3;
    f.c = -f.c;
    f.d ^= 0x55;
    f.e = !f.e;
    f.f = f.f * 3 + 1;
    f.g = f.g - 100;
    f.big = (uint64_t)f.big * 5;
    f.ch = f.ch + 9;
    return f;
}

static uint64_t flags_hash(struct flags f) {
    return (uint64_t)f.a + f.b * 2u + (uint64_t)(int64_t)f.c * 17 + f.d * 101u + f.e * 7u + f.f * 13ull +
           (uint64_t)(int64_t)f.g * 29 + (uint64_t)f.big * 3 + f.ch;
}

/* ---- nested aggregates, unions ---- */

struct point { int x, y; };
struct rect { struct point lo, hi; };
struct shape {
    enum { CIRCLE, RECT, POLY } kind;
    union {
        struct { struct point c; int r; } circle;
        struct rect rect;
        struct { int n; struct point v[6]; } poly;
    } u;
    const char *label;
};

static int64_t area2(struct shape s) {
    switch (s.kind) {
    case CIRCLE: return 3 * 2 * (int64_t)s.u.circle.r * s.u.circle.r;
    case RECT: return 2 * (int64_t)(s.u.rect.hi.x - s.u.rect.lo.x) * (s.u.rect.hi.y - s.u.rect.lo.y);
    case POLY: {
        int64_t a = 0;
        for (int i = 0; i < s.u.poly.n; i++) {
            struct point p = s.u.poly.v[i], q = s.u.poly.v[(i + 1) % s.u.poly.n];
            a += (int64_t)p.x * q.y - (int64_t)q.x * p.y;
        }
        return a < 0 ? -a : a;
    }
    }
    return -1;
}

static struct rect bbox(struct shape s) {
    struct rect r;
    switch (s.kind) {
    case CIRCLE:
        r = (struct rect){ { s.u.circle.c.x - s.u.circle.r, s.u.circle.c.y - s.u.circle.r },
                           { s.u.circle.c.x + s.u.circle.r, s.u.circle.c.y + s.u.circle.r } };
        break;
    case RECT: r = s.u.rect; break;
    default:
        r.lo = r.hi = s.u.poly.v[0];
        for (int i = 1; i < s.u.poly.n; i++) {
            struct point p = s.u.poly.v[i];
            if (p.x < r.lo.x) r.lo.x = p.x;
            if (p.y < r.lo.y) r.lo.y = p.y;
            if (p.x > r.hi.x) r.hi.x = p.x;
            if (p.y > r.hi.y) r.hi.y = p.y;
        }
    }
    return r;
}

static const struct shape shapes[] = {
    { CIRCLE, { .circle = { { 1, 2 }, 5 } }, "circle" },
    { .kind = RECT, .u.rect = { .lo = { -3, -4 }, .hi.x = 10, .hi.y = 20 }, .label = "rect" },
    { POLY, { .poly = { 4, { { 0, 0 }, { 4, 0 }, { 4, 3 }, [3] = { 0, 3 } } } }, "quad" },
    { .kind = POLY, .u.poly.n = 5, .u.poly.v = { { 0, 0 }, { 2, -1 }, { 4, 0 }, { 3, 3 }, { 1, 3 } }, .label = "pent" },
};

/* a tagged-union expression tree held by value in an array */
typedef struct expr {
    enum { E_NUM, E_ADD, E_MUL, E_NEG, E_IF } op;
    union {
        int64_t num;
        struct { short l, r; } bin;
        struct { short c, t, f; } cond;
    };
} expr;

static int64_t eval(const expr *pool, int i) {
    expr e = pool[i];   /* by value */
    switch (e.op) {
    case E_NUM: return e.num;
    case E_ADD: return eval(pool, e.bin.l) + eval(pool, e.bin.r);
    case E_MUL: return eval(pool, e.bin.l) * eval(pool, e.bin.r);
    case E_NEG: return -eval(pool, e.bin.l);
    case E_IF: return eval(pool, e.cond.c) ? eval(pool, e.cond.t) : eval(pool, e.cond.f);
    }
    return 0;
}

/* ---- flexible array member ---- */

struct vec {
    size_t n;
    int32_t data[];
};

static union {
    struct vec v;
    unsigned char raw[sizeof(struct vec) + 64 * sizeof(int32_t)];
    max_align_t align;
} vec_storage;

static int64_t vec_sum(const struct vec *v) {
    int64_t s = 0;
    for (size_t i = 0; i < v->n; i++) s += v->data[i];
    return s;
}

/* ---- arrays of structures, assignment, comparison ---- */

struct particle { float x, y, vx, vy; uint16_t id; uint8_t alive : 1, hits : 7; };

static struct particle step(struct particle p, float dt) {
    p.x += p.vx * dt;
    p.y += p.vy * dt;
    if (p.x < 0 || p.x > 100) { p.vx = -p.vx; p.hits++; }
    if (p.y < 0 || p.y > 100) { p.vy = -p.vy; p.hits++; }
    if (p.hits > 20) p.alive = 0;
    return p;
}

int main(void) {
    for (int k = 1; k < 4; k++) {
        double t = take_all(make_s1(k), k, make_s3(k), k * 0.5, make_s4p(k), make_s5(k), (float)k, make_s7(k),
                            make_sif(k), make_sf3(k), make_sd1(k), make_sd2(k), (char)('A' + k), make_sdl(k),
                            make_sld(k), make_sfd(k), make_spad(k), make_si3(k), make_s20(k), make_sd4(k),
                            make_s13(k), make_s100(k));
        mixd(t);
        print_f64(t);
    }
    {
        s13 a = make_s13(3), b;
        b = a;
        b.name[0] = 'Z';
        print_str(a.name, 12);
        print_str(b.name, 12);
        s100 c = make_s100(9);
        mix(sum_bytes(&c, sizeof c));
        s7 d = make_s7(5);
        mix(sum_bytes(&d, sizeof d));
        spad e = make_spad(200);
        mix(e.a + e.c);
        mixd(e.b);
    }
    {
        sd2 r = twice_sd2(make_sd2, 4);
        print_f64(r.a);
        print_f64(r.b);
        s20 x = make_s20(7), y = make_s20(11);
        s20 z = add_s20(x, y);
        print_i64(x.a[1]);
        print_i64(z.a[1]);
        print_i64(z.b);
        make_s20_fn f = make_s20;
        print_i64(f(2).a[1] + make_s20(3).b);
        print_i64(make_sld(5).a >> 30);
        print_f64(make_sfd(2).a);
        print_f64(make_sf3(6).b);
    }
    print_f64(va_structs(8, make_sd2(1), make_s3(2), make_sif(3), make_s100(4), make_sd2(5), make_s3(6),
                         make_sif(7), make_s100(8)));
    {
        struct flags f = { 1, 5, -9, 100, 1, 999999, -2000, 0xffffffffffull, 15 };
        uint64_t fh = 0;
        for (int i = 0; i < 6; i++) {
            f = flip(f);
            fh = fh * 31 + flags_hash(f);
        }
        print_u64(fh);
        print_i64(f.c);
        print_i64(f.g);
        print_u64(f.big);
        print_i64(f.ch);
        struct flags g = { .big = 1, .g = -1, .c = -16 };
        print_i64(g.c + g.g + (int)g.big + g.a + g.b);
    }
    for (size_t i = 0; i < sizeof shapes / sizeof shapes[0]; i++) {
        struct shape s = shapes[i];
        struct rect b = bbox(s);
        print_i64(area2(s));
        print_i64(b.lo.x * 1000 + b.lo.y * 100 + b.hi.x * 10 + b.hi.y);
        const char *l = s.label;
        size_t n = 0;
        while (l[n]) n++;
        print_str(l, n);
    }
    {
        expr pool[] = {
            { E_NUM, { .num = 6 } },
            { E_NUM, { .num = 7 } },
            { .op = E_MUL, .bin = { 0, 1 } },
            { .op = E_NEG, .bin = { 2, 0 } },
            { .op = E_ADD, .bin = { 3, 1 } },
            { .op = E_NUM, .num = 0 },
            { .op = E_IF, .cond = { 5, 2, 4 } },
            { .op = E_IF, .cond = { 4, 6, 0 } },
        };
        print_i64(eval(pool, 4));
        print_i64(eval(pool, 6));
        print_i64(eval(pool, 7));
    }
    {
        struct vec *v = &vec_storage.v;
        v->n = 64;
        for (size_t i = 0; i < v->n; i++) v->data[i] = (int32_t)(i * i) - 1000;
        print_i64(vec_sum(v));
        print_i64((int64_t)sizeof(struct vec) == (int64_t)offsetof(struct vec, data) ||
                  (int64_t)sizeof(struct vec) >= (int64_t)offsetof(struct vec, data));
    }
    {
        struct particle ps[16];
        for (int i = 0; i < 16; i++)
            ps[i] = (struct particle){ .x = (float)(i * 6), .y = (float)(i * 5 % 100), .vx = (float)(i % 5) - 2.0f,
                                       .vy = 1.5f - (float)(i % 3), .id = (uint16_t)(i + 100), .alive = 1 };
        for (int t = 0; t < 500; t++)
            for (int i = 0; i < 16; i++)
                if (ps[i].alive) ps[i] = step(ps[i], 0.75f);
        int alive = 0, hits = 0;
        for (int i = 0; i < 16; i++) {
            alive += ps[i].alive;
            hits += ps[i].hits;
            mixd(ps[i].x);
            mixd(ps[i].y);
        }
        print_i64(alive);
        print_i64(hits);
        struct particle copy[16];
        for (int i = 0; i < 16; i++) copy[i] = ps[15 - i];
        print_i64(copy[0].id);
        print_f64(copy[3].vx);
    }
    print_u64(h);
    return 0;
}

/* expect: 26335.5 */
/* expect: 27403 */
/* expect: 28476.5 */
/* expect: defghijklmno */
/* expect: Zefghijklmno */
/* expect: 13.5 */
/* expect: -22.5 */
/* expect: 7000000049 */
/* expect: 18000000126 */
/* expect: 46254 */
/* expect: 2000023143 */
/* expect: -40 */
/* expect: 2.5 */
/* expect: 3 */
/* expect: 1214.5 */
/* expect: 5348686749811667263 */
/* expect: -9 */
/* expect: 1496 */
/* expect: 1099511612151 */
/* expect: 5 */
/* expect: -16 */
/* expect: 150 */
/* expect: -4233 */
/* expect: circle */
/* expect: 624 */
/* expect: -3280 */
/* expect: rect */
/* expect: 24 */
/* expect: 43 */
/* expect: quad */
/* expect: 22 */
/* expect: -57 */
/* expect: pent */
/* expect: -35 */
/* expect: -35 */
/* expect: -35 */
/* expect: 21344 */
/* expect: 1 */
/* expect: 16 */
/* expect: 127 */
/* expect: 115 */
/* expect: 0 */
/* expect: 10994203718998674981 */
