/* A local structure built and then copied whole into its destination is
   built there (copies.forward_into): `top` has no copy loop (H is apart
   from G, which the inlined code reads), `same` keeps its one (the
   destination is read while t is built), and `zero` fills r itself, with
   no copy of a filled temporary. Copy and fill loops on x86-64 align their
   destinations (and $63); on AArch64 a copy loop loads with LDP. */
// lower-selfhost: --target=native --cpu=x86-64 -O2 --c-freestanding | and\s+\$0x3f, | 2
// lower: --target=aarch64 -O2 --c-freestanding | \bldp\s+q\d+, q\d+, \[x\d+\], #64 | 1
typedef struct { int n; unsigned d[400]; } big;
static big G, H;

static void build(big *r, const big *a) {
    big t;
    for (int i = 0; i < 400; i++) t.d[i] = a->d[399 - i] + 1u;
    t.n = a->n;
    while (t.n > 0 && t.d[t.n - 1] == 0) t.n--;
    *r = t;
}

void top(void) { build(&H, &G); }
void same(void) { build(&G, &G); }
void zero(big *r) { *r = (big){ 0 }; }
