/* restrict on block-scope pointers, structure members and the parameters
   of inlined functions (ir.IN_RSCOPE, ir.IN_RESTRICT): a load after a
   store through another restricted pointer of the same block is the stored
   value (11; without restrict it is loaded again: 14), and so after
   inlining (12) and for two restrict-qualified members (1001, in main, and in its copy inlined into the start-up); a loop
   through block-scope restricted pointers, or inlined restrict parameters,
   is vectorized without the overlap check that plain pointers need (two:
   s2's, and inbody's, whose restricted pointers are a block's in the
   loop's body: each iteration another execution of it, but still
   vectorized). */
// lower-selfhost: --target=native --cpu=x86-64 -O2 --c-freestanding | mov\s+\$0xb,%eax | 1
// lower-selfhost: --target=native --cpu=x86-64 -O2 --c-freestanding | mov\s+\$0xe,%eax | 0
// lower-selfhost: --target=native --cpu=x86-64 -O2 --c-freestanding | mov\s+\$0xc,%eax | 1
// lower-selfhost: --target=native --cpu=x86-64 -O2 --c-freestanding | mov\s+\$0x3e9,%eax | 2
// lower-selfhost: --target=native --cpu=x86-64 -O2 --c-freestanding | \bmulps\b | 4
// lower-selfhost: --target=native --cpu=x86-64 -O2 --c-freestanding | cmp\s+\$0xf, | 2
// lower: --target=aarch64 -O2 --c-freestanding | \bldr\s+w\d+,\s+\[x0\] | 1
int blk(int *a, int *b) {
    int *restrict p = a;
    int *restrict q = b;
    *p = 11;
    *q = 2;
    return *p;
}
int plain(int *a, int *b) {
    int *p = a, *q = b;
    *p = 14;
    *q = 2;
    return *p;
}
static inline int in(int *restrict p, int *restrict q) {
    *p = 12;
    *q = 13;
    return *p;
}
int inl(int *a, int *b) { return in(a, b); }
struct pair { int *restrict x; int *restrict y; };
int *ga, *gb;
int main(void) {
    struct pair s = { ga, gb };  /* (a file-scope structure type: main's) */
    s.x[0] = 1001;
    s.y[0] = 17;
    return s.x[0];
}
void s1(float *y0, const float *x0, float a, int n) {
    float *restrict y = y0;
    const float *restrict x = x0;
    for (int i = 0; i < n; i++)
        y[i] = a * x[i] + y[i];
}
void s2(float *y, const float *x, float a, int n) {
    for (int i = 0; i < n; i++)
        y[i] = a * x[i] + y[i];
}
static inline void sx(float *restrict y, const float *restrict x, float a, int n) {
    for (int i = 0; i < n; i++)
        y[i] = a * x[i] + y[i];
}
void sxc(float *y, const float *x, int n) { sx(y, x, 2.0f, n); }
void inbody(float *a, const float *b, int n) {
    for (int i = 0; i < n; i++) {
        float *restrict p = a;
        const float *restrict q = b;
        p[i] = q[i] * 2.0f;
    }
}
