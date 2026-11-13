/* The loop transformations' instructions: a copy loop is a memory
   operation (rep movsb for long ones, after the inline paths for up to 32
   bytes), a fill with zeros rep stosb, an element-wise float loop vector
   multiplies and adds (mulps, addps; AArch64 fmul, fadd of .4s), an
   integer sum vector adds, a byte search vector compares (pcmpeqb; cmeq),
   a sign extension of a load one movslq from memory. */
// lower-selfhost: --target=native --cpu=x86-64 -O2 --c-freestanding | rep movsb | 2
// lower-selfhost: --target=native --cpu=x86-64 -O2 --c-freestanding | rep stos | 1
// lower-selfhost: --target=native --cpu=x86-64 -O2 --c-freestanding | \bmulps\b | 1
// lower-selfhost: --target=native --cpu=x86-64 -O2 --c-freestanding | \baddps\b | 1
// lower-selfhost: --target=native --cpu=x86-64 -O2 --c-freestanding | \bpaddd\b | 2
// lower-selfhost: --target=native --cpu=x86-64 -O2 --c-freestanding | \bpcmpeqb\b | 2
// lower-selfhost: --target=native --cpu=x86-64 -O2 --c-freestanding | movslq\s+\(%r | 1
// lower-selfhost: --target=native --cpu=x86-64-v3 -O2 --c-freestanding | \bvmulps\s.*ymm | 1
// lower: --target=aarch64 -O2 --c-freestanding | \bfmul\s+v\d+\.4s | 1
// lower: --target=aarch64 -O2 --c-freestanding | \bfadd\s+v\d+\.4s | 1
// lower: --target=aarch64 -O2 --c-freestanding | \badd\s+v\d+\.4s | 2
// lower: --target=aarch64 -O2 --c-freestanding | \bcmeq\s+v\d+\.16b | 1
// lower: --target=aarch64 -O2 --c-freestanding | \bbl\s+\S+\s+<memmove> | 1
// lower: --target=aarch64 -O2 --c-freestanding | \bbl\s+\S+\s+<memset> | 1
void copy(char *d, const char *s, unsigned long n) {
    for (unsigned long i = 0; i < n; i++)
        d[i] = s[i];
}
void zero(int *p, int n) {
    for (int i = 0; i < n; i++)
        p[i] = 0;
}
void saxpy(float *restrict y, const float *restrict x, float a, int n) {
    for (int i = 0; i < n; i++)
        y[i] = a * x[i] + y[i];
}
int sum(const int *x, int n) {
    int s = 0;
    for (int i = 0; i < n; i++)
        s += x[i];
    return s;
}
long find(const unsigned char *p, long n, unsigned char c) {
    long i;
    for (i = 0; i < n; i++)
        if (p[i] == c)
            break;
    return i;
}
long widen(const int *p, long i) {
    return p[i];
}
