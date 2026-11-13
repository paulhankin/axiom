/* restrict on block-scope pointers against everything not based on them
   (6.7.3.1p4; ir.restrict_tagged): a load after a store through a block's
   restricted pointer, past a store through a plain parameter (11), a
   global (21), an inlined call's restrict parameter's (41) or a
   non-restrict helper inlined into the block (51), and past an outer
   block's restricted pointer from an inner one's (61), is the stored value;
   a copy of the pointer used after its block (31) and an access after the
   block (71) are loaded again. A copy made in the block is based on it (33).
   In main, a local structure (of a file-scope type) with restrict-qualified
   members is taken apart and its members apart (16 + 81: 97, in main and in its copy inlined into the start-up), as a file-scope restricted pointer is (81); in another function
   the file-scope one is not (101: main's block may not be running). A
   block's static restricted pointer is its block's (91). A structure
   assignment (a fill, 111, or a copy, 112) through a plain pointer is apart
   from a store through a block's restricted pointer. Loops through a block's restricted pointer,
   indexed or incremented, are vectorized without an overlap check. */
// lower-selfhost: --target=native --cpu=x86-64 -O2 --c-freestanding | mov\s+\$0xb,%eax | 1
// lower-selfhost: --target=native --cpu=x86-64 -O2 --c-freestanding | mov\s+\$0x15,%eax | 1
// lower-selfhost: --target=native --cpu=x86-64 -O2 --c-freestanding | mov\s+\$0x1f,%eax | 0
// lower-selfhost: --target=native --cpu=x86-64 -O2 --c-freestanding | mov\s+\$0x21,%eax | 1
// lower-selfhost: --target=native --cpu=x86-64 -O2 --c-freestanding | mov\s+\$0x29,%eax | 1
// lower-selfhost: --target=native --cpu=x86-64 -O2 --c-freestanding | mov\s+\$0x33,%eax | 1
// lower-selfhost: --target=native --cpu=x86-64 -O2 --c-freestanding | mov\s+\$0x3d,%eax | 1
// lower-selfhost: --target=native --cpu=x86-64 -O2 --c-freestanding | mov\s+\$0x47,%eax | 0
// lower-selfhost: --target=native --cpu=x86-64 -O2 --c-freestanding | mov\s+\$0x61,%eax | 2
// lower-selfhost: --target=native --cpu=x86-64 -O2 --c-freestanding | mov\s+\$0x5b,%eax | 1
// lower-selfhost: --target=native --cpu=x86-64 -O2 --c-freestanding | mov\s+\$0x65,%eax | 0
// lower-selfhost: --target=native --cpu=x86-64 -O2 --c-freestanding | \bmulps\b | 2
// lower-selfhost: --target=native --cpu=x86-64 -O2 --c-freestanding | cmp\s+\$0xf, | 0
// lower-selfhost: --target=native --cpu=x86-64 -O2 --c-freestanding | mov\s+\$0x6f,%eax | 1
// lower-selfhost: --target=native --cpu=x86-64 -O2 --c-freestanding | mov\s+\$0x70,%eax | 1
// lower: --target=aarch64 -O2 --c-freestanding | \bldr\s+w\d+,\s+\[x0\] | 2
int g;
int plain_param(int *a, int *b) {
    int *restrict p = a;
    *p = 11;
    *b = 2;
    return *p;
}
int plain_glob(int *a) {
    int *restrict p = a;
    *p = 21;
    g = 2;
    return *p;
}
int esc(int *a, int *b) {
    int *q;
    {
        int *restrict p = a;
        q = p;
    }
    *q = 31;
    *b = 2;
    return *q;
}
int copy_in(int *a, int *b) {
    int *restrict p = a;
    int *q = p;
    *q = 33;
    *b = 2;
    return *p;
}
static inline int one(int *restrict p, int *q) {
    *p = 41;
    *q = 2;
    return *p;
}
int call1(int *a, int *b) { return one(a, b); }
static inline void put(int *q, int v) { *q = v; }
int surv(int *a, int *b) {
    int *restrict p = a;
    *p = 51;
    put(b, 2);
    return *p;
}
int nested(int *a) {
    int *restrict p = a;
    {
        int *restrict q = p + 1;
        *q = 61;
        *p = 2;
        return *q;
    }
}
int after(int *a, int *b) {
    {
        int *restrict p = a;
        *p = 71;
    }
    *b = 2;
    return *a;
}
struct pair { int *restrict x; int *restrict y; };
int *ga, *gb;
int *restrict gr;
int gobj;
int main(void) {
    struct pair s = { ga, gb };
    s.x[0] = 16;
    s.y[0] = 17;
    int t = s.x[0];
    gr = ga;  /* (a file-scope restricted pointer: main's) */
    *gr = 81;
    gobj = 3;
    return t + *gr;
}
int stat(int *a) {
    static int *restrict p;
    p = a;
    *p = 91;
    gobj = 2;
    return *p;
}
int notmain(void) {
    gr = ga;
    *gr = 101;
    gobj = 2;
    return *gr;
}
void vplain(float *y0, const float *x, float a, int n) {
    float *restrict y = y0;
    for (int i = 0; i < n; i++)
        y[i] = a * x[i] + y[i];
}
void incp(float *d0, const float *s, int n) {
    float *restrict d = d0;
    for (int i = 0; i < n; i++)
        *d++ = s[i] * 3.0f;
}
struct big { int v[16]; };
int mset(struct big *a, struct big *b) {
    struct big *restrict p = a;
    p->v[0] = 111;
    *b = (struct big){0};
    return p->v[0];
}
int mcpy(struct big *a, struct big *b, struct big *c) {
    struct big *restrict p = a;
    p->v[0] = 112;
    *b = *c;
    return p->v[0];
}
